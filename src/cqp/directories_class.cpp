#include "./sdk_class.h"

#include "app.h"

#include "utils/filesystem.h"

using namespace std;
namespace fs = filesystem;

string Sdk::Directories::app() const {
    return string_from_coolq(CQ_getAppDirectory(this->ac_));
}

string Sdk::Directories::app_tmp() const {
    const auto tmpdir = app() + "\\tmp\\";
    const auto ansi_tmpdir = ansi(tmpdir);
    if (!fs::exists(ansi_tmpdir)) {
        fs::create_directories(ansi_tmpdir);
    }
    return tmpdir;
}

string Sdk::Directories::coolq() const {
    const auto app_dir = app();
    const auto suffix = string("app\\" CQAPP_ID "\\");
    return app_dir.substr(0, app_dir.length() - suffix.length());
}
