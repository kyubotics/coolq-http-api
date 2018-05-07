#include "./file.h"

#include <spdlog/spdlog.h>

namespace cqhttp::logging {
    FileHandler::FileHandler() {
        logger_ = spdlog::stdout_color_mt("console");
    }

    void FileHandler::log(const cq::logging::Level level, const std::string &tag, const std::string &msg) const {
        logger_->info(msg);
    }
} // namespace cqhttp::logging
