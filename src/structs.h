// 
// structs.h : Define some structures that represent certain objects of QQ.
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

#include "utils/pack_class.h"

struct Stranger {
    const static size_t MIN_SIZE = 18;

    int64_t user_id;
    std::string nickname;
    int32_t sex;
    int32_t age;

    json json() const {
        return {
            {"user_id", user_id},
            {"nickname", nickname},
            {"sex", sex == 0 ? "male" : sex == 1 ? "female" : "unknown"},
            {"age", age}
        };
    }

    static Stranger from_bytes(bytes &bytes) {
        auto pack = Pack(bytes);
        Stranger stranger;
        stranger.user_id = pack.pop_int64();
        stranger.nickname = pack.pop_string();
        stranger.sex = pack.pop_int32();
        stranger.age = pack.pop_int32();
        return stranger;
    }
};

struct Group {
    const static size_t MIN_SIZE = 10;

    int64_t group_id;
    std::string group_name;

    json json() const {
        return {
            {"group_id", group_id},
            {"group_name", group_name}
        };
    }

    static Group from_bytes(bytes &bytes) {
        auto pack = Pack(bytes);
        Group group;
        group.group_id = pack.pop_int64();
        group.group_name = pack.pop_string();
        return group;
    }
};

struct GroupMember {
    const static size_t MIN_SIZE = 58;

    int64_t group_id;
    int64_t user_id;
    std::string nickname;
    std::string card;
    int32_t sex;
    int32_t age;
    std::string area;
    int32_t join_time;
    int32_t last_sent_time;
    std::string level;
    int32_t role;
    bool unfriendly;
    std::string title;
    int32_t title_expire_time;
    bool card_changeable;

    json json() const {
        return {
            {"group_id", group_id},
            {"user_id", user_id},
            {"nickname", nickname},
            {"card", card},
            {"sex", sex == 0 ? "male" : sex == 1 ? "female" : "unknown"},
            {"age", age},
            {"area", area},
            {"join_time", join_time},
            {"last_sent_time", last_sent_time},
            {"level", level},
            {"role",role == 3 ? "owner" : role == 2 ? "admin" : role == 1 ? "member" : "unknown"},
            {"unfriendly", unfriendly},
            {"title", title},
            {"title_expire_time", title_expire_time},
            {"card_changeable", card_changeable}
        };
    }

    static GroupMember from_bytes(bytes &bytes) {
        auto pack = Pack(bytes);
        GroupMember member;
        member.group_id = pack.pop_int64();
        member.user_id = pack.pop_int64();
        member.nickname = pack.pop_string();
        member.card = pack.pop_string();
        member.sex = pack.pop_int32();
        member.age = pack.pop_int32();
        member.area = pack.pop_string();
        member.join_time = pack.pop_int32();
        member.last_sent_time = pack.pop_int32();
        member.level = pack.pop_string();
        member.role = pack.pop_int32();
        member.unfriendly = pack.pop_bool();
        member.title = pack.pop_string();
        member.title_expire_time = pack.pop_int32();
        member.card_changeable = pack.pop_bool();
        return member;
    }
};

struct Anonymous {
    const static size_t MIN_SIZE = 12;

    int64_t id;
    std::string name;
    bytes token;

    json json() const {
        return {
            {"id", id},
            {"name", name}
        };
    }

    static Anonymous from_bytes(bytes &bytes) {
        auto pack = Pack(bytes);
        Anonymous anonymous;
        anonymous.id = pack.pop_int64();
        anonymous.name = pack.pop_string();
        anonymous.token = pack.pop_token();
        return anonymous;
    }
};

struct GroupFile {
    const static size_t MIN_SIZE = 20;

    std::string id;
    std::string name;
    int64_t size;
    int64_t busid;

    json json() const {
        return {
            {"id", id},
            {"name", name},
            {"size", size},
            {"busid", busid}
        };
    }

    static GroupFile from_bytes(bytes &bytes) {
        auto pack = Pack(bytes);
        GroupFile file;
        file.id = pack.pop_string();
        file.name = pack.pop_string();
        file.size = pack.pop_int64();
        file.busid = pack.pop_int64();
        return file;
    }
};
