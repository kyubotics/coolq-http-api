// 
// message_class.h : Define Message class.
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

class Message {
public:
    static std::string escape(std::string msg);
    static std::string unescape(std::string msg);

    Message(const std::string &msg_str);
    Message(const json &msg_json);

    using Direction = int;

    struct Directions {
        static const Direction OUTWARD = 0;
        static const Direction INWARD = 1;
    };

    using Format = std::string;

    struct Formats {
        static const Format STRING;
        static const Format ARRAY;
    };

    /**
     * Convert message to a string, which can be sent directly (CQ codes will be enhanced as OUTWARD).
     */
    std::string process_outward() const;

    /**
     * Convert received message to a json value in the specified format.
     * 
     * \param fmt: the desired message format, if not passed in, use the one in config file
     */
    json Message::process_inward(std::optional<Format> fmt = std::nullopt) const;

    struct Segment {
        std::string type;
        std::map<std::string, std::string> data;

        std::vector<Segment> enhanced(const Direction direction = Directions::OUTWARD) const;
    };

private:
    std::list<Segment> segments_;
};

void to_json(json &j, const Message::Segment &seg);
void from_json(const json &j, Message::Segment &seg);
