#pragma once
#include"CText.h"
#include"Cursor.h"
#include<Windows.h>
#define UM_UP		0x00000001		//	UM_UP		光标上移
#define UM_DOWN		0x00000002		//	UM_DOWN		光标下移
#define UM_LEFT		0x00000004		//	UM_LEFT		光标左移
#define UM_RIGHT	0x00000008		//	UM_RIGHT	光标右移
#define UM_RETURN	0x00000010		//	UM_RETURN	回车换行
#define UM_END		0x00000020		//	UM_END		END行尾
#define UM_CHAR		0x00000040		//	UM_CHAR		插入字符
#define UM_TEXT		0x00000080		//	UM_TEXT		显示文本
#define UM_DELETE	0x00000100		//	UM_DELETE	删除字符
#define UM_CURSOR	0x00000012		//	鼠标位置
#define UM_HOME	    0X00000014

#define UR_ZH		0x00000200		
#define UR_EN		0x00000400
#define UR_INVALID	0xFFFFFFFF
#define UR_VALID	0x00000000
#define UR_LINEHEAD 0x00000003
#define UR_SUCCESS	0x00000005
#define UR_ERROR	0x00000009
#define UR_LINETALL	0x00000007
//返回值
//	UP/DOWN/END:
//					移动后光标的绝对横坐标
//
//	case LEFT/RIGHT:
//					若当前光标位置有字符 返回字符属性 UR_ZH 中文 UR_EN 英文
//					若当前光标位置无字符 返回UR_INVALID
//
//	case RETURN/CHAR:
//					异常返回UR_INVALID 否则 UR_VALID
//
//	case DELETE:
//					若光标位置有字符 返回UR_VALID 
//					否则返回删除行尾换行符后把下一行提上来而增加的画面宽度(单位为像素)
//
//	case TEXT:
//					一个指向待输出行文本的指针
LRESULT WINAPI UserMessageProc(int x, int y, UINT message, WPARAM wParam, LPARAM lParam);
Cursor*  Initialize_Cursor(CText* p,int Width,int Height);
void Free_Cursor(Cursor*& p);
CText* pText = NULL;
Cursor* pCursor = NULL;