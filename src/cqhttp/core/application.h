#pragma once

#include "cqhttp/core/common.h"

#include "cqhttp/core/action.h"
#include "cqhttp/core/context.h"
#include "cqhttp/core/event.h"
#include "cqhttp/core/plugin.h"
#include "cqhttp/core/vendor/ctpl/ctpl_stl.h"
#include "cqhttp/core/vendor/scheduler/Scheduler.h"

namespace cqhttp {
    class Application {
    public:
        void on_initialize();
        void on_enable();
        void on_disable();
        void on_coolq_start();
        void on_coolq_exit();

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

        void on_meta_event(const cqhttp::MetaEvent &event, json &data) {
            iterate_hooks(&Plugin::hook_meta_event, EventContext<cqhttp::MetaEvent>(event, data));
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

        bool resize_worker_thread_pool(int n_threads) const;
        bool push_async_task(const std::function<void()> &task) const;

        utils::JsonEx &config() { return config_; }
        utils::JsonEx &store() { return store_; }
        Bosma::Scheduler &scheduler() {
            if (scheduler_) {
                return *scheduler_;
            }
            throw std::runtime_error("scheduler has not been created");
        }

    private:
        std::vector<std::shared_ptr<Plugin>> plugins_;
        utils::JsonEx config_;
        utils::JsonEx store_;
        std::shared_ptr<ctpl::thread_pool> worker_thread_pool_;
        std::shared_ptr<Bosma::Scheduler> scheduler_;

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
