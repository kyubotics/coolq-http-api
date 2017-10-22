// 
// main_handlers.cpp : Implement additional API handlers.
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

#include "./handler.h"
#include "utils/params_class.h"

using namespace std;

HANDLER(_get_friend_list) {
    const auto cookies = sdk->get_cookies();
    const auto g_tk = to_string(sdk->get_csrf_token());
    const auto login_qq = to_string(sdk->get_login_qq());

    {
        // try mobile QZone API
        const auto url = "http://m.qzone.com/friend/mfriend_list?g_tk=" + g_tk + "&res_uin=" + login_qq +
                "&res_type=normal&format=json";
        const auto res = get_remote_json(url, true, cookies).value_or(nullptr);
        try {
            if (res.at("code").get<int>() == 0) {
                // succeeded
                auto resp_data = res.at("data");
                result.data = json::array();

                map<int64_t, int> gpid_idx_map;
                for (auto gp : resp_data.at("gpnames")) {
                    auto res_gp = json::object();
                    auto gpid = gp.at("gpid").get<int64_t>();
                    res_gp["friend_group_id"] = gpid;
                    res_gp["friend_group_name"] = gp.at("gpname").get<string>();
                    res_gp["friends"] = json::array();
                    gpid_idx_map[gpid] = result.data.size();
                    result.data.push_back(res_gp);
                }

                for (auto frnd : resp_data.at("list")) {
                    auto gpid = frnd.at("groupid").get<int64_t>();
                    auto res_frnd = json::object();
                    res_frnd["user_id"] = frnd.at("uin").get<int64_t>();
                    res_frnd["nickname"] = frnd.at("nick").get<string>();
                    res_frnd["remark"] = frnd.at("remark").get<string>();
                    result.data[gpid_idx_map[gpid]]["friends"].push_back(res_frnd);
                }

                result.retcode = RetCodes::OK;
                return;
            }
        } catch (exception &) { }
    }

    {
        // try desktop web QZone API
        const auto url =
                "https://h5.qzone.qq.com/proxy/domain/r.qzone.qq.com/cgi-bin/tfriend/friend_show_qqfriends.cgi?g_tk=" +
                g_tk + "&uin=" + login_qq;
        const auto res = get_remote_json(url, true, cookies).value_or(nullptr);
        try {
            auto resp_data = res;
            result.data = json::array();

            map<int64_t, int> gpid_idx_map;
            for (auto gp : resp_data.at("gpnames")) {
                auto res_gp = json::object();
                auto gpid = gp.at("gpid").get<int64_t>();
                res_gp["friend_group_id"] = gpid;
                res_gp["friend_group_name"] = gp.at("gpname").get<string>();
                res_gp["friends"] = json::array();
                gpid_idx_map[gpid] = result.data.size();
                result.data.push_back(res_gp);
            }

            for (auto frnd : resp_data.at("items")) {
                auto gpid = frnd.at("groupid").get<int64_t>();
                auto res_frnd = json::object();
                res_frnd["user_id"] = frnd.at("uin").get<int64_t>();
                res_frnd["nickname"] = frnd.at("name").get<string>();
                res_frnd["remark"] = frnd.at("remark").get<string>();
                result.data[gpid_idx_map[gpid]]["friends"].push_back(res_frnd);
            }

            result.retcode = RetCodes::OK;
            return;
        } catch (exception &) {}
    }

    // failed
    result.data = nullptr;
}
