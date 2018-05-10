#pragma once
#ifndef UNICODE
#define UNICODE
#endif // !UNICODE
#include<sstream>
#include<fstream>
#include"CLine.h"
#include<queue>
#include<Windows.h>

//�ı��洢�ṹ
class CText
{
	friend class Text_iterator;
	friend class Cursor;
	friend struct Record;
public:
	CText();
	~CText();
	void NewFile();																//�½��ļ�
	void ReadText(std::string filename);										//���ļ����б༭
	void Save(std::string filename = "", bool isChange = false);				//���浽�ı�  
	void ClearAll();															//����ı�
	void ShowText()const;
	void DeleteLines(int first, int last);										//ɾ������
	Position Delete(Position first, Position last);								//ɾ��ѡ�в���
	Position	BackSpace(Position position);									//�˸��
	Position	Insert(Position start, std::wstring String);						//�����ַ���
	std::wstring Copy(Position start, Position end);							//��������
	Position	EnterNewLine(Position position);								//�����position�󰴻س�
	bool	SeekStrings(std::wstring Str, Position& start, Position& end);		//�����ַ���
	Position	Replace(Position start, Position end, std::wstring Str);		//�滻�ַ���
	bool        isSaved();														//�Ƿ񱣴�
	std::string FilePath();														//�����ļ�·��
																				//��������
public:
	int Line_Size(int LineNumber);												//����ĳ���ַ�����
	int Line_Width(int LineNumber, int Width, int end = 0);						//���ص�ǰ����ռλ��
	int Max_Line_Width(int Width);												//�ı�����п�
	int Line_Number()const;														//��������
	CLine* GetLinePointer(int LineNumber);										//��ȡ��ָ��
	Position First_Position();													//�����ı���һ���ַ�λ��
	Position End_Position();													//�����ı����һ���ַ�λ��
private:
	CLine * pFirstLineHead;							//���׵�ַ
	int			nLineNumbers;							//����
	std::string	FileName;								//�ļ���		
	bool		bSave;									//�Ƿ񱣴�
	void UpDataLineNumber(CLine* p, int Start);			//�����к�
	void InsertLine(int AfterLineNumber);				//���кź���������
};

/*ȫ�ı�������*/
class Text_iterator
{
public:
	Text_iterator() = default;
	Text_iterator(CText & Text, int LineNumber = 1, int position = 1);
	~Text_iterator();
	bool isEnd();												//�Ƿ�Ϊ��ĩ
	void GoPosition(Position position);							//��λ
	void GoEnd();												//ת����ĩ
	Position GetCurPositin();									//��ǰλ��
	int CurrentLineNumber();									//��ǰ�к�
																//operators
	TCHAR operator*();
	Text_iterator& operator++();
	Text_iterator  operator++(int);
	Text_iterator& operator--();
	Text_iterator  operator--(int);
	Text_iterator& operator+(int n);
	Text_iterator& operator-(int n);
	bool operator==(const Text_iterator& Text);
	bool operator!=(const Text_iterator& Text);
private:
	CText * pText;											//ָ���ı��ļ������ָ��
	Line_iterator currLine;									//�󶨵ĵ�ǰ�е�����
};
int* GetNextValArray(std::wstring SubStr);					//��ȡ�ִ���NextVal����
size_t NumberOfZH(const std::wstring &wstr);				//���ַ����������ַ�����
std::wstring StringToWString(const std::string& s);			//ʵ���ַ�ת��
std::string WStringToString(const std::wstring& ws);		//ʵ���ַ�ת��
std::string wchTostring(TCHAR* pwch);						//���ַ�ָ��ת��Ϊstring
void WStringToWch(const std::wstring &ws, TCHAR* &pwch);	//���ַ���ת��Ϊָ��
std::queue<std::wstring> WStrToLineWStr(std::wstring WSTR);	//���������з����ַ���ת��Ϊ��ͬ�е��ַ���(�������з�)		
