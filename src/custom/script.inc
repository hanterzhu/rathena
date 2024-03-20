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

/* ===========================================================
 * 指令: checksuspend
 * 描述: 获取指定角色或指定账号当前在线角色的挂机模式
 * 用法: checksuspend {<角色编号|账号编号|"角色名称">};
 * 返回: 角色不存在返回 -1, 否则返回当前的挂机状态
 * -----------------------------------------------------------*/
BUILDIN_FUNC(checkoffline) {
	TBL_PC* sd = nullptr;

	if (script_hasdata(st, 2)) {
		if (script_isstring(st, 2))
			sd = map_nick2sd(script_getstr(st, 2), false);
		else {
			int id = script_getnum(st, 2);
			sd = map_id2sd(id);
			if (!sd)
				sd = map_charid2sd(id);
		}
	}
	else {
		if (!script_rid2sd(sd)) {
			script_pushint(st, -1);
			return SCRIPT_CMD_SUCCESS;
		}
	}

	if (!sd) {
		script_pushint(st, -1);
		return SCRIPT_CMD_SUCCESS;
	}

	if ((sd->state.autotrade & AUTOTRADE_OFFLINE) == AUTOTRADE_OFFLINE)
		script_pushint(st, SUSPEND_MODE_OFFLINE);
	else if ((sd->state.autotrade & AUTOTRADE_NORMAL) == AUTOTRADE_NORMAL)
		script_pushint(st, SUSPEND_MODE_NORMAL);
	else
		script_pushint(st, SUSPEND_MODE_NONE);

	return SCRIPT_CMD_SUCCESS;
}

/**
login <角色编号>{,<默认是否坐下>{,<默认身体朝向>{,<默认脑袋朝向>{,<登录模式>}}}};

将指定的角色以特定的登录模式拉上线

角色编号:
	指定一个想使之上线的角色编号

默认是否坐下:
	0 表示站立, 1 表示坐下
	可选值, 默认值为: 0

	此选项会被 suspend_{afk|normal|offline}_sitdown 战斗配置选项覆盖
	具体被哪个选项影响取决于 <登录模式> 的值

默认身体朝向:
    1 = 西北      0 = 正北      7 = 东北
              ↖     |     ↗
    2 = 正西  <-     +     ->   6 = 正东
              ↙     |     ↘
    3 = 西南      4 = 正南      5 = 东南
	可选值, 默认值为: 4 (正南)

	此选项会被 suspend_{afk|normal|offline}_bodydirection 战斗配置选项覆盖
	具体被哪个选项影响取决于 <登录模式> 的值

默认脑袋朝向:
	0 = 相对于身体朝向的正前方, 头部看向正前方
	1 = 相对于身体朝向的正前方, 头部看向右看
	2 = 相对于身体朝向的正前方, 头部看向左看
	可选值, 默认值为: 0 (正前方)

	此选项会被 suspend_{afk|normal|offline}_headdirection 战斗配置选项覆盖
	具体被哪个选项影响取决于 <登录模式> 的值

登录模式:
	SUSPEND_MODE_OFFLINE	- 挂机模式
	SUSPEND_MODE_AFK		- 离开模式 (AFK)
	SUSPEND_MODE_NORMAL		- 普通模式
	可选值, 默认值为: SUSPEND_MODE_NORMAL

提示信息:
	请不要在 OnInit / OnInterIfInit / OnInterIfInitOnce 事件中使用该指令,
	因为在触发这几个事件的时候服务器并未就绪.

	若必须要在这个时机使用本指令的话, 可以使用 addtimer 将延迟几秒再执行.

返回值:
	成功返回 1, 失败返回 0
**/
BUILDIN_FUNC(login) {
	script_pushint(st, 0);

	uint32 charid = 0;
	charid = script_getnum(st, 2);

	int sit = 0;
	if (!script_get_optnum(st, 3, "Sitdown or not", sit, true, 0)) {
		return SCRIPT_CMD_FAILURE;
	}
	sit = cap_value(sit, 0, 1);

	int body_dir = DIR_SOUTH;
	if (!script_get_optnum(st, 4, "Body Direction", body_dir, true, DIR_SOUTH)) {
		return SCRIPT_CMD_FAILURE;
	}
	body_dir = cap_value(body_dir, 0, 7);

	int head_dir = 0;
	if (!script_get_optnum(st, 5, "Head Direction", head_dir, true, 0)) {
		return SCRIPT_CMD_FAILURE;
	}
	head_dir = cap_value(head_dir, 0, 2);

	int mode = SUSPEND_MODE_NONE;
	if (!script_get_optnum(st, 6, "Login Mode", mode, true, SUSPEND_MODE_NORMAL)) {
		return SCRIPT_CMD_FAILURE;
	}
	if (!suspend_mode_valid(mode)) {
		mode = SUSPEND_MODE_NORMAL;
	}

	if (suspend_recall(charid, (e_suspend_mode)mode, body_dir, head_dir, sit)) {
		script_pushint(st, 1);
	}

	return SCRIPT_CMD_SUCCESS;
}

