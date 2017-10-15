// 
// def.h : Define CoolQ constants.
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
