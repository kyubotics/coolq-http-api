#include "./sdk_class.h"

#include "app.h"

#include <boost/filesystem.hpp>

using namespace std;
using namespace boost::filesystem;

string Sdk::Directories::app() const {
    return string_decode(CQ_getAppDirectory(this->ac_), Encodings::ANSI);
}

string Sdk::Directories::app_tmp() const {
    const auto tmpdir = app() + "\\tmp\\";
    const auto ansi_tmpdir = ansi(tmpdir);
    if (!exists(ansi_tmpdir)) {
        create_directories(ansi_tmpdir);
    }
    return tmpdir;
}

string Sdk::Directories::coolq() const {
    const auto app_dir = app();
    const auto suffix = string("app\\" CQAPP_ID "\\");
    return app_dir.substr(0, app_dir.length() - suffix.length());
}
