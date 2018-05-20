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

#define PRINTPAINTSIZE(hTextInfo) \
	wprintf_s(TEXT("Paint's size: (%d, %d) "), PAINTSIZE((hTextInfo)).x, PAINTSIZE((hTextInfo)).y)

#define PRINTPAINTPOS(hTextInfo) \
	wprintf_s(TEXT("Paint's position: (%d, %d) "), PAINTPOS((hTextInfo)).x, PAINTPOS((hTextInfo)).y)

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