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
	wprintf_s(TEXT("Caret's position: (%d, %d) "), CARETPOS((hTextInfo)).x, CARETPOS((hTextInfo)).y)

#define PRINTWINDOWSIZE(hTextInfo) \
	wprintf_s(TEXT("Window's size: (%d, %d) "), WINDOWSIZE((hTextInfo)).x, WINDOWSIZE((hTextInfo)).y)

#define PRINTWINDOWPOS(hTextInfo) \
	wprintf_s(TEXT("Window's position: (%d, %d) "), WINDOWPOS((hTextInfo)).x, WINDOWPOS((hTextInfo)).y)

#define PRINTPAGESIZE(hTextInfo) \
	wprintf_s(TEXT("Page's size: (%d, %d) "), PAGESIZE((hTextInfo)).x, PAGESIZE((hTextInfo)).y)

#define PRINTTEXTSIZE(hTextInfo) \
	wprintf_s(TEXT("Text's size: (%d, %d) "), TEXTSIZE((hTextInfo)).x, TEXTSIZE((hTextInfo)).y)

#define PRINTSTARTPOS(hTextInfo) \
	wprintf_s(TEXT("Start's position: (%d, %d) "), STARTPOS((hTextInfo)).x, STARTPOS((hTextInfo)).y)

#define PRINTENDPOS(hTextInfo) \
	wprintf_s(TEXT("End's position: (%d, %d) "), ENDPOS((hTextInfo)).x, ENDPOS((hTextInfo)).y)

#define PRINTENDL \
	wprintf_s(TEXT("\n"));
// ++++++++++++++++++++++++++++++++++++++++++ END ++++++++++++++++++++++++++++++++++++++++++++++ //

#endif // DEBUG