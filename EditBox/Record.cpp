#include"stdafx.h"

Record::Record(UINT flag)
{
	ACT = flag;
	pText = NULL;
}

Record::~Record()
{
	pText = NULL;
}

//撤销 
void Record::ReDo(CText* p)
{
	pText = p;
	switch (ACT)
	{
	case RD_DELETE:
	{
		start.Sequence--;									//修正插入点
		end = pText->Insert(start, Deleted_Data);			//end记录恢复删除后最后一个字符的位置
		Deleted_Data.clear();								//清空删除信息
		start.Sequence++;									//start记录恢复删除后第一个字符位置
		break;
	}
	case RD_REPLACE:
	{
		end = p->Replace(start, end, Deleted_Data);			//反向替换
		break;
	}
	case RD_INSERT:
	{
		start = pText->Delete(start, end);
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
		break;
	}
	case RD_MERGE_LINE:
	{
		start = pText->EnterNewLine(start);
		break;
	}
	default:
		break;
	}
}

void Record::Save_Delete_Data(CText * pc, Position first, Position last)
{
	pText = pc;
	Deleted_Data = pText->Copy(first, last);		//拷贝删除的字符串
	start = first;
}

void Record::Save_Merge_Line_Data(CText * p, int LineNumber)
{
	LineNumber--;
	if (LineNumber < 1)
		throw std::invalid_argument("行号小于1，传参错误");
	CLine* pLine = p->GetLinePointer(LineNumber);
	start = end = { LineNumber,pLine->nDataSize };		//保存EnterNewLine位置
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

void Clear_Record(std::stack<Record*>* pRecord_Stack)
{
	while (!pRecord_Stack->empty())
	{
		Record* p = pRecord_Stack->top();
		delete p;
		pRecord_Stack->pop();
	}
}



