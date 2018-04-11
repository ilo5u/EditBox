#include "stdafx.h"

CLine::CLine(int LNum, CLine * pNext)
{
	nLineNumber = LNum;
	pNextLine = pNext;
	pLineHead = NULL;
	bBlankLine = true;
	nBlocks = nDataSize = 0;
}

CLine::CLine(CLine & L)
{
	CLine* pLine = new CLine(L.nLineNumber);
	std::wstring String = L.TransformToWString(1, L.size());
	pLine->CreateLine(String);
	swap(*pLine,true);
	pLine->pLineHead = NULL;
	delete pLine;
}

CLine::CLine(CLine && L)
{
	swap(L, true);
	L.pLineHead = NULL;
}

CLine::~CLine()
{
	ClearLine();
	pNextLine = NULL;
}

//������ǰ��(һ�����ڳ�ʼ��֮��)
void CLine::CreateLine(std::wstring & String)
{
	if (String.empty() || String[0] == L'\n')		//�س� ֱ�ӻ��� ����Ϊ��
	{
		bBlankLine = true;
		return;
	}
	//�ǿ���
	bBlankLine = false;
	if (pLineHead != NULL)
		ClearLine();
	nDataSize = String.size();
	//��ǰ���������ݿ�����
	nBlocks = nDataSize / BLOCK_SIZE;
	if (nDataSize%BLOCK_SIZE != 0)
		nBlocks = nBlocks + 1;
	pLineHead = new DataBlock;
	DataBlock* p = pLineHead;
		//���δ���nBlocks����
	std::wstring::iterator Iterator = String.begin();
	for (int i = 1; i <= nBlocks; i++)
	{
		//�Ե�������������
		for (int j = 0; Iterator != String.end() && j < BLOCK_SIZE; j++)
		{
			p->Strings[j] = *Iterator;
			++Iterator;
		}
		if (i != nBlocks)
		{
			p->pNextBlock = new DataBlock;
			p = p->pNextBlock;
		}
	}
}

//��յ�ǰ���������ݿ�
void CLine::ClearLine()
{
	if (pLineHead != NULL)
	{
		DataBlock* p = pLineHead;
		while (p != NULL)
		{
			DataBlock* temp = p->pNextBlock;
			delete p;
			p = temp;
		}
		pLineHead = NULL;
		nDataSize = 0;
		nBlocks = 0;
	}
	bBlankLine = true;
}

void CLine::ShowLineData()
{
	printf("%d ", nLineNumber);
	if (pLineHead != NULL)
	{
		std::wcout.imbue(std::locale(std::locale(), "", LC_CTYPE));		//�л����������������
	//	printf("�к�:%d,���ݿ�����:%d,�ַ�����:%d,����:", nLineNumber, nBlocks, nDataSize);
		DataBlock* p = pLineHead;
		while (p != NULL)
		{
			DataBlock* temp = p->pNextBlock;
			printf("%S", p->Strings);
			p = temp;
		}
		std::cout << std::endl;
	}
	else
	{
		std::cout << std::endl;
	}
}

