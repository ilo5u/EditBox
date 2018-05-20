#pragma once
#include"CText.h"
#include <Windows.h>

//������  ����ı������Ľ���������
class Cursor
{
public:
	Cursor(CText* p, int width, int height);
	void SethText(CText* p);											//���ı����
	void SetWidth(int width);											//���ÿ��
	void SetHeight(int height);											//���ø߶�
	bool isEnBefore(int LineNumber, int x);								//���ǰ�Ƿ�ΪӢ��
	bool isEnAfter(int LineNumber, int x);								//�����Ƿ�ΪӢ��
	bool isLegalCursor(int LineNumber, int x);							//�ж�x���Ĺ������ʾ�����Ƿ�Ϸ�
	bool isTextTail(int x, int y);										//���ع���Ƿ����ı�ĩβ
	int Characters_before_Cursor(int LineNumber, int x);				//���ع��ǰ���ַ�����
	int CharactersProperty_before_Cursor(int LineNumber, int x);		//���ع��ǰ�ַ�����
	int CursorLocation(int LineNumber, int x);							//�ض�λ���x(ʹ֮�Ϸ�)
	Position CursorToPosition(int x, int y);							//���Ϸ������λ��ǰ���ַ�λ��ת�����ı�λ��
	Position CursorToPosition_After(int x, int y);						//���Ϸ������λ�ú���ַ�λ��ת�����ı�λ��
	POINT    PositionToCursor(Position position);						//����position��Ĺ��λ��	
	POINT    PositionToCursor_Before(Position position);				//����positionǰ�Ĺ��λ��	
	void Choose(Position s, Position e);								//����ѡ����Ϣ
	void SetChoose();													//����ѡ��
	void ResetChoose();													//���ѡ��
	bool isChoose();													//�Ƿ�ѡ��
																		//���ѡ����Ϣ
	Position start;
	Position end;
private:
	CText * pText;					//���ı�ָ��
	int		nWidth;					//�󶨵�λ�п�
	int		nHeight;				//���ַ��߶�
	bool	bChoose;				//�Ƿ����ѡ���ֶ�
};

//������Ϣ��
struct Install
{
	static int Width;						//�ַ����
	static int Height;						//�и�
};



Cursor* Initialize_Cursor(CText* p, int Width, int Height);
void    Alloc_Buffer(wchar_t* &p, size_t &Old_Size, size_t New_Size);
void    Free_Buffer(wchar_t* &p);
void    Set_Height_Light(int LineNumber, Position ps, Position pe, short int& start, short int& end);
