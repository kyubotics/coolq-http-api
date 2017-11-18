// 
// handlers.cpp : Implement API handlers.
// 
// Copyright (C) 2017  Richard Chien <richardchienthebest@gmail.com>
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
// 

#include "app.h"

#include <boost/filesystem.hpp>
#include <boost/process.hpp>

#include "./types.h"
#include "structs.h"
#include "utils/params_class.h"
#include "service/hub_class.h"

using namespace std;
namespace fs = boost::filesystem;

ApiHandlerMap api_handlers;

using RetCodes = ApiResult::RetCodes;

extern ApiHandlerMap api_handlers;

static bool __add_api_handler(const string &name, ApiHandler handler) {
    api_handlers[name] = handler;
    return true;
}

#define HANDLER(handler_name) \
    static void __##handler_name(const Params &, ApiResult &); \
    static bool __dummy_##handler_name = __add_api_handler(#handler_name, __##handler_name); \
    static void __##handler_name(const Params &params, ApiResult &result)

static void handle_async(const Params &params, ApiResult &result, ApiHandler handler) {
    static const auto TAG = u8"API异步";
    if (pool) {
        pool->push([params, result, handler](int) {
            // copy "params" and "result" in the async task
            auto async_params = params;
            auto async_result = result;
            handler(async_params, async_result);
            Log::d(TAG, u8"成功执行一个 API 请求异步处理任务");
        });
        Log::d(TAG, u8"API 请求异步处理任务已进入线程池等待执行");
        result.retcode = RetCodes::ASYNC;
    } else {
        Log::d(TAG, u8"工作线程池未正确初始化，无法进行异步处理，请尝试重启插件");
        result.retcode = RetCodes::BAD_THREAD_POOL;
    }
}

#pragma region Send Message

HANDLER(send_private_msg) {
    auto user_id = params.get_integer("user_id", 0);
    auto message = params.get_message();
    if (user_id && !message.empty()) {
        result.retcode = sdk->send_private_msg(user_id, message);
    }
}

HANDLER(send_private_msg_async) {
    handle_async(params, result, __send_private_msg);
}

HANDLER(send_group_msg) {
    auto group_id = params.get_integer("group_id", 0);
    auto message = params.get_message();
    if (group_id && !message.empty()) {
        result.retcode = sdk->send_group_msg(group_id, message);
    }
}

HANDLER(send_group_msg_async) {
    handle_async(params, result, __send_group_msg);
}

HANDLER(send_discuss_msg) {
    auto discuss_id = params.get_integer("discuss_id", 0);
    auto message = params.get_message();
    if (discuss_id && !message.empty()) {
        result.retcode = sdk->send_discuss_msg(discuss_id, message);
    }
}

