//
// Created by dylan on 2024/1/9.
//

#pragma once

#include <unordered_map>

#include <common/cbasetypes.hpp>

#include "map.hpp"

struct s_damage_log {
    int master_id;
    int64 damage;
};

enum e_damage_log_type {
    DMGLOG_RECEIVE = 0,
    DMGLOG_INFLICT
};

typedef std::shared_ptr<s_damage_log> s_damage_log_ptr;
typedef std::unordered_map<int, s_damage_log_ptr> damage_log_map;

void set_log_damage(struct block_list* bl);
bool need_log_damage(struct block_list* bl);
void do_log_damage(struct block_list* src, struct block_list* target, e_damage_log_type type, int damage);
int64 query_log_damage(struct block_list* src, int target_id, e_damage_log_type type, bool combine);
damage_log_map* log_damage_getmap(struct block_list* bl, e_damage_log_type type);
damage_log_map* log_damage_aggregation(damage_log_map* log_map , bool combine);
bool log_damage_asc(std::pair<int, s_damage_log_ptr>& l, std::pair<int, s_damage_log_ptr>& r);
bool log_damage_desc(std::pair<int, s_damage_log_ptr>& l, std::pair<int, s_damage_log_ptr>& r);
void log_damage_free(struct block_list* bl);
void log_damage_reset(struct block_list* bl);

#define log_damage_receive(src, target, damage) do_log_damage(src, target, DMGLOG_RECEIVE, damage)
#define log_damage_inflict(src, target, damage) do_log_damage(src, target, DMGLOG_INFLICT, damage)