/*
ɾ������[first,last]֮����ַ�
*/
Line_iterator CLine::DeleteLine(int first, int last)
{
	if (bBlankLine || first > last || first > nDataSize || last == 0)	//���л���Ч�Ĳ���
		return Line_iterator(*this);
	if (first == 1 && last == nDataSize)		//ɾ������
	{
		ClearLine();
		return Line_iterator(*this);
	}
	Line_iterator it_first(*this, first);
	Line_iterator it_last(*this, last);
	Line_iterator it_end = end();
	if (it_end == it_last)
	{
		DeleteSpareBlocks(it_first.pBlock->pNextBlock);
		it_first.pBlock->pNextBlock = NULL;
		if (it_first.nIndex%BLOCK_SIZE == 1)
		{
			--it_first;
			DeleteSpareBlocks(it_first.pBlock->pNextBlock);
			it_first.pBlock->pNextBlock = NULL;
		}
		else
		{
			*it_first = L'\0';
			--it_first;
		}
		nDataSize -= (last - first + 1);
		return it_first;
	}
	else 
	{
		it_first = copy(it_first, it_last + 1, it_end);
		DeleteLine((it_first + 1).nIndex, nDataSize);
		return it_first;
	}
}
/*
��position�����˸��
���� false ��ʾ��Ҫ���� �׳�false���ⲿ���ú�������
���� true ��ʾ��ȷɾ���ַ�
*/
bool CLine::BackSpace(Position position)
{
	if (position.Sequence == 0)
	{
		if (position.LineNumber == 1)
			return true;
		else
			return false;
	}
	else
	{
		DeleteLine(position.Sequence, position.Sequence);
		return true;
	}
}
/*
��start��������ַ��� ��
eg		abcdef    insert "123"  start=3
------->abc123def
			start==0			����������ַ���
			start==nDataSize	����ĩ����ַ�
			start==x			��xλ�ú�������ַ���
			���ز���ĵ�һλ�ַ�λ��
		return Line_iterator(4)
*/
Line_iterator CLine::InsertStrings(int start, std::wstring String)
{
	if (String.empty())
		return end();
	//�ڿ��в���
	if (bBlankLine)
	{
		CreateLine(String);
		bBlankLine = false;
		return begin();
	}
	Line_iterator it_start(*this,start);
	Line_iterator it_end = this->end();
	int Size_to_add = String.size();
	if (start == nDataSize)		//ֱ������β�����ַ�
	{
		//����ǰ��Ϊ����
		if (bBlankLine)
		{
			CreateLine(String);
			return begin();
		}
		//�Ƚ���ǰ�鲹����������µ�strings
		int n = nBlocks * BLOCK_SIZE - it_start.nIndex;
		while (n > 0 && (!String.empty()))
		{
			wchar_t wch = String[0];
			String.erase(String.begin());
			++it_start;
			*it_start = wch;
			n--;
		}
		//����ʱString�Էǿգ����½��в��뵽����ĩβ
		if (!String.empty())
		{
			CLine* pnewLine = new CLine(nLineNumber);
			pnewLine->CreateLine(String);
			it_start.pBlock->pNextBlock = pnewLine->pLineHead;
			nBlocks += pnewLine->nBlocks;
			pnewLine->pLineHead = NULL;
			delete pnewLine;
		}
		else
		{
			if (n != 0)
			{
				*(it_start + 1) = L'\0';
			}
		}
	}
	else if (start == 0)	//�����ײ���
	{
		Line_iterator it_start=begin();
		CLine* pnewLine = new CLine(nLineNumber);
		pnewLine->CreateLine(String);
		//�ν�
		Line_iterator NewLineEnd = pnewLine->end();
		NewLineEnd.pBlock->pNextBlock = pLineHead;
		pLineHead = pnewLine->pLineHead;
		pnewLine->pLineHead = NULL;
		nBlocks += pnewLine->nBlocks;
		//�ƶ�
		int Size_to_move = pnewLine->nBlocks*BLOCK_SIZE - pnewLine->nDataSize;
		if (Size_to_move != 0)			//��Ҫ���Ͽ�λ
		{
			NewLineEnd = copy(NewLineEnd + 1, it_start, it_end);
			*(NewLineEnd + 1) = L'\0';
			if (NewLineEnd.pBlock->pNextBlock != NULL)		//�ƶ�֮�����������ݿ�
			{
				DeleteSpareBlocks(NewLineEnd.pBlock->pNextBlock);
				NewLineEnd.pBlock->pNextBlock = NULL;
			}
		}
	}
	else     //��֮�����
	{
		std::wstring backstr = TransformToWString(it_start.nIndex + 1, nDataSize);		//����start������ַ���
		//ɾ���ⲿ��
		nDataSize -= backstr.size();
		DeleteSpareBlocks(it_start.pBlock->pNextBlock);
		it_start.pBlock->pNextBlock = NULL;
		//��������
		String += backstr;
		Size_to_add = String.size();
		return InsertStrings(start, String);
	}
	nDataSize += Size_to_add;
	return Line_iterator(*this, start + 1);
}

Line_iterator CLine::begin()
{
	return Line_iterator(*this);
}

