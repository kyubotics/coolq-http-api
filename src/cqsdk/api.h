#pragma once

#include "./common.h"

#include "./utils/base64.h"
#include "./utils/string.h"
#include "./app.h"
#include "./enums.h"
#include "./target.h"
#include "./message.h"
#include "./types.h"
#include "./utils/binpack.h"

namespace cq::api {
    /**
     * Init all API functions.
     * This is internally called in the Initialize exported function.
     */
    void __init();

    /**
     * Provide ways to access the raw CoolQ API functions.
     */
    namespace raw {
        #include "./api_funcs.h"
    }

    #pragma region Send Message

    inline int32_t send_private_msg(const int64_t user_id, const std::string &msg) {
        return raw::CQ_sendPrivateMsg(app::auth_code, user_id, utils::string_to_coolq(msg).c_str());
    }

    inline int32_t send_group_msg(const int64_t group_id, const std::string &msg) {
        return raw::CQ_sendGroupMsg(app::auth_code, group_id, utils::string_to_coolq(msg).c_str());
    }

    inline int32_t send_discuss_msg(const int64_t discuss_id, const std::string &msg) {
        return raw::CQ_sendDiscussMsg(app::auth_code, discuss_id, utils::string_to_coolq(msg).c_str());
    }

    inline int32_t delete_msg(const int64_t msg_id) {
        return raw::CQ_deleteMsg(app::auth_code, msg_id);
    }

    #pragma endregion

    #pragma region Send Like

    inline int32_t send_like(const int64_t user_id) {
        return raw::CQ_sendLike(app::auth_code, user_id);
    }

    inline int32_t send_like(const int64_t user_id, const int32_t times) {
        return raw::CQ_sendLikeV2(app::auth_code, user_id, times);
    }

    #pragma endregion

    #pragma region Group & Discuss Operation

    inline int32_t set_group_kick(const int64_t group_id, const int64_t user_id, const bool reject_add_request) {
        return raw::CQ_setGroupKick(app::auth_code, group_id, user_id, reject_add_request);
    }

    inline int32_t set_group_ban(const int64_t group_id, const int64_t user_id, const int64_t duration) {
        return raw::CQ_setGroupBan(app::auth_code, group_id, user_id, duration);
    }

    inline int32_t set_group_anonymous_ban(const int64_t group_id, const std::string &flag, const int64_t duration) {
        return raw::CQ_setGroupAnonymousBan(app::auth_code, group_id, utils::string_to_coolq(flag).c_str(), duration);
    }

    inline int32_t set_group_whole_ban(const int64_t group_id, const bool enable) {
        return raw::CQ_setGroupWholeBan(app::auth_code, group_id, enable);
    }

    inline int32_t set_group_admin(const int64_t group_id, const int64_t user_id, const bool enable) {
        return raw::CQ_setGroupAdmin(app::auth_code, group_id, user_id, enable);
    }

    inline int32_t set_group_anonymous(const int64_t group_id, const bool enable) {
        return raw::CQ_setGroupAnonymous(app::auth_code, group_id, enable);
    }

    inline int32_t set_group_card(const int64_t group_id, const int64_t user_id, const std::string &card) {
        return raw::CQ_setGroupCard(app::auth_code, group_id, user_id, utils::string_to_coolq(card).c_str());
    }

    inline int32_t set_group_leave(const int64_t group_id, const bool is_dismiss) {
        return raw::CQ_setGroupLeave(app::auth_code, group_id, is_dismiss);
    }

    inline int32_t set_group_special_title(const int64_t group_id, const int64_t user_id,
                                           const std::string &special_title, const int64_t duration) {
        return raw::CQ_setGroupSpecialTitle(app::auth_code, group_id, user_id,
                                            utils::string_to_coolq(special_title).c_str(), duration);
    }

    inline int32_t set_discuss_leave(const int64_t discuss_id) {
        return raw::CQ_setDiscussLeave(app::auth_code, discuss_id);
    }

    #pragma endregion

    #pragma region Request Operation

    inline int32_t set_friend_add_request(const std::string &flag, const request::Operation operation,
                                          const std::string &remark) {
        return raw::CQ_setFriendAddRequest(app::auth_code, utils::string_to_coolq(flag).c_str(),
                                           operation, utils::string_to_coolq(remark).c_str());
    }

    inline int32_t set_group_add_request(const std::string &flag, const request::SubType type,
                                         const request::Operation operation) {
        return raw::CQ_setGroupAddRequest(app::auth_code, utils::string_to_coolq(flag).c_str(), type, operation);
    }

    inline int32_t set_group_add_request(const std::string &flag, const request::SubType type,
                                         const request::Operation operation, const std::string &reason) {
        return raw::CQ_setGroupAddRequestV2(app::auth_code, utils::string_to_coolq(flag).c_str(), type, operation,
                                            utils::string_to_coolq(reason).c_str());
    }

