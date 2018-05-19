#pragma once

#include "cqhttp/core/plugin.h"

namespace cqhttp::plugins {
    struct Updater : Plugin {
        void hook_enable(Context &ctx) override;
        void hook_missed_action(ActionContext &ctx) override;

    private:
        std::string update_source_;
        std::string update_channel_;
        bool auto_perform_update_{};

        std::string latest_url() const;
        std::string version_info_url(const std::string &version, int build_number) const;
        std::string version_cpk_url(const std::string &version, int build_number) const;

        // return tuple<is_newer, version, build_number, description>
        std::optional<std::tuple<bool, std::string, int, std::string>> get_latest_version() const;

        void check_update(bool automatic) const;
        bool perform_update(const std::string &version, int build_number) const;
    };

    static std::shared_ptr<Updater> updater = std::make_shared<Updater>();
} // namespace cqhttp::plugins
