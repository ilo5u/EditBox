#pragma once

#include "resource.h"
#include <Windows.h>

BOOL WINAPI MyTextOutW(_In_ HDC hdc, _In_ int x, _In_ int y, _In_reads_(c) LPCWSTR lpString, _In_ int c, _In_ short s, _In_ short e, _In_ int width);

#ifdef UNICODE
#define MyTextOut MyTextOutW
#else
#define MyTextOut MyTextOutA
#endif // UNICODE