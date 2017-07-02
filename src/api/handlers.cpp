// 
// handlers.cpp : Add and implement API handlers.
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

#include "ApiRequest.h"
#include "structs.h"
#include "Pack.h"
#include "types.h"
#include "helpers.h"

using namespace std;

ApiHandlerMap api_handlers;

static bool add_handler(const char *name, ApiHandler handler) {
    api_handlers[name] = handler;
    return true;
}

#define HANDLER(handler_name) \
    static void _##handler_name(const ApiRequest &, ApiResult &); \
    static bool _dummy_##handler_name = add_handler(#handler_name, _##handler_name); \
    static void _##handler_name(const ApiRequest &request, ApiResult &result)

#pragma region Send Message

HANDLER(send_private_msg) {
    auto user_id = request.get_int_param("user_id", 0);
    auto message = request.get_msg_param();
    if (user_id && message) {
        result.retcode = CQ->send_private_msg(user_id, message);
    }
}

HANDLER(send_group_msg) {
    auto group_id = request.get_int_param("group_id", 0);
    auto message = request.get_msg_param();
    if (group_id && message) {
        result.retcode = CQ->send_group_msg(group_id, message);
    }
}

HANDLER(send_discuss_msg) {
    auto discuss_id = request.get_int_param("discuss_id", 0);
    auto message = request.get_msg_param();
    if (discuss_id && message) {
        result.retcode = CQ->send_discuss_msg(discuss_id, message);
    }
}

#pragma endregion

#pragma region Send Like

HANDLER(send_like) { // CoolQ Pro only
    auto user_id = request.get_int_param("user_id", 0);
    auto times = static_cast<int32_t>(request.get_int_param("times", 1));
    if (user_id && times > 0) {
        if (times == 1) {
            result.retcode = CQ->send_like(user_id);
        } else {
            result.retcode = CQ->send_like(user_id, times);
        }
    }
}

#pragma endregion

#pragma region Group & Discuss Operation

HANDLER(set_group_kick) {
    auto group_id = request.get_int_param("group_id", 0);
    auto user_id = request.get_int_param("user_id", 0);
    auto reject_add_request = request.get_bool_param("reject_add_request", false);
    if (group_id && user_id) {
        result.retcode = CQ->set_group_kick(group_id, user_id, reject_add_request);
    }
}

HANDLER(set_group_ban) {
    auto group_id = request.get_int_param("group_id", 0);
    auto user_id = request.get_int_param("user_id", 0);
    auto duration = request.get_int_param("duration", 30 * 60 /* 30 minutes */);
    if (group_id && user_id && duration >= 0) {
        result.retcode = CQ->set_group_ban(group_id, user_id, duration);
    }
}

HANDLER(set_group_anonymous_ban) {
    auto group_id = request.get_int_param("group_id", 0);
    auto anonymous_flag = request.get_str_param("flag", "");
    auto duration = request.get_int_param("duration", 30 * 60 /* 30 minutes */);
    if (group_id && anonymous_flag && duration >= 0) {
        result.retcode = CQ->set_group_anonymous_ban(group_id, anonymous_flag, duration);
    }
}

HANDLER(set_group_whole_ban) {
    auto group_id = request.get_int_param("group_id", 0);
    auto enable = request.get_bool_param("enable", true);
    if (group_id) {
        result.retcode = CQ->set_group_whole_ban(group_id, enable);
    }
}

HANDLER(set_group_admin) {
    auto group_id = request.get_int_param("group_id", 0);
    auto user_id = request.get_int_param("user_id", 0);
    auto enable = request.get_bool_param("enable", true);
    if (group_id && user_id) {
        result.retcode = CQ->set_group_admin(group_id, user_id, enable);
    }
}

HANDLER(set_group_anonymous) { // CoolQ Pro only
    auto group_id = request.get_int_param("group_id", 0);
    auto enable = request.get_bool_param("enable", true);
    if (group_id) {
        result.retcode = CQ->set_group_anonymous(group_id, enable);
    }
}

HANDLER(set_group_card) {
    auto group_id = request.get_int_param("group_id", 0);
    auto user_id = request.get_int_param("user_id", 0);
    auto card = request.get_str_param("card", "");
    if (group_id && user_id) {
        result.retcode = CQ->set_group_card(group_id, user_id, card);
    }
}

HANDLER(set_group_leave) {
    auto group_id = request.get_int_param("group_id", 0);
    auto is_dismiss = request.get_bool_param("is_dismiss", false);
    if (group_id) {
        result.retcode = CQ->set_group_leave(group_id, is_dismiss);
    }
}

HANDLER(set_group_special_title) {
    auto group_id = request.get_int_param("group_id", 0);
    auto user_id = request.get_int_param("user_id", 0);
    auto special_title = request.get_str_param("special_title", "");
    auto duration = request.get_int_param("duration", -1 /* permanent */); // seems to have no effect
    if (group_id && user_id) {
        result.retcode = CQ->set_group_special_title(group_id, user_id, special_title, duration);
    }
}

