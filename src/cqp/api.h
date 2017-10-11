//
// Created by richard on 2017/8/5.
//

#pragma once

#ifdef API
#include "api_initiator.h"
#else
#define DEFINED_API
#define API(ReturnType, ApiName, ...) \
    typedef ReturnType (__stdcall *CQ_##ApiName##_T)(__VA_ARGS__); \
    extern CQ_##ApiName##_T CQ_##ApiName;
#endif

#include <cstdint>

typedef int32_t cq_bool_t;

// Send Message
API(int32_t, sendPrivateMsg, int32_t auth_code, int64_t qq, const char *msg)
API(int32_t, sendGroupMsg, int32_t auth_code, int64_t group_id, const char *msg)
API(int32_t, sendDiscussMsg, int32_t auth_code, int64_t discuss_id, const char *msg)

// Send Like
API(int32_t, sendLike, int32_t auth_code, int64_t qq)
API(int32_t, sendLikeV2, int32_t auth_code, int64_t qq, int32_t times)

// Group & Discuss Operation
API(int32_t, setGroupKick, int32_t auth_code, int64_t group_id, int64_t qq, cq_bool_t reject_add_request)
API(int32_t, setGroupBan, int32_t auth_code, int64_t group_id, int64_t qq, int64_t duration)
API(int32_t, setGroupAnonymousBan, int32_t auth_code, int64_t group_id, const char *anonymous, int64_t duration)
API(int32_t, setGroupWholeBan, int32_t auth_code, int64_t group_id, cq_bool_t enable)
API(int32_t, setGroupAdmin, int32_t auth_code, int64_t group_id, int64_t qq, cq_bool_t set)
API(int32_t, setGroupAnonymous, int32_t auth_code, int64_t group_id, cq_bool_t enable)
API(int32_t, setGroupCard, int32_t auth_code, int64_t group_id, int64_t qq, const char *new_card)
API(int32_t, setGroupLeave, int32_t auth_code, int64_t group_id, cq_bool_t is_dismiss)
API(int32_t, setGroupSpecialTitle, int32_t auth_code, int64_t group_id, int64_t qq, const char *new_special_title,
    int64_t duration)
API(int32_t, setDiscussLeave, int32_t auth_code, int64_t discuss_id)

// Request Operation
API(int32_t, setFriendAddRequest, int32_t auth_code, const char *response_flag, int32_t response_operation,
    const char * remark)
API(int32_t, setGroupAddRequest, int32_t auth_code, const char *response_flag, int32_t request_type,
    int32_t response_operation)
API(int32_t, setGroupAddRequestV2, int32_t auth_code, const char *response_flag, int32_t request_type,
    int32_t response_operation, const char *reason)

// Get QQ Information
API(int64_t, getLoginQQ, int32_t auth_code)
API(const char *, getLoginNick, int32_t auth_code)
API(const char *, getStrangerInfo, int32_t auth_code, int64_t qq, cq_bool_t no_cache)
API(const char *, getGroupList, int32_t auth_code)
API(const char *, getGroupMemberList, int32_t auth_code, int64_t group_id)
API(const char *, getGroupMemberInfoV2, int32_t auth_code, int64_t group_id, int64_t qq, cq_bool_t no_cache)

// Get CoolQ Information
API(const char *, getCookies, int32_t auth_code)
API(int32_t, getCsrfToken, int32_t auth_code)
API(const char *, getAppDirectory, int32_t auth_code)
API(const char *, getRecord, int32_t auth_code, const char *file, const char *out_format)

API(int32_t, addLog, int32_t auth_code, int32_t log_level, const char *category, const char *log_msg)
API(int32_t, setFatal, int32_t auth_code, const char *error_info)
API(int32_t, setRestart, int32_t auth_code) // currently banned by sdk

#ifdef DEFINED_API
#undef API
#endif

void init_dll();