/* ===========================================================
 * 指令: batrec_reset
 * 描述: 清除指定单位的战斗记录
 * 用法: batrec_reset <记录宿主的单位编号>;
 * 返回: 该指令无论成功与否, 都不会有返回值
 * -----------------------------------------------------------*/
BUILDIN_FUNC(log_damage_reset) {
    struct block_list* bl = map_id2bl(script_getnum(st, 2));
    if (!bl) {
        bl = &(map_charid2sd(script_getnum(st, 2)))->bl;
    }

    log_damage_reset(bl);
    return SCRIPT_CMD_SUCCESS;
}

/* ===========================================================
 * 指令: batrec_query
 * 描述: 查询指定单位的战斗记录, 查看与交互目标单位产生的具体记录值
 * 用法: batrec_query <记录宿主的单位编号>,<交互目标的单位编号>,<记录类型>{,<聚合规则>};
 * 返回: 返回 0 表示查无记录
 * -----------------------------------------------------------*/
BUILDIN_FUNC(damage_log_query) {
    struct block_list* bl = map_id2bl(script_getnum(st, 2));
    if (!bl) {
        bl = &(map_charid2sd(script_getnum(st, 2)))->bl;
    }

    if (!bl) {
        script_pushint(st, 0);
        return SCRIPT_CMD_SUCCESS;
    }

    int log_type = script_getnum(st, 4);
    if (log_type != DMGLOG_RECEIVE && log_type != DMGLOG_INFLICT) {
        ShowError("%s: The damage log type is not invalid.\n", __func__);
        return SCRIPT_CMD_FAILURE;
    }

    int aggregation = 1; //combine
    if (!script_get_optnum(st, 5, "Aggregation strategy", aggregation, true, 1)) {
        return SCRIPT_CMD_FAILURE;
    }

    int64 damage = query_log_damage(bl, script_getnum(st, 3), (e_damage_log_type)log_type, (bool)aggregation);

    script_pushint(st, damage);
    return SCRIPT_CMD_SUCCESS;
}

/* ===========================================================
 * 指令: damage_log_rank
 * 描述: 查询指定单位的战斗记录并对记录的值进行排序, 返回排行榜单
 * 用法: batrec_rank <记录宿主的单位编号>,<返回交互目标的单位编号数组>,<返回记录值数组>,<记录类型>{,<聚合规则>{,<排序规则>}};
 * 返回: 失败返回 -1, 含 0 正整数表示数组中返回的榜单记录数
 * -----------------------------------------------------------*/
