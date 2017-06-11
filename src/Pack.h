#pragma once

#include "app.h"

class BytesNotEnoughError : Exception {
    using Exception::Exception;
};

class Pack {
public:
    Pack() : bytes_(""), curr_(bytes_.begin()) {}
    Pack(bytes b) : bytes_(b), curr_(bytes_.begin()) {}

    size_t size() const { return bytes_.end() - curr_; }
    bool empty() const { return size() == 0; }

    int16_t pop_int16();
    int32_t pop_int32();
    int64_t pop_int64();
    str pop_string();

private:
    bytes bytes_;
    std::string::iterator curr_;
};
