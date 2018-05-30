/*--------------------------------------------------------------------------------------------------
// File : timer.cpp
//
// Description:
//             提供单例定时器用于做鼠标事件的控制
//
// Author: 程鑫
//
// Date: 2018-05-06
//
--------------------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "miniword.h"
#include <mutex>

/*---------------------------------------------
	@Description:
		定时器的运行子线程函数

	@Paramter:
		lpVoid: 定时器句柄

	@Return:
		运行结果（默认返回0）

	@Author:
		程鑫
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
		查询定时器是否已经停止

	@Paramter:
		hTimer: 定时器句柄

	@Return:
		TRUE: 停止
		FALSE: 未停止

	@Author:
		程鑫
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
		定时器构造函数
		采用单例模型，当定时器被创建时就开始倒计时

	@Paramter:
		tr: 定时器时长

	@Author:
		程鑫
---------------------------------------------*/
_TIMER::_TIMER(const DWORD& tr) :
	time_rest(tr)
{
	time_mutex = CreateMutex(NULL, FALSE, TEXT("TIME MUTEX"));
	time_thread_id = CreateThread(NULL, 0, RunTimer, (LPVOID)this, 0, NULL);
}

/*---------------------------------------------
	@Description:
		定时器析构函数
		释放资源

	@Author:
		程鑫
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
		创建定时器

	@Paramter:
		time_length: 定时器时长

	@Return:
		一个定时器对象的句柄

	@Author:
		程鑫
---------------------------------------------*/
HTIMER _stdcall CreateTimer(DWORD time_length)
{
	HTIMER new_timer = new TIMER{ time_length };
	return new_timer;
}

/*---------------------------------------------
	@Description:
		销毁定时器

	@Paramter:
		yimer: 某个定时器的句柄

	@Return:
		处理结果（默认返回TRUE）

	@Author:
		程鑫
---------------------------------------------*/
BOOL _stdcall KillTimer(HTIMER timer)
{
	delete timer;
	return (TRUE);
}