#include "stdafx.h"

Record::Record(UINT flag)
{
	ACT = flag;
	pText = NULL;
	pData = NULL;
}

Record::~Record()
{
	pText = NULL;
	delete pData;
	pData = NULL;
}

//撤销 
void Record::ReDo(CText* p)
{
	pText = p;
	switch (ACT)
	{
		case RD_DELETE:
		{

		}
		case RD_INSERT:
		{
			pText->Delete(start, end);
			break;
		}
		case RD_RETURN:
		{
			int LineNumber = start.LineNumber;
			int nextLineNumber = LineNumber + 1;
			CLine* pLine = pText->GetLinePointer(LineNumber);
			CLine* pNextLine = pLine->pNextLine;
			std::wstring Str = pNextLine->TransformToWString(1, pNextLine->nDataSize);
			pText->DeleteLines(nextLineNumber, nextLineNumber);
			pLine->InsertStrings(start.Sequence, Str);
		}
		default:
			break;
	}
}

void Record::Save_Delete_Data(CText * pc, Position first, Position last)
{
	pText = pc;
	Set_Choose_Data(first, last);
	//对删除内容保留
	int LineNumber = first.LineNumber;
	std::wstring Str;
	pData = new CText;
	pData->pFirstLineHead = new CLine(LineNumber);			//创建首行
	CLine* pLine = pText->GetLinePointer(first.LineNumber);	
	CLine* p = pData->pFirstLineHead;
	if (first.LineNumber == last.LineNumber)
	{
		Str = pLine->TransformToWString(first.Sequence, last.Sequence);
		pData->pFirstLineHead->CreateLine(Str);
	}
	else
	{
		Str = pLine->TransformToWString(first.Sequence, pLine->nDataSize);
		pData->pFirstLineHead->CreateLine(Str);
		LineNumber++;
		p->pNextLine = new CLine(LineNumber);
		pLine = pLine->pNextLine;
		p = p->pNextLine;
		while (LineNumber != last.LineNumber)
		{
			Str = pLine->TransformToWString(1, pLine->nDataSize);
			p->CreateLine(Str);
			LineNumber++;
			p->pNextLine = new CLine(LineNumber);		//创建下一行
			p = p->pNextLine;
			pLine = pLine->pNextLine;
		}
		Str = pLine->TransformToWString(1, last.Sequence);
		p->CreateLine(Str);
	}
}

void Record::Save_Insert_Data(Position first, Position last)
{
	start = first;
	end = last;
}


void Record::Set_Choose_Data(Position first, Position last)
{
	start = first;
	end = last;
}

//清空选中信息
void Record::Clear_Choose_Data()
{
	start.LineNumber = start.Sequence = 0;
	end.LineNumber = end.Sequence = 0;
}


