#include "./logging.h"

#include "cqhttp/logging/handler.h"
#include "cqhttp/logging/handlers/default.h"
#include "cqhttp/logging/handlers/file.h"

using namespace std;

namespace cqhttp::logging {
    using cq::logging::Level;

    static int level = Level::DEBUG;

    static vector<shared_ptr<Handler>> handlers = {
        make_shared<DefaultHandler>(),
    };

    void init(const Level level) {
        handlers = {
            make_shared<DefaultHandler>(),
            make_shared<FileHandler>(),
        };

        for (auto &handler : handlers) {
            handler->init();
        }

        set_level(level);
    }

    void destroy() {
        for (auto &handler : handlers) {
            handler->destroy();
        }
    }

    void set_level(const Level level) { logging::level = static_cast<int>(level) / 10 * 10; }

    void log(const Level level, const std::string &tag, const std::string &msg) {
        if (level / 10 * 10 >= logging::level) {
            for (const auto &handler : handlers) {
                handler->log(level, tag, msg);
            }
        }
    }
} // namespace cqhttp::logging