    #pragma endregion

    #pragma region Get QQ Information

    inline int64_t get_login_user_id() {
        return raw::CQ_getLoginQQ(app::auth_code);
    }

    inline std::string get_login_nickname() {
        const auto nick = raw::CQ_getLoginNick(app::auth_code);
        return nick ? utils::string_from_coolq(nick) : std::string();
    }

    inline std::string get_stranger_info_raw(const int64_t user_id, const bool no_cache = false) {
        return utils::base64::decode(raw::CQ_getStrangerInfo(app::auth_code, user_id, no_cache));
    }

    inline std::string get_group_list_raw() {
        return utils::base64::decode(raw::CQ_getGroupList(app::auth_code));
    }

    inline std::string get_group_member_list_raw(const int64_t group_id) {
        return utils::base64::decode(raw::CQ_getGroupMemberList(app::auth_code, group_id));
    }

    inline std::string get_group_member_info_raw(const int64_t group_id, const int64_t user_id,
                                                 const bool no_cache = false) {
        return utils::base64::decode(raw::CQ_getGroupMemberInfoV2(app::auth_code, group_id, user_id, no_cache));
    }

    #pragma endregion

    #pragma region Get CoolQ Information

    inline std::string get_cookies() {
        const auto cookies = raw::CQ_getCookies(app::auth_code);
        return cookies ? utils::string_from_coolq(cookies) : std::string();
    }

    inline int32_t get_csrf_token() {
        return raw::CQ_getCsrfToken(app::auth_code);
    }

    inline std::string get_app_directory() {
        return utils::string_from_coolq(raw::CQ_getAppDirectory(app::auth_code));
    }

    inline std::string get_record(const std::string &file, const std::string &out_format) {
        const auto raw = raw::CQ_getRecord(app::auth_code, utils::string_to_coolq(file).c_str(),
                                           utils::string_to_coolq(out_format).c_str());
        return utils::string_from_coolq(raw);
    }

    #pragma endregion

    #pragma region CoolQ Self-operation

    //int32_t set_fatal(const char *error_info) {
    //    return raw::CQ_setFatal(app::auth_code, error_info);
    //}
    //
    //int32_t set_restart() {
    //    return raw::CQ_setRestart(app::auth_code);
    //}

    #pragma endregion

    #pragma region CQSDK Bonus

    inline int32_t send_msg(const Target &target, const std::string &msg) {
        if (target.group_id.has_value()) {
            return send_group_msg(target.group_id.value(), msg);
        }
        if (target.discuss_id.has_value()) {
            return send_discuss_msg(target.discuss_id.value(), msg);
        }
        if (target.user_id.has_value()) {
            return send_private_msg(target.user_id.value(), msg);
        }
        return -1;
    }

    inline int32_t send_private_msg(const int64_t user_id, const message::Message &msg) {
        return send_private_msg(user_id, std::string(msg));
    }

    inline int32_t send_group_msg(const int64_t group_id, const message::Message &msg) {
        return send_group_msg(group_id, std::string(msg));
    }

    inline int32_t send_discuss_msg(const int64_t discuss_id, const message::Message &msg) {
        return send_discuss_msg(discuss_id, std::string(msg));
    }

    inline int32_t send_msg(const Target &target, const message::Message &msg) {
        return send_msg(target, std::string(msg));
    }

    inline User get_stranger_info(const int64_t user_id, const bool no_cache = false) {
        return User::from_bytes(get_stranger_info_raw(user_id, no_cache));
    }

    inline std::vector<Group> get_group_list() {
        auto bytes = get_group_list_raw();
        std::vector<Group> result;
        if (bytes.size() > 4 /* at least has a count */) {
            auto pack = utils::BinPack(bytes);
            const auto count = pack.pop_int<int32_t>();
            for (auto i = 0; i < count; i++) {
                result.push_back(Group::from_bytes(pack.pop_token()));
            }
        }
        return result;
    }

    inline std::vector<GroupMember> get_group_member_list(const int64_t group_id) {
        auto bytes = get_group_member_list_raw(group_id);
        std::vector<GroupMember> result;
        if (bytes.size() > 4 /* at least has a count */) {
            auto pack = utils::BinPack(bytes);
            const auto count = pack.pop_int<int32_t>();
            for (auto i = 0; i < count; i++) {
                result.push_back(GroupMember::from_bytes(pack.pop_token()));
            }
        }
        return result;
    }

    inline GroupMember get_group_member_info(const int64_t group_id, const int64_t user_id,
                                             const bool no_cache = false) {
        return GroupMember::from_bytes(get_group_member_info_raw(group_id, user_id, no_cache));
    }

    inline User get_login_info() {
        return get_stranger_info(get_login_user_id());
    }

    #pragma endregion
}
