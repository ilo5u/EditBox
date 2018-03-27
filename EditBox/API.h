#pragma once
#include"CText.h"
#include"Cursor.h"
#include<Windows.h>

Cursor*  Initialize_Cursor(CText* p,int Width,int Height);
void Free_Cursor(Cursor*& p);
CText* pText = NULL;
Cursor* pCursor = NULL;