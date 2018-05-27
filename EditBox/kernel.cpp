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

#include "stdafx.h"
#include "miniword.h"
#include "api.h"

#pragma comment(lib, "comctl32.lib")

/*---------------------------------------------
	@Description:
		����MiniWord��Ϣ�������

	@Paramter:
		hWnd: �����ھ��
		hInst: ��ǰʵ��

	@Return:
		������

	@Author:
		����
---------------------------------------------*/
HTEXTINFO __stdcall CreateTextInfo(HWND hWnd, HINSTANCE hInst)
{
	LPTEXTINFO lpTextInfo = new TEXTINFO{};
	if (!lpTextInfo)
		return HTEXTINFO(nullptr);

	if (!(lpTextInfo->m_hUser = CreateUser(hWnd)))
		return HTEXTINFO(nullptr);

	if (!(lpTextInfo->m_hKernel = CreateKernel(hWnd, lpTextInfo->m_hUser)))
		return HTEXTINFO(nullptr);

	if (!(lpTextInfo->m_hGDI = CreateGDI(hWnd, hInst, lpTextInfo->m_hUser)))
		return HTEXTINFO(nullptr);

	lpTextInfo->m_hWnd  = hWnd;
	lpTextInfo->m_hInst = hInst;

	return HTEXTINFO(lpTextInfo);
}

/*---------------------------------------------
	@Description:
		�ͷ�MiniWord��Ϣ�������

	@Paramter:
		hWnd: �����ھ��
		hTextInfo: ������

	@Return:
		�ͷŽ����Ĭ��ΪTRUE��

	@Author:
		����
---------------------------------------------*/
BOOL __stdcall ReleaseTextInfo(HWND hWnd, HTEXTINFO hTextInfo)
{
	ReleaseGDI(hWnd, hTextInfo->m_hGDI);
	ReleaseKernel(hWnd, hTextInfo->m_hKernel);
	ReleaseUser(hWnd, hTextInfo->m_hUser);

	delete hTextInfo;
	return (TRUE);
}

/*---------------------------------------------
	@Description:
		�����û���Ϣ�������

	@Paramter:
		hWnd: �����ھ��

	@Return:
		������

	@Author:
		����
---------------------------------------------*/
HTEXTUSER __stdcall CreateUser(HWND hWnd)
{
	LPTEXTUSER lpUser = new TEXTUSER{};
	if (!lpUser)
		return HTEXTUSER(nullptr);

	lpUser->m_pCharPixelSize = POINT{ 8, 16 };
	lpUser->m_hFont = CreateFont(
		lpUser->m_pCharPixelSize.y,		// �߶�
		lpUser->m_pCharPixelSize.x,		// ���
		0,		// ˮƽ
		0,		// ��б
		400,	// �ֶ�
		0,		// ����б
		0,		// ���»���
		0,		// ���л���
		GB2312_CHARSET,					// �ַ���
		OUT_DEFAULT_PRECIS,				// �������
		CLIP_DEFAULT_PRECIS,			// �ü�����
		DEFAULT_QUALITY,				// �������
		DEFAULT_PITCH | FF_DONTCARE,	// ���
		TEXT("New Curier")				// ����
	);
	lpUser->m_pCaretPixelSize = POINT{ 1, 16 };
	lpUser->m_pCaretPixelPos  = POINT{ 0, 0 };
	lpUser->m_cCaretCoord     = COORD{ 0, 0 };

	lpUser->m_pMinCharPixelSize = POINT{ 6, 12 };
	lpUser->m_pMaxCharPixelSize = POINT{ 12, 24 };

	lpUser->m_fMask &= DEFAULT;
	ZeroMemory(lpUser->m_szFindWhat, MAX_LOADSTRING);
	ZeroMemory(lpUser->m_szReplaceWhat, MAX_LOADSTRING);

	return HTEXTUSER(lpUser);
}

/*---------------------------------------------
	@Description:
		�ͷ��û���Ϣ�������

	@Paramter:
		hWnd: �����ھ��
		hUser: ������

	@Return:
		�ͷŽ����Ĭ��ΪTRUE��

	@Author:
		����
---------------------------------------------*/
BOOL __stdcall ReleaseUser(HWND hWnd, HTEXTUSER hUser)
{
	delete hUser;
	return (TRUE);
}

/*---------------------------------------------
	@Description:
		�����ں���Ϣ�������

	@Paramter:
		hWnd: �����ھ��
		hUser: �û���Ϣ���������

	@Return:
		������

	@Author:
		����
---------------------------------------------*/
HTEXTKERNEL __stdcall CreateKernel(HWND hWnd, HTEXTUSER hUser)
{
	LPTEXTKERNEL lpKernel = new TEXTKERNEL{};
	if (!lpKernel)
		return HTEXTKERNEL(nullptr);

	if (!(lpKernel->m_hText = CreateText(hUser->m_pCharPixelSize.x, hUser->m_pCharPixelSize.y)))
		return HTEXTKERNEL(nullptr);

	lpKernel->m_pTextPixelSize = POINT{ 0, 0 };
	lpKernel->m_pStartPixelPos = POINT{ 0, 0 };
	lpKernel->m_pEndPixelPos   = POINT{ 0, 0 };

	return HTEXTKERNEL(lpKernel);
}

/*---------------------------------------------
	@Description:
		�ͷ��ں���Ϣ�������

	@Paramter:
		hWnd: �����ھ��
		hKernel: ������

	@Return:
		�ͷŽ����Ĭ��ΪTRUE��

	@Author:
		����
---------------------------------------------*/
BOOL __stdcall ReleaseKernel(HWND hWnd, HTEXTKERNEL hKernel)
{
	DestroyText(hKernel->m_hText);
	delete hKernel;
	return (TRUE);
}

