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

//���� 
void Record::ReDo(CText* p)
{
	pText = p;
	switch (ACT)
	{
	case RD_DELETE:
	{
		start.Sequence--;									//���������
		end = pText->Insert(start, Deleted_Data);			//end��¼�ָ�ɾ�������һ���ַ���λ��
		Deleted_Data.clear();								//���ɾ����Ϣ
		start.Sequence++;									//start��¼�ָ�ɾ�����һ���ַ�λ��
		break;
	}
	case RD_REPLACE:
	{
		end = p->Replace(start, end, Deleted_Data);			//�����滻
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
	Deleted_Data = pText->Copy(first, last);		//����ɾ�����ַ���
	start = first;
}

void Record::Save_Merge_Line_Data(CText * p, int LineNumber)
{
	LineNumber--;
	if (LineNumber < 1)
		throw std::invalid_argument("�к�С��1�����δ���");
	CLine* pLine = p->GetLinePointer(LineNumber);
	start = end = { LineNumber,pLine->nDataSize };		//����EnterNewLineλ��
}



void Record::Set_Choose_Data(Position first, Position last)
{
	start = first;
	end = last;
}

//���ѡ����Ϣ
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



