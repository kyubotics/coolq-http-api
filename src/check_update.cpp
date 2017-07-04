// 
// check_update.cpp : Implement functions to update CPK file.
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

#include "check_update.h"

#include "app.h"

#include <json.hpp>
#include <fstream>

#include "curl_wrapper.h"
#include "helpers.h"

using json = nlohmann::json;
using namespace std;

const static str CPK_REL_PATH = "app\\io.github.richardchien.coolqhttpapi.cpk";
const static str LATEST_URL = "https://api.github.com/repos/richardchien/coolq-http-api/releases/latest";
const static str DOWNLOAD_URL = "https://git.oschina.net/richardchien/coolq-http-api-cpk/raw/master/v{}/io.github.richardchien.coolqhttpapi.cpk";

static int MessageBox(UINT type, const str &text) {
    return MessageBoxW(nullptr,
                       text.to_wstring().c_str(),
                       L"CoolQ HTTP API",
                       type | MB_SETFOREGROUND | MB_TASKMODAL | MB_TOPMOST);
}

static void output(const str &text, bool manual) {
    if (manual) {
        MessageBox(MB_OK, text);
    }
    L.i("更新", text);
}

static void do_update(const str &version);

void check_update(bool manual) {
    if (!isfile(get_coolq_root() + CPK_REL_PATH)) {
        output("当前可能正在使用自行修改编译的 DLL，无法检查更新", manual);
        return;
    }

    L.i("更新", "正在检查更新……");
    auto resp = curl::Request(LATEST_URL,
                              curl::Headers{
                                  {"Accept", "application/vnd.github.v3+json"},
                                  {"User-Agent", CQ_APP_USER_AGENT}
                              }).get();
    if (resp.status_code != 200) {
        output("检查更新失败，请检查你的网络连接", manual);
        return;
    }

    json json = resp.json();
    auto latest_version = str(json["tag_name"].get<string>()).lstrip("v");
    L.d("更新", "当前最新版：" + latest_version);
    auto latest_semantic_version = latest_version.split(".");
    auto curr_semantic_version = str(CQ_APP_VERSION).split(regex("[\\s\\.-]+"));
    auto need_update = true;

    if (latest_semantic_version.size() != 3 || curr_semantic_version.size() < 3) {
        // something went wrong
        output("当前或最新版本号无法识别，检查更新失败", manual);
        return;
    }

    auto i = 0;
    for (; i < 3; i++) {
        if (int(latest_semantic_version[i]) < int(curr_semantic_version[i])) {
            need_update = false;
            break;
        }
        if (int(latest_semantic_version[i]) > int(curr_semantic_version[i])) {
            break;
        }
    }
    if (need_update == true && i == 3 && curr_semantic_version.size() == 3) {
        // current version is not something like 2.2.1-beta1
        need_update = false;
    }

    if (!need_update) {
        output("当前已是最新版本", manual);
        return;
    }

    L.i("更新", "检查到新版本：" + latest_version);
    auto code = MessageBox(MB_YESNO | MB_ICONQUESTION, "检查到新版本：" + latest_version + "，是否现在更新？");
    if (code == IDYES) {
        do_update(latest_version);
    } else {
        L.i("更新", "已选择取消更新插件");
    }
}

static void do_update(const str &version) {
    L.i("更新", "开始更新 CoolQ HTTP API 插件……");

    ofstream file(ansi(get_coolq_root() + CPK_REL_PATH), ios::binary);
    if (!file.is_open()) {
        MessageBox(MB_OK, "更新失败，无法写入文件");
        return;
    }

    auto req = curl::Request(DOWNLOAD_URL.format(version),
                             curl::Headers{{"User-Agent",
                                 "Mozilla/5.0 (Windows NT 10.0; Win64; x64) "
                                 "AppleWebKit/537.36 (KHTML, like Gecko) "
                                 "Chrome/56.0.2924.87 Safari/537.36"}});
    req.write_data = &file;
    req.write_func = [](char *buf, size_t size, size_t count, void *file) {
                *static_cast<ofstream *>(file) << string(buf, count);
                return size * count;
            };
    auto resp = req.get();
    file.close();

    if (resp.status_code != 200) {
        MessageBox(MB_OK, "更新失败，无法下载文件");
        return;
    }

    L.i("更新", "更新成功，将在重启酷 Q 后生效");
    MessageBoxW(MB_OK, L"已成功下载新版本，将在重启酷 Q 后生效。");
}
