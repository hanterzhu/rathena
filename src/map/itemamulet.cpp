// Copyright (c) Pandas Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "itemamulet.hpp"

#include <unordered_map>
#include <vector>
#include <algorithm>
#include <memory>

#include "pc.hpp"
#include "itemdb.hpp"

#include <common/database.hpp>
#include <common/nullpo.hpp>
#include <common/utils.hpp>
#include <common/showmsg.hpp>

extern short current_equip_item_index;

AmuletProperties amulet_properties_db;

//************************************
// Method:		amulet_is
// Description:	给定一个道具编号确认它是否为一个护身符类型的道具
// Parameter:	t_itemid nameid
// Returns:		bool 返回 true 则表示该道具为护身符
//************************************
bool amulet_is(t_itemid nameid) {
	struct item_data* item = itemdb_search(nameid);
	return item->type == IT_AMULET;
}

//************************************
// Method:		amulet_type
// Description:	给定一个道具编号返回它的道具类型, 此函数用于 clif 给客户端发数据前使用
// Parameter:	t_itemid nameid
// Returns:		int
//************************************
int amulet_type(t_itemid nameid) {
	return (amulet_is(nameid) ? IT_ETC : itemdb_search(nameid)->type);
}

//************************************
// Method:		amulet_is_firstone
// Description:	给定的 item 是否为该角色身上同类型护身符中的第一个
// Parameter:	map_session_data * sd
// Parameter:	struct item * item
// Parameter:	int amount
// Returns:		bool 返回 true 表示这是同类护身符中的第一个
//************************************
bool amulet_is_firstone(map_session_data *sd, struct item *item, int amount) {
 	nullpo_retr(false, sd);
 	nullpo_retr(false, item);

	if (!sd || !item)
		return false;

	amount = cap_value(amount, 0, MAX_AMOUNT);
	if (item && item->nameid == 0 || amount <= 0)
		return false;

	if (!amulet_is(item->nameid))
		return false;

	bool is_firstone = true;
	for (int i = 0; i < MAX_INVENTORY; i++) {
		if (sd->inventory.u.items_inventory[i].nameid == item->nameid) {
			is_firstone = false;
		}
	}

	return is_firstone;
}

//************************************
// Method:		amulet_is_lastone
// Description: 判断删除掉指定数量的护身符道具后, 角色身上就不存在其他同类护身符了
// Parameter:	map_session_data * sd
// Parameter:	int n
// Parameter:	int amount
// Returns:		bool 返回 true 表示这次删除后，角色身上就不存在同类的护身符道具了
//************************************
bool amulet_is_lastone(map_session_data *sd, int n, int amount) {
	nullpo_retr(false, sd);

	amount = cap_value(amount, 0, MAX_AMOUNT);
	if (!sd || !sd->inventory_data[n] || amount <= 0)
		return false;

    t_itemid item_id = sd->inventory_data[n]->nameid;
	if (!amulet_is(item_id))
		return false;

    uint16 count = 0;

    for (int i = 0; i < MAX_INVENTORY; i++) {
        if (sd->inventory.u.items_inventory[i].nameid == item_id) {
            count += sd->inventory.u.items_inventory[i].amount;
        }
    }

	return ((count - amount) <= 0);
}

//************************************
// Method:		amulet_apply_additem
// Description:	添加新的护身符道具时, 根据需要重算角色的能力
// Parameter:	map_session_data * sd
// Parameter:	int n
// Parameter:	bool is_firstone
// Returns:		void
//************************************
void amulet_apply_additem(map_session_data *sd, int n, bool is_firstone) {
	nullpo_retv(sd);

	if (!sd || !sd->inventory_data[n])
		return;

	struct item_data *item = sd->inventory_data[n];
	if (!amulet_is(item->nameid))
		return;

	if (is_firstone && item->equip_script) {
		// 如果这是背包中出现的第一个同种类护身符，那么触发"穿戴脚本"
		run_script(item->equip_script, 0, sd->bl.id, 0);
		status_calc_pc(sd, SCO_NONE);
	}
	else if (item->script) {
		// 否则如果有"使用脚本"的话, 也需要重算一下角色能力
		status_calc_pc(sd, SCO_NONE);
	}
}

