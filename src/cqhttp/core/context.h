#pragma once

#include "cqhttp/core/common.h"

#include "cqhttp/utils/jsonex.h"

namespace cqhttp {
    struct Context {
        using Next = std::function<void()>;

        /**
         * This is granted to be non-null during calling plugins' hook functions.
         */
        utils::JsonEx *config = nullptr;

        /**
         * Should be called by plugins' hook funtions if they want to let other plugins play.
         */
        Next next = nullptr;
    };

    template <typename E>
    struct EventContext : Context {
        /**
         * The raw event object (as a constant).
         */
        const E &event;

        /**
         * The jsonified event data. It may be modified by plugins' hook functions.
         */
        json &data;

        EventContext(const E &event, json &data) : event(event), data(data) {}
    };

    struct ActionContext : Context {
        /**
         * The action's name.
         */
        std::string action;

        /**
         * The action parameters. It may be modified by plugins' hook functions.
         */
        utils::JsonEx &params;

        /**
         * The jsonified action result. It may be modified by plugins' hook functions.
         */
        json &result;

        ActionContext(const std::string &action, utils::JsonEx &params, json &result)
            : action(action), params(params), result(result) {}
    };
} // namespace cqhttp
