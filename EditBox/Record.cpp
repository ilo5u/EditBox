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

//���� 
void Record::ReDo(CText* p)
{
	pText = p;
	switch (ACT)
	{
		case RD_DELETE:
		{
			int StartLineNumber = start.LineNumber;
			int EndLineNumber = end.LineNumber;
			int n = StartLineNumber;								//��Ӧ��ǰ�����к�
			std::wstring Str;
			CLine* pLine = pData->pFirstLineHead;
			start = { start.LineNumber,start.Sequence - 1 };		//�ָ�����ʼ��
			Str = pLine->TransformToWString(1, pLine->nDataSize);
			if (StartLineNumber == EndLineNumber)					//ɾ���Ķ�����ͬһ��
			{
				pText->Insert(start, Str);
				delete pData;
				pData = NULL;
				break;
			}      
			pText->EnterNewLine(start);								//ɾ���Ķ������һ��	
			pText->Insert(start, Str);								//�׶λָ�
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
			if (pLine != NULL)									//����β��
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
		default:
			break;
	}
}

void Record::Save_Delete_Data(CText * pc, Position first, Position last)
{
	pText = pc;
	Set_Choose_Data(first, last);
	//��ɾ�����ݱ���
	int LineNumber = first.LineNumber;
	std::wstring Str;
	pData = new CText;
	pData->pFirstLineHead = new CLine(LineNumber);			//��������
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
			p->pNextLine = new CLine(LineNumber);		//������һ��
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

//���ѡ����Ϣ
void Record::Clear_Choose_Data()
{
	start.LineNumber = start.Sequence = 0;
	end.LineNumber = end.Sequence = 0;
}


