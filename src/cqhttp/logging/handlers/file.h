#pragma once

#include "cqhttp/logging/handlers/_spdlog.h"

namespace cqhttp::logging {
    struct FileHandler : SpdlogHandler {
        FileHandler(const std::string &filename, const size_t max_file_size, const size_t max_files)
            : filename_(filename), max_file_size_(max_file_size), max_files_(max_files) {}
        void init() override;
        void destroy() override;

    private:
        std::string filename_;
        size_t max_file_size_;
        size_t max_files_;
    };
} // namespace cqhttp::logging
