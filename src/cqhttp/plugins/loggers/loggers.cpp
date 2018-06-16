#include "./loggers.h"

#include "cqhttp/logging/handlers/console.h"
#include "cqhttp/logging/handlers/file.h"

using namespace std;

namespace cqhttp::plugins {
    static const auto TAG = u8"日志";

    void Loggers::hook_enable(Context &ctx) {
        const auto file_handler =
            make_shared<logging::FileHandler>(cq::dir::app("log") + to_string(cq::api::get_login_user_id()) + ".log");
        file_handler->init();
        register_handler("file", file_handler);
        logging::unregister_handler("default"); // remove default logging handler

        if (ctx.config->get_bool("show_log_console", false)) {
            const auto console_handler = make_shared<logging::ConsoleHandler>();
            console_handler->init();
            register_handler("console", console_handler);
            logging::info_success(TAG, u8"日志控制台开启成功");
        }

        cq::logging::info_success(TAG, u8"日志系统初始化成功");
        cq::logging::info(TAG, u8"请在酷 Q 主目录的 app\\io.github.richardchien.coolqhttpapi\\log 中查看日志文件");
        ctx.next();
    }

    void Loggers::hook_disable(Context &ctx) {
        ctx.next();
        if (auto handler = logging::unregister_handler("console")) {
            handler->destroy();
        }
        if (auto handler = logging::unregister_handler("file")) {
            handler->destroy();
        }
    }

    void Loggers::hook_coolq_start(Context &ctx) { ctx.next(); }

    void Loggers::hook_coolq_exit(Context &ctx) { ctx.next(); }
} // namespace cqhttp::plugins
