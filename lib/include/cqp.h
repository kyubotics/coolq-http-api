/*
* CoolQ SDK for VC++
* Api Version 9.6
* Written by Coxxs & Thanks for the help of orzFly
*
* Modified by Richard Chien
*/

#pragma once

#define CQAPIVER 9
#define CQAPIVERTEXT "9"

#ifndef CQAPI
#define CQAPI(ReturnType) extern "C" __declspec(dllimport) ReturnType __stdcall
#endif

#define CQEVENT(ReturnType, Name, Size) __pragma(comment(linker, "/EXPORT:" #Name "=_" #Name "@" #Size)) extern "C" __declspec(dllexport) ReturnType __stdcall Name

typedef int32_t cq_bool_t;

#define EVENT_IGNORE 0
#define EVENT_BLOCK 1

#define REQUEST_ALLOW 1
#define REQUEST_DENY 2

#define REQUEST_GROUPADD 1
#define REQUEST_GROUPINVITE 2

#define CQLOG_DEBUG 0
#define CQLOG_INFO 10
#define CQLOG_INFOSUCCESS 11
#define CQLOG_INFORECV 12
#define CQLOG_INFOSEND 13
#define CQLOG_WARNING 20
#define CQLOG_ERROR 30
#define CQLOG_FATAL 40

// Send Message
CQAPI(int32_t)
CQ_sendPrivateMsg(int32_t auth_code, int64_t qq, const char *msg);
CQAPI(int32_t)
CQ_sendGroupMsg(int32_t auth_code, int64_t group_id, const char *msg);
CQAPI(int32_t)
CQ_sendDiscussMsg(int32_t auth_code, int64_t discuss_id, const char *msg);

// Send Like
CQAPI(int32_t)
CQ_sendLike(int32_t auth_code, int64_t qq);
CQAPI(int32_t)
CQ_sendLikeV2(int32_t auth_code, int64_t qq, int32_t times);

// Group & Discuss Operation
CQAPI(int32_t)
CQ_setGroupKick(int32_t auth_code, int64_t group_id, int64_t qq, cq_bool_t reject_add_request);
CQAPI(int32_t)
CQ_setGroupBan(int32_t auth_code, int64_t group_id, int64_t qq, int64_t duration);
CQAPI(int32_t)
CQ_setGroupAnonymousBan(int32_t auth_code, int64_t group_id, const char *anonymous, int64_t duration);
CQAPI(int32_t)
CQ_setGroupWholeBan(int32_t auth_code, int64_t group_id, cq_bool_t enable);
CQAPI(int32_t)
CQ_setGroupAdmin(int32_t auth_code, int64_t group_id, int64_t qq, cq_bool_t set);
CQAPI(int32_t)
CQ_setGroupAnonymous(int32_t auth_code, int64_t group_id, cq_bool_t enable);
CQAPI(int32_t)
CQ_setGroupCard(int32_t auth_code, int64_t group_id, int64_t qq, const char *new_card);
CQAPI(int32_t)
CQ_setGroupLeave(int32_t auth_code, int64_t group_id, cq_bool_t is_dismiss);
CQAPI(int32_t)
CQ_setGroupSpecialTitle(int32_t auth_code, int64_t group_id, int64_t qq, const char *new_special_title, int64_t duration);
CQAPI(int32_t)
CQ_setDiscussLeave(int32_t auth_code, int64_t discuss_id);

// Request Operation
CQAPI(int32_t)
CQ_setFriendAddRequest(int32_t auth_code, const char *response_flag, int32_t response_operation, const char *remark);
CQAPI(int32_t)
CQ_setGroupAddRequest(int32_t auth_code, const char *response_flag, int32_t request_type, int32_t response_operation);
CQAPI(int32_t)
CQ_setGroupAddRequestV2(int32_t auth_code, const char *response_flag, int32_t request_type, int32_t response_operation, const char *reason);

// Get QQ Information
CQAPI(int64_t)
CQ_getLoginQQ(int32_t auth_code);
CQAPI(const char *)
CQ_getLoginNick(int32_t auth_code);
CQAPI(const char *)
CQ_getStrangerInfo(int32_t auth_code, int64_t qq, cq_bool_t no_cache);
CQAPI(const char *)
CQ_getGroupMemberList(int32_t auth_code, int64_t group_id);
CQAPI(const char *)
CQ_getGroupMemberInfoV2(int32_t auth_code, int64_t group_id, int64_t qq, cq_bool_t no_cache);

// Get CoolQ Information
CQAPI(const char *)
CQ_getCookies(int32_t auth_code);
CQAPI(int32_t)
CQ_getCsrfToken(int32_t auth_code);
CQAPI(const char *)
CQ_getAppDirectory(int32_t auth_code);
CQAPI(const char *)
CQ_getRecord(int32_t auth_code, const char *file, const char *out_format);

// CoolQ Self-operation
CQAPI(int32_t)
CQ_addLog(int32_t auth_code, int32_t log_level, const char *category, const char *log_msg);
CQAPI(int32_t)
CQ_setFatal(int32_t auth_code, const char *error_info);
CQAPI(int32_t)
CQ_setRestart(int32_t auth_code); // currently banned by CQ
