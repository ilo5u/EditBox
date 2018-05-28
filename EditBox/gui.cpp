/*--------------------------------------------------------------------------------------------------
// File : *.cpp, *.h
//
// Description:
//             (此处添加注释)
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
		指定矩形区域无效化

	@Paramter:
		hTextInfo: 绑定某个文本句柄
		left: 矩形左端
		right: 矩形右端
		top: 矩形顶端
		bottom: 矩形底端

	@Return:
		处理结果（默认返回TRUE）

	@Author:
		程鑫
---------------------------------------------*/
BOOL MyInvalidateRect(HTEXTINFO hTextInfo, LONG left, LONG right, LONG top, LONG bottom)
{
	RECT rcInvalidate;
	SetRect(&rcInvalidate,
		min(max(0, left), PAINTSIZE(hTextInfo).x),
		min(max(0, top), PAINTSIZE(hTextInfo).y),
		max(min(PAINTSIZE(hTextInfo).x, right), 0),
		max(min(PAINTSIZE(hTextInfo).y, bottom), 0)
	); // 设置矩形区域
	InvalidateRect(hTextInfo->m_hWnd, &rcInvalidate, FALSE);
	return (TRUE);
}

/*---------------------------------------------
	@Description:
		用户按下Backspace键时，
		调整Delete前光标的像素位置；
		主要针对没有高亮部分时，
		按下Backspace键需要删除光标前一个字符

	@Paramter:
		hTextInfo: 绑定某个文本句柄

	@Return:
		处理结果（默认返回TRUE）

	@Author:
		程鑫
---------------------------------------------*/
BOOL AdjustCaretPosBeforeBackspace(HTEXTINFO hTextInfo)
{
	if (STARTPOS(hTextInfo) == ENDPOS(hTextInfo))
	{	// 当前未选中片段 删除光标前一个字符
		POINT pOldEndPos = ENDPOS(hTextInfo);
		SendMessage(hTextInfo->m_hWnd, WM_KEYDOWN, VK_LEFT, NULL);   // 光标左移
		SelectHighlight(hTextInfo, STARTPOS(hTextInfo), pOldEndPos); // 重设高亮部分(光标后一个字符)
	}

	return (TRUE);
}

/*---------------------------------------------
	@Description:
		滑动绘图区域使光标落在该区域内，
		并重新显示光标

	@Paramter:
		hTextInfo: 绑定某个文本句柄

	@Return:
		处理结果（默认返回TRUE）

	@Author:
		程鑫
---------------------------------------------*/
BOOL AdjustPaintPos(HTEXTINFO hTextInfo)
{
	// 计算偏移量
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

	// 设置滚动条
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
	); // 设置绘图区域的像素位置
	SelectCaretPos(hTextInfo, CARETPOS(hTextInfo), CARETCOORD(hTextInfo));

	MyScrollWindow(hTextInfo,
		(iOldHorzPos - iNowHorzPos) * CHARSIZE(hTextInfo).x,
		(iOldVertPos - iNowVertPos) * CHARSIZE(hTextInfo).y
	);	// 滑动窗口

	return (TRUE);
}

/*---------------------------------------------
	@Description:
		移动光标操作

	@Paramter:
		hTextInfo: 绑定某个文本句柄
		message: 消息类型
			VK_LEFT
			VK_RIGHT
			VK_UP
			VK_DOWN
			VK_HOME
			VK_END

	@Return:
		处理结果（默认返回TRUE）

	@Author:
		程鑫
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
		); // 设置移动光标之前的高亮部分为无效区域
		SelectHighlight(hTextInfo,
			kernelinfo.m_pCaretPixelPos,
			kernelinfo.m_pCaretPixelPos
		);

		SelectCaretPos(hTextInfo, kernelinfo.m_pCaretPixelPos, kernelinfo.m_cCaretCoord);
		AdjustPaintPos(hTextInfo); // 滑动绘图区 使光标落在绘图区域内

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
		设置绘图区域的像素位置

	@Paramter:
		hTextInfo: 绑定某个文本句柄
		thePos: 待设置的位置

	@Return:
		处理结果（默认返回TRUE）

	@Author:
		程鑫
---------------------------------------------*/
BOOL SelectPaintPos(HTEXTINFO hTextInfo, POINT thePixelPos)
{
	PAINTPOS(hTextInfo) = thePixelPos;
	return (TRUE);
}

