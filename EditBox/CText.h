#pragma once
#ifndef UNICODE
#define UNICODE
#endif // !UNICODE
#include<sstream>
#include<fstream>
#include"CLine.h"
#include<queue>
#include<Windows.h>

//文本存储结构
class CText
{
	friend class Text_iterator;
	friend class Cursor;
	friend struct Record;
public:
	CText();
	~CText();
	void NewFile();																//新建文件
	void ReadText(std::string filename);										//打开文件进行编辑
	void Save(std::string filename = "", bool isChange = false);				//保存到文本  
	void ClearAll();															//清空文本
	void ShowText()const;
	void DeleteLines(int first, int last);										//删除整行
	Position Delete(Position first, Position last);								//删除选中部分
	Position	BackSpace(Position position);									//退格键
	Position	Insert(Position start, std::wstring String);						//插入字符串
	std::wstring Copy(Position start, Position end);							//拷贝段落
	Position	EnterNewLine(Position position);								//光标在position后按回车
	std::queue<Position>	SeekStrings(std::wstring Str, Position start);		//查找字符串
	Position	Replace(Position start, Position end, std::wstring Str);		//替换字符串
	bool        isSaved();														//是否保存
	std::string FilePath();														//返回文件路径
																				//交互函数
public:
	int Line_Size(int LineNumber);												//返回某行字符数量
	int Line_Width(int LineNumber, int Width, int end = 0);						//返回当前行所占位宽
	int Max_Line_Width(int Width);												//文本最大行宽
	int Line_Number()const;														//返回行数
	CLine* GetLinePointer(int LineNumber);										//获取行指针
	Position First_Position();													//返回文本第一个字符位置
	Position End_Position();													//返回文本最后一个字符位置
private:
	CLine * pFirstLineHead;							//行首地址
	int			nLineNumbers;							//行数
	std::string	FileName;								//文件名		
	bool		bSave;									//是否保存
	void UpDataLineNumber(CLine* p, int Start);			//更新行号
	void InsertLine(int AfterLineNumber);				//在行号后面插入空行
};

/*全文本迭代器*/
class Text_iterator
{
public:
	Text_iterator() = default;
	Text_iterator(CText & Text, int LineNumber = 1, int position = 1);
	~Text_iterator();
	bool isEnd();												//是否为行末
	void GoPosition(Position position);							//定位
	void GoEnd();												//转到行末
	Position GetCurPositin();									//当前位置
	int CurrentLineNumber();									//当前行号
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
	CText * pText;											//指向文本文件对象的指针
	Line_iterator currLine;									//绑定的当前行迭代器
};
int* GetNextValArray(std::wstring SubStr);					//获取字串的NextVal数组
size_t NumberOfZH(const std::wstring &wstr);				//宽字符串中中文字符数量
std::wstring StringToWString(const std::string& s);			//实现字符转换
std::string WStringToString(const std::wstring& ws);		//实现字符转换
std::string wchTostring(TCHAR* pwch);						//宽字符指针转换为string
void WStringToWch(const std::wstring &ws, TCHAR* &pwch);		//宽字符串转化为指针
std::queue<std::wstring> WStrToLineWStr(std::wstring WSTR);	//将包含换行符的字符串转化为不同行的字符串(不含换行符)		
