#pragma once

#include "cqhttp/core/common.h"

namespace cqhttp::action {
    class Params {
    public:
        Params() : params_(nullptr) {}
        explicit Params(const json &j) : params_(j) {}

        /**
         * Return std::nullopt if the key does not exist.
         */
        std::optional<json> get(const std::string &key) const {
            if (const auto it = params_.find(key); it != params_.end()) {
                return *it;
            }
            return std::nullopt;
        }

        template <typename Type>
        std::optional<Type> get(const std::string &key) const {
            auto v = get(key);
            if (v) {
                try {
                    return v->get<Type>();
                } catch (json::exception &) {
                    // type doesn't match
                }
            }
            return std::nullopt;
        }

        std::string get_string(const std::string &key, const std::string &default_val = "") const {
            return get<std::string>(key).value_or(default_val);
        }

//        cq::Message get_message(const std::string &key = "message",
//                                const std::string &auto_escape_key = "auto_escape") const {
//            if (auto msg_opt = get(key); msg_opt && !msg_opt->is_null()) {
//                if (msg_opt->is_string()) {
//                    if (get_bool(auto_escape_key, false)) {
//                        msg_opt = Message::escape(msg_opt->get<std::string>());
//                    }
//                }
//                return Message(msg_opt.value()).process_outward();
//            }
//            return "";
//        }

//        int64_t get_integer(const std::string &key, const int64_t default_val = 0) const;
//        bool get_bool(const std::string &key, const bool default_val = false) const;

    private:
        json params_;
    };
}
