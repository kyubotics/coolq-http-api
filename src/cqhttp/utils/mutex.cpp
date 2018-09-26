#include "./mutex.h"

#include <filesystem>
#include <fstream>

using namespace std;
namespace fs = std::filesystem;

namespace cqhttp::utils::mutex {
    static bool acquire(string ansi_file_path) {
        ofstream f(ansi_file_path);
        return f.is_open();
    }

    void with_file_lock(string file_path, const function<void()> func) {
        const auto ansi_file_path = ansi(file_path);
        while (fs::exists(ansi_file_path)) {
            this_thread::sleep_for(50ms);
        }

        if (acquire(ansi_file_path)) {
            func();
            fs::remove(ansi_file_path);
        } else {
            throw runtime_error("failed to acquire file lock");
        }
    }
} // namespace cqhttp::utils::mutex
