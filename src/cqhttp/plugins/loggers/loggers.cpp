#include "./loggers.h"

#include "cqhttp/logging/handlers/console.h"
#include "cqhttp/logging/handlers/file.h"

using namespace std;

namespace cqhttp::plugins {
    static const auto TAG = u8"日志";

    void Loggers::hook_enable(Context &ctx) {
        cq::logging::info(TAG, u8"插件版本：" CQHTTP_VERSION);
        cq::logging::info(TAG, u8"已加载的配置：\n" + ctx.config->raw.dump(2));

        cq::logging::info_success(TAG, u8"日志系统初始化成功");
        cq::logging::info(TAG, u8"请在酷 Q 主目录的 app\\io.github.richardchien.coolqhttpapi\\log 中查看日志文件");

        if (ctx.config->get_bool("show_log_console", false)) {
            const auto handler = make_shared<logging::ConsoleHandler>();
            handler->init();
            register_handler("console", handler);
            logging::info_success(TAG, u8"日志控制台开启成功");
        }
        ctx.next();
    }

    void Loggers::hook_disable(Context &ctx) {
        ctx.next();
        if (auto handler = logging::unregister_handler("console")) {
            handler->destroy();
        }
    }

    void Loggers::hook_coolq_start(Context &ctx) {
        const auto handler = make_shared<logging::FileHandler>();
        handler->init();
        register_handler("file", handler);
        logging::unregister_handler("default"); // remove default logging handler
        ctx.next();
    }

    void Loggers::hook_coolq_exit(Context &ctx) {
        ctx.next();
        if (auto handler = logging::unregister_handler("file")) {
            handler->destroy();
        }
    }
} // namespace cqhttp::plugins
