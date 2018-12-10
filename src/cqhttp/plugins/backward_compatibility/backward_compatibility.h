#pragma once

#include "cqhttp/core/plugin.h"

namespace cqhttp::plugins {
    struct BackwardCompatibility : Plugin {
        BackwardCompatibility() = default;
        std::string name() const override { return "backward_compatibility"; }
        void hook_enable(Context &ctx) override;
        void hook_after_event(EventContext<cq::Event> &ctx) override;

    private:
        bool enabled_{};
    };

    static std::shared_ptr<BackwardCompatibility> backward_compatibility = std::make_shared<BackwardCompatibility>();
} // namespace cqhttp::plugins
