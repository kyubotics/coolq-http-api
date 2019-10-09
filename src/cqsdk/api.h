#pragma once

#include "./common.h"

#include "./app.h"
#include "./enums.h"
#include "./target.h"
#include "./types.h"
#include "./utils/string.h"

namespace cq::exception {
    struct ApiError : RuntimeError {
        int code;
        ApiError(const int code) : RuntimeError("failed to call coolq api") { this->code = code; }

        static const auto INVALID_DATA = 100;
        static const auto INVALID_TARGET = 101;
    };
} // namespace cq::exception

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

    inline void __throw_if_needed(const int32_t ret) noexcept(false) {
        if (ret < 0) {
            throw exception::ApiError(ret);
        }
    }

    inline void __throw_if_needed(const void *const ret_ptr) noexcept(false) {
        if (!ret_ptr) {
            throw exception::ApiError(exception::ApiError::INVALID_DATA);
        }
    }

#pragma region Message

    inline int64_t send_private_msg(const int64_t user_id, const std::string &msg) noexcept(false) {
        const auto ret = raw::CQ_sendPrivateMsg(app::auth_code, user_id, utils::string_to_coolq(msg).c_str());
        __throw_if_needed(ret);
        return ret;
    }

    inline int64_t send_group_msg(const int64_t group_id, const std::string &msg) noexcept(false) {
        const auto ret = raw::CQ_sendGroupMsg(app::auth_code, group_id, utils::string_to_coolq(msg).c_str());
        __throw_if_needed(ret);
        return ret;
    }

    inline int64_t send_discuss_msg(const int64_t discuss_id, const std::string &msg) noexcept(false) {
        const auto ret = raw::CQ_sendDiscussMsg(app::auth_code, discuss_id, utils::string_to_coolq(msg).c_str());
        __throw_if_needed(ret);
        return ret;
    }

    inline void delete_msg(const int64_t msg_id) noexcept(false) {
        __throw_if_needed(raw::CQ_deleteMsg(app::auth_code, msg_id));
    }

#pragma endregion

#pragma region Send Like

    inline void send_like(const int64_t user_id) noexcept(false) {
        __throw_if_needed(raw::CQ_sendLike(app::auth_code, user_id));
    }

    inline void send_like(const int64_t user_id, const int32_t times) noexcept(false) {
        __throw_if_needed(raw::CQ_sendLikeV2(app::auth_code, user_id, times));
    }

#pragma endregion

#pragma region Group &Discuss Operation

    inline void set_group_kick(const int64_t group_id, const int64_t user_id,
                               const bool reject_add_request) noexcept(false) {
        __throw_if_needed(raw::CQ_setGroupKick(app::auth_code, group_id, user_id, reject_add_request));
    }

    inline void set_group_ban(const int64_t group_id, const int64_t user_id, const int64_t duration) noexcept(false) {
        __throw_if_needed(raw::CQ_setGroupBan(app::auth_code, group_id, user_id, duration));
    }

    inline void set_group_anonymous_ban(const int64_t group_id, const std::string &flag,
                                        const int64_t duration) noexcept(false) {
        __throw_if_needed(
            raw::CQ_setGroupAnonymousBan(app::auth_code, group_id, utils::string_to_coolq(flag).c_str(), duration));
    }

    inline void set_group_whole_ban(const int64_t group_id, const bool enable) noexcept(false) {
        __throw_if_needed(raw::CQ_setGroupWholeBan(app::auth_code, group_id, enable));
    }

    inline void set_group_admin(const int64_t group_id, const int64_t user_id, const bool enable) noexcept(false) {
        __throw_if_needed(raw::CQ_setGroupAdmin(app::auth_code, group_id, user_id, enable));
    }

    inline void set_group_anonymous(const int64_t group_id, const bool enable) noexcept(false) {
        __throw_if_needed(raw::CQ_setGroupAnonymous(app::auth_code, group_id, enable));
    }

    inline void set_group_card(const int64_t group_id, const int64_t user_id, const std::string &card) noexcept(false) {
        __throw_if_needed(
            raw::CQ_setGroupCard(app::auth_code, group_id, user_id, utils::string_to_coolq(card).c_str()));
    }

    inline void set_group_leave(const int64_t group_id, const bool is_dismiss) noexcept(false) {
        __throw_if_needed(raw::CQ_setGroupLeave(app::auth_code, group_id, is_dismiss));
    }

    inline void set_group_special_title(const int64_t group_id, const int64_t user_id, const std::string &special_title,
                                        const int64_t duration) noexcept(false) {
        __throw_if_needed(raw::CQ_setGroupSpecialTitle(
            app::auth_code, group_id, user_id, utils::string_to_coolq(special_title).c_str(), duration));
    }

    inline void set_discuss_leave(const int64_t discuss_id) noexcept(false) {
        __throw_if_needed(raw::CQ_setDiscussLeave(app::auth_code, discuss_id));
    }

