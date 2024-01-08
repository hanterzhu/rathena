//
// Created by dylan on 2024/1/7.
//

#pragma once

#include <common/cbasetypes.hpp>
#include <common/db.hpp>
#include <common/timer.hpp>

class map_session_data;

enum e_suspend_mode : uint8 {
	SUSPEND_MODE_NONE     = 0x0000,
	SUSPEND_MODE_OFFLINE  = 0x0001,		// ���߹һ�
	SUSPEND_MODE_NORMAL   = 0x0002		// ��ͨģʽ
};

struct s_suspender {
	uint32 account_id;					// �˺ű�� (���ݿ��е�����)
	uint32 char_id;						// ��ɫ��� (suspender_db ������)
	int m;								// ��ͼ���
	uint16 x, y;						// ��ͼ�� X �� Y ����
	unsigned char sex,					// �Ա� (M ��ʾ����, F ��ʾŮ��)
		dir,							// ֽ�������峯��
		head_dir,						// ֽ����ͷ������
		sit;							// �Ƿ�����
	enum e_suspend_mode mode;			// ģʽ
	t_tick tick;						// ���߹һ� �� �뿪ģʽ ����ʼʱ��
	long val1, val2, val3, val4;		// ���Ӳ���
	map_session_data* sd;
};

#define suspend_mode_valid(x) (x == SUSPEND_MODE_OFFLINE || x == SUSPEND_MODE_NORMAL)

bool suspend_recall(uint32 charid, e_suspend_mode mode = SUSPEND_MODE_NORMAL, unsigned char body_dir = 4, unsigned char head_dir = 0, unsigned char sit = 0);
void suspend_recall_all();

void suspend_recall_postfix(map_session_data* sd);
void suspend_active(map_session_data* sd, enum e_suspend_mode smode);
void suspend_deactive(map_session_data* sd, bool keep_database);

void do_final_suspend(void);
void do_init_suspend(void);

