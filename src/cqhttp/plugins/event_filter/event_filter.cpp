#include "./event_filter.h"

#include <filesystem>
#include <fstream>

using namespace std;
namespace fs = std::filesystem;

namespace cqhttp::plugins {
    static const auto TAG = u8"事件过滤器";

    void EventFilter::hook_enable(Context &ctx) {
        filter_ = nullptr;

        const auto filter_filename = ctx.config->get_string("event_filter", "");
        if (!filter_filename.empty()) {
            // should load and use filter
            const auto path = cq::dir::app() + filter_filename;
            if (const auto ws_path = s2ws(path); fs::is_regular_file(ws_path)) {
                if (ifstream f(ws_path); f.is_open()) {
                    try {
                        json filter_json;
                        f >> filter_json;
                        if (filter_json.is_object()) {
                            filter_ = construct_filter(filter_json);
                            logging::debug(TAG, u8"过滤规则加载成功");
                        } else {
                            logging::error(TAG, u8"过滤规则必须是 JSON 对象");
                        }
                    } catch (json::parse_error &) {
                        logging::error(TAG, u8"过滤规则不是合法的 JSON");
                    } catch (FilterSyntexError &e) {
                        logging::error(TAG, string(u8"过滤规则语法错误，错误信息：") + e.what());
                    }
                }
            } else {
                logging::error(TAG, u8"没有找到过滤规则文件 " + filter_filename);
            }

            if (!filter_) {
                // we was expecting to load a filter, but failed
                // so we should block all event by default

                class BlockAllFilter : public Filter {
                public:
                    bool eval(const json &) override { return false; }
                };

                filter_ = make_shared<BlockAllFilter>();
                logging::warning(TAG, u8"过滤规则加载失败，将暂停所有事件上报");
            }
        }

        ctx.next();
    }

    void EventFilter::hook_disable(Context &ctx) {
        filter_ = nullptr;
        ctx.next();
    }

    void EventFilter::hook_after_event(EventContext<cq::Event> &ctx) {
        // use hook_after_event here because we want it to work just before the web things
        if (!filter_ || filter_->eval(ctx.data)) {
            // filter not used, or filter passed
            ctx.next();
        }
    }
} // namespace cqhttp::plugins
