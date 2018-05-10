#pragma once

#include <Windows.h>

#define DEBUG
#ifdef DEBUG

// ++++++++++++++++++++ DEBUG INFO MESSAGEBOX OUTPUT MACRO ++++++++++++++ //
void chDEBUGPRINTBOX(LPCWSTR msg);

void chDEBUGMESSAGEBOX(LPCWSTR lpszFmt, ...);

void chDEBUGTEXTOUT(HWND hWnd, LPCWSTR lpString, int x, int y);
// ++++++++++++++++++++++++++++++++ END +++++++++++++++++++++++++++++++++ //

// ++++++++++++++++++++++++++++ DEBUG INFO CONSOLE OUTPUT MACRO ++++++++++++++++++++++++++++++++ //
#define PRINTCARETPOS(hTextInfo) \
	wprintf_s(TEXT("CaretPos = (%d, %d) "), CARETPOS((hTextInfo)).x, CARETPOS((hTextInfo)).y)

#define PRINTWINDOWSIZE(hTextInfo) \
	wprintf_s(TEXT("WindowSize = (%d, %d) "), WINDOWSIZE((hTextInfo)).x, WINDOWSIZE((hTextInfo)).y)

#define PRINTWINDOWPOS(hTextInfo) \
	wprintf_s(TEXT("WindowPos = (%d, %d) "), WINDOWPOS((hTextInfo)).x, WINDOWPOS((hTextInfo)).y)

#define PRINTPAGESIZE(hTextInfo) \
	wprintf_s(TEXT("PageSize = (%d, %d) "), PAGESIZE((hTextInfo)).x, PAGESIZE((hTextInfo)).y)

#define PRINTENDL \
	wprintf_s(TEXT("\n"));
// ++++++++++++++++++++++++++++++++++++++++++ END ++++++++++++++++++++++++++++++++++++++++++++++ //

#endif // DEBUG