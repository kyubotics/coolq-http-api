#include <cqhttp/extension/extension.h>

using namespace std;

namespace cqhttp::extension::extensions {
    struct LongPolling : Extension {
        void hook_enable(Context &ctx) override {
            // ctx.call_action("send_private_msg", {{"user_id", 1002647525}, {"message", "测试 hook_enable"}});
        }

        void hook_disable(Context &ctx) override {
            // ctx.call_action("send_private_msg", {{"user_id", 1002647525}, {"message", "测试 hook_disable"}});
        }

        void hook_after_event(EventContext &ctx) override {
            // ctx.call_action("send_private_msg", {{"user_id", 1002647525}, {"message", "测试 hook_after_event"}});
        }

        void hook_missed_action(ActionContext &ctx) override {
            // ctx.call_action("send_private_msg", {{"user_id", 1002647525}, {"message", "测试 hook_missed_action"}});

            if (ctx.action == "get_updates") {
                ctx.result.code = ActionResult::Codes::OK;
                ctx.result.data = {{"foo", "bar"}};
            }
        }
    };

    PLUGIN_CREATOR { return make_shared<LongPolling>(); }
} // namespace cqhttp::extension::extensions
