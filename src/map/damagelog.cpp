//
// Created by dylan on 2024/1/9.
//

#include "damagelog.hpp"

#include <unordered_map>

#include <common/nullpo.hpp>
#include <common/showmsg.hpp>

#include "mob.hpp"
#include "pet.hpp"
#include "homunculus.hpp"
#include "mercenary.hpp"
#include "elemental.hpp"
#include "unit.hpp"
#include "map.hpp"

damage_log_map* log_damage_aggregation(damage_log_map* log_map , bool combine) {
    nullpo_retr(nullptr, log_map);

    if (!combine) {
        return log_map;
    }

    damage_log_map* master_map = new damage_log_map;
    damage_log_map* slave_map = new damage_log_map;

    for (auto& pair : *log_map) {
        if (!pair.second->master_id) {
            s_damage_log_ptr item = std::make_shared<s_damage_log>();
            item->master_id = 0;
            item->damage = pair.second->damage;
            master_map->insert(std::make_pair(pair.first, item));
        } else {
            slave_map->insert(pair);
        }
    }

    for (auto& pair : *slave_map) {
        auto it = master_map->find(pair.second->master_id);
        if (it != master_map->end()) {
            it->second->damage += pair.second->damage;
        } else {
            s_damage_log_ptr item = std::make_shared<s_damage_log>();
            item->master_id = 0;
            item->damage = pair.second->damage;
            master_map->insert(std::make_pair(pair.second->master_id, item));
        }
    }

    return master_map;
}

int log_damage_key(struct block_list* bl) {
    // 若是玩家则使用角色编号作为键
    // 否则使用 BlockID (AKA: GameID) 作为键
    nullpo_ret(bl);
    if ( bl->type == BL_PC) {
        return ((TBL_PC*)bl)->status.char_id;
    }
    return bl->id;
}


damage_log_map* log_damage_getmap(struct block_list* bl, e_damage_log_type type) {
    nullpo_retr(nullptr, bl);

    unit_data *ud = unit_bl2ud(bl);

    if (type == DMGLOG_INFLICT) {
        return ud->extend.damage_map_inflict;
    } else {
        return ud->extend.damage_map_receive;
    }

}

void set_log_damage(struct block_list* bl) {
    nullpo_retv(bl);
    unit_data *ud = unit_bl2ud(bl);
    ud->extend.log_flag = true;
}

bool need_log_damage(struct block_list* bl) {
    nullpo_retr(false, bl);
    unit_data *ud = unit_bl2ud(bl);
    if (bl->type == BL_MOB) {
        set_log_damage(bl);
    }
    if(!ud->extend.damage_map_receive) {
        ud->extend.damage_map_receive = new damage_log_map;
    }
    if(!ud->extend.damage_map_inflict) {
        ud->extend.damage_map_inflict = new damage_log_map;
    }
    return ud->extend.log_flag;
}

void do_log_damage(struct block_list* src, struct block_list* target, e_damage_log_type type, int damage) {
    nullpo_retv(src);
    nullpo_retv(target);
    if (!need_log_damage(src))
        return;
    int src_id = log_damage_key(src);
    int target_id = log_damage_key(target);

    if (src_id == target_id)
        return;

    damage_log_map* map = log_damage_getmap(src,type);

    bool flag = false;
    for (const auto& pair : *map) {
        if (pair.first == target_id) {
            pair.second->damage += (int64)damage;
            flag = true;
            ShowInfo("%d 增加伤害类型: %d, id: %d, master id: %d, damage: %d\n", src_id, type, target_id, pair.second->master_id, pair.second->damage);
            break;
        }
    }
    if (!flag) {
        s_damage_log_ptr item = std::make_shared<s_damage_log>();
        int master_id;
        TBL_MOB* md;
        TBL_HOM* hd;
        TBL_PET* pd;
        TBL_MER* mc;
        TBL_ELEM* ed;
        if (type == DMGLOG_INFLICT) {
            switch (src->type) {
                case BL_MOB:
                    if ((md = map_id2md(src_id))) {
                        master_id = md->master_id;
                    }
                    break;
                case BL_HOM:
                    if ((hd = map_id2hd(src_id))) {
                        master_id = hd->homunculus.char_id;
                    }
                    break;
                case BL_PET:
                    if ((pd = map_id2pd(src_id))) {
                        master_id = pd->pet.char_id;
                    }
                    break;
                case BL_MER:
                    if ((mc =  map_id2mc(src_id))) {
                        master_id = mc->mercenary.char_id;
                    }
                    break;
                case BL_ELEM :
                    if ((ed = map_id2ed(src_id))) {
                        master_id = ed->elemental.char_id;
                    }
                    break;
                default:
                    master_id = 0;
                    break;
            }
        } else {
            switch (target->type) {
                case BL_MOB:
                    if ((md = map_id2md(target_id))) {
                        master_id = md->master_id;
                    }
                    break;
                case BL_HOM:
                    if ((hd = map_id2hd(target_id))) {
                        master_id = hd->homunculus.char_id;
                    }
                    break;
                case BL_PET:
                    if ((pd = map_id2pd(target_id))) {
                        master_id = pd->pet.char_id;
                    }
                    break;
                case BL_MER:
                    if ((mc =  map_id2mc(target_id))) {
                        master_id = mc->mercenary.char_id;
                    }
                    break;
                case BL_ELEM :
                    if ((ed = map_id2ed(target_id))) {
                        master_id = ed->elemental.char_id;
                    }
                    break;
                default:
                    master_id = 0;
                    break;
            }
        }
        item->master_id = master_id;
        item->damage = (int64)damage;
        ShowInfo("%d 首次伤害类型: %d, id: %d, master id: %d, damage: %d\n", src_id, type, target_id, item->master_id, item->damage);
        map->insert(std::make_pair(target_id, item));
    }
}

int64 query_log_damage(struct block_list* src, int target_id, e_damage_log_type type, bool combine) {
    nullpo_retr(0, src);

    damage_log_map* origin_map = log_damage_getmap(src, type);

    damage_log_map* result_map = log_damage_aggregation(origin_map, combine);

    nullpo_retr(0, result_map);

    auto it = result_map->find(target_id);

    if (it == result_map->end()) {
        return 0;
    }

    return it->second->damage;

}

bool log_damage_asc(std::pair<int, s_damage_log_ptr>& l, std::pair<int, s_damage_log_ptr>& r) {
    return l.second->damage < r.second->damage;
}

bool log_damage_desc(std::pair<int, s_damage_log_ptr>& l, std::pair<int, s_damage_log_ptr>& r) {
    return l.second->damage > r.second->damage;
}

void log_damage_free(struct block_list* bl) {
    nullpo_retv(bl);
    unit_data *ud = unit_bl2ud(bl);
    if (ud->extend.damage_map_inflict) {
        delete ud->extend.damage_map_inflict;
        ud->extend.damage_map_inflict = nullptr;
    }

    if (ud->extend.damage_map_receive) {
        delete ud->extend.damage_map_receive;
        ud->extend.damage_map_receive = nullptr;
    }

}

void log_damage_reset(struct block_list* bl) {
    nullpo_retv(bl);
    unit_data *ud = unit_bl2ud(bl);

    if (ud->extend.damage_map_inflict) {
        ud->extend.damage_map_inflict->clear();
    }

    if (ud->extend.damage_map_receive) {
        ud->extend.damage_map_receive->clear();
    }


}



