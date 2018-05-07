#include "./logging.h"

#include <map>

#include "cqhttp/logging/handler.h"
#include "cqhttp/logging/handlers/default.h"
#include "cqhttp/logging/handlers/file.h"

using namespace std;

namespace cqhttp::logging {
    void log(const Level level, const std::string &tag, const std::string &msg) {
        static map<string, shared_ptr<IHandler>> handlers = {
            {"default", make_shared<DefaultHandler>()},
            {"file", make_shared<FileHandler>()},
        };

        for (const auto &[name, handler] : handlers) {
            handler->log(level, tag, msg);
        }
    }
} // namespace cqhttp::logging