#pragma endregion

#pragma region Request Operation

    inline void set_friend_add_request(const std::string &flag, const request::Operation operation,
                                       const std::string &remark) noexcept(false) {
        __throw_if_needed(raw::CQ_setFriendAddRequest(
            app::auth_code, utils::string_to_coolq(flag).c_str(), operation, utils::string_to_coolq(remark).c_str()));
    }

    inline void set_group_add_request(const std::string &flag, const request::SubType type,
                                      const request::Operation operation) noexcept(false) {
        __throw_if_needed(
            raw::CQ_setGroupAddRequest(app::auth_code, utils::string_to_coolq(flag).c_str(), type, operation));
    }

    inline void set_group_add_request(const std::string &flag, const request::SubType type,
                                      const request::Operation operation, const std::string &reason) noexcept(false) {
        __throw_if_needed(raw::CQ_setGroupAddRequestV2(app::auth_code,
                                                       utils::string_to_coolq(flag).c_str(),
                                                       type,
                                                       operation,
                                                       utils::string_to_coolq(reason).c_str()));
    }

#pragma endregion

#pragma region Get QQ Information

    inline int64_t get_login_user_id() noexcept { return raw::CQ_getLoginQQ(app::auth_code); }

    inline std::string get_login_nickname() noexcept(false) {
        const auto ret = raw::CQ_getLoginNick(app::auth_code);
        __throw_if_needed(ret);
        return utils::string_from_coolq(ret);
    }

    inline std::string get_stranger_info_base64(const int64_t user_id, const bool no_cache = false) noexcept(false) {
        const auto ret = raw::CQ_getStrangerInfo(app::auth_code, user_id, no_cache);
        __throw_if_needed(ret);
        return utils::string_from_coolq(ret);
    }

    inline std::string get_friend_list_base64() noexcept(false) {
        const auto ret = raw::CQ_getFriendList(app::auth_code, false);
        __throw_if_needed(ret);
        return utils::string_from_coolq(ret);
    }

    inline std::string get_group_list_base64() noexcept(false) {
        const auto ret = raw::CQ_getGroupList(app::auth_code);
        __throw_if_needed(ret);
        return utils::string_from_coolq(ret);
    }

    inline std::string get_group_info_base64(const int64_t group_id, const bool no_cache = false) noexcept(false) {
        const auto ret = raw::CQ_getGroupInfo(app::auth_code, group_id, no_cache);
        __throw_if_needed(ret);
        return utils::string_from_coolq(ret);
    }

    inline std::string get_group_member_list_base64(const int64_t group_id) noexcept(false) {
        const auto ret = raw::CQ_getGroupMemberList(app::auth_code, group_id);
        __throw_if_needed(ret);
        return utils::string_from_coolq(ret);
    }

    inline std::string get_group_member_info_base64(const int64_t group_id, const int64_t user_id,
                                                    const bool no_cache = false) noexcept(false) {
        const auto ret = raw::CQ_getGroupMemberInfoV2(app::auth_code, group_id, user_id, no_cache);
        __throw_if_needed(ret);
        return utils::string_from_coolq(ret);
    }

#pragma endregion

