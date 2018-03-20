#pragma once
#ifndef UNICODE
#define UNICODE
#endif // !UNICODE
#include<sstream>
#include<fstream>
#include"CLine.h"
//文本存储结构
class CText
{
public:
	CText();
	~CText();
	void CreateText(std::string filename);
	void ClearAll();
	void ShowText()const;
	void DeleteLines(int first, int last);					//删除整行
	void Delete(Position first, Position last);				//删除选中部分
	void BackSpace(Position position);						//退格键
private:
	CLine *		pFirstLineHead;		//行首地址
	int			nLineNumbers;		//行数
	std::string	FileName;			//文件名	
	void UpDataLineNumber(CLine* p,int Start);				//更新行号
};
