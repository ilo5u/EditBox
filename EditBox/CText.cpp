#include "CText.h"

CText::CText()
{
	pFirstLineHead = NULL;
	nLineNumbers = 0;
	FileName = ".txt";
}

CText::~CText()
{
	ClearAll();
}

void CText::CreateText(std::string filename)
{
	if (pFirstLineHead != NULL)
		ClearAll();
	FileName = filename + FileName;
	std::wifstream	File_r(FileName, std::wifstream::in);		//以读的方式打开txt文件
	std::wstring LineStr;
	CLine* p = nullptr;
	while (std::getline(File_r, LineStr))
	{
		nLineNumbers++;
		if (nLineNumbers == 1)
		{
			if (LineStr.empty())		//空文件
			{
				File_r.close();
				return;		
			}
			pFirstLineHead = new CLine(nLineNumbers);
			pFirstLineHead->CreateLine(LineStr);
			p = pFirstLineHead;
		}
		else
		{
			p->pNextLine = new CLine(nLineNumbers);
			p = p->pNextLine;
			p->CreateLine(LineStr);
		}

	}
	File_r.close();
}

void CText::ClearAll()
{
	while (pFirstLineHead != NULL)
	{
		CLine* temp = pFirstLineHead;
		pFirstLineHead = pFirstLineHead->pNextLine;
		delete temp;
	}
	FileName = ".txt";
	nLineNumbers = 0;
}

void CText::ShowText() const
{
	if (pFirstLineHead != NULL)
	{
		CLine* p = pFirstLineHead;
		while (p != NULL)
		{
			p->ShowLineData();
			p = p->pNextLine;
		}
	}
	else
	{
		std::cout << "空文件" << std::endl;
	}
}

void CText::DeleteLines(int first, int last)
{
	if (first > last || last > nLineNumbers)
		return;
	if (first == 1)
	{
		if (last == nLineNumbers)
		{
			ClearAll();
			return;
		}
		else
		{
			while (first <= last)
			{
				CLine* p = pFirstLineHead;
				pFirstLineHead = pFirstLineHead->pNextLine;
				delete p;
				first++;
			}
			UpDataLineNumber(pFirstLineHead, 1);

		}
	}
	else
	{
		CLine* p = pFirstLineHead;
		while (p->nLineNumber != first - 1)
			p = p->pNextLine;
		while (first <= last)
		{
			CLine* temp = p->pNextLine;
			p->pNextLine = temp->pNextLine;
			delete temp;
			first++;
		}
		UpDataLineNumber(p->pNextLine, p->nLineNumber + 1);
	}
}

void CText::Delete(Position first, Position last)
{
	//删除中间完整行
	if (last.LineNumber - first.LineNumber > 1)
		DeleteLines(first.LineNumber + 1, last.LineNumber - 1);
	//删除剩余部分	首先定位起始行位置
	CLine* p = pFirstLineHead;
	while (p->nLineNumber != first.LineNumber)
		p = p->pNextLine;
	//first last位于相邻两行
	if (first.LineNumber < last.LineNumber)
	{
		p->DeleteLine(first.Sequence, p->nDataSize);
		CLine* pNext = p->pNextLine;
		std::wstring Str = pNext->TransformToWString(last.Sequence + 1, pNext->nDataSize);
		DeleteLines(pNext->nLineNumber, pNext->nLineNumber);
		p->InsertStrings(first.Sequence - 1, Str);
	}
	//first last位于同一行
	else
	{
		p->DeleteLine(first.Sequence, last.Sequence);
	}

}

void CText::BackSpace(Position position)
{
	CLine* preLine = NULL;
	CLine* p = pFirstLineHead;
	while (p->nLineNumber != position.LineNumber)
	{
		preLine = p;
		p = p->pNextLine;
	}
	//需换行
	if (!(p->BackSpace(position)))
	{
		std::wstring LineStr = p->TransformToWString(1, p->nDataSize);
		DeleteLines(p->nLineNumber, p->nLineNumber);
		preLine->InsertStrings(preLine->nDataSize, LineStr);
	}
}

void CText::UpDataLineNumber(CLine * p, int Start)
{
	while (p != NULL)
	{
		p->SetLineNumber(Start);
		p = p->pNextLine;
		Start++;
	}
	nLineNumbers = Start - 1;
}
