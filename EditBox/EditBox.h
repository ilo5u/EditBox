#pragma once

#include "resource.h"
#include <Windows.h>

typedef struct _TIMER
{
public:
	_TIMER(const DWORD& tr);
	~_TIMER();

	DWORD     time_rest;
	HANDLE    time_mutex;
	HANDLE    time_thread_id;
} TIMER, *HTIMER;

HTIMER _stdcall CreateTimer(DWORD time_length);

BOOL IsTimerStopped(HTIMER hTimer);

BOOL _stdcall KillTimer(HTIMER timer);

BOOL WINAPI MyTextOutW(_In_ HDC hdc, _In_ int x, _In_ int y, _In_reads_(c) LPCWSTR lpString, _In_ int c, _In_ short s, _In_ short e, _In_ int width);

#ifdef UNICODE
#define MyTextOut MyTextOutW
#else
#define MyTextOut MyTextOutA
#endif // UNICODE

BOOL WINAPI MyDrawTextW(_In_ HDC hdc, _In_ int x, _In_ int y, _In_reads_(c) LPCWSTR lpString, _In_ LPRECT lpRect, _In_ int c, _In_ short s, _In_ short e, _In_ int width);

#ifdef UNICODE
#define MyDrawText MyDrawTextW
#else
#define MyDrawText MyDrawTextA
#endif // UNICODE

void chDEBUGPRINTBOX(LPCWSTR msg);

void chDEBUGMESSAGEBOX(LPCWSTR lpszFmt, ...);

void chDEBUGTEXTOUT(HWND hWnd, LPCWSTR lpString, int x, int y);