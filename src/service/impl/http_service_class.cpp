//#include "./http_service_class.h"
//#include "./service_impl_common.h"
//
//#include <fstream>
//
//using namespace std;
//using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;
//
//extern ApiHandlerMap api_handlers; // defined in handlers.cpp
//
//void HttpService::init() {
//    Log::d(TAG, u8"初始化 HTTP");
//
//    // recreate http server instance
//    server_ = make_shared<HttpServer>();
//
//    server_->default_resource["GET"]
//            = server_->default_resource["POST"]
//            = [](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
//                response->write(SimpleWeb::StatusCode::client_error_not_found);
//            };
//
//    for (const auto &handler_kv : api_handlers) {
//        const auto path_regex = "^/" + handler_kv.first + "/?$";
//        server_->resource[path_regex]["GET"] = server_->resource[path_regex]["POST"]
//                = [&handler_kv](shared_ptr<HttpServer::Response> response,
//                                shared_ptr<HttpServer::Request> request) {
//                    Log::d(TAG, u8"收到 API 请求：" + request->method
//                           + u8" " + request->path
//                           + (request->query_string.empty() ? "" : "?" + request->query_string));
//
//                    auto json_params = json::object();
//                    json args = request->parse_query_string(), form;
//
//                    auto authorized = authorize(request->header, args, [&response](auto status_code) {
//                        response->write(status_code);
//                    });
//                    if (!authorized) {
//                        Log::d(TAG, u8"没有提供 Token 或 Token 不符，已拒绝请求");
//                        return;
//                    }
//
//                    if (request->method == "POST") {
//                        string content_type;
//                        if (const auto it = request->header.find("Content-Type");
//                            it != request->header.end()) {
//                            content_type = it->second;
//                            Log::d(TAG, u8"Content-Type: " + content_type);
//                        }
//
//                        auto body_string = request->content.string();
//                        Log::d(TAG, u8"HTTP 正文内容：" + body_string);
//
//                        if (string_starts_with(content_type, "application/x-www-form-urlencoded")) {
//                            form = SimpleWeb::QueryString::parse(body_string);
//                        } else if (string_starts_with(content_type, "application/json")) {
//                            try {
//                                json_params = json::parse(body_string); // may throw invalid_argument
//                                if (!json_params.is_object()) {
//                                    throw invalid_argument("must be a JSON object");
//                                }
//                            } catch (invalid_argument &) {
//                                Log::d(TAG, u8"HTTP 正文的 JSON 无效或者不是对象");
//                                response->write(SimpleWeb::StatusCode::client_error_bad_request);
//                                return;
//                            }
//                        } else if (!content_type.empty()) {
//                            Log::d(TAG, u8"Content-Type 不支持");
//                            response->write(SimpleWeb::StatusCode::client_error_not_acceptable);
//                            return;
//                        }
//                    }
//
//                    // merge form and args to json params
//                    for (auto data : {form, args}) {
//                        if (data.is_object()) {
//                            for (auto it = data.begin(); it != data.end(); ++it) {
//                                json_params[it.key()] = it.value();
//                            }
//                        }
//                    }
//
//                    Log::d(TAG, u8"API 处理函数 " + handler_kv.first + u8" 开始处理请求");
//                    ApiResult result;
//                    Params params(json_params);
//                    handler_kv.second(params, result); // call the real handler
//
//                    decltype(request->header) headers{
//                        {"Content-Type", "application/json; charset=UTF-8"}
//                    };
//                    auto resp_body = result.json().dump();
//                    Log::d(TAG, u8"响应数据已准备完毕：" + resp_body);
//                    response->write(resp_body, headers);
//                    Log::d(TAG, u8"响应内容已发送");
//                    Log::i(TAG, u8"已成功处理一个 API 请求：" + request->path);
//                };
//    }
//
//    // data files handler
//    const auto regex = "^/(data/(?:bface|image|record|show)/.+)$";
//    server_->resource[regex]["GET"] = [](shared_ptr<HttpServer::Response> response,
//                                         shared_ptr<HttpServer::Request> request) {
//        if (!config.serve_data_files) {
//            response->write(SimpleWeb::StatusCode::client_error_not_found);
//            return;
//        }
//
//        auto relpath = request->path_match.str(1);
//        string_replace(relpath, "/", "\\");
//        Log::d(TAG, u8"收到 GET 数据文件请求，相对路径：" + relpath);
//
//        if (string_contains(relpath, "..")) {
//            Log::d(TAG, u8"请求的数据文件路径中有非法字符，已拒绝请求");
//            response->write(SimpleWeb::StatusCode::client_error_forbidden);
//            return;
//        }
//
//        auto filepath = sdk->directories().coolq() + relpath;
//        auto ansi_filepath = ansi(filepath);
//        if (!fs::is_regular_file(ansi_filepath)) {
//            // is not a file
//            Log::d(TAG, u8"相对路径 " + relpath + u8" 所制定的内容不存在，或为非文件类型，无法发送");
//            response->write(SimpleWeb::StatusCode::client_error_not_found);
//            return;
//        }
//
//        if (ifstream f(ansi_filepath, ios::in | ios::binary); f.is_open()) {
//            auto length = fs::file_size(ansi_filepath);
//            response->write(decltype(request->header){
//                {"Content-Length", to_string(length)},
//                {"Content-Type", "application/octet-stream"},
//                {"Content-Disposition", "attachment"}
//            });
//            *response << f.rdbuf();
//            Log::d(TAG, u8"文件内容已发送完毕");
//        } else {
//            Log::d(TAG, u8"文件 " + relpath + u8" 打开失败，请检查文件系统权限");
//            response->write(SimpleWeb::StatusCode::client_error_forbidden);
//            return;
//        }
//
//        Log::i(TAG, u8"已成功发送文件：" + relpath);
//    };
//
//    ServiceBase::init();
//}
//
//void HttpService::finalize() {
//    server_ = nullptr;
//    ServiceBase::finalize();
//}
//
//void HttpService::start() {
//    if (config.use_http) {
//        init();
//
//        server_->config.thread_pool_size = server_thread_pool_size();
//        server_->config.address = config.host;
//        server_->config.port = config.port;
//        thread_ = thread([&]() {
//            started_ = true;
//            try {
//                server_->start();
//            } catch (...) {}
//            started_ = false; // since it reaches here, the server is absolutely stopped
//        });
//        Log::d(TAG, u8"开启 API HTTP 服务器成功，开始监听 http://"
//               + server_->config.address + ":" + to_string(server_->config.port));
//    }
//}
//
//void HttpService::stop() {
//    if (started_) {
//        server_->stop();
//        started_ = false;
//    }
//    if (thread_.joinable()) {
//        thread_.join();
//    }
//    finalize();
//}
//
//bool HttpService::good() const {
//    if (config.use_http) {
//        return initialized_ && started_;
//    }
//    return ServiceBase::good();
//}
