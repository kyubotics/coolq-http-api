#include "request_handlers.h"

#include <jansson.h>

#include "request.h"
#include "encoding.h"

using namespace std;

extern int ac; // global AuthCode in appmain.cpp

CQHTTP_REQUEST_HANDLER(send_private_msg)
(const struct cqhttp_request &request)
{
    struct cqhttp_result result;
    int64_t user_id = cqhttp_get_integer_param(request, "user_id", 0);
    char *msg = cqhttp_get_param(request, "msg");
    if (user_id && msg)
        CQ_sendPrivateMsg(ac, user_id, utf8_to_gbk(msg).c_str());
    else
        result.status = CQHTTP_STATUS_FAILED;
    if (msg)
        free(msg);
    return result;
}

CQHTTP_REQUEST_HANDLER(send_group_msg)
(const struct cqhttp_request &request)
{
    struct cqhttp_result result;
    int64_t group_id = cqhttp_get_integer_param(request, "group_id", 0);
    char *msg = cqhttp_get_param(request, "msg");
    if (group_id && msg)
        CQ_sendGroupMsg(ac, group_id, utf8_to_gbk(msg).c_str());
    else
        result.status = CQHTTP_STATUS_FAILED;
    if (msg)
        free(msg);
    return result;
}

CQHTTP_REQUEST_HANDLER(send_discuss_msg)
(const struct cqhttp_request &request)
{
    struct cqhttp_result result;
    int64_t discuss_id = cqhttp_get_integer_param(request, "discuss_id", 0);
    char *msg = cqhttp_get_param(request, "msg");
    if (discuss_id && msg)
        CQ_sendDiscussMsg(ac, discuss_id, utf8_to_gbk(msg).c_str());
    else
        result.status = CQHTTP_STATUS_FAILED;
    if (msg)
        free(msg);
    return result;
}

CQHTTP_REQUEST_HANDLER(send_like) // CoolQ Pro only
(const struct cqhttp_request &request)
{
    struct cqhttp_result result;
    int64_t user_id = cqhttp_get_integer_param(request, "user_id", 0);
    if (user_id)
        CQ_sendLike(ac, user_id);
    else
        result.status = CQHTTP_STATUS_FAILED;
    return result;
}

CQHTTP_REQUEST_HANDLER(set_group_kick)
(const struct cqhttp_request &request)
{
    struct cqhttp_result result;
    int64_t group_id = cqhttp_get_integer_param(request, "group_id", 0);
    int64_t user_id = cqhttp_get_integer_param(request, "user_id", 0);
    bool reject_add_request = cqhttp_get_bool_param(request, "reject_add_request", false);
    if (group_id && user_id)
        CQ_setGroupKick(ac, group_id, user_id, reject_add_request);
    else
        result.status = CQHTTP_STATUS_FAILED;
    return result;
}

CQHTTP_REQUEST_HANDLER(set_group_ban)
(const struct cqhttp_request &request)
{
    struct cqhttp_result result;
    int64_t group_id = cqhttp_get_integer_param(request, "group_id", 0);
    int64_t user_id = cqhttp_get_integer_param(request, "user_id", 0);
    int64_t duration = cqhttp_get_integer_param(request, "duration", 30 * 60 /* 30 minutes */);
    if (group_id && user_id && duration >= 0)
        CQ_setGroupBan(ac, group_id, user_id, duration);
    else
        result.status = CQHTTP_STATUS_FAILED;
    return result;
}

CQHTTP_REQUEST_HANDLER(set_group_anonymous_ban)
(const struct cqhttp_request &request)
{
    struct cqhttp_result result;
    int64_t group_id = cqhttp_get_integer_param(request, "group_id", 0);
    char *anonymous = cqhttp_get_param(request, "anonymous");
    int64_t duration = cqhttp_get_integer_param(request, "duration", 30 * 60 /* 30 minutes */);
    if (group_id && anonymous && duration >= 0)
        CQ_setGroupAnonymousBan(ac, group_id, utf8_to_gbk(anonymous).c_str(), duration);
    else
        result.status = CQHTTP_STATUS_FAILED;
    if (anonymous)
        free(anonymous);
    return result;
}

