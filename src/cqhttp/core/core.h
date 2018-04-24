#pragma once

#include "cqhttp/core/common.h"

#include "cqhttp/core/action.h"
#include "cqhttp/core/context.h"
#include "cqhttp/core/plugin.h"

namespace cqhttp {
    class Application {
    public:
        void on_initialize() {
            initialized_ = true;
            iterate_hooks(&Plugin::hook_initialize, Context());
        }

        void on_enable() {
            enabled_ = true;
            config_ = utils::JsonEx();
            iterate_hooks(&Plugin::hook_enable, Context());
        }

        void on_disable() {
            enabled_ = false;
            iterate_hooks(&Plugin::hook_disable, Context());
        }

        void on_coolq_start() { iterate_hooks(&Plugin::hook_coolq_start, Context()); }

        void on_coolq_exit() {
            enabled_ = false;
            iterate_hooks(&Plugin::hook_coolq_exit, Context());
        }

        void on_message_event(const cq::MessageEvent &event, json &data) {
            iterate_hooks(&Plugin::hook_message_event, EventContext<cq::MessageEvent>(event, data));
        }

        void on_notice_event(const cq::NoticeEvent &event, json &data) {
            iterate_hooks(&Plugin::hook_notice_event, EventContext<cq::NoticeEvent>(event, data));
        }

        void on_request_event(const cq::RequestEvent &event, json &data) {
            iterate_hooks(&Plugin::hook_request_event, EventContext<cq::RequestEvent>(event, data));
        }

        void on_before_action(const std::string &action, utils::JsonEx &params, ActionResult &result) {
            iterate_hooks(&Plugin::hook_before_action, ActionContext(action, params, result));
        }

        void on_missed_action(const std::string &action, utils::JsonEx &params, ActionResult &result) {
            iterate_hooks(&Plugin::hook_missed_action, ActionContext(action, params, result));
        }

        void on_after_action(const std::string &action, utils::JsonEx &params, ActionResult &result) {
            iterate_hooks(&Plugin::hook_after_action, ActionContext(action, params, result));
        }

        bool is_initialized() const { return initialized_; }
        bool is_enabled() const { return enabled_; }

    private:
        std::vector<std::shared_ptr<Plugin>> plugins_;
        utils::JsonEx config_;

        bool initialized_ = false;
        bool enabled_ = false;

        template <typename HookFunc, typename Ctx>
        void iterate_hooks(const HookFunc hook_func, Ctx ctx) {
            ctx.config = &config_;

            auto it = plugins_.begin();
            Context::Next next = [&] {
                if (it == plugins_.end()) {
                    return;
                }

                ctx.next = next;
                (**it++.*hook_func)(ctx);
            };
            next();
        }

        friend void use(const std::shared_ptr<Plugin> plugin);
    };

    extern Application __app;

    /**
     * Initialize the module.
     * This will assign the event handlers of "cqhttp" module to "cqsdk" module.
     */
    void init();

    inline void use(const std::shared_ptr<Plugin> plugin) { __app.plugins_.push_back(plugin); }
} // namespace cqhttp
