#include "stdafx.h"
#include "editbox.h"
#include "api.h"

HTEXTINFO __stdcall CreateTextInfo(HWND hWnd)
{
	LPTEXTINFO lpTextInfo = new TEXTINFO{};
	if (!lpTextInfo)
		return HTEXTINFO(nullptr);

	if (!(lpTextInfo->m_hUser = CreateUser(hWnd)))
		return HTEXTINFO(nullptr);

	if (!(lpTextInfo->m_hKernel = CreateKernel(hWnd, lpTextInfo->m_hUser)))
		return HTEXTINFO(nullptr);

	if (!(lpTextInfo->m_hGDI = CreateGDI(hWnd, lpTextInfo->m_hUser)))
		return HTEXTINFO(nullptr);

	lpTextInfo->m_hWnd = hWnd;

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

	lpUser->m_pCharSize = POINT{ 8, 16 };
	lpUser->m_hFont = CreateFont(
		lpUser->m_pCharSize.y,		// 高度
		lpUser->m_pCharSize.x,		// 宽度
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
	lpUser->m_pCaretSize = POINT{ 1, 16 };
	lpUser->m_pCaretPos  = POINT{ 0, 0 };

	RECT rcClient;
	GetClientRect(hWnd, &rcClient);
	lpUser->m_pWindowSize = POINT{ rcClient.right - rcClient.left, rcClient.bottom - rcClient.top };
	lpUser->m_pWindowPos  = POINT{ 0, 0 };
	lpUser->m_pPageSize   = POINT{
		lpUser->m_pWindowSize.x / USWIDTH(lpUser->m_pCharSize.x),
		lpUser->m_pWindowSize.y / USHEIGHT(lpUser->m_pCharSize.y)
	};

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

	if (!(lpKernel->m_hText = CreateText(hUser->m_pCharSize.x, hUser->m_pCharSize.y)))
		return HTEXTKERNEL(nullptr);

	lpKernel->m_pTextSize = POINT{ 0, 0 };
	lpKernel->m_pStartPos = POINT{ 0, 0 };
	lpKernel->m_pEndPos   = POINT{ 0, 0 };

	return HTEXTKERNEL(lpKernel);
}

BOOL __stdcall ReleaseKernel(HWND hWnd, HTEXTKERNEL hKernel)
{
	delete hKernel;
	return (TRUE);
}

HTEXTGDI __stdcall CreateGDI(HWND hWnd, HTEXTUSER hUser)
{
	LPTEXTGDI lpGDI = new TEXTGDI{};
	if (!lpGDI)
		return HTEXTGDI(nullptr);

	HDC hdc = GetDC(hWnd);
	lpGDI->m_hMemDC  = CreateCompatibleDC(hdc);
	lpGDI->m_hBitmap = CreateCompatibleBitmap(hdc, hUser->m_pWindowSize.x, hUser->m_pWindowSize.y);
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