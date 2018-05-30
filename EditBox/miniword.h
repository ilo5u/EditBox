/*--------------------------------------------------------------------------------------------------
// File : *.cpp, *.h
//
// Description:
//             (�˴����ע��)
//
// Author: XXX
//
// Date: 2018-XX-XX
//
--------------------------------------------------------------------------------------------------*/

#pragma once

#include <Windows.h>
#include <shellapi.h>

#include "resource.h"

#define MAX_LOADSTRING 100

////////////////////////// ��ʱ��ģ�� ///////////////////////////////
typedef struct _TIMER
{
public:
	_TIMER(const DWORD& tr);
	~_TIMER();

	DWORD     time_rest;       // ��ʱ��ʱ��
	HANDLE    time_mutex;      // ���������¼�����
	HANDLE    time_thread_id;  // ��ʱ�����е����߳�id
} TIMER, *HTIMER;

/* ------------ struct TIMER's function statement ----------- */
HTIMER _stdcall CreateTimer(DWORD time_length);
BOOL   _stdcall KillTimer(HTIMER timer);
BOOL            IsTimerStopped(HTIMER hTimer);
/* ------------------------ end ----------------------------- */

////////////////////////// �ں�ģ�� /////////////////////////////////
class CText;
typedef CText *HTEXT, *LPTEXT;

// �ı��û��ṹ
typedef struct _TEXT_USER
{
	HFONT m_hFont;			  // ����ʵ��
	POINT m_pCharPixelSize;   // �ַ����ش�С
	POINT m_pCaretPixelSize;  // ������ش�С
	POINT m_pCaretPixelPos;   // �������λ��
	COORD m_cCaretCoord;      // ����߼�����(X�� Y��)

	POINT m_pMinCharPixelSize;  // ��С�ַ����ش�С
	POINT m_pMaxCharPixelSize;  // ����ַ����ش�С

	ULONGLONG m_fMask;                     // ����ģʽ
	TCHAR m_szFindWhat[MAX_LOADSTRING];    // ���Ҵ�
	TCHAR m_szReplaceWhat[MAX_LOADSTRING]; // �滻��
} TEXTUSER, *LPTEXTUSER, *HTEXTUSER;
/* ----------- struct TEXTUSER's function statement ------------ */
HTEXTUSER __stdcall CreateUser(HWND hWnd);
BOOL      __stdcall ReleaseUser(HWND hWnd, HTEXTUSER hUser);
/* --------------------------- end ----------------------------- */

// �ı��ں˽ṹ
typedef struct _TEXT_KERNEL
{
	HTEXT m_hText;          // �ı�����ʵ��
	POINT m_pTextPixelSize; // �ı����ش�С
	POINT m_pStartPixelPos; // �������λ��
	POINT m_pEndPixelPos;   // �յ�����λ��
} TEXTKERNEL, *LPTEXTKERNEL, *HTEXTKERNEL;
/* ----------- struct TEXTKERNEL's function statement ------------- */
HTEXTKERNEL __stdcall CreateKernel(HWND hWnd, HTEXTUSER hUser);
BOOL        __stdcall ReleaseKernel(HWND hWnd, HTEXTKERNEL hKernel);
/* ----------------------------- end ------------------------------ */

/* �ı�ͼ���豸�ṹ */
typedef struct _TEXT_GDI
{
	POINT   m_pClientPixelSize; // ��ʾ�����ش�С
	POINT   m_pPaintPixelPos;   // ��ͼ������λ��
	POINT   m_pPaintPixelSize;  // ��ͼ�����ش�С
	POINT   m_pPageSize;        // ҳ�淶Χ(X�� Y��)

	HWND    m_hStatus;          // ״̬������
	POINT   m_pBufferPixelSize; // ����λͼ���ش�С
	HDC     m_hMemDC;           // �����豸
	HBRUSH  m_hBrush;           // Ĭ�ϱ���ˢ
	HBITMAP m_hBitmap;          // ����λͼ

	HTIMER  m_hMouseTimer;      // ����ƶ���ʱ��
	HTIMER  m_hSaveTimer;       // �Զ����涨ʱ��
} TEXTGDI, *LPTEXTGDI, *HTEXTGDI;
/* -------------- struct TEXTGDI's function statement ------------------- */
HTEXTGDI __stdcall CreateGDI(HWND hWnd, HINSTANCE hInst, HTEXTUSER hUser);
BOOL     __stdcall ReleaseGDI(HWND hWnd, HTEXTGDI hGDI);
/* -------------------------------- end --------------------------------- */

// �ı��ṹ
typedef struct _TEXT_INFO
{
	HTEXTKERNEL	m_hKernel;  // �ں˹�����
	HTEXTGDI	m_hGDI;     // ͼ���豸������
	HTEXTUSER	m_hUser;    // �û�������
	HWND		m_hWnd;     // �����ھ��
	HINSTANCE   m_hInst;    // ��ǰӦ��ʵ��
} TEXTINFO, *LPTEXTINFO, *HTEXTINFO;
/* ------------- struct TEXTINFO's function statement ------------- */
HTEXTINFO __stdcall CreateTextInfo(HWND hWnd, HINSTANCE hInst);
BOOL      __stdcall ReleaseTextInfo(HWND hWnd, HTEXTINFO hTextInfo);
/* ---------------------------- end ------------------------------- */

//////////////////////////// IOģ�� /////////////////////////////////
BOOL WINAPI MyTextOutW(_In_ HDC hdc, 
	_In_ int x, _In_ int y, _In_reads_(c) LPCWSTR lpString, 
	_In_ int c, _In_ short s, _In_ short e, _In_ int width);

#ifdef UNICODE
#define MyTextOut MyTextOutW
#else
#define MyTextOut MyTextOutA
#endif // UNICODE

//////////////////////////// ��ͼģ�� ///////////////////////////////
BOOL	MyInvalidateRect(HTEXTINFO, 
	LONG, LONG, LONG, LONG); // ������Ч����

BOOL    AdjustCaretPosBeforeBackspace(HTEXTINFO); // �û���Backspace��ʱ �������λ��
BOOL    AdjustPaintPos(HTEXTINFO);                // ��������
BOOL    MoveCaret(HTEXTINFO, UINT);               // �ƶ����

BOOL    SelectPaintPos(HTEXTINFO, POINT);         // ���ô���λ��
BOOL    SelectCaretPos(HTEXTINFO, POINT, COORD);         // ���ù��λ��
BOOL    SelectHighlight(HTEXTINFO, POINT, POINT); // ���ø�������
BOOL    SelectTextSize(HTEXTINFO, POINT);         // �����ı���Χ
BOOL    SelectWindow(HTEXTINFO, POINT, LPCWSTR);  // �����ı�������ʾ

BOOL    MyScrollWindow(HTEXTINFO, int, int);      // ��������

BOOL    PaintWindow(HTEXTINFO, LPRECT);           // �ػ洰��

BOOL    SelectClientSize(HTEXTINFO, LPRECT);      // ������ʾ����С(WM_SIZE)
BOOL    SelectCharSize(HTEXTINFO, LONG, LONG);    // ���������С

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
#define SUB_KEY_VALUE      TEXT("*\\shell\\MiniWord")
#define DEFAULT_PATH       TEXT("C:\\MiniWord\\")
#define HELP_DOCUMENT_PATH TEXT("C:\\MiniWord\\Help\\readme.txt")
#define HELP_DOCUMENT_DIR  TEXT("C:\\MiniWord\\Help\\")
#define DOCUMENT_LENGTH    2909
// +++++++++++++++++++++ END +++++++++++++++++++++++ //