HANDLER(send_discuss_msg_async) {
    handle_async(params, result, __send_discuss_msg);
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

HANDLER(send_msg_async) {
    handle_async(params, result, __send_msg);
}

#pragma endregion

#pragma region Send Like

HANDLER(send_like) {
    // CoolQ Pro only
    auto user_id = params.get_integer("user_id", 0);
    auto times = static_cast<int32_t>(params.get_integer("times", 1));
    if (user_id && times > 0) {
        if (times == 1) {
            result.retcode = sdk->send_like(user_id);
        } else {
            result.retcode = sdk->send_like(user_id, times);
        }
    }
}

#pragma endregion

#pragma region Group & Discuss Operation

HANDLER(set_group_kick) {
    auto group_id = params.get_integer("group_id", 0);
    auto user_id = params.get_integer("user_id", 0);
    auto reject_add_request = params.get_bool("reject_add_request", false);
    if (group_id && user_id) {
        result.retcode = sdk->set_group_kick(group_id, user_id, reject_add_request);
    }
}

HANDLER(set_group_ban) {
    auto group_id = params.get_integer("group_id", 0);
    auto user_id = params.get_integer("user_id", 0);
    auto duration = params.get_integer("duration", 30 * 60 /* 30 minutes */);
    if (group_id && user_id && duration >= 0) {
        result.retcode = sdk->set_group_ban(group_id, user_id, duration);
    }
}

HANDLER(set_group_anonymous_ban) {
    auto group_id = params.get_integer("group_id", 0);
    auto anonymous_flag = params.get_string("flag", "");
    auto duration = params.get_integer("duration", 30 * 60 /* 30 minutes */);
    if (group_id && !anonymous_flag.empty() && duration >= 0) {
        result.retcode = sdk->set_group_anonymous_ban(group_id, anonymous_flag, duration);
    }
}

HANDLER(set_group_whole_ban) {
    auto group_id = params.get_integer("group_id", 0);
    auto enable = params.get_bool("enable", true);
    if (group_id) {
        result.retcode = sdk->set_group_whole_ban(group_id, enable);
    }
}

HANDLER(set_group_admin) {
    auto group_id = params.get_integer("group_id", 0);
    auto user_id = params.get_integer("user_id", 0);
    auto enable = params.get_bool("enable", true);
    if (group_id && user_id) {
        result.retcode = sdk->set_group_admin(group_id, user_id, enable);
    }
}

HANDLER(set_group_anonymous) {
    // CoolQ Pro only
    auto group_id = params.get_integer("group_id", 0);
    auto enable = params.get_bool("enable", true);
    if (group_id) {
        result.retcode = sdk->set_group_anonymous(group_id, enable);
    }
}

HANDLER(set_group_card) {
    auto group_id = params.get_integer("group_id", 0);
    auto user_id = params.get_integer("user_id", 0);
    auto card = params.get_string("card", "");
    if (group_id && user_id) {
        result.retcode = sdk->set_group_card(group_id, user_id, card);
    }
}

HANDLER(set_group_leave) {
    auto group_id = params.get_integer("group_id", 0);
    auto is_dismiss = params.get_bool("is_dismiss", false);
    if (group_id) {
        result.retcode = sdk->set_group_leave(group_id, is_dismiss);
    }
}

HANDLER(set_group_special_title) {
    auto group_id = params.get_integer("group_id", 0);
    auto user_id = params.get_integer("user_id", 0);
    auto special_title = params.get_string("special_title", "");
    auto duration = params.get_integer("duration", -1 /* permanent */); // seems to have no effect
    if (group_id && user_id) {
        result.retcode = sdk->set_group_special_title(group_id, user_id, special_title, duration);
    }
}

HANDLER(set_discuss_leave) {
    auto discuss_id = params.get_integer("discuss_id", 0);
    if (discuss_id) {
        result.retcode = sdk->set_discuss_leave(discuss_id);
    }
}

#pragma endregion

#pragma region Request Operation

HANDLER(set_friend_add_request) {
    auto flag = params.get_string("flag", "");
    auto approve = params.get_bool("approve", true);
    auto remark = params.get_string("remark", "");
    if (!flag.empty()) {
        result.retcode = sdk->set_friend_add_request(flag, approve ? CQREQUEST_ALLOW : CQREQUEST_DENY, remark);
    }
}

HANDLER(set_group_add_request) {
    auto flag = params.get_string("flag", "");
    auto type = params.get_string("type", "");
    auto approve = params.get_bool("approve", true);
    auto reason = params.get_string("reason", "");
    auto request_type = -1;
    if (type == "add") {
        request_type = CQREQUEST_GROUPADD;
    } else if (type == "invite") {
        request_type = CQREQUEST_GROUPINVITE;
    }
    if (!flag.empty() && request_type != -1) {
        result.retcode = sdk->set_group_add_request(flag, request_type, approve ? CQREQUEST_ALLOW : CQREQUEST_DENY,
                                                    reason);
    }
}

#pragma endregion

#pragma region Get QQ Information

HANDLER(get_login_info) {
    auto id = sdk->get_login_qq();
    auto nickname = sdk->get_login_nick();
    result.retcode = nickname.empty() ? RetCodes::INVALID_DATA : RetCodes::OK;
    result.data = {
        {"user_id", id},
        {"nickname", nickname}
    };
}

HANDLER(get_stranger_info) {
    auto user_id = params.get_integer("user_id", 0);
    auto no_cache = params.get_bool("no_cache", false);
    if (user_id) {
        auto bytes = sdk->get_stranger_info_raw(user_id, no_cache);
        if (bytes.size() >= Stranger::MIN_SIZE) {
            auto stranger = Stranger::from_bytes(bytes);
            result.data = stranger.json();
            result.retcode = RetCodes::OK;
        } else {
            result.retcode = RetCodes::INVALID_DATA;
        }
    }
}

HANDLER(get_group_list) {
    auto bytes = sdk->get_group_list_raw();
    if (bytes.size() >= 4 /* at least has a count */) {
        auto pack = Pack(bytes);

        auto group_list = json::array();

        const auto count = pack.pop_int<int32_t>(); // get number of groups
        for (auto i = 0; i < count; i++) {
            const auto token = pack.pop_token();
            auto group = Group::from_bytes(token);
            group_list.push_back(group.json());
        }

        result.data = group_list;
        result.retcode = RetCodes::OK;
    } else {
        result.retcode = RetCodes::INVALID_DATA;
    }
}

HANDLER(get_group_member_list) {
    auto group_id = params.get_integer("group_id", 0);
    if (group_id) {
        auto bytes = sdk->get_group_member_list_raw(group_id);
        if (bytes.size() >= 4 /* at least has a count */) {
            auto pack = Pack(bytes);

            auto member_list = json::array();

            const auto count = pack.pop_int<int32_t>();
            for (auto i = 0; i < count; i++) {
                const auto token = pack.pop_token();
                auto member = GroupMember::from_bytes(token);
                member_list.push_back(member.json());
            }

            result.data = member_list;
            result.retcode = RetCodes::OK;
        } else {
            result.retcode = RetCodes::INVALID_DATA;
        }
    }
}

HANDLER(get_group_member_info) {
    auto group_id = params.get_integer("group_id", 0);
    auto user_id = params.get_integer("user_id", 0);
    auto no_cache = params.get_bool("no_cache", false);
    if (group_id && user_id) {
        auto bytes = sdk->get_group_member_info_raw(group_id, user_id, no_cache);
        if (bytes.size() >= GroupMember::MIN_SIZE) {
            auto member = GroupMember::from_bytes(bytes);
            result.data = member.json();
            result.retcode = RetCodes::OK;
        } else {
            result.retcode = RetCodes::INVALID_DATA;
        }
    }
}

#pragma endregion

#pragma region Get CoolQ Information

HANDLER(get_cookies) {
    auto cookies = sdk->get_cookies();
    result.retcode = cookies.empty() ? RetCodes::INVALID_DATA : RetCodes::OK;
    result.data = {{"cookies", cookies}};
}

HANDLER(get_csrf_token) {
    auto token = sdk->get_csrf_token();
    result.retcode = token ? RetCodes::OK : RetCodes::INVALID_DATA;
    result.data = {{"token", token}};
}

#pragma endregion

#pragma region Extras

HANDLER(get_status) {
    result.retcode = RetCodes::OK;
    result.data = {
        {"app_initialized", app.is_initialized()},
        {"app_enabled", app.is_enabled()}
    };

    for (const auto &entry : ServiceHub::instance().get_services()) {
        result.data[entry.first + "_service_good"] = entry.second->good();
    }

    const Params tmp_params(json{
        {"user_id", 10000},
        {"no_cache", true}
    });
    ApiResult tmp_result;
    __get_stranger_info(tmp_params, tmp_result);

    auto online = tmp_result.retcode == RetCodes::OK;
    result.data["online"] = online;

    result.data["good"] = app.is_initialized()
            && (app.is_enabled() && ServiceHub::instance().good()
                || !app.is_enabled())
            && online;
}

#ifdef _DEBUG
#define BUILD_CONFIGURATION "debug"
#else
#define BUILD_CONFIGURATION "release"
#endif

HANDLER(get_version_info) {
    const auto coolq_directory = sdk->directories().coolq();
    string coolq_edition = "air";
    if (fs::is_regular_file(ansi(coolq_directory + "CQP.exe"))) {
        coolq_edition = "pro";
    }
    result.retcode = RetCodes::OK;
    result.data = {
        {"coolq_directory", coolq_directory},
        {"coolq_edition", coolq_edition},
        {"plugin_version", CQAPP_VERSION},
        {"plugin_build_number", CQAPP_BUILD_NUMBER},
        {"plugin_build_configuration", BUILD_CONFIGURATION}
    };
}

HANDLER(set_restart) {
    constexpr size_t size = 1024;
    wchar_t w_exec_path[size]{};
    GetModuleFileName(nullptr, w_exec_path, size);

    const auto restart_batch_path = sdk->directories().app_tmp() + "restart.bat";
    const auto ansi_restart_batch_path = ansi(restart_batch_path);
    if (ofstream f(ansi_restart_batch_path); f.is_open()) {
        f << "taskkill /F /PID " << _getpid() << "\r\n"
                << "timeout 1 > NUL\r\n"
                << "start \"\" \"" << ansi(ws2s(w_exec_path)) << "\" /account " << sdk->get_login_qq();
    }

    try {
        boost::process::spawn(ansi_restart_batch_path);
        result.retcode = RetCodes::OK;
    } catch (exception &) { }
}

HANDLER(set_restart_plugin) {
    app.restart_async(2000);
    result.retcode = RetCodes::ASYNC;
}

#pragma endregion

#pragma region Experimental

HANDLER(_get_friend_list) {
    const auto cookies = sdk->get_cookies();
    const auto g_tk = to_string(sdk->get_csrf_token());
    const auto login_qq = to_string(sdk->get_login_qq());

    {
        // try mobile QZone API
        const auto url = "http://m.qzone.com/friend/mfriend_list?g_tk=" + g_tk + "&res_uin=" + login_qq +
                "&res_type=normal&format=json";
        const auto res = get_remote_json(url, true, cookies).value_or(nullptr);
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

                result.retcode = RetCodes::OK;
                return;
            }
        } catch (exception &) {}
    }

    {
        // try desktop web QZone API
        const auto url =
                "https://h5.qzone.qq.com/proxy/domain/r.qzone.qq.com/cgi-bin/tfriend/friend_show_qqfriends.cgi?g_tk=" +
                g_tk + "&uin=" + login_qq;
        const auto res = get_remote_json(url, true, cookies).value_or(nullptr);
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

            result.retcode = RetCodes::OK;
            return;
        } catch (exception &) {}
    }

    // failed
    result.data = nullptr;
}

#pragma endregion
