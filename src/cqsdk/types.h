#pragma once

#include "./common.h"

#include "./utils/binpack.h"

namespace cq {
    enum class Sex { MALE = 0, FEMALE = 1, UNKNOWN = 255 };

    enum class GroupRole { MEMBER = 1, ADMIN = 2, OWNER = 3 };

    struct User {
        const static size_t MIN_SIZE = 18;

        int64_t user_id = 0;
        std::string nickname;
        Sex sex = Sex::UNKNOWN;
        int32_t age = 0;

        static User from_bytes(const std::string &bytes) {
            auto pack = utils::BinPack(bytes);
            User stranger;
            try {
                stranger.user_id = pack.pop_int<int64_t>();
                stranger.nickname = pack.pop_string();
                stranger.sex = static_cast<Sex>(pack.pop_int<int32_t>());
                stranger.age = pack.pop_int<int32_t>();
            } catch (std::out_of_range &) {}
            return stranger;
        }
    };

    struct Group {
        const static size_t MIN_SIZE = 10;

        int64_t group_id = 0;
        std::string group_name;

        static Group from_bytes(const std::string &bytes) {
            auto pack = utils::BinPack(bytes);
            Group group;
            try {
                group.group_id = pack.pop_int<int64_t>();
                group.group_name = pack.pop_string();
            } catch (std::out_of_range &) {}
            return group;
        }
    };

    struct GroupMember : User {
        const static size_t MIN_SIZE = 58;

        int64_t group_id = 0;
        // int64_t user_id; // from User
        // std::string nickname; // from User
        std::string card;
        // Sex sex = Sex::UNKNOWN; // from User
        // int32_t age; // from User
        std::string area;
        int32_t join_time = 0;
        int32_t last_sent_time = 0;
        std::string level;
        GroupRole role = GroupRole::MEMBER;
        bool unfriendly = false;
        std::string title;
        int32_t title_expire_time = 0;
        bool card_changeable = false;

        static GroupMember from_bytes(const std::string &bytes) {
            auto pack = utils::BinPack(bytes);
            GroupMember member;
            try {
                member.group_id = pack.pop_int<int64_t>();
                member.user_id = pack.pop_int<int64_t>();
                member.nickname = pack.pop_string();
                member.card = pack.pop_string();
                member.sex = static_cast<Sex>(pack.pop_int<int32_t>());
                member.age = pack.pop_int<int32_t>();
                member.area = pack.pop_string();
                member.join_time = pack.pop_int<int32_t>();
                member.last_sent_time = pack.pop_int<int32_t>();
                member.level = pack.pop_string();
                member.role = static_cast<GroupRole>(pack.pop_int<int32_t>());
                member.unfriendly = pack.pop_bool();
                member.title = pack.pop_string();
                member.title_expire_time = pack.pop_int<int32_t>();
                member.card_changeable = pack.pop_bool();
            } catch (std::out_of_range &) {}
            return member;
        }
    };

    struct Anonymous {
        const static size_t MIN_SIZE = 12;

        int64_t id = 0;
        std::string name;
        std::string token;

        static Anonymous from_bytes(const std::string &bytes) {
            auto pack = utils::BinPack(bytes);
            Anonymous anonymous;
            try {
                anonymous.id = pack.pop_int<int64_t>();
                anonymous.name = pack.pop_string();
                anonymous.token = pack.pop_token();
            } catch (std::out_of_range &) {}
            return anonymous;
        }
    };

    struct File {
        const static size_t MIN_SIZE = 20;

        std::string id;
        std::string name;
        int64_t size = 0;
        int64_t busid = 0;

        static File from_bytes(const std::string &bytes) {
            auto pack = utils::BinPack(bytes);
            File file;
            try {
                file.id = pack.pop_string();
                file.name = pack.pop_string();
                file.size = pack.pop_int<int64_t>();
                file.busid = pack.pop_int<int64_t>();
            } catch (std::out_of_range &) {}
            return file;
        }
    };
}
