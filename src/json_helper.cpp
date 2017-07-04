#include "common.h"

namespace rc {
    void to_json(json &j, const str &s) {
        j = encode(s);
    }

    void from_json(const json &j, str &s) {
        if (j.is_string()) {
            s = j.get<std::string>();
        }
    }
}
