#pragma once

#include "cqhttp/core/plugin.h"

namespace cqhttp::plugins {
    struct CACertDownloader : Plugin {
        void hook_enable(Context &ctx) override;
    };

    static std::shared_ptr<CACertDownloader> ca_cert_downloader = std::make_shared<CACertDownloader>();
} // namespace cqhttp::plugins
