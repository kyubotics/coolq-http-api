#pragma once

#include "common.h"

#include <cqp.h>

#include "conf/Config.h"
#include "Structs.h"
#include "encoding/base64.h"

class CQApp {
public:
    Config config;
    bool enabled = false;

    CQApp(int32_t auth_code) : ac_(auth_code) {}

    #pragma region Send Message

    int32_t sendPrivateMsg(int64_t qq, const str &msg) const {
        return CQ_sendPrivateMsg(this->ac_, qq, encode(msg, Encoding::GBK).c_str());
    }

    int32_t sendGroupMsg(int64_t group_id, const str &msg) const {
        return CQ_sendGroupMsg(this->ac_, group_id, encode(msg, Encoding::GBK).c_str());
    }

    int32_t sendDiscussMsg(int64_t discuss_id, const str &msg) const {
        return CQ_sendDiscussMsg(this->ac_, discuss_id, encode(msg, Encoding::GBK).c_str());
    }

    #pragma endregion

    #pragma region Send Like

    int32_t sendLike(int64_t qq) const {
        return CQ_sendLike(this->ac_, qq);
    }

    int32_t sendLikeV2(int64_t qq, int32_t times) const {
        return CQ_sendLikeV2(this->ac_, qq, times);
    }

    #pragma endregion

    #pragma region Group & Discuss Operation

    int32_t setGroupKick(int64_t group_id, int64_t qq, bool reject_add_request) const {
        return CQ_setGroupKick(this->ac_, group_id, qq, reject_add_request);
    }

    int32_t setGroupBan(int64_t group_id, int64_t qq, int64_t duration) const {
        return CQ_setGroupBan(this->ac_, group_id, qq, duration);
    }

    int32_t setGroupAnonymousBan(int64_t group_id, const str &anonymous_flag, int64_t duration) const {
        return CQ_setGroupAnonymousBan(this->ac_, group_id, encode(anonymous_flag, Encoding::GBK).c_str(), duration);
    }

    int32_t setGroupWholeBan(int64_t group_id, bool enable) const {
        return CQ_setGroupWholeBan(this->ac_, group_id, enable);
    }

    int32_t setGroupAdmin(int64_t group_id, int64_t qq, bool set) const {
        return CQ_setGroupAdmin(this->ac_, group_id, qq, set);
    }

    int32_t setGroupAnonymous(int64_t group_id, bool enable) const {
        return CQ_setGroupAnonymous(this->ac_, group_id, enable);
    }

    int32_t setGroupCard(int64_t group_id, int64_t qq, const str &new_card) const {
        return CQ_setGroupCard(this->ac_, group_id, qq, encode(new_card, Encoding::GBK).c_str());
    }

    int32_t setGroupLeave(int64_t group_id, bool is_dismiss) const {
        return CQ_setGroupLeave(this->ac_, group_id, is_dismiss);
    }

    int32_t setGroupSpecialTitle(int64_t group_id, int64_t qq, const str &new_special_title, int64_t duration) const {
        return CQ_setGroupSpecialTitle(this->ac_, group_id, qq, encode(new_special_title, Encoding::GBK).c_str(), duration);
    }

    int32_t setDiscussLeave(int64_t discuss_id) const {
        return CQ_setDiscussLeave(this->ac_, discuss_id);
    }

    #pragma endregion

    #pragma region Request Operation

    int32_t setFriendAddRequest(const str &response_flag, int32_t response_operation, const str &remark) const {
        return CQ_setFriendAddRequest(this->ac_, encode(response_flag, Encoding::GBK).c_str(),
                                      response_operation, encode(remark, Encoding::GBK).c_str());
    }

    int32_t setGroupAddRequest(const str &response_flag, int32_t request_type, int32_t response_operation) const {
        return CQ_setGroupAddRequest(this->ac_, encode(response_flag, Encoding::GBK).c_str(),
                                     request_type, response_operation);
    }

    int32_t setGroupAddRequestV2(const str &response_flag, int32_t request_type, int32_t response_operation, const str &reason) const {
        return CQ_setGroupAddRequestV2(this->ac_, encode(response_flag, Encoding::GBK).c_str(),
                                       request_type, response_operation, encode(reason, Encoding::GBK).c_str());
    }

    #pragma endregion

    #pragma region Get QQ Information

    int64_t getLoginQQ() const {
        return CQ_getLoginQQ(this->ac_);
    }

    str getLoginNick() const {
        auto nick = CQ_getLoginNick(this->ac_);
        return nick ? decode(nick, Encoding::GBK) : str();
    }

    bytes getStrangerInfoRaw(int64_t qq, bool no_cache) const {
        return base64_decode(CQ_getStrangerInfo(this->ac_, qq, no_cache));
    }

    bytes getGroupListRaw() const {
        return base64_decode(CQ_getGroupList(this->ac_));
    }

    bytes getGroupMemberListRaw(int64_t group_id) const {
        return base64_decode(CQ_getGroupMemberList(this->ac_, group_id));
    }

    bytes getGroupMemberInfoRawV2(int64_t group_id, int64_t qq, bool no_cache) const {
        return base64_decode(CQ_getGroupMemberInfoV2(this->ac_, group_id, qq, no_cache));
    }

    #pragma endregion

    #pragma region Get CoolQ Information

    str getCookies() const {
        auto cookies = CQ_getCookies(this->ac_);
        return cookies ? decode(cookies, Encoding::GBK) : str();
    }

    int32_t getCsrfToken() const {
        return CQ_getCsrfToken(this->ac_);
    }

    str getAppDirectory() const {
        return decode(CQ_getAppDirectory(this->ac_), Encoding::GBK);
    }

    // const char *getRecord(const char *file, const char *out_format) const {
    //     return CQ_getRecord(this->ac_, file, out_format);
    // }

    #pragma endregion

    #pragma region CoolQ Self-operation

    int32_t addLog(int32_t level, const str &category, const str &msg) const {
        return CQ_addLog(this->ac_, level, encode(category, Encoding::GBK).c_str(), encode(msg, Encoding::GBK).c_str());
    }

    // int32_t setFatal(const char *error_info) const {
    //     return CQ_setFatal(this->ac_, error_info);
    // }
    // 
    // int32_t setRestart() const {
    //     return CQ_setRestart(this->ac_);
    // }

    #pragma endregion

private:
    int32_t ac_;
};