//************************************
// Method:		amulet_apply_delitem
// Description:	删除老的护身符道具时, 根据需要重算角色的能力
// Parameter:	map_session_data * sd
// Parameter:	int n
// Parameter:	bool is_lastone
// Returns:		void
//************************************
void amulet_apply_delitem(map_session_data *sd, int n, bool is_lastone) {
	nullpo_retv(sd);

	if (!sd || !sd->inventory_data[n])
		return;

	struct item_data *item = sd->inventory_data[n];
	if (!amulet_is(item->nameid))
		return;

	if (is_lastone && item->unequip_script) {
		// 如果移除的护身符是身上同种类护身符的最后一个，那么触发"卸装脚本"
		short save_current_equip_item_index = current_equip_item_index;
		current_equip_item_index = n;
		run_script(item->unequip_script, 0, sd->bl.id, 0);
		current_equip_item_index = save_current_equip_item_index;
		status_calc_pc(sd, SCO_NONE);
	}
	else if (item->script){
		// 否则如果有"使用脚本"的话, 也需要重算一下角色能力
		status_calc_pc(sd, SCO_NONE);
	}
}

//************************************
// Method:      amulet_status_calc
// Description: 重新应用角色身上全部护身符道具的脚本效果
// Parameter:   map_session_data * sd
// Parameter:   uint8 opt
// Returns:     void
//************************************
void amulet_status_calc(map_session_data *sd, uint8 opt) {
	nullpo_retv(sd);

	if (!sd || sd->extend.amulet_calculating)
		return;

	sd->extend.amulet_calculating = true;

	short save_current_equip_item_index = current_equip_item_index;

    std::unordered_map<t_itemid, std::vector<std::shared_ptr<s_amulet_cal>>> amulet_map;

	for (uint32 i = 0; i < MAX_INVENTORY; i++) {
        if (!sd || !sd->inventory_data[i])
			continue;

		if (!amulet_is(sd->inventory_data[i]->nameid))
			continue;

		std::shared_ptr<item_data> id = item_db.find(sd->inventory_data[i]->nameid);
		if (id && itemdb_isNoEquip(id.get(), sd->bl.m))
			continue;

		if (opt&SCO_FIRST && sd->inventory_data[i]->equip_script) {
			run_script(sd->inventory_data[i]->equip_script, 0, sd->bl.id, 0);
		}

		if (sd->inventory_data[i]->script) {
            uint32 nameid = sd->inventory_data[i]->nameid;
            uint32 maxstack = sd->inventory_data[i]->extend.amulet_max_stack;
            uint32 priority = sd->inventory_data[i]->extend.amulet_priority;
            uint32 group_maxstack = sd->inventory_data[i]->extend.amulet_group_max_stack;
            auto it = amulet_map.find(sd->inventory_data[i]->extend.amulet_group);
            if (it != amulet_map.end()) {
                std::vector<std::shared_ptr<s_amulet_cal>> amulets = it->second; // 获取值
                //如有就加数量，没有就新增
                bool flag = false;
                for (const auto &amulet: amulets) {
                    if (amulet->nameid == nameid) {
                        amulet->amount += sd->inventory.u.items_inventory[i].amount;
                        flag = true;
                        break;
                    }
                }
                if (!flag) {
                    amulet_map[sd->inventory_data[i]->extend.amulet_group].push_back(std::make_shared<s_amulet_cal>(s_amulet_cal{nameid, priority, group_maxstack, maxstack, sd->inventory.u.items_inventory[i].amount, i, sd->inventory_data[i]->script}));
                }
            } else {
                //key不存在
                amulet_map[sd->inventory_data[i]->extend.amulet_group].push_back(std::make_shared<s_amulet_cal>(s_amulet_cal{nameid, priority, group_maxstack, maxstack, sd->inventory.u.items_inventory[i].amount, i, sd->inventory_data[i]->script}));
            }

		}
	}

    for (const auto& pair : amulet_map) {
        std::vector<std::shared_ptr<s_amulet_cal>> amulets = pair.second;
        std::sort(amulets.begin(), amulets.end(),
                  [](const std::shared_ptr<s_amulet_cal> & a, const std::shared_ptr<s_amulet_cal> & b) {
                      return a->priority < b->priority;
                  });
        uint32 totalMax = amulets.at(0)->group_maxstack;
        uint32 currentTotal = 0;
        for (const auto &amulet: amulets) {
            uint32 effectiveCount = min(amulet->maxstack, amulet->amount);
            current_equip_item_index = amulet->index;
            if (currentTotal + effectiveCount > totalMax) {
                effectiveCount = max(0, totalMax - currentTotal);
            }
            for (uint16 k = 0; k < effectiveCount; k++) {
                run_script(amulet->script, 0, sd->bl.id, 0);
                if (!sd->extend.amulet_calculating) {
                    current_equip_item_index = save_current_equip_item_index;
                    return;
                }
            }
            currentTotal += effectiveCount;
            if (currentTotal >= totalMax) {
                break;
            }
        }
    }
	current_equip_item_index = save_current_equip_item_index;
	sd->extend.amulet_calculating = false;
}

