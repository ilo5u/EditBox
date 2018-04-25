#pragma once
#ifndef UNICODE
#define UNICODE
#endif // !UNICODE
#include<tchar.h>
#include<string>
#include<iostream>
#define BLOCK_SIZE	5
struct Position
{
	int LineNumber;		//�����к�
	int Sequence;		//�������
						//operators
	bool operator<(Position& position);
	bool operator<=(Position& position);
	bool operator>(Position& position);
	bool operator>=(Position& position);
	bool operator==(Position& position);
	bool operator!=(Position& position);
};
//ÿ�η�������ݿ�
class Line_iterator;			//�е�����
struct DataBlock
{
	DataBlock() :pNextBlock(NULL) { }
	DataBlock* pNextBlock;
	TCHAR	   Strings[BLOCK_SIZE];
};
class CLine
{
	friend class Line_iterator;
	friend class CText;
	friend class Cursor;
	friend class Text_iterator;
	friend struct Record;
public:
	CLine(int LNum, CLine* pNext = NULL);
	CLine(CLine& L);
	~CLine();
	void CreateLine(std::wstring& String);							//������
	void ClearLine();												//�����
	void ShowLineData();											//��ʾ��ǰ���ַ���
	Line_iterator DeleteLine(int first, int last);					//ɾ����ǰ��[first,end]�ڵ��ַ���		�ַ���1��ʼ����ɾ����ĵ�һλ
	bool BackSpace(Position position);								//�˸��  ɾ��Position λ���ϵ��ַ�  
	Line_iterator InsertStrings(int start, std::wstring String);		//��start "����" �����ַ���
	Line_iterator begin();											//����ָ�������ַ��ĵ�����
	Line_iterator end();											//����ָ����β�ַ��ĵ�����
	std::wstring  TransformToWString(int first, int last);			//��wstringf����ʽ����[first,last]�е��ַ���
	size_t size()const;												//���ر����ַ�������
	CLine& operator=(CLine& Line);									//��ֵ
	void SetLineNumber(int Number);									//����/�����к�
	bool isBlankLine()const;										//���ص�ǰ���Ƿ�Ϊ��
	int  Line_Width(int Width, int end = 0);						//���ص�ǰ���п�
private:
	CLine * pNextLine;											//��һ��
	DataBlock*	pLineHead;											//����
	int			nLineNumber;										//��ǰ�к�
	int			nDataSize;											//��ǰ���ַ�����
	int			nBlocks;											//�ѿ����
	bool		bBlankLine;											//�Ƿ�Ϊ����
	void	DeleteSpareBlocks(DataBlock* p);
	void	UpDataLineNumber();										//�����к�
};

class Line_iterator
{
	friend class CLine;
	friend class Text_iterator;
public:
	Line_iterator() = default;
	Line_iterator(CLine& theLine, int index = 1);
	Line_iterator(const Line_iterator& m);
	~Line_iterator();
	void Set(CLine& theLine, int index = 1);					//�趨����
	int CurrentPosition()const;									//���ص�ǰ������ָ��λ��
	CLine* GetLinePointer();									//���ص�ǰ����ָ��
	bool isValid()const;										//�Ƿ�Ϊ��Ч(�ǿ�)����
																//operators
	Line_iterator& operator++();
	Line_iterator operator++(int);
	Line_iterator& operator--();
	Line_iterator operator--(int);
	Line_iterator& operator+(int n);
	Line_iterator& operator-(int n);
	TCHAR&	operator*();
	Line_iterator& operator=(const Line_iterator& m);
	bool operator==(const Line_iterator& m);
	bool operator!=(const Line_iterator& m);
private:
	CLine * pLine;			//��ǰ��ָ��			
	TCHAR *		pWChar;			//ָ��CLine�е�һ���ַ�
	DataBlock*	pBlock;			//��ǰָ���ַ��������ݿ�
	int			nIndex;			//��ǰָ����ַ������е�λ��(ע�������±꣬��1��ʼ)	
};
//�õ���������[first,last]֮����ַ���
size_t operator-(Line_iterator last, Line_iterator first);
//ͬһ���У���[first,last]���ַ���start��ʼ������
Line_iterator copy(Line_iterator start, Line_iterator first, Line_iterator	last);