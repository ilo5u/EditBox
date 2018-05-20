#include "stdafx.h"
#include "miniword.h"
#include <mutex>

BOOL WINAPI MyTextOutW(_In_ HDC hdc, _In_ int x, _In_ int y, _In_reads_(c) LPCWSTR lpString, _In_ int c, _In_ short s, _In_ short e, _In_ int width)
{
	int iCount = 0;
	int iLeft = x;
	int iRight = x;

	//°×µ×ºÚ×Ö
	SetTextColor(hdc, RGB(0x00, 0x00, 0x00));
	SetBkColor(hdc, RGB(0xFF, 0xFF, 0xFF));
	// ...

	while (iCount < s)
	{
		if (lpString[iCount] & TCHAR(0xFF00))
		{	// ºº×Ö
			iRight = iRight + (width << 1);
		}
		else
		{
			iRight = iRight + width;
		}
		++iCount;
	}
	TextOut(hdc, iLeft, y, lpString, iCount);

	// À¶µ×°××Ö
	SetTextColor(hdc, RGB(0xFF, 0xFF, 0xFF));
	SetBkColor(hdc, RGB(0x1C, 0x86, 0xEE));
	// ...

	iLeft = iRight;
	while (iCount < e)
	{
		if (lpString[iCount] & TCHAR(0xFF00))
		{	// ºº×Ö
			iRight = iRight + (width << 1);
		}
		else
		{
			iRight = iRight + width;
		}
		++iCount;
	}
	TextOut(hdc, iLeft, y, lpString + s, iCount - s);

	//°×µ×ºÚ×Ö
	SetTextColor(hdc, RGB(0x00, 0x00, 0x00));
	SetBkColor(hdc, RGB(0xFF, 0xFF, 0xFF));
	// ...

	iLeft = iRight;
	while (iCount < c)
	{
		if (lpString[iCount] & TCHAR(0xFF00))
		{	// ºº×Ö
			iRight = iRight + (width << 1);
		}
		else
		{
			iRight = iRight + width;
		}
		++iCount;
	}
	TextOut(hdc, iLeft, y, lpString + e, iCount - e);

	// assert(!(iCount != c));

	return (TRUE);
}