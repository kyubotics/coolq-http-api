#include "request_handlers.h"

#include <jansson/jansson.h>
#include <regex>

#include "request.h"
#include "encoding/encoding.h"
#include "encoding/base64.h"
#include "cqcode.h"

using namespace std;

extern int ac; // global AuthCode in appmain.cpp

CQHTTP_REQUEST_HANDLER(send_private_msg)
(const struct cqhttp_request &request) {
    struct cqhttp_result result;
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
    return result;
}

CQHTTP_REQUEST_HANDLER(send_group_msg)
(const struct cqhttp_request &request) {
    struct cqhttp_result result;
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
    return result;
}

CQHTTP_REQUEST_HANDLER(send_discuss_msg)
(const struct cqhttp_request &request) {
    struct cqhttp_result result;
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
    return result;
}

CQHTTP_REQUEST_HANDLER(send_like) // CoolQ Pro only
(const struct cqhttp_request &request) {
    struct cqhttp_result result;
    auto user_id = cqhttp_get_integer_param(request, "user_id", 0);
    auto times = static_cast<int32_t>(cqhttp_get_integer_param(request, "times", 1));
    if (user_id && times > 0) {
        if (times == 1) {
            result.retcode = CQ->sendLike(user_id);
        } else {
            result.retcode = CQ->sendLikeV2(user_id, times);
        }
    }
    return result;
}

CQHTTP_REQUEST_HANDLER(set_group_kick)
(const struct cqhttp_request &request) {
    struct cqhttp_result result;
    auto group_id = cqhttp_get_integer_param(request, "group_id", 0);
    auto user_id = cqhttp_get_integer_param(request, "user_id", 0);
    auto reject_add_request = cqhttp_get_bool_param(request, "reject_add_request", false);
    if (group_id && user_id) {
        result.retcode = CQ->setGroupKick(group_id, user_id, reject_add_request);
    }
    return result;
}

CQHTTP_REQUEST_HANDLER(set_group_ban)
(const struct cqhttp_request &request) {
    struct cqhttp_result result;
    auto group_id = cqhttp_get_integer_param(request, "group_id", 0);
    auto user_id = cqhttp_get_integer_param(request, "user_id", 0);
    auto duration = cqhttp_get_integer_param(request, "duration", 30 * 60 /* 30 minutes */);
    if (group_id && user_id && duration >= 0) {
        result.retcode = CQ->setGroupBan(group_id, user_id, duration);
    }
    return result;
}

CQHTTP_REQUEST_HANDLER(set_group_anonymous_ban)
(const struct cqhttp_request &request) {
    struct cqhttp_result result;
    auto group_id = cqhttp_get_integer_param(request, "group_id", 0);
    auto anonymous_flag = cqhttp_get_str_param(request, "flag", "");
    auto duration = cqhttp_get_integer_param(request, "duration", 30 * 60 /* 30 minutes */);
    if (group_id && anonymous_flag && duration >= 0) {
        result.retcode = CQ->setGroupAnonymousBan(group_id, anonymous_flag, duration);
    }
    return result;
}

CQHTTP_REQUEST_HANDLER(set_group_whole_ban)
(const struct cqhttp_request &request) {
    struct cqhttp_result result;
    auto group_id = cqhttp_get_integer_param(request, "group_id", 0);
    auto enable = cqhttp_get_bool_param(request, "enable", true);
    if (group_id) {
        result.retcode = CQ->setGroupWholeBan(group_id, enable);
    }
    return result;
}

CQHTTP_REQUEST_HANDLER(set_group_admin)
(const struct cqhttp_request &request) {
    struct cqhttp_result result;
    auto group_id = cqhttp_get_integer_param(request, "group_id", 0);
    auto user_id = cqhttp_get_integer_param(request, "user_id", 0);
    auto enable = cqhttp_get_bool_param(request, "enable", true);
    if (group_id && user_id) {
        result.retcode = CQ->setGroupAdmin(group_id, user_id, enable);
    }
    return result;
}

CQHTTP_REQUEST_HANDLER(set_group_anonymous) // CoolQ Pro only
(const struct cqhttp_request &request) {
    struct cqhttp_result result;
    auto group_id = cqhttp_get_integer_param(request, "group_id", 0);
    auto enable = cqhttp_get_bool_param(request, "enable", true);
    if (group_id) {
        result.retcode = CQ->setGroupAnonymous(group_id, enable);
    }
    return result;
}