/*---------------------------------------------
	@Description:
		设置光标的像素位置

	@Paramter:
		hTextInfo: 绑定某个文本句柄
		thePixelPos: 待设置的位置
		theCoord: 待设置的行列号

	@Return:
		处理结果（默认返回TRUE）

	@Author:
		程鑫
---------------------------------------------*/
BOOL SelectCaretPos(HTEXTINFO hTextInfo, POINT thePixelPos, COORD theCoord)
{
	static BOOL s_bCaretAlreadyHiden = false; // 用于判断当前光标是否已经被隐藏，避免多次hide光标

	CARETPOS(hTextInfo) = thePixelPos;
	CARETCOORD(hTextInfo) = theCoord;
	if (!INRANGEX(thePixelPos.x,
		PAINTPOS(hTextInfo).x,
		PAINTPOS(hTextInfo).x + PAGESIZE(hTextInfo).x * CHARSIZE(hTextInfo).x)
		|| !INRANGEY(thePixelPos.y,
			PAINTPOS(hTextInfo).y,
			PAINTPOS(hTextInfo).y + PAGESIZE(hTextInfo).y * CHARSIZE(hTextInfo).y)) // 光标未落在窗口内
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
		设置高亮区域，
		起点必处在终点前面

	@Paramter:
		hTextInfo: 绑定某个文本句柄
		theStart: 高亮部分的起点
		theEnd: 高亮部分的终点

	@Return:
		处理结果（默认返回TRUE）

	@Author:
		程鑫
---------------------------------------------*/
BOOL SelectHighlight(HTEXTINFO hTextInfo, POINT theStart, POINT theEnd)
{
	STARTPOS(hTextInfo) = theStart;
	ENDPOS(hTextInfo) = theEnd;

	return (TRUE);
}

/*---------------------------------------------
	@Description:
		设置文本范围的大小，
		用于更新滚动条的相关信息

	@Paramter:
		hTextInfo: 绑定某个文本句柄
		theSize: 文本范围的大小

	@Return:
		处理结果（默认返回TRUE）

	@Author:
		程鑫
---------------------------------------------*/
BOOL SelectTextSize(HTEXTINFO hTextInfo, POINT theSize)
{
	TEXTSIZE(hTextInfo) = theSize;

	// 设置滚动条
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
		设置窗口的相关信息，
		标题栏、状态栏、滚动条

	@Paramter:
		hTextInfo: 绑定某个文本句柄
		pTextSize: 文本范围的大小
		szTitle: 标题栏名称

	@Return:
		处理结果（默认返回TRUE）

	@Author:
		程鑫
---------------------------------------------*/
BOOL SelectWindow(HTEXTINFO hTextInfo, POINT pTextSize, LPCWSTR szTitle)
{
	SetWindowText(hTextInfo->m_hWnd, szTitle);

	SelectTextSize(hTextInfo, pTextSize);     // 设置文本范围（空文本）
	SelectPaintPos(hTextInfo, POINT{ 0, 0 }); // 设置窗口位置（左上角

	CreateCaret(hTextInfo->m_hWnd, NULL,
		CARETSIZE(hTextInfo).x,
		CARETSIZE(hTextInfo).y
	); // 创建光标
	SelectCaretPos(hTextInfo, POINT{ 0, 0 }, COORD{ 0, 0 }); // 设置光标位置（左上角）
	ShowCaret(hTextInfo->m_hWnd);

	SelectHighlight(hTextInfo,
		CARETPOS(hTextInfo),
		CARETPOS(hTextInfo)
	);	// 设置高亮部分（无）

	MyInvalidateRect(hTextInfo,
		0, PAINTSIZE(hTextInfo).x,
		0, PAINTSIZE(hTextInfo).y
	);

	return (TRUE);
}

/*---------------------------------------------
	@Description:
		滚动显示区

	@Paramter:
		hTextInfo: 绑定某个文本句柄
		xOffset: 横向滚动距离
		yOffset: 纵向滚动距离

	@Return:
		处理结果（默认返回TRUE）

	@Author:
		程鑫
---------------------------------------------*/
BOOL MyScrollWindow(HTEXTINFO hTextInfo, int xOffset, int yOffset)
{
	RECT rcScroll;
	SetRect(&rcScroll, xOffset, 0, PAINTSIZE(hTextInfo).x + xOffset, PAINTSIZE(hTextInfo).y);
	if (xOffset < 0)
	{	// 向左滑动
		rcScroll.left = -xOffset > PAINTSIZE(hTextInfo).x ?
			PAINTSIZE(hTextInfo).x : -xOffset;
		rcScroll.right = PAINTSIZE(hTextInfo).x;
		MyInvalidateRect(hTextInfo,
			max(0, xOffset + PAINTSIZE(hTextInfo).x - PAINTSIZE(hTextInfo).x % CHARSIZE(hTextInfo).x), PAINTSIZE(hTextInfo).x,
			0, PAINTSIZE(hTextInfo).y
		);
	}
	else
	{	// 向右滑动
		rcScroll.left = 0;
		rcScroll.right = xOffset > PAINTSIZE(hTextInfo).x ?
			0 : PAINTSIZE(hTextInfo).x - xOffset;
		MyInvalidateRect(hTextInfo,
			0, min(xOffset, PAINTSIZE(hTextInfo).x),
			0, PAINTSIZE(hTextInfo).y
		);
	}

	if (yOffset < 0)
	{	// 向上滑动
		rcScroll.top = -yOffset > PAINTSIZE(hTextInfo).y ?
			PAINTSIZE(hTextInfo).y : -yOffset;
		rcScroll.bottom = PAINTSIZE(hTextInfo).y;
		MyInvalidateRect(hTextInfo,
			0, PAINTSIZE(hTextInfo).x,
			max(0, yOffset + PAINTSIZE(hTextInfo).y - PAINTSIZE(hTextInfo).y % CHARSIZE(hTextInfo).y), PAINTSIZE(hTextInfo).y
		);
	}
	else
	{	// 向下滑动
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
		重绘窗口，
		将无效区域的内容重绘至缓冲区

	@Paramter:
		hTextInfo: 绑定某个文本句柄
		lpRepaint: 无效矩形区域

	@Return:
		处理结果（默认返回TRUE）

	@Author:
		程鑫
---------------------------------------------*/
BOOL PaintWindow(HTEXTINFO hTextInfo, LPRECT lpRepaint)
{
	RECT rcBuffer{ 0, 0, BUFFERSIZE(hTextInfo).x, BUFFERSIZE(hTextInfo).y };
	FillRect(MEMDC(hTextInfo), &rcBuffer, BRUSH(hTextInfo)); // 绘制缓冲区位图

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
		设置显示字体大小

	@Paramter:
		hTextInfo: 绑定某个文本句柄
		newCharWidth: 字体宽度
		newCharHeight: 字体高度

	@Return:
		处理结果（默认返回TRUE）

	@Author:
		程鑫
---------------------------------------------*/
BOOL SelectCharSize(HTEXTINFO hTextInfo, LONG newCharWidth, LONG newCharHeight)
{
	// 等比例放缩
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

	// 重设滚动条
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
		newCharHeight,		// 高度
		newCharWidth,		// 宽度
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
		TEXT("宋体")				// 名称
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
