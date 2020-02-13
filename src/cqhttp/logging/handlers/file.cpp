#include "./file.h"

#include <spdlog/sinks/rotating_file_sink.h>

#include "cqhttp/utils/filesystem.h"
#include "cqsdk/utils/string.h"

using namespace std;

namespace cqhttp::logging {
    static const auto LOGGER_NAME = "file";
    static const auto TAG = u8"日志";

    void FileHandler::init() {
        try {
            logger_ = spdlog::rotating_logger_mt(LOGGER_NAME, cq::utils::ansi(filename_), max_file_size_, max_files_);
            logger_->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%L] %v");
            logger_->flush_on(spdlog::level::trace);
            logger_->set_level(spdlog::level::debug);
        } catch (spdlog::spdlog_ex &) {
            logger_ = nullptr;
            cq::logging::error(TAG,
                               u8"文件日志初始化失败，请检查文件权限，或删除 酷Q 主目录的 "
                                   + utils::fs::app_dir_rel_path("log") + u8" 后重试");
        }
    }

    void FileHandler::destroy() { spdlog::drop(LOGGER_NAME); }

    void FileHandler::log(const cq::logging::Level level, const std::string &tag, const std::string &msg) const {
        if (logger_) logger_->log(convert_level(level), "[{}] {}", tag, msg);
    }
} // namespace cqhttp::logging
