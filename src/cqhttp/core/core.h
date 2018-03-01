#pragma once

#include "cqhttp/core/common.h"
#include "cqhttp/core/plugin.h"

namespace cqhttp {
    class Application {
    public:
        void use(const std::shared_ptr<Plugin> plugin) { plugins_.push_back(plugin); }

/**
 * Generate lifecycle hooks.
 */
#define LIFECYCLE(Name)                 \
    void __hook_##Name() {              \
        auto it = plugins_.begin();     \
        Plugin::Next next = [&]() {     \
            if (it == plugins_.end()) { \
                return;                 \
            }                           \
            (*it++)->hook_##Name(next); \
        };                              \
        next();                         \
    }

        LIFECYCLE(initialize);
        LIFECYCLE(enable);
        LIFECYCLE(disable);
        LIFECYCLE(coolq_start);
        LIFECYCLE(coolq_exit);

#undef LIFECYCLE

/**
 * Generate event hooks.
 */
#define EVENT(EventType)                                                                     \
    void __hook_##EventType##_event(                                                         \
        const cq::EventType::Type type, const cq::EventType::SubType sub_type, json &data) { \
        auto it = plugins_.begin();                                                          \
        Plugin::Next next = [&]() {                                                          \
            if (it == plugins_.end()) {                                                      \
                return;                                                                      \
            }                                                                                \
            (*it++)->hook_##EventType##_event(type, sub_type, data, next);                   \
        };                                                                                   \
        next();                                                                              \
    }

        EVENT(message);
        EVENT(notice);
        EVENT(request);

#undef EVENT

        void __hook_before_action(const std::string &action, json &params, json &result) {
            auto it = plugins_.begin();
            Plugin::Next next = [&]() {
                if (it == plugins_.end()) {
                    return;
                }
                (*it++)->hook_before_action(action, params, result, next);
            };
            next();
        }

        void __hook_missed_action(const std::string &action, json &params, json &result) {
            auto it = plugins_.begin();
            Plugin::Next next = [&]() {
                if (it == plugins_.end()) {
                    return;
                }
                (*it++)->hook_missed_action(action, params, result, next);
            };
            next();
        }

        void __hook_after_action(const std::string &action, json &params, json &result) {
            auto it = plugins_.begin();
            Plugin::Next next = [&]() {
                if (it == plugins_.end()) {
                    return;
                }
                (*it++)->hook_after_action(action, params, result, next);
            };
            next();
        }

    private:
        std::vector<std::shared_ptr<Plugin>> plugins_;
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
}  // namespace cqhttp
