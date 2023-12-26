// Copyright (c) Pandas Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "itemamulet.hpp"

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
// Description:	����һ�����߱��ȷ�����Ƿ�Ϊһ����������͵ĵ���
// Parameter:	t_itemid nameid
// Returns:		bool ���� true ���ʾ�õ���Ϊ�����
//************************************
bool amulet_is(t_itemid nameid) {
	struct item_data* item = itemdb_search(nameid);
	return item->type == IT_AMULET;
}

//************************************
// Method:		amulet_type
// Description:	����һ�����߱�ŷ������ĵ�������, �˺������� clif ���ͻ��˷�����ǰʹ��
// Parameter:	t_itemid nameid
// Returns:		int
//************************************
int amulet_type(t_itemid nameid) {
	return (amulet_is(nameid) ? IT_ETC : itemdb_search(nameid)->type);
}

//************************************
// Method:		amulet_is_firstone
// Description:	������ item �Ƿ�Ϊ�ý�ɫ����ͬ���ͻ�����еĵ�һ��
// Parameter:	map_session_data * sd
// Parameter:	struct item * item
// Parameter:	int amount
// Returns:		bool ���� true ��ʾ����ͬ�໤����еĵ�һ��
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

	uint32 i = MAX_INVENTORY;
	bool is_firstone = true;
	for (i = 0; i < MAX_INVENTORY; i++) {
		if (sd->inventory.u.items_inventory[i].nameid == item->nameid) {
			is_firstone = false;
		}
	}

	return is_firstone;
}

//************************************
// Method:		amulet_is_lastone
// Description: �ж�ɾ����ָ�������Ļ�������ߺ�, ��ɫ���ϾͲ���������ͬ�໤�����
// Parameter:	map_session_data * sd
// Parameter:	int n
// Parameter:	int amount
// Returns:		bool ���� true ��ʾ���ɾ���󣬽�ɫ���ϾͲ�����ͬ��Ļ����������
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
// Description:	����µĻ��������ʱ, ������Ҫ�����ɫ������
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
		// ������Ǳ����г��ֵĵ�һ��ͬ���໤�������ô����"�����ű�"
		run_script(item->equip_script, 0, sd->bl.id, 0);
		status_calc_pc(sd, SCO_NONE);
	}
	else if (item->script) {
		// ���������"ʹ�ýű�"�Ļ�, Ҳ��Ҫ����һ�½�ɫ����
		status_calc_pc(sd, SCO_NONE);
	}
}

//************************************
// Method:		amulet_apply_delitem
// Description:	ɾ���ϵĻ��������ʱ, ������Ҫ�����ɫ������
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
		// ����Ƴ��Ļ����������ͬ���໤��������һ������ô����"жװ�ű�"
		short save_current_equip_item_index = current_equip_item_index;
		current_equip_item_index = n;
		run_script(item->unequip_script, 0, sd->bl.id, 0);
		current_equip_item_index = save_current_equip_item_index;
		status_calc_pc(sd, SCO_NONE);
	}
	else if (item->script){
		// ���������"ʹ�ýű�"�Ļ�, Ҳ��Ҫ����һ�½�ɫ����
		status_calc_pc(sd, SCO_NONE);
	}
}

//************************************
// Method:      amulet_status_calc
// Description: ����Ӧ�ý�ɫ����ȫ����������ߵĽű�Ч��
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

    std::unordered_map<t_itemid, int> countMap;

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
            countMap[sd->inventory_data[i]->nameid] += sd->inventory.u.items_inventory[i].amount;
		}
	}

    for (const auto& pair : countMap) {
        for (uint32 i = 0; i < MAX_INVENTORY; i++) {
            if (!sd || !sd->inventory_data[i])
                continue;

            if (pair.first != sd->inventory_data[i]->nameid)
                continue;

            current_equip_item_index = i;
            std::shared_ptr<item_data> id = item_db.find(sd->inventory_data[i]->nameid);
            int amount = id->extend.amulet_max_stack < pair.second ? id->extend.amulet_max_stack : pair.second;
            for (uint16 k = 0; k < amount; k++) {
                run_script(sd->inventory_data[i]->script, 0, sd->bl.id, 0);
                if (!sd->extend.amulet_calculating) {
                    current_equip_item_index = save_current_equip_item_index;
                    return;
                }
            }
            break; //����һ�ξ���
        }
    }
	current_equip_item_index = save_current_equip_item_index;
	sd->extend.amulet_calculating = false;
}

//************************************
// Method:      getDefaultLocation
// Description: ��ȡ YAML �����ļ��ľ���·��
// Returns:     const std::string
//************************************
const std::string AmuletProperties::getDefaultLocation() {
    return std::string(db_path) + "/amulet_properties.yml";
}

//************************************
// Method:      parseBodyNode
// Description: ���� Body �ڵ����Ҫ������
// Parameter:   const ryml::NodeRef & node
// Returns:     uint64
//************************************
uint64 AmuletProperties::parseBodyNode(const ryml::NodeRef& node) {
    uint32 nameid = 0;

    if (!this->asUInt32(node, "ItemID", nameid)) {
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
    }

    properties_item->maxstack = 1;

    if (this->nodeExists(node, "MaxStack")) {
        uint32 maxstack;

        if (!this->asUInt32(node, "MaxStack", maxstack)) {
            this->invalidWarning(node["MaxStack"], "ItemId \"%d\" maxstack set default to 1\n", nameid);
        } else {
            properties_item->maxstack = maxstack;
        }
    }

    if (!exists) {
        this->put(properties_item->nameid, properties_item);
    }

    return 1;
}

//************************************
// Method:      parsePropertiesToItemDB
// Description: Ϊ ItemDatabase �еĵ������û����
// Access:      public
// Parameter:   ItemDatabase & item_db
// Returns:     void
//************************************
void AmuletProperties::parsePropertiesToItemDB(ItemDatabase& item_db) {
    for (const auto& it : item_db) {
        auto value = this->getProperty(it.first);
        auto item = it.second;

        if (value) {
            item->extend.amulet_max_stack = value->maxstack;
        }

    }
}

//************************************
// Method:      getProperty
// Description: ��ȡһ�����߱�ŵ���������
// Access:      public
// Parameter:   uint32 nameid
//************************************
std::shared_ptr<s_amulet_properties> AmuletProperties::getProperty(uint32 nameid) {
    return amulet_properties_db.find(nameid);
}
