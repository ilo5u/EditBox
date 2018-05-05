#include "stdafx.h"
#include "editbox.h"
#include <mutex>

static DWORD __stdcall RunTimer(LPVOID lpVoid)
{
	HTIMER hTimer = (HTIMER)lpVoid;

	WaitForSingleObject(hTimer->time_mutex, INFINITE);
	Sleep(hTimer->time_rest);
	ReleaseMutex(hTimer->time_mutex);
	return (0);
}

BOOL IsTimerStopped(HTIMER hTimer)
{
	if (WaitForSingleObject(hTimer->time_mutex, 50) == WAIT_TIMEOUT)
		return (FALSE);

	ReleaseMutex(hTimer->time_mutex);
	return (TRUE);
}

_TIMER::_TIMER(const DWORD& tr) :
	time_rest(tr)
{
	time_mutex = CreateMutex(NULL, FALSE, TEXT("TIME MUTEX"));
	time_thread_id = CreateThread(NULL, 0, RunTimer, (LPVOID)this, 0, NULL);
}

_TIMER::~_TIMER()
{
	WaitForSingleObject(time_thread_id, INFINITE);
	CloseHandle(time_thread_id);

	WaitForSingleObject(time_mutex, INFINITE);
	CloseHandle(time_mutex);
}

HTIMER _stdcall CreateTimer(DWORD time_length)
{
	HTIMER new_timer = new TIMER{ time_length };
	return new_timer;
}

BOOL _stdcall KillTimer(HTIMER timer)
{
	delete timer;
	return (TRUE);
}