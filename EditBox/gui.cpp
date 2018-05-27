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
#include "debug.h"

BOOL operator==(POINT left, POINT right)
{
	return (left.x == right.x) && (left.y == right.y);
}

/*---------------------------------------------
	@Description:
		ָ������������Ч��

	@Paramter:
		hTextInfo: ��ĳ���ı����
		left: �������
		right: �����Ҷ�
		top: ���ζ���
		bottom: ���ε׶�

	@Return:
		��������Ĭ�Ϸ���TRUE��

	@Author:
		����
---------------------------------------------*/
BOOL MyInvalidateRect(HTEXTINFO hTextInfo, LONG left, LONG right, LONG top, LONG bottom)
{
	RECT rcInvalidate;
	SetRect(&rcInvalidate,
		min(max(0, left), PAINTSIZE(hTextInfo).x),
		min(max(0, top), PAINTSIZE(hTextInfo).y),
		max(min(PAINTSIZE(hTextInfo).x, right), 0),
		max(min(PAINTSIZE(hTextInfo).y, bottom), 0)
	); // ���þ�������
	InvalidateRect(hTextInfo->m_hWnd, &rcInvalidate, FALSE);
	return (TRUE);
}

/*---------------------------------------------
	@Description:
		�û�����Backspace��ʱ��
		����Deleteǰ��������λ�ã�
		��Ҫ���û�и�������ʱ��
		����Backspace����Ҫɾ�����ǰһ���ַ�

	@Paramter:
		hTextInfo: ��ĳ���ı����

	@Return:
		��������Ĭ�Ϸ���TRUE��

	@Author:
		����
---------------------------------------------*/
BOOL AdjustCaretPosBeforeBackspace(HTEXTINFO hTextInfo)
{
	if (STARTPOS(hTextInfo) == ENDPOS(hTextInfo))
	{	// ��ǰδѡ��Ƭ�� ɾ�����ǰһ���ַ�
		POINT pOldEndPos = ENDPOS(hTextInfo);
		SendMessage(hTextInfo->m_hWnd, WM_KEYDOWN, VK_LEFT, NULL);   // �������
		SelectHighlight(hTextInfo, STARTPOS(hTextInfo), pOldEndPos); // �����������(����һ���ַ�)
	}

	return (TRUE);
}

