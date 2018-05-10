#include "./dir.h"

#include <boost/filesystem.hpp>

#include "./api.h"
#include "./app.h"
#include "./utils/string.h"

using namespace std;
namespace fs = boost::filesystem;

namespace cq::dir {
    static void create_dir_if_not_exists(const string &dir) {
        const auto ansi_dir = utils::ansi(dir);
        if (!fs::exists(ansi_dir)) {
            fs::create_directories(ansi_dir);
        }
    }

    string root() {
        const auto app_dir = app();
        const auto suffix = string("app\\" + app::id + "\\");
        return app_dir.substr(0, app_dir.length() - suffix.length());
    }

    string app(const std::string &sub) {
        if (sub.empty()) {
            return api::get_app_directory();
        }
        const auto dir = api::get_app_directory() + (sub.empty() ? "" : sub + "\\");
        create_dir_if_not_exists(dir);
        return dir;
    }
} // namespace cq::dir
