#include "./logging.h"

#include <map>

#include "cqhttp/logging/handler.h"
#include "cqhttp/logging/handlers/default.h"

using namespace std;

namespace cqhttp::logging {
    using cq::logging::Level;

    static int level = Level::INFO;
    static bool disable_coolq_log = true;

    static map<string, shared_ptr<Handler>> handlers = {
        {"default", make_shared<DefaultHandler>()},
    };

    void register_handler(const string &name, const std::shared_ptr<Handler> &handler) {
        if (handler) {
            handlers[name] = handler;
        }
    }

    shared_ptr<Handler> unregister_handler(const std::string &name) {
        if (const auto it = handlers.find(name); it != handlers.end()) {
            auto handler = it->second;
            handlers.erase(name);
            return handler;
        }
        return nullptr;
    }

    void set_level(const Level level) { logging::level = static_cast<int>(level) / 10 * 10; }

    void set_disable_coolq_log(const bool disable) { disable_coolq_log = disable; }

    void log(const Level level, const std::string &tag, const std::string &msg) {
        if (level / 10 * 10 >= logging::level) {
            for (const auto & [_, handler] : handlers) {
                handler->log(level, tag, msg);
            }
        }

        if (!disable_coolq_log && level >= Level::INFO && handlers.count("default") == 0) {
            // log info level and above to CoolQ
            cq::logging::log(level, tag, msg);
        }
    }
} // namespace cqhttp::logging
