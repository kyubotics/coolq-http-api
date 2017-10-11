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

//#include "loader.h"

#include "app.h"

#include <fstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include "helpers.h"
//#include "Message.h"
#include "config_class.h"

using namespace std;

optional<Config> load_configuration(const string &filepath) {
    Config config;

    const auto ansi_filepath = ansi(filepath);
    if (!isfile(filepath)) {
        // first init, create default config
        Log::i(u8"配置", u8"没有找到配置文件，写入默认配置");
        ofstream file(ansi_filepath);
        if (file.is_open()) {
            file << "[general]" << endl
                    << "host=0.0.0.0" << endl
                    << "port=5700" << endl
                    << "post_url=" << endl
                    << "post_timeout=20" << endl
                    << "token=" << endl
                    << "post_message_format=string" << endl
                    << "serve_data_file=no" << endl
                    << "auto_check_update=no" << endl;
            file.close();
        }
    } else {
        // load from config file
        try {
            boost::property_tree::ptree pt;
            read_ini(ansi_filepath, pt);
            const auto login_qq_str = to_string(sdk->get_login_qq());
            #define GET_CONFIG(key, type) \
                config.key = pt.get<type>(login_qq_str + "." #key, config.key); \
                Log::d(u8"配置", #key ": " + to_string(config.key))
            GET_CONFIG(host, string);
            GET_CONFIG(port, int);
            GET_CONFIG(post_url, string);
            GET_CONFIG(post_timeout, long);
            GET_CONFIG(token, string);
            GET_CONFIG(post_message_format, string);
            GET_CONFIG(serve_data_file, bool);
            GET_CONFIG(auto_check_update, bool);
            #undef GET_CONFIG
        } catch (...) {
            // failed to load configurations
            return nullopt;
        }
    }

    return config;
}
