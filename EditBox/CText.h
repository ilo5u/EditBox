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
#define AUTO_SAVE_TIME 300														//自动保存时间间隔
#define TAB_SIZE	4															//Tab字符为4空格
//文本存储结构
class CText
{
	friend class Text_iterator;
	friend class Cursor;
	friend struct Record;
	friend UINT __stdcall Auto_Save_Timer_Thread(LPVOID);						//自动保存定时器线程
public:
	CText();
	~CText();
	void NewFile();																//新建文件
	void ReadText(std::string filename);										//打开文件进行编辑
	void Save();																//保存到文本  
	void SaveAs(std::string FullPath);											//另存为
	void ClearAll();															//清空文本
	void ShowText()const;
	void DeleteLines(int first, int last);										//删除整行
	Position Delete(Position first, Position last);								//删除选中部分
	Position	BackSpace(Position position);									//退格键
	Position	Insert(Position start, std::wstring String);					//插入字符串
	std::wstring Copy(Position start, Position end);							//拷贝段落
	Position	EnterNewLine(Position position);								//光标在position后按回车
	bool	SeekStrings(std::wstring Str, Position& start, Position& end, bool upper_lower = true);		//查找字符串
	bool	ReSeekStrings(std::wstring Str, Position& start, Position& end, bool upper_lower = true);	//向前查找字符串
	Position	Replace(Position start, Position end, std::wstring Str);		//替换字符串
	bool        isSaved();														//是否保存
	static std::string Path;													//默认存储路径
	//交互函数
public:
	int Line_Size(int LineNumber);												//返回某行字符数量
	int Line_Width(int LineNumber, int Width, int end = 0);						//返回当前行所占位宽
	int Max_Line_Width(int Width);												//文本最大行宽
	int Line_Number()const;														//返回行数
	int Characters(Position start, Position end);								//记录字符数量
	int All_Characters();														//全文字符数量
	CLine* GetLinePointer(int LineNumber);										//获取行指针
	Position First_Position();													//返回文本第一个字符位置
	Position End_Position();													//返回文本最后一个字符位置
	void Set_File_Name(const std::string Name);									//设置文件名
	void Set_Path(const std::string path);										//设置存储路径
	std::string File_Name();													//返回文件路径
private:
	CLine * pFirstLineHead;														//行首地址
	int			nLineNumbers;													//行数
	std::string	FileName;														//文件名		
	bool		bSave;															//是否保存
	int			Start_Time;														//定时器启动时间
	HANDLE      hAuto_Save;														//定时器句柄
	void UpDataLineNumber(CLine* p, int Start);									//更新行号
	void InsertLine(int AfterLineNumber);										//在行号后面插入空行
	bool upper_lower_match(TCHAR ch1, TCHAR ch2, bool upper_lower);				//判断是否符合匹配
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
	Text_iterator operator+(int n);
	Text_iterator operator-(int n);
	bool operator==(const Text_iterator& Text);
	bool operator!=(const Text_iterator& Text);
	bool operator<(const Text_iterator& Text);
	bool operator<=(const Text_iterator& Text);
private:
	CText * pText;											//指向文本文件对象的指针
	Line_iterator currLine;									//绑定的当前行迭代器
};

int* GetNextValArray(std::wstring SubStr);					//获取字串的NextVal数组

size_t NumberOfZH(const std::wstring &wstr);				//宽字符串中中文字符数量

std::wstring StringToWString(const std::string& s);			//实现字符转换
std::string WStringToString(const std::wstring& ws);		//实现字符转换
std::string wchTostring(TCHAR* pwch);						//宽字符指针转换为string
void WStringToWch(const std::wstring &ws, TCHAR* &pwch);	//宽字符串转化为指针
std::queue<std::wstring> WStrToLineWStr(std::wstring WSTR);	//将包含换行符的字符串转化为不同行的字符串(不含换行符)		
void TabToSpace(std::wstring& Str);							//将字符串中制表符更替为对应的空格

UINT __stdcall Auto_Save_Timer_Thread(LPVOID);				//自动保存定时器线程
std::string Generate_Default_File_Name(const std::string& Path);
int Match_File_Name(const std::string &FileName);			//匹配文件名