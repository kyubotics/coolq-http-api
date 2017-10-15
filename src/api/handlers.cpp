#include "app.h"

#include <boost/filesystem.hpp>

#include "./types.h"
#include "structs.h"

using namespace std;
using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;
using Response = HttpServer::Response;
using Request = HttpServer::Request;
using RetCodes = ApiResult::RetCodes;

ApiHandlerMap api_handlers;

static bool __add_api_handler(const string &name, ApiHandler handler) {
    api_handlers[name] = handler;
    return true;
}

#define HANDLER(handler_name) \
    static void __##handler_name(const ApiParams &, ApiResult &); \
    static bool __dummy_##handler_name = __add_api_handler(#handler_name, __##handler_name); \
    static void __##handler_name(const ApiParams &params, ApiResult &result)

static void handle_async(const ApiParams &params, ApiResult &result, ApiHandler handler) {
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
            result.retcode = RetCodes::OK;;
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

        const auto count = pack.pop_int32(); // get number of groups
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

            const auto count = pack.pop_int32();
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

HANDLER(get_version_info) {
    auto root_dir = get_coolq_root();
    string coolq_edition = "air";
    if (boost::filesystem::is_regular_file(ansi(root_dir + "CQP.exe"))) {
        coolq_edition = "pro";
    }
    auto plugin_version = CQAPP_VERSION;
    result.retcode = RetCodes::OK;
    result.data = {
        {"coolq_edition", coolq_edition},
        {"plugin_version", plugin_version}
    };
}

#pragma endregion
