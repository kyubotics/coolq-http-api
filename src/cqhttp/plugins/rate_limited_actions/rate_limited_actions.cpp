#include "./rate_limited_actions.h"

#include "cqhttp/core/core.h"
#include "cqhttp/utils/mutex.h"

using namespace std;

namespace cqhttp::plugins {
    const auto TAG = u8"限速动作";

    using utils::mutex::with_unique_lock;

    void RateLimitedActions::hook_enable(Context &ctx) {
        enabled_ = ctx.config->get_bool("enable_rate_limited_actions", false);
        if (enabled_) {
            interval_ = chrono::milliseconds(ctx.config->get_integer("rate_limit_interval", 500));
            chan_ = make_shared<Channel<QueuedContext>>();
            worker_thread_ = thread([&]() {
                worker_running_ = true;
                for (;;) {
                    try {
                        if (QueuedContext queued_ctx; chan_->get(queued_ctx)) {
                            call_action(queued_ctx.action, queued_ctx.params);
                            logging::debug(TAG, u8"成功执行一个限速动作");
                            this_thread::sleep_for(interval_);
                        }
                    } catch (...) {
                        break;
                    }
                }
                worker_running_ = false;
            });
        }

        ctx.next();
    }

    void RateLimitedActions::hook_disable(Context &ctx) {
        if (enabled_) {
            chan_->close();
            if (worker_thread_.joinable()) {
                worker_thread_.join();
            }
        }
        ctx.next();
    }

    void RateLimitedActions::hook_missed_action(ActionContext &ctx) {
        if (enabled_) {
            const auto suffix = "_async_rate_limited";
            const auto suffix_len = strlen(suffix);
            if (boost::ends_with(ctx.action, suffix)) {
                const auto action = ctx.action.substr(0, ctx.action.length() - suffix_len);
                if (!boost::ends_with(action, suffix)) {
                    chan_->put(QueuedContext{action, ctx.params});
                    logging::debug(TAG, u8"限速动作已进入限速队列等待执行");
                    ctx.result.code = ActionResult::Codes::ASYNC;
                }
            }
        }

        ctx.next();
    }
} // namespace cqhttp::plugins
