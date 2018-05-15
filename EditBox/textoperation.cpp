#include "stdafx.h"
#include "editbox.h"
#include "debug.h"

BOOL MyInvalidateRect(HWND hWnd, LONG left, LONG right, LONG top, LONG bottom)
{
	RECT rcPaint{ left, top, right, bottom };
	InvalidateRect(hWnd, &rcPaint, FALSE);

	return (TRUE);
}

BOOL operator==(POINT left, POINT right)
{
	return (left.x == right.x) && (left.y == right.y);
}

BOOL AdjustCaretPosBeforeBackspace(HWND hWnd, HTEXTINFO hTextInfo)
{
	if (STARTPOS(hTextInfo) == ENDPOS(hTextInfo))
	{	// ��ǰδѡ��Ƭ�� ɾ�����ǰһ���ַ�
		POINT pOldEndPos = ENDPOS(hTextInfo);                        // ��¼ѡ��β��
		SendMessage(hWnd, WM_KEYDOWN, VK_LEFT, NULL);                // �������
		SelectHighlight(hTextInfo, STARTPOS(hTextInfo), pOldEndPos); // �����������(����һ���ַ�)
	}

	return (TRUE);
}

BOOL AdjustWindowPos(HWND hWnd, HTEXTINFO hTextInfo)
{
	// ����ƫ����
	int xOffset = INRANGEX(CARETPOS(hTextInfo).x, 
		WINDOWPOS(hTextInfo).x, 
		WINDOWPOS(hTextInfo).x + PAGESIZE(hTextInfo).x * USWIDTH(CHARSIZE(hTextInfo).x)
	) ?	0 : CARETPOS(hTextInfo).x - WINDOWPOS(hTextInfo).x - (PAGESIZE(hTextInfo).x >> 1) * USWIDTH(CHARSIZE(hTextInfo).x);

	int yOffset = INRANGEY(CARETPOS(hTextInfo).y, 
		WINDOWPOS(hTextInfo).y, 
		WINDOWPOS(hTextInfo).y + PAGESIZE(hTextInfo).y * USHEIGHT(CHARSIZE(hTextInfo).y)
	) ? 0 : CARETPOS(hTextInfo).y < WINDOWPOS(hTextInfo).y 
		? CARETPOS(hTextInfo).y - WINDOWPOS(hTextInfo).y
		: CARETPOS(hTextInfo).y - WINDOWPOS(hTextInfo).y - (PAGESIZE(hTextInfo).y - 1) * USHEIGHT(CHARSIZE(hTextInfo).y);

	// ���ù�����
	SCROLLINFO shInfo;
	shInfo.cbSize = sizeof(SCROLLINFO);
	shInfo.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;
	GetScrollInfo(hWnd, SB_HORZ, &shInfo);

	int iOldHorzPos = shInfo.nPos;
	shInfo.nPos = shInfo.nPos + xOffset / USWIDTH(CHARSIZE(hTextInfo).x);
	if (shInfo.nPos < shInfo.nMin)
		shInfo.nPos = shInfo.nMin;
	if (shInfo.nPos > shInfo.nMax - (int)shInfo.nPage + 1)
		shInfo.nPos = shInfo.nMax - (int)shInfo.nPage + 1;
	int iNowHorzPos = shInfo.nPos;

	shInfo.cbSize = sizeof(SCROLLINFO);
	shInfo.fMask = SIF_POS;
	SetScrollInfo(hWnd, SB_HORZ, &shInfo, TRUE);

	SCROLLINFO svInfo;
	svInfo.cbSize = sizeof(SCROLLINFO);
	svInfo.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;
	GetScrollInfo(hWnd, SB_VERT, &svInfo);

	int iOldVertPos = svInfo.nPos;
	svInfo.nPos = svInfo.nPos + yOffset / USHEIGHT(CHARSIZE(hTextInfo).y);
	if (svInfo.nPos < svInfo.nMin)
		svInfo.nPos = svInfo.nMin;
	if (svInfo.nPos > svInfo.nMax - (int)svInfo.nPage + 1)
		svInfo.nPos = svInfo.nMax - svInfo.nPage + 1;
	int iNowVertPos = svInfo.nPos;

	svInfo.cbSize = sizeof(SCROLLINFO);
	svInfo.fMask = SIF_POS;
	SetScrollInfo(hWnd, SB_VERT, &svInfo, TRUE);

	SelectWindowPos(hTextInfo, 
		POINT{ iNowHorzPos * USWIDTH(CHARSIZE(hTextInfo).x), iNowVertPos * USHEIGHT(CHARSIZE(hTextInfo).y) }
	); // ���ô���λ��

	MyScrollWindow(hWnd, 
		(iOldHorzPos - iNowHorzPos) * USWIDTH(CHARSIZE(hTextInfo).x), 
		(iOldVertPos - iNowVertPos) * USHEIGHT(CHARSIZE(hTextInfo).y)
	);	// ��������

	MyInvalidateRect(hWnd,
		0, WINDOWSIZE(hTextInfo).x,
		STARTPOS(hTextInfo).y - WINDOWPOS(hTextInfo).y,
		ENDPOS(hTextInfo).y - WINDOWPOS(hTextInfo).y + USHEIGHT(CHARSIZE(hTextInfo).y));

	return (TRUE);
}

