#pragma once

#include "cqhttp/core/common.h"
#include "cqhttp/core/plugin.h"

namespace cqhttp {
    class Application {
    public:
        void use(const std::shared_ptr<Plugin> plugin) { plugins_.push_back(plugin); }

        void __hook_initialize() { iterate_hooks(&Plugin::hook_initialize); }

        void __hook_enable() { iterate_hooks(&Plugin::hook_enable); }

        void __hook_disable() { iterate_hooks(&Plugin::hook_disable); }

        void __hook_coolq_start() { iterate_hooks(&Plugin::hook_coolq_start); }

        void __hook_coolq_exit() { iterate_hooks(&Plugin::hook_coolq_exit); }

        void __hook_message_event(const cq::message::Type type, const cq::message::SubType sub_type, json &data) {
            iterate_hooks(&Plugin::hook_message_event, type, sub_type, data);
        }

        void __hook_notice_event(const cq::notice::Type type, const cq::notice::SubType sub_type, json &data) {
            iterate_hooks(&Plugin::hook_notice_event, type, sub_type, data);
        }

        void __hook_request_event(const cq::request::Type type, const cq::request::SubType sub_type, json &data) {
            iterate_hooks(&Plugin::hook_request_event, type, sub_type, data);
        }

        void __hook_before_action(const std::string &action, json &params, json &result) {
            iterate_hooks(&Plugin::hook_before_action, action, params, result);
        }

        void __hook_missed_action(const std::string &action, json &params, json &result) {
            iterate_hooks(&Plugin::hook_missed_action, action, params, result);
        }

        void __hook_after_action(const std::string &action, json &params, json &result) {
            iterate_hooks(&Plugin::hook_after_action, action, params, result);
        }

    private:
        std::vector<std::shared_ptr<Plugin>> plugins_;

        template <typename HookFunc, typename... Args>
        void iterate_hooks(const HookFunc &hook_func, Args &&... args) {
            auto it = plugins_.begin();
            Plugin::Next next = [&]() {
                if (it == plugins_.end()) {
                    return;
                }

                (**it++.*hook_func)(std::forward<Args>(args)..., next);
            };
            next();
        }
    };

    /**
     * Initialize the module.
     * This will assign the event handlers of "cqhttp" module to "cqsdk" module.
     */
    void init();

    /**
     * Apply the given application. This will override any previous applied applications.
     */
    void apply(const std::shared_ptr<Application> app);
} // namespace cqhttp
