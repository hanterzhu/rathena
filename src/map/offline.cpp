//
// Created by dylan on 2024/1/7.
//
#include "offline.hpp"

#include <common/malloc.hpp> // aMalloc, aFree
#include <common/nullpo.hpp>
#include <common/showmsg.hpp>
#include <common/sql.hpp>

#include "map.hpp"
#include "chrif.hpp"
#include "pc.hpp"
#include "channel.hpp"
#include "battle.hpp"
#include "chat.hpp"
#include "trade.hpp"
#include "storage.hpp"
#include "party.hpp"
#include "duel.hpp"
#include "guild.hpp"

static DBMap* suspender_db;

//************************************
// Method:      suspend_set_status
// Description: 建立 s_suspender 后对 sd 单位进行初始化设置
// Access:      public
// Parameter:   struct s_suspender * sp
// Returns:     void
//************************************
void suspend_set_status(struct s_suspender* sp) {
	if (!sp) return;

	// 设置 autotrade 标记
	sp->sd->state.autotrade |= AUTOTRADE_ENABLED;

	switch (sp->mode) {
        case SUSPEND_MODE_OFFLINE:
            sp->sd->state.autotrade |= AUTOTRADE_OFFLINE;
            break;
        case SUSPEND_MODE_NORMAL:
            sp->sd->state.autotrade |= AUTOTRADE_NORMAL;
            break;
	}

	sp->sd->state.block_action &= ~PCBLOCK_IMMUNE;

	sp->sd->extend.at_dir = sp->dir;
	sp->sd->extend.at_head_dir = sp->head_dir;
	sp->sd->extend.at_sit = sp->sit;
}

//************************************
// Method:      suspend_recall
// Description: 单独召唤指定角色编号的玩家上线
// Access:      public
// Parameter:   uint32 charid
// Parameter:   e_suspend_mode mode
// Parameter:   unsigned char body_dir
// Parameter:   unsigned char head_dir
// Parameter:   unsigned char sit
// Returns:     bool
//************************************
bool suspend_recall(uint32 charid, e_suspend_mode mode, unsigned char body_dir, unsigned char head_dir, unsigned char sit) {

	bool ret = false;

	do
	{
		if (map_charid2sd(charid) != nullptr)
			break;

		if (Sql_Query(mmysql_handle,
			"SELECT `account_id`, `char_id`, `sex` "
			"FROM `%s` "
			"WHERE `char_id` = %d;",
			"char", charid) != SQL_SUCCESS)
		{
			Sql_ShowDebug(mmysql_handle);
			break;
		}

		if (Sql_NumRows(mmysql_handle) != 1) {
			break;
		}

		if (SQL_SUCCESS != Sql_NextRow(mmysql_handle)) {
			break;
		}

		char* data = NULL;
		struct s_suspender* sp = NULL;
		CREATE(sp, struct s_suspender, 1);
		memset(sp, 0, sizeof(struct s_suspender));

		Sql_GetData(mmysql_handle, 0, &data, NULL); sp->account_id = atoi(data);
		Sql_GetData(mmysql_handle, 1, &data, NULL); sp->char_id = atoi(data);
		Sql_GetData(mmysql_handle, 2, &data, NULL); sp->sex = (data[0] == 'F') ? SEX_FEMALE : SEX_MALE;

		TBL_PC* sd = nullptr;
		if ((sd = map_id2sd(sp->account_id)) != NULL) {
			aFree(sp);
			break;
		}

		if (chrif_search(sp->account_id) != NULL) {
			aFree(sp);
			break;
		}

		// 初始化一个玩家对象
		CREATE(sp->sd, map_session_data, 1);
		pc_setnewpc(sp->sd, sp->account_id, sp->char_id, 0, gettick(), sp->sex, 0);

		sp->mode = mode;
		sp->dir = body_dir;
		sp->head_dir = head_dir;
		sp->sit = sit;

		suspend_set_status(sp);

		// 向服务器请求上线该玩家
		chrif_authreq(sp->sd, true);
		uidb_put(suspender_db, sp->char_id, sp);
		ret = true;
	} while (false);

	Sql_FreeResult(mmysql_handle);
	return ret;
}

