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
    pool->push([](int) {
        check_update(false);
    });
    return 0;
}
