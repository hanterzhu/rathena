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

BUILDIN_FUNC(get_unique_id)
{
	struct map_session_data* sd;

	if (!script_rid2sd(sd))
	{
		script_pushint(st, 0);
		return SCRIPT_CMD_FAILURE;
	}

	script_pushint(st, session[sd->fd]->gepard_info.unique_id);

	return SCRIPT_CMD_SUCCESS;
}
