/*
* CoolQ SDK for VC++
* Api Version 9.6
* Written by Coxxs & Thanks for the help of orzFly
*/
#pragma once

#define CQAPIVER 9
#define CQAPIVERTEXT "9"

#ifndef CQAPI
#define CQAPI(ReturnType) extern "C" __declspec(dllimport) ReturnType __stdcall
#endif

#define CQEVENT(ReturnType, Name, Size) __pragma(comment(linker, "/EXPORT:" #Name "=_" #Name "@" #Size)) extern "C" __declspec(dllexport) ReturnType __stdcall Name

typedef int32_t CQBOOL;

#define EVENT_IGNORE 0 //事件_忽略
#define EVENT_BLOCK 1  //事件_拦截

#define REQUEST_ALLOW 1 //请求_通过
#define REQUEST_DENY 2  //请求_拒绝

#define REQUEST_GROUPADD 1    //请求_群添加
#define REQUEST_GROUPINVITE 2 //请求_群邀请

#define CQLOG_DEBUG 0        //调试 灰色
#define CQLOG_INFO 10        //信息 黑色
#define CQLOG_INFOSUCCESS 11 //信息(成功) 紫色
#define CQLOG_INFORECV 12    //信息(接收) 蓝色
#define CQLOG_INFOSEND 13    //信息(发送) 绿色
#define CQLOG_WARNING 20     //警告 橙色
#define CQLOG_ERROR 30       //错误 红色
#define CQLOG_FATAL 40       //致命错误 深红

CQAPI(int32_t)
CQ_sendPrivateMsg(int32_t AuthCode, int64_t QQID, const char *msg);
CQAPI(int32_t)
CQ_sendGroupMsg(int32_t AuthCode, int64_t groupid, const char *msg);
CQAPI(int32_t)
CQ_sendDiscussMsg(int32_t AuthCode, int64_t discussid, const char *msg);
CQAPI(int32_t)
CQ_sendLike(int32_t AuthCode, int64_t QQID);
CQAPI(int32_t)
CQ_setGroupKick(int32_t AuthCode, int64_t groupid, int64_t QQID, CQBOOL rejectaddrequest);
CQAPI(int32_t)
CQ_setGroupBan(int32_t AuthCode, int64_t groupid, int64_t QQID, int64_t duration);
CQAPI(int32_t)
CQ_setGroupAnonymousBan(int32_t AuthCode, int64_t groupid, const char *anonymous, int64_t duration);
CQAPI(int32_t)
CQ_setGroupWholeBan(int32_t AuthCode, int64_t groupid, CQBOOL enableban);
CQAPI(int32_t)
CQ_setGroupAdmin(int32_t AuthCode, int64_t groupid, int64_t QQID, CQBOOL setadmin);
CQAPI(int32_t)
CQ_setGroupAnonymous(int32_t AuthCode, int64_t groupid, CQBOOL enableanonymous);
CQAPI(int32_t)
CQ_setGroupCard(int32_t AuthCode, int64_t groupid, int64_t QQID, const char *newcard);
CQAPI(int32_t)
CQ_setGroupLeave(int32_t AuthCode, int64_t groupid, CQBOOL isdismiss);
CQAPI(int32_t)
CQ_setGroupSpecialTitle(int32_t AuthCode, int64_t groupid, int64_t QQID, const char *newspecialtitle, int64_t duration);
CQAPI(int32_t)
CQ_setDiscussLeave(int32_t AuthCode, int64_t discussid);
CQAPI(int32_t)
CQ_setFriendAddRequest(int32_t AuthCode, const char *responseflag, int32_t responseoperation, const char *remark);
CQAPI(int32_t)
CQ_setGroupAddRequestV2(int32_t AuthCode, const char *responseflag, int32_t requesttype, int32_t responseoperation, const char *reason);
CQAPI(const char *)
CQ_getGroupMemberInfoV2(int32_t AuthCode, int64_t groupid, int64_t QQID, CQBOOL nocache);
CQAPI(const char *)
CQ_getStrangerInfo(int32_t AuthCode, int64_t QQID, CQBOOL nocache);
CQAPI(int32_t)
CQ_addLog(int32_t AuthCode, int32_t priority, const char *category, const char *content);
CQAPI(const char *)
CQ_getCookies(int32_t AuthCode);
CQAPI(int32_t)
CQ_getCsrfToken(int32_t AuthCode);
CQAPI(int64_t)
CQ_getLoginQQ(int32_t AuthCode);
CQAPI(const char *)
CQ_getLoginNick(int32_t AuthCode);
CQAPI(const char *)
CQ_getAppDirectory(int32_t AuthCode);
CQAPI(int32_t)
CQ_setFatal(int32_t AuthCode, const char *errorinfo);
