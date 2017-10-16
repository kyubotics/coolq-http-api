// 
// menuentry.cpp : Export menu entry functions to DLL.
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

#include "app.h"

#include "update.h"

using namespace std;

/**
 * Menu: Reload.
 */
CQEVENT(int32_t, __menu_reload, 0)() {
    app.disable();
    app.enable();
    Log::i(u8"重新加载", u8"HTTP API 重新加载成功");
    return 0;
}

/**
 * Menu: Check update.
 */
CQEVENT(int32_t, __menu_check_update, 0)() {
    if (const auto version_opt = check_update(); version_opt) {
        auto latest_version = version_opt.value();
        if (get<0>(latest_version)) {
            // should update
            auto description = get<3>(latest_version);
            const auto code = message_box(MB_OKCANCEL, u8"发现新版本：v" + get<1>(latest_version)
                                          + u8"\r\n\r\n更新信息：\r\n"
                                          + (description.empty() ? u8"无" : description)
                                          + u8"\r\n\r\n是否现在更新？");
            if (code == IDYES) {
                // do update
            }
        } else {
            message_box(MB_OK, u8"没有发现更新版本。");
        }
    } else {
        message_box(MB_OK, u8"检查更新失败，请检查网络连接是否通畅，或尝试更换更新源。");
    }

    return 0;
}
