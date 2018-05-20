#pragma once

#include <Windows.h>

#include "resource.h"

#define STRING_SIZE 200

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
	HFONT m_hFont;			  // 字体实例
	POINT m_pCharPixelSize;   // 字符像素大小
	POINT m_pCaretPixelSize;  // 光标像素大小
	POINT m_pCaretPixelPos;   // 光标像素位置
	COORD m_cCaretCoord;      // 光标逻辑坐标(X列 Y行)

	POINT m_pMinCharPixelSize;
	POINT m_pMaxCharPixelSize;

	ULONGLONG m_fMask;
	TCHAR m_szFindWhat[STRING_SIZE];
	TCHAR m_szReplaceWhat[STRING_SIZE];
} TEXTUSER, *LPTEXTUSER, *HTEXTUSER;
HTEXTUSER __stdcall CreateUser(HWND hWnd);
BOOL      __stdcall ReleaseUser(HWND hWnd, HTEXTUSER hUser);

// 文本内核结构
typedef struct _TEXT_KERNEL
{
	HTEXT m_hText;          // 文本对象实例
	POINT m_pTextPixelSize; // 文本像素大小
	POINT m_pStartPixelPos; // 起点像素位置
	POINT m_pEndPixelPos;   // 终点像素位置
} TEXTKERNEL, *LPTEXTKERNEL, *HTEXTKERNEL;
HTEXTKERNEL __stdcall CreateKernel(HWND hWnd, HTEXTUSER hUser);
BOOL        __stdcall ReleaseKernel(HWND hWnd, HTEXTKERNEL hKernel);

/* 文本图形设备结构 */
typedef struct _TEXT_GDI
{
	POINT   m_pClientPixelSize; // 显示区像素大小
	POINT   m_pPaintPixelPos;   // 绘图区像素位置
	POINT   m_pPaintPixelSize;  // 绘图区像素大小
	POINT   m_pPageSize;        // 页面范围(X列 Y行)

	HWND    m_hStatus;          // 状态栏窗口
	POINT   m_pBufferPixelSize; // 缓冲位图像素大小
	HDC     m_hMemDC;           // 缓冲设备
	HBRUSH  m_hBrush;           // 默认背景刷
	HBITMAP m_hBitmap;          // 缓冲位图

	HTIMER  m_hMouseTimer;      // 鼠标移动定时器
	HTIMER  m_hSaveTimer;       // 自动保存定时器
} TEXTGDI, *LPTEXTGDI, *HTEXTGDI;
HTEXTGDI __stdcall CreateGDI(HWND hWnd, HINSTANCE hInst, HTEXTUSER hUser);
BOOL     __stdcall ReleaseGDI(HWND hWnd, HTEXTGDI hGDI);

// 文本结构
typedef struct _TEXT_INFO
{
	HTEXTKERNEL	m_hKernel;
	HTEXTGDI	m_hGDI;
	HTEXTUSER	m_hUser;
	HWND		m_hWnd;
	HINSTANCE   m_hInst;
} TEXTINFO, *LPTEXTINFO, *HTEXTINFO;
HTEXTINFO __stdcall CreateTextInfo(HWND hWnd, HINSTANCE hInst);
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
BOOL	MyInvalidateRect(HTEXTINFO, LONG, LONG, LONG, LONG);         // 设置无效区域

BOOL    AdjustCaretPosBeforeBackspace(HTEXTINFO); // 用户按Backspace键时 调整光标位置
BOOL    AdjustPaintPos(HTEXTINFO);                // 滑动窗口
BOOL    MoveCaret(HTEXTINFO, UINT);               // 移动光标

BOOL    SelectPaintPos(HTEXTINFO, POINT);         // 设置窗口位置
BOOL    SelectCaretPos(HTEXTINFO, POINT, COORD);         // 设置光标位置
BOOL    SelectHighlight(HTEXTINFO, POINT, POINT); // 设置高亮部分
BOOL    SelectTextSize(HTEXTINFO, POINT);         // 设置文本范围
BOOL    SelectWindow(HTEXTINFO, POINT, LPCWSTR);  // 命名文本窗口显示

