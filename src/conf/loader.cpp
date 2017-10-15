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
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/filesystem.hpp>

#include "helpers.h"
#include "config_struct.h"

using namespace std;

optional<Config> load_configuration(const string &filepath) {
    static const auto TAG = u8"配置";

    Config config;

    Log::d(TAG, u8"尝试加载配置文件");

    const auto login_qq_str = to_string(sdk->get_login_qq());

    const auto ansi_filepath = ansi(filepath);
    if (!boost::filesystem::is_regular_file(ansi_filepath)) {
        // create default config file
        Log::i(TAG, u8"没有找到配置文件，写入默认配置");
        if (ofstream file(ansi_filepath); file.is_open()) {
            file << "[" << login_qq_str << "]" << endl
                    << "host=0.0.0.0" << endl
                    << "port=5700" << endl
                    << "post_url=" << endl
                    << "token=" << endl
                    << "post_message_format=string" << endl
                    << "serve_data_files=no" << endl
                    << "auto_check_update=no" << endl
                    << "thread_pool_size=4" << endl;
            file.close();
        } else {
            Log::e(TAG, u8"写入默认配置失败，请检查文件系统权限");
        }
    }

    // load from config file
    try {
        boost::property_tree::ptree pt;
        read_ini(ansi_filepath, pt);

        #define GET_CONFIG(key, type) \
            config.key = pt.get<type>(login_qq_str + "." #key, config.key); \
            Log::d(TAG, #key "=" + to_string(config.key))
        GET_CONFIG(host, string);
        GET_CONFIG(port, unsigned short);
        GET_CONFIG(post_url, string);
        GET_CONFIG(token, string);
        GET_CONFIG(post_message_format, string);
        GET_CONFIG(serve_data_files, bool);
        GET_CONFIG(auto_check_update, bool);
        GET_CONFIG(thread_pool_size, int);
        #undef GET_CONFIG

        Log::i(TAG, u8"加载配置文件成功");
    } catch (...) {
        // failed to load configurations
        Log::e(TAG, u8"加载配置文件失败，请检查配置文件格式和访问权限");
        return nullopt;
    }

    return config;
}
