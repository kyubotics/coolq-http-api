#include "./filesystem.h"

#include <fstream>
#include <iterator>

using namespace std;

bool filesystem::exists(const string &ansi_path) {
    return GetFileAttributesA(ansi_path.c_str()) != INVALID_FILE_ATTRIBUTES;
}

bool filesystem::is_directory(const string &ansi_path) {
    return exists(ansi_path) && (GetFileAttributesA(ansi_path.c_str()) & FILE_ATTRIBUTE_DIRECTORY);
}

bool filesystem::is_regular_file(const string &ansi_path) {
    return exists(ansi_path) && !is_directory(ansi_path);
}

uintmax_t filesystem::file_size(const string &ansi_path) {
    WIN32_FILE_ATTRIBUTE_DATA attrs;
    if (!GetFileAttributesExA(ansi_path.c_str(), GetFileExInfoStandard, &attrs)) {
        throw filesystem_error("failed to get file size");
    }
    LARGE_INTEGER size;
    size.HighPart = attrs.nFileSizeHigh;
    size.LowPart = attrs.nFileSizeLow;
    return size.QuadPart;
}

bool filesystem::remove(const string &ansi_path) {
    return DeleteFileA(ansi_path.c_str());
}

bool filesystem::create_directories(const string &ansi_path) {
    auto parent_dir_ok = true;
    if (const auto last_sep = ansi_path.find_last_of("\\"); last_sep != string::npos) {
        if (const auto parent_dir = ansi_path.substr(0, last_sep); !is_directory(parent_dir)) {
            parent_dir_ok = create_directories(parent_dir);
        }
    }

    if (!parent_dir_ok) return false;
    return CreateDirectoryA(ansi_path.c_str(), nullptr);
}

bool filesystem::copy_file(const string &ansi_old_path, const string &ansi_new_path, const bool overwrite_existing) {
    if (!is_regular_file(ansi_old_path)
        || is_regular_file(ansi_new_path) && !overwrite_existing) {
        return false;
    }

    ifstream is(ansi_old_path, ios::in | ios::binary);
    ofstream os(ansi_new_path, ios::out | ios::binary);

    const auto buffer = std::get_temporary_buffer<char>(8192);
    while (is.good() && os) {
        is.read(buffer.first, buffer.second);
        os.write(buffer.first, is.gcount());
    }

    return_temporary_buffer(buffer.first);

    if (os.fail()) return false;
    if (is.eof()) return true;
    return false;
}
