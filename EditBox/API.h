#pragma once

#include"Cursor.h"
#include<Windows.h>
#include"Record.h"

typedef CText* HTEXT;
HTEXT	_stdcall	CreateText(int iCharWidth, int iCharHeight);
BOOL	_stdcall	DestroyText(HTEXT &hText);

typedef ULONGLONG FPARAM;	// _INT64
typedef ULONGLONG SPARAM;	// _INT64
typedef ULONGLONG RVALUE;	// _INT64

#define LODWORD(l) ((UINT)(((ULONGLONG)(l)) & 0xffffffff))			// ��˫�ֽ�
#define HIDWORD(l) ((UINT)(((ULONGLONG)(l) >> 32) & 0xffffffff))	// ��˫�ֽ�

#define UM_UP		0x00000026
#define UM_DOWN		0x00000028
#define UM_LEFT		0x00000025
#define UM_RIGHT	0x00000027
#define UM_RETURN	0x00000010
#define UM_END		0x00000023
#define UM_CHAR		0x00000040
#define UM_TEXT		0x00000080
#define UM_DELETE	0x00000100
#define UM_CURSOR	0x00000200
#define UM_CHOOSE	0x00000400
#define UM_HOME		0x00000024
#define UM_NEW		0x00001000
#define UM_OPEN		0x00002000
#define UM_SAVE		0x00004000
#define UM_CLOSE	0x00008000
#define UM_ALL		0x00000011
#define UM_COPY		0x00000012
#define UM_PASTE	0x00000014
#define UM_CANCEL	0x00000018
#define UM_FIND		0x00000021
#define UM_REPLACE  0x00000022

#define UR_NOPATH		0xf0000000f0100000
#define UR_NOTSAVED		0xf0000000f1000000
#define UR_NOTCANCEL	0xf0000000f1100000
#define UR_ERROR		0xffffffffffffffff
#define UR_SUCCESS		0xf0000010f0000000
#define UR_SAVED		0xf0000100f0000000

#define FIND_COMPELTE 0x0000000000000001
#define FIND_LHCASE   0x0000000000000002

#define TEXT_SIZE	300				//��ʾ�ı�����
#define TAB_SIZE	4				//Tab�ַ�Ϊ4�ո�
RVALUE _stdcall UserMessageProc(HTEXT hText, int x, int y, UINT message, FPARAM fParam, SPARAM sParam);

Cursor*  Initialize_Cursor(CText* p, int Width, int Height);
void Alloc_Buffer(wchar_t* &p, size_t &Old_Size, size_t New_Size);
void Free_Buffer(wchar_t* &p);
void Set_Height_Light(int LineNumber, Position ps, Position pe, short int& start, short int& end);
