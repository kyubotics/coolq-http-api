#include "./http.h"

#include <filesystem>
#include <fstream>

#include "cqhttp/plugins/web/server_common.h"
#include "cqhttp/utils/crypt.h"
#include "cqhttp/utils/http.h"

using namespace std;
namespace fs = std::filesystem;
namespace api = cq::api;
using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;

namespace cqhttp::plugins {
    static const auto TAG = "HTTP";

    void Http::init_server() {
        logging::debug(TAG, u8"初始化 HTTP 服务器");

        // recreate http server instance
        server_ = make_shared<HttpServer>();

        server_->default_resource["GET"] = server_->default_resource["POST"] =
            [](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
                response->write(SimpleWeb::StatusCode::client_error_not_found);
            };

        const auto action_path_regex = "^/([^/\\s]+)/?$";
        server_->resource[action_path_regex]["GET"] = server_->resource[action_path_regex]["POST"] =
            [=](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
                logging::debug(TAG,
                               u8"收到 API 请求：" + request->method + " " + request->path
                                   + (request->query_string.empty() ? "" : "?" + request->query_string));

                auto params = json::object();
                const json args = request->parse_query_string();
                json form;

                const auto authorized = authorize(access_token_, request->header, args, [&response](auto status_code) {
                    response->write(status_code);
                });
                if (!authorized) {
                    logging::debug(TAG, u8"没有提供 Token 或 Token 不符，已拒绝请求");
                    return;
                }

                if (request->method == "POST") {
                    string content_type;
                    if (const auto it = request->header.find("Content-Type"); it != request->header.end()) {
                        content_type = it->second;
                        logging::debug(TAG, u8"Content-Type: " + content_type);
                    }

                    const auto body_string = request->content.string();
                    logging::debug(TAG, u8"HTTP 正文内容：" + body_string);

                    if (boost::starts_with(content_type, "application/x-www-form-urlencoded")) {
                        form = SimpleWeb::QueryString::parse(body_string);
                    } else if (boost::starts_with(content_type, "application/json")) {
                        try {
                            params = json::parse(body_string); // may throw json::parse_error
                            if (!params.is_object()) {
                                throw invalid_argument("must be a JSON object");
                            }
                        } catch (...) {
                            logging::debug(TAG, u8"HTTP 正文的 JSON 无效或者不是对象");
                            response->write(SimpleWeb::StatusCode::client_error_bad_request);
                            return;
                        }
                    } else if (!content_type.empty()) {
                        logging::debug(TAG, u8"Content-Type 不支持");
                        response->write(SimpleWeb::StatusCode::client_error_not_acceptable);
                        return;
                    }
                }

                // merge form and args to json params
                for (auto data : {form, args}) {
                    if (data.is_object()) {
                        for (auto it = data.begin(); it != data.end(); ++it) {
                            params[it.key()] = it.value();
                        }
                    }
                }

                const auto action = request->path_match.str(1);
                logging::debug(TAG, u8"开始执行动作 " + action);

                const auto result = call_action(action, params);
                if (result.code == ActionResult::Codes::HTTP_NOT_FOUND) {
                    // no "Plugin::hook_missed_action" handled this action, we return 404
                    logging::debug(TAG, u8"没有找到相应的处理函数，动作 " + action + u8" 执行失败");
                    response->write(SimpleWeb::StatusCode::client_error_not_found);
                } else {
                    logging::debug(TAG, u8"动作 " + action + u8" 执行成功");
                    const decltype(request->header) headers{{"Content-Type", "application/json; charset=UTF-8"}};
                    const auto resp_body = json(result).dump();
                    logging::debug(TAG, u8"响应数据已准备完毕：" + resp_body);
                    response->write(resp_body, headers);
                    logging::debug(TAG, u8"响应内容已发送");
                    logging::info_success(TAG, u8"已成功处理一个 API 请求：" + request->path);
                }
            };

        // data files handler
        const auto regex = "^/(data/(?:bface|image|record|show)/.+)$";
        server_->resource[regex]["GET"] =
            [=](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
                if (!serve_data_files_) {
                    response->write(SimpleWeb::StatusCode::client_error_not_found);
                    return;
                }

                const auto authorized = authorize(access_token_, request->header, {}, [&response](auto status_code) {
                    response->write(status_code);
                });
                if (!authorized) {
                    logging::debug(TAG, u8"没有提供 Token 或 Token 不符，已拒绝请求");
                    return;
                }

                auto relpath = request->path_match.str(1);
                boost::algorithm::replace_all(relpath, "/", "\\");
                logging::debug(TAG, u8"收到 GET 数据文件请求，相对路径：" + relpath);

                if (boost::algorithm::contains(relpath, "..")) {
                    logging::debug(TAG, u8"请求的数据文件路径中有非法字符，已拒绝请求");
                    response->write(SimpleWeb::StatusCode::client_error_forbidden);
                    return;
                }

                const auto filepath = cq::dir::root() + relpath;
                const auto ansi_filepath = ansi(filepath);
                if (!fs::is_regular_file(ansi_filepath)) {
                    // is not a file
                    logging::debug(TAG, u8"相对路径 " + relpath + u8" 所制定的内容不存在，或为非文件类型，无法发送");
                    response->write(SimpleWeb::StatusCode::client_error_not_found);
                    return;
                }

                if (ifstream f(ansi_filepath, ios::in | ios::binary); f.is_open()) {
                    const auto length = fs::file_size(ansi_filepath);
                    response->write(decltype(request->header){{"Content-Length", to_string(length)},
                                                              {"Content-Type", "application/octet-stream"},
                                                              {"Content-Disposition", "attachment"}});
                    *response << f.rdbuf();
                    logging::debug(TAG, u8"文件内容已发送完毕");
                } else {
                    logging::debug(TAG, u8"文件 " + relpath + u8" 打开失败，请检查文件系统权限");
                    response->write(SimpleWeb::StatusCode::client_error_forbidden);
                    return;
                }

                logging::info_success(TAG, u8"已成功发送文件：" + relpath);
            };
    }

