#include "./action.h"

#include <Windows.h>
#include <boost/filesystem.hpp>
#include <boost/process.hpp>
#include <set>

#include "cqhttp/core/core.h"
#include "cqhttp/utils/filesystem.h"
#include "cqhttp/utils/http.h"

using namespace std;
namespace api = cq::api;
namespace fs = boost::filesystem;

namespace cqhttp {
    using Codes = ActionResult::Codes;

    using ActionHandler = function<void(const utils::JsonEx &, ActionResult &)>;
    using ActionHandlerMap = unordered_map<string, ActionHandler>;

    using ApiError = cq::exception::ApiError;

    static ActionHandlerMap action_handlers;

    ActionResult call_action(const string &action, const json &params) {
        utils::JsonEx params_ex = params;

        ActionResult result;
        __app.on_before_action(action, params_ex, result);

        if (const auto it = action_handlers.find(action); it != action_handlers.end()) {
            it->second(params_ex, result);
        } else {
            __app.on_missed_action(action, params_ex, result);
        }

        __app.on_after_action(action, params_ex, result);
        return result;
    }

    static bool add_action_handler(const string &name, const ActionHandler handler) {
        action_handlers[name] = handler;
        return true;
    }

#define HANDLER(handler_name)                                                                 \
    static void __##handler_name(const utils::JsonEx &, ActionResult &);                      \
    static bool __dummy_##handler_name = add_action_handler(#handler_name, __##handler_name); \
    static void __##handler_name(const utils::JsonEx &params, ActionResult &result)

    static int to_retcode(const ApiError &err) {
        if (err.code <= 0) {
            return err.code;
        }
        switch (err.code) {
        case ApiError::INVALID_DATA:
            return Codes::INVALID_DATA;
        case ApiError::INVALID_TARGET:
        default:
            return Codes::DEFAULT_ERROR;
        }
    }

    static void call_func_and_set_retcode(const function<void()> func, ActionResult &result) {
        try {
            func();
            result.code = Codes::OK;
        } catch (const ApiError &e) {
            result.code = to_retcode(e);
        }
    }

#define CALL_API_BEGIN try {
#define CALL_API_END                 \
    result.code = Codes::OK;         \
    }                                \
    catch (const ApiError &e) {      \
        result.code = to_retcode(e); \
    }

#pragma region Send Message

    HANDLER(send_private_msg) {
        const auto user_id = params.get_integer("user_id", 0);
        const auto message = params.get_message();
        if (user_id && !message.empty()) {
            CALL_API_BEGIN
            result.data = {{"message_id", api::send_private_msg(user_id, message)}};
            CALL_API_END
        }
    }

    HANDLER(send_group_msg) {
        const auto group_id = params.get_integer("group_id", 0);
        const auto message = params.get_message();
        if (group_id && !message.empty()) {
            CALL_API_BEGIN
            result.data = {{"message_id", api::send_group_msg(group_id, message)}};
            CALL_API_END
        }
    }

    HANDLER(send_discuss_msg) {
        const auto discuss_id = params.get_integer("discuss_id", 0);
        const auto message = params.get_message();
        if (discuss_id && !message.empty()) {
            CALL_API_BEGIN
            result.data = {{"message_id", api::send_discuss_msg(discuss_id, message)}};
            CALL_API_END
        }
    }

    HANDLER(send_msg) {
        const auto message_type = params.get_string("message_type");
        if (message_type == "private") {
            __send_private_msg(params, result);
        } else if (message_type == "group") {
            __send_group_msg(params, result);
        } else if (message_type == "discuss") {
            __send_discuss_msg(params, result);
        }
    }

    HANDLER(delete_msg) {
        const auto message_id = params.get_integer("message_id", 0);
        if (message_id > 0) {
            CALL_API_BEGIN
            api::delete_msg(message_id);
            CALL_API_END
        }
    }

#pragma endregion

#pragma region Send Like

    HANDLER(send_like) {
        // CoolQ Pro only
        const auto user_id = params.get_integer("user_id", 0);
        const auto times = static_cast<int32_t>(params.get_integer("times", 1));
        if (user_id && times > 0) {
            CALL_API_BEGIN
            if (times == 1) {
                api::send_like(user_id);
            } else {
                api::send_like(user_id, times);
            }
            CALL_API_END
        }
    }

#pragma endregion

