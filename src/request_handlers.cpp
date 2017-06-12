#include "request_handlers.h"

#include <jansson/jansson.h>

#include "request.h"
#include "cqcode.h"
#include "Structs.h"
#include "Pack.h"

using namespace std;

CQHTTP_REQUEST_HANDLER(send_private_msg) {
    auto user_id = cqhttp_get_integer_param(request, "user_id", 0);
    auto msg = cqhttp_get_str_param(request, "message", "");
    auto is_raw = cqhttp_get_bool_param(request, "is_raw", false);
    if (user_id && msg) {
        string final_str;
        if (is_raw) {
            msg = message_escape(msg);
        } else {
            msg = enhance_cq_code(msg, CQCODE_ENHANCE_OUTCOMING);
        }
        result.retcode = CQ->sendPrivateMsg(user_id, msg);
    }
}

CQHTTP_REQUEST_HANDLER(send_group_msg) {
    auto group_id = cqhttp_get_integer_param(request, "group_id", 0);
    auto msg = cqhttp_get_str_param(request, "message", "");
    auto is_raw = cqhttp_get_bool_param(request, "is_raw", false);
    if (group_id && msg) {
        if (is_raw) {
            msg = message_escape(msg);
        } else {
            msg = enhance_cq_code(msg, CQCODE_ENHANCE_OUTCOMING);
        }
        result.retcode = CQ->sendGroupMsg(group_id, msg);
    }
}

CQHTTP_REQUEST_HANDLER(send_discuss_msg) {
    auto discuss_id = cqhttp_get_integer_param(request, "discuss_id", 0);
    auto msg = cqhttp_get_str_param(request, "message", "");
    auto is_raw = cqhttp_get_bool_param(request, "is_raw", false);
    if (discuss_id && msg) {
        if (is_raw) {
            msg = message_escape(msg);
        } else {
            msg = enhance_cq_code(msg, CQCODE_ENHANCE_OUTCOMING);
        }
        result.retcode = CQ->sendDiscussMsg(discuss_id, msg);
    }
}

CQHTTP_REQUEST_HANDLER(send_like) { // CoolQ Pro only
    auto user_id = cqhttp_get_integer_param(request, "user_id", 0);
    auto times = static_cast<int32_t>(cqhttp_get_integer_param(request, "times", 1));
    if (user_id && times > 0) {
        if (times == 1) {
            result.retcode = CQ->sendLike(user_id);
        } else {
            result.retcode = CQ->sendLikeV2(user_id, times);
        }
    }
}

CQHTTP_REQUEST_HANDLER(set_group_kick) {
    auto group_id = cqhttp_get_integer_param(request, "group_id", 0);
    auto user_id = cqhttp_get_integer_param(request, "user_id", 0);
    auto reject_add_request = cqhttp_get_bool_param(request, "reject_add_request", false);
    if (group_id && user_id) {
        result.retcode = CQ->setGroupKick(group_id, user_id, reject_add_request);
    }
}

CQHTTP_REQUEST_HANDLER(set_group_ban) {
    auto group_id = cqhttp_get_integer_param(request, "group_id", 0);
    auto user_id = cqhttp_get_integer_param(request, "user_id", 0);
    auto duration = cqhttp_get_integer_param(request, "duration", 30 * 60 /* 30 minutes */);
    if (group_id && user_id && duration >= 0) {
        result.retcode = CQ->setGroupBan(group_id, user_id, duration);
    }
}

CQHTTP_REQUEST_HANDLER(set_group_anonymous_ban) {
    auto group_id = cqhttp_get_integer_param(request, "group_id", 0);
    auto anonymous_flag = cqhttp_get_str_param(request, "flag", "");
    auto duration = cqhttp_get_integer_param(request, "duration", 30 * 60 /* 30 minutes */);
    if (group_id && anonymous_flag && duration >= 0) {
        result.retcode = CQ->setGroupAnonymousBan(group_id, anonymous_flag, duration);
    }
}