#pragma region Get CoolQ Information

    inline std::string get_cookies() noexcept(false) {
        const auto ret = raw::CQ_getCookies(app::auth_code);
        __throw_if_needed(ret);
        return utils::string_from_coolq(ret);
    }

    inline std::string get_cookies(const std::string &domain) noexcept(false) {
        const auto ret = raw::CQ_getCookiesV2(app::auth_code, utils::string_to_coolq(domain).c_str());
        __throw_if_needed(ret);
        return utils::string_from_coolq(ret);
    }

    inline int32_t get_csrf_token() noexcept { return raw::CQ_getCsrfToken(app::auth_code); }

    inline std::string get_app_directory() noexcept(false) {
        const auto ret = raw::CQ_getAppDirectory(app::auth_code);
        __throw_if_needed(ret);
        return utils::string_from_coolq(ret);
    }

    inline std::string get_record(const std::string &file, const std::string &out_format,
                                  const bool full_path = false) noexcept(false) {
        const auto ret =
            full_path
                ? raw::CQ_getRecordV2(
                      app::auth_code, utils::string_to_coolq(file).c_str(), utils::string_to_coolq(out_format).c_str())
                : raw::CQ_getRecord(
                      app::auth_code, utils::string_to_coolq(file).c_str(), utils::string_to_coolq(out_format).c_str());
        __throw_if_needed(ret);
        return utils::string_from_coolq(ret);
    }

    inline std::string get_image(const std::string &file) noexcept(false) {
        const auto ret = raw::CQ_getImage(app::auth_code, utils::string_to_coolq(file).c_str());
        __throw_if_needed(ret);
        return utils::string_from_coolq(ret);
    }

    inline bool can_send_image() noexcept(false) { return static_cast<bool>(raw::CQ_canSendImage(app::auth_code)); }

    inline bool can_send_record() noexcept(false) { return static_cast<bool>(raw::CQ_canSendRecord(app::auth_code)); }

#pragma endregion

#pragma region CoolQ Self - operation

    // int32_t set_fatal(const char *error_info) {
    //    return raw::CQ_setFatal(app::auth_code, error_info);
    //}
    //
    // int32_t set_restart() {
    //    return raw::CQ_setRestart(app::auth_code);
    //}

#pragma endregion

#pragma region CQSDK Bonus

    inline int64_t send_msg(const Target &target, const std::string &msg) noexcept(false) {
        if (target.group_id.has_value()) {
            return send_group_msg(target.group_id.value(), msg);
        }
        if (target.discuss_id.has_value()) {
            return send_discuss_msg(target.discuss_id.value(), msg);
        }
        if (target.user_id.has_value()) {
            return send_private_msg(target.user_id.value(), msg);
        }
        throw exception::ApiError(exception::ApiError::INVALID_TARGET);
    }

    inline User get_stranger_info(const int64_t user_id, const bool no_cache = false) noexcept(false) {
        try {
            return ObjectHelper::from_base64<User>(get_stranger_info_base64(user_id, no_cache));
        } catch (exception::ParseError &) {
            throw exception::ApiError(exception::ApiError::INVALID_DATA);
        }
    }

    inline std::vector<Friend> get_friend_list() noexcept(false) {
        try {
            return ObjectHelper::multi_from_base64<std::vector<Friend>>(get_friend_list_base64());
        } catch (exception::ParseError &) {
            throw exception::ApiError(exception::ApiError::INVALID_DATA);
        }
    }

    inline std::vector<Group> get_group_list() noexcept(false) {
        try {
            return ObjectHelper::multi_from_base64<std::vector<Group>>(get_group_list_base64());
        } catch (exception::ParseError &) {
            throw exception::ApiError(exception::ApiError::INVALID_DATA);
        }
    }

    inline Group get_group_info(const int64_t group_id, const bool no_cache = false) noexcept(false) {
        try {
            return ObjectHelper::from_base64<Group>(get_group_info_base64(group_id, no_cache));
        } catch (exception::ParseError &) {
            throw exception::ApiError(exception::ApiError::INVALID_DATA);
        }
    }

    inline std::vector<GroupMember> get_group_member_list(const int64_t group_id) noexcept(false) {
        try {
            return ObjectHelper::multi_from_base64<std::vector<GroupMember>>(get_group_member_list_base64(group_id));
        } catch (exception::ParseError &) {
            throw exception::ApiError(exception::ApiError::INVALID_DATA);
        }
    }

    inline GroupMember get_group_member_info(const int64_t group_id, const int64_t user_id,
                                             const bool no_cache = false) noexcept(false) {
        try {
            return ObjectHelper::from_base64<GroupMember>(get_group_member_info_base64(group_id, user_id, no_cache));
        } catch (exception::ParseError &) {
            throw exception::ApiError(exception::ApiError::INVALID_DATA);
        }
    }

    inline User get_login_info() noexcept(false) { return get_stranger_info(get_login_user_id()); }

#pragma endregion
} // namespace cq::api
