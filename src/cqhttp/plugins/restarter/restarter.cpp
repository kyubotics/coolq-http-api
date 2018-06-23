#include "./restarter.h"

#include <Windows.h>

#include "cqhttp/core/core.h"

using namespace std;

namespace cqhttp::plugins {
    static const auto TAG = u8"重启";

    void Restarter::hook_initialize(Context &ctx) {
        restart_worker_running_ = true;
        restart_worker_thread_ = thread([&]() {
            while (restart_worker_running_) {
                if (should_restart_) {
                    should_restart_ = false;
                    if (restart_delay_ > 0) {
                        logging::info(TAG, u8"HTTP API 插件将在 " + to_string(restart_delay_) + u8" 毫秒后重启");
                    }
                    Sleep(restart_delay_);
                    app.on_disable();
                    app.on_enable();
                    logging::info(TAG, u8"HTTP API 插件重启成功");
                }
                Sleep(300); // wait 300 ms for the next check
            }
        });

        ctx.next();
    }

    void Restarter::hook_coolq_exit(Context &ctx) {
        restart_worker_running_ = false;
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
        restart_delay_ = delay;
        should_restart_ = true; // this will let the restart worker do it
        ctx.result.code = ActionResult::Codes::ASYNC;
    }
} // namespace cqhttp::plugins
