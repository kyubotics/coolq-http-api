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

#include <map>

#define MSG_FMT_STRING "string"
#define MSG_FMT_ARRAY "array"

class Message {
public:
    static str escape(const str &msg);
    static str unescape(const str &msg);

    Message(const str &msg_str);
    Message(json_t *msg_json);

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

    struct Segment {
        str type;
        std::map<str, str> data;

        const static int ENHANCE_INCOMING = 0;
        const static int ENHANCE_OUTCOMING = 1;

        Segment enhanced(int mode = ENHANCE_OUTCOMING) const;
    };

private:
    std::vector<Segment> segments_;
};
