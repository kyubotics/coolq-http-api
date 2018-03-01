#pragma once

#include "cqhttp/core/plugin.h"

namespace cqhttp::plugins {
    struct Http : Plugin {
        // void hook_initialize(const Next next) override;
        void hook_enable(const Next next) override {
            logging::debug("wow", "你好");
            next();
        }
        // void hook_disable(const Next next) override;
        // void hook_coolq_start(const Next next) override;
        // void hook_coolq_exit(const Next next) override;
        // void hook_message_event(const cq::message::Type type, const cq::message::SubType sub_type, json &data,
        //                        const Next next) override;
        // void hook_notice_event(const cq::notice::Type type, const cq::notice::SubType sub_type, json &data,
        //                       const Next next) override;
        // void hook_request_event(const cq::request::Type type, const cq::request::SubType sub_type, json &data,
        //                        const Next next) override;
        // void hook_before_action(const std::string &name, json &params, const Next next) override;
        // void hook_after_action(const std::string &name, json &result, const Next next) override;
    };

    static std::shared_ptr<Http> http = std::make_shared<Http>();
} // namespace cqhttp::plugins
