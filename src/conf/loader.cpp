//
// loader.cpp : Provide function to load configuration.
//
// Copyright (C) 2017  Richard Chien <richardchienthebest@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include "loader.h"

#include "app.h"

#include <fstream>

#include "conf/ini.h"
#include "helpers.h"
#include "Message.h"

using namespace std;

bool load_configuration(const str &filepath, Config &config) {
    // set default values
    config.host = "0.0.0.0";
    config.port = 5700;
    config.post_url = "";
    config.token = "";
    config.pattern = "";
    config.post_message_format = MSG_FMT_STRING;

    FILE *conf_file = nullptr;
    fopen_s(&conf_file, filepath.c_str(), "r");
    if (!conf_file) {
        // first init, save default config
        L.d("配置", "没有找到配置文件，写入默认配置");
        ofstream file(filepath.c_str());
        file << "[general]" << endl
                << "host=0.0.0.0" << endl
                << "port=5700" << endl
                << "post_url=" << endl
                << "token=" << endl;
    } else {
        // load from config file
        auto callback = [&](const str &section, const str &name, const str &value) {
                    static auto login_qq_str = str(CQ->getLoginQQ());

                    if (section == "general" || isnumber(section) && login_qq_str == section) {
                        if (name == "host") {
                            config.host = value;
                        } else if (name == "port") {
                            config.port = int(value);
                        } else if (name == "post_url") {
                            config.post_url = value;
                        } else if (name == "token") {
                            config.token = value;
                        } else if (name == "pattern") {
                            config.pattern = regex(value.to_bytes());
                        } else if (name == "post_message_format") {
                            config.post_message_format = value;
                        }
                    }
                    return 1;
                };
        // pass the real lambda callback as the user data to allow capturing
        // see http://bannalia.blogspot.com/2016/07/passing-capturing-c-lambda-functions-as.html
        if (0 != ini_parse_file(conf_file, [](void *real_cb, const char *section, const char *name, const char *value) {
                                    return (*static_cast<decltype(callback)*>(real_cb))(section, name, value);
                                }, &callback)) {
            return false; // failed to parse
        }
        fclose(conf_file);
    }
    return true;
}