    void Http::hook_enable(Context &ctx) {
        post_url_ = ctx.config->get_string("post_url", "");
        if (!post_url_.empty() && !regex_search(post_url_, regex("^https?://"))) {
            // bad post url, we warn the user, and ignore the post url
            logging::warning(TAG, u8"HTTP 上报地址 " + post_url_ + u8" 不是合法地址，将被忽略");
            post_url_ = "";
        }
        post_timeout_ = ctx.config->get_integer("post_timeout", 0);
        secret_ = ctx.config->get_string("secret", "");

        use_http_ = ctx.config->get_bool("use_http", true);
        access_token_ = ctx.config->get_string("access_token", "");
        serve_data_files_ = ctx.config->get_bool("serve_data_files", false);

        if (use_http_) {
            init_server();

            server_->config.thread_pool_size =
                fix_server_thread_pool_size(ctx.config->get_integer("server_thread_pool_size", 1));
            server_->config.address = ctx.config->get_string("host", "0.0.0.0");
            server_->config.port = ctx.config->get_integer("port", 5700);
            thread_ = thread([&]() {
                started_ = true;
                try {
                    server_->start(); // loop to accept requests
                } catch (...) {
                }
                started_ = false; // since it reaches here, the server is absolutely stopped
            });
            logging::info_success(TAG,
                                  u8"开启 HTTP 服务器成功，开始监听 http://" + server_->config.address + ":"
                                      + to_string(server_->config.port));
        }

        ctx.next();
    }

    void Http::hook_disable(Context &ctx) {
        if (started_) {
            server_->stop();
            started_ = false;
        }
        if (thread_.joinable()) {
            thread_.join();
        }

        server_ = nullptr;

        ctx.next();
    }

