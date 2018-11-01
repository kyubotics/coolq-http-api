#pragma once

#undef IGNORE

namespace cq {
    namespace event {
        enum Operation {
            IGNORE = 0,
            BLOCK = 1,
        };

        enum Type {
            UNKNOWN,
            MESSAGE,
            NOTICE,
            REQUEST,
        };
    } // namespace event

    namespace message {
        enum Type {
            UNKNOWN,
            PRIVATE,
            GROUP,
            DISCUSS,
        };

        enum SubType {
            SUBTYPE_DEFAULT = 1,

            PRIVATE_FRIEND = 11,
            PRIVATE_OTHER = 1,
            PRIVATE_GROUP = 2,
            PRIVATE_DISCUSS = 3,
        };
    } // namespace message

    namespace notice {
        enum Type {
            UNKNOWN,
            GROUP_UPLOAD,
            GROUP_ADMIN,
            GROUP_MEMBER_DECREASE,
            GROUP_MEMBER_INCREASE,
            FRIEND_ADD,
        };

        enum SubType {
            SUBTYPE_DEFAULT = 1,

            GROUP_ADMIN_UNSET = 1,
            GROUP_ADMIN_SET = 2,
            GROUP_MEMBER_DECREASE_LEAVE = 1,
            GROUP_MEMBER_DECREASE_KICK = 2,
            GROUP_MEMBER_INCREASE_APPROVE = 1,
            GROUP_MEMBER_INCREASE_INVITE = 2,
        };
    } // namespace notice

    namespace request {
        enum Operation {
            APPROVE = 1,
            REJECT = 2,
        };

        enum Type {
            UNKNOWN,
            FRIEND,
            GROUP,
        };

        enum SubType {
            SUBTYPE_DEFAULT = 1,

            GROUP_ADD = 1,
            GROUP_INVITE = 2,
        };
    } // namespace request
} // namespace cq