CQHTTP_REQUEST_HANDLER(set_group_card)
(const struct cqhttp_request &request) {
    struct cqhttp_result result;
    auto group_id = cqhttp_get_integer_param(request, "group_id", 0);
    auto user_id = cqhttp_get_integer_param(request, "user_id", 0);
    auto card = cqhttp_get_str_param(request, "card", "");
    if (group_id && user_id) {
        result.retcode = CQ->setGroupCard(group_id, user_id, card);
    }
    return result;
}

CQHTTP_REQUEST_HANDLER(set_group_leave)
(const struct cqhttp_request &request) {
    struct cqhttp_result result;
    auto group_id = cqhttp_get_integer_param(request, "group_id", 0);
    auto is_dismiss = cqhttp_get_bool_param(request, "is_dismiss", false);
    if (group_id) {
        result.retcode = CQ->setGroupLeave(group_id, is_dismiss);
    }
    return result;
}

CQHTTP_REQUEST_HANDLER(set_group_special_title)
(const struct cqhttp_request &request) {
    struct cqhttp_result result;
    auto group_id = cqhttp_get_integer_param(request, "group_id", 0);
    auto user_id = cqhttp_get_integer_param(request, "user_id", 0);
    auto special_title = cqhttp_get_str_param(request, "special_title", "");
    auto duration = cqhttp_get_integer_param(request, "duration", -1 /* permanent */); // seems to have no effect
    if (group_id && user_id) {
        result.retcode = CQ->setGroupSpecialTitle(group_id, user_id, special_title, duration);
    }
    return result;
}

CQHTTP_REQUEST_HANDLER(set_discuss_leave)
(const struct cqhttp_request &request) {
    struct cqhttp_result result;
    auto discuss_id = cqhttp_get_integer_param(request, "discuss_id", 0);
    if (discuss_id) {
        result.retcode = CQ_setDiscussLeave(ac, discuss_id);
    }
    return result;
}

CQHTTP_REQUEST_HANDLER(set_friend_add_request)
(const struct cqhttp_request &request) {
    struct cqhttp_result result;
    auto flag = cqhttp_get_str_param(request, "flag", "");
    auto approve = cqhttp_get_bool_param(request, "approve", true);
    auto remark = cqhttp_get_str_param(request, "remark", "");
    if (flag) {
        result.retcode = CQ->setFriendAddRequest(flag, approve ? REQUEST_ALLOW : REQUEST_DENY, remark);
    }
    return result;
}

CQHTTP_REQUEST_HANDLER(set_group_add_request)
(const struct cqhttp_request &request) {
    struct cqhttp_result result;
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
    return result;
}

CQHTTP_REQUEST_HANDLER(get_login_info)
(const struct cqhttp_request &request) {
    struct cqhttp_result result;
    auto id = CQ->getLoginQQ();
    auto nickname = CQ->getLoginNick();
    result.retcode = nickname ? CQHTTP_RETCODE_OK : CQHTTP_RETCODE_INVALID_DATA;
    result.data = json_pack("{s:I,s:s?}", "user_id", id, "nickname", encode(nickname).c_str());
    return result;
}

void get_integer_from_decoded_bytes(const string &bytes, size_t start, size_t size, void *dst) {
    string sub = bytes.substr(start, size);
    reverse(sub.begin(), sub.end());
    memcpy(dst, sub.data(), size);
}

void get_string_from_decoded_bytes(const string &bytes, size_t start, size_t *size, string *dst) {
    int16_t str_length = 0;
    get_integer_from_decoded_bytes(bytes, start, 2, &str_length);
    if (str_length == 0)
        *dst = "";
    else
        *dst = gbk_to_utf8(bytes.substr(start + 2, str_length).c_str());
    *size = 2 + str_length;
}

#define INIT(bytes_str) \
    size_t _start = 0, _size; \
    string &_bytes = bytes_str;
#define INTEGER(field) \
    _size = sizeof(field); \
    get_integer_from_decoded_bytes(_bytes, _start, _size, &field); \
    _start += _size;