CQHTTP_REQUEST_HANDLER(set_group_whole_ban) {
    auto group_id = cqhttp_get_integer_param(request, "group_id", 0);
    auto enable = cqhttp_get_bool_param(request, "enable", true);
    if (group_id) {
        result.retcode = CQ->setGroupWholeBan(group_id, enable);
    }
}

CQHTTP_REQUEST_HANDLER(set_group_admin) {
    auto group_id = cqhttp_get_integer_param(request, "group_id", 0);
    auto user_id = cqhttp_get_integer_param(request, "user_id", 0);
    auto enable = cqhttp_get_bool_param(request, "enable", true);
    if (group_id && user_id) {
        result.retcode = CQ->setGroupAdmin(group_id, user_id, enable);
    }
}

CQHTTP_REQUEST_HANDLER(set_group_anonymous) { // CoolQ Pro only
    auto group_id = cqhttp_get_integer_param(request, "group_id", 0);
    auto enable = cqhttp_get_bool_param(request, "enable", true);
    if (group_id) {
        result.retcode = CQ->setGroupAnonymous(group_id, enable);
    }
}

CQHTTP_REQUEST_HANDLER(set_group_card) {
    auto group_id = cqhttp_get_integer_param(request, "group_id", 0);
    auto user_id = cqhttp_get_integer_param(request, "user_id", 0);
    auto card = cqhttp_get_str_param(request, "card", "");
    if (group_id && user_id) {
        result.retcode = CQ->setGroupCard(group_id, user_id, card);
    }
}

CQHTTP_REQUEST_HANDLER(set_group_leave) {
    auto group_id = cqhttp_get_integer_param(request, "group_id", 0);
    auto is_dismiss = cqhttp_get_bool_param(request, "is_dismiss", false);
    if (group_id) {
        result.retcode = CQ->setGroupLeave(group_id, is_dismiss);
    }
}

CQHTTP_REQUEST_HANDLER(set_group_special_title) {
    auto group_id = cqhttp_get_integer_param(request, "group_id", 0);
    auto user_id = cqhttp_get_integer_param(request, "user_id", 0);
    auto special_title = cqhttp_get_str_param(request, "special_title", "");
    auto duration = cqhttp_get_integer_param(request, "duration", -1 /* permanent */); // seems to have no effect
    if (group_id && user_id) {
        result.retcode = CQ->setGroupSpecialTitle(group_id, user_id, special_title, duration);
    }
}

CQHTTP_REQUEST_HANDLER(set_discuss_leave) {
    auto discuss_id = cqhttp_get_integer_param(request, "discuss_id", 0);
    if (discuss_id) {
        result.retcode = CQ_setDiscussLeave(ac, discuss_id);
    }
}

CQHTTP_REQUEST_HANDLER(set_friend_add_request) {
    auto flag = cqhttp_get_str_param(request, "flag", "");
    auto approve = cqhttp_get_bool_param(request, "approve", true);
    auto remark = cqhttp_get_str_param(request, "remark", "");
    if (flag) {
        result.retcode = CQ->setFriendAddRequest(flag, approve ? REQUEST_ALLOW : REQUEST_DENY, remark);
    }
}

CQHTTP_REQUEST_HANDLER(set_group_add_request) {
    auto flag = cqhttp_get_str_param(request, "flag", "");
    auto type = cqhttp_get_str_param(request, "type", "");
    auto approve = cqhttp_get_bool_param(request, "approve", true);
    auto reason = cqhttp_get_str_param(request, "reason", "");
    if (!reason) {
        reason = cqhttp_get_str_param(request, "remark", ""); // for compatibility with v1.1.3 and before
    }
    auto request_type = -1;
    if (type == "add") {
        request_type = REQUEST_GROUPADD;
    } else if (type == "invite") {
        request_type = REQUEST_GROUPINVITE;
    }
    if (flag && request_type != -1) {
        result.retcode = CQ->setGroupAddRequestV2(flag, request_type, approve ? REQUEST_ALLOW : REQUEST_DENY, reason);
    }
}