BOOL    MyScrollWindow(HTEXTINFO, int, int);      // 滑动窗口

BOOL    PaintWindow(LPRECT, HTEXTINFO);           // 重绘窗口

BOOL    SelectClientSize(HTEXTINFO, LPRECT);      // 设置显示区大小(WM_SIZE)
BOOL    SelectCharSize(HTEXTINFO, LONG, LONG);    // 设置字体大小

// Convenient macro
// +++++++++++++++++++++++++++ Kernel ++++++++++++++++++++++++++ //
#define HTEXT(hTextInfo)    (hTextInfo->m_hKernel->m_hText)
#define TEXTSIZE(hTextInfo) (hTextInfo->m_hKernel->m_pTextPixelSize)
#define STARTPOS(hTextInfo) (hTextInfo->m_hKernel->m_pStartPixelPos)
#define ENDPOS(hTextInfo)   (hTextInfo->m_hKernel->m_pEndPixelPos)
// +++++++++++++++++++++++++++   End  ++++++++++++++++++++++++++ //

// +++++++++++++++++++++++++++   GDI  ++++++++++++++++++++++++++ //
#define PAINTSIZE(hTextInfo)  (hTextInfo->m_hGDI->m_pPaintPixelSize)
#define PAINTPOS(hTextInfo)   (hTextInfo->m_hGDI->m_pPaintPixelPos)
#define CLIENTSIZE(hTextInfo) (hTextInfo->m_hGDI->m_pClientPixelSize)
#define PAGESIZE(hTextInfo)   (hTextInfo->m_hGDI->m_pPageSize)
#define BUFFERSIZE(hTextInfo) (hTextInfo->m_hGDI->m_pBufferPixelSize)
#define MEMDC(hTextInfo)      (hTextInfo->m_hGDI->m_hMemDC)
#define BITMAP(hTextInfo)     (hTextInfo->m_hGDI->m_hBitmap)
#define BRUSH(hTextInfo)      (hTextInfo->m_hGDI->m_hBrush)
#define MTIMER(hTextInfo)     (hTextInfo->m_hGDI->m_hMouseTimer)
#define STIMER(hTextInfo)     (hTextInfo->m_hGDI->m_hSaveTimer)
#define STATUS(hTextInfo)     (hTextInfo->m_hGDI->m_hStatus)
// +++++++++++++++++++++++++++   End  ++++++++++++++++++++++++++ //

// +++++++++++++++++++++++++++   User ++++++++++++++++++++++++++ //
#define FONT(hTextInfo)          (hTextInfo->m_hUser->m_hFont)
#define CARETPOS(hTextInfo)      (hTextInfo->m_hUser->m_pCaretPixelPos)
#define CHARSIZE(hTextInfo)      (hTextInfo->m_hUser->m_pCharPixelSize)
#define CARETSIZE(hTextInfo)     (hTextInfo->m_hUser->m_pCaretPixelSize)
#define CARETCOORD(hTextInfo)    (hTextInfo->m_hUser->m_cCaretCoord)
#define MINCHARSIZE(hTextInfo)   (hTextInfo->m_hUser->m_pMinCharPixelSize)
#define MAXCHARSIZE(hTextInfo)   (hTextInfo->m_hUser->m_pMaxCharPixelSize)
#define MASK(hTextInfo)          (hTextInfo->m_hUser->m_fMask)
#define FINDSTRING(hTextInfo)    (hTextInfo->m_hUser->m_szFindWhat)
#define REPLACESTRING(hTextInfo) (hTextInfo->m_hUser->m_szReplaceWhat)
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
#define DEFAULT 0x0000000000000000
#define FIND    0x0010000000000000
#define REPLACE 0x0001000000000000
// +++++++++++++++ END ++++++++++++++ //

// +++++++++++++++++++ CONFIG ++++++++++++++++++++++ //
#define SUB_KEY_VALUE TEXT("*\\shell\\EditBox")
#define DEFAULT_PATH  TEXT("C:\\MiniWord\\Default\\")
// +++++++++++++++++++++ END +++++++++++++++++++++++ //