#define STRING(field) \
    get_string_from_decoded_bytes(_bytes, _start, &_size, &field); \
    _start += _size;

struct raw_stranger_info {
    int64_t user_id;
    string nickname; // utf8
    int32_t sex;
    int32_t age;

    json_t *json() const {
        json_t *data = json_object();
        json_object_set_new(data, "user_id", json_integer(user_id));
        json_object_set_new(data, "nickname", json_string(nickname.c_str()));
        json_object_set_new(data, "sex", json_string(sex == 0 ? "male" : (sex == 1 ? "female" : "unknown")));
        json_object_set_new(data, "age", json_integer(age));
        return data;
    }
};

CQHTTP_REQUEST_HANDLER(get_stranger_info)
(const struct cqhttp_request &request) {
    struct cqhttp_result result;
    int64_t user_id = cqhttp_get_integer_param(request, "user_id", 0);
    bool no_cache = cqhttp_get_bool_param(request, "no_cache", false);
    if (user_id) {
        string bytes = base64_decode(gbk_to_utf8(CQ_getStrangerInfo(ac, user_id, no_cache)));
        if (bytes.size() >= 18 /* minimum valid bytes size */) {
            struct raw_stranger_info stranger_info;
            INIT(bytes);
            INTEGER(stranger_info.user_id);
            STRING(stranger_info.nickname);
            INTEGER(stranger_info.sex);
            INTEGER(stranger_info.age);
            result.data = stranger_info.json();
            result.retcode = CQHTTP_RETCODE_OK;
        } else {
            result.retcode = CQHTTP_RETCODE_INVALID_DATA;
        }
    }
    return result;
}

struct raw_group_member_info {
    int64_t group_id;
    int64_t user_id;
    string nickname; // all strings are converted to utf8
    string card;
    int32_t sex;
    int32_t age;
    string area;
    int32_t join_time;
    int32_t last_sent_time;
    string level;
    int32_t role;
    int32_t unfriendly; // unsure
    string title;
    int32_t title_expire_time;
    int32_t card_changeable;

    json_t *json() const {
        json_t *data = json_object();
        json_object_set_new(data, "group_id", json_integer(group_id));
        json_object_set_new(data, "user_id", json_integer(user_id));
        json_object_set_new(data, "nickname", json_string(nickname.c_str()));
        json_object_set_new(data, "card", json_string(card.c_str()));
        json_object_set_new(data, "sex", json_string(sex == 0 ? "male" : (sex == 1 ? "female" : "unknown")));
        json_object_set_new(data, "age", json_integer(age));
        json_object_set_new(data, "area", json_string(area.c_str()));
        json_object_set_new(data, "join_time", json_integer(join_time));
        json_object_set_new(data, "last_sent_time", json_integer(last_sent_time));
        json_object_set_new(data, "level", json_string(level.c_str()));
        json_object_set_new(data, "role", json_string(role == 3 ? "owner" : (role == 2 ? "admin" : (role == 1 ? "member" : "unknown"))));
        json_object_set_new(data, "unfriendly", json_boolean(unfriendly));
        json_object_set_new(data, "title", json_string(title.c_str()));
        json_object_set_new(data, "title_expire_time", json_integer(title_expire_time));
        json_object_set_new(data, "card_changeable", json_boolean(card_changeable));
        return data;
    }
};

/**
* See https://cqp.cc/t/26287 for how to parse the encoded data.
*/
CQHTTP_REQUEST_HANDLER(get_group_member_info)
(const struct cqhttp_request &request) {
    struct cqhttp_result result;
    int64_t group_id = cqhttp_get_integer_param(request, "group_id", 0);
    int64_t user_id = cqhttp_get_integer_param(request, "user_id", 0);
    bool no_cache = cqhttp_get_bool_param(request, "no_cache", false);
    if (group_id && user_id) {
        string bytes = base64_decode(gbk_to_utf8(CQ_getGroupMemberInfoV2(ac, group_id, user_id, no_cache)));
        if (bytes.size() >= 58 /* minimum valid bytes size */) {
            struct raw_group_member_info member_info;
            INIT(bytes);
            INTEGER(member_info.group_id);
            INTEGER(member_info.user_id);
            STRING(member_info.nickname);
            STRING(member_info.card);
            INTEGER(member_info.sex);
            INTEGER(member_info.age);
            STRING(member_info.area);
            INTEGER(member_info.join_time);
            INTEGER(member_info.last_sent_time);
            STRING(member_info.level);
            INTEGER(member_info.role);
            INTEGER(member_info.unfriendly);
            STRING(member_info.title);
            INTEGER(member_info.title_expire_time);
            INTEGER(member_info.card_changeable);
            result.data = member_info.json();
            result.retcode = CQHTTP_RETCODE_OK;
        } else {
            result.retcode = CQHTTP_RETCODE_INVALID_DATA;
        }
    }
    return result;
}

