#pragma once
#include"CText.h"
#include <Windows.h>
//鼠标操作  完成文本与鼠标的交互、操作
class Cursor
{
public:
	Cursor(CText* p,int width,int height);
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
	std::wstring Copy(Position start, Position end);					//拷贝选段
private:
	CText * pText;					//绑定文本指针
	int		nWidth;					//绑定单位行宽
	int		nHeight;				//绑定字符高度
};

//配置信息类
struct Install
{			
	void SetWidth(int width);
	void SetHeight(int height);
	static int Width;						//字符宽度
	static int Height;						//行高
};
