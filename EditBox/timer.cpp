/*--------------------------------------------------------------------------------------------------
// File : timer.cpp
//
// Description:
//             �ṩ������ʱ������������¼��Ŀ���
//
// Author: ����
//
// Date: 2018-05-06
//
--------------------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "miniword.h"
#include <mutex>

/*---------------------------------------------
	@Description:
		��ʱ�����������̺߳���

	@Paramter:
		lpVoid: ��ʱ�����

	@Return:
		���н����Ĭ�Ϸ���0��

	@Author:
		����
---------------------------------------------*/
static DWORD __stdcall RunTimer(LPVOID lpVoid)
{
	HTIMER hTimer = (HTIMER)lpVoid;

	WaitForSingleObject(hTimer->time_mutex, INFINITE);
	Sleep(hTimer->time_rest);
	ReleaseMutex(hTimer->time_mutex);
	return (0);
}

/*---------------------------------------------
	@Description:
		��ѯ��ʱ���Ƿ��Ѿ�ֹͣ

	@Paramter:
		hTimer: ��ʱ�����

	@Return:
		TRUE: ֹͣ
		FALSE: δֹͣ

	@Author:
		����
---------------------------------------------*/
BOOL IsTimerStopped(HTIMER hTimer)
{
	if (WaitForSingleObject(hTimer->time_mutex, 50) == WAIT_TIMEOUT)
		return (FALSE);

	ReleaseMutex(hTimer->time_mutex);
	return (TRUE);
}

/*---------------------------------------------
	@Description:
		��ʱ�����캯��
		���õ���ģ�ͣ�����ʱ��������ʱ�Ϳ�ʼ����ʱ

	@Paramter:
		tr: ��ʱ��ʱ��

	@Author:
		����
---------------------------------------------*/
_TIMER::_TIMER(const DWORD& tr) :
	time_rest(tr)
{
	time_mutex = CreateMutex(NULL, FALSE, TEXT("TIME MUTEX"));
	time_thread_id = CreateThread(NULL, 0, RunTimer, (LPVOID)this, 0, NULL);
}

/*---------------------------------------------
	@Description:
		��ʱ����������
		�ͷ���Դ

	@Author:
		����
---------------------------------------------*/
_TIMER::~_TIMER()
{
	WaitForSingleObject(time_thread_id, INFINITE);
	CloseHandle(time_thread_id);

	WaitForSingleObject(time_mutex, INFINITE);
	CloseHandle(time_mutex);
}

/*---------------------------------------------
	@Description:
		������ʱ��

	@Paramter:
		time_length: ��ʱ��ʱ��

	@Return:
		һ����ʱ������ľ��

	@Author:
		����
---------------------------------------------*/
HTIMER _stdcall CreateTimer(DWORD time_length)
{
	HTIMER new_timer = new TIMER{ time_length };
	return new_timer;
}

/*---------------------------------------------
	@Description:
		���ٶ�ʱ��

	@Paramter:
		yimer: ĳ����ʱ���ľ��

	@Return:
		��������Ĭ�Ϸ���TRUE��

	@Author:
		����
---------------------------------------------*/
BOOL _stdcall KillTimer(HTIMER timer)
{
	delete timer;
	return (TRUE);
}