/*---------------------------------------------
	@Description:
		������ͼ����ʹ������ڸ������ڣ�
		��������ʾ���

	@Paramter:
		hTextInfo: ��ĳ���ı����

	@Return:
		��������Ĭ�Ϸ���TRUE��

	@Author:
		����
---------------------------------------------*/
BOOL AdjustPaintPos(HTEXTINFO hTextInfo)
{
	// ����ƫ����
	int xOffset = INRANGEX(CARETPOS(hTextInfo).x,
		PAINTPOS(hTextInfo).x,
		PAINTPOS(hTextInfo).x + PAGESIZE(hTextInfo).x * CHARSIZE(hTextInfo).x
	) ? 0 : CARETPOS(hTextInfo).x - PAINTPOS(hTextInfo).x - (PAGESIZE(hTextInfo).x >> 1) * CHARSIZE(hTextInfo).x;

	int yOffset = INRANGEY(CARETPOS(hTextInfo).y,
		PAINTPOS(hTextInfo).y,
		PAINTPOS(hTextInfo).y + PAGESIZE(hTextInfo).y * CHARSIZE(hTextInfo).y
	) ? 0 : CARETPOS(hTextInfo).y < PAINTPOS(hTextInfo).y
		? CARETPOS(hTextInfo).y - PAINTPOS(hTextInfo).y
		: CARETPOS(hTextInfo).y - PAINTPOS(hTextInfo).y - (PAGESIZE(hTextInfo).y - 1) * CHARSIZE(hTextInfo).y;

	// ���ù�����
	SCROLLINFO shInfo;
	shInfo.cbSize = sizeof(SCROLLINFO);
	shInfo.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;
	GetScrollInfo(hTextInfo->m_hWnd, SB_HORZ, &shInfo);

	int iOldHorzPos = shInfo.nPos;
	shInfo.nPos = shInfo.nPos + xOffset / CHARSIZE(hTextInfo).x;
	if (shInfo.nPos < shInfo.nMin)
		shInfo.nPos = shInfo.nMin;
	if (shInfo.nPos > shInfo.nMax - (int)shInfo.nPage + 1)
		shInfo.nPos = shInfo.nMax - (int)shInfo.nPage + 1;
	int iNowHorzPos = shInfo.nPos;

	shInfo.cbSize = sizeof(SCROLLINFO);
	shInfo.fMask = SIF_POS;
	SetScrollInfo(hTextInfo->m_hWnd, SB_HORZ, &shInfo, TRUE);

	SCROLLINFO svInfo;
	svInfo.cbSize = sizeof(SCROLLINFO);
	svInfo.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;
	GetScrollInfo(hTextInfo->m_hWnd, SB_VERT, &svInfo);

	int iOldVertPos = svInfo.nPos;
	svInfo.nPos = svInfo.nPos + yOffset / CHARSIZE(hTextInfo).y;
	if (svInfo.nPos < svInfo.nMin)
		svInfo.nPos = svInfo.nMin;
	if (svInfo.nPos > svInfo.nMax - (int)svInfo.nPage + 1)
		svInfo.nPos = svInfo.nMax - (int)svInfo.nPage + 1;
	int iNowVertPos = svInfo.nPos;

	svInfo.cbSize = sizeof(SCROLLINFO);
	svInfo.fMask = SIF_POS;
	SetScrollInfo(hTextInfo->m_hWnd, SB_VERT, &svInfo, TRUE);

	SelectPaintPos(hTextInfo,
		POINT{ iNowHorzPos * CHARSIZE(hTextInfo).x, iNowVertPos * CHARSIZE(hTextInfo).y }
	); // ���û�ͼ���������λ��
	SelectCaretPos(hTextInfo, CARETPOS(hTextInfo), CARETCOORD(hTextInfo));

	MyScrollWindow(hTextInfo,
		(iOldHorzPos - iNowHorzPos) * CHARSIZE(hTextInfo).x,
		(iOldVertPos - iNowVertPos) * CHARSIZE(hTextInfo).y
	);	// ��������

	return (TRUE);
}

/*---------------------------------------------
	@Description:
		�ƶ�������

	@Paramter:
		hTextInfo: ��ĳ���ı����
		message: ��Ϣ����
			VK_LEFT
			VK_RIGHT
			VK_UP
			VK_DOWN
			VK_HOME
			VK_END

	@Return:
		��������Ĭ�Ϸ���TRUE��

	@Author:
		����
---------------------------------------------*/
BOOL MoveCaret(HTEXTINFO hTextInfo, UINT message)
{
	KERNELINFO kernelinfo;
	switch (UserMessageProc(HTEXT(hTextInfo),
		CARETPOS(hTextInfo).x, CARETPOS(hTextInfo).y,
		message, NULL, NULL, &kernelinfo))
	{
	case UR_SUCCESS:
	{
		MyInvalidateRect(hTextInfo,
			0, PAINTSIZE(hTextInfo).x,
			STARTPOS(hTextInfo).y - PAINTPOS(hTextInfo).y,
			ENDPOS(hTextInfo).y - PAINTPOS(hTextInfo).y + CHARSIZE(hTextInfo).y
		); // �����ƶ����֮ǰ�ĸ�������Ϊ��Ч����
		SelectHighlight(hTextInfo,
			kernelinfo.m_pCaretPixelPos,
			kernelinfo.m_pCaretPixelPos
		);

		SelectCaretPos(hTextInfo, kernelinfo.m_pCaretPixelPos, kernelinfo.m_cCaretCoord);
		AdjustPaintPos(hTextInfo); // ������ͼ�� ʹ������ڻ�ͼ������

		TCHAR szCharCount[MAX_LOADSTRING];
		wsprintf(szCharCount, TEXT("%d characters"), kernelinfo.m_uiCount);
		SendMessage(STATUS(hTextInfo), SB_SETTEXT, SBT_NOBORDERS | (WORD)0x00, (LPARAM)szCharCount);
	}
	break;
	default:
		break;
	}
	return (TRUE);
}