Line_iterator CLine::end()
{
	Line_iterator It = begin() + max((nDataSize - 1),0);
	return It;
}
/*
[first,last]�ڵ��ַ�ת��Ϊwstring��ʽ
eg.		abcdef  [2,4]
return  "bcd"
*/
std::wstring CLine::TransformToWString(int first, int last) 
{
	std::wstring WStr;
	if (bBlankLine || first > nDataSize)
		return WStr;
	TCHAR wch;
	Line_iterator it_first(*this, first);
	Line_iterator it_last(*this, last);
	while (first <= last)
	{
		wch = *it_first;
		WStr.push_back(wch);
		++it_first;
		++first;
	}
	return WStr;
}

size_t CLine::size() const
{
	return nDataSize;
}

CLine & CLine::operator=(CLine Line)
{
	if (pLineHead != NULL)
		ClearLine();
	swap(Line,true);
	return *this;
}

void CLine::SetLineNumber(int Number)
{
	nLineNumber = Number;
}

bool CLine::isBlankLine() const
{
	return bBlankLine;
}
//���ص�ǰ���п����أ�
int CLine::Line_Width(int Width)
{
	if (bBlankLine)
		return 0;
	int Width_EN = Width;
	int Width_ZH = Width * 2;
	int	Total_Width = 0;
	Line_iterator iterator(*this);
	for (int i = 1; i <= nDataSize; i++)
	{
		short int m = *iterator;		//�õ��ַ��ı���ֵ
		if (WORD(m >> 8) > 0)		//����
			Total_Width += Width_ZH;
		else                       //Ӣ��
			Total_Width += Width_EN;
		++iterator;
	}
	return Total_Width;
}

//ɾ����������ݿ�
void CLine::DeleteSpareBlocks(DataBlock * p)
{
	while (p != NULL)
	{
		DataBlock* temp = p->pNextBlock;
		delete p;
		nBlocks--;
		p = temp;
	}
}

void CLine::swap(CLine & Line, bool SWAP_Next_Line_DATA)
{
	using std::swap;
	swap(pLineHead, Line.pLineHead);
	swap(nBlocks, Line.nBlocks);
	swap(nDataSize, Line.nDataSize);
	swap(bBlankLine, Line.bBlankLine);
	//�б�Ҫ�Ļ�������һ�еĶ���
	if (SWAP_Next_Line_DATA)
	{
		swap(pNextLine, Line.pNextLine);
		swap(nLineNumber, Line.nLineNumber);
	}
		
}

Line_iterator::Line_iterator(CLine & theLine, int index):pLine(&theLine)
{
	if (theLine.pLineHead != NULL)					//��Ϊ���е����
	{
		pWChar = theLine.pLineHead->Strings;		//ָ�������ַ�
		pBlock = theLine.pLineHead;
		nIndex = 1;
		if (index)
			*this = *this + (index - 1);
		else
			nIndex = 0;		//����0˵������Ϊ��
	}
	else
	{
		pLine = NULL;
		pWChar = NULL;
		pBlock = NULL;
		nIndex = 0;
	}
}

Line_iterator::Line_iterator(const Line_iterator & m)
{
	pWChar = m.pWChar;
	pBlock = m.pBlock;
	nIndex = m.nIndex;
	pLine = m.pLine;
}


Line_iterator & Line_iterator::operator++()
{
	if (nIndex%BLOCK_SIZE == 0)
	{
		pBlock = pBlock->pNextBlock;
		pWChar = pBlock->Strings;
	}
	else 
	{
		pWChar++;
	}
	nIndex++;
	return *this;
}

Line_iterator Line_iterator::operator++(int)
{
	Line_iterator temp(*this);
	++(*this);
	return temp;
}

Line_iterator & Line_iterator::operator--()
{
	if (nIndex%BLOCK_SIZE == 1)
	{
		DataBlock* p = pLine->pLineHead;
		while (p != pBlock && p->pNextBlock != pBlock)
		{
			p = p->pNextBlock;
		}
		pWChar = &(p->Strings[BLOCK_SIZE - 1]);
		pBlock = p;
	}
	else
	{
		pWChar--;
	}
	nIndex--;
	return *this;
}

