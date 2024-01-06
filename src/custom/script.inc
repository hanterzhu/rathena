// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

/**
 * Custom Script Commands
 * Place the body of custom script commands in this file.
 * Format:
 *	BUILDIN_FUNC(command_name)
 *	{
 *		<code>;
 *	}
 **/

//BUILDIN_FUNC(example)
//{
//	ShowInfo("buildin_example: It works!\n");
//	script_pushint(st,1);
//	return 0;
//}

BUILDIN_FUNC(get_unique_id) {
	struct map_session_data* sd;

	if (!script_rid2sd(sd))
	{
		script_pushint(st, 0);
		return SCRIPT_CMD_FAILURE;
	}

	script_pushint(st, session[sd->fd]->gepard_info.unique_id);

	return SCRIPT_CMD_SUCCESS;
}

// <mob id>
BUILDIN_FUNC(get_mob_location) {
	struct map_session_data* sd;
	if (!script_rid2sd(sd)) {
		script_pushint(st, 0);
		return SCRIPT_CMD_FAILURE;
	}
	int mob_id = script_getnum(st,2);
	if (!mobdb_checkid(mob_id)) {
		script_pushint(st, 0);
		return SCRIPT_CMD_FAILURE;
	}
    std::shared_ptr<s_mob_db> mob = mob_db.find(mob_id);
    if(!mob) {
		script_pushint(st, 0);
		return SCRIPT_CMD_FAILURE;
    }
    const std::vector<spawn_info> spawns = mob_get_spawns(mob_id);
    if (spawns.size() <= 0) {
        script_pushint(st, 0);
        return SCRIPT_CMD_FAILURE;
    } else {
        script_cleararray_pc( sd, "@mob_location$" );
        script_cleararray_pc( sd, "@mob_location_num" );
		for (int i = 0; i < spawns.size(); i++) {
            int16 mapid = map_mapindex2mapid(spawns[i].mapindex);
            if (mapid < 0) continue;
            pc_setregstr(sd,reference_uid(add_str("@mob_location$"), i), map_getmapdata(mapid)->name);
            pc_setreg(sd,reference_uid(add_str("@mob_location_num"), i), spawns[i].qty);
        }
    }
    script_pushint(st, 1);
    return SCRIPT_CMD_SUCCESS;
}

// <mob name>
BUILDIN_FUNC(mob_name2id) {
	struct map_session_data* sd;
	if (!script_rid2sd(sd)) {
		script_pushint(st, 0);
		return SCRIPT_CMD_FAILURE;
	}
	const char *str;
	str = script_getstr(st,2);
    script_pushint(st,mobdb_searchname(str));
    return SCRIPT_CMD_SUCCESS;
}

/** <道具的背包序号>,<要查看的信息类型>{,<角色编号>};
要查看的信息类型:
	0   - 物品编号
	1   - 堆叠数量
	2   - 装备的穿戴位置
	3   - 精炼值
	4   - 是否已鉴定 (若已鉴定则返回 1, 未鉴定则返回 0)
	5   - 是否已损坏 (若已损坏则返回 1, 未损坏则返回 0)
	6   - 第一个卡槽的卡片编号
	7   - 第二个卡槽的卡片编号
	8   - 第三个卡槽的卡片编号
	9   - 第四个卡槽的卡片编号
	10  - 过期时间 (Unix时间戳, 0 表示永不过期)
	11  - 唯一编号 (unique_id)

	12  - 第一个随机属性的编号 (ROA_ID)
	13  - 第二个随机属性的编号 (ROA_ID)
	14  - 第三个随机属性的编号 (ROA_ID)
	15  - 第四个随机属性的编号 (ROA_ID)
	16  - 第五个随机属性的编号 (ROA_ID)

	17  - 第一个随机属性的值 (ROA_VALUE)
	18  - 第二个随机属性的值 (ROA_VALUE)
	19  - 第三个随机属性的值 (ROA_VALUE)
	20  - 第四个随机属性的值 (ROA_VALUE)
	21  - 第五个随机属性的值 (ROA_VALUE)

	22  - 第一个随机属性的参数 (ROA_PARAM)
	23  - 第二个随机属性的参数 (ROA_PARAM)
	24  - 第三个随机属性的参数 (ROA_PARAM)
	25  - 第四个随机属性的参数 (ROA_PARAM)
	26  - 第五个随机属性的参数 (ROA_PARAM)

	27  - 道具的绑定类型
	28  - 道具的附魔评级 (Grade Level)
	29  - 道具作为第二套快速切换装备时的穿戴位置
	30  - 道具是否被标记为最喜欢的道具 (0: 未标记; 1: 已标记)

装备的穿戴位置:
	如果为 0 则表示还没穿戴在身上, 位置请参考 EQP_* 开头的常量
	查询的信息类型 2 和 29 都是 EQP_* 开头的常量.

道具的绑定类型:
	Bound_None      -   没有绑定 (与 0 等价, 但更推荐用常量)
	Bound_Account   -   绑定账号的道具, 可放个人仓库, 全账号共享
	Bound_Guild     -   绑定公会的道具, 可放公会仓库与他人共享, 拥有物品的人离开公会后物品消失
	Bound_Party     -   绑定队伍的道具, 离开队伍后消失
	Bound_Char      -   绑定角色的道具, 不能放个人仓库
**/
BUILDIN_FUNC(getinventoryinfo) {
    map_session_data *sd = nullptr;
	struct item_data *id = nullptr;
	int idx = script_getnum(st, 2);

	if (!script_charid2sd(4, sd)) {
		script_pushint(st, -1);
		return SCRIPT_CMD_SUCCESS;
	}

	struct s_storage* stor = &sd->inventory;
	struct item* inventory = stor->u.items_inventory;

	if (st->state == RERUNLINE) {
		return SCRIPT_CMD_SUCCESS;
	}

	if (!stor || !inventory) {
		ShowError("buildin_getinventoryinfo: cannot read inventory or storage data.\n");
		script_pushint(st, -1);
		return SCRIPT_CMD_FAILURE;
	}

	if (idx < 0 || idx >= stor->max_amount) {
		script_pushint(st, -1);
		return SCRIPT_CMD_SUCCESS;
	}

	if (!item_db.exists(inventory[idx].nameid)) {
		script_pushint(st, -1);
		return SCRIPT_CMD_SUCCESS;
	}

	if (inventory[idx].amount <= 0) {
		script_pushint(st, -1);
		return SCRIPT_CMD_SUCCESS;
	}

	int type = script_getnum(st, 3);
	switch (type)
	{
	case 0:  script_pushint(st, inventory[idx].nameid); break;
	case 1:  script_pushint(st, inventory[idx].amount); break;
	case 2:  script_pushint(st, inventory[idx].equip); break;
	case 3:  script_pushint(st, inventory[idx].refine); break;
	case 4:  script_pushint(st, inventory[idx].identify); break;
	case 5:  script_pushint(st, inventory[idx].attribute); break;
	case 6:  script_pushint(st, inventory[idx].card[0]); break;
	case 7:  script_pushint(st, inventory[idx].card[1]); break;
	case 8:  script_pushint(st, inventory[idx].card[2]); break;
	case 9:  script_pushint(st, inventory[idx].card[3]); break;
	case 10: script_pushint(st, inventory[idx].expire_time); break;
	case 11: script_pushint(st, inventory[idx].unique_id); break;
	case 12: case 13: case 14: case 15: case 16:
		script_pushint(st, inventory[idx].option[type - 12].id); break;
	case 17: case 18: case 19: case 20: case 21:
		script_pushint(st, inventory[idx].option[type - 17].value); break;
	case 22: case 23: case 24: case 25: case 26:
		script_pushint(st, inventory[idx].option[type - 22].param); break;
	case 27: script_pushint(st, inventory[idx].bound); break;
	case 28: script_pushint(st, inventory[idx].enchantgrade); break;
	case 29: script_pushint(st, inventory[idx].equipSwitch); break;
	case 30: script_pushint(st, inventory[idx].favorite); break;
	default:
		ShowWarning("buildin_getinventoryinfo: The type should be in range 0-%d, currently type is: %d.\n", 30, type);
		script_pushint(st, -1);
		return SCRIPT_CMD_FAILURE;
	}

	return SCRIPT_CMD_SUCCESS;
}

