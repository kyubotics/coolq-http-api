#pragma once

#include "common.h"

class BytesNotEnoughError : Exception {
    using Exception::Exception;
};

class Pack {
public:
    Pack() : bytes_(""), curr_(0) {}
    Pack(bytes &b) : bytes_(b), curr_(0) {}

    size_t size() const { return this->bytes_.size() - this->curr_; }
    bool empty() const { return size() == 0; }

    int16_t pop_int16();
    int32_t pop_int32();
    int64_t pop_int64();
    str pop_string();
    bytes pop_bytes(size_t len);
    bytes pop_token();
    bool pop_bool();

private:
    bytes bytes_;
    size_t curr_;
};
