#include "./updater.h"

#include <Windows.h>
#include <filesystem>

#include "cqhttp/core/core.h"
#include "cqhttp/core/helpers.h"
#include "cqhttp/utils/gui.h"
#include "cqhttp/utils/http.h"

using namespace std;

namespace cqhttp::plugins {
    static const auto TAG = u8"更新";

    using utils::http::get_json;
    using utils::http::download_file;
    using utils::gui::message_box;
    namespace fs = std::filesystem;

    void Updater::hook_enable(Context &ctx) {
        update_source_ = helpers::get_update_source_url();
        update_channel_ = ctx.config->get_string("update_channel", "stable");
        auto_perform_update_ = ctx.config->get_bool("auto_perform_update", false);

        if (ctx.config->get_bool("auto_check_update", false)) {
            call_action(".check_update", {{"automatic", true}});
        }

        ctx.next();
    }

    void Updater::hook_missed_action(ActionContext &ctx) {
        if (ctx.action != ".check_update") {
            ctx.next();
            return;
        }

        const auto automatic = ctx.params.get_bool("automatic", false);
        app.push_async_task([automatic, this] { check_update(automatic); });
        ctx.result.code = ActionResult::Codes::ASYNC;
    }

    std::string Updater::latest_url() const {
        return update_source_ + "latest/v" + to_string(CQHTTP_VERSION_MAJOR) + "/" + update_channel_ + ".json";
    }

    std::string Updater::version_info_url(const string &version, const int build_number) const {
        return update_source_ + "versions/" + version + "(b" + to_string(build_number) + ")/info.json";
    }

    std::string Updater::version_cpk_url(const string &version, const int build_number) const {
        return update_source_ + "versions/" + version + "(b" + to_string(build_number) + ")/" CQHTTP_ID ".cpk";
    }

    // tuple<is_newer, version, build_number, description>
    optional<std::tuple<bool, string, int, string>> Updater::get_latest_version() const {
        const auto data_opt = get_json(latest_url(), true);
        if (!data_opt) {
            return nullopt;
        }

        auto data = data_opt.value();
        if (data.is_object() && data.find("version") != data.end() && data["version"].is_string()
            && data.find("build") != data.end() && data["build"].is_number_integer()) {
            auto version = data["version"].get<string>();
            auto build_number = data["build"].get<int>();
            string description;
            if (const auto info = get_json(version_info_url(version, build_number), true).value_or(nullptr);
                info.is_object() && info.find("description") != info.end() && info["description"].is_string()) {
                description = info["description"].get<string>();
            }

            auto result = make_tuple(false, version, build_number, description);
            if (data["build"].get<int>() > CQHTTP_BUILD_NUMBER) {
                get<0>(result) = true;
            }
            return result;
        }
        return nullopt;
    }

    static bool version_locked() { return fs::exists(ansi(cq::dir::app() + "version.lock")); }

    void Updater::check_update(const bool automatic) const {
        if (automatic) logging::info(TAG, u8"正在检查更新...");

        if (const auto version_opt = get_latest_version(); version_opt) {
            bool is_newer;
            string version, description;
            int build_number;
            tie(is_newer, version, build_number, description) = version_opt.value();

            if (is_newer) {
                // should update
                if (automatic) logging::info(TAG, u8"发现新版本：" + version + u8", build " + to_string(build_number));
                if (version_locked()) {
                    message_box(MB_OK | MB_ICONWARNING, u8"发现新版本：" + version
                            + u8"\r\n\r\n更新信息：\r\n"
                            + (description.empty() ? u8"无" : description)
                            + u8"\r\n\r\n当前环境下不允许自动更新，如果你正在使用 Docker，请拉取最新版本的 Docker 镜像或解除版本锁（删除 app/io.github.richardchien.coolqhttpapi/version.lock）。");
                } else {
                    if (automatic && auto_perform_update_) {
                        // auto update
                        if (perform_update(version, build_number)) {
                            logging::info_success(TAG, u8"更新成功，将在重启 酷Q 后生效");
                        } else {
                            logging::error(TAG, u8"更新失败，请检查网络连接是否通畅，或尝试更换更新源。");
                        }
                    } else {
                        // ask for manually confirming
                        auto code = message_box(MB_YESNO | MB_ICONQUESTION,
                                                u8"发现新版本：" + version + u8"\r\n\r\n更新信息：\r\n"
                                                    + (description.empty() ? u8"无" : description)
                                                    + u8"\r\n\r\n是否现在更新？");
                        if (code == IDYES) {
                            if (perform_update(version, build_number)) {
                                message_box(MB_OK | MB_ICONINFORMATION, u8"更新成功，请重启酷 Q 以生效。");
                            } else {
                                message_box(MB_OK | MB_ICONERROR,
                                            u8"更新失败，请检查网络连接是否通畅，或尝试更换更新源。");
                            }
                        }
                    }
                }
            } else {
                if (automatic)
                    logging::info(TAG, u8"没有发现更新版本");
                else
                    message_box(MB_OK | MB_ICONINFORMATION, u8"没有发现更新版本。");
            }
        } else {
            if (automatic)
                logging::error(TAG, u8"检查更新失败，请检查网络连接是否通畅，或尝试更换更新源");
            else
                message_box(MB_OK | MB_ICONERROR, u8"检查更新失败，请检查网络连接是否通畅，或尝试更换更新源。");
        }
    }

    bool Updater::perform_update(const std::string &version, const int build_number) const {
        const auto cpk_url = version_cpk_url(version, build_number);
        const auto tmp_path = cq::dir::app_per_account("tmp") + version + "_build_" + to_string(build_number) + ".cpk";
        if (!download_file(cpk_url, tmp_path, true)) {
            // download failed
            return false;
        }

        const auto local_cpk_path = cq::dir::root() + "app\\" CQHTTP_ID ".cpk";
        try {
            copy_file(ansi(tmp_path), ansi(local_cpk_path), fs::copy_options::overwrite_existing);
            fs::remove(ansi(tmp_path));
            return true;
        } catch (fs::filesystem_error &) {
            return false;
        }
    }
} // namespace cqhttp::plugins
