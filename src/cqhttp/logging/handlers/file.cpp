#include "./file.h"

#include "cqsdk/utils/string.h"

using namespace std;

namespace cqhttp::logging {
    static const auto LOGGER_NAME = "file";

    void FileHandler::init() {
        logger_ = spdlog::rotating_logger_mt(LOGGER_NAME, cq::utils::ansi(filename_), max_file_size_, max_files_);
        logger_->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%L] %v");
        logger_->flush_on(spdlog::level::trace);
        logger_->set_level(spdlog::level::debug);
    }

    void FileHandler::destroy() { spdlog::drop(LOGGER_NAME); }
} // namespace cqhttp::logging
