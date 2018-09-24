#include "./heartbeat_generator.h"

#include "cqhttp/core/core.h"

using namespace std;

namespace cqhttp::plugins {
    static const auto TAG = u8"心跳发生器";

    void HeartbeatGenerator::hook_enable(Context &ctx) {
        if (ctx.config->get_bool("enable_heartbeat", false)) {
            const auto heartbeat_interval =
                chrono::milliseconds(ctx.config->get_integer("heartbeat_interval", 15 * 1000));

            app.scheduler().interval(heartbeat_interval, [] {
                logging::debug(TAG, u8"扑通");
                const auto result = call_action("get_status");
                emit_heartbeat_meta_event(result.data);
            });
            logging::info_success(TAG, u8"心跳发生器启动成功");
        }

        ctx.next();
    }
} // namespace cqhttp::plugins
