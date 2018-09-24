#include "./event.h"

#include "cqhttp/core/core.h"

using namespace std;

namespace cqhttp {
    template <typename E, typename OnEventFunc>
    static void emit_event(const E &event, json &data, const OnEventFunc on_event) {
        // emit the event in thread pool
        app.push_async_task([=] {
            const auto e = event;
            auto d = data;
            app.on_before_event(e, d);
            (app.*on_event)(e, d);
            app.on_after_event(e, d);
        });
    }

    void emit_event(const cq::MessageEvent &event, json &data) {
        emit_event(event, data, &Application::on_message_event);
    }

    void emit_event(const cq::NoticeEvent &event, json &data) {
        emit_event(event, data, &Application::on_notice_event);
    }

    void emit_event(const cq::RequestEvent &event, json &data) {
        emit_event(event, data, &Application::on_request_event);
    }

    void emit_event(const cqhttp::MetaEvent &event, json &data) {
        emit_event(event, data, &Application::on_meta_event);
    }

    void emit_lifecycle_meta_event(const MetaEvent::SubType sub_type) {
        LifecycleMetaEvent e;
        e.sub_type = sub_type;
        emit_event(e);
    }

    void emit_heartbeat_meta_event(const json status) {
        HeartbeatMetaEvent e;
        e.status = status;
        emit_event(e);
    }
} // namespace cqhttp
