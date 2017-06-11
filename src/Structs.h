#pragma once

#include "app.h"

#include <jansson/jansson.h>

struct Stranger {
    int64_t user_id;
    std::string nickname;
    int32_t sex;
    int32_t age;

    json_t *json() const {
        auto data = json_object();
        json_object_set_new(data, "user_id", json_integer(user_id));
        json_object_set_new(data, "nickname", json_string(nickname.c_str()));
        json_object_set_new(data, "sex", json_string(sex == 0 ? "male" : (sex == 1 ? "female" : "unknown")));
        json_object_set_new(data, "age", json_integer(age));
        return data;
    }
};

struct Group {
    int64_t group_id;
    std::string group_name;

    json_t *json() const {
        auto data = json_object();
        json_object_set_new(data, "group_id", json_integer(group_id));
        json_object_set_new(data, "group_name", json_string(group_name.c_str()));
        return data;
    }
};

struct GroupMember {
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
    int32_t unfriendly;
    std::string title;
    int32_t title_expire_time;
    int32_t card_changeable;

    json_t *json() const {
        auto data = json_object();
        json_object_set_new(data, "group_id", json_integer(group_id));
        json_object_set_new(data, "user_id", json_integer(user_id));
        json_object_set_new(data, "nickname", json_string(nickname.c_str()));
        json_object_set_new(data, "card", json_string(card.c_str()));
        json_object_set_new(data, "sex", json_string(sex == 0 ? "male" : (sex == 1 ? "female" : "unknown")));
        json_object_set_new(data, "age", json_integer(age));
        json_object_set_new(data, "area", json_string(area.c_str()));
        json_object_set_new(data, "join_time", json_integer(join_time));
        json_object_set_new(data, "last_sent_time", json_integer(last_sent_time));
        json_object_set_new(data, "level", json_string(level.c_str()));
        json_object_set_new(data, "role", json_string(role == 3 ? "owner" : (role == 2 ? "admin" : (role == 1 ? "member" : "unknown"))));
        json_object_set_new(data, "unfriendly", json_boolean(unfriendly));
        json_object_set_new(data, "title", json_string(title.c_str()));
        json_object_set_new(data, "title_expire_time", json_integer(title_expire_time));
        json_object_set_new(data, "card_changeable", json_boolean(card_changeable));
        return data;
    }
};
