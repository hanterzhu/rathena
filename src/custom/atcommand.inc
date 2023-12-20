// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

/**
 * Custom Atcommands
 * Place the body of custom atcommands in this file.
 * Format:
 *	ACMD_FUNC(command_name)
 *	{
 *		<code>;
 *	}
 **/

//ACMD_FUNC(newcommand)
//{
//	clif_displaymessage(fd, "It works!");
//	clif_specialeffect(&sd->bl, EF_HEARTCASTING, AREA);
//	return 0;
//}

ACMD_FUNC(gepard_block_nick)
{
	struct map_session_data* violator_sd;
	time_t time_server;
	unsigned int duration;
	unsigned int violator_account_id = 0;
	unsigned int violator_unique_id = 0;
	char reason_str[GEPARD_REASON_LENGTH];
	char unban_time_str[GEPARD_TIME_STR_LENGTH];
	char duration_type, violator_name[NAME_LENGTH];
	const char* command_info = "Wrong input (usage: @gepard_block_nick <duration> <duration_type m/h/d> \"<char name>\" <reason>)";

	nullpo_retr(-1, sd);

	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));

	if (!*message || sscanf(message, "%u %c \"%23[^\"]\" %99[^\n]", &duration, &duration_type, violator_name, reason_str) < 4)
	{
		clif_displaymessage(fd, command_info);
		return -1;
	}

	time(&time_server);

	switch (duration_type)
	{
		case 'm':
			time_server += (duration * 60);
		break;

		case 'h':
			time_server += (duration * 3600);
		break;

		case 'd':
			time_server += (duration * 86400);
		break;

		default:
			duration = 0;
		break;
	}

	if (duration == 0)
	{
		clif_displaymessage(fd, command_info);
		return -1;
	}

	strftime(unban_time_str, sizeof(unban_time_str), "%Y-%m-%d %H:%M:%S", localtime(&time_server));

	sprintf(atcmd_output, "Request: block by name - %s", violator_name);
	clif_displaymessage(fd, atcmd_output);

	violator_sd = map_nick2sd(violator_name, false);

	if (violator_sd != NULL)
	{
		violator_account_id = violator_sd->status.account_id;
		violator_unique_id = session[violator_sd->fd]->gepard_info.unique_id;
	}

	chrif_gepard_req_block(violator_unique_id, violator_name, violator_account_id, sd->status.name, sd->status.account_id, unban_time_str, reason_str);

	return 0;
}

ACMD_FUNC(gepard_block_account_id)
{
	struct map_session_data* violator_sd;
	time_t time_server;
	unsigned int duration;
	unsigned int violator_account_id = 0;
	unsigned int violator_unique_id = 0;
	char reason_str[GEPARD_REASON_LENGTH];
	char duration_type, unban_time_str[GEPARD_TIME_STR_LENGTH];
	const char* command_info = "Wrong input (usage: @gepard_block_account_id <duration> <duration_type m/h/d> <account ID> <reason>)";

	nullpo_retr(-1, sd);

	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));

	if (!*message || sscanf(message, "%u %c %u %99[^\n]", &duration, &duration_type, &violator_account_id, reason_str) < 4)
	{
		clif_displaymessage(fd, command_info);
		return -1;
	}

	time(&time_server);

	switch (duration_type)
	{
		case 'm':
			time_server += (duration * 60);
		break;

		case 'h':
			time_server += (duration * 3600);
		break;

		case 'd':
			time_server += (duration * 86400);
		break;

		default:
			duration = 0;
		break;
	}

	if (duration == 0)
	{
		clif_displaymessage(fd, command_info);
		return -1;
	}

	strftime(unban_time_str, sizeof(unban_time_str), "%Y-%m-%d %H:%M:%S", localtime(&time_server));

	sprintf(atcmd_output, "Request: block by account ID: %u", violator_account_id);
	clif_displaymessage(fd, atcmd_output);

	violator_sd = map_id2sd(violator_account_id);

	if (violator_sd != NULL)
	{
		violator_account_id = violator_sd->status.account_id;
		violator_unique_id = session[violator_sd->fd]->gepard_info.unique_id;
	}

	chrif_gepard_req_block(violator_unique_id, atcmd_player_name, violator_account_id, sd->status.name, sd->status.account_id, unban_time_str, reason_str);

	return 0;
}

