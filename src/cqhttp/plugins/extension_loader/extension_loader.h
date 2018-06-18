#pragma once

#include "cqhttp/core/plugin.h"

#include <Windows.h>

#include "cqhttp/extension/extension.h"

namespace cqhttp::plugins {
    struct ExtensionLoader : Plugin {
        void hook_enable(Context &ctx) override;
        void hook_disable(Context &ctx) override;
        void hook_after_event(EventContext<cq::Event> &ctx) override;
        void hook_missed_action(ActionContext &ctx) override;

    private:
		bool enable_{};
        std::vector<HMODULE> dll_handles_;
        std::vector<std::shared_ptr<extension::Extension>> extensions_;
    };

    static std::shared_ptr<ExtensionLoader> extension_loader = std::make_shared<ExtensionLoader>();
} // namespace cqhttp::plugins
