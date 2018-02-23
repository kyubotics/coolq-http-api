#include "./dir.h"

#include <boost/filesystem.hpp>

#include "./app.h"
#include "./api.h"
#include "./utils/string.h"

using namespace std;
namespace fs = boost::filesystem;

namespace cq::dir {
    string root() {
        const auto app_dir = app();
        const auto suffix = string("app\\" + app::id + "\\");
        return app_dir.substr(0, app_dir.length() - suffix.length());
    }

    string app() {
        return api::get_app_directory();
    }

    string tmp() {
        const auto tmpdir = app() + "\\tmp\\";
        const auto ansi_tmpdir = utils::ansi(tmpdir);
        if (!fs::exists(ansi_tmpdir)) {
            fs::create_directories(ansi_tmpdir);
        }
        return tmpdir;
    }
}
