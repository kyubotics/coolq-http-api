#include "./log_class.h"

using namespace std;

extern optional<Sdk> sdk;

mutex Log::log_mutex_;

void Log::log(const int level, const string &tag, const string &msg) {
    if (sdk) {
        unique_lock<mutex> lock(log_mutex_);
        sdk->add_log(level, tag, msg);
    }
}
