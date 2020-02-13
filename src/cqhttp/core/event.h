#pragma once

#include "cqhttp/core/common.h"

namespace cqhttp {
    struct MetaEvent : cq::Event {
        MetaEvent() { type = cq::event::UNKNOWN; }

        enum Type {
            UNKNOWN,
            LIFECYCLE,
            HEARTBEAT,
        };

        enum SubType {
            SUBTYPE_DEFAULT,

            LIFECYCLE_ENABLE,
            LIFECYCLE_DISABLE,
            LIFECYCLE_CONNECT,
        };

        Type meta_event_type = UNKNOWN;
        SubType sub_type = SUBTYPE_DEFAULT;
    };

    inline void to_json(json &j, const MetaEvent::Type &t) {
        switch (t) {
        case MetaEvent::LIFECYCLE:
            j = "lifecycle";
            break;
        case MetaEvent::HEARTBEAT:
            j = "heartbeat";
            break;
        default:
            j = "unknown";
            break;
        }
    }

    struct LifecycleMetaEvent final : MetaEvent {
        LifecycleMetaEvent() : MetaEvent() { meta_event_type = LIFECYCLE; }
    };

    inline void to_json(json &j, const LifecycleMetaEvent &e) {
        const auto sub_type_str = [&]() -> std::string {
            switch (e.sub_type) {
            case MetaEvent::LIFECYCLE_ENABLE:
                return "enable";
            case MetaEvent::LIFECYCLE_DISABLE:
                return "disable";
            case MetaEvent::LIFECYCLE_CONNECT:
                return "connect";
            default:
                return "unknown";
            }
        }();

        j = {
            {"post_type", "meta_event"},
            {"meta_event_type", e.meta_event_type},
            {"sub_type", sub_type_str},
        };
    }

    struct HeartbeatMetaEvent final : MetaEvent {
        HeartbeatMetaEvent() : MetaEvent() { meta_event_type = HEARTBEAT; }

        json status;
        int64_t interval; // in millisecond
    };

    inline void to_json(json &j, const HeartbeatMetaEvent &e) {
        j = {
            {"post_type", "meta_event"},
            {"meta_event_type", e.meta_event_type},
            {"status", e.status},
            {"interval", e.interval},
        };
    }

    void emit_event(const cq::MessageEvent &event, json &data);
    void emit_event(const cq::NoticeEvent &event, json &data);
    void emit_event(const cq::RequestEvent &event, json &data);
    void emit_event(const cqhttp::MetaEvent &event, json &data);

    template <typename E, typename = typename std::enable_if<std::is_base_of<cq::Event, E>::value>::type>
    void emit_event(E event) {
        emit_event(event, json(event));
    }

    void emit_lifecycle_meta_event(const MetaEvent::SubType sub_type);
    void emit_heartbeat_meta_event(const json status, const int64_t interval);
} // namespace cqhttp
