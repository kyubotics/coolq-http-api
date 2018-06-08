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
                            const auto gpid = gp.at("gpid");
                            res_gp["friend_group_id"] = gpid;
                            res_gp["friend_group_name"] = gp.at("gpname");
                            res_gp["friends"] = json::array();
                            gpid_idx_map[gpid] = result.data.size();
                            result.data.push_back(res_gp);
                        }

                        for (auto frnd : resp_data.at("list")) {
                            const auto gpid = frnd.at("groupid");
                            auto res_frnd = json::object();
                            res_frnd["user_id"] = frnd.at("uin");
                            res_frnd["nickname"] = frnd.at("nick");
                            res_frnd["remark"] = frnd.at("remark");
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
                        const auto gpid = gp.at("gpid");
                        res_gp["friend_group_id"] = gpid;
                        res_gp["friend_group_name"] = gp.at("gpname");
                        res_gp["friends"] = json::array();
                        gpid_idx_map[gpid] = result.data.size();
                        result.data.push_back(res_gp);
                    }

                    for (auto frnd : resp_data.at("items")) {
                        const auto gpid = frnd.at("groupid");
                        auto res_frnd = json::object();
                        res_frnd["user_id"] = frnd.at("uin");
                        res_frnd["nickname"] = frnd.at("name");
                        res_frnd["remark"] = frnd.at("remark");
                        result.data[gpid_idx_map[gpid]]["friends"].push_back(res_frnd);
                    }

                    result.code = Codes::OK;
                    return;
                } catch (exception &) {
                }
            }
        } catch (cq::exception::ApiError &) {
        }

        // failed
        result.code = Codes::DEFAULT_ERROR;
        result.data = nullptr;
    }

    static void action_get_group_info(ActionContext &ctx) {
        auto &result = ctx.result;

        const auto group_id = ctx.params.get_integer("group_id");
        if (group_id <= 0) {
            result.code = Codes::DEFAULT_ERROR;
            return;
        }

        try {
            const auto login_id_str = to_string(api::get_login_user_id());
            const auto cookies =
                "pt2gguin=o" + login_id_str + ";ptisp=os;p_uin=o" + login_id_str + ";" + api::get_cookies();
            const auto g_tk = to_string(api::get_csrf_token());

            const auto url = "http://qun.qzone.qq.com/cgi-bin/get_group_member?g_tk=" + g_tk + "&uin=" + login_id_str
                             + "&neednum=1&groupid=" + to_string(group_id);
            const auto res = utils::http::get_json(url, true, cookies).value_or(nullptr);

            try {
                const auto data = res.at("data");

                result.data = json::object();
                result.data["group_id"] = group_id;
                result.data["group_name"] = data.at("group_name");
                result.data["create_time"] = data.at("create_time");
                result.data["category"] = data.at("class");
                result.data["member_count"] = data.at("total");
                result.data["introduction"] = data.at("finger_memo");
                result.data["admins"] = json::array();
                for (const auto &admin : data.at("item")) {
                    if (admin.at("iscreator") == 0 && admin.at("ismanager") == 0) {
                        // skip non-admin (may be bot itself)
                        continue;
                    }

                    json j = {
                        {"user_id", admin.at("uin")},
                        {"nickname", admin.at("nick")},
                    };
                    if (admin.at("iscreator") == 1) {
                        j["role"] = "owner";
                    } else {
                        j["role"] = "admin";
                    }
                    result.data["admins"].push_back(j);
                }
                result.code = Codes::OK;
                return;
            } catch (exception &) {
            }
        } catch (cq::exception::ApiError &) {
        }

        // failed
        result.code = Codes::DEFAULT_ERROR;
        result.data = nullptr;
    }

    void ExperimentalActions::hook_missed_action(ActionContext &ctx) {
        if (ctx.action == "_get_friend_list") {
            action_get_friend_list(ctx);
        } else if (ctx.action == "_get_group_info") {
            action_get_group_info(ctx);
        }

        ctx.next();
    }
} // namespace cqhttp::plugins
