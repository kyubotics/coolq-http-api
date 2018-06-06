//
// Define a set of simplified classes and interfaces
// for extensions.
//
// The key points here is to restrict the mutability of data
// passed to extensions, and to reduce dependencies as many as possible.
//
// An extensions is supposed to include this header file
// at the very beginning of it main source file.
//

#pragma once

#include <functional>
#include <nlohmann/json.hpp>

namespace cqhttp::extension {
    struct ActionResult {
        struct Codes {
            static const int OK = 0;
            static const int ASYNC = 1;

            static const int DEFAULT_ERROR = 100; // arguments missed or definitely invalid
            static const int INVALID_DATA = 102; // the data that CoolQ returns is invalid
            static const int OPERATION_FAILED = 103; // insufficient user privilege or filesystem error
        };

        int code = Codes::DEFAULT_ERROR;
        nlohmann::json data;

        ActionResult() = default;
        ActionResult(const int code, const nlohmann::json &data = nullptr) : code(code), data(data) {}
    };

    struct Context {
        Context(const nlohmann::json &config) : config(config) {}

        const nlohmann::json &config;

        ActionResult call_action(const std::string &action,
                                 const nlohmann::json &params = nlohmann::json::object()) const {
            if (__bridge.call_action) return __bridge.call_action(action, params);
            return ActionResult(ActionResult::Codes::DEFAULT_ERROR);
        }

        std::string get_config_string(const std::string &key, const std::string &default_val = "") const {
            return __bridge.get_config_string ? __bridge.get_config_string(key, default_val) : default_val;
        }

        int64_t get_config_integer(const std::string &key, const int64_t default_val = 0) const {
            return __bridge.get_config_integer ? __bridge.get_config_integer(key, default_val) : default_val;
        }

        bool get_config_bool(const std::string &key, const bool default_val = false) const {
            return __bridge.get_config_bool ? __bridge.get_config_bool(key, default_val) : default_val;
        }

        struct Bridge {
            std::function<ActionResult(const std::string &action, const nlohmann::json &params)> call_action;
            std::function<std::string(const std::string &key, const std::string &default_val)> get_config_string;
            std::function<int64_t(const std::string &key, int64_t default_val)> get_config_integer;
            std::function<bool(const std::string &key, bool default_val)> get_config_bool;
        };

        Bridge __bridge;
    };

    struct EventContext : Context {
        EventContext(const nlohmann::json &config, const nlohmann::json &data) : Context(config), data(data) {}
        const nlohmann::json &data;
    };

    struct ActionContext : Context {
        ActionContext(const nlohmann::json &config, const std::string &action, const nlohmann::json &params,
                      ActionResult &result)
            : Context(config), action(action), params(params), result(result) {}

        const std::string &action;
        const nlohmann::json &params;
        ActionResult &result;
    };

    struct Extension {
        Extension() = default;
        virtual ~Extension() = default;

        struct Info {
            std::string name{};
            std::string version{};
            int build_number{};
            std::string description{};
        };

        virtual Info info() const = 0;
        virtual void hook_enable(Context &ctx) {}
        virtual void hook_disable(Context &ctx) {}
        virtual void hook_after_event(EventContext &ctx) {}
        virtual void hook_missed_action(ActionContext &ctx) {}
    };

    typedef std::shared_ptr<Extension> (*ExtensionCreator)();
    typedef ExtensionCreator (*ExtensionCreatorGetter)();

#define PLUGIN_CREATOR                                                                                     \
    std::shared_ptr<Extension> __create_extension();                                                       \
    extern "C" __declspec(dllexport) ExtensionCreator GetExtensionCreator() { return __create_extension; } \
    std::shared_ptr<Extension> __create_extension()
} // namespace cqhttp::extension