//************************************
// Method:      suspend_recall_all
// Description: 将全部挂起的角色按照各自的模式召回上线
// Returns:     void
//************************************
void suspend_recall_all() {
	int offline = 0, normal = 0;

	do
	{
		if (Sql_Query(mmysql_handle,
			"SELECT `account_id`, `char_id`, `sex`, `body_direction`, `head_direction`, `sit`, `mode`, `tick`, `val1`, `val2`, `val3`, `val4` "
			"FROM `%s` "
			"ORDER BY `account_id`;",
			suspend_table) != SQL_SUCCESS)
		{
			Sql_ShowDebug(mmysql_handle);
			break;
		}

		if (!Sql_NumRows(mmysql_handle))
			break;

		while (SQL_SUCCESS == Sql_NextRow(mmysql_handle)) {
			char* data = NULL;

			struct s_suspender* sp = NULL;
			CREATE(sp, struct s_suspender, 1);
			memset(sp, 0, sizeof(struct s_suspender));

			Sql_GetData(mmysql_handle, 0, &data, NULL); sp->account_id = atoi(data);
			Sql_GetData(mmysql_handle, 1, &data, NULL); sp->char_id = atoi(data);
			Sql_GetData(mmysql_handle, 2, &data, NULL); sp->sex = (data[0] == 'F') ? SEX_FEMALE : SEX_MALE;
			Sql_GetData(mmysql_handle, 3, &data, NULL); sp->dir = atoi(data);
			Sql_GetData(mmysql_handle, 4, &data, NULL); sp->head_dir = atoi(data);
			Sql_GetData(mmysql_handle, 5, &data, NULL); sp->sit = atoi(data);

			Sql_GetData(mmysql_handle, 6, &data, NULL); sp->mode = e_suspend_mode(atoi(data));
			Sql_GetData(mmysql_handle, 7, &data, NULL); sp->tick = strtoll(data, nullptr, 10);

			Sql_GetData(mmysql_handle, 8, &data, NULL); sp->val1 = atoi(data);
			Sql_GetData(mmysql_handle, 9, &data, NULL); sp->val2 = atoi(data);
			Sql_GetData(mmysql_handle, 10, &data, NULL); sp->val3 = atoi(data);
			Sql_GetData(mmysql_handle, 11, &data, NULL); sp->val4 = atoi(data);

			TBL_PC* sd = nullptr;
			if ((sd = map_id2sd(sp->account_id)) != NULL) {
				aFree(sp);
				continue;
			}

			struct auth_node* node = nullptr;
			if (chrif_search(sp->account_id) != NULL) {
				aFree(sp);
				continue;
			}

			// 初始化一个玩家对象
			CREATE(sp->sd, map_session_data, 1);
			pc_setnewpc(sp->sd, sp->account_id, sp->char_id, 0, gettick(), sp->sex, 0);
			suspend_set_status(sp);

			switch (sp->mode) {
                case SUSPEND_MODE_OFFLINE: offline++; break;
                case SUSPEND_MODE_NORMAL: normal++; break;
			}

			// 向服务器请求上线该玩家
			chrif_authreq(sp->sd, true);
			uidb_put(suspender_db, sp->char_id, sp);
		}
	} while (false);

	Sql_FreeResult(mmysql_handle);
	ShowStatus("Done loading '" CL_WHITE "%d" CL_RESET "' suspend player records (Offline: '%d', Normal: '%d').\n", db_size(suspender_db), offline, normal);
}

//************************************
// Method:      suspend_recall_postfix
// Description: 被召回的角色成功上线后需要做的后置处理
// Parameter:   map_session_data * sd
// Returns:     void
//************************************
void suspend_recall_postfix(map_session_data* sd) {
	nullpo_retv(sd);

	struct s_suspender* sp = NULL;
	if (sp = (struct s_suspender*)uidb_get(suspender_db, sd->status.char_id)) {
		pc_setdir(sd, sp->dir, sp->head_dir);
		clif_changed_dir(&sd->bl, AREA_WOS);
		if (sp->sit) {
			pc_setsit(sd);
			skill_sit(sd, 1);
			clif_sitting(&sd->bl);
		}
	}

}