// <道具的背包序号>,{,<角色编号>};
BUILDIN_FUNC(getitemcombatpower) {
    map_session_data *sd = nullptr;
    struct item_data *id = nullptr;
    int idx = script_getnum(st, 2);

    if (!script_charid2sd(3, sd)) {
        script_pushint(st, -1);
        return SCRIPT_CMD_SUCCESS;
    }

    struct s_storage* stor = &sd->inventory;
    struct item* inventory = stor->u.items_inventory;

    if (st->state == RERUNLINE) {
        return SCRIPT_CMD_SUCCESS;
    }

    if (!stor || !inventory) {
        ShowError("buildin_getitemcombatpower: cannot read inventory or storage data.\n");
        script_pushint(st, -1);
        return SCRIPT_CMD_FAILURE;
    }

    if (idx < 0 || idx >= stor->max_amount) {
        script_pushint(st, -1);
        return SCRIPT_CMD_SUCCESS;
    }

    if (!item_db.exists(inventory[idx].nameid)) {
        script_pushint(st, -1);
        return SCRIPT_CMD_SUCCESS;
    }

    if (inventory[idx].amount <= 0) {
        script_pushint(st, -1);
        return SCRIPT_CMD_SUCCESS;
    }
    script_pushint(st, getitem_combat_power(&inventory[idx]));
    return SCRIPT_CMD_SUCCESS;
}

/* ===========================================================
 * 指令: processhalt
 * 描述: 在事件处理代码中使用该指令, 可以中断源代码的后续处理逻辑
 * 用法: processhalt {<是否设置中断>};
 * 返回: 该指令无论成功失败, 都不会有返回值
 * -----------------------------------------------------------*/
BUILDIN_FUNC(processhalt) {
    map_session_data *sd = nullptr;
    char* postfix = NULL;

    if (!script_rid2sd(sd))
        return SCRIPT_CMD_SUCCESS;

    if (sd->extend.workinevent == NPCE_MAX) {
        ShowError("buildin_processhalt: Require work in event script.\n");
        return SCRIPT_CMD_FAILURE;
    }

    const char* name = npc_get_script_event_name(sd->extend.workinevent);
    if (name == nullptr) {
        ShowError("buildin_processhalt: Can not get the event name for event type : %d\n", sd->extend.workinevent);
        return SCRIPT_CMD_FAILURE;
    }

    std::string evname = std::string(name);
    if (evname.find("Filter") == std::string::npos) {
        ShowError("buildin_processhalt: The '%s' event is not support processhalt.\n", evname.c_str());
        return SCRIPT_CMD_FAILURE;
    }

    bool makehalt = true;
    if (script_hasdata(st, 2) && script_isint(st, 2)) {
        makehalt = (cap_value(script_getnum(st, 2), 0, 1) == 1);
    }

    if (!setProcessHalt(sd, sd->extend.workinevent, makehalt)) {
        ShowError("buildin_processhalt: An error occurred while setting the '%s' event halt status to '%d'.\n", evname.c_str(), (int32)makehalt);
        return SCRIPT_CMD_FAILURE;
    }
    return SCRIPT_CMD_SUCCESS;
}