BOOL MoveCaret(HWND hWnd, UINT message, HTEXTINFO hTextInfo)
{
	RVALUE result;
	if ((result =
		UserMessageProc(HTEXT(hTextInfo),
			CARETPOS(hTextInfo).x, CARETPOS(hTextInfo).y,
			message, NULL, NULL)) != UR_ERROR)
	{
		SelectCaretPos(hTextInfo,
			POINT{ (LONG)LODWORD(result), (LONG)HIDWORD(result) }
		);	// ���ù��λ��
		AdjustWindowPos(hWnd, hTextInfo);
		SelectHighlight(hTextInfo,
			CARETPOS(hTextInfo),
			CARETPOS(hTextInfo)
		);	// ���ø�������(��)
	}
	return (TRUE);
}

BOOL SelectWindowPos(HTEXTINFO hTextInfo, POINT thePos)
{
	WINDOWPOS(hTextInfo) = thePos;
	return (TRUE);
}

BOOL SelectWindowSize(HTEXTINFO hTextInfo, LPRECT lpRect)
{
	WINDOWSIZE(hTextInfo) = POINT{
		lpRect->right - lpRect->left,
		lpRect->bottom - lpRect->top
	};

	PAGESIZE(hTextInfo) = POINT{
		WINDOWSIZE(hTextInfo).x / USWIDTH(CHARSIZE(hTextInfo).x) - 2,
		WINDOWSIZE(hTextInfo).y / USHEIGHT(CHARSIZE(hTextInfo).y) - 1
	};

	SCROLLINFO shInfo;
	shInfo.cbSize = sizeof(SCROLLINFO);
	shInfo.fMask = SIF_PAGE;
	shInfo.nPage = PAGESIZE(hTextInfo).x;
	SetScrollInfo(hTextInfo->m_hWnd, SB_HORZ, &shInfo, TRUE);

	SCROLLINFO svInfo;
	svInfo.cbSize = sizeof(SCROLLINFO);
	svInfo.fMask = SIF_PAGE;
	svInfo.nPage = PAGESIZE(hTextInfo).y;
	SetScrollInfo(hTextInfo->m_hWnd, SB_VERT, &svInfo, TRUE);

	return (TRUE);
}

BOOL SelectCaretPos(HTEXTINFO hTextInfo, POINT thePos)
{
	CARETPOS(hTextInfo) = thePos;
	if (!INRANGEX(thePos.x,
		WINDOWPOS(hTextInfo).x,
		WINDOWPOS(hTextInfo).x + PAGESIZE(hTextInfo).x * USWIDTH(CHARSIZE(hTextInfo).x))
		|| !INRANGEY(thePos.y,
			WINDOWPOS(hTextInfo).y,
			WINDOWPOS(hTextInfo).y + PAGESIZE(hTextInfo).y * USHEIGHT(CHARSIZE(hTextInfo).y)))
	{	// ���δ���ڴ�����
		HideCaret(hTextInfo->m_hWnd);
	}
	else
		ShowCaret(hTextInfo->m_hWnd);
	SetCaretPos(CARETPOS(hTextInfo).x - WINDOWPOS(hTextInfo).x, CARETPOS(hTextInfo).y - WINDOWPOS(hTextInfo).y);
	return (TRUE);
}