BUILDIN_FUNC(damage_log_rank) {
    map_session_data* sd = nullptr;
    sd = map_id2sd(st->rid);

    struct block_list* bl = map_id2bl(script_getnum(st, 2));

    if (!bl) {
        bl = &(map_charid2sd(script_getnum(st, 2)))->bl;
    }

    if (!bl) {
        script_pushint(st, -1);
        return SCRIPT_CMD_SUCCESS;
    }

    int gid_array_varid = 0;
    char* gid_array_varname = nullptr;
    struct script_data* gid_array_vardata = nullptr;
    if (!script_get_array(st, 3, gid_array_varid, gid_array_varname, gid_array_vardata)) {
        script_pushint(st, -1);
        return SCRIPT_CMD_FAILURE;
    }
    script_cleararray_st(st, 3);

    int dmg_array_varid = 0;
    char* dmg_array_varname = nullptr;
    struct script_data* dmg_array_vardata = nullptr;
    if (!script_get_array(st, 4, dmg_array_varid, dmg_array_varname, dmg_array_vardata)) {
        script_pushint(st, -1);
        return SCRIPT_CMD_FAILURE;
    }
    script_cleararray_st(st, 4);

    int log_type = script_getnum(st, 5);
    if (log_type != DMGLOG_INFLICT && log_type != DMGLOG_RECEIVE) {
        ShowError("%s: The battle record type is not invalid.\n", __func__);
        script_pushint(st, -1);
        return SCRIPT_CMD_FAILURE;
    }

    int aggregation = 1; //combine
    if (!script_get_optnum(st, 6, "Aggregation strategy", aggregation, true, 1)) {
        script_pushint(st, -1);
        return SCRIPT_CMD_FAILURE;
    }

    int sort_type = 1; //desc
    if (!script_get_optnum(st, 7, "Sort Type", sort_type, true, 1)) {
        script_pushint(st, -1);
        return SCRIPT_CMD_FAILURE;
    }

    damage_log_map* origin_map = nullptr;
    if (!(origin_map = log_damage_getmap(bl, (e_damage_log_type)log_type))) {
        ShowError("%s: The damage log is not invalid.\n", __func__);
        script_pushint(st, -1);
        return SCRIPT_CMD_FAILURE;
    }

    damage_log_map* result_map = log_damage_aggregation(origin_map, (bool)aggregation);

    std::vector<std::pair<int, s_damage_log_ptr>> rec_sorted;
    for (auto& it : *result_map) {
        rec_sorted.push_back(it);
    }

    if (sort_type == 1) {
        std::sort(rec_sorted.begin(), rec_sorted.end(), log_damage_desc);
    } else {
        std::sort(rec_sorted.begin(), rec_sorted.end(), log_damage_asc);
    }

    for (int i = 0; i < rec_sorted.size(); i++) {
        int64 uid = reference_uid(gid_array_varid, i);
        set_reg_num(st, sd, uid, gid_array_varname, rec_sorted[i].first, reference_getref(gid_array_vardata));

        uid = reference_uid(dmg_array_varid, i);
        set_reg_num(st, sd, uid, dmg_array_varname, rec_sorted[i].second->damage, reference_getref(dmg_array_vardata));
    }

    script_pushint(st, rec_sorted.size());
    return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(getpetoptinfo) {
    map_session_data *sd;
    int val;
    if (script_rid2sd(sd) && current_pet_opt_index != -1 && sd->pd && sd->pd->pet.extend.option[current_pet_opt_index].id) {
        int param = script_getnum(st, 2);

        switch (param) {
            case ROA_ID:
                val = sd->pd->pet.extend.option[current_pet_opt_index].id;
                break;
            case ROA_VALUE:
                val = sd->pd->pet.extend.option[current_pet_opt_index].value;
                break;
            case ROA_PARAM:
                val = sd->pd->pet.extend.option[current_pet_opt_index].param;
                break;
            default:
                ShowWarning("buildin_getpetoptinfo: Invalid attribute type %d (Max %d).\n", param, MAX_ITEM_RDM_OPT);
                val = 0;
                break;
        }
        script_pushint(st, val);
    }
    else {
        script_pushint(st, 0);
    }
    return SCRIPT_CMD_SUCCESS;
}