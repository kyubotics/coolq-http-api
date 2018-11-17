#include "./dir.h"

#include <filesystem>

#include "./api.h"
#include "./app.h"
#include "./utils/string.h"

using namespace std;
namespace fs = std::filesystem;

namespace cq::dir {
    static void create_dir_if_not_exists(const string &dir) {
        const auto ansi_dir = utils::ansi(dir);
        if (!fs::exists(ansi_dir)) {
            fs::create_directories(ansi_dir);
        }
    }

    string root() {
        constexpr size_t size = 1024;
        wchar_t w_exec_path[size]{};
        GetModuleFileNameW(nullptr, w_exec_path, size); // this will get "C:\\Some\\Path\\CQA\\CQA.exe"
        auto exec_path = utils::ws2s(w_exec_path);
        return exec_path.substr(0, exec_path.rfind("\\")) + "\\";
    }

    string app(const std::string &sub_dir_name) {
        if (sub_dir_name.empty()) {
            return api::get_app_directory();
        }
        const auto dir = api::get_app_directory() + (sub_dir_name.empty() ? "" : sub_dir_name + "\\");
        create_dir_if_not_exists(dir);
        return dir;
    }

    std::string app_per_account(const std::string &sub_dir_name) {
        const auto dir = app(sub_dir_name) + to_string(api::get_login_user_id()) + "\\";
        create_dir_if_not_exists(dir);
        return dir;
    }
} // namespace cq::dir
