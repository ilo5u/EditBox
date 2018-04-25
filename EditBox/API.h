#pragma once

#include"Cursor.h"
#include<Windows.h>
#include"Record.h"
#pragma once


typedef CText* HTEXT;
HTEXT	_stdcall	CreateText(int iCharWidth, int iCharHeight);
BOOL	_stdcall	DestroyText(HTEXT &hText);

typedef ULONGLONG FPARAM;	// _INT64
typedef ULONGLONG SPARAM;	// _INT64
typedef ULONGLONG RVALUE;	// _INT64

#define LODWORD(l) ((UINT)(((ULONGLONG)(l)) & 0xffffffff))			// 低双字节
#define HIDWORD(l) ((UINT)(((ULONGLONG)(l) >> 32) & 0xffffffff))	// 高双字节

#define ZHWIDTH(l) (l << 1)
#define USWIDTH(l) (l)
#define ZHHEIGHT(l) (l)
#define USHEIGHT(l) (l)

#define UM_UP		0x00000001
#define UM_DOWN		0x00000002
#define UM_LEFT		0x00000004
#define UM_RIGHT	0x00000008
#define UM_RETURN	0x00000010
#define UM_END		0x00000020
#define UM_CHAR		0x00000040
#define UM_TEXT		0x00000080
#define UM_DELETE	0x00000100
#define UM_CURSOR	0x00000200
#define UM_CHOOSE	0x00000400
#define UM_HOME		0x00000800
#define UM_NEW		0x00001000
#define UM_OPEN		0x00002000
#define UM_SAVE		0x00004000
#define UM_CLOSE	0x00008000
#define UM_ALL		0x00000011
#define UM_COPY		0x00000012
#define UM_PASTE	0x00000014
#define UM_CANCEL	0x00000018
#define UM_FIND		0x00000021

#define UR_NOPATH		0xf0000000f0100000
#define UR_NOTSAVED		0xf0000000f1000000
#define UR_NOTCANCEL	0xf0000000f1100000
#define UR_ERROR		0xf0000001f0000000
#define UR_SUCCESS		0xf0000010f0000000
#define UR_SAVED		0xf0000100f0000000

#define INRANGE(x, l, r) ((x) >= (l) && (x) <= (r))
#define TEXT_SIZE	300				//显示文本长度
#define TAB_SIZE	4				//Tab字符为4空格
RVALUE _stdcall UserMessageProc(HTEXT hText, int x, int y, UINT message, FPARAM fParam, SPARAM sParam);

Cursor*  Initialize_Cursor(CText* p, int Width, int Height);
void Alloc_Buffer(wchar_t* &p, size_t &Old_Size, size_t New_Size);
void Free_Buffer(wchar_t* &p);
void Set_Height_Light(int LineNumber, Position ps, Position pe, short int& start, short int& end);
