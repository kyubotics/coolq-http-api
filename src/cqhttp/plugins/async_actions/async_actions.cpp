#include "./async_actions.h"

using namespace std;

namespace cqhttp::plugins {
    static const auto TAG = u8"异步动作";

    void AsyncActions::hook_enable(Context &ctx) {
        const auto pool_size = ctx.config->get_integer("thread_pool_size", 4);
        if (!pool_) {
            pool_ = make_shared<ctpl::thread_pool>(pool_size > 0 ? pool_size : thread::hardware_concurrency() * 2 + 1);
            logging::debug(TAG, u8"线程池创建成功");
        }
        ctx.next();
    }

    void AsyncActions::hook_disable(Context &ctx) {
        if (pool_) {
            pool_->stop();
            pool_ = nullptr;
            logging::debug(TAG, u8"线程池关闭成功");
        }
        ctx.next();
    }

    void AsyncActions::hook_missed_action(ActionContext &ctx) {
        if (boost::algorithm::ends_with(ctx.action, "_async")) {
            if (pool_) {
                const auto action = ctx.action.substr(0, ctx.action.length() - strlen("_async"));
                pool_->push([action, params = ctx.params](int) {
                    call_action(action, params);
                    logging::debug(TAG, u8"成功执行一个异步动作");
                });
                logging::debug(TAG, u8"异步动作已进入线程池等待执行");
                ctx.result.code = ActionResult::Codes::ASYNC;
            } else {
                logging::debug(TAG, u8"线程池未正确初始化，无法执行异步动作，请尝试重启插件");
                ctx.result.code = ActionResult::Codes::BAD_THREAD_POOL;
            }
        }

        ctx.next();
    }
} // namespace cqhttp::plugins
