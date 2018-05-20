#include "stdafx.h"
#include "miniword.h"
#include "api.h"

#pragma comment(lib, "comctl32.lib")

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

BOOL __stdcall ReleaseTextInfo(HWND hWnd, HTEXTINFO hTextInfo)
{
	ReleaseGDI(hWnd, hTextInfo->m_hGDI);
	ReleaseKernel(hWnd, hTextInfo->m_hKernel);
	ReleaseUser(hWnd, hTextInfo->m_hUser);

	delete hTextInfo;
	return (TRUE);
}

HTEXTUSER __stdcall CreateUser(HWND hWnd)
{
	LPTEXTUSER lpUser = new TEXTUSER{};
	if (!lpUser)
		return HTEXTUSER(nullptr);

	lpUser->m_pCharPixelSize = POINT{ 8, 16 };
	lpUser->m_hFont = CreateFont(
		lpUser->m_pCharPixelSize.y,		// 高度
		lpUser->m_pCharPixelSize.x,		// 宽度
		0,		// 水平
		0,		// 倾斜
		400,	// 粗度
		0,		// 不倾斜
		0,		// 无下划线
		0,		// 无中划线
		GB2312_CHARSET,					// 字符集
		OUT_DEFAULT_PRECIS,				// 输出精度
		CLIP_DEFAULT_PRECIS,			// 裁剪精度
		DEFAULT_QUALITY,				// 输出质量
		DEFAULT_PITCH | FF_DONTCARE,	// 间距
		TEXT("New Curier")				// 名称
	);
	lpUser->m_pCaretPixelSize = POINT{ 1, 16 };
	lpUser->m_pCaretPixelPos  = POINT{ 0, 0 };
	lpUser->m_cCaretCoord     = COORD{ 0, 0 };

	lpUser->m_pMinCharPixelSize = POINT{ 6, 12 };
	lpUser->m_pMaxCharPixelSize = POINT{ 12, 24 };

	lpUser->m_fMask &= DEFAULT;
	ZeroMemory(lpUser->m_szFindWhat, STRING_SIZE);
	ZeroMemory(lpUser->m_szReplaceWhat, STRING_SIZE);

	return HTEXTUSER(lpUser);
}

BOOL __stdcall ReleaseUser(HWND hWnd, HTEXTUSER hUser)
{
	delete hUser;
	return (TRUE);
}

HTEXTKERNEL __stdcall CreateKernel(HWND hWnd, HTEXTUSER hUser)
{
	LPTEXTKERNEL lpKernel = new TEXTKERNEL{};
	if (!lpKernel)
		return HTEXTKERNEL(nullptr);

	if (!(lpKernel->m_hText = CreateText(hUser->m_pCharPixelSize.x, hUser->m_pCharPixelSize.y, DEFAULT_PATH)))
		return HTEXTKERNEL(nullptr);

	lpKernel->m_pTextPixelSize = POINT{ 0, 0 };
	lpKernel->m_pStartPixelPos = POINT{ 0, 0 };
	lpKernel->m_pEndPixelPos   = POINT{ 0, 0 };

	return HTEXTKERNEL(lpKernel);
}

BOOL __stdcall ReleaseKernel(HWND hWnd, HTEXTKERNEL hKernel)
{
	DestroyText(hKernel->m_hText);
	delete hKernel;
	return (TRUE);
}

HTEXTGDI __stdcall CreateGDI(HWND hWnd, HINSTANCE hInst, HTEXTUSER hUser)
{
	LPTEXTGDI lpGDI = new TEXTGDI{};
	if (!lpGDI)
		return HTEXTGDI(nullptr);

	// 获取显示区大小
	RECT rcClient;
	GetClientRect(hWnd, &rcClient);
	lpGDI->m_pClientPixelSize = POINT{
		rcClient.right  - rcClient.left,
		rcClient.bottom - rcClient.top
	};

	// 绘制滚动条
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

	// 绘制状态栏
	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_BAR_CLASSES;
	InitCommonControlsEx(&icex);
	lpGDI->m_hStatus = CreateWindow(STATUSCLASSNAME, TEXT(""), WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
		0, 0, 0, 0, hWnd, NULL, hInst, NULL);
	int nSize[2] = { (int)lpGDI->m_pClientPixelSize.x * 3 / 4, -1 };
	SendMessage(lpGDI->m_hStatus, SB_SETPARTS, sizeof(nSize) / sizeof(nSize[0]), (LPARAM)nSize);

	TCHAR szCaretCoord[MAX_PATH];
	wsprintf(szCaretCoord, TEXT("Ins"));
	SendMessage(lpGDI->m_hStatus, SB_SETTEXT, SBT_NOBORDERS | (WORD)0x00, (LPARAM)szCaretCoord);
	wsprintf(szCaretCoord, TEXT("%d rows, %d cols"), hUser->m_cCaretCoord.Y, hUser->m_cCaretCoord.X);
	SendMessage(lpGDI->m_hStatus, SB_SETTEXT, SBT_NOBORDERS | (WORD)0x01, (LPARAM)szCaretCoord);

	// 设置绘图区域大小
	RECT rcStatus;
	SendMessage(lpGDI->m_hStatus, SB_GETRECT, NULL, (LPARAM)&rcStatus);
	lpGDI->m_pPaintPixelSize = POINT{
		lpGDI->m_pClientPixelSize.x,
		lpGDI->m_pClientPixelSize.y - rcStatus.bottom
	};
	lpGDI->m_pPaintPixelPos = POINT{ 0, 0 };

	// 设置页面的行列数
	lpGDI->m_pPageSize = POINT{
		lpGDI->m_pPaintPixelSize.x / hUser->m_pCharPixelSize.x,
		lpGDI->m_pPaintPixelSize.y / hUser->m_pCharPixelSize.y
	};

	// 设置缓冲位图的大小
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

	// 待扩展
	lpGDI->m_hMouseTimer = nullptr;
	lpGDI->m_hSaveTimer  = nullptr;

	return HTEXTGDI(lpGDI);
}

BOOL __stdcall ReleaseGDI(HWND hWnd, HTEXTGDI hGDI)
{
	DeleteObject(hGDI->m_hBitmap);
	DeleteObject(hGDI->m_hBrush);
	DeleteObject(hGDI->m_hMemDC);
	delete hGDI;
	return (TRUE);
}