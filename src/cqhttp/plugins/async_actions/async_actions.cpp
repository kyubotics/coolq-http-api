#include "./async_actions.h"

#include "cqhttp/core/core.h"

using namespace std;

namespace cqhttp::plugins {
    static const auto TAG = u8"异步动作";

    void AsyncActions::hook_missed_action(ActionContext &ctx) {
        if (boost::algorithm::ends_with(ctx.action, "_async")) {
            const auto action = ctx.action.substr(0, ctx.action.length() - strlen("_async"));
            const auto ok = app.push_async_task([action, params = ctx.params] {
                call_action(action, params);
                logging::debug(TAG, u8"成功执行一个异步动作");
            });
            if (ok) {
                logging::debug(TAG, u8"异步动作已进入全局线程池等待执行");
                ctx.result.code = ActionResult::Codes::ASYNC;
            } else {
                logging::debug(TAG, u8"全局线程池无法执行异步动作，请尝试重启插件");
                ctx.result.code = ActionResult::Codes::BAD_THREAD_POOL;
            }
        }

        ctx.next();
    }
} // namespace cqhttp::plugins
