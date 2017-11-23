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

#include "./loader.h"

#include "app.h"

#include <fstream>

#include "./config_struct.h"
#include "utils/ini_reader.h"
#include "utils/filesystem.h"

using namespace std;
namespace fs = filesystem;

static const auto DEFAULT_UPDATE_SOURCE =
        "https://raw.githubusercontent.com/richardchien/coolq-http-api-release/master/";

optional<Config> load_configuration(const string &filepath) {
    static const auto TAG = u8"配置";

    Config config;

    Log::d(TAG, u8"尝试加载配置文件");

    const auto ansi_filepath = ansi(filepath);
    if (!fs::is_regular_file(ansi_filepath)) {
        // create default config file
        Log::i(TAG, u8"没有找到配置文件，写入默认配置");
        if (ofstream file(ansi_filepath); file.is_open()) {
            file << "[general]" << endl
                    << "host=0.0.0.0" << endl
                    << "port=5700" << endl
                    << "use_http=yes" << endl
                    << "ws_host=0.0.0.0" << endl
                    << "ws_port=5700" << endl
                    << "use_ws=no" << endl
                    << "post_url=" << endl
                    << "access_token=" << endl
                    << "secret=" << endl
                    << "post_message_format=string" << endl
                    << "serve_data_files=no" << endl
                    << "update_source=" << DEFAULT_UPDATE_SOURCE << endl
                    << "update_channel=stable" << endl
                    << "auto_check_update=no" << endl
                    << "auto_perform_update=no" << endl
                    << "thread_pool_size=4" << endl
                    << "server_thread_pool_size=1" << endl;
        } else {
            Log::e(TAG, u8"写入默认配置失败，请检查文件系统权限");
        }
    }

    // load from config file
    try {
        const auto login_qq_str = to_string(sdk->get_login_qq());

        INIReader reader(ansi_filepath);
        if (const auto err = reader.ParseError(); err != 0) {
            throw err;
        }

        #define LOG(key) Log::d(TAG, #key "=" + to_string(config.key))
        #define GET_STR_CONFIG(key) \
            const auto __general_##key = reader.Get("general", #key, config.key); \
            config.key = reader.Get(login_qq_str, #key, __general_##key); \
            LOG(key)
        #define GET_INT_CONFIG(key, type) \
            const auto __general_##key = reader.Get("general", #key, to_string(config.key)); \
            config.key = static_cast<type>(stoll(reader.Get(login_qq_str, #key, __general_##key))); \
            LOG(key)
        #define GET_BOOL_CONFIG(key) \
            const auto __general_##key = reader.Get("general", #key, to_string(config.key)); \
            config.key = to_bool(reader.Get(login_qq_str, #key, __general_##key)).value_or(config.key); \
            LOG(key)

        GET_STR_CONFIG(host);
        GET_INT_CONFIG(port, unsigned short);
        GET_BOOL_CONFIG(use_http);
        GET_STR_CONFIG(ws_host);
        GET_INT_CONFIG(ws_port, unsigned short);
        GET_BOOL_CONFIG(use_ws);
        GET_STR_CONFIG(ws_reverse_api_url);
        GET_STR_CONFIG(ws_reverse_event_url);
        GET_INT_CONFIG(ws_reverse_reconnect_interval, unsigned long);
        GET_BOOL_CONFIG(use_ws_reverse);
        GET_STR_CONFIG(post_url);
        GET_STR_CONFIG(access_token);
        GET_STR_CONFIG(secret);
        GET_STR_CONFIG(post_message_format);
        GET_BOOL_CONFIG(serve_data_files);
        GET_STR_CONFIG(update_source);
        GET_STR_CONFIG(update_channel);
        GET_BOOL_CONFIG(auto_check_update);
        GET_BOOL_CONFIG(auto_perform_update);
        GET_INT_CONFIG(thread_pool_size, size_t);
        GET_INT_CONFIG(server_thread_pool_size, size_t);

        #undef LOG
        #undef GET_STR_CONFIG
        #undef GET_INT_CONFIG
        #undef GET_BOOL_CONFIG

        Log::i(TAG, u8"加载配置文件成功");
    } catch (...) {
        // failed to load configurations
        Log::e(TAG, u8"加载配置文件失败，请检查配置文件格式和访问权限");
        return nullopt;
    }

    return config;
}
