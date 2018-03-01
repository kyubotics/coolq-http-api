// We don't use "#pragma once" here, because this file is intended to be included twice,
// by sdk_class.h and sdk.cpp, respectively to declare and define SDK functions.
// Except for the two files mentioned above, no file is allowed to include this.

#ifndef FUNC
#define DEFINED_FUNC_MACRO
#define FUNC(ReturnType, FuncName, ...)                              \
    typedef ReturnType(__stdcall *__CQ_##FuncName##_T)(__VA_ARGS__); \
    extern __CQ_##FuncName##_T CQ_##FuncName; // only DECLARE the functions
#endif

#include <cstdint>

using cq_bool_t = int32_t;

// Message
FUNC(int32_t, sendPrivateMsg, int32_t auth_code, int64_t qq, const char *msg)
FUNC(int32_t, sendGroupMsg, int32_t auth_code, int64_t group_id, const char *msg)
FUNC(int32_t, sendDiscussMsg, int32_t auth_code, int64_t discuss_id, const char *msg)
FUNC(int32_t, deleteMsg, int32_t auth_code, int64_t msg_id)

// Send Like
FUNC(int32_t, sendLike, int32_t auth_code, int64_t qq)
FUNC(int32_t, sendLikeV2, int32_t auth_code, int64_t qq, int32_t times)

// Group & Discuss Operation
FUNC(int32_t, setGroupKick, int32_t auth_code, int64_t group_id, int64_t qq, cq_bool_t reject_add_request)
FUNC(int32_t, setGroupBan, int32_t auth_code, int64_t group_id, int64_t qq, int64_t duration)
FUNC(int32_t, setGroupAnonymousBan, int32_t auth_code, int64_t group_id, const char *anonymous, int64_t duration)
FUNC(int32_t, setGroupWholeBan, int32_t auth_code, int64_t group_id, cq_bool_t enable)
FUNC(int32_t, setGroupAdmin, int32_t auth_code, int64_t group_id, int64_t qq, cq_bool_t set)
FUNC(int32_t, setGroupAnonymous, int32_t auth_code, int64_t group_id, cq_bool_t enable)
FUNC(int32_t, setGroupCard, int32_t auth_code, int64_t group_id, int64_t qq, const char *new_card)
FUNC(int32_t, setGroupLeave, int32_t auth_code, int64_t group_id, cq_bool_t is_dismiss)
FUNC(int32_t, setGroupSpecialTitle, int32_t auth_code, int64_t group_id, int64_t qq, const char *new_special_title,
     int64_t duration)
FUNC(int32_t, setDiscussLeave, int32_t auth_code, int64_t discuss_id)

// Request Operation
FUNC(int32_t, setFriendAddRequest, int32_t auth_code, const char *response_flag, int32_t response_operation,
     const char *remark)
FUNC(int32_t, setGroupAddRequest, int32_t auth_code, const char *response_flag, int32_t request_type,
     int32_t response_operation)
FUNC(int32_t, setGroupAddRequestV2, int32_t auth_code, const char *response_flag, int32_t request_type,
     int32_t response_operation, const char *reason)

// Get QQ Information
FUNC(int64_t, getLoginQQ, int32_t auth_code)
FUNC(const char *, getLoginNick, int32_t auth_code)
FUNC(const char *, getStrangerInfo, int32_t auth_code, int64_t qq, cq_bool_t no_cache)
FUNC(const char *, getGroupList, int32_t auth_code)
FUNC(const char *, getGroupMemberList, int32_t auth_code, int64_t group_id)
FUNC(const char *, getGroupMemberInfoV2, int32_t auth_code, int64_t group_id, int64_t qq, cq_bool_t no_cache)

// Get CoolQ Information
FUNC(const char *, getCookies, int32_t auth_code)
FUNC(int32_t, getCsrfToken, int32_t auth_code)
FUNC(const char *, getAppDirectory, int32_t auth_code)
FUNC(const char *, getRecord, int32_t auth_code, const char *file, const char *out_format)

FUNC(int32_t, addLog, int32_t auth_code, int32_t log_level, const char *category, const char *log_msg)
FUNC(int32_t, setFatal, int32_t auth_code, const char *error_info)
FUNC(int32_t, setRestart, int32_t auth_code) // currently ineffective

#ifdef DEFINED_FUNC_MACRO
#undef FUNC
#endif
