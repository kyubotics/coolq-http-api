// 
// Pack.h : Define structure of Pack class,
// which is to help parse information from raw bytes.
// 
// Copyright (C) 2017  Richard Chien <richardchienthebest@gmail.com>
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
// 

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
