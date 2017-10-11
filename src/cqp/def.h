//
// Created by richard on 2017/8/5.
//

#pragma once

#define CQEVENT(ReturnType, Name, Size) __pragma(comment(linker, "/EXPORT:" #Name "=_" #Name "@" #Size)) extern "C" __declspec(dllexport) ReturnType __stdcall Name

#define CQEVENT_IGNORE 0
#define CQEVENT_BLOCK 1

#define CQREQUEST_ALLOW 1
#define CQREQUEST_DENY 2

#define CQREQUEST_GROUPADD 1
#define CQREQUEST_GROUPINVITE 2

#define CQLOG_DEBUG 0
#define CQLOG_INFO 10
#define CQLOG_INFOSUCCESS 11
#define CQLOG_INFORECV 12
#define CQLOG_INFOSEND 13
#define CQLOG_WARNING 20
#define CQLOG_ERROR 30
#define CQLOG_FATAL 40
