#include "stdafx.h"
#include "editbox.h"

void chDEBUGPRINTBOX(LPCWSTR msg)
{
	MessageBox(NULL, msg, TEXT("DEBUG"), MB_OKCANCEL);
}

void chDEBUGTEXTOUT(HWND hWnd, LPCWSTR lpString, int x, int y)
{
	HDC hdc = GetDC(hWnd);
	TextOut(hdc, x, y, lpString, lstrlen(lpString));
	ReleaseDC(hWnd, hdc);
}

void chDEBUGMESSAGEBOX(LPCWSTR lpszFmt, ...)
{
	TCHAR out[1000] = { 0 };
	va_list arg_ptr;
	va_start(arg_ptr, lpszFmt);
	_vsnwprintf_s(out, _countof(out), lpszFmt, arg_ptr);
	MessageBox(NULL, out, TEXT("DEBUG"), MB_OKCANCEL);
}