/*---------------------------------------------
	@Description:
		���û�ͼ���������λ��

	@Paramter:
		hTextInfo: ��ĳ���ı����
		thePos: �����õ�λ��

	@Return:
		��������Ĭ�Ϸ���TRUE��

	@Author:
		����
---------------------------------------------*/
BOOL SelectPaintPos(HTEXTINFO hTextInfo, POINT thePixelPos)
{
	PAINTPOS(hTextInfo) = thePixelPos;
	return (TRUE);
}

/*---------------------------------------------
	@Description:
		���ù�������λ��

	@Paramter:
		hTextInfo: ��ĳ���ı����
		thePixelPos: �����õ�λ��
		theCoord: �����õ����к�

	@Return:
		��������Ĭ�Ϸ���TRUE��

	@Author:
		����
---------------------------------------------*/
BOOL SelectCaretPos(HTEXTINFO hTextInfo, POINT thePixelPos, COORD theCoord)
{
	static BOOL s_bCaretAlreadyHiden = false; // �����жϵ�ǰ����Ƿ��Ѿ������أ�������hide���

	CARETPOS(hTextInfo) = thePixelPos;
	CARETCOORD(hTextInfo) = theCoord;
	if (!INRANGEX(thePixelPos.x,
		PAINTPOS(hTextInfo).x,
		PAINTPOS(hTextInfo).x + PAGESIZE(hTextInfo).x * CHARSIZE(hTextInfo).x)
		|| !INRANGEY(thePixelPos.y,
			PAINTPOS(hTextInfo).y,
			PAINTPOS(hTextInfo).y + PAGESIZE(hTextInfo).y * CHARSIZE(hTextInfo).y)) // ���δ���ڴ�����
	{
		if (!s_bCaretAlreadyHiden)
		{
			HideCaret(hTextInfo->m_hWnd);
			s_bCaretAlreadyHiden = true;
		}
	}
	else
	{
		if (s_bCaretAlreadyHiden)
		{
			ShowCaret(hTextInfo->m_hWnd);
			s_bCaretAlreadyHiden = false;
		}
	}
	SetCaretPos(CARETPOS(hTextInfo).x - PAINTPOS(hTextInfo).x, CARETPOS(hTextInfo).y - PAINTPOS(hTextInfo).y);

	TCHAR szCaretCoord[MAX_PATH];
	wsprintf(szCaretCoord, TEXT("%d rows, %d cols"), theCoord.Y, theCoord.X);
	SendMessage(STATUS(hTextInfo), SB_SETTEXT, SBT_NOBORDERS | (WORD)0x01, (LPARAM)szCaretCoord);

	return (TRUE);
}

/*---------------------------------------------
	@Description:
		���ø�������
		���ش����յ�ǰ��

	@Paramter:
		hTextInfo: ��ĳ���ı����
		theStart: �������ֵ����
		theEnd: �������ֵ��յ�

	@Return:
		��������Ĭ�Ϸ���TRUE��

	@Author:
		����
---------------------------------------------*/
BOOL SelectHighlight(HTEXTINFO hTextInfo, POINT theStart, POINT theEnd)
{
	STARTPOS(hTextInfo) = theStart;
	ENDPOS(hTextInfo) = theEnd;

	return (TRUE);
}

/*---------------------------------------------
	@Description:
		�����ı���Χ�Ĵ�С��
		���ڸ��¹������������Ϣ

	@Paramter:
		hTextInfo: ��ĳ���ı����
		theSize: �ı���Χ�Ĵ�С

	@Return:
		��������Ĭ�Ϸ���TRUE��

	@Author:
		����
---------------------------------------------*/
BOOL SelectTextSize(HTEXTINFO hTextInfo, POINT theSize)
{
	TEXTSIZE(hTextInfo) = theSize;

	// ���ù�����
	SCROLLINFO shInfo;
	shInfo.cbSize = sizeof(SCROLLINFO);
	shInfo.fMask = SIF_ALL;
	GetScrollInfo(hTextInfo->m_hWnd, SB_HORZ, &shInfo);

	shInfo.nPage = PAINTSIZE(hTextInfo).x / CHARSIZE(hTextInfo).x;
	shInfo.nMin = 0;
	shInfo.nMax = theSize.x / CHARSIZE(hTextInfo).x;

	shInfo.cbSize = sizeof(SCROLLINFO);
	shInfo.fMask = SIF_PAGE | SIF_RANGE | SIF_DISABLENOSCROLL;
	SetScrollInfo(hTextInfo->m_hWnd, SB_HORZ, &shInfo, TRUE);

	SCROLLINFO svInfo;
	svInfo.cbSize = sizeof(SCROLLINFO);
	svInfo.fMask = SIF_ALL;
	GetScrollInfo(hTextInfo->m_hWnd, SB_VERT, &svInfo);

	svInfo.nPage = PAINTSIZE(hTextInfo).y / CHARSIZE(hTextInfo).y;
	svInfo.nMin = 0;
	svInfo.nMax = theSize.y / CHARSIZE(hTextInfo).y;

	svInfo.cbSize = sizeof(SCROLLINFO);
	svInfo.fMask = SIF_PAGE | SIF_RANGE | SIF_DISABLENOSCROLL;
	SetScrollInfo(hTextInfo->m_hWnd, SB_VERT, &svInfo, TRUE);

	return (TRUE);
}

