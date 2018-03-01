#include "./core.h"

using namespace std;

namespace cqhttp {
    static vector<function<void()>> callback_initializers;

    static bool add_callback_initializer(const function<void()> initializer) {
        callback_initializers.push_back(initializer);
        return true;
    }

    static shared_ptr<Application> app;

    void init() {
        for (const auto &initializer : callback_initializers) {
            initializer();
        }
    }

    void apply(const shared_ptr<Application> app) {
        cqhttp::app = app;
    }

    #define APP(FuncName, ...) \
        if (app) { app->__##FuncName(__VA_ARGS__); }

    /**
     * Generate lifecycle callbacks.
     */
    #define LIFECYCLE(Name) \
        static void __on_##Name() { APP(hook_##Name); } \
        static bool __dummy_on_##Name = add_callback_initializer([]() { \
            cq::app::on_##Name = __on_##Name; \
        })

    LIFECYCLE(initialize);
    LIFECYCLE(enable);
    LIFECYCLE(disable);
    LIFECYCLE(coolq_start);
    LIFECYCLE(coolq_exit);

    /**
     * Generate event callbacks.
     */
    #define EVENT(Name, ...) \
        static void __##Name##_event(__VA_ARGS__); \
        static bool __dummy_##Name##_event = add_callback_initializer([]() { \
            cq::event::Name = __##Name##_event; \
        }); \
        static void __##Name##_event(__VA_ARGS__)

    EVENT(on_private_msg, const cq::PrivateMessageEvent &e) {
        json data = e;
        APP(hook_message_event, e.message_type, e.sub_type, data);
    }

    EVENT(on_group_msg, const cq::GroupMessageEvent &e) {
        json data = e;
        APP(hook_message_event, e.message_type, e.sub_type, data);
    }

    EVENT(on_discuss_msg, const cq::DiscussMessageEvent &e) {
        json data = e;
        APP(hook_message_event, e.message_type, e.sub_type, data);
    }

    EVENT(on_group_upload, const cq::GroupUploadEvent &e) {
        json data = e;
        APP(hook_notice_event, e.notice_type, e.sub_type, data);
    }

    EVENT(on_group_admin, const cq::GroupAdminEvent &e) {
        json data = e;
        APP(hook_notice_event, e.notice_type, e.sub_type, data);
    }

    EVENT(on_group_member_decrease, const cq::GroupMemberDecreaseEvent &e) {
        json data = e;
        APP(hook_notice_event, e.notice_type, e.sub_type, data);
    }

    EVENT(on_group_member_increase, const cq::GroupMemberIncreaseEvent &e) {
        json data = e;
        APP(hook_notice_event, e.notice_type, e.sub_type, data);
    }

    EVENT(on_friend_add, const cq::FriendAddEvent &e) {
        json data = e;
        APP(hook_notice_event, e.notice_type, e.sub_type, data);
    }

    EVENT(on_friend_request, const cq::FriendRequestEvent &e) {
        json data = e;
        APP(hook_request_event, e.request_type, e.sub_type, data);
    }

    EVENT(on_group_request, const cq::GroupRequestEvent &e) {
        json data = e;
        APP(hook_request_event, e.request_type, e.sub_type, data);
    }
}
