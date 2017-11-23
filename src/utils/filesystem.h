#pragma once

#include "common.h"

namespace filesystem {
    class filesystem_error : public std::system_error {
    public:
        explicit filesystem_error(const std::string &message,
                                  const std::error_code err_code =
                                          make_error_code(std::errc::operation_not_permitted)) :
            system_error(err_code, message) { }

        filesystem_error(const filesystem_error &) = default;
        filesystem_error &operator=(const filesystem_error &) = default;
    };

    bool exists(const std::string &ansi_path);
    bool is_directory(const std::string &ansi_path);
    bool is_regular_file(const std::string &ansi_path);
    uintmax_t file_size(const std::string &ansi_path);
    bool remove(const std::string &ansi_path);
    bool create_directories(const std::string &ansi_path);
    bool copy_file(const std::string &ansi_old_path, const std::string &ansi_new_path,
                   const bool overwrite_existing = false);
}
