#include "./experimental_actions.h"

#include "cqhttp/plugins/experimental_actions/vendor/pugixml/pugixml.hpp"
#include "cqhttp/utils/http.h"

using namespace std;
namespace api = cq::api;

namespace cqhttp::plugins {
    using Codes = ActionResult::Codes;

    static void action_get_friend_list(ActionContext &ctx) {
        auto &result = ctx.result;

        const auto flat = ctx.params.get_bool("flat", false);

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

                        if (flat) {
                            result.data = {
                                {"friend_groups", json::array()},
                                {"friends", json::array()},
                            };

                            for (auto gp : resp_data.at("gpnames")) {
                                result.data["friend_groups"].push_back({
                                    {"friend_group_id", gp.at("gpid")},
                                    {"friend_group_name", gp.at("gpname")},
                                });
                            }

                            for (auto frnd : resp_data.at("list")) {
                                result.data["friends"].push_back({
                                    {"user_id", frnd.at("uin")},
                                    {"nickname", frnd.at("nick")},
                                    {"remark", frnd.at("remark")},
                                    {"friend_group_id", frnd.at("groupid")},
                                });
                            }
                        } else {
                            result.data = json::array();

                            map<int64_t, int> gpid_idx_map;
                            for (auto gp : resp_data.at("gpnames")) {
                                auto res_gp = json::object();
                                const auto gpid = gp.at("gpid");
                                res_gp.emplace("friend_group_id", gpid);
                                res_gp.emplace("friend_group_name", gp.at("gpname"));
                                res_gp.emplace("friends", json::array());
                                gpid_idx_map[gpid] = result.data.size();
                                result.data.push_back(res_gp);
                            }

                            for (auto frnd : resp_data.at("list")) {
                                const auto gpid = frnd.at("groupid");
                                auto res_frnd = json::object();
                                res_frnd.emplace("user_id", frnd.at("uin"));
                                res_frnd.emplace("nickname", frnd.at("nick"));
                                res_frnd.emplace("remark", frnd.at("remark"));
                                result.data[gpid_idx_map[gpid]]["friends"].push_back(res_frnd);
                            }
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

                    if (flat) {
                        result.data = {
                            {"friend_groups", json::array()},
                            {"friends", json::array()},
                        };

                        for (auto gp : resp_data.at("gpnames")) {
                            result.data["friend_groups"].push_back({
                                {"friend_group_id", gp.at("gpid")},
                                {"friend_group_name", gp.at("gpname")},
                            });
                        }

                        for (auto frnd : resp_data.at("items")) {
                            result.data["friends"].push_back({
                                {"user_id", frnd.at("uin")},
                                {"nickname", frnd.at("name")},
                                {"remark", frnd.at("remark")},
                                {"friend_group_id", frnd.at("groupid")},
                            });
                        }
                    } else {
                        result.data = json::array();

                        map<int64_t, int> gpid_idx_map;
                        for (auto gp : resp_data.at("gpnames")) {
                            auto res_gp = json::object();
                            const auto gpid = gp.at("gpid");
                            res_gp.emplace("friend_group_id", gpid);
                            res_gp.emplace("friend_group_name", gp.at("gpname"));
                            res_gp["friends"] = json::array();
                            gpid_idx_map[gpid] = result.data.size();
                            result.data.push_back(res_gp);
                        }

                        for (auto frnd : resp_data.at("items")) {
                            const auto gpid = frnd.at("groupid");
                            auto res_frnd = json::object();
                            res_frnd.emplace("user_id", frnd.at("uin"));
                            res_frnd.emplace("nickname", frnd.at("name"));
                            res_frnd.emplace("remark", frnd.at("remark"));
                            result.data[gpid_idx_map[gpid]]["friends"].push_back(res_frnd);
                        }
                    }

                    result.code = Codes::OK;
                    return;
                } catch (exception &) {
                }
            }
        } catch (cq::exception::ApiError &) {
        }

        // failed
        result.code = Codes::CREDENTIAL_INVALID;
        result.data = nullptr;
    }

    static void action_get_group_info(ActionContext &ctx) {
        auto &result = ctx.result;

        const auto group_id = ctx.params.get_integer("group_id");
        if (group_id <= 0) {
            result.code = Codes::DEFAULT_ERROR;
            result.data = nullptr;
            return;
        }

        string login_id_str;
        string cookies;
        string csrf_token;
        try {
            login_id_str = to_string(api::get_login_user_id());
            cookies = "pt2gguin=o" + login_id_str + ";ptisp=os;p_uin=o" + login_id_str + ";" + api::get_cookies();
            csrf_token = to_string(api::get_csrf_token());
        } catch (cq::exception::ApiError &) {
            goto GET_GROUP_INFO_FAILED;
        }

        result.data = json::object();

        {
            // get basic info
            const auto url = "http://qun.qzone.qq.com/cgi-bin/get_group_member?g_tk=" + csrf_token
                             + "&uin=" + login_id_str + "&neednum=1&groupid=" + to_string(group_id);
            const auto res = utils::http::get_json(url, true, cookies).value_or(nullptr);

            try {
                const auto data = res.at("data");
                result.data.emplace("group_id", group_id);
                result.data.emplace("group_name", data.at("group_name"));
                result.data.emplace("create_time", data.at("create_time"));
                result.data.emplace("category", data.at("class"));
                result.data.emplace("member_count", data.at("total"));
                result.data.emplace("introduction", data.at("finger_memo"));
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
                result.data.emplace("admin_count", result.data["admins"].size());
            } catch (exception &) {
            }
        }

        {
            // get some extra info, like group size (max member count)
            const auto url = "http://qinfo.clt.qq.com/cgi-bin/qun_info/get_members_info_v1?friends=1&name=1&gc="
                             + to_string(group_id) + "&bkn=" + csrf_token + "&src=qinfo_v3";
            const auto data = utils::http::get_json(url, true, cookies).value_or(nullptr);

            try {
                result.data.emplace("owner_id", data.at("owner"));
                result.data.emplace("max_admin_count", data.at("max_admin"));
                result.data.emplace("max_member_count", data.at("max_num"));
                if (result.data.count("member_count") == 0) {
                    // it's actually impossible to reach here, but we check it in case
                    result.data.emplace("member_count", data.at("mem_num"));
                }
            } catch (exception &) {
            }
        }

        if (!result.data.empty()) {
            // we got some information at least
            result.code = Codes::OK;
            return;
        }

    GET_GROUP_INFO_FAILED:
        result.code = Codes::CREDENTIAL_INVALID;
        result.data = nullptr;
    }

    static void action_get_vip_info(ActionContext &ctx) {
        auto &result = ctx.result;
        result.data = {
            {"user_id", nullptr},
            {"nickname", nullptr},
            {"level", nullptr},
            {"level_speed", nullptr},
            {"vip_level", nullptr},
            {"vip_growth_speed", nullptr},
            {"vip_growth_total", nullptr},
        };

        const auto user_id = ctx.params.get_integer("user_id");
        if (user_id <= 0) {
            result.code = Codes::DEFAULT_ERROR;
            result.data = nullptr;
            return;
        }
        result.data["user_id"] = user_id;

        const auto user_id_str = to_string(user_id);
        const string fake_ua =
            "Mozilla/5.0 (iPhone; CPU iPhone OS 12_0 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) "
            "Mobile/16A5288q QQ/6.5.5.0 TIM/2.2.5.401 V1_IPH_SQ_6.5.5_1_TIM_D Pixel/750 Core/UIWebView "
            "Device/Apple(iPhone 6s) NetType/WIFI";
        string cookies;
        try {
            cookies = api::get_cookies();
            const auto info = api::get_stranger_info(user_id);
            result.data["nickname"] = info.nickname;
        } catch (cq::exception::ApiError &) {
            goto GET_VIP_INFO_FAILED;
        }

        {
            const auto url = "https://h5.vip.qq.com/p/mc/cardv2/other?platform=1&qq=" + user_id_str
                             + "&adtag=geren&aid=mvip.pingtai.mobileqq.androidziliaoka.fromqita";
            const auto resp = utils::http::get(url, {{"Cookie", cookies}, {"User-Agent", fake_ua}});

            if (smatch m; regex_search(
                    resp.body,
                    m,
                    regex(
                        R"(<div class="mod-radar-pk">[\s\S]*class="ui-btn-wrap"[\s\S]*?<\/button>\s*<\/div>\s*<\/div>)"))) {
                const auto content = m.str();
                pugi::xml_document doc;
                if (doc.load_string(content.c_str())) {
                    try {
                        const auto nodes = doc.select_nodes("//div[contains(@class, 'pk-line-guest')]//p");
                        auto it = nodes.begin();
                        result.data["level"] = (*it++).node().text().as_int();
                        result.data["level_speed"] = roundf((*it++).node().text().as_float() * 10.0) / 10.0;
                        auto vip_level = boost::trim_copy(string((*it).node().text().as_string()));
                        if (vip_level.empty()) {
                            vip_level = boost::trim_copy(string((*it).node().first_child().text().as_string()));
                        }
                        result.data["vip_level"] = vip_level;
                        it++;
                        result.data["vip_growth_speed"] = (*it++).node().text().as_int();
                        result.data["vip_growth_total"] = (*it++).node().text().as_int();
                    } catch (...) {
                    }
                }
            }
        }

        result.code = Codes::OK;
        return;

    GET_VIP_INFO_FAILED:
        result.code = Codes::CREDENTIAL_INVALID;
        result.data = nullptr;
    }

    static void action_group_notice(ActionContext &ctx, bool post_new_notice) {
        auto &result = ctx.result;
        result.code = Codes::OK;
        result.data = nullptr;

        const auto group_id = ctx.params.get_integer("group_id");
        if (group_id <= 0) {
            result.code = Codes::DEFAULT_ERROR;
            return;
        }

        string params;
        string cookies;
        try {
            const int csrf_token = api::get_csrf_token();
            params = "bkn=" + to_string(csrf_token) + "&qid=" + to_string(group_id);
            cookies = api::get_cookies();
        } catch (exception &) {
            result.code = Codes::CREDENTIAL_INVALID;
            return;
        }

        // list the current group notices and get the gsi
        string gsi;
        try {
            const auto url = "https://web.qun.qq.com/cgi-bin/announce/get_t_list?" + params
                             + "&ft=23&s=-1&n=10&ni=1&i=1";
            const auto res = utils::http::get_json(url, true, cookies).value_or(nullptr);
            result.data = res.at("feeds");
            gsi = res.at("gsi");
        } catch (exception &) {
            result.code = Codes::OPERATION_FAILED;
            return;
        }
        if (!post_new_notice) {
            return;  // get_group_notice finishes here
        }

        const auto text = ctx.params.get_string("text");
        const auto title = ctx.params.get_string("title");
        // well it's theoretically possible to create a empty group notice...
        // but that's not useful anyway, hence just let it go
        if (text.empty() || title.empty()) {
            result.code = Codes::DEFAULT_ERROR;
            return;
        }

        // reuse the bkn & gsi parameters from above to post a new group notice
        try {
            const auto url = "https://web.qun.qq.com/cgi-bin/announce/add_qun_notice";
            const auto body = params + "&gsi=" + gsi + "&text=" + text + "&title=" + title;
            const auto post_response = utils::http::post(url, body, {{"Cookie", cookies}});
            const auto res = json::parse(post_response.body);
            if (res.at("ec").get<int>()) {  // error code
                result.code = Codes::CREDENTIAL_INVALID;
            } else if (res.at("id").get<int>() == 0) {  // insufficient user permission for posting a new notice
                result.code = Codes::OPERATION_FAILED;
            }
        } catch (exception &) {
            result.code = Codes::INVALID_DATA;
        }
    }

    void ExperimentalActions::hook_missed_action(ActionContext &ctx) {
        if (ctx.action == "_get_friend_list") {
            action_get_friend_list(ctx);
        } else if (ctx.action == "_get_group_info") {
            action_get_group_info(ctx);
        } else if (ctx.action == "_get_vip_info") {
            action_get_vip_info(ctx);
        } else if (ctx.action == "_get_group_notice") {
            action_group_notice(ctx, false);
        } else if (ctx.action == "_send_group_notice") {
            action_group_notice(ctx, true);
        } else {
            ctx.next();
        }
    }
} // namespace cqhttp::plugins
