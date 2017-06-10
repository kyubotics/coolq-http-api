#include "loader.h"

#include <fstream>

#include "conf/ini.h"
#include "helpers.h"

using namespace std;

Config load_configuration(const string &filepath) {
    Config config;
    FILE *conf_file = nullptr;
    fopen_s(&conf_file, filepath.c_str(), "r");
    if (!conf_file) {
        // first init, save default config
        LOG_D("配置", "没有找到配置文件，写入默认配置");
        ofstream file(filepath);
        file << "[general]\nhost=0.0.0.0\nport=5700\npost_url=\ntoken=\n";
    } else {
        // load from config file
        LOG_D("配置", "读取配置文件");
        auto callback = [&](const char *section, const char *name, const char *value) {
            static auto login_qq_str = itos(CQ_getLoginQQ(ac));

            if (string(section) == "general" || (isnumber(section) && login_qq_str == section)) {
                string field = name;
                if (field == "host")
                    config.host = value;
                else if (field == "port")
                    config.port = atoi(value);
                else if (field == "post_url")
                    config.post_url = value;
                else if (field == "token")
                    config.token = value;
                else if (field == "pattern")
                    config.pattern = regex(value);
                else
                    return 0; /* unknown name, error */
            } else {
                return 0; /* unknown section, error */
            }
            return 1;
        };
        // pass the real lambda callback as the user data to allow capturing
        // see http://bannalia.blogspot.com/2016/07/passing-capturing-c-lambda-functions-as.html
        auto legacy_callback = [](void *real_cb, const char *section, const char *name, const char *value) {
            return (*static_cast<decltype(callback)*>(real_cb))(section, name, value);
        };
        ini_parse_file(conf_file, legacy_callback, &callback);
        fclose(conf_file);
    }
    return config;
}
