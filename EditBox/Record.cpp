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
			int StartLineNumber = start.LineNumber;
			int EndLineNumber = end.LineNumber;
			int n = StartLineNumber;								//对应当前处理行号
			std::wstring Str;
			CLine* pLine = pData->pFirstLineHead;
			start = { start.LineNumber,start.Sequence - 1 };		//恢复的起始点
			Str = pLine->TransformToWString(1, pLine->nDataSize);
			if (StartLineNumber == EndLineNumber)					//删除的段落在同一行
			{
				pText->Insert(start, Str);
				delete pData;
				pData = NULL;
				break;
			}      
			pText->EnterNewLine(start);								//删除的段落大于一行	
			pText->Insert(start, Str);								//首段恢复
			while (pLine->pNextLine != NULL)
			{
				pLine = pLine->pNextLine;
				if (pLine->nLineNumber == EndLineNumber)
					break;
				n++;
				Str = pLine->TransformToWString(1, pLine->nDataSize);
				pText->InsertLine(n - 1);
				CLine* p = pText->GetLinePointer(n);
				p->CreateLine(Str);
			}
			if (pLine != NULL)									//处理尾行
			{
				Str = pLine->TransformToWString(1, pLine->nDataSize);
				pText->Insert({ EndLineNumber ,0 }, Str);
			}
			delete pData;
			pData = NULL;
			break;
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
		case RD_MERGE_LINE:
		{
			pText->EnterNewLine(start);
			break;
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

void Record::Save_Merge_Line_Data(CText * p, int LineNumber)
{
	LineNumber--;
	if (LineNumber < 1)
		throw std::invalid_argument("行号小于1，传参错误");
	CLine* pLine = p->GetLinePointer(LineNumber);
	start = end = { LineNumber,pLine->nDataSize };		//保存EnterNewLine位置
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


