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
            cq::logging::info(u8"日志",
                              u8"请在酷 Q 主目录的 app\\io.github.richardchien.coolqhttpapi\\log 中查看日志文件");
            iterate_hooks(&Plugin::hook_enable, Context());
        }

        void on_disable() {
            enabled_ = false;
            iterate_hooks(&Plugin::hook_disable, Context());
        }

        void on_coolq_start() {
            logging::init();
            iterate_hooks(&Plugin::hook_coolq_start, Context());
        }

        void on_coolq_exit() {
            if (enabled_) {
                // generate a fake "disable" event at exit
                // this leads to a lifecycle change, check plugin.h for the lifecycle graph
                on_disable();
            }
            iterate_hooks(&Plugin::hook_coolq_exit, Context());
            logging::destroy();
        }

        void on_before_event(const cq::Event &event, json &data) {
            iterate_hooks(&Plugin::hook_before_event, EventContext<cq::Event>(event, data));
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

        void on_after_event(const cq::Event &event, json &data) {
            iterate_hooks(&Plugin::hook_after_event, EventContext<cq::Event>(event, data));
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

        bool initialized() const { return initialized_; }
        bool enabled() const { return enabled_; }
        bool plugins_good() const {
            return std::all_of(plugins_.cbegin(), plugins_.cend(), [](const auto &p) { return p->good(); });
        }
        bool good() const { return initialized() && enabled() && plugins_good(); }

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

        friend void use(std::shared_ptr<Plugin> plugin);
    };
} // namespace cqhttp
