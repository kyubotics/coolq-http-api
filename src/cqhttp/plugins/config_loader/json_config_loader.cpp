#include "./json_config_loader.h"

#include <fstream>

using namespace std;
using namespace cq;

namespace cqhttp::plugins {
    static bool load_config(json &config, const std::string &filename, const initializer_list<string> &sections = {}) {
        auto loaded = false;

        const auto filepath = filename + ".json";
        if (ifstream f(ansi(filepath)); f.is_open()) {
            json j;
            try {
                f >> j;

                if (sections.size() > 0) {
                    for (const auto &sec : sections) {
                        try {
                            const auto child = j.at(sec);
                            config.update(child);
                        } catch (...) {
                            // section may not exist, skip it
                        }
                    }
                } else {
                    // no sections are specified, use the root object
                    config.update(j);
                }

                loaded = true;
            } catch (...) {
                // failed to read
            }
        }

        return loaded;
    }

    void JsonConfigLoader::hook_enable(Context &ctx) {
        auto loaded = ctx.config->get_bool("_LOADED", false);

        const auto login_id_str = to_string(api::get_login_user_id());

        if (!loaded) {
            // try <appid>\config.json
            const auto filename = dir::app() + "config";
            loaded = load_config(ctx.config->raw, filename, initializer_list<string>{"general", login_id_str});
        }

        if (!loaded) {
            // try <appid>\config\general.json
            const auto config_dir = dir::app() + "config\\";
            loaded = load_config(ctx.config->raw, config_dir + "general");

            // try <appid>\config\<user_id>.json
            loaded = load_config(ctx.config->raw, config_dir + login_id_str) || loaded;
        }

        ctx.config->raw["_LOADED"] = loaded;
        ctx.next();
    }
} // namespace cqhttp::plugins
