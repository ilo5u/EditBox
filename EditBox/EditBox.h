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

// ����һ���Զ�ʱ��
HTIMER _stdcall CreateTimer(DWORD time_length);

// ��ʱ���Ƿ�ֹͣ
BOOL IsTimerStopped(HTIMER hTimer);

// ����һ���Զ�ʱ��
BOOL _stdcall KillTimer(HTIMER timer);
// ��ʱ��

// RESOURCE
class CText;
typedef CText *HTEXT, *LPTEXT;

// �ı��ں˽ṹ
typedef struct _TEXT_KERNEL
{
	HTEXT m_hText;			// �ı�ʵ��
	POINT m_pTextSize;		// �ı���Χ
	POINT m_pStartPos;		// ѡ�����
	POINT m_pEndPos;		// ѡ���յ�
} TEXTKERNEL, *LPTEXTKERNEL, *HTEXTKERNEL;
HTEXTKERNEL __stdcall CreateKernel(HWND hWnd, HTEXTUSER hUser);
BOOL __stdcall ReleaseKernel(HWND hWnd, HTEXTKERNEL hKernel);

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
BOOL __stdcall ReleaseUser(HWND hWnd, HTEXTUSER hUser);

// �ı�ͼ���豸�ṹ
typedef struct _TEXT_GDI
{
	HDC m_hMemDC;			// �����豸
	HBRUSH m_hBrush;		// Ĭ�ϱ���ˢ
	HBITMAP m_hBitmap;		// ����λͼ
	HTIMER m_hMouseTimer;	// ����ƶ���ʱ��
	HTIMER m_hSaveTimer;	// �Զ����涨ʱ��
} TEXTGDI, *LPTEXTGDI, *HTEXTGDI;
HTEXTGDI __stdcall CreateGDI(HWND hWnd, HTEXTUSER hUser);
BOOL __stdcall ReleaseGDI(HWND hWnd, HTEXTGDI hGDI);

// �ı��ṹ
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