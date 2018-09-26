#include "./ca_cert_downloader.h"

#include <filesystem>

#include "cqhttp/core/core.h"
#include "cqhttp/core/helpers.h"
#include "cqhttp/utils/http.h"

using namespace std;
namespace fs = std::filesystem;

namespace cqhttp::plugins {
    static const auto TAG = u8"CA证书";

    using utils::http::download_file;
    using helpers::get_asset_url;

    void CACertDownloader::hook_enable(Context &ctx) {
        const auto cacert_file = cq::dir::app("tmp") + "cacert.pem";
        auto ok = true;

        if (not fs::exists(ansi(cacert_file))) {
            logging::info(TAG,
                          u8"正在下载 CA 证书文件，这只会在第一次启动时进行，如果耗时较长，请在配置文件中更换更新源");
            if (download_file(get_asset_url("cacert.pem"), cacert_file, true)) {
                logging::info_success(TAG, u8"下载 CA 证书文件成功");
            } else {
                logging::warning(TAG, u8"下载 CA 证书文件失败，可能导致无法访问 HTTPS 和 WSS");
                ok = false;
            }
        }

        if (ok) {
            app.store().put("cacert_file", cacert_file);
        }

        ctx.next();
    }
} // namespace cqhttp::plugins
