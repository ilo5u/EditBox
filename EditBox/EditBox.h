#pragma once

#include <Windows.h>

#include "resource.h"

// ��ʱ��
typedef struct _TIMER
{
public:
	_TIMER(const DWORD& tr);
	~_TIMER();

	DWORD     time_rest;
	HANDLE    time_mutex;
	HANDLE    time_thread_id;
} TIMER, *HTIMER;
HTIMER _stdcall CreateTimer(DWORD time_length); // ����һ���Զ�ʱ��
BOOL   _stdcall KillTimer(HTIMER timer);        // ����һ���Զ�ʱ��
BOOL            IsTimerStopped(HTIMER hTimer);  // ��ʱ���Ƿ�ֹͣ

// RESOURCE
class CText;
typedef CText *HTEXT, *LPTEXT;

// �ı��û��ṹ
typedef struct _TEXT_USER
{
	HFONT m_hFont;			// ����ʵ��
	POINT m_pCharSize;		// �ַ���С
	POINT m_pCaretSize;		// ����С
	POINT m_pCaretPos;		// ���λ��
	POINT m_pWindowPos;		// ����λ��
	POINT m_pWindowSize;	// ���ڴ�С
	POINT m_pPageSize;		// ҳ�淶Χ
} TEXTUSER, *LPTEXTUSER, *HTEXTUSER;
HTEXTUSER __stdcall CreateUser(HWND hWnd);
BOOL      __stdcall ReleaseUser(HWND hWnd, HTEXTUSER hUser);

// �ı��ں˽ṹ
typedef struct _TEXT_KERNEL
{
	HTEXT m_hText;			// �ı�ʵ��
	POINT m_pTextSize;		// �ı���Χ
	POINT m_pStartPos;		// ѡ�����
	POINT m_pEndPos;		// ѡ���յ�
} TEXTKERNEL, *LPTEXTKERNEL, *HTEXTKERNEL;
HTEXTKERNEL __stdcall CreateKernel(HWND hWnd, HTEXTUSER hUser);
BOOL        __stdcall ReleaseKernel(HWND hWnd, HTEXTKERNEL hKernel);

// �ı�ͼ���豸�ṹ
typedef struct _TEXT_GDI
{
	HDC     m_hMemDC;		// �����豸
	HBRUSH  m_hBrush;		// Ĭ�ϱ���ˢ
	HBITMAP m_hBitmap;		// ����λͼ
	HTIMER  m_hMouseTimer;	// ����ƶ���ʱ��
	HTIMER  m_hSaveTimer;	// �Զ����涨ʱ��
} TEXTGDI, *LPTEXTGDI, *HTEXTGDI;
HTEXTGDI __stdcall CreateGDI(HWND hWnd, HTEXTUSER hUser);
BOOL     __stdcall ReleaseGDI(HWND hWnd, HTEXTGDI hGDI);

// �ı��ṹ
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
BOOL	MyInvalidateRect(HWND, LONG, LONG, LONG, LONG);         // ������Ч����

BOOL    AdjustCaretPosBeforeBackspace(HWND, HTEXTINFO);         // �û���Backspace��ʱ �������λ��
BOOL    AdjustWindowPos(HWND, HTEXTINFO);                       // ��������
BOOL    MoveCaret(HWND, UINT, HTEXTINFO);                       // �ƶ����

BOOL    SelectWindowPos(HTEXTINFO, POINT);                      // ���ô���λ��
BOOL    SelectWindowSize(HTEXTINFO hTextInfo, LPRECT lpRect);   // ���ô��ڷ�Χ
BOOL    SelectCaretPos(HTEXTINFO, POINT);                       // ���ù��λ��
BOOL    SelectHighlight(HTEXTINFO, POINT, POINT);               // ���ø�������
BOOL    SelectTextSize(HTEXTINFO, POINT);                       // �����ı���Χ
BOOL    SelectUntitledWindow(HWND, HTEXTINFO);                  // Ĭ���ı�������ʾ
BOOL    SelectTitledWindow(HWND, HTEXTINFO, POINT, LPCWSTR);    // �����ı�������ʾ

BOOL    MyScrollWindow(HWND, int, int);                         // ��������
BOOL    DefaultFill(HWND, HTEXTINFO);                           // ���

BOOL    PaintWindow(LPPAINTSTRUCT, HTEXTINFO);                  // �ػ洰��

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