//************************************
// Method:      suspend_active
// Description: 激活某个角色的某种挂起模式, 并使其断开与客户端的连接
// Parameter:   map_session_data * sd
// Parameter:   enum e_suspend_mode smode
// Returns:     void
//************************************
void suspend_active(map_session_data* sd, enum e_suspend_mode smode) {
	nullpo_retv(sd);
	long val[4] = { 0 };

	// 先删除数据库中原先的数据记录, 以便接下来重新登记
	suspend_deactive(sd, false);

	// 设置 autotrade 标记
	sd->state.autotrade |= AUTOTRADE_ENABLED;

	switch (smode) {
        case SUSPEND_MODE_OFFLINE:
            sd->state.autotrade |= AUTOTRADE_OFFLINE;
            break;
        case SUSPEND_MODE_NORMAL:
            sd->state.autotrade |= AUTOTRADE_NORMAL;
            break;
	}

	sd->state.block_action &= ~PCBLOCK_IMMUNE;

	struct s_suspender* sp = NULL;
	CREATE(sp, struct s_suspender, 1);
	memset(sp, 0, sizeof(struct s_suspender));

	sp->account_id = sd->status.account_id;
	sp->char_id = sd->status.char_id;
	sp->sex = (sd->status.sex == SEX_FEMALE ? 'F' : 'M');
	sp->dir = sd->ud.dir;
	sp->head_dir = sd->head_dir;
	sp->sit = pc_issit(sd);
	sp->mode = smode;
	sp->tick = (t_tick)time(NULL);
	uidb_put(suspender_db, sp->char_id, sp);

	// 这里需要立刻填充相关的备份信息, 避免在完成指令下线后,
	// 服务器没还重启的情况下, 角色就被 recall 导致朝向等数据无法恢复
	sd->extend.at_dir = sd->ud.dir;
	sd->extend.at_head_dir = sd->head_dir;
	sd->extend.at_sit = pc_issit(sd);

	if (Sql_Query(mmysql_handle, "INSERT INTO `%s`(`account_id`, `char_id`, `sex`, `map`, `x`, `y`, `body_direction`, `head_direction`, `sit`, `mode`, `tick`, `val1`, `val2`, `val3`, `val4`) "
		"VALUES( %d, %d, '%c', '%s', %d, %d, '%d', '%d', '%d', '%hu', '%" PRtf "', '%ld', '%ld', '%ld', '%ld' );",
		suspend_table, sd->status.account_id, sd->status.char_id, sd->status.sex == SEX_FEMALE ? 'F' : 'M', map_getmapdata(sd->bl.m)->name,
		sd->bl.x, sd->bl.y, sd->ud.dir, sd->head_dir, pc_issit(sd), int(smode), gettick(), val[0], val[1], val[2], val[3]) != SQL_SUCCESS) {
		Sql_ShowDebug(mmysql_handle);
	}

	// 若正在聊天室内, 则离开聊天室
	if (sd->chatID)
		chat_leavechat(sd, 0);

	// 若正在进行交易, 则立刻取消交易
	if (sd->trade_partner)
		trade_tradecancel(sd);

	// 关闭正在访问的仓库, 防止卡住公会仓库
	if (sd->state.storage_flag == 1)
		storage_storage_quit(sd, 0);
	else if (sd->state.storage_flag == 2)
		storage_guild_storage_quit(sd, 0);
	else if (sd->state.storage_flag == 3)
		storage_premiumStorage_quit(sd);

	// 重置仓库访问标记位
	sd->state.storage_flag = 0;

	// 若正在被邀请加入队伍, 则立刻回绝
	if (sd->party_invite > 0)
		party_reply_invite(sd, sd->party_invite, 0);

	// 若正在被邀请加入公会, 则立刻回绝
	if (sd->guild_invite > 0)
		guild_reply_invite(sd, sd->guild_invite, 0);

	// 若正在被邀请创建公会同盟, 则立刻回绝
	if (sd->guild_alliance > 0)
		guild_reply_reqalliance(sd, sd->guild_alliance_account, 0);

	// 若处于决斗状态, 则离开决斗
	if (sd->duel_group > 0)
		duel_leave(sd->duel_group, sd);

	// 离开所有聊天频道
	channel_pcquit(sd, 0xF);

	clif_authfail_fd(sd->fd, 15);

	chrif_save(sd, CSAVE_AUTOTRADE);
}

//************************************
// Method:      suspend_suspender_remove
// Description: 释放某个具体的 struct s_suspender 结构体
// Parameter:   struct s_suspender * sp
// Parameter:   bool remove
// Returns:     void
//************************************
static void suspend_suspender_remove(struct s_suspender* sp, bool remove) {
	nullpo_retv(sp);
	if (remove)
		uidb_remove(suspender_db, sp->char_id);
	aFree(sp);
}

//************************************
// Method:      suspend_suspender_free
// Description: 用于释放 suspender_db 数据的子函数
// Parameter:   DBKey key
// Parameter:   DBData * data
// Parameter:   va_list ap
// Returns:     int
//************************************
static int suspend_suspender_free(DBKey key, DBData* data, va_list ap) {
	struct s_suspender* sp = (struct s_suspender*)db_data2ptr(data);
	if (!sp) return -1;
	suspend_suspender_remove(sp, false);
	return 0;
}

//************************************
// Method:      suspend_deactive
// Description: 反激活某个角色的挂起模式, 下次地图服务器启动将不再自动上线
// Access:      public
// Parameter:   map_session_data * sd
// Parameter:   bool keep_database
// Returns:     void
//************************************
void suspend_deactive(map_session_data* sd, bool keep_database) {
	nullpo_retv(sd);

	// 若希望保留下次地图服务器开启能自动上线的话, 那么不移除数据库中的记录
	// 反之则需要移除对应的数据库记录
	if (!keep_database) {
		if (Sql_Query(mmysql_handle, "DELETE FROM `%s` WHERE `account_id` = %d;", suspend_table, sd->status.account_id) != SQL_SUCCESS) {
			Sql_ShowDebug(mmysql_handle);
		}
	}

	struct s_suspender* sp = NULL;
	if (sp = (struct s_suspender*)uidb_get(suspender_db, sd->status.char_id)) {
		suspend_suspender_remove(sp, true);
	}
}

//************************************
// Method:      do_final_suspend
// Description: 释放玩家挂起子系统
// Parameter:   void
// Returns:     void
//************************************
void do_final_suspend(void) {
	suspender_db->destroy(suspender_db, suspend_suspender_free);
}

//************************************
// Method:      do_init_suspend
// Description: 初始化玩家挂起子系统
// Parameter:   void
// Returns:     void
//************************************
void do_init_suspend(void) {
	suspender_db = uidb_alloc(DB_OPT_BASE);
}
