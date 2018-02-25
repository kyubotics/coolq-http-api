#pragma once

#include "cqhttp/core/common.h"
#include "cqhttp/utils/string.h"

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

        cq::Message get_message(const std::string &key = "message",
                                const std::string &auto_escape_key = "auto_escape") const {
            if (auto msg_json_opt = get(key); msg_json_opt) {
                try {
                    if (msg_json_opt->is_string() && get_bool(auto_escape_key, false)) {
                        return cq::MessageSegment::text(msg_json_opt->get<std::string>());
                    }

                    return msg_json_opt->get<cq::Message>();
                } catch (json::exception &) {
                    return cq::Message();
                }
            }
            return cq::Message();
        }

        int64_t get_integer(const std::string &key, const int64_t default_val = 0) const {
            auto result = default_val;
            if (auto v_opt = get(key); v_opt && v_opt->is_string()) {
                try {
                    result = stoll(v_opt->get<std::string>());
                } catch (std::invalid_argument &) {
                    // invalid integer string
                }
            } else if (v_opt->is_number_integer()) {
                result = v_opt->get<int64_t>();
            }
            return result;
        }

        bool get_bool(const std::string &key, const bool default_val = false) const {
            auto result = default_val;
            if (auto v_opt = get(key); v_opt && v_opt->is_string()) {
                result = utils::to_bool(v_opt->get<std::string>(), default_val);
            } else if (v_opt->is_boolean()) {
                result = v_opt->get<bool>();
            }
            return result;
        }

    private:
        json params_;
    };

    class Result {
    public:
    private:
    };
}