BOOL SelectHighlight(HTEXTINFO hTextInfo, POINT theStart, POINT theEnd)
{
	MyInvalidateRect(hTextInfo->m_hWnd,
		0, WINDOWSIZE(hTextInfo).x,
		STARTPOS(hTextInfo).y - WINDOWPOS(hTextInfo).y, 
		ENDPOS(hTextInfo).y - WINDOWPOS(hTextInfo).y + USHEIGHT(CHARSIZE(hTextInfo).y)
	);

	STARTPOS(hTextInfo) = theStart;
	ENDPOS(hTextInfo)   = theEnd;

	MyInvalidateRect(hTextInfo->m_hWnd,
		0, WINDOWSIZE(hTextInfo).x,
		STARTPOS(hTextInfo).y - WINDOWPOS(hTextInfo).y,
		ENDPOS(hTextInfo).y - WINDOWPOS(hTextInfo).y + USHEIGHT(CHARSIZE(hTextInfo).y)
	);

	return (TRUE);
}

BOOL SelectTextSize(HTEXTINFO hTextInfo, POINT theSize)
{
	TEXTSIZE(hTextInfo) = theSize;

	// ���ù�����
	SCROLLINFO shInfo;
	shInfo.cbSize = sizeof(SCROLLINFO);
	shInfo.fMask = SIF_RANGE | SIF_PAGE;
	GetScrollInfo(hTextInfo->m_hWnd, SB_HORZ, &shInfo);

	shInfo.nPage = WINDOWSIZE(hTextInfo).x / USWIDTH(CHARSIZE(hTextInfo).x) - 2;
	shInfo.nMin = 0;
	shInfo.nMax = max(theSize.x / USWIDTH(CHARSIZE(hTextInfo).x) - 1, (int)shInfo.nPage - 1);

	shInfo.cbSize = sizeof(SCROLLINFO);
	shInfo.fMask = SIF_RANGE | SIF_PAGE;
	SetScrollInfo(hTextInfo->m_hWnd, SB_HORZ, &shInfo, FALSE);

	SCROLLINFO svInfo;
	svInfo.cbSize = sizeof(SCROLLINFO);
	svInfo.fMask = SIF_RANGE | SIF_PAGE;
	GetScrollInfo(hTextInfo->m_hWnd, SB_VERT, &svInfo);

	svInfo.nPage = WINDOWSIZE(hTextInfo).y / USHEIGHT(CHARSIZE(hTextInfo).y) - 1;
	svInfo.nMin  = 0;
	svInfo.nMax  = max(theSize.y / USHEIGHT(CHARSIZE(hTextInfo).y), (int)svInfo.nPage - 1);

	svInfo.cbSize = sizeof(SCROLLINFO);
	svInfo.fMask = SIF_RANGE | SIF_PAGE;
	SetScrollInfo(hTextInfo->m_hWnd, SB_VERT, &svInfo, FALSE);

	return (TRUE);
}

BOOL SelectUntitledWindow(HWND hWnd, HTEXTINFO hTextInfo)
{
	SetWindowText(hWnd, TEXT("Untitled.txt"));

	RECT rect;
	GetClientRect(hWnd, &rect);
	SelectWindowSize(hTextInfo, &rect);			// ���ô��ڴ�С���̶�ΪУ׼�����ʾ���֣�
	SelectWindowPos(hTextInfo, POINT{ 0, 0 });	// ���ô���λ�ã����Ͻǣ�

	SelectTextSize(hTextInfo, POINT{ 0 , 0 });	// �����ı���Χ�����ı���

	CreateCaret(hWnd, NULL,
		CARETSIZE(hTextInfo).y,
		CARETSIZE(hTextInfo).y
	); // �������

	SelectCaretPos(hTextInfo, POINT{ 0 , 0 });	// ���ù��λ�ã����Ͻǣ�
	SelectHighlight(hTextInfo,
		CARETPOS(hTextInfo),
		CARETPOS(hTextInfo)
	);	// ���ø������֣��ޣ�

	return (TRUE);
}
BOOL SelectTitledWindow(HWND hWnd, HTEXTINFO hTextInfo, POINT pTextSize, LPCWSTR lpszFileName)
{
	SetWindowText(hWnd, lpszFileName);	        // ���ô��ڱ���Ϊ�ļ���

	RECT rect;
	GetClientRect(hWnd, &rect);
	SelectWindowSize(hTextInfo, &rect);			// ���ô��ڴ�С���̶�ΪУ׼�����ʾ���֣�
	SelectWindowPos(hTextInfo, POINT{ 0, 0 });	// ���ô���λ�ã����Ͻǣ�

	SelectCaretPos(hTextInfo, POINT{ 0, 0 });	// ���ù��λ��(���Ͻ�)
	SelectHighlight(hTextInfo,
		CARETPOS(hTextInfo),
		CARETPOS(hTextInfo)
	);	// ���ø�������(��)

	MyInvalidateRect(hWnd,
		0, WINDOWSIZE(hTextInfo).x,
		0, WINDOWSIZE(hTextInfo).y
	);	// ���´�����ʾ(ȫ��)
	return (TRUE);
}

