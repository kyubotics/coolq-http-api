#pragma once

#include "cqhttp/core/common.h"
#include "cqhttp/core/plugin.h"

namespace cqhttp {
    class Application {
    public:
        void use(const std::shared_ptr<Plugin> plugin) {
            plugins_.push_back(plugin);
        }

        #define LIFECYCLE(Name) \
            void on_##Name() { \
                auto it = plugins_.begin(); \
                Plugin::Next next = [&]() { \
                    if (it == plugins_.end()) { return; } \
                    (*it++)->hook_##Name(next); \
                }; \
                next(); \
            }

        LIFECYCLE(initialize);
        LIFECYCLE(enable);
        LIFECYCLE(disable);
        LIFECYCLE(coolq_start);
        LIFECYCLE(coolq_exit);

        #undef LIFECYCLE

        #define EVENT(EventType) \
            void on_##EventType##_event(const cq::EventType::Type type, const cq::EventType::SubType sub_type, \
                                        json &data) { \
                auto it = plugins_.begin(); \
                Plugin::Next next = [&]() { \
                    if (it == plugins_.end()) { return; } \
                    (*it++)->hook_##EventType##_event(type, sub_type, data, next); \
                }; \
                next(); \
            }

        EVENT(message);
        EVENT(notice);
        EVENT(request);

        #undef EVENT

        void on_before_action(const std::string &name, json &params) {
            auto it = plugins_.begin();
            Plugin::Next next = [&]() {
                if (it == plugins_.end()) { return; }
                (*it++)->hook_before_action(name, params, next);
            };
            next();
        }

        void on_after_action(const std::string &name, json &result) {
            auto it = plugins_.begin();
            Plugin::Next next = [&]() {
                if (it == plugins_.end()) { return; }
                (*it++)->hook_after_action(name, result, next);
            };
            next();
        }

    private:
        std::vector<std::shared_ptr<Plugin>> plugins_;
    };

    void apply(const std::shared_ptr<Application> app);
}
