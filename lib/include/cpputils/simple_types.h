#pragma once

#include <cstdint>

namespace rc {
    typedef int64_t index_t;

    struct slice {
        index_t start;
        index_t stop;
        index_t stride;
        bool use_default_stop;

        explicit slice(index_t start = 0) : slice(start, 0, 1) {
            this->use_default_stop = true;
        }

        explicit slice(index_t start, index_t stop, index_t stride = 1) :
            start(start), stop(stop), stride(stride) {
            this->use_default_stop = false;
        }
    };
}