/*---------------------------------------------
	@Description:
		���ô��ڵ������Ϣ��
		��������״̬����������

	@Paramter:
		hTextInfo: ��ĳ���ı����
		pTextSize: �ı���Χ�Ĵ�С
		szTitle: ����������

	@Return:
		��������Ĭ�Ϸ���TRUE��

	@Author:
		����
---------------------------------------------*/
BOOL SelectWindow(HTEXTINFO hTextInfo, POINT pTextSize, LPCWSTR szTitle)
{
	SetWindowText(hTextInfo->m_hWnd, szTitle);

	SelectTextSize(hTextInfo, pTextSize);     // �����ı���Χ�����ı���
	SelectPaintPos(hTextInfo, POINT{ 0, 0 }); // ���ô���λ�ã����Ͻ�

	CreateCaret(hTextInfo->m_hWnd, NULL,
		CARETSIZE(hTextInfo).x,
		CARETSIZE(hTextInfo).y
	); // �������
	SelectCaretPos(hTextInfo, POINT{ 0, 0 }, COORD{ 0, 0 }); // ���ù��λ�ã����Ͻǣ�
	ShowCaret(hTextInfo->m_hWnd);

	SelectHighlight(hTextInfo,
		CARETPOS(hTextInfo),
		CARETPOS(hTextInfo)
	);	// ���ø������֣��ޣ�

	MyInvalidateRect(hTextInfo,
		0, PAINTSIZE(hTextInfo).x,
		0, PAINTSIZE(hTextInfo).y
	);

	return (TRUE);
}

/*---------------------------------------------
	@Description:
		������ʾ��

	@Paramter:
		hTextInfo: ��ĳ���ı����
		xOffset: �����������
		yOffset: �����������

	@Return:
		��������Ĭ�Ϸ���TRUE��

	@Author:
		����
---------------------------------------------*/
BOOL MyScrollWindow(HTEXTINFO hTextInfo, int xOffset, int yOffset)
{
	RECT rcScroll;
	SetRect(&rcScroll, xOffset, 0, PAINTSIZE(hTextInfo).x + xOffset, PAINTSIZE(hTextInfo).y);
	if (xOffset < 0)
	{	// ���󻬶�
		rcScroll.left = -xOffset > PAINTSIZE(hTextInfo).x ?
			PAINTSIZE(hTextInfo).x : -xOffset;
		rcScroll.right = PAINTSIZE(hTextInfo).x;
		MyInvalidateRect(hTextInfo,
			max(0, xOffset + PAINTSIZE(hTextInfo).x - PAINTSIZE(hTextInfo).x % CHARSIZE(hTextInfo).x), PAINTSIZE(hTextInfo).x,
			0, PAINTSIZE(hTextInfo).y
		);
	}
	else
	{	// ���һ���
		rcScroll.left = 0;
		rcScroll.right = xOffset > PAINTSIZE(hTextInfo).x ?
			0 : PAINTSIZE(hTextInfo).x - xOffset;
		MyInvalidateRect(hTextInfo,
			0, min(xOffset, PAINTSIZE(hTextInfo).x),
			0, PAINTSIZE(hTextInfo).y
		);
	}

	if (yOffset < 0)
	{	// ���ϻ���
		rcScroll.top = -yOffset > PAINTSIZE(hTextInfo).y ?
			PAINTSIZE(hTextInfo).y : -yOffset;
		rcScroll.bottom = PAINTSIZE(hTextInfo).y;
		MyInvalidateRect(hTextInfo,
			0, PAINTSIZE(hTextInfo).x,
			max(0, yOffset + PAINTSIZE(hTextInfo).y - PAINTSIZE(hTextInfo).y % CHARSIZE(hTextInfo).y), PAINTSIZE(hTextInfo).y
		);
	}
	else
	{	// ���»���
		rcScroll.top = 0;
		rcScroll.bottom = yOffset > PAINTSIZE(hTextInfo).y ?
			PAINTSIZE(hTextInfo).y : PAINTSIZE(hTextInfo).y - yOffset;
		MyInvalidateRect(hTextInfo,
			0, PAINTSIZE(hTextInfo).x,
			0, min(yOffset, PAINTSIZE(hTextInfo).y)
		);
	}
	ScrollWindowEx(hTextInfo->m_hWnd, xOffset, yOffset, &rcScroll, NULL, NULL, NULL, SW_INVALIDATE);

	return (TRUE);
}

