#include "./extension_loader.h"

#include <filesystem>

#include "cqsdk/utils/string.h"

using namespace std;
namespace fs = std::filesystem;
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

    static void touch_extension(ext::Extension &extension) {
        const auto ext_name = extension.info().name;
        const auto tag = u8"扩展: " + ext_name;
        extension.logger.debug = [=](const std::string &msg) { logging::debug(tag, msg); };
        extension.logger.info = [=](const std::string &msg) { logging::info(tag, msg); };
        extension.logger.info_success = [=](const std::string &msg) { logging::info_success(tag, msg); };
        extension.logger.warning = [=](const std::string &msg) { logging::warning(tag, msg); };
        extension.logger.error = [=](const std::string &msg) { logging::error(tag, msg); };
    }

    void ExtensionLoader::hook_enable(Context &ctx) {
        enable_ = ctx.config->get_bool("use_extension", false);

        if (enable_) {
            string extensions_dir;
            string tmp_ext_dir;
            try {
                extensions_dir = cq::dir::app("extensions");
                tmp_ext_dir = cq::dir::app_per_account("tmp") + "extensions\\";
                fs::create_directories(ansi(tmp_ext_dir));
            } catch (fs::filesystem_error &) {
                logging::error(TAG, u8"创建目录失败，无法加载扩展");
                goto HOOK_ENABLE_END;
            }

            for (auto it = fs::directory_iterator(ansi(extensions_dir)); it != fs::directory_iterator(); ++it) {
                const auto filename = string_decode(it->path().filename().string(), cq::utils::Encoding::ANSI);
                if (boost::ends_with(filename, ".dll") && !boost::starts_with(filename, "_")) {
                    // we should load this dll
                    auto succeeded = false;
                    string failure_reason;
                    ext::Extension::Info extension_info;

                    try {
                        const auto tmp_dll_path = tmp_ext_dir + filename;
                        const auto ansi_tmp_dll_path = ansi(tmp_dll_path);
                        copy_file(*it, ansi_tmp_dll_path, fs::copy_options::overwrite_existing);

                        const auto hdl = LoadLibraryA(ansi_tmp_dll_path.c_str());
                        if (hdl) {
                            const auto get_creator = reinterpret_cast<ext::ExtensionCreatorGetter>(
                                GetProcAddress(hdl, "GetExtensionCreator"));
                            if (get_creator) {
                                dll_handles_.push_back(hdl);
                                const auto creator = get_creator();
                                const auto extension = creator();
                                touch_extension(*extension);
                                extension_info = extension->info();
                                extensions_.push_back(extension);
                                succeeded = true;
                            } else {
                                failure_reason = u8"DLL 文件没有正确导出 GetExtensionCreator";
                            }
                        } else {
                            failure_reason = u8"LoadLibrary 失败，GetLastError = " + to_string(GetLastError());
                        }
                    } catch (fs::filesystem_error &) {
                        failure_reason = u8"拷贝 DLL 文件到临时目录失败";
                    }

                    if (succeeded) {
                        logging::info_success(TAG,
                                              u8"扩展 " + extension_info.name + " (" + filename + ") v"
                                                  + extension_info.version + u8" 加载成功");
                    } else {
                        logging::warning(TAG, u8"扩展 " + filename + u8" 加载失败，原因：" + failure_reason);
                    }
                }
            }

            auto ext_ctx = convert_context(ctx);
            for (auto &extension : extensions_) {
                extension->on_create(ext_ctx);
            }
        }

    HOOK_ENABLE_END:
        ctx.next();
    }

    void ExtensionLoader::hook_disable(Context &ctx) {
        if (enable_) {
            auto ext_ctx = convert_context(ctx);
            for (auto &extension : extensions_) {
                extension->on_destroy(ext_ctx);
            }
            extensions_.clear();

            for (auto &hdl : dll_handles_) {
                FreeLibrary(hdl);
            }
            dll_handles_.clear();

            try {
                fs::remove_all(ansi(cq::dir::app_per_account("tmp") + "extensions\\"));
            } catch (fs::filesystem_error &) {
            }
        }

        ctx.next();
    }

    void ExtensionLoader::hook_after_event(EventContext<cq::Event> &ctx) {
        if (enable_) {
            auto ext_ctx = convert_context(ctx);
            for (auto &extension : extensions_) {
                extension->on_event(ext_ctx);
            }
        }

        ctx.next();
    }

    void ExtensionLoader::hook_missed_action(ActionContext &ctx) {
        if (enable_) {
            ext::ActionResult ext_result(ctx.result.code, std::move(ctx.result.data));
            auto ext_ctx = convert_context(ctx, ext_result);
            for (auto &extension : extensions_) {
                extension->on_missed_action(ext_ctx);
            }

            ctx.result.code = ext_ctx.result.code;
            ctx.result.data = std::move(ext_ctx.result.data);
        }

        ctx.next();
    }
} // namespace cqhttp::plugins
