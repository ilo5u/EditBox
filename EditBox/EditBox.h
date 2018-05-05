#pragma once

#include <Windows.h>

#include "resource.h"

// 定时器
typedef struct _TIMER
{
public:
	_TIMER(const DWORD& tr);
	~_TIMER();

	DWORD     time_rest;
	HANDLE    time_mutex;
	HANDLE    time_thread_id;
} TIMER, *HTIMER;

// 创建一次性定时器
HTIMER _stdcall CreateTimer(DWORD time_length);

// 定时器是否停止
BOOL IsTimerStopped(HTIMER hTimer);

// 销毁一次性定时器
BOOL _stdcall KillTimer(HTIMER timer);
// 定时器

// RESOURCE
class CText;
typedef CText *HTEXT, *LPTEXT;

// 文本内核结构
typedef struct _TEXT_KERNEL
{
	HTEXT m_hText;			// 文本实例
	POINT m_pTextSize;		// 文本范围
	POINT m_pStartPos;		// 选段起点
	POINT m_pEndPos;		// 选段终点
} TEXTKERNEL, *LPTEXTKERNEL, *HTEXTKERNEL;
HTEXTKERNEL __stdcall CreateKernel(HWND hWnd, HTEXTUSER hUser);
BOOL __stdcall ReleaseKernel(HWND hWnd, HTEXTKERNEL hKernel);

// 文本用户结构
typedef struct _TEXT_USER
{
	HFONT m_hFont;			// 字体实例
	POINT m_pCharSize;		// 字符大小
	POINT m_pCaretSize;		// 光标大小
	POINT m_pCaretPos;		// 光标位置
	POINT m_pWindowPos;		// 窗口位置
	POINT m_pWindowSize;	// 窗口大小
	POINT m_pPageSize;		// 页面范围
} TEXTUSER, *LPTEXTUSER, *HTEXTUSER;
HTEXTUSER __stdcall CreateUser(HWND hWnd);
BOOL __stdcall ReleaseUser(HWND hWnd, HTEXTUSER hUser);

// 文本图形设备结构
typedef struct _TEXT_GDI
{
	HDC m_hMemDC;			// 缓冲设备
	HBRUSH m_hBrush;		// 默认背景刷
	HBITMAP m_hBitmap;		// 缓冲位图
	HTIMER m_hMouseTimer;	// 鼠标移动定时器
	HTIMER m_hSaveTimer;	// 自动保存定时器
} TEXTGDI, *LPTEXTGDI, *HTEXTGDI;
HTEXTGDI __stdcall CreateGDI(HWND hWnd, HTEXTUSER hUser);
BOOL __stdcall ReleaseGDI(HWND hWnd, HTEXTGDI hGDI);

// 文本结构
typedef struct _TEXT_INFO
{
	HTEXTKERNEL	m_hKernel;
	HTEXTGDI	m_hGDI;
	HTEXTUSER	m_hUser;
	HWND		m_hWnd;
} TEXTINFO, *LPTEXTINFO, *HTEXTINFO;
HTEXTINFO __stdcall CreateTextInfo(HWND hWnd);
BOOL __stdcall ReleaseTextInfo(HWND hWnd, HTEXTINFO hTextInfo);
typedef RECT *LPRECT;
BOOL __stdcall SelectWindowSize(HTEXTINFO hTextInfo, LPRECT lpRect);

// IO
BOOL WINAPI MyTextOutW(_In_ HDC hdc, _In_ int x, _In_ int y, _In_reads_(c) LPCWSTR lpString, _In_ int c, _In_ short s, _In_ short e, _In_ int width);

#ifdef UNICODE
#define MyTextOut MyTextOutW
#else
#define MyTextOut MyTextOutA
#endif // UNICODE