#include "./application.h"

using namespace std;

namespace cqhttp {
    static const auto TAG = u8"核心";

    void Application::on_initialize() {
        initialized_ = true;
        iterate_hooks(&Plugin::hook_initialize, Context());
    }

    void Application::on_enable() {
        logging::info(TAG, u8"插件版本：" CQHTTP_VERSION);

        enabled_ = true;
        config_ = utils::JsonEx();
        worker_thread_pool_ = make_shared<ctpl::thread_pool>(1);
        logging::debug(TAG, u8"全局线程池创建成功");

        iterate_hooks(&Plugin::hook_enable, Context());
        emit_lifecycle_meta_event(MetaEvent::LIFECYCLE_ENABLE);
    }

    void Application::on_disable() {
        emit_lifecycle_meta_event(MetaEvent::LIFECYCLE_DISABLE);

        enabled_ = false;
        if (worker_thread_pool_) {
            worker_thread_pool_->stop();
            worker_thread_pool_ = nullptr;
            logging::debug(TAG, u8"全局线程池关闭成功");
        }

        iterate_hooks(&Plugin::hook_disable, Context());
    }

    void Application::on_coolq_start() { iterate_hooks(&Plugin::hook_coolq_start, Context()); }

    void Application::on_coolq_exit() {
        if (enabled_) {
            // generate a fake "disable" event at exit
            // this leads to a lifecycle change, check plugin.h for the lifecycle graph
            on_disable();
        }
        iterate_hooks(&Plugin::hook_coolq_exit, Context());
    }

    bool Application::resize_worker_thread_pool(const int n_threads) const {
        if (!worker_thread_pool_) {
            return false;
        }
        worker_thread_pool_->resize(n_threads);
        return true;
    }

    bool Application::push_async_task(const std::function<void()> &task) const {
        if (!worker_thread_pool_) {
            return false;
        }
        worker_thread_pool_->push([=](int) { task(); });
        return true;
    }
} // namespace cqhttp