ACMD_FUNC(gepard_block_unique_id)
{
	time_t time_server;
	unsigned int duration;
	unsigned int violator_unique_id = 0;
	char reason_str[GEPARD_REASON_LENGTH];
	char duration_type, unban_time_str[GEPARD_TIME_STR_LENGTH];
	const char* command_info = "Wrong input (usage: @gepard_block_unique_id <duration> <duration_type m/h/d> <unique ID> <reason>)";

	nullpo_retr(-1, sd);

	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));

	if (!*message || sscanf(message, "%u %c %u %99[^\n]", &duration, &duration_type, &violator_unique_id, reason_str) < 4)
	{
		clif_displaymessage(fd, command_info);
		return -1;
	}

	time(&time_server);

	switch (duration_type)
	{
		case 'm':
			time_server += (duration * 60);
		break;

		case 'h':
			time_server += (duration * 3600);
		break;

		case 'd':
			time_server += (duration * 86400);
		break;

		default:
			duration = 0;
		break;
	}

	if (duration == 0)
	{
		clif_displaymessage(fd, command_info);
		return -1;
	}

	strftime(unban_time_str, sizeof(unban_time_str), "%Y-%m-%d %H:%M:%S", localtime(&time_server));

	sprintf(atcmd_output, "Request: block by unqiue ID: %u", violator_unique_id);
	clif_displaymessage(fd, atcmd_output);

	chrif_gepard_req_block(violator_unique_id, NULL, 0, sd->status.name, sd->status.account_id, unban_time_str, reason_str);

	return 0;
}

ACMD_FUNC(gepard_unblock_nick)
{
	char violator_name[NAME_LENGTH];
	const char* command_info = "Wrong input (usage: @gepard_unblock_nick <char name>)";

	nullpo_retr(-1, sd);

	if (!*message || sscanf(message, "\"%23[^\"]\"[^\n]", violator_name) < 1)
	{
		clif_displaymessage(fd, command_info);
		return -1;
	}

	sprintf(atcmd_output, "Request: unblock by name - %s", violator_name);

	clif_displaymessage(fd, atcmd_output);

	chrif_gepard_req_unblock(0, violator_name, 0, sd->status.account_id);

	return 0;
}

ACMD_FUNC(gepard_unblock_account_id)
{
	int violator_aid;
	const char* command_info = "Wrong input (usage: @gepard_unblock_account_id <account ID>)";

	nullpo_retr(-1, sd);

	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));

	if (!*message || sscanf(message, "%d", &violator_aid) < 1)
	{
		clif_displaymessage(fd, command_info);
		return -1;
	}

	sprintf(atcmd_output, "Request: unblock by account id - %d", violator_aid);

	clif_displaymessage(fd, atcmd_output);

	chrif_gepard_req_unblock(0, NULL, violator_aid, sd->status.account_id);

	return 0;
}

ACMD_FUNC(gepard_unblock_unique_id)
{
	unsigned int violator_unique_id;
	const char* command_info = "Wrong input (usage: @gepard_unblock_unique_id <unique ID>)";

	nullpo_retr(-1, sd);

	if (!*message || sscanf(message, "%u", &violator_unique_id) < 1)
	{
		clif_displaymessage(fd, command_info);
		return -1;
	}

	sprintf(atcmd_output, "Request: unblock by unique id - %u", violator_unique_id);

	clif_displaymessage(fd, atcmd_output);

	chrif_gepard_req_unblock(violator_unique_id, NULL, 0, sd->status.account_id);

	return 0;
}