CQHTTP_REQUEST_HANDLER(get_login_info) {
    auto id = CQ->getLoginQQ();
    auto nickname = CQ->getLoginNick();
    result.retcode = nickname ? CQHTTP_RETCODE_OK : CQHTTP_RETCODE_INVALID_DATA;
    result.data = json_pack("{s:I,s:s?}", "user_id", id, "nickname", nickname.c_str());
}

CQHTTP_REQUEST_HANDLER(get_stranger_info) {
    auto user_id = cqhttp_get_integer_param(request, "user_id", 0);
    auto no_cache = cqhttp_get_bool_param(request, "no_cache", false);
    if (user_id) {
        auto bytes = CQ->getStrangerInfoRaw(user_id, no_cache);
        if (bytes.size() >= Stranger::MIN_SIZE) {
            auto stranger = Stranger::from_bytes(bytes);
            result.data = stranger.json();
            result.retcode = CQHTTP_RETCODE_OK;
        } else {
            result.retcode = CQHTTP_RETCODE_INVALID_DATA;
        }
    }
}

CQHTTP_REQUEST_HANDLER(get_group_list) {
    auto bytes = CQ->getGroupListRaw();
    if (bytes.size() >= 4 /* at least has a count */) {
        auto pack = Pack(bytes);

        auto group_list = json_array();

        auto count = pack.pop_int32(); // get number of groups
        for (auto i = 0; i < count; i++) {
            auto token = pack.pop_bytes(pack.pop_int16());
            auto group = Group::from_bytes(token);
            json_array_append_new(group_list, group.json());
        }

        result.data = group_list;
        result.retcode = CQHTTP_RETCODE_OK;
    } else {
        result.retcode = CQHTTP_RETCODE_INVALID_DATA;
    }
}

CQHTTP_REQUEST_HANDLER(get_group_member_list) {
    auto group_id = cqhttp_get_integer_param(request, "group_id", 0);
    if (group_id) {
        auto bytes = CQ->getGroupMemberListRaw(group_id);
        if (bytes.size() >= 4 /* at least has a count */) {
            auto pack = Pack(bytes);

            auto member_list = json_array();

            auto count = pack.pop_int32();
            for (auto i = 0; i < count; i++) {
                auto token = pack.pop_bytes(pack.pop_int16());
                auto member = GroupMember::from_bytes(token);
                json_array_append_new(member_list, member.json());
            }

            result.data = member_list;
            result.retcode = CQHTTP_RETCODE_OK;
        } else {
            result.retcode = CQHTTP_RETCODE_INVALID_DATA;
        }
    }
}

CQHTTP_REQUEST_HANDLER(get_group_member_info) {
    auto group_id = cqhttp_get_integer_param(request, "group_id", 0);
    auto user_id = cqhttp_get_integer_param(request, "user_id", 0);
    auto no_cache = cqhttp_get_bool_param(request, "no_cache", false);
    if (group_id && user_id) {
        auto bytes = CQ->getGroupMemberInfoRawV2(group_id, user_id, no_cache);
        if (bytes.size() >= GroupMember::MIN_SIZE) {
            auto member = GroupMember::from_bytes(bytes);
            result.data = member.json();
            result.retcode = CQHTTP_RETCODE_OK;
        } else {
            result.retcode = CQHTTP_RETCODE_INVALID_DATA;
        }
    }
}

CQHTTP_REQUEST_HANDLER(get_cookies) {
    auto cookies = CQ->getCookies();
    result.retcode = cookies ? CQHTTP_RETCODE_OK : CQHTTP_RETCODE_INVALID_DATA;
    result.data = json_pack("{s:s?}", "cookies", cookies.c_str());
}

CQHTTP_REQUEST_HANDLER(get_csrf_token) {
    auto token = CQ->getCsrfToken();
    result.retcode = token ? CQHTTP_RETCODE_OK : CQHTTP_RETCODE_INVALID_DATA;
    result.data = json_pack("{s:i}", "token", token);
}