#pragma region Group &Discuss Operation

    HANDLER(set_group_kick) {
        const auto group_id = params.get_integer("group_id", 0);
        const auto user_id = params.get_integer("user_id", 0);
        const auto reject_add_request = params.get_bool("reject_add_request", false);
        if (group_id && user_id) {
            CALL_API_BEGIN
            api::set_group_kick(group_id, user_id, reject_add_request);
            CALL_API_END
        }
    }

    HANDLER(set_group_ban) {
        const auto group_id = params.get_integer("group_id", 0);
        const auto user_id = params.get_integer("user_id", 0);
        const auto duration = params.get_integer("duration", 30 * 60 /* 30 minutes */);
        if (group_id && user_id && duration >= 0) {
            CALL_API_BEGIN
            api::set_group_ban(group_id, user_id, duration);
            CALL_API_END
        }
    }

    HANDLER(set_group_anonymous_ban) {
        const auto group_id = params.get_integer("group_id", 0);
        const auto anonymous_flag = params.get_string("flag", "");
        const auto duration = params.get_integer("duration", 30 * 60 /* 30 minutes */);
        if (group_id && !anonymous_flag.empty() && duration >= 0) {
            CALL_API_BEGIN
            api::set_group_anonymous_ban(group_id, anonymous_flag, duration);
            CALL_API_END
        }
    }

    HANDLER(set_group_whole_ban) {
        const auto group_id = params.get_integer("group_id", 0);
        const auto enable = params.get_bool("enable", true);
        if (group_id) {
            CALL_API_BEGIN
            api::set_group_whole_ban(group_id, enable);
            CALL_API_END
        }
    }

    HANDLER(set_group_admin) {
        const auto group_id = params.get_integer("group_id", 0);
        const auto user_id = params.get_integer("user_id", 0);
        const auto enable = params.get_bool("enable", true);
        if (group_id && user_id) {
            CALL_API_BEGIN
            api::set_group_admin(group_id, user_id, enable);
            CALL_API_END
        }
    }

    HANDLER(set_group_anonymous) {
        // CoolQ Pro only
        const auto group_id = params.get_integer("group_id", 0);
        const auto enable = params.get_bool("enable", true);
        if (group_id) {
            CALL_API_BEGIN
            api::set_group_anonymous(group_id, enable);
            CALL_API_END
        }
    }

    HANDLER(set_group_card) {
        const auto group_id = params.get_integer("group_id", 0);
        const auto user_id = params.get_integer("user_id", 0);
        const auto card = params.get_string("card", "");
        if (group_id && user_id) {
            CALL_API_BEGIN
            api::set_group_card(group_id, user_id, card);
            CALL_API_END
        }
    }

    HANDLER(set_group_leave) {
        const auto group_id = params.get_integer("group_id", 0);
        const auto is_dismiss = params.get_bool("is_dismiss", false);
        if (group_id) {
            CALL_API_BEGIN
            api::set_group_leave(group_id, is_dismiss);
            CALL_API_END
        }
    }

    HANDLER(set_group_special_title) {
        const auto group_id = params.get_integer("group_id", 0);
        const auto user_id = params.get_integer("user_id", 0);
        const auto special_title = params.get_string("special_title", "");
        const auto duration = params.get_integer("duration", -1 /* permanent */); // seems to have no effect
        if (group_id && user_id) {
            CALL_API_BEGIN
            api::set_group_special_title(group_id, user_id, special_title, duration);
            CALL_API_END
        }
    }

    HANDLER(set_discuss_leave) {
        const auto discuss_id = params.get_integer("discuss_id", 0);
        if (discuss_id) {
            CALL_API_BEGIN
            api::set_discuss_leave(discuss_id);
            CALL_API_END
        }
    }

#pragma endregion

#pragma region Request Operation

    HANDLER(set_friend_add_request) {
        const auto flag = params.get_string("flag", "");
        const auto approve = params.get_bool("approve", true);
        const auto remark = params.get_string("remark", "");
        if (!flag.empty()) {
            CALL_API_BEGIN
            api::set_friend_add_request(flag, approve ? cq::request::APPROVE : cq::request::REJECT, remark);
            CALL_API_END
        }
    }

    HANDLER(set_group_add_request) {
        const auto flag = params.get_string("flag", "");
        const auto type = params.get_string("type", "");
        const auto approve = params.get_bool("approve", true);
        const auto reason = params.get_string("reason", "");
        auto request_type = -1;
        if (type == "add") {
            request_type = cq::request::GROUP_ADD;
        } else if (type == "invite") {
            request_type = cq::request::GROUP_INVITE;
        }
        if (!flag.empty() && request_type != -1) {
            CALL_API_BEGIN
            api::set_group_add_request(flag,
                                       static_cast<cq::request::SubType>(request_type),
                                       approve ? cq::request::APPROVE : cq::request::REJECT,
                                       reason);
            CALL_API_END
        }
    }