CQHTTP_REQUEST_HANDLER(get_group_member_list)
(const struct cqhttp_request &request) {
    struct cqhttp_result result;
    int64_t group_id = cqhttp_get_integer_param(request, "group_id", 0);
    if (group_id) {
        string bytes = base64_decode(gbk_to_utf8(CQ_getGroupMemberList(ac, group_id)));
        if (bytes.size() >= 10 /* minimum valid bytes size */) {
            INIT(bytes);
            int32_t count;
            INTEGER(count); // get number of group members

            auto member_list = json_array();

            for (auto i = 0; i < count; i++) {
                int16_t token_len;
                INTEGER(token_len); // no use

                struct raw_group_member_info member_info;
                INTEGER(member_info.group_id);
                INTEGER(member_info.user_id);
                STRING(member_info.nickname);
                STRING(member_info.card);
                INTEGER(member_info.sex);
                INTEGER(member_info.age);
                STRING(member_info.area);
                INTEGER(member_info.join_time);
                INTEGER(member_info.last_sent_time);
                STRING(member_info.level);
                INTEGER(member_info.role);
                INTEGER(member_info.unfriendly);
                STRING(member_info.title);
                INTEGER(member_info.title_expire_time);
                INTEGER(member_info.card_changeable);
                json_array_append_new(member_list, member_info.json());
            }

            result.data = member_list;
            result.retcode = CQHTTP_RETCODE_OK;
        } else {
            result.retcode = CQHTTP_RETCODE_INVALID_DATA;
        }
    }
    return result;
}

struct raw_group_info {
    int64_t group_id;
    string group_name;

    json_t *json() const {
        json_t *data = json_object();
        json_object_set_new(data, "group_id", json_integer(group_id));
        json_object_set_new(data, "group_name", json_string(group_name.c_str()));
        return data;
    }
};

CQHTTP_REQUEST_HANDLER(get_group_list)
(const struct cqhttp_request &request) {
    struct cqhttp_result result;
    string bytes = base64_decode(gbk_to_utf8(CQ_getGroupList(ac)));
    if (bytes.size() >= 10 /* minimum valid bytes size */) {
        INIT(bytes);
        int32_t count;
        INTEGER(count); // get number of groups

        auto group_list = json_array();

        for (auto i = 0; i < count; i++) {
            int16_t token_len;
            INTEGER(token_len); // no use

            struct raw_group_info group_info;
            INTEGER(group_info.group_id);
            STRING(group_info.group_name);
            json_array_append_new(group_list, group_info.json());
        }

        result.data = group_list;
        result.retcode = CQHTTP_RETCODE_OK;
    } else {
        result.retcode = CQHTTP_RETCODE_INVALID_DATA;
    }
    return result;
}

#undef INIT
#undef INTEGER
#undef STRING

CQHTTP_REQUEST_HANDLER(get_cookies)
(const struct cqhttp_request &request) {
    struct cqhttp_result result;
    const char *cookies = CQ_getCookies(ac);
    result.retcode = cookies ? CQHTTP_RETCODE_OK : CQHTTP_RETCODE_INVALID_DATA;
    result.data = json_pack("{s:s?}", "cookies", gbk_to_utf8(cookies).c_str());
    return result;
}

CQHTTP_REQUEST_HANDLER(get_csrf_token)
(const struct cqhttp_request &request) {
    struct cqhttp_result result;
    int32_t token = CQ_getCsrfToken(ac);
    result.retcode = token ? CQHTTP_RETCODE_OK : CQHTTP_RETCODE_INVALID_DATA;
    result.data = json_pack("{s:I}", "token", (int64_t) token);
    return result;
}
