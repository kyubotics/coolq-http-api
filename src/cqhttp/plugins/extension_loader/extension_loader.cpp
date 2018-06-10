#include "./extension_loader.h"

#include <boost/filesystem.hpp>

#include "cqsdk/utils/string.h"

using namespace std;
namespace fs = boost::filesystem;
namespace ext = cqhttp::extension;

namespace cqhttp::plugins {
    static const auto TAG = u8"扩展";

    template <typename Ctx, typename ExtCtx>
    static void make_bridge(Ctx &ctx, ExtCtx &ext_ctx) {
        ext_ctx.__bridge.call_action = [](const std::string &action, const nlohmann::json &params) {
            const auto result = call_action(action, params);
            return ext::ActionResult(result.code, result.data);
        };
        ext_ctx.__bridge.get_config_string = [&](const std::string &key, const std::string &default_val) {
            return ctx.config->get_string(key, default_val);
        };
        ext_ctx.__bridge.get_config_integer = [&](const std::string &key, const int64_t default_val) {
            return ctx.config->get_integer(key, default_val);
        };
        ext_ctx.__bridge.get_config_bool = [&](const std::string &key, const bool default_val) {
            return ctx.config->get_bool(key, default_val);
        };
    }

    template <>
    static void make_bridge(ActionContext &ctx, ext::ActionContext &ext_ctx) {
        make_bridge(static_cast<Context &>(ctx), static_cast<ext::Context &>(ext_ctx));
        ext_ctx.__param_bridge.get_param_string = [&](const std::string &key, const std::string &default_val) {
            return ctx.params.get_string(key, default_val);
        };
        ext_ctx.__param_bridge.get_param_integer = [&](const std::string &key, const int64_t default_val) {
            return ctx.params.get_integer(key, default_val);
        };
        ext_ctx.__param_bridge.get_param_bool = [&](const std::string &key, const bool default_val) {
            return ctx.params.get_bool(key, default_val);
        };
    }

    static ext::Context convert_context(Context &ctx) {
        ext::Context ext_ctx;
        make_bridge(ctx, ext_ctx);
        return ext_ctx;
    }

    template <typename E>
    static ext::EventContext convert_context(EventContext<E> &ctx) {
        ext::EventContext ext_ctx(ctx.data);
        make_bridge(ctx, ext_ctx);
        return ext_ctx;
    }

    static ext::ActionContext convert_context(ActionContext &ctx, ext::ActionResult &result) {
        ext::ActionContext ext_ctx(ctx.action, ctx.params, result);
        make_bridge(ctx, ext_ctx);
        return ext_ctx;
    }

    void ExtensionLoader::hook_enable(Context &ctx) {
        const auto ansi_extensions_dir = ansi(cq::dir::app("extensions"));
        if (fs::is_directory(ansi_extensions_dir)) {
            for (auto it = fs::directory_iterator(ansi_extensions_dir); it != fs::directory_iterator(); ++it) {
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
                            const auto get_creator = reinterpret_cast<ext::ExtensionCreatorGetter>(
                                GetProcAddress(hdl, "GetExtensionCreator"));
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
                        logging::info_success(TAG,
                                              "扩展 " + extension_info.name + " (" + filename + ") v"
                                                  + extension_info.version + " 加载成功");
                    } else {
                        logging::info(TAG, "扩展 " + filename + " 加载失败");
                    }
                }
            }

            auto ext_ctx = convert_context(ctx);
            for (auto &extension : extensions_) {
                extension->on_create(ext_ctx);
            }
        }

        ctx.next();
    }

    void ExtensionLoader::hook_disable(Context &ctx) {
        auto ext_ctx = convert_context(ctx);
        for (auto &extension : extensions_) {
            extension->on_destroy(ext_ctx);
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
            extension->on_event(ext_ctx);
        }

        ctx.next();
    }

    void ExtensionLoader::hook_missed_action(ActionContext &ctx) {
        ext::ActionResult ext_result(ctx.result.code, std::move(ctx.result.data));
        auto ext_ctx = convert_context(ctx, ext_result);
        for (auto &extension : extensions_) {
            extension->on_missed_action(ext_ctx);
        }

        ctx.result.code = ext_ctx.result.code;
        ctx.result.data = std::move(ext_ctx.result.data);

        ctx.next();
    }
} // namespace cqhttp::plugins