    /**
     * Replaced with ".handle_quick_operation" action.
     */
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
                try {
                    reply.send(msg_ev.target);
                } catch (cq::exception::ApiError &) {
                }
            }

            if (msg_ev.message_type == cq::message::GROUP) {
                const auto grp_msg_ev = static_cast<const cq::GroupMessageEvent &>(ctx.event);

                if (params.get_bool("delete", false)) {
                    logging::info(TAG, u8"执行快速操作：群组撤回成员消息");
                    try {
                        api::delete_msg(grp_msg_ev.message_id);
                    } catch (cq::exception::ApiError &) {
                    }
                }

                if (params.get_bool("kick", false) && !grp_msg_ev.is_anonymous()) {
                    logging::info(TAG, u8"执行快速操作：群组踢人");
                    try {
                        api::set_group_kick(grp_msg_ev.group_id, grp_msg_ev.user_id, false);
                    } catch (cq::exception::ApiError &) {
                    }
                }

                if (params.get_bool("ban", false)) {
                    const auto duration = 30 * 60; // 30 minutes by default
                    logging::info(TAG, u8"执行快速操作：群组禁言");
                    try {
                        if (grp_msg_ev.is_anonymous()) {
                            api::set_group_anonymous_ban(grp_msg_ev.group_id, grp_msg_ev.anonymous.flag, duration);
                        } else {
                            api::set_group_ban(grp_msg_ev.group_id, grp_msg_ev.user_id, duration);
                        }
                    } catch (cq::exception::ApiError &) {
                    }
                }
            }
        } else if (ctx.event.type == cq::event::REQUEST) {
            const auto req_ev = static_cast<const cq::RequestEvent &>(ctx.event);

            if (auto approve_opt = params.get<bool>("approve"); approve_opt) {
                const auto approve = approve_opt.value();
                try {
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
                } catch (cq::exception::ApiError &) {
                }
            }
        }
    }

    static utils::http::Response post_json(const string &url, const json &payload, const string &secret,
                                           const long timeout) {
        const auto body = payload.dump();
        utils::http::Headers headers{
            {"Content-Type", "application/json; charset=UTF-8"},
            {"X-Self-ID", to_string(api::get_login_user_id())},
        };
        if (!secret.empty()) {
            headers["X-Signature"] = "sha1=" + utils::crypt::hmac_sha1_hex(secret, body);
        }
        return post(url, payload.dump(), headers, timeout);
    }

    void Http::hook_after_event(EventContext<cq::Event> &ctx) {
        if (!post_url_.empty()) {
            logging::debug(TAG, u8"开始通过 HTTP 上报事件");
            const auto resp = post_json(post_url_, ctx.data, secret_, post_timeout_);

            if (resp.status_code == 0) {
                logging::warning(TAG, u8"HTTP 上报地址 " + post_url_ + u8" 无法访问");
            } else {
                const auto log_msg = u8"通过 HTTP 上报数据到 " + post_url_ + (resp.ok() ? u8" 成功" : u8" 失败")
                                     + u8"，状态码：" + to_string(resp.status_code);
                if (resp.ok()) {
                    logging::info_success(TAG, log_msg);
                } else {
                    logging::warning(TAG, log_msg);
                }
            }

            if (resp.ok() && !resp.body.empty()) {
                logging::debug(TAG, u8"收到响应 " + resp.body);

                const auto resp_payload = resp.get_json();
                if (resp_payload.is_object()) {
                    const utils::JsonEx params = resp_payload;

                    // note here that the ctx.data object was processed by backward_compatibility plugin,
                    // but now that the ".handle_quick_operation" action can handle legacy data format,
                    // it's ok here to use ctx.data directly
                    call_action(".handle_quick_operation", {{"context", ctx.data}, {"operation", params.raw}});

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

    void Http::hook_missed_action(ActionContext &ctx) {
        if (ctx.action != ".handle_quick_operation") {
            ctx.next();
            return;
        }

        ctx.result.code = ActionResult::Codes::DEFAULT_ERROR;

        // note that the following code must handle legacy event data format,
        // because the user may enable backward compatibility, and if that happens,
        // we will see legacy event data in this function

        const auto context = utils::JsonEx(ctx.params.get("context").value_or(nullptr));
        const auto operation = utils::JsonEx(ctx.params.get("operation").value_or(nullptr));

        if (!context.raw.is_object() || !operation.raw.is_object()) {
            return;
        }

        const auto post_type = context.get_string("post_type");
        if (post_type == "message") {
            const auto message_type = context.get_string("message_type");
            auto reply = operation.get_message("reply");
            if (!reply.empty()) {
                if ((message_type == "group" || message_type == "discuss") && operation.get_bool("at_sender", true)) {
                    reply = cq::MessageSegment::at(context.get_integer("user_id")) + " " + reply;
                }

                logging::info(TAG, u8"执行快速操作：回复");
                auto params = context.raw;
                params["message"] = reply;
                call_action("send_msg", params);
            }

            if (message_type == "group") {
                const auto anonymous = context.get("anonymous").value_or(nullptr);
                const auto is_anonymous =
                    anonymous.is_object() || anonymous.is_string() && !anonymous.get<string>().empty();
                if (operation.get_bool("delete", false)) {
                    logging::info(TAG, u8"执行快速操作：群组撤回成员消息");
                    call_action("delete_msg", context.raw);
                }

                if (operation.get_bool("kick", false) && !is_anonymous) {
                    logging::info(TAG, u8"执行快速操作：群组踢人");
                    call_action("set_group_kick", context.raw);
                }

                if (operation.get_bool("ban", false)) {
                    const auto duration = operation.get_integer("ban_duration", 30 * 60 /* 30 minutes by default */);
                    logging::info(TAG, u8"执行快速操作：群组禁言");
                    auto params = context.raw;
                    params["duration"] = duration;
                    if (is_anonymous) {
                        call_action("set_group_anonymous_ban", params);
                    } else {
                        call_action("set_group_ban", params);
                    }
                }
            }
        } else if (post_type == "request") {
            const auto request_type = context.get_string("request_type");
            if (auto approve_opt = operation.get<bool>("approve"); approve_opt) {
                auto params = context.raw;
                params["approve"] = approve_opt.value();
                params["remark"] = operation.get_string("remark");
                params["reason"] = operation.get_string("reason");
                if (request_type == "friend") {
                    logging::info(TAG, u8"执行快速操作：处理好友请求");
                    call_action("set_friend_add_request", params);
                } else if (request_type == "group") {
                    logging::info(TAG, u8"执行快速操作：处理群组请求");
                    call_action("set_group_add_request", params);
                }
            }
        }

        ctx.result.code = ActionResult::Codes::OK;
    }
} // namespace cqhttp::plugins
