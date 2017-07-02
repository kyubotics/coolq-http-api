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
    config.post_timeout = 20;
    config.token = "";
    config.pattern = "";
    config.post_message_format = MSG_FMT_STRING;
    config.serve_data_file = false;
    config.auto_check_update = false;

    auto ansi_filepath = ansi(filepath);
    FILE *conf_file = nullptr;
    fopen_s(&conf_file, ansi_filepath.c_str(), "r");
    if (!conf_file) {
        // first init, save default config
        L.i("配置", "没有找到配置文件，写入默认配置");
        ofstream file(ansi_filepath);
        if (file.is_open()) {
            file << "[general]" << endl
                    << "host=0.0.0.0" << endl
                    << "port=5700" << endl
                    << "post_url=" << endl
                    << "post_timeout=20" << endl
                    << "token=" << endl
                    << "pattern=" << endl
                    << "post_message_format=string" << endl
                    << "serve_data_file=no" << endl
                    << "auto_check_update=no" << endl;
            file.close();
        }
    } else {
        // load from config file
        auto callback = [&](const str &section, const str &name, const str &value) {
                    static auto login_qq_str = str(CQ->get_login_qq());
                    if (section == "general" || isnumber(section) && login_qq_str == section) {
                        if (name == "host") {
                            config.host = value;
                        } else if (name == "port") {
                            config.port = int(value);
                        } else if (name == "post_url") {
                            config.post_url = value;
                        } else if (name == "post_timeout") {
                            if (value) {
                                config.post_timeout = long(long long(value));
                            }
                        } else if (name == "token") {
                            config.token = value;
                        } else if (name == "pattern") {
                            config.pattern = regex(value.to_bytes());
                        } else if (name == "post_message_format") {
                            config.post_message_format = value;
                        } else if (name == "serve_data_file") {
                            auto v = value.lower();
                            if (v == "yes" || v == "true" || v == "1") {
                                config.serve_data_file = true;
                            }
                        } else if (name == "auto_check_update") {
                            auto v = value.lower();
                            if (v == "yes" || v == "true" || v == "1") {
                                config.auto_check_update = true;
                            }
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

    #define PRINT_CONFIG(key) L.d("配置", #key ": " + str(CQ->config.key))
    PRINT_CONFIG(host);
    PRINT_CONFIG(port);
    PRINT_CONFIG(post_url);
    PRINT_CONFIG(post_timeout);
    PRINT_CONFIG(token);
    PRINT_CONFIG(post_message_format);
    PRINT_CONFIG(serve_data_file);
    PRINT_CONFIG(auto_check_update);

    return true;
}
