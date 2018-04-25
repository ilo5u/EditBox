#include "stdafx.h"
#include "editbox.h"
#include <process.h>
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
	HTIMER new_timer = new TIMER{time_length};
	return new_timer;
}

BOOL _stdcall KillTimer(HTIMER timer)
{
	delete timer;
	return (TRUE);
}

BOOL WINAPI MyTextOutW(_In_ HDC hdc, _In_ int x, _In_ int y, _In_reads_(c) LPCWSTR lpString, _In_ int c, _In_ short s, _In_ short e, _In_ int width)
{
	int iCount = 0;
	int iLeft = x;
	int iRight = x;

	//°×µ×ºÚ×Ö
	SetTextColor(hdc, RGB(0x00, 0x00, 0x00));
	SetBkColor(hdc, RGB(0xFF, 0xFF, 0xFF));
	// ...

	while (iCount < s)
	{
		if (lpString[iCount] & TCHAR(0xFF00))
		{	// ºº×Ö
			iRight = iRight + (width << 1);
		}
		else
		{
			iRight = iRight + width;
		}
		++iCount;
	}
	TextOut(hdc, iLeft, y, lpString, iCount);

	// À¶µ×°××Ö
	SetTextColor(hdc, RGB(0xFF, 0xFF, 0xFF));
	SetBkColor(hdc, RGB(0x1C, 0x86, 0xEE));
	// ...

	iLeft = iRight;
	while (iCount < e)
	{
		if (lpString[iCount] & TCHAR(0xFF00))
		{	// ºº×Ö
			iRight = iRight + (width << 1);
		}
		else
		{
			iRight = iRight + width;
		}
		++iCount;
	}
	TextOut(hdc, iLeft, y, lpString + s, iCount - s);

	//°×µ×ºÚ×Ö
	SetTextColor(hdc, RGB(0x00, 0x00, 0x00));
	SetBkColor(hdc, RGB(0xFF, 0xFF, 0xFF));
	// ...

	iLeft = iRight;
	while (iCount < c)
	{
		if (lpString[iCount] & TCHAR(0xFF00))
		{	// ºº×Ö
			iRight = iRight + (width << 1);
		}
		else
		{
			iRight = iRight + width;
		}
		++iCount;
	}
	TextOut(hdc, iLeft, y, lpString + e, iCount - e);

	// assert(!(iCount != c));

	return (TRUE);
}

void chDEBUGPRINTBOX(LPCWSTR msg)
{
	MessageBox(NULL, msg, TEXT("DEBUG"), MB_OKCANCEL);
}

void chDEBUGTEXTOUT(HWND hWnd, LPCWSTR lpString, int x, int y)
{
	HDC hdc = GetDC(hWnd);
	TextOut(hdc, x, y, lpString, lstrlen(lpString));
	ReleaseDC(hWnd, hdc);
}

void chDEBUGMESSAGEBOX(LPCWSTR lpszFmt, ...)
{
	TCHAR out[1000] = { 0 };
	va_list arg_ptr;
	va_start(arg_ptr, lpszFmt);
	_vsnwprintf_s(out, _countof(out), lpszFmt, arg_ptr);
	MessageBox(NULL, out, TEXT("DEBUG"), MB_OKCANCEL);
}

BOOL WINAPI MyDrawTextW(_In_ HDC hdc, _In_ int x, _In_ int y, _In_reads_(c) LPCWSTR lpchText, _In_ LPRECT lpRect, _In_ int c, _In_ short s, _In_ short e, _In_ int width)
{
	
	return (TRUE);
}