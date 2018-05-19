#include "./experimental_actions.h"

#include "cqhttp/utils/http.h"

using namespace std;
namespace api = cq::api;

namespace cqhttp::plugins {
    using Codes = ActionResult::Codes;

    static void action_get_friend_list(ActionContext &ctx) {
        auto &result = ctx.result;

        try {
            const auto cookies = api::get_cookies();
            const auto g_tk = to_string(api::get_csrf_token());
            const auto login_qq = to_string(api::get_login_user_id());

            {
                // try mobile QZone API
                const auto url = "http://m.qzone.com/friend/mfriend_list?g_tk=" + g_tk + "&res_uin=" + login_qq
                                 + "&res_type=normal&format=json";
                const auto res = utils::http::get_json(url, true, cookies).value_or(nullptr);
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

                        result.code = Codes::OK;
                        return;
                    }
                } catch (exception &) {
                }
            }

            {
                // try desktop web QZone API
                const auto url =
                    "https://h5.qzone.qq.com/proxy/domain/r.qzone.qq.com/cgi-bin/tfriend/"
                    "friend_show_qqfriends.cgi?g_tk="
                    + g_tk + "&uin=" + login_qq;
                const auto res = utils::http::get_json(url, true, cookies).value_or(nullptr);
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

                    result.code = Codes::OK;
                    return;
                } catch (exception &) {
                }
            }

            // failed
            result.data = nullptr;
            result.code = Codes::DEFAULT_ERROR;
        } catch (cq::exception::ApiError &) {
            result.code = Codes::DEFAULT_ERROR;
        }
    }

    void ExperimentalActions::hook_missed_action(ActionContext &ctx) {
        if (ctx.action == "_get_friend_list") {
            action_get_friend_list(ctx);
        }

        ctx.next();
    }
} // namespace cqhttp::plugins
