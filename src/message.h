// 
// Message.h : Define structure and methods of Message class.
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

#define MSG_FMT_STRING "string"
#define MSG_FMT_ARRAY "array"

class Message {
public:
    Message(const str &msg_str) : msg_str_(msg_str), msg_json_(nullptr) {}

    Message(json_t *msg_json) : msg_str_(""), msg_json_(msg_json) {
        json_incref(msg_json);
    }

    Message(const Message &other) {
        this->msg_str_ = other.msg_str_;
        this->msg_json_ = other.msg_json_;
        json_incref(this->msg_json_);
    }

    /**
     * Convert message to a string, which can be sent directly (CQ codes will be enhanced as OUTCOMING).
     */
    str process_outcoming() const;

    /**
     * Convert received message to a json value in the specified format.
     * 
     * \param msg_fmt: the desired message format, if not passed in, use the one in config file
     */
    json_t *process_incoming(str msg_fmt = "") const;

private:
    str msg_str_;
    json_t *msg_json_;
};
