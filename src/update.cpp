#include "./update.h"

#include "app.h"

using namespace std;
using boost::algorithm::ends_with;

static string update_source() {
    if (ends_with(config.update_source, "/")) {
        return config.update_source;
    }
    return config.update_source + "/";
}

static string latest_url() {
    return update_source() + "latest/" + config.update_channel + ".json";
}

static string version_info_url(const string &version, int build_number) {
    return update_source() + "versions/" + version + "_build_" + to_string(build_number) + "/info.json";
}

optional<tuple<bool, string, int, string>> check_update() {
    const auto data_opt = get_remote_json(latest_url());
    if (!data_opt) {
        return nullopt;
    }

    auto data = data_opt.value();
    if (data.is_object()
        && data.find("version") != data.end() && data["version"].is_string()
        && data.find("build") != data.end() && data["build"].is_number_integer()) {
        auto version = data["version"].get<string>();
        auto build_number = data["build"].get<int>();
        string description;
        if (const auto info = get_remote_json(version_info_url(version, build_number)).value_or(nullptr);
            info.is_object() && info.find("description") != info.end() && info["description"].is_string()) {
            description = info["description"].get<string>();
        }

        auto result = make_tuple(false, version, build_number, description);
        if (data["build"].get<int>() > CQAPP_BUILD_NUMBER) {
            get<0>(result) = true;
        }
        return result;
    }
    return nullopt;
}
