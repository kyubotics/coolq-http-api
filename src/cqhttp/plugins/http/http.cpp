#include "./http.h"

#include "cqhttp/utils/http.h"

using namespace std;
namespace api = cq::api;

namespace cqhttp::plugins {
    static const auto TAG = "HTTP";

    void Http::hook_enable(Context &ctx) {
        logging::debug("http", "enable");

        use_http_ = ctx.config->get_bool("use_http", true);
        post_url_ = ctx.config->get_string("post_url", "");

        ctx.next();
    }

    static void handle_quick_operation(const EventContext<cq::Event> &ctx, const utils::JsonEx &params) {
        if (ctx.event.type == cq::event::MESSAGE) {
            const auto msg_ev = static_cast<const cq::MessageEvent &>(ctx.event);

            auto reply = params.get_message("reply");
            if (!reply.empty()) {
                if ((msg_ev.message_type == cq::message::GROUP || msg_ev.message_type == cq::message::DISCUSS)
                    && params.get_bool("at_sender", true)) {
                    reply = cq::MessageSegment::at(msg_ev.target.user_id.value()) + " " + reply;
                }

                logging::info(TAG, u8"执行快速操作：回复");
                reply.send(msg_ev.target);
            }

            if (msg_ev.message_type == cq::message::GROUP) {
                const auto grp_msg_ev = static_cast<const cq::GroupMessageEvent &>(ctx.event);

                if (params.get_bool("delete", false)) {
                    logging::info(TAG, u8"执行快速操作：群组撤回成员消息");
                    api::delete_msg(grp_msg_ev.message_id);
                }

                if (params.get_bool("kick", false) && !grp_msg_ev.is_anonymous()) {
                    logging::info(TAG, u8"执行快速操作：群组踢人");
                    api::set_group_kick(grp_msg_ev.group_id, grp_msg_ev.user_id, false);
                }

                if (params.get_bool("ban", false)) {
                    const auto duration = 30 * 60; // 30 minutes by default
                    logging::info(TAG, u8"执行快速操作：群组禁言");
                    if (grp_msg_ev.is_anonymous()) {
                        api::set_group_anonymous_ban(grp_msg_ev.group_id, grp_msg_ev.anonymous.flag, duration);
                    } else {
                        api::set_group_ban(grp_msg_ev.group_id, grp_msg_ev.user_id, duration);
                    }
                }
            }
        } else if (ctx.event.type == cq::event::REQUEST) {
            const auto req_ev = static_cast<const cq::RequestEvent &>(ctx.event);

            if (auto approve_opt = params.get<bool>("approve"); approve_opt) {
                const auto approve = approve_opt.value();
                if (req_ev.request_type == cq::request::FRIEND) {
                    const auto frnd_req_ev = static_cast<const cq::FriendRequestEvent &>(ctx.event);
                    logging::info(TAG, u8"执行快速操作：处理好友请求");
                    api::set_friend_add_request(frnd_req_ev.flag,
                                                approve ? cq::request::APPROVE : cq::request::REJECT,
                                                params.get_string("remark"));
                } else if (req_ev.request_type == cq::request::GROUP) {
                    const auto grp_req_ev = static_cast<const cq::GroupRequestEvent &>(ctx.event);
                    logging::info(TAG, u8"执行快速操作：处理群组请求");
                    api::set_group_add_request(grp_req_ev.flag,
                                               grp_req_ev.sub_type,
                                               approve ? cq::request::APPROVE : cq::request::REJECT,
                                               params.get_string("reason"));
                }
            }
        }
    }

    void Http::hook_after_event(EventContext<cq::Event> &ctx) {
        if (!post_url_.empty()) {
            logging::debug(TAG, u8"开始通过 HTTP 上报事件");
            const auto resp = utils::http::post_json(post_url_, ctx.data);

            if (resp.status_code == 0) {
                logging::info(TAG, u8"HTTP 上报地址 " + post_url_ + u8" 无法访问");
            } else {
                logging::info_success(TAG,
                                      u8"通过 HTTP 上报数据到 " + post_url_ + (resp.ok() ? u8" 成功" : u8" 失败")
                                          + u8"，状态码：" + to_string(resp.status_code));
            }

            if (resp.ok() && !resp.body.empty()) {
                logging::debug(TAG, u8"收到响应 " + resp.body);

                const auto resp_payload = resp.get_json();
                if (resp_payload.is_object()) {
                    const utils::JsonEx params = resp_payload;

                    handle_quick_operation(ctx, params);

                    if (params.get_bool("block", false)) {
                        ctx.event.block();
                    }
                } else {
                    logging::debug(TAG, u8"上报响应不是有效的 JSON，已忽略");
                }
            }
        }

        ctx.next();
    }
} // namespace cqhttp::plugins
