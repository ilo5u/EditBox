#include "stdafx.h"
int Install::Width = 10;
int Install::Height = 10;
Cursor::Cursor(CText * p, int width, int height)
{
	pText = p;
	nWidth = width;
	nHeight = height;
}

void Cursor::SethText(CText * p)
{
	pText = p;
}

void Cursor::SetWidth(int width)
{
	nWidth = width;
}

void Cursor::SetHeight(int height)
{
	nHeight = height;
}
/*
判断窗口坐标x前的字符是否为英文类型
*/
bool Cursor::isEnBefore(int LineNumber, int x)
{
	if (CharactersProperty_before_Cursor(LineNumber, x) == -1)
		return true;
	else
		return false;
}

/*
判断窗口坐标x后的字符是否为英文类型
*/
bool Cursor::isEnAfter(int LineNumber, int x)
{
	if (!isLegalCursor(LineNumber, x))	//检查光标合法性
		throw std::invalid_argument("invalid Cursor 'x'");
	int Size = Characters_before_Cursor(LineNumber, x);
	if (Size == pText->Line_Size(LineNumber))
		throw std::invalid_argument("光标x后无字符");
	CLine* p = pText->GetLinePointer(LineNumber);
	Line_iterator iterator(*p, Size + 1);
	short int m = *iterator;
	if (WORD(m >> 8) > 0)
		return false;
	else
		return true;
}
//光标是否夹在汉字中间
bool Cursor::isLegalCursor(int LineNumber, int x)
{
	int Sum = 0;
	CLine* p = pText->GetLinePointer(LineNumber);
	if (x > p->Line_Width(nWidth))
		return false;
	Line_iterator iterator(*p);
	while (Sum < x)
	{
		short int m = *iterator;
		++iterator;
		if (WORD(m >> 8) > 0)
			Sum += nWidth;
		Sum += nWidth;
	}
	if (Sum == x)
		return true;
	else
		return false;
}

/*返回光标是否位于文本末尾*/
bool Cursor::isTextTail(int x, int y)
{
	int LineNumber = y / nHeight + 1;
	Position p1 = CursorToPosition(x, y);		//光标前字符位置
	CLine* p = pText->GetLinePointer(LineNumber);
	if (p == NULL)
		return true;
	//光标位于行尾
	if (p->nDataSize == p1.Sequence)
	{
		if (p->nLineNumber == pText->Line_Number())
		{
			//光标位于文本末尾
			return true;
		}
	}
	return false;
}

/*返回光标前的字符数量*/
int Cursor::Characters_before_Cursor(int LineNumber, int x)
{
	if (!isLegalCursor(LineNumber, x))						//检查光标合法性
		throw std::invalid_argument("invalid Cursor 'x'");
	CLine* p = pText->GetLinePointer(LineNumber);
	int Width = 0;
	int n = 0;
	Line_iterator iterator(*p);
	while (Width < x)
	{
		short int m = *iterator;
		if (WORD(m >> 8) > 0)
			Width += nWidth * 2;
		else
			Width += nWidth;
		++iterator;
		++n;
	}
	if (Width != x)
		throw std::invalid_argument("error coordinate 'x'");
	return n;
}

/*
返回光标前的字符属性(空、中、英)
0---光标处于行首 无字符
1---中文字符
-1--英文字符
*/
int Cursor::CharactersProperty_before_Cursor(int LineNumber, int x)
{
	if (x == 0)
		return 0;
	if (!isLegalCursor(LineNumber, x))		//检查光标合法性
		throw std::invalid_argument("invalid Cursor 'x'");
	int Size = Characters_before_Cursor(LineNumber, x);		//光标前字符数量
	CLine* p = pText->GetLinePointer(LineNumber);
	Line_iterator iterator(*p, Size);
	int m = *iterator;
	if (WORD(m >> 8) > 0)
		return 1;
	else
		return -1;
}

/*
鼠标点击时对光标重定位
使之返回一个合法的光标位置
*/
int Cursor::CursorLocation(int LineNumber, int x)
{
	CLine* pLine = pText->GetLinePointer(LineNumber);
	int Length =pText-> Line_Width(LineNumber, nWidth);
	if (x > Length)
		return Length;
	else
	{
		while (true)
		{
			if (isLegalCursor(LineNumber, x))
				return x;
			else
				x--;
		}
	}
	
}
/*
返回光标前字符的位置
eg. 3 abcde|fd
return (3,5)
*/
Position Cursor::CursorToPosition(int x, int y)
{
	int LineNumber = y / nHeight + 1;
	//光标合法性检测
	if (!isLegalCursor(LineNumber, x))
		throw std::invalid_argument("invalid (x,y)");
	int n = Characters_before_Cursor(LineNumber, x);
	/*
	if (n == 0)		//光标位于行首
	{		
		if (LineNumber == 1)
			throw std::invalid_argument("光标位于文本头，之前无字符");
		//对位于行首的情况，认为光标前字符位置为上一行行末尾字符位置
		LineNumber--;
		CLine* pLine = pText->GetLinePointer(LineNumber);
		n = pLine->nDataSize;
	}*/
	return { LineNumber,n };
}
/*
返回光标后的字符位置
eg. 3 abcde|fd
return (3,6)
	3 abcdefd|
	4 1234
return (4,0)  若光标位于行尾 则返回下一行起始位置
*/
Position Cursor::CursorToPosition_After(int x, int y)
{
	int LineNumber = y / nHeight + 1;
	Position p1 = CursorToPosition(x, y);		//光标前字符位置
	CLine* p = pText->GetLinePointer(LineNumber);
	//光标位于行尾
	if (p->nDataSize == p1.Sequence)
	{
		if (p->nLineNumber == pText->Line_Number())
		{
			//光标位于文本末尾
			throw std::invalid_argument("光标位于文本末尾 后面没有字符");
		}
		//return {LineNumber+1,0}
		return { LineNumber + 1,1 };
	}
	else
		return { LineNumber,p1.Sequence + 1 };
}
/*
返回position位置后的光标POINT信息
*/
POINT Cursor::PositionToCursor(Position position)
{
	POINT point;
	int x, y;
	y = (position.LineNumber - 1)*nHeight;
	CLine* pLine = pText->GetLinePointer(position.LineNumber);
	Line_iterator iterator(*pLine);
	x = 0;
	for (int i = 1; i <= position.Sequence; i++)
	{
		short int m = *iterator;
		x += nWidth;
		if (WORD(m >> 8) > 0)
			x += nWidth;
		++iterator;
	}
	point.x = x;
	point.y = y;
	return point;
}
/*复制选段内容并在合适位置添加换行符*/
std::wstring Cursor::Copy(Position start, Position end)
{
	std::wstring wstr;
	Text_iterator first(*pText, start.LineNumber, start.Sequence);
	Text_iterator last(*pText, end.LineNumber, end.Sequence);
	Text_iterator last_next = last + 1;
	int currentLineNumber = start.LineNumber;
	while (first != last_next)
	{
		if (currentLineNumber != first.CurrentLineNumber())		//需要换行
		{
			wstr.push_back(L'\n');
			currentLineNumber++;
		}
		TCHAR wch = *first;
		if (wch == L'\n')
			currentLineNumber++;
		wstr.push_back(wch);
		++first;
	}
	return wstr;
}

void Install::SetWidth(int width)
{
	Install::Width = width;
}

void Install::SetHeight(int height)
{
	Install::Height = height;
}
