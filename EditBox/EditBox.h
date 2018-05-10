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
HTIMER _stdcall CreateTimer(DWORD time_length); // 创建一次性定时器
BOOL   _stdcall KillTimer(HTIMER timer);        // 销毁一次性定时器
BOOL            IsTimerStopped(HTIMER hTimer);  // 定时器是否停止

// RESOURCE
class CText;
typedef CText *HTEXT, *LPTEXT;

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
BOOL      __stdcall ReleaseUser(HWND hWnd, HTEXTUSER hUser);

// 文本内核结构
typedef struct _TEXT_KERNEL
{
	HTEXT m_hText;			// 文本实例
	POINT m_pTextSize;		// 文本范围
	POINT m_pStartPos;		// 选段起点
	POINT m_pEndPos;		// 选段终点
} TEXTKERNEL, *LPTEXTKERNEL, *HTEXTKERNEL;
HTEXTKERNEL __stdcall CreateKernel(HWND hWnd, HTEXTUSER hUser);
BOOL        __stdcall ReleaseKernel(HWND hWnd, HTEXTKERNEL hKernel);

// 文本图形设备结构
typedef struct _TEXT_GDI
{
	HDC     m_hMemDC;		// 缓冲设备
	HBRUSH  m_hBrush;		// 默认背景刷
	HBITMAP m_hBitmap;		// 缓冲位图
	HTIMER  m_hMouseTimer;	// 鼠标移动定时器
	HTIMER  m_hSaveTimer;	// 自动保存定时器
} TEXTGDI, *LPTEXTGDI, *HTEXTGDI;
HTEXTGDI __stdcall CreateGDI(HWND hWnd, HTEXTUSER hUser);
BOOL     __stdcall ReleaseGDI(HWND hWnd, HTEXTGDI hGDI);

// 文本结构
typedef struct _TEXT_INFO
{
	HTEXTKERNEL	m_hKernel;
	HTEXTGDI	m_hGDI;
	HTEXTUSER	m_hUser;
	HWND		m_hWnd;
} TEXTINFO, *LPTEXTINFO, *HTEXTINFO;
HTEXTINFO __stdcall CreateTextInfo(HWND hWnd);
BOOL      __stdcall ReleaseTextInfo(HWND hWnd, HTEXTINFO hTextInfo);
typedef RECT *LPRECT;

// IO
BOOL WINAPI MyTextOutW(_In_ HDC hdc, _In_ int x, _In_ int y, _In_reads_(c) LPCWSTR lpString, _In_ int c, _In_ short s, _In_ short e, _In_ int width);

#ifdef UNICODE
#define MyTextOut MyTextOutW
#else
#define MyTextOut MyTextOutA
#endif // UNICODE

// Operation function
BOOL	MyInvalidateRect(HWND, LONG, LONG, LONG, LONG);         // 设置无效区域

BOOL    AdjustCaretPosBeforeBackspace(HWND, HTEXTINFO);         // 用户按Backspace键时 调整光标位置
BOOL    AdjustWindowPos(HWND, HTEXTINFO);                       // 滑动窗口
BOOL    MoveCaret(HWND, UINT, HTEXTINFO);                       // 移动光标

BOOL    SelectWindowPos(HTEXTINFO, POINT);                      // 设置窗口位置
BOOL    SelectWindowSize(HTEXTINFO hTextInfo, LPRECT lpRect);   // 设置窗口范围
BOOL    SelectCaretPos(HTEXTINFO, POINT);                       // 设置光标位置
BOOL    SelectHighlight(HTEXTINFO, POINT, POINT);               // 设置高亮部分
BOOL    SelectTextSize(HTEXTINFO, POINT);                       // 设置文本范围
BOOL    SelectUntitledWindow(HWND, HTEXTINFO);                  // 默认文本窗口显示
BOOL    SelectTitledWindow(HWND, HTEXTINFO, POINT, LPCWSTR);    // 命名文本窗口显示

BOOL    MyScrollWindow(HWND, int, int);                         // 滑动窗口
BOOL    DefaultFill(HWND, HTEXTINFO);                           // 描边

BOOL    PaintWindow(LPPAINTSTRUCT, HTEXTINFO);                  // 重绘窗口

// Convenient macro
// +++++++++++++++++++++++++++ Kernel ++++++++++++++++++++++++++ //
#define HTEXT(hTextInfo)    (hTextInfo->m_hKernel->m_hText)
#define TEXTSIZE(hTextInfo) (hTextInfo->m_hKernel->m_pTextSize)
#define STARTPOS(hTextInfo) (hTextInfo->m_hKernel->m_pStartPos)
#define ENDPOS(hTextInfo)   (hTextInfo->m_hKernel->m_pEndPos)
// +++++++++++++++++++++++++++   End  ++++++++++++++++++++++++++ //

// +++++++++++++++++++++++++++   GDI  ++++++++++++++++++++++++++ //
#define MEMDC(hTextInfo)  (hTextInfo->m_hGDI->m_hMemDC)
#define BITMAP(hTextInfo) (hTextInfo->m_hGDI->m_hBitmap)
#define BRUSH(hTextInfo)  (hTextInfo->m_hGDI->m_hBrush)
#define MTIMER(hTextInfo) (hTextInfo->m_hGDI->m_hMouseTimer)
#define STIMER(hTextInfo) (hTextInfo->m_hGDI->m_hSaveTimer)
// +++++++++++++++++++++++++++   End  ++++++++++++++++++++++++++ //

// +++++++++++++++++++++++++++   User ++++++++++++++++++++++++++ //
#define FONT(hTextInfo)       (hTextInfo->m_hUser->m_hFont)
#define CARETPOS(hTextInfo)   (hTextInfo->m_hUser->m_pCaretPos)
#define CHARSIZE(hTextInfo)   (hTextInfo->m_hUser->m_pCharSize)
#define CARETSIZE(hTextInfo)  (hTextInfo->m_hUser->m_pCaretSize)
#define WINDOWPOS(hTextInfo)  (hTextInfo->m_hUser->m_pWindowPos)
#define WINDOWSIZE(hTextInfo) (hTextInfo->m_hUser->m_pWindowSize)
#define PAGESIZE(hTextInfo)   (hTextInfo->m_hUser->m_pPageSize)
// +++++++++++++++++++++++++++   End  ++++++++++++++++++++++++++ //

// ++++++++++++++ Convenient Operation ++++++++++++++ //
BOOL operator==(POINT left, POINT right);

#define INRANGEX(x, l, r) ((x) >= (l) && (x) <= (r))
#define INRANGEY(y, t, b) ((y) >= (t) && (y) <  (b))

#define ZHWIDTH(l)  ((l) << 1)
#define USWIDTH(l)  (l)
#define ZHHEIGHT(l) (l)
#define USHEIGHT(l) (l)
// ++++++++++++++++++++++ END +++++++++++++++++++++++ //

// +++++++++++ FIND MACRO +++++++++++ //
#define AFTER_CARET   0x00000004
#define BEFORE_CARET  0x00000008
#define WHOLE_TEXT    0x00000010
#define FIND          0x00000020
#define REPLACE       0x00000040
// +++++++++++++++ END ++++++++++++++ //