CQHTTP_REQUEST_HANDLER(set_group_whole_ban)
(const struct cqhttp_request &request)
{
    struct cqhttp_result result;
    int64_t group_id = cqhttp_get_integer_param(request, "group_id", 0);
    bool enabled = cqhttp_get_bool_param(request, "enabled", true);
    if (group_id)
        CQ_setGroupWholeBan(ac, group_id, enabled);
    else
        result.status = CQHTTP_STATUS_FAILED;
    return result;
}

CQHTTP_REQUEST_HANDLER(set_group_admin)
(const struct cqhttp_request &request)
{
    struct cqhttp_result result;
    int64_t group_id = cqhttp_get_integer_param(request, "group_id", 0);
    int64_t user_id = cqhttp_get_integer_param(request, "user_id", 0);
    bool enabled = cqhttp_get_bool_param(request, "enabled", true);
    if (group_id && user_id)
        CQ_setGroupAdmin(ac, group_id, user_id, enabled);
    else
        result.status = CQHTTP_STATUS_FAILED;
    return result;
}

CQHTTP_REQUEST_HANDLER(set_group_anonymous) // CoolQ Pro only
(const struct cqhttp_request &request)
{
    struct cqhttp_result result;
    int64_t group_id = cqhttp_get_integer_param(request, "group_id", 0);
    bool enabled = cqhttp_get_bool_param(request, "enabled", true);
    if (group_id)
        CQ_setGroupAnonymous(ac, group_id, enabled);
    else
        result.status = CQHTTP_STATUS_FAILED;
    return result;
}

CQHTTP_REQUEST_HANDLER(set_group_card)
(const struct cqhttp_request &request)
{
    struct cqhttp_result result;
    int64_t group_id = cqhttp_get_integer_param(request, "group_id", 0);
    int64_t user_id = cqhttp_get_integer_param(request, "user_id", 0);
    char *card = cqhttp_get_param(request, "card");
    if (group_id && user_id)
        CQ_setGroupCard(ac, group_id, user_id, card ? utf8_to_gbk(card).c_str() : NULL);
    else
        result.status = CQHTTP_STATUS_FAILED;
    if (card)
        free(card);
    return result;
}

CQHTTP_REQUEST_HANDLER(set_group_leave)
(const struct cqhttp_request &request)
{
    struct cqhttp_result result;
    int64_t group_id = cqhttp_get_integer_param(request, "group_id", 0);
    bool is_dismiss = cqhttp_get_bool_param(request, "is_dismiss", false);
    if (group_id)
        CQ_setGroupLeave(ac, group_id, is_dismiss);
    else
        result.status = CQHTTP_STATUS_FAILED;
    return result;
}

CQHTTP_REQUEST_HANDLER(set_group_special_title)
(const struct cqhttp_request &request)
{
    struct cqhttp_result result;
    int64_t group_id = cqhttp_get_integer_param(request, "group_id", 0);
    int64_t user_id = cqhttp_get_integer_param(request, "user_id", 0);
    char *special_title = cqhttp_get_param(request, "special_title");
    int64_t duration = cqhttp_get_integer_param(request, "duration", 30 * 24 * 60 * 60 /* 30 days */);
    if (group_id && user_id)
        CQ_setGroupSpecialTitle(ac, group_id, user_id, special_title ? utf8_to_gbk(special_title).c_str() : NULL, duration);
    else
        result.status = CQHTTP_STATUS_FAILED;
    if (special_title)
        free(special_title);
    return result;
}

CQHTTP_REQUEST_HANDLER(set_discuss_leave)
(const struct cqhttp_request &request)
{
    struct cqhttp_result result;
    int64_t discuss_id = cqhttp_get_integer_param(request, "discuss_id", 0);
    if (discuss_id)
        CQ_setDiscussLeave(ac, discuss_id);
    else
        result.status = CQHTTP_STATUS_FAILED;
    return result;
}
