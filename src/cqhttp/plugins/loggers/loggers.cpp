#include "./loggers.h"

#include <boost/filesystem.hpp>

#include "cqhttp/logging/handlers/console.h"
#include "cqhttp/logging/handlers/file.h"

using namespace std;
namespace fs = boost::filesystem;

namespace cqhttp::plugins {
    static const auto TAG = u8"日志";

    static std::string log_filename() { return cq::dir::app("log") + to_string(cq::api::get_login_user_id()) + ".log"; }

    static void create_file_logger() {
        const auto file_handler = make_shared<logging::FileHandler>(log_filename());
        file_handler->init();
        register_handler("file", file_handler);
    }

    static void remove_file_logger() {
        if (auto handler = logging::unregister_handler("file")) {
            handler->destroy();
        }
    }

    void Loggers::hook_enable(Context &ctx) {
        create_file_logger();
        logging::unregister_handler("default"); // remove default logging handler

        if (ctx.config->get_bool("show_log_console", false)) {
            const auto console_handler = make_shared<logging::ConsoleHandler>();
            console_handler->init();
            register_handler("console", console_handler);
            logging::info_success(TAG, u8"日志控制台开启成功");
        }

        cq::logging::info_success(TAG, u8"日志系统初始化成功");
        cq::logging::info(TAG, u8"请在酷 Q 主目录的 app\\io.github.richardchien.coolqhttpapi\\log\\ 中查看日志文件");
        ctx.next();
    }

    void Loggers::hook_disable(Context &ctx) {
        ctx.next();
        if (auto handler = logging::unregister_handler("console")) {
            handler->destroy();
        }
        remove_file_logger();
    }

    void Loggers::hook_missed_action(ActionContext &ctx) {
        if (ctx.action != "clean_plugin_log") {
            ctx.next();
            return;
        }

        remove_file_logger();
        fs::remove(ansi(log_filename()));
        create_file_logger();
        ctx.result.code = ActionResult::Codes::OK;
    }
} // namespace cqhttp::plugins
