#include "./worker_pool_resizer.h"

#include "cqhttp/core/core.h"

using namespace std;

namespace cqhttp::plugins {
    static const auto TAG = u8"线程池";

    void WorkerPoolResizer::hook_enable(Context &ctx) {
        const auto pool_size = ctx.config->get_integer("thread_pool_size", 4);
        if (!app.resize_worker_thread_pool(pool_size > 0 ? pool_size : thread::hardware_concurrency() * 2 + 1)) {
            logging::warning(TAG, u8"调整全局线程池大小失败");
        }
        ctx.next();
    }
} // namespace cqhttp::plugins
