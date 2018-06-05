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
            if (__call_action) {
                return __call_action(action, params);
            }
            return ActionResult(ActionResult::Codes::DEFAULT_ERROR);
        }

        std::function<ActionResult(const std::string &, const nlohmann::json &)> __call_action;
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
