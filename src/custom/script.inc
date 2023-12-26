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