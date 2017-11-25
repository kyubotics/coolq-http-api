// 
// pack_class.cpp : Implement Pack class.
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

#include "./pack_class.h"

using namespace std;

void Pack::check_enough(const size_t needed) const {
    if (this->size() < needed) {
        throw BytesNotEnoughError(
            (string("there aren't enough bytes to pop (") + to_string(needed) + " bytes needed)").c_str());
    }
}

string Pack::pop_string() {
    const auto len = pop_int<int16_t>();
    if (len == 0) {
        return string();
    }
    check_enough(len);
    auto result = string_from_coolq(this->bytes_.substr(this->curr_, len));
    this->curr_ += len;
    return result;
}

bytes Pack::pop_bytes(const size_t len) {
    auto result = this->bytes_.substr(this->curr_, len);
    this->curr_ += len;
    return result;
}

bytes Pack::pop_token() {
    return this->pop_bytes(this->pop_int<int16_t>());
}

bool Pack::pop_bool() {
    return static_cast<bool>(pop_int<int32_t>());
}