/*---------------------------------------------
	@Description:
		�ػ洰�ڣ�
		����Ч����������ػ���������

	@Paramter:
		hTextInfo: ��ĳ���ı����
		lpRepaint: ��Ч��������

	@Return:
		��������Ĭ�Ϸ���TRUE��

	@Author:
		����
---------------------------------------------*/
BOOL PaintWindow(HTEXTINFO hTextInfo, LPRECT lpRepaint)
{
	RECT rcBuffer{ 0, 0, BUFFERSIZE(hTextInfo).x, BUFFERSIZE(hTextInfo).y };
	FillRect(MEMDC(hTextInfo), &rcBuffer, BRUSH(hTextInfo)); // ���ƻ�����λͼ

	KERNELINFO kernelinfo;
	for (LONG yOffset = lpRepaint->top; yOffset < lpRepaint->bottom; yOffset += CHARSIZE(hTextInfo).y)
	{
		switch (UserMessageProc(HTEXT(hTextInfo),
			PAINTPOS(hTextInfo).x + lpRepaint->left, PAINTPOS(hTextInfo).y + yOffset,
			UM_TEXT,
			(FPARAM)PAINTPOS(hTextInfo).x + lpRepaint->right,
			(SPARAM)PAINTPOS(hTextInfo).y + yOffset, &kernelinfo))
		{
		case UR_SUCCESS:
		{
			if (kernelinfo.m_uiCount > 0)
			{
				MyTextOut(MEMDC(hTextInfo),
					kernelinfo.m_bInside ? lpRepaint->left - CHARSIZE(hTextInfo).x : lpRepaint->left,
					yOffset,
					kernelinfo.m_lpchText,
					kernelinfo.m_uiCount, kernelinfo.m_uiStart, kernelinfo.m_uiEnd,
					CHARSIZE(hTextInfo).x
				), wprintf_s(TEXT("bInside = %d, s = %d, e = %d, c = %d, yOffset = %d: %ls\n"),
					kernelinfo.m_bInside,
					kernelinfo.m_uiStart, kernelinfo.m_uiEnd, kernelinfo.m_uiCount, yOffset, kernelinfo.m_lpchText);
				if (kernelinfo.m_lpchText[0] == TEXT('\t'))
					wprintf_s(L"There is a tab.\n");
			}

		}
		break;
		default:
			break;
		}
	}
	return (TRUE);
}

BOOL SelectClientSize(HTEXTINFO hTextInfo, LPRECT lpRect)
{
	return (TRUE);
}

