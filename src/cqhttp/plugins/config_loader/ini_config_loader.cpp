#include "./ini_config_loader.h"

#include <boost/filesystem.hpp>
#include <boost/property_tree/ini_parser.hpp>

using namespace std;
using namespace cq;
namespace fs = boost::filesystem;

namespace cqhttp::plugins {
    static void override_config(json &config, const boost::property_tree::ptree &pt,
                                const initializer_list<string> &sections) {
        for (const auto &sec : sections) {
            try {
                const auto pt_child = pt.get_child(sec);
                for (const auto &item : pt_child) {
                    config[item.first] = item.second.get_value<string>();
                }
            } catch (...) {
                // section may not exist, skip it
            }
        }
    }

    static bool load_config(json &config, const string &filename,
                            const initializer_list<string> &sections = {"general"}) {
        const auto exts = {".cfg", ".ini"};

        boost::property_tree::ptree pt;
        auto loaded = false;

        for (const auto &ext : exts) {
            const auto filepath = filename + ext;
            if (fs::is_regular_file(ansi(filepath))) {
                try {
                    read_ini(ansi(filepath), pt);
                    override_config(config, pt, sections);
                    loaded = true;
                    break;
                } catch (...) {
                    // failed to read
                }
            }
        }

        return loaded;
    }

    void IniConfigLoader::hook_enable(Context &ctx) {
        auto loaded = ctx.config->get_bool("_LOADED", false);

        const auto login_id_str = to_string(api::get_login_user_id());

        if (!loaded) {
            // try <appid>\config.(cfg|ini)
            const auto filename = dir::app() + "config";
            loaded = load_config(ctx.config->raw, filename, initializer_list<string>{"general", login_id_str});
        }

        if (!loaded) {
            // try <appid>\config\general.(cfg|ini)
            const auto config_dir = dir::app() + "config\\";
            loaded = load_config(ctx.config->raw, config_dir + "general");

            // try <appid>\config\<user_id>.(cfg|ini)
            loaded = load_config(ctx.config->raw, config_dir + login_id_str, {"general", login_id_str}) || loaded;
        }

        ctx.config->raw["_LOADED"] = loaded;
        ctx.next();
    }
} // namespace cqhttp::plugins
