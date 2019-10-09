#pragma once

#include "./common.h"

#include "./enums.h"
#include "./message.h"
#include "./target.h"
#include "./types.h"

namespace cq::event {
    struct Event {
        Type type;
        Target target;

        mutable Operation operation = IGNORE;
        void block() const { operation = BLOCK; }
    };

    struct MessageEvent : Event {
        message::Type message_type;
        message::SubType sub_type;
        int32_t message_id;
        std::string raw_message;
        message::Message message;
        int32_t font;
    };

    struct NoticeEvent : Event {
        int32_t time;
        notice::Type notice_type;
        notice::SubType sub_type;
    };

    struct RequestEvent : Event {
        int32_t time;
        request::Type request_type;
        request::SubType sub_type;
        std::string comment;
        std::string flag;
    };

    struct UserIdMixin {
        int64_t user_id;
    };

    struct GroupIdMixin {
        int64_t group_id;
    };

    struct DiscussIdMixin {
        int64_t discuss_id;
    };

    struct OperatorIdMixin {
        int64_t operator_id;
    };

    struct PrivateMessageEvent final : MessageEvent, UserIdMixin {
        PrivateMessageEvent() {
            type = MESSAGE;
            message_type = message::PRIVATE;
        }
    };

    struct GroupMessageEvent final : MessageEvent, UserIdMixin, GroupIdMixin {
        GroupMessageEvent() {
            type = MESSAGE;
            message_type = message::GROUP;
        }

        Anonymous anonymous;

        bool is_anonymous() const { return !anonymous.name.empty(); }
    };

    struct DiscussMessageEvent final : MessageEvent, UserIdMixin, DiscussIdMixin {
        DiscussMessageEvent() {
            type = MESSAGE;
            message_type = message::DISCUSS;
        }
    };

    struct GroupUploadEvent final : NoticeEvent, UserIdMixin, GroupIdMixin {
        GroupUploadEvent() {
            type = NOTICE;
            notice_type = notice::GROUP_UPLOAD;
        }

        File file;
    };

    struct GroupAdminEvent final : NoticeEvent, UserIdMixin, GroupIdMixin {
        GroupAdminEvent() {
            type = NOTICE;
            notice_type = notice::GROUP_ADMIN;
        }
    };

    struct GroupMemberDecreaseEvent final : NoticeEvent, UserIdMixin, GroupIdMixin, OperatorIdMixin {
        GroupMemberDecreaseEvent() {
            type = NOTICE;
            notice_type = notice::GROUP_MEMBER_DECREASE;
        }
    };

    struct GroupMemberIncreaseEvent final : NoticeEvent, UserIdMixin, GroupIdMixin, OperatorIdMixin {
        GroupMemberIncreaseEvent() {
            type = NOTICE;
            notice_type = notice::GROUP_MEMBER_INCREASE;
        }
    };

    struct GroupBanEvent final : NoticeEvent, UserIdMixin, GroupIdMixin, OperatorIdMixin {
        GroupBanEvent() {
            type = NOTICE;
            notice_type = notice::GROUP_BAN;
        }

        int64_t duration;
    };

    struct FriendAddEvent final : NoticeEvent, UserIdMixin {
        FriendAddEvent() {
            type = NOTICE;
            notice_type = notice::FRIEND_ADD;
        }
    };

    struct FriendRequestEvent final : RequestEvent, UserIdMixin {
        FriendRequestEvent() {
            type = REQUEST;
            request_type = request::FRIEND;
        }
    };

    struct GroupRequestEvent final : RequestEvent, UserIdMixin, GroupIdMixin {
        GroupRequestEvent() {
            type = REQUEST;
            request_type = request::GROUP;
        }
    };

    extern std::function<void(const PrivateMessageEvent &)> on_private_msg;
    extern std::function<void(const GroupMessageEvent &)> on_group_msg;
    extern std::function<void(const DiscussMessageEvent &)> on_discuss_msg;
    extern std::function<void(const GroupUploadEvent &)> on_group_upload;
    extern std::function<void(const GroupAdminEvent &)> on_group_admin;
    extern std::function<void(const GroupMemberDecreaseEvent &)> on_group_member_decrease;
    extern std::function<void(const GroupMemberIncreaseEvent &)> on_group_member_increase;
    extern std::function<void(const GroupBanEvent &)> on_group_ban;
    extern std::function<void(const FriendAddEvent &)> on_friend_add;
    extern std::function<void(const FriendRequestEvent &)> on_friend_request;
    extern std::function<void(const GroupRequestEvent &)> on_group_request;
} // namespace cq::event