/*---------------------------------------------
	@Description:
		����ͼ���豸��Ϣ�������

	@Paramter:
		hWnd: �����ھ��
		hInst: ��ǰʵ��
		hUser: �û���Ϣ���������

	@Return:
		������

	@Author:
		����
---------------------------------------------*/
HTEXTGDI __stdcall CreateGDI(HWND hWnd, HINSTANCE hInst, HTEXTUSER hUser)
{
	LPTEXTGDI lpGDI = new TEXTGDI{};
	if (!lpGDI)
		return HTEXTGDI(nullptr);

	// ��ȡ��ʾ����С
	RECT rcClient;
	GetClientRect(hWnd, &rcClient);
	lpGDI->m_pClientPixelSize = POINT{
		rcClient.right  - rcClient.left,
		rcClient.bottom - rcClient.top
	};

	// ���ƹ�����
	SCROLLINFO shInfo;
	shInfo.cbSize = sizeof(SCROLLINFO);
	shInfo.fMask  = SIF_ALL | SIF_DISABLENOSCROLL;
	GetScrollInfo(hWnd, SB_HORZ, &shInfo);

	shInfo.nMin  = 0;
	shInfo.nMax  = 0;
	shInfo.nPage = 0;

	shInfo.cbSize = sizeof(SCROLLINFO);
	shInfo.fMask  = SIF_ALL | SIF_DISABLENOSCROLL;
	SetScrollInfo(hWnd, SB_HORZ, &shInfo, TRUE);

	SCROLLINFO svInfo;
	svInfo.cbSize = sizeof(SCROLLINFO);
	svInfo.fMask  = SIF_ALL | SIF_DISABLENOSCROLL;
	GetScrollInfo(hWnd, SB_VERT, &svInfo);

	svInfo.nMin  = 0;
	svInfo.nMax  = 0;
	svInfo.nPage = 0;

	svInfo.cbSize = sizeof(SCROLLINFO);
	svInfo.fMask  = SIF_ALL | SIF_DISABLENOSCROLL;
	SetScrollInfo(hWnd, SB_VERT, &svInfo, TRUE);

	// ����״̬��
	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_BAR_CLASSES;
	InitCommonControlsEx(&icex);
	lpGDI->m_hStatus = CreateWindow(STATUSCLASSNAME, TEXT(""), WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
		0, 0, 0, 0, hWnd, NULL, hInst, NULL);
	int nSize[2] = { (int)lpGDI->m_pClientPixelSize.x * 3 / 4, -1 };
	SendMessage(lpGDI->m_hStatus, SB_SETPARTS, sizeof(nSize) / sizeof(nSize[0]), (LPARAM)nSize);

	TCHAR szCaretCoord[MAX_PATH];
	wsprintf(szCaretCoord, TEXT("%d rows, %d cols"), hUser->m_cCaretCoord.Y, hUser->m_cCaretCoord.X);
	SendMessage(lpGDI->m_hStatus, SB_SETTEXT, SBT_NOBORDERS | (WORD)0x01, (LPARAM)szCaretCoord);

	// ���û�ͼ�����С
	RECT rcStatus;
	SendMessage(lpGDI->m_hStatus, SB_GETRECT, NULL, (LPARAM)&rcStatus);
	lpGDI->m_pPaintPixelSize = POINT{
		lpGDI->m_pClientPixelSize.x,
		lpGDI->m_pClientPixelSize.y - rcStatus.bottom
	};
	lpGDI->m_pPaintPixelPos = POINT{ 0, 0 };

	// ����ҳ���������
	lpGDI->m_pPageSize = POINT{
		lpGDI->m_pPaintPixelSize.x / hUser->m_pCharPixelSize.x,
		lpGDI->m_pPaintPixelSize.y / hUser->m_pCharPixelSize.y
	};

	// ���û���λͼ�Ĵ�С
	lpGDI->m_pBufferPixelSize = POINT{
		(lpGDI->m_pPageSize.x + 1) * hUser->m_pCharPixelSize.x,
		(lpGDI->m_pPageSize.y + 1) * hUser->m_pCharPixelSize.y
	};

	HDC hdc = GetDC(hWnd);
	lpGDI->m_hMemDC  = CreateCompatibleDC(hdc);
	lpGDI->m_hBitmap = CreateCompatibleBitmap(hdc, lpGDI->m_pBufferPixelSize.x, lpGDI->m_pBufferPixelSize.y);
	lpGDI->m_hBrush  = CreateSolidBrush(RGB(0xFF, 0xFF, 0xFF));

	SelectObject(lpGDI->m_hMemDC, lpGDI->m_hBitmap);
	SelectObject(lpGDI->m_hMemDC, lpGDI->m_hBrush);
	SelectObject(lpGDI->m_hMemDC, hUser->m_hFont);
	ReleaseDC(hWnd, hdc);

	// ����չ
	lpGDI->m_hMouseTimer = nullptr;
	lpGDI->m_hSaveTimer  = nullptr;

	return HTEXTGDI(lpGDI);
}

/*---------------------------------------------
	@Description:
		�ͷ�ͼ���豸��Ϣ�������

	@Paramter:
		hWnd: �����ھ��
		hGDI: ������

	@Return:
		�ͷŽ����Ĭ��ΪTRUE��

	@Author:
		����
---------------------------------------------*/
BOOL __stdcall ReleaseGDI(HWND hWnd, HTEXTGDI hGDI)
{
	DeleteObject(hGDI->m_hBitmap);
	DeleteObject(hGDI->m_hBrush);
	DeleteObject(hGDI->m_hMemDC);
	delete hGDI;
	return (TRUE);
}