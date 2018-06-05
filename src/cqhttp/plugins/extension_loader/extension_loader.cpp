#include "./extension_loader.h"

#include <boost/filesystem.hpp>

#include "cqsdk/utils/string.h"

using namespace std;
namespace fs = boost::filesystem;
namespace ext = cqhttp::extension;

namespace cqhttp::plugins {
    static const auto TAG = u8"扩展";

    static ext::ActionResult call_action_wrapper(const std::string &action, const nlohmann::json &params) {
        const auto result = call_action(action, params);
        return ext::ActionResult(result.code, result.data);
    }

    static ext::Context convert_context(Context &context) {
        ext::Context ext_ctx(context.config->raw);
        ext_ctx.__call_action = call_action_wrapper;
        return ext_ctx;
    }

    template <typename E>
    static ext::EventContext convert_context(EventContext<E> &context) {
        ext::EventContext ext_ctx(context.config->raw, context.data);
        ext_ctx.__call_action = call_action_wrapper;
        return ext_ctx;
    }

    static ext::ActionContext convert_context(ActionContext &context, ext::ActionResult &result) {
        ext::ActionContext ext_ctx(context.config->raw, context.action, context.params, result);
        ext_ctx.__call_action = call_action_wrapper;
        return ext_ctx;
    }

    void ExtensionLoader::hook_enable(Context &ctx) {
        for (auto it = fs::directory_iterator(ansi(cq::dir::app("extensions"))); it != fs::directory_iterator(); ++it) {
            const auto filename = string_decode(it->path().filename().string(), cq::utils::Encoding::ANSI);
            if (boost::ends_with(filename, ".dll") && !boost::starts_with(filename, "_")) {
                // we should load this dll
                auto succeeded = false;
                ext::Extension::Info extension_info;

                try {
                    const auto tmp_ext_dir = cq::dir::app("tmp") + "extensions\\";
                    fs::create_directory(ansi(tmp_ext_dir));

                    const auto tmp_dll_path = tmp_ext_dir + filename;
                    const auto ansi_tmp_dll_path = ansi(tmp_dll_path);
                    copy_file(*it, ansi_tmp_dll_path, boost::filesystem::copy_option::overwrite_if_exists);

                    const auto hdl = LoadLibraryA(ansi_tmp_dll_path.c_str());
                    if (hdl) {
                        const auto get_creator =
                            reinterpret_cast<ext::ExtensionCreatorGetter>(GetProcAddress(hdl, "GetExtensionCreator"));
                        if (get_creator) {
                            dll_handles_.push_back(hdl);
                            const auto creator = get_creator();
                            const auto plugin = creator();
                            extension_info = plugin->info();
                            extensions_.push_back(plugin);
                            succeeded = true;
                        }
                    }
                } catch (fs::filesystem_error &) {
                }

                if (succeeded) {
                    logging::info_success(
                        TAG,
                        "扩展 " + extension_info.name + " (" + filename + ") v" + extension_info.version + " 加载成功");
                } else {
                    logging::info(TAG, "扩展 " + filename + " 加载失败");
                }
            }
        }

        auto ext_ctx = convert_context(ctx);
        for (auto &extension : extensions_) {
            extension->hook_enable(ext_ctx);
        }

        ctx.next();
    }

    void ExtensionLoader::hook_disable(Context &ctx) {
        auto ext_ctx = convert_context(ctx);
        for (auto &extension : extensions_) {
            extension->hook_disable(ext_ctx);
        }
        extensions_.clear();

        for (auto &hdl : dll_handles_) {
            FreeLibrary(hdl);
        }
        dll_handles_.clear();

        fs::remove_all(ansi(cq::dir::app("tmp") + "extensions\\"));

        ctx.next();
    }

    void ExtensionLoader::hook_after_event(EventContext<cq::Event> &ctx) {
        auto ext_ctx = convert_context(ctx);
        for (auto &extension : extensions_) {
            extension->hook_after_event(ext_ctx);
        }

        ctx.next();
    }

    void ExtensionLoader::hook_missed_action(ActionContext &ctx) {
        ext::ActionResult ext_result(ctx.result.code, ctx.result.data);
        auto ext_ctx = convert_context(ctx, ext_result);
        for (auto &extension : extensions_) {
            extension->hook_missed_action(ext_ctx);
        }

        ctx.result.code = ext_ctx.result.code;
        ctx.result.data = ext_ctx.result.data;

        ctx.next();
    }
} // namespace cqhttp::plugins