/*---------------------------------------------
	@Description:
		������ʾ�����С

	@Paramter:
		hTextInfo: ��ĳ���ı����
		newCharWidth: ������
		newCharHeight: ����߶�

	@Return:
		��������Ĭ�Ϸ���TRUE��

	@Author:
		����
---------------------------------------------*/
BOOL SelectCharSize(HTEXTINFO hTextInfo, LONG newCharWidth, LONG newCharHeight)
{
	// �ȱ�������
	PAINTPOS(hTextInfo).x = PAINTPOS(hTextInfo).x * newCharWidth / CHARSIZE(hTextInfo).x;
	PAINTPOS(hTextInfo).y = PAINTPOS(hTextInfo).y * newCharHeight / CHARSIZE(hTextInfo).y;

	STARTPOS(hTextInfo).x = STARTPOS(hTextInfo).x * newCharWidth / CHARSIZE(hTextInfo).x;
	STARTPOS(hTextInfo).y = STARTPOS(hTextInfo).y * newCharHeight / CHARSIZE(hTextInfo).y;

	ENDPOS(hTextInfo).x = ENDPOS(hTextInfo).x * newCharWidth / CHARSIZE(hTextInfo).x;
	ENDPOS(hTextInfo).y = ENDPOS(hTextInfo).y * newCharHeight / CHARSIZE(hTextInfo).y;

	PAGESIZE(hTextInfo).x = PAINTSIZE(hTextInfo).x / newCharWidth;
	PAGESIZE(hTextInfo).y = PAINTSIZE(hTextInfo).y / newCharHeight;

	TEXTSIZE(hTextInfo).x = TEXTSIZE(hTextInfo).x * newCharWidth / CHARSIZE(hTextInfo).x;
	TEXTSIZE(hTextInfo).y = TEXTSIZE(hTextInfo).y * newCharHeight / CHARSIZE(hTextInfo).y;

	// ���������
	SCROLLINFO shInfo;
	shInfo.cbSize = sizeof(SCROLLINFO);
	shInfo.fMask = SIF_ALL;
	GetScrollInfo(hTextInfo->m_hWnd, SB_HORZ, &shInfo);

	shInfo.nPage = PAGESIZE(hTextInfo).x;
	shInfo.nMin = 0;
	shInfo.nMax = TEXTSIZE(hTextInfo).x / newCharWidth;
	shInfo.nPos = PAINTPOS(hTextInfo).x / newCharWidth;

	shInfo.cbSize = sizeof(SCROLLINFO);
	shInfo.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
	SetScrollInfo(hTextInfo->m_hWnd, SB_HORZ, &shInfo, TRUE);

	SCROLLINFO svInfo;
	svInfo.cbSize = sizeof(SCROLLINFO);
	svInfo.fMask = SIF_ALL;
	GetScrollInfo(hTextInfo->m_hWnd, SB_VERT, &svInfo);

	svInfo.nPage = PAGESIZE(hTextInfo).y;
	svInfo.nMin = 0;
	svInfo.nMax = TEXTSIZE(hTextInfo).y / newCharHeight;
	svInfo.nPos = PAINTPOS(hTextInfo).y / newCharHeight;

	svInfo.cbSize = sizeof(SCROLLINFO);
	svInfo.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
	SetScrollInfo(hTextInfo->m_hWnd, SB_VERT, &svInfo, TRUE);

	SelectCaretPos(hTextInfo,
		POINT{ CARETPOS(hTextInfo).x * newCharWidth / CHARSIZE(hTextInfo).x,
		CARETPOS(hTextInfo).y * newCharHeight / CHARSIZE(hTextInfo).y },
		CARETCOORD(hTextInfo)
	);

	DeleteObject(FONT(hTextInfo));
	FONT(hTextInfo) = CreateFont(
		newCharHeight,		// �߶�
		newCharWidth,		// ���
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
	SelectObject(MEMDC(hTextInfo), FONT(hTextInfo));

	CHARSIZE(hTextInfo).x = newCharWidth;
	CHARSIZE(hTextInfo).y = newCharHeight;

	UserMessageProc(HTEXT(hTextInfo), CHARSIZE(hTextInfo).x, CHARSIZE(hTextInfo).y,
		UM_CHANGECHARSIZE, NULL, NULL, NULL);

	MyInvalidateRect(hTextInfo,
		0, PAINTSIZE(hTextInfo).x,
		0, PAINTSIZE(hTextInfo).y
	);

	DestroyCaret();
	CARETSIZE(hTextInfo).y = newCharHeight;
	CreateCaret(hTextInfo->m_hWnd, NULL, CARETSIZE(hTextInfo).x, CARETSIZE(hTextInfo).y);
	ShowCaret(hTextInfo->m_hWnd);

	return (TRUE);
}
