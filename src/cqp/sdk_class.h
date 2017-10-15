// 
// sdk_class.h : Wrap C-style CoolQ SDK functions.
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

#include "./funcs.h"

#include "utils/base64.h"

class Sdk {
public:
    bool enabled = false;

    Sdk(int32_t auth_code) : ac_(auth_code) {}

    #pragma region Send Message

    int32_t send_private_msg(int64_t qq, const std::string &msg) const {
        return CQ_sendPrivateMsg(this->ac_, qq, string_encode(msg, Encodings::ANSI).c_str());
    }

    int32_t send_group_msg(int64_t group_id, const std::string &msg) const {
        return CQ_sendGroupMsg(this->ac_, group_id, string_encode(msg, Encodings::ANSI).c_str());
    }

    int32_t send_discuss_msg(int64_t discuss_id, const std::string &msg) const {
        return CQ_sendDiscussMsg(this->ac_, discuss_id, string_encode(msg, Encodings::ANSI).c_str());
    }

    #pragma endregion

    #pragma region Send Like

    int32_t send_like(int64_t qq) const {
        return CQ_sendLike(this->ac_, qq);
    }

    int32_t send_like(int64_t qq, int32_t times) const {
        return CQ_sendLikeV2(this->ac_, qq, times);
    }

    #pragma endregion

    #pragma region Group & Discuss Operation

    int32_t set_group_kick(int64_t group_id, int64_t qq, bool reject_add_request) const {
        return CQ_setGroupKick(this->ac_, group_id, qq, reject_add_request);
    }

    int32_t set_group_ban(int64_t group_id, int64_t qq, int64_t duration) const {
        return CQ_setGroupBan(this->ac_, group_id, qq, duration);
    }

    int32_t set_group_anonymous_ban(int64_t group_id, const std::string &anonymous_flag, int64_t duration) const {
        return CQ_setGroupAnonymousBan(this->ac_, group_id, string_encode(anonymous_flag, Encodings::ANSI).c_str(),
                                       duration);
    }

    int32_t set_group_whole_ban(int64_t group_id, bool enable) const {
        return CQ_setGroupWholeBan(this->ac_, group_id, enable);
    }

    int32_t set_group_admin(int64_t group_id, int64_t qq, bool set) const {
        return CQ_setGroupAdmin(this->ac_, group_id, qq, set);
    }

    int32_t set_group_anonymous(int64_t group_id, bool enable) const {
        return CQ_setGroupAnonymous(this->ac_, group_id, enable);
    }

    int32_t set_group_card(int64_t group_id, int64_t qq, const std::string &new_card) const {
        return CQ_setGroupCard(this->ac_, group_id, qq, string_encode(new_card, Encodings::ANSI).c_str());
    }

    int32_t set_group_leave(int64_t group_id, bool is_dismiss) const {
        return CQ_setGroupLeave(this->ac_, group_id, is_dismiss);
    }

    int32_t set_group_special_title(int64_t group_id, int64_t qq, const std::string &new_special_title,
                                    int64_t duration) const {
        return CQ_setGroupSpecialTitle(this->ac_, group_id, qq,
                                       string_encode(new_special_title, Encodings::ANSI).c_str(), duration);
    }

    int32_t set_discuss_leave(int64_t discuss_id) const {
        return CQ_setDiscussLeave(this->ac_, discuss_id);
    }

    #pragma endregion

    #pragma region Request Operation

    int32_t set_friend_add_request(const std::string &response_flag, int32_t response_operation,
                                   const std::string &remark) const {
        return CQ_setFriendAddRequest(this->ac_, string_encode(response_flag, Encodings::ANSI).c_str(),
                                      response_operation, string_encode(remark, Encodings::ANSI).c_str());
    }

    int32_t set_group_add_request(const std::string &response_flag, int32_t request_type,
                                  int32_t response_operation) const {
        return CQ_setGroupAddRequest(this->ac_, string_encode(response_flag, Encodings::ANSI).c_str(),
                                     request_type, response_operation);
    }

    int32_t set_group_add_request(const std::string &response_flag, int32_t request_type, int32_t response_operation,
                                  const std::string &reason) const {
        return CQ_setGroupAddRequestV2(this->ac_, string_encode(response_flag, Encodings::ANSI).c_str(),
                                       request_type, response_operation,
                                       string_encode(reason, Encodings::ANSI).c_str());
    }

    #pragma endregion

    #pragma region Get QQ Information

    int64_t get_login_qq() const {
        return CQ_getLoginQQ(this->ac_);
    }

    std::string get_login_nick() const {
        auto nick = CQ_getLoginNick(this->ac_);
        return nick ? string_decode(nick, Encodings::ANSI) : std::string();
    }

    bytes get_stranger_info_raw(int64_t qq, bool no_cache) const {
        return base64_decode(CQ_getStrangerInfo(this->ac_, qq, no_cache));
    }

    bytes get_group_list_raw() const {
        return base64_decode(CQ_getGroupList(this->ac_));
    }

    bytes get_group_member_list_raw(int64_t group_id) const {
        return base64_decode(CQ_getGroupMemberList(this->ac_, group_id));
    }

    bytes get_group_member_info_raw(int64_t group_id, int64_t qq, bool no_cache) const {
        return base64_decode(CQ_getGroupMemberInfoV2(this->ac_, group_id, qq, no_cache));
    }

    #pragma endregion

    #pragma region Get CoolQ Information

    std::string get_cookies() const {
        auto cookies = CQ_getCookies(this->ac_);
        return cookies ? string_decode(cookies, Encodings::ANSI) : std::string();
    }

    int32_t get_csrf_token() const {
        return CQ_getCsrfToken(this->ac_);
    }

    std::string get_app_directory() const {
        return string_decode(CQ_getAppDirectory(this->ac_), Encodings::ANSI);
    }

    /**
     * Get root directory of CoolQ, including the trailing "\".
     */
    std::string get_coolq_directory() const {
        static std::string dir;
        if (dir.empty()) {
            const auto app_dir = get_app_directory();
            const auto suffix = std::string("app\\" CQAPP_ID "\\");
            dir = app_dir.substr(0, app_dir.length() - suffix.length());
        }
        return dir;
    }

    // const char *get_record(const char *file, const char *out_format) const {
    //     return CQ_getRecord(this->ac_, file, out_format);
    // }

    #pragma endregion

    #pragma region CoolQ Self-operation

    int32_t add_log(int32_t level, const std::string &category, const std::string &msg) const {
        return CQ_addLog(this->ac_, level, string_encode(category, Encodings::ANSI).c_str(),
                         string_encode(msg, Encodings::ANSI).c_str());
    }

    //int32_t set_fatal(const char *error_info) const {
    //    return CQ_setFatal(this->ac_, error_info);
    //}
    //
    //int32_t set_restart() const {
    //    return CQ_setRestart(this->ac_);
    //}

    #pragma endregion

private:
    int32_t ac_;
};
