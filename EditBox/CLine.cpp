#include"stdafx.h"
CLine::CLine(int LNum, CLine * pNext)
{
	nLineNumber = LNum;
	pNextLine = pNext;
	pLineHead = NULL;
	bBlankLine = true;
	nBlocks = nDataSize = 0;
}

CLine::CLine(CLine & L) :pLineHead(NULL), pNextLine(L.pNextLine), nLineNumber(L.nLineNumber)
{
	std::wstring String = L.TransformToWString(1, L.size());
	CreateLine(String);
}


CLine::~CLine()
{
	ClearLine();
	pNextLine = NULL;
}

//������ǰ��(һ�����ڳ�ʼ��֮��)
void CLine::CreateLine(std::wstring & String)
{
	if (isChange_Line_Character(String))			//�س� ֱ�ӻ��� ����Ϊ��
	{
		bBlankLine = true;
		nDataSize = 0;
		pLineHead = NULL;
		nBlocks = 0;
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
�������һ���ַ�������
*/
Line_iterator CLine::DeleteLine(int first, int last)
{
	if (bBlankLine || first > last || first > nDataSize || last == 0)	//���л���Ч�Ĳ���
		return end();
	if (first == 1 && last == nDataSize)		//ɾ������
	{
		ClearLine();
		return end();
	}
	Line_iterator it_first(*this, first);
	Line_iterator it_last(*this, last);
	Line_iterator it_end = end();
	if (it_end == it_last)
	{
		--it_first;
		DeleteSpareBlocks(it_first.pBlock->pNextBlock);
		it_first.pBlock->pNextBlock = NULL;
		/*
		if (it_first.nIndex%BLOCK_SIZE == 1)
		{
		--it_first;
		DeleteSpareBlocks(it_first.pBlock->pNextBlock);
		it_first.pBlock->pNextBlock = NULL;
		}
		*/
		nDataSize -= (last - first + 1);
		return it_first;
	}
	else
	{
		it_first = copy(it_first, it_last + 1, it_end);
		DeleteLine(it_first.nIndex + 1, nDataSize);
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
��start ���� �����ַ��� ��
eg		abcdef    insert "123"  start=3
------->abc123def
start==0			����������ַ���
start==nDataSize	����ĩ����ַ�
start==x			��xλ�ú�������ַ���
���ز���ĵ�һλ�ַ�λ��
return Line_iterator(4)
*/
void CLine::InsertStrings(int start, std::wstring String)
{
	if (isChange_Line_Character(String))			//���뻻�з�
	{
		if (start == 0)			//����
		{
			CLine* p = new CLine(*this);		//��������
			ClearLine();
			pNextLine = p;
		}
		else
		{
			String = TransformToWString(start + 1, nDataSize);
			DeleteLine(start + 1, nDataSize);
			CLine* p = new CLine(nLineNumber + 1, pNextLine);
			p->CreateLine(String);
			pNextLine = p;
		}
		UpDataLineNumber();
		return;
	}
	//�ڿ��в���
	if (bBlankLine)
	{
		CreateLine(String);
		return;
	}
	Line_iterator it_start(*this, start);
	Line_iterator it_end = end();
	int Size_to_add = String.size();
	if (start == nDataSize)		//ֱ������β�����ַ�
	{
		nDataSize += Size_to_add;
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
	}
	else if (start == 0)	//�����ײ���
	{
		CLine* pnewLine = new CLine(nLineNumber);
		pnewLine->CreateLine(String);

		int istart = pnewLine->nDataSize + 1;
		pnewLine->end().pBlock->pNextBlock = pLineHead;
		pLineHead = pnewLine->pLineHead;
		nBlocks += pnewLine->nBlocks;

		int ifirst = pnewLine->nBlocks*BLOCK_SIZE + 1;
		int ilast = ifirst + nDataSize - 1;
		nDataSize += pnewLine->nBlocks*BLOCK_SIZE;

		Line_iterator iend = copy(Line_iterator(*this, istart), Line_iterator(*this, ifirst), Line_iterator(*this, ilast));
		DeleteLine(iend.nIndex + 1, nDataSize);

	}
	else     //��֮�����
	{
		std::wstring backstr = TransformToWString(start + 1, nDataSize);				//����start������ַ���
		DeleteLine(start + 1, nDataSize);												//ɾ���ⲿ��
		String += backstr;
		return InsertStrings(nDataSize, String);

	}

}

Line_iterator CLine::begin()
{
	return Line_iterator(*this);
}

Line_iterator CLine::end()
{
	return Line_iterator(*this, nDataSize);
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
	last = min(nDataSize, last);
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

CLine & CLine::operator=(CLine& Line)
{
	if (pLineHead != NULL)
		ClearLine();
	std::wstring String = Line.TransformToWString(1, Line.size());
	CreateLine(String);
	pNextLine = Line.pNextLine;
	nLineNumber = Line.nLineNumber;
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
//���ص�ǰ���п�[1-nDatasize/end]�����أ� ������������ѡ 
//end == 0 �����п� end != 0 ����[1-end]�Ŀ��
int CLine::Line_Width(int Width, int end)
{
	if (bBlankLine)
		return 0;
	int n = (end == 0 ? nDataSize : min(end, nDataSize));
	int Width_EN = Width;
	int Width_ZH = Width * 2;
	int	Total_Width = 0;
	Line_iterator iterator(*this);
	for (int i = 1; i <= n; i++)
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

/*
ɾ��pָ�뼰��֮���DataBlock
*/
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

//�����к�
void CLine::UpDataLineNumber()
{
	CLine* pLine = pNextLine;
	int n = nLineNumber + 1;
	while (pLine != NULL)
	{
		pLine->nLineNumber = n;
		n++;
		pLine = pLine->pNextLine;
	}
}




Line_iterator::Line_iterator(CLine & theLine, int index) :pLine(&theLine)
{
	if (theLine.pLineHead != NULL && index > 0)					//��Ϊ���е����
	{
		pWChar = theLine.pLineHead->Strings;						//ָ�������ַ�
		pBlock = theLine.pLineHead;
		nIndex = 1;
		bAfter_end = FALSE;
		*this = *this + (index - 1);
	}
	else
	{
		pLine = NULL;
		pWChar = NULL;
		pBlock = NULL;
		nIndex = 0;
		bAfter_end = TRUE;
	}
}

Line_iterator::Line_iterator(const Line_iterator & m)
{
	pWChar = m.pWChar;
	pBlock = m.pBlock;
	nIndex = m.nIndex;
	pLine = m.pLine;
	bAfter_end = m.bAfter_end;
}


Line_iterator & Line_iterator::operator++()
{
	//��ֹԽ�紦��
	if (bAfter_end)
		return *this;
	if (nIndex%BLOCK_SIZE == 0)
	{
		pBlock = pBlock->pNextBlock;
		pWChar = pBlock->Strings;

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
	/*
	��ֹ����Խ�紦��
	...
	*/
	if (bAfter_end)
	{
		bAfter_end = FALSE;
		return *this;
	}
	if (nIndex%BLOCK_SIZE == 1)
	{
		DataBlock* p = pLine->pLineHead;
		while (p != pBlock && p->pNextBlock != pBlock)
		{
			p = p->pNextBlock;
		}
		pWChar = p->Strings;
		pBlock = p;
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
	bAfter_end = m.bAfter_end;
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
	if (theLine.pLineHead != NULL && index > 0)
	{
		pWChar = theLine.pLineHead->Strings;		//ָ�������ַ�
		pBlock = theLine.pLineHead;
		nIndex = 1;
		bAfter_end = false;
		*this = *this + (index - 1);
	}
	else
	{
		pWChar = NULL;
		pBlock = NULL;
		nIndex = 0;
		bAfter_end = TRUE;
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

Line_iterator  Line_iterator::operator+(int n)
{
	Line_iterator temp(*this);
	while (n)
	{
		++temp;
		--n;
	}
	return temp;
}

Line_iterator  Line_iterator::operator-(int n)
{
	Line_iterator temp(*this);
	while (n)
	{
		--(temp);
		--n;
	}
	return temp;
}
/*
���ص�������ָλ�õ��ַ�����
ע�⣺���ڴ��ڿ��е�����£�ʹ��ʱӦ����֤pWChar��Ч
*/
TCHAR& Line_iterator::operator*()
{
	return pWChar[(nIndex + BLOCK_SIZE - 1) % BLOCK_SIZE];
}
/*
��Ҫ����*����ǰ�ж�
����ֵ��
true		��ǰ������ָ����Ч�ַ�
false		��ǰ������δ�󶨻�ǰ��Ϊ���л���ָ��β��
*/
bool Line_iterator::isValid() const
{
	if (pWChar == NULL || bAfter_end == TRUE)
		return false;
	else
		return true;
}

bool Line_iterator::isEnd()
{
	if (nIndex == pLine->nDataSize)
		return true;
	else
		return false;
}

bool Line_iterator::operator==(const Line_iterator & m)
{
	return	pBlock == m.pBlock&&nIndex == m.nIndex&&bAfter_end == m.bAfter_end;
}

bool Line_iterator::operator<(const Line_iterator & m)
{
	if (pLine->nLineNumber < m.pLine->nLineNumber)
		return true;
	else if (pLine->nLineNumber == m.pLine->nLineNumber)
	{
		if (nIndex + bAfter_end < m.nIndex + m.bAfter_end)
			return true;
		else
			return false;
	}
	else
		return false;
}

bool Line_iterator::operator<=(const Line_iterator & m)
{
	if (*this < m)
		return true;
	if (pLine->nLineNumber == m.pLine->nLineNumber&&nIndex + bAfter_end == m.nIndex + m.bAfter_end)
		return true;
	else
		return false;
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

/*
��[first,last]�����ݿ�������ʼλ��Ϊstart�ĵط�
����ֵΪ���������һ���ַ�λ��
*/
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

//�ж��Ƿ�Ϊ���з�
bool isChange_Line_Character(std::wstring Str)
{
	return Str.empty();
}

//P1<P2 ��ʾp1 �� P2֮ǰ 
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