HANDLER(set_discuss_leave) {
    auto discuss_id = request.get_int_param("discuss_id", 0);
    if (discuss_id) {
        result.retcode = CQ->set_discuss_leave(discuss_id);
    }
}

#pragma endregion

#pragma region Request Operation

HANDLER(set_friend_add_request) {
    auto flag = request.get_str_param("flag", "");
    auto approve = request.get_bool_param("approve", true);
    auto remark = request.get_str_param("remark", "");
    if (flag) {
        result.retcode = CQ->set_friend_add_request(flag, approve ? REQUEST_ALLOW : REQUEST_DENY, remark);
    }
}

HANDLER(set_group_add_request) {
    auto flag = request.get_str_param("flag", "");
    auto type = request.get_str_param("type", "");
    auto approve = request.get_bool_param("approve", true);
    auto reason = request.get_str_param("reason", "");
    auto request_type = -1;
    if (type == "add") {
        request_type = REQUEST_GROUPADD;
    } else if (type == "invite") {
        request_type = REQUEST_GROUPINVITE;
    }
    if (flag && request_type != -1) {
        result.retcode = CQ->set_group_add_request(flag, request_type, approve ? REQUEST_ALLOW : REQUEST_DENY, reason);
    }
}

#pragma endregion

#pragma region Get QQ Information

HANDLER(get_login_info) {
    auto id = CQ->get_login_qq();
    auto nickname = CQ->get_login_nick();
    result.retcode = nickname ? ApiRetCode::OK : ApiRetCode::INVALID_DATA;
    result.data = json_pack("{s:I,s:s?}", "user_id", id, "nickname", nickname.c_str());
}

HANDLER(get_stranger_info) {
    auto user_id = request.get_int_param("user_id", 0);
    auto no_cache = request.get_bool_param("no_cache", false);
    if (user_id) {
        auto bytes = CQ->get_stranger_info_raw(user_id, no_cache);
        if (bytes.size() >= Stranger::MIN_SIZE) {
            auto stranger = Stranger::from_bytes(bytes);
            result.data = stranger.json();
            result.retcode = ApiRetCode::OK;
        } else {
            result.retcode = ApiRetCode::INVALID_DATA;
        }
    }
}

HANDLER(get_group_list) {
    auto bytes = CQ->get_group_list_raw();
    if (bytes.size() >= 4 /* at least has a count */) {
        auto pack = Pack(bytes);

        auto group_list = json_array();

        auto count = pack.pop_int32(); // get number of groups
        for (auto i = 0; i < count; i++) {
            auto token = pack.pop_token();
            auto group = Group::from_bytes(token);
            json_array_append_new(group_list, group.json());
        }

        result.data = group_list;
        result.retcode = ApiRetCode::OK;
    } else {
        result.retcode = ApiRetCode::INVALID_DATA;
    }
}

HANDLER(get_group_member_list) {
    auto group_id = request.get_int_param("group_id", 0);
    if (group_id) {
        auto bytes = CQ->get_group_member_list_raw(group_id);
        if (bytes.size() >= 4 /* at least has a count */) {
            auto pack = Pack(bytes);

            auto member_list = json_array();

            auto count = pack.pop_int32();
            for (auto i = 0; i < count; i++) {
                auto token = pack.pop_token();
                auto member = GroupMember::from_bytes(token);
                json_array_append_new(member_list, member.json());
            }

            result.data = member_list;
            result.retcode = ApiRetCode::OK;
        } else {
            result.retcode = ApiRetCode::INVALID_DATA;
        }
    }
}

HANDLER(get_group_member_info) {
    auto group_id = request.get_int_param("group_id", 0);
    auto user_id = request.get_int_param("user_id", 0);
    auto no_cache = request.get_bool_param("no_cache", false);
    if (group_id && user_id) {
        auto bytes = CQ->get_group_member_info_raw(group_id, user_id, no_cache);
        if (bytes.size() >= GroupMember::MIN_SIZE) {
            auto member = GroupMember::from_bytes(bytes);
            result.data = member.json();
            result.retcode = ApiRetCode::OK;
        } else {
            result.retcode = ApiRetCode::INVALID_DATA;
        }
    }
}

#pragma region Get CoolQ Information

HANDLER(get_cookies) {
    auto cookies = CQ->get_cookies();
    result.retcode = cookies ? ApiRetCode::OK : ApiRetCode::INVALID_DATA;
    result.data = json_pack("{s:s?}", "cookies", cookies.c_str());
}

HANDLER(get_csrf_token) {
    auto token = CQ->get_csrf_token();
    result.retcode = token ? ApiRetCode::OK : ApiRetCode::INVALID_DATA;
    result.data = json_pack("{s:i}", "token", token);
}

HANDLER(get_version_info) {
    auto root_dir = get_coolq_root();
    auto coolq_edition = "air";
    if (isfile(root_dir + "CQP.exe")) {
        coolq_edition = "pro";
    }
    auto plugin_version = CQ_APP_VERSION;
    result.retcode = ApiRetCode::OK;
    result.data = json_pack("{s:s,s:s}",
                            "coolq_edition", coolq_edition,
                            "plugin_version", plugin_version);
}

#pragma endregion
