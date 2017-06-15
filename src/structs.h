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

#include "Pack.h"

struct Stranger {
    const static size_t MIN_SIZE = 18;

    int64_t user_id;
    str nickname;
    int32_t sex;
    int32_t age;

    json_t *json() const {
        auto data = json_object();
        json_object_set_new(data, "user_id", json_integer(user_id));
        json_object_set_new(data, "nickname", json_string(nickname.c_str()));
        json_object_set_new(data, "sex", json_string(sex == 0 ? "male" : sex == 1 ? "female" : "unknown"));
        json_object_set_new(data, "age", json_integer(age));
        return data;
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
    str group_name;

    json_t *json() const {
        auto data = json_object();
        json_object_set_new(data, "group_id", json_integer(group_id));
        json_object_set_new(data, "group_name", json_string(group_name.c_str()));
        return data;
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
    str nickname;
    str card;
    int32_t sex;
    int32_t age;
    str area;
    int32_t join_time;
    int32_t last_sent_time;
    str level;
    int32_t role;
    bool unfriendly;
    str title;
    int32_t title_expire_time;
    bool card_changeable;

    json_t *json() const {
        auto data = json_object();
        json_object_set_new(data, "group_id", json_integer(group_id));
        json_object_set_new(data, "user_id", json_integer(user_id));
        json_object_set_new(data, "nickname", json_string(nickname.c_str()));
        json_object_set_new(data, "card", json_string(card.c_str()));
        json_object_set_new(data, "sex", json_string(sex == 0 ? "male" : sex == 1 ? "female" : "unknown"));
        json_object_set_new(data, "age", json_integer(age));
        json_object_set_new(data, "area", json_string(area.c_str()));
        json_object_set_new(data, "join_time", json_integer(join_time));
        json_object_set_new(data, "last_sent_time", json_integer(last_sent_time));
        json_object_set_new(data, "level", json_string(level.c_str()));
        json_object_set_new(data, "role", json_string(role == 3 ? "owner" : role == 2 ? "admin" : role == 1 ? "member" : "unknown"));
        json_object_set_new(data, "unfriendly", json_boolean(unfriendly));
        json_object_set_new(data, "title", json_string(title.c_str()));
        json_object_set_new(data, "title_expire_time", json_integer(title_expire_time));
        json_object_set_new(data, "card_changeable", json_boolean(card_changeable));
        return data;
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
    str name;
    bytes token;

    json_t *json() const {
        auto data = json_object();
        json_object_set_new(data, "id", json_integer(id));
        json_object_set_new(data, "name", json_string(name.c_str()));
        return data;
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

    str id;
    str name;
    int64_t size;
    int64_t busid;

    json_t *json() const {
        auto data = json_object();
        json_object_set_new(data, "id", json_string(id.c_str()));
        json_object_set_new(data, "name", json_string(name.c_str()));
        json_object_set_new(data, "size", json_integer(size));
        json_object_set_new(data, "busid", json_integer(busid));
        return data;
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
