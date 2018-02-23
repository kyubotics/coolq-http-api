#pragma once

#include "./common.h"

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

        explicit Target(const int64_t user_id) : user_id(user_id) {}

        explicit Target(const int64_t group_or_discuss_id, const Type type) {
            if (type == GROUP) {
                group_id = group_or_discuss_id;
            } else if (type == DISCUSS) {
                discuss_id = group_or_discuss_id;
            }
        }

        explicit Target(const int64_t user_id, const int64_t group_or_discuss_id, const Type type) :
            Target(group_or_discuss_id, type) {
            this->user_id = user_id;
        }
    };
}
