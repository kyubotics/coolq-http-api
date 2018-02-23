#pragma once

#include "./common.h"
#include "./enums.h"

namespace cq {
    struct Target {
        std::optional<int64_t> user_id;
        std::optional<int64_t> group_id;
        std::optional<int64_t> discuss_id;

        enum Type {
            PRIVATE,
            GROUP,
            DISCUSS
        };

        Target() = default;
        Target(const int64_t user_id) : user_id(user_id) {}

        Target(const int64_t user_id, const int64_t group_or_discuss_id, const Type type) : Target(user_id) {
            if (type == Type::GROUP) {
                group_id = group_or_discuss_id;
            } else if (type == Type::DISCUSS) {
                discuss_id = group_or_discuss_id;
            }
        }
    };
}

namespace cq::event {
    struct Event {
        Type type;
        Target target;
    };

    struct MessageEvent : virtual Event {
        message::Type message_type;
        message::SubType sub_type;
        int32_t message_id;
        std::string message;
        int32_t font;
    };

    struct NoticeEvent : virtual Event {
        int32_t time;
        notice::Type notice_type;
        notice::SubType sub_type;
    };

    struct RequestEvent : virtual Event {
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

        std::string anonymous;
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

        std::string file;
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

    extern std::function<Operation(const PrivateMessageEvent &)> on_private_msg;
    extern std::function<Operation(const GroupMessageEvent &)> on_group_msg;
    extern std::function<Operation(const DiscussMessageEvent &)> on_discuss_msg;
    extern std::function<Operation(const GroupUploadEvent &)> on_group_upload;
    extern std::function<Operation(const GroupAdminEvent &)> on_group_admin;
    extern std::function<Operation(const GroupMemberDecreaseEvent &)> on_group_member_decrease;
    extern std::function<Operation(const GroupMemberIncreaseEvent &)> on_group_member_increase;
    extern std::function<Operation(const FriendAddEvent &)> on_friend_add;
    extern std::function<Operation(const FriendRequestEvent &)> on_friend_request;
    extern std::function<Operation(const GroupRequestEvent &)> on_group_request;
}
