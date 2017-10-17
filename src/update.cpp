// 
// update.cpp : Implement update functions.
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

#include "./update.h"

#include "app.h"

#include <boost/filesystem.hpp>

using namespace std;
using boost::algorithm::ends_with;

static string update_source() {
    if (ends_with(config.update_source, "/")) {
        return config.update_source;
    }
    return config.update_source + "/";
}

static string latest_url() {
    return update_source() + "latest/" + config.update_channel + ".json";
}

static string version_info_url(const string &version, const int build_number) {
    return update_source() + "versions/" + version + "(b" + to_string(build_number) + ")/info.json";
}

static string version_cpk_url(const string &version, const int build_number) {
    return update_source() + "versions/" + version + "(b" + to_string(build_number) + ")/" CQAPP_ID ".cpk";
}

// return tuple<is_newer, version, build_number, description>
optional<tuple<bool, string, int, string>> get_latest_version() {
    const auto data_opt = get_remote_json(latest_url());
    if (!data_opt) {
        return nullopt;
    }

    auto data = data_opt.value();
    if (data.is_object()
        && data.find("version") != data.end() && data["version"].is_string()
        && data.find("build") != data.end() && data["build"].is_number_integer()) {
        auto version = data["version"].get<string>();
        auto build_number = data["build"].get<int>();
        string description;
        if (const auto info = get_remote_json(version_info_url(version, build_number)).value_or(nullptr);
            info.is_object() && info.find("description") != info.end() && info["description"].is_string()) {
            description = info["description"].get<string>();
        }

        auto result = make_tuple(false, version, build_number, description);
        if (data["build"].get<int>() > CQAPP_BUILD_NUMBER) {
            get<0>(result) = true;
        }
        return result;
    }
    return nullopt;
}

bool perform_update(const string &version, const int build_number) {
    const auto cpk_url = version_cpk_url(version, build_number);
    const auto tmp_path = sdk->directories().app_tmp() + version + "_build_" + to_string(build_number) + ".cpk";
    if (!download_remote_file(cpk_url, tmp_path, true)) {
        // download failed
        return false;
    }

    const auto local_cpk_path = sdk->directories().coolq() + "app\\" CQAPP_ID ".cpk";
    try {
        copy_file(ansi(tmp_path), ansi(local_cpk_path), boost::filesystem::copy_option::overwrite_if_exists);
        boost::filesystem::remove(ansi(tmp_path));
        return true;
    } catch (boost::filesystem::filesystem_error &) {
        return false;
    }
}

void check_update(const bool is_automatically) {
    static const auto TAG = u8"检查更新";

    if (is_automatically) Log::i(TAG, u8"正在检查更新...");

    if (const auto version_opt = get_latest_version(); version_opt) {
        bool is_newer;
        string version, description;
        int build_number;
        tie(is_newer, version, build_number, description) = version_opt.value();

        if (is_newer) {
            // should update
            if (is_automatically) Log::i(TAG, u8"发现新版本：" + version + u8", build " + to_string(build_number));
            const auto code = message_box(MB_YESNO | MB_ICONQUESTION, u8"发现新版本：" + version
                                          + u8"\r\n\r\n更新信息：\r\n"
                                          + (description.empty() ? u8"无" : description)
                                          + u8"\r\n\r\n是否现在更新？");
            if (code == IDYES) {
                if (perform_update(version, build_number)) {
                    message_box(MB_OK | MB_ICONINFORMATION, u8"更新成功，请重启酷 Q 以生效。");
                } else {
                    message_box(MB_OK | MB_ICONERROR, u8"更新失败，请检查网络连接是否通畅，或尝试更换更新源。");
                }
            }
        } else {
            if (is_automatically) Log::i(TAG, u8"没有发现更新版本");
            else message_box(MB_OK | MB_ICONINFORMATION, u8"没有发现更新版本。");
        }
    } else {
        if (is_automatically) Log::e(TAG, u8"检查更新失败，请检查网络连接是否通畅，或尝试更换更新源");
        else message_box(MB_OK | MB_ICONERROR, u8"检查更新失败，请检查网络连接是否通畅，或尝试更换更新源。");
    }
}