#pragma endregion

#pragma region Get QQ Information

    HANDLER(get_login_info) {
        CALL_API_BEGIN
        result.data["user_id"] = api::get_login_user_id();
        result.data["nickname"] = api::get_login_nickname();
        CALL_API_END
    }

    HANDLER(get_stranger_info) {
        const auto user_id = params.get_integer("user_id", 0);
        const auto no_cache = params.get_bool("no_cache", false);
        if (user_id) {
            CALL_API_BEGIN
            result.data = api::get_stranger_info(user_id, no_cache);
            CALL_API_END
        }
    }

    HANDLER(get_group_list) {
        CALL_API_BEGIN
        result.data = api::get_group_list();
        CALL_API_END
    }

    HANDLER(get_group_member_list) {
        const auto group_id = params.get_integer("group_id", 0);
        if (group_id) {
            CALL_API_BEGIN
            result.data = api::get_group_member_list(group_id);
            CALL_API_END
        }
    }

    HANDLER(get_group_member_info) {
        const auto group_id = params.get_integer("group_id", 0);
        const auto user_id = params.get_integer("user_id", 0);
        const auto no_cache = params.get_bool("no_cache", false);
        if (group_id && user_id) {
            CALL_API_BEGIN
            result.data = api::get_group_member_info(group_id, user_id, no_cache);
            CALL_API_END
        }
    }

#pragma endregion

#pragma region Get CoolQ Information

    HANDLER(get_cookies) {
        CALL_API_BEGIN
        result.data = {{"cookies", api::get_cookies()}};
        CALL_API_END
    }

    HANDLER(get_csrf_token) {
        CALL_API_BEGIN
        result.data = {{"token", api::get_csrf_token()}};
        CALL_API_END
    }

    HANDLER(get_record) {
        const auto file = params.get_string("file");
        const auto out_format = params.get_string("out_format");
        if (!file.empty() && !out_format.empty()) {
            CALL_API_BEGIN
            result.data = {{"file", api::get_record(file, out_format)}};
            CALL_API_END
        }
    }

#pragma endregion

#pragma region Extras

    HANDLER(get_status) {
        result.code = Codes::OK;

        const auto app_initialized = __app.is_initialized();
        const auto app_enabled = __app.is_enabled();

        result.data = {{"app_initialized", app_initialized}, {"app_enabled", app_enabled}};

        ActionResult tmp_result;
        __get_stranger_info(json{{"user_id", 10000}, {"no_cache", true}}, tmp_result);

        auto online = tmp_result.code == Codes::OK;
        result.data["online"] = online;

        result.data["good"] = app_initialized && app_enabled && online;
    }

#ifdef _DEBUG
#define BUILD_CONFIGURATION "debug"
#else
#define BUILD_CONFIGURATION "release"
#endif

    HANDLER(get_version_info) {
        const auto coolq_directory = cq::dir::root();
        string coolq_edition = "air";
        if (fs::is_regular_file(ansi(coolq_directory + "CQP.exe"))) {
            coolq_edition = "pro";
        }
        result.code = Codes::OK;
        result.data = {{"coolq_directory", coolq_directory},
                       {"coolq_edition", coolq_edition},
                       {"plugin_version", CQHTTP_VERSION},
                       {"plugin_build_number", CQHTTP_BUILD_NUMBER},
                       {"plugin_build_configuration", BUILD_CONFIGURATION}};
    }

    HANDLER(set_restart) {
        constexpr size_t size = 1024;
        wchar_t w_exec_path[size]{};
        GetModuleFileNameW(nullptr, w_exec_path, size);

        const auto restart_batch_path = cq::dir::app_tmp() + "restart.bat";
        const auto ansi_restart_batch_path = ansi(restart_batch_path);
        if (ofstream f(ansi_restart_batch_path); f.is_open()) {
            f << "taskkill /F /PID " << _getpid() << "\r\n"
              << "timeout 1 > NUL\r\n"
              << "start \"\" \"" << ansi(ws2s(w_exec_path)) << "\" /account " << api::get_login_user_id();
        }

        try {
            boost::process::spawn(ansi_restart_batch_path);
            result.code = Codes::OK;
        } catch (exception &) {
            result.code = Codes::OPERATION_FAILED;
        }
    }

    HANDLER(set_restart_plugin) {
        // app.restart_async(2000);
        // result.code = Codes::ASYNC;
    }

    HANDLER(clean_data_dir) {
        const auto data_dir = params.get_string("data_dir");
        set<string> allowed_data_dirs = {"image", "record", "show", "bface"};
        if (allowed_data_dirs.find(data_dir) != allowed_data_dirs.cend()) {
            const auto ws_dir_fullpath = s2ws(utils::data_file_full_path(data_dir, ""));
            try {
                fs::remove_all(ws_dir_fullpath);
                fs::create_directory(ws_dir_fullpath);
                result.code = Codes::OK;
            } catch (fs::filesystem_error &) {
                result.code = Codes::OPERATION_FAILED;
            }
        }
    }

