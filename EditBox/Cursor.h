#pragma once
#include"CText.h"
#include <Windows.h>

//鼠标操作  完成文本与鼠标的交互、操作
class Cursor
{
public:
	Cursor(CText* p, int width, int height);
	void SethText(CText* p);											//绑定文本句柄
	void SetWidth(int width);											//设置宽度
	void SetHeight(int height);											//设置高度
	bool isEnBefore(int LineNumber, int x);								//光标前是否为英文
	bool isEnAfter(int LineNumber, int x);								//光标后是否为英文
	bool isLegalCursor(int LineNumber, int x);							//判断x处的光标在显示器上是否合法
	bool isTextTail(int x, int y);										//返回光标是否在文本末尾
	int Characters_before_Cursor(int LineNumber, int x);				//返回光标前的字符数量
	int CharactersProperty_before_Cursor(int LineNumber, int x);		//返回光标前字符属性
	int CursorLocation(int LineNumber, int x);							//重定位光标x(使之合法)
	Position CursorToPosition(int x, int y);							//（合法）光标位置前的字符位置转化到文本位置
	Position CursorToPosition_After(int x, int y);						//（合法）光标位置后的字符位置转化到文本位置
	POINT    PositionToCursor(Position position);						//返回position后的光标位置	
	POINT    PositionToCursor_Before(Position position);				//返回position前的光标位置	
	void Choose(Position s, Position e);								//设置选段信息
	void SetChoose();													//设置选中
	void ResetChoose();													//清空选中
	bool isChoose();													//是否选中
																		//光标选中信息
	Position start;
	Position end;
private:
	CText * pText;					//绑定文本指针
	int		nWidth;					//绑定单位行宽
	int		nHeight;				//绑定字符高度
	bool	bChoose;				//是否存在选中字段
};

//配置信息类
struct Install
{
	static int Width;						//字符宽度
	static int Height;						//行高
};



Cursor* Initialize_Cursor(CText* p, int Width, int Height);
void    Alloc_Buffer(wchar_t* &p, size_t &Old_Size, size_t New_Size);
void    Free_Buffer(wchar_t* &p);
void    Set_Height_Light(int LineNumber, Position ps, Position pe, short int& start, short int& end);
