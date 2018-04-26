#pragma once

#include "./common.h"

namespace cq {
    struct Target {
        std::optional<int64_t> user_id;
        std::optional<int64_t> group_id;
        std::optional<int64_t> discuss_id;

        enum Type { USER, GROUP, DISCUSS };

        Target() = default;

        explicit Target(const int64_t user_or_group_or_discuss_id, const Type type = USER) {
            switch (type) {
            case USER:
                user_id = user_or_group_or_discuss_id;
                break;
            case GROUP:
                group_id = user_or_group_or_discuss_id;
                break;
            case DISCUSS:
                discuss_id = user_or_group_or_discuss_id;
            }
        }

        explicit Target(const int64_t user_id, const int64_t group_or_discuss_id, const Type type)
            : Target(group_or_discuss_id, type) {
            this->user_id = user_id;
        }

        static Target user(const int64_t user_id) { return Target(user_id, USER); }
        static Target group(const int64_t group_id) { return Target(group_id, GROUP); }
        static Target discuss(const int64_t discuss_id) { return Target(discuss_id, DISCUSS); }
    };
} // namespace cq
