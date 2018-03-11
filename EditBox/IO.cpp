#include "stdafx.h"
#include "EditBox.h"

BOOL WINAPI MyTextOutW(_In_ HDC hdc, _In_ int x, _In_ int y, _In_reads_(c) LPCWSTR lpString, _In_ int c)
{
	return TextOut(hdc, x, y, lpString, c);
}