Line_iterator Line_iterator::operator--(int)
{
	Line_iterator temp(*this);
	--(*this);
	return temp;
}

Line_iterator & Line_iterator::operator=(const Line_iterator & m)
{
	pWChar = m.pWChar;
	pBlock = m.pBlock;
	nIndex = m.nIndex;
	pLine = m.pLine;
	return *this;
}

Line_iterator::~Line_iterator()
{
	pLine = NULL;
	pWChar = NULL;
	pLine = NULL;
	pBlock = NULL;
}
/*�����е�������ǰ�󶨵���*/
void Line_iterator::Set(CLine & theLine, int index)
{
	pLine = &theLine;
	if (theLine.pLineHead != NULL)
	{
		pWChar = theLine.pLineHead->Strings;		//ָ�������ַ�
		pBlock = theLine.pLineHead;
		nIndex = 1;
		if (index)
			*this = *this + (index - 1);
	}
	else
	{
		pWChar = NULL;
		pBlock = NULL;
		nIndex = 0;
	}
}
//���ص�ǰ������ָ����ַ�����λ��
int Line_iterator::CurrentPosition() const
{
	return nIndex;
}
//���ص�ǰ��ָ��
CLine * Line_iterator::GetLinePointer()
{
	return pLine;
}

Line_iterator & Line_iterator::operator+(int n)
{
	int BeginIndex = nIndex % BLOCK_SIZE - 1;		//��ǰ���ڿ����±�
	int Blocks_to_add = n / BLOCK_SIZE;				
	int Remainder = n % BLOCK_SIZE;					//���ƫ����
	nIndex += Blocks_to_add * BLOCK_SIZE;
	while (Blocks_to_add)
	{
		pBlock = pBlock->pNextBlock;
		Blocks_to_add--;
	}
	pWChar = &(pBlock->Strings[BeginIndex]);
	while (Remainder)
	{
		++(*this);
		--Remainder;
	}
	return *this;
}

Line_iterator & Line_iterator::operator-(int n)
{
	while (n)
	{
		--(*this);
		--n;
	}
	return *this;
}
/*
���ص�������ָλ�õ��ַ�����
ע�⣺���ڴ��ڿ��е�����£�ʹ��ʱӦ����֤pWChar��Ч
*/
TCHAR& Line_iterator::operator*()
{
	return *pWChar;
}
/*
��Ҫ����*����ǰ�ж�
����ֵ��
		true		��ǰ������ָ����Ч�ַ�
		false		��ǰ������δ�󶨻�ǰ��Ϊ����
*/
bool Line_iterator::isValid() const
{
	if (pWChar == NULL)
		return false;
	else
		return true;
}

bool Line_iterator::operator==(const Line_iterator & m)
{
	return	pBlock == m.pBlock&&nIndex == m.nIndex;		//��ָ����ַ�λ��ͬһ������ͬһλ�� �����
}

bool Line_iterator::operator!=(const Line_iterator & m)
{
	return !(*this == m);
}

size_t operator-(Line_iterator last, Line_iterator first)
{
	size_t distance = 0;
	while (first != last)
	{
		++first;
		++distance;
	}
	return distance + 1;
}

Line_iterator copy(Line_iterator start, Line_iterator first, Line_iterator last)
{
	while (first != last)
	{
		*start = *first;
		++first;
		++start;
	}
	*start = *last;
	return start;
}

bool Position::operator<(Position & position)
{
	if (LineNumber < position.LineNumber)
		return true;
	else
		return Sequence < position.Sequence;
}

bool Position::operator<=(Position & position)
{
	return !(*this > position);
}

bool Position::operator>(Position & position)
{
	return (!(*this < position)) && (*this != position);
}

bool Position::operator>=(Position & position)
{
	return !(*this < position);
}

bool Position::operator==(Position & position)
{
	return LineNumber == position.LineNumber&&Sequence == position.Sequence;
}

bool Position::operator!=(Position & position)
{
	return !(*this == position);
}
