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
    int64_t id = cqhttp_get_integer_param(request, "id", 0);
    char *msg = cqhttp_get_param(request, "msg");
    if (id && msg)
        CQ_sendPrivateMsg(ac, id, utf8_to_gbk(msg).c_str());
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
    int64_t id = cqhttp_get_integer_param(request, "id", 0);
    char *msg = cqhttp_get_param(request, "msg");
    if (id && msg)
        CQ_sendGroupMsg(ac, id, utf8_to_gbk(msg).c_str());
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
    int64_t id = cqhttp_get_integer_param(request, "id", 0);
    char *msg = cqhttp_get_param(request, "msg");
    if (id && msg)
        CQ_sendDiscussMsg(ac, id, utf8_to_gbk(msg).c_str());
    else
        result.status = CQHTTP_STATUS_FAILED;
    if (msg)
        free(msg);
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
