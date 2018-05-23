#pragma once
#ifndef UNICODE
#define UNICODE
#endif // !UNICODE
#include<sstream>
#include<fstream>
#include"CLine.h"
#include"MyExpection.h"
#include<queue>
#include<Windows.h>
#include<time.h>
#include<process.h>
#define AUTO_SAVE_TIME 300														//�Զ�����ʱ����
#define TAB_SIZE	4															//Tab�ַ�Ϊ4�ո�
//�ı��洢�ṹ
class CText
{
	friend class Text_iterator;
	friend class Cursor;
	friend struct Record;
	friend UINT __stdcall Auto_Save_Timer_Thread(LPVOID);						//�Զ����涨ʱ���߳�
public:
	CText();
	~CText();
	void NewFile();																//�½��ļ�
	void ReadText(std::string filename);										//���ļ����б༭
	void Save();																//���浽�ı�  
	void SaveAs(std::string FullPath);											//���Ϊ
	void ClearAll();															//����ı�
	void ShowText()const;
	void DeleteLines(int first, int last);										//ɾ������
	Position Delete(Position first, Position last);								//ɾ��ѡ�в���
	Position	BackSpace(Position position);									//�˸��
	Position	Insert(Position start, std::wstring String);					//�����ַ���
	std::wstring Copy(Position start, Position end);							//��������
	Position	EnterNewLine(Position position);								//�����position�󰴻س�
	bool	SeekStrings(std::wstring Str, Position& start, Position& end, bool upper_lower = true);		//�����ַ���
	bool	ReSeekStrings(std::wstring Str, Position& start, Position& end, bool upper_lower = true);	//��ǰ�����ַ���
	Position	Replace(Position start, Position end, std::wstring Str);		//�滻�ַ���
	bool        isSaved();														//�Ƿ񱣴�
	static std::string Path;													//Ĭ�ϴ洢·��
	//��������
public:
	int Line_Size(int LineNumber);												//����ĳ���ַ�����
	int Line_Width(int LineNumber, int Width, int end = 0);						//���ص�ǰ����ռλ��
	int Max_Line_Width(int Width);												//�ı�����п�
	int Line_Number()const;														//��������
	int Characters(Position start, Position end);								//��¼�ַ�����
	int All_Characters();														//ȫ���ַ�����
	CLine* GetLinePointer(int LineNumber);										//��ȡ��ָ��
	Position First_Position();													//�����ı���һ���ַ�λ��
	Position End_Position();													//�����ı����һ���ַ�λ��
	void Set_File_Name(const std::string Name);									//�����ļ���
	void Set_Path(const std::string path);										//���ô洢·��
	std::string File_Name();													//�����ļ�·��
private:
	CLine * pFirstLineHead;														//���׵�ַ
	int			nLineNumbers;													//����
	std::string	FileName;														//�ļ���		
	bool		bSave;															//�Ƿ񱣴�
	int			Start_Time;														//��ʱ������ʱ��
	HANDLE      hAuto_Save;														//��ʱ�����
	void UpDataLineNumber(CLine* p, int Start);									//�����к�
	void InsertLine(int AfterLineNumber);										//���кź���������
	bool upper_lower_match(TCHAR ch1, TCHAR ch2, bool upper_lower);				//�ж��Ƿ����ƥ��
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
	Text_iterator operator+(int n);
	Text_iterator operator-(int n);
	bool operator==(const Text_iterator& Text);
	bool operator!=(const Text_iterator& Text);
	bool operator<(const Text_iterator& Text);
	bool operator<=(const Text_iterator& Text);
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
void TabToSpace(std::wstring& Str);							//���ַ������Ʊ������Ϊ��Ӧ�Ŀո�

UINT __stdcall Auto_Save_Timer_Thread(LPVOID);				//�Զ����涨ʱ���߳�
std::string Generate_Default_File_Name(const std::string& Path);
int Match_File_Name(const std::string &FileName);			//ƥ���ļ���