// 
// Pack.cpp : Implement methods of Pack class.
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

#include "Pack.h"

#include "helpers.h"

static void check_enough(size_t remained, size_t needed) {
    if (remained < needed) {
        throw BytesNotEnoughError(str("there aren't enough bytes to pop (")
            + str(needed) + " bytes needed)");
    }
}

static void get_integer_and_move_forward(bytes &bytes, size_t &start, size_t &size, void *dst) {
    auto s = bytes.substr(start, size);
    start += size;
    reverse(s.begin(), s.end());
    memcpy(dst, s.data(), size);
}

int16_t Pack::pop_int16() {
    auto size = sizeof(int16_t);
    check_enough(this->size(), size);

    int16_t result;
    get_integer_and_move_forward(this->bytes_, this->curr_, size, &result);
    return result;
}

int32_t Pack::pop_int32() {
    auto size = sizeof(int32_t);
    check_enough(this->size(), size);

    int32_t result;
    get_integer_and_move_forward(this->bytes_, this->curr_, size, &result);
    return result;
}

int64_t Pack::pop_int64() {
    auto size = sizeof(int64_t);
    check_enough(this->size(), size);

    int64_t result;
    get_integer_and_move_forward(this->bytes_, this->curr_, size, &result);
    return result;
}

str Pack::pop_string() {
    check_enough(this->size(), 2);
    auto len = pop_int16();
    if (len == 0) {
        return str();
    }
    check_enough(this->size(), len);
    auto result = decode(this->bytes_.substr(this->curr_, len), Encoding::ANSI);
    this->curr_ += len;
    return result;
}

bytes Pack::pop_bytes(size_t len) {
    auto result = this->bytes_.substr(this->curr_, len);
    this->curr_ += len;
    return result;
}

bytes Pack::pop_token() {
    return this->pop_bytes(this->pop_int16());
}

bool Pack::pop_bool() {
    return static_cast<bool>(pop_int32());
}
