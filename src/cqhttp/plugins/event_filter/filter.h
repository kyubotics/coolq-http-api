#pragma once

#include "cqhttp/core/plugin.h"

namespace cqhttp::plugins {
    class IFilter {
    public:
        virtual ~IFilter() = default;
        virtual bool eval(const json &payload) = 0;
    };

    struct FilterSyntexError : std::invalid_argument {
        using invalid_argument::invalid_argument;
    };

    std::shared_ptr<IFilter> construct_filter(const json &root_filter);
} // namespace cqhttp::plugins