//************************************
// Method:      getDefaultLocation
// Description: 获取 YAML 数据文件的具体路径
// Returns:     const std::string
//************************************
const std::string AmuletProperties::getDefaultLocation() {
    return std::string(db_path) + "/extend/amulet_properties.yml";
}

//************************************
// Method:      parseBodyNode
// Description: 解析 Body 节点的主要处理函数
// Parameter:   const ryml::NodeRef & node
// Returns:     uint64
//************************************
uint64 AmuletProperties::parseBodyNode(const ryml::NodeRef& node) {
    uint32 group;

    if (!this->asUInt32(node, "Group", group)) {
        return 0;
    }

    uint32 gruop_maxstack;
    if (this->nodeExists(node, "MaxStack")) {
        if (!this->asUInt32(node, "MaxStack", gruop_maxstack)) {
            this->invalidWarning(node["MaxStack"], "Group \"%d\" maxstack set default to 1\n", group);
            gruop_maxstack = 1;
        }
    }

    if (this->nodeExists(node, "Items")) {
        const auto& subNode = node["Items"];
        for (const auto& subit : subNode) {
            uint32 nameid = 0;

            if (!this->asUInt32(subit, "ItemID", nameid)) {
                return 0;
            }

            if (!item_db.exists(nameid)) {
                this->invalidWarning(node, "Unknown item ID %hu in Amulet Properties Database.\n", nameid);
                return 0;
            }

            auto properties_item = this->find(nameid);
            bool exists = properties_item != nullptr;

            if (!exists) {
                properties_item = std::make_shared<s_amulet_properties>();
                properties_item->nameid = nameid;
                properties_item->group = group;
                properties_item->group_maxstack = gruop_maxstack;
            }

            if (this->nodeExists(subit, "MaxStack")) {
                uint32 maxstack;

                if (!this->asUInt32(subit, "MaxStack", maxstack)) {
                    this->invalidWarning(subit["MaxStack"], "ItemId \"%d\" maxstack set default to 1\n", nameid);
                    properties_item->maxstack = 1;
                } else {
                    properties_item->maxstack = maxstack;
                }
            }

            if (this->nodeExists(subit, "Priority")) {
                uint32 priority;
                if (!this->asUInt32(subit, "Priority", priority)) {
                    this->invalidWarning(subit["Priority"], "ItemId \"%d\" priority set default to 1\n", nameid);
                    properties_item->priority = 1;
                } else {
                    properties_item->priority = priority;
                }
            }

            if (!exists) {
                this->put(properties_item->nameid, properties_item);
            }

        }
    }

    return 1;
}

//************************************
// Method:      parsePropertiesToItemDB
// Description: 为 ItemDatabase 中的道具设置护身符
// Access:      public
// Parameter:   ItemDatabase & item_db
// Returns:     void
//************************************
void AmuletProperties::parsePropertiesToItemDB(ItemDatabase& item_db) {
    for (const auto& it : item_db) {
        auto value = this->getProperty(it.first);
        auto item = it.second;

        if (value) {
            item->extend.amulet_group = value->group;
            item->extend.amulet_group_max_stack = value->group_maxstack;
            item->extend.amulet_priority = value->priority;
            item->extend.amulet_max_stack = value->maxstack;
        }

    }
}

//************************************
// Method:      getProperty
// Description: 获取一个道具编号的特殊属性
// Access:      public
// Parameter:   uint32 nameid
//************************************
std::shared_ptr<s_amulet_properties> AmuletProperties::getProperty(uint32 nameid) {
    return amulet_properties_db.find(nameid);
}
