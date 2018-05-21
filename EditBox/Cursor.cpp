#include "stdafx.h"
int Install::Width = 10;
int Install::Height = 10;
Cursor::Cursor(CText * p, int width, int height)
{
	pText = p;
	nWidth = width;
	nHeight = height;
	bChoose = 0;
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
�жϴ�������xǰ���ַ��Ƿ�ΪӢ������
*/
bool Cursor::isEnBefore(int LineNumber, int x)
{
	if (CharactersProperty_before_Cursor(LineNumber, x) == -1)
		return true;
	else
		return false;
}

/*
�жϴ�������x����ַ��Ƿ�ΪӢ������
*/
bool Cursor::isEnAfter(int LineNumber, int x)
{
	if (!isLegalCursor(LineNumber, x))	//�����Ϸ���
		x = CursorLocation(LineNumber, x);
	int Size = Characters_before_Cursor(LineNumber, x);
	if (Size == pText->Line_Size(LineNumber))
		throw std::invalid_argument("���x�����ַ�");
	CLine* p = pText->GetLinePointer(LineNumber);
	Line_iterator iterator(*p, Size + 1);
	short int m = *iterator;
	if (WORD(m >> 8) > 0)
		return false;
	else
		return true;
}
//����Ƿ���ں����м�
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

/*���ع���Ƿ�λ���ı�ĩβ*/
bool Cursor::isTextTail(int x, int y)
{
	int LineNumber = y / nHeight + 1;
	Position p1 = CursorToPosition(x, y);		//���ǰ�ַ�λ��
	CLine* p = pText->GetLinePointer(LineNumber);
	if (p == NULL)
		return true;
	//���λ����β
	if (p->nDataSize == p1.Sequence)
	{
		if (p->nLineNumber == pText->Line_Number())
		{
			//���λ���ı�ĩβ
			return true;
		}
	}
	return false;
}

/*���ع��ǰ���ַ�����*/
int Cursor::Characters_before_Cursor(int LineNumber, int x)
{
	if (!isLegalCursor(LineNumber, x))						//�����Ϸ���
		x = CursorLocation(LineNumber, x);
	CLine* p = pText->GetLinePointer(LineNumber);
	if (p == NULL)											//��ǰΪ������ �ַ�����Ϊ0
		return 0;
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
	return n;
}

/*
���ع��ǰ���ַ�����(�ա��С�Ӣ)
0---��괦������ ���ַ�
1---�����ַ�
-1--Ӣ���ַ�
*/
int Cursor::CharactersProperty_before_Cursor(int LineNumber, int x)
{
	if (x == 0)
		return 0;
	if (!isLegalCursor(LineNumber, x))						//�����Ϸ���
		throw std::invalid_argument("invalid Cursor 'x'");
	int Size = Characters_before_Cursor(LineNumber, x);		//���ǰ�ַ�����
	CLine* p = pText->GetLinePointer(LineNumber);
	if (p == NULL)											//������ ���ַ�
		return 0;
	Line_iterator iterator(*p, Size);
	int m = *iterator;
	if (WORD(m >> 8) > 0)
		return 1;
	else
		return -1;
}

/*
�����ʱ�Թ���ض�λ
ʹ֮����һ���Ϸ��Ĺ��λ��
���λ
*/
int Cursor::CursorLocation(int LineNumber, int x)
{
	int Length = pText->Line_Width(LineNumber, nWidth);
	if (x > Length)
		return Length;
	else
	{
		while (true)
		{
			if (isLegalCursor(LineNumber, x))
				return x;
			else
				x++;
		}
	}

}
/*
���ع��ǰ�ַ���λ��
eg. 3 abcde|fd
return (3,5)
eg. 3 |abcdef
return {3,0}
*/
Position Cursor::CursorToPosition(int x, int y)
{
	int LineNumber = y / nHeight + 1;
	//���Ϸ��Լ��
	if (!isLegalCursor(LineNumber, x))
		x = CursorLocation(LineNumber, x);
	int n = Characters_before_Cursor(LineNumber, x);

	return { LineNumber,n };
}
/*
���ع�����ַ�λ��
eg. 3 abcde|fd
return (3,6)
3 abcdefd|
4 1234
return (4,0)  �����λ����β �򷵻���һ����ʼλ��
*/
Position Cursor::CursorToPosition_After(int x, int y)
{
	int LineNumber = y / nHeight + 1;
	Position p1 = CursorToPosition(x, y);		//���ǰ�ַ�λ��
	CLine* p = pText->GetLinePointer(LineNumber);
	//���λ����β
	if (p->nDataSize == p1.Sequence)
	{
		if (p->nLineNumber == pText->Line_Number())
		{
			//���λ���ı�ĩβ
			throw std::invalid_argument("���λ���ı�ĩβ ����û���ַ�");
		}
		return { LineNumber + 1,min(1,p->pNextLine->nDataSize) };		//��һ������
	}
	else
		return { LineNumber,p1.Sequence + 1 };
}
/*
����positionλ�ú�Ĺ��POINT��Ϣ
*/
POINT Cursor::PositionToCursor(Position position)
{
	POINT point;
	int x, y;
	y = (position.LineNumber - 1)*nHeight;
	if (position.Sequence != 0)
		x = pText->Line_Width(position.LineNumber, nWidth, position.Sequence);
	else
		x = 0;
	point.x = x;
	point.y = y;
	return point;
}
/*
����positionλ��ǰ�Ĺ��POINT��Ϣ
*/
POINT Cursor::PositionToCursor_Before(Position position)
{
	POINT point;
	int x, y;
	y = (position.LineNumber - 1)*nHeight;
	if (position.Sequence <= 1)
		x = 0;
	else
		x = pText->Line_Width(position.LineNumber, nWidth, position.Sequence - 1);
	point.x = x;
	point.y = y;
	return point;
}

void Cursor::Choose(Position s, Position e)
{
	start = s;
	end = e;
	bChoose = 1;
}

void Cursor::SetChoose()
{
	bChoose = 1;
}

void Cursor::ResetChoose()
{
	bChoose = 0;
}

bool Cursor::isChoose()
{
	return bChoose;
}

