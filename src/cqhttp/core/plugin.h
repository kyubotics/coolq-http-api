#pragma once

#include "cqhttp/core/common.h"

namespace cqhttp {
    struct Plugin {
        virtual ~Plugin() = default;

        using Next = std::function<void()>;

        virtual void hook_initialize(const Next next) { next(); }
        virtual void hook_enable(const Next next) { next(); }
        virtual void hook_disable(const Next next) { next(); }
        virtual void hook_coolq_start(const Next next) { next(); }
        virtual void hook_coolq_exit(const Next next) { next(); }

        virtual void hook_message_event(const cq::message::Type type, const cq::message::SubType sub_type, json &data,
                                        const Next next) { next(); }

        virtual void hook_notice_event(const cq::notice::Type type, const cq::notice::SubType sub_type, json &data,
                                       const Next next) { next(); }

        virtual void hook_request_event(const cq::request::Type type, const cq::request::SubType sub_type, json &data,
                                        const Next next) { next(); }

        virtual void hook_before_action(const std::string &action, json &params, const Next next) { next(); }

        virtual void hook_missed_action(const std::string &action, json &params, json &result, const Next next) {
            next();
        }

        virtual void hook_after_action(const std::string &action, json &result, const Next next) { next(); }
    };
}
