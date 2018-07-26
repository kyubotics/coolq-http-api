#include "./restarter.h"

#include "cqhttp/core/core.h"
#include "cqhttp/utils/mutex.h"

using namespace std;

namespace cqhttp::plugins {
    static const auto TAG = u8"重启";

    using utils::mutex::with_unique_lock;

    void Restarter::hook_initialize(Context &ctx) {
        restart_worker_thread_ = thread([&]() {
            with_unique_lock(mutex_, [&] { restart_worker_running_ = true; });

            for (;;) {
                auto local_should_restart = false;
                auto local_should_stop_self = false;

                {
                    unique_lock<mutex> lock(mutex_);
                    cv_.wait(lock, [&] {
                        local_should_restart = should_restart_;
                        local_should_stop_self = !restart_worker_running_;
                        return local_should_restart
                               || local_should_stop_self; // stop waiting only if we should do restart or stop ourself
                    });
                }

                if (local_should_stop_self) {
                    break;
                }

                if (local_should_restart) {
                    chrono::milliseconds local_restart_delay;
                    with_unique_lock(mutex_, [&] {
                        local_restart_delay = restart_delay_;
                        should_restart_ = false;
                    });
                    if (local_restart_delay.count() > 0) {
                        logging::info(
                            TAG, u8"HTTP API 插件将在 " + to_string(local_restart_delay.count()) + u8" 毫秒后重启");
                    }
                    this_thread::sleep_for(local_restart_delay);
                    app.on_disable();
                    app.on_enable();
                    logging::info(TAG, u8"HTTP API 插件重启成功");
                }
            }
        });

        ctx.next();
    }

    void Restarter::hook_coolq_exit(Context &ctx) {
        // notify the restart worker stop itself
        with_unique_lock(mutex_, [&] { restart_worker_running_ = false; });
        cv_.notify_all();

        if (restart_worker_thread_.joinable()) {
            restart_worker_thread_.join();
        }
    }

    void Restarter::hook_missed_action(ActionContext &ctx) {
        if (ctx.action != "set_restart_plugin") {
            ctx.next();
            return;
        }

        const auto delay = ctx.params.get_integer("delay", 0);

        // notify the restart worker do restart
        with_unique_lock(mutex_, [&] {
            restart_delay_ = chrono::milliseconds(delay);
            should_restart_ = true;
        });
        cv_.notify_all();

        ctx.result.code = ActionResult::Codes::ASYNC;
    }
} // namespace cqhttp::plugins