BOOL MyScrollWindow(HWND hWnd, int xOffset, int yOffset)
{
	RECT rcScroll;
	GetClientRect(hWnd, &rcScroll);
	RECT rcPaint;
	ScrollWindowEx(hWnd, xOffset, yOffset, &rcScroll, NULL, NULL, &rcPaint, SW_INVALIDATE);
	InvalidateRect(hWnd, &rcPaint, FALSE);

	return (TRUE);
}

BOOL DefaultFill(HWND hWnd, HTEXTINFO hTextInfo)
{
	GetDC(hWnd);
	RECT rcClient;
	GetClientRect(hWnd, &rcClient);

	HDC hdc = GetDC(hWnd);

	RECT rcRight;	// �ұ߽�
	SetRect(&rcRight, PAGESIZE(hTextInfo).x * USWIDTH(CHARSIZE(hTextInfo).x), 0, rcClient.right, rcClient.bottom);
	FillRect(hdc, &rcRight, BRUSH(hTextInfo));

	RECT rcBottom;	// �±߽�
	SetRect(&rcBottom, 0, PAGESIZE(hTextInfo).y * USHEIGHT(CHARSIZE(hTextInfo).y), rcClient.right, rcClient.bottom);
	FillRect(hdc, &rcBottom, BRUSH(hTextInfo));

	ReleaseDC(hWnd, hdc);

	return (TRUE);
}

BOOL PaintWindow(LPPAINTSTRUCT lpPaint, HTEXTINFO hTextInfo)
{
	RECT rect{ 0, 0, WINDOWSIZE(hTextInfo).x, WINDOWSIZE(hTextInfo).y };
	FillRect(MEMDC(hTextInfo), &rect, BRUSH(hTextInfo));

	RVALUE result;
	LPWSTR lpchText = nullptr;
	for (LONG yOffset = 0;
		yOffset < WINDOWSIZE(hTextInfo).y; yOffset += USHEIGHT(CHARSIZE(hTextInfo)).y)
	{
		if ((result =
			UserMessageProc(HTEXT(hTextInfo), WINDOWPOS(hTextInfo).x, WINDOWPOS(hTextInfo).y + yOffset,
				UM_TEXT, 
				(FPARAM)((FPARAM)(WINDOWPOS(hTextInfo).y + yOffset) << 32 | (FPARAM)(WINDOWPOS(hTextInfo).x + WINDOWSIZE(hTextInfo).x + USWIDTH(CHARSIZE(hTextInfo).x))),
				(SPARAM)(&lpchText))) != UR_ERROR)
		{
			int   iCount = (int)HIDWORD(result);
			short iStart = (short)LOWORD(LODWORD(result));
			short iEnd   = (short)HIWORD(LODWORD(result));

			if (iCount > 0)
				MyTextOut(MEMDC(hTextInfo), 0, yOffset, 
					lpchText, iCount, iStart, iEnd, USWIDTH(CHARSIZE(hTextInfo).x));
#ifdef DEBUG
			if (iCount > 0 && INRANGEY(yOffset, lpPaint->rcPaint.top, lpPaint->rcPaint.bottom))
				wprintf(TEXT("At y = %d, The line is: %ls\n"), yOffset, lpchText);
#endif // DEBUG

		}
	}
	return (TRUE);
}