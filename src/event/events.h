// 
// events.cpp : Declare event handlers.
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

#include "common.h"

int32_t event_private_msg(int32_t sub_type, int32_t send_time, int64_t from_qq, const std::string &msg, int32_t font);
int32_t event_group_msg(int32_t sub_type, int32_t send_time, int64_t from_group, int64_t from_qq, const std::string &from_anonymous, const std::string &msg, int32_t font);
int32_t event_discuss_msg(int32_t sub_Type, int32_t send_time, int64_t from_discuss, int64_t from_qq, const std::string &msg, int32_t font);
int32_t event_group_upload(int32_t sub_type, int32_t send_time, int64_t from_group, int64_t from_qq, const std::string &file);
int32_t event_group_admin(int32_t sub_type, int32_t send_time, int64_t from_group, int64_t being_operate_qq);
int32_t event_group_member_decrease(int32_t sub_type, int32_t send_time, int64_t from_group, int64_t from_qq, int64_t being_operate_qq);
int32_t event_group_member_increase(int32_t sub_type, int32_t send_time, int64_t from_group, int64_t from_qq, int64_t being_operate_qq);
int32_t event_friend_add(int32_t sub_type, int32_t send_time, int64_t from_qq);
int32_t event_add_friend_request(int32_t sub_type, int32_t send_time, int64_t from_qq, const std::string &msg, const std::string &response_flag);
int32_t event_add_group_request(int32_t sub_type, int32_t send_time, int64_t from_group, int64_t from_qq, const std::string &msg, const std::string &response_flag);