#pragma endregion

#pragma region Experimental

    HANDLER(_get_friend_list) {
        try {
            const auto cookies = api::get_cookies();
            const auto g_tk = to_string(api::get_csrf_token());
            const auto login_qq = to_string(api::get_login_user_id());

            {
                // try mobile QZone API
                const auto url = "http://m.qzone.com/friend/mfriend_list?g_tk=" + g_tk + "&res_uin=" + login_qq
                                 + "&res_type=normal&format=json";
                const auto res = utils::http::get_json(url, true, cookies).value_or(nullptr);
                try {
                    if (res.at("code").get<int>() == 0) {
                        // succeeded
                        auto resp_data = res.at("data");
                        result.data = json::array();

                        map<int64_t, int> gpid_idx_map;
                        for (auto gp : resp_data.at("gpnames")) {
                            auto res_gp = json::object();
                            auto gpid = gp.at("gpid").get<int64_t>();
                            res_gp["friend_group_id"] = gpid;
                            res_gp["friend_group_name"] = gp.at("gpname").get<string>();
                            res_gp["friends"] = json::array();
                            gpid_idx_map[gpid] = result.data.size();
                            result.data.push_back(res_gp);
                        }

                        for (auto frnd : resp_data.at("list")) {
                            auto gpid = frnd.at("groupid").get<int64_t>();
                            auto res_frnd = json::object();
                            res_frnd["user_id"] = frnd.at("uin").get<int64_t>();
                            res_frnd["nickname"] = frnd.at("nick").get<string>();
                            res_frnd["remark"] = frnd.at("remark").get<string>();
                            result.data[gpid_idx_map[gpid]]["friends"].push_back(res_frnd);
                        }

                        result.code = Codes::OK;
                        return;
                    }
                } catch (exception &) {
                }
            }

            {
                // try desktop web QZone API
                const auto url =
                    "https://h5.qzone.qq.com/proxy/domain/r.qzone.qq.com/cgi-bin/tfriend/"
                    "friend_show_qqfriends.cgi?g_tk="
                    + g_tk + "&uin=" + login_qq;
                const auto res = utils::http::get_json(url, true, cookies).value_or(nullptr);
                try {
                    auto resp_data = res;
                    result.data = json::array();

                    map<int64_t, int> gpid_idx_map;
                    for (auto gp : resp_data.at("gpnames")) {
                        auto res_gp = json::object();
                        auto gpid = gp.at("gpid").get<int64_t>();
                        res_gp["friend_group_id"] = gpid;
                        res_gp["friend_group_name"] = gp.at("gpname").get<string>();
                        res_gp["friends"] = json::array();
                        gpid_idx_map[gpid] = result.data.size();
                        result.data.push_back(res_gp);
                    }

                    for (auto frnd : resp_data.at("items")) {
                        auto gpid = frnd.at("groupid").get<int64_t>();
                        auto res_frnd = json::object();
                        res_frnd["user_id"] = frnd.at("uin").get<int64_t>();
                        res_frnd["nickname"] = frnd.at("name").get<string>();
                        res_frnd["remark"] = frnd.at("remark").get<string>();
                        result.data[gpid_idx_map[gpid]]["friends"].push_back(res_frnd);
                    }

                    result.code = Codes::OK;
                    return;
                } catch (exception &) {
                }
            }

            // failed
            result.data = nullptr;
        } catch (const ApiError &e) {
            result.code = to_retcode(e);
        }
    }

#pragma endregion

} // namespace cqhttp
