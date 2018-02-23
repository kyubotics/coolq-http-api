// 
// appentry.cpp : Export app entry functions to DLL.
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

///**
// * Return app info.
// */
//CQEVENT(const char *, AppInfo, 0)
//() {
//    // CoolQ API version: 9
//    return "9," CQAPP_ID;
//}
//
///**
// * Initialize SDK using the auth code given by CoolQ.
// */
//CQEVENT(int32_t, Initialize, 4)
//(const int32_t auth_code) {
//    app.initialize(auth_code);
//    return 0;
//}
//
///**
// * Event: Plugin is enabled.
// */
//CQEVENT(int32_t, Enable, 0)
//() {
//    app.enable();
//    if (config.auto_check_update) {
//        pool->push([](int) {
//            check_update(true);
//        });
//    }
//    return 0;
//}
//
///**
// * Event: Plugin is disabled.
// */
//CQEVENT(int32_t, Disable, 0)
//() {
//    app.disable();
//    return 0;
//}
//
///**
// * Event: CoolQ is starting.
// */
//CQEVENT(int32_t, Start, 0)
//() {
//    return 0;
//}
//
///**
// * Event: CoolQ is exiting.
// */
//CQEVENT(int32_t, Exit, 0)
//() {
//    app.exit();
//    return 0;
//}
