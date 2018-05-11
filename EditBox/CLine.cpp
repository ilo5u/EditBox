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

//创建当前行(一般用于初始化之后)
void CLine::CreateLine(std::wstring & String)
{
	if (String.empty())			//回车 直接换行 此行为空
	{
		bBlankLine = true;
		nDataSize = 0;
		pLineHead = NULL;
		nBlocks = 0;
		return;
	}
	//非空行
	bBlankLine = false;
	if (pLineHead != NULL)
		ClearLine();
	nDataSize = String.size();
	//当前行所需数据块数量
	nBlocks = nDataSize / BLOCK_SIZE;
	if (nDataSize%BLOCK_SIZE != 0)
		nBlocks = nBlocks + 1;
	pLineHead = new DataBlock;
	DataBlock* p = pLineHead;
	//依次创建nBlocks个块
	std::wstring::iterator Iterator = String.begin();
	for (int i = 1; i <= nBlocks; i++)
	{
		//对单个块输入数据
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

//清空当前行所有数据块
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
		std::wcout.imbue(std::locale(std::locale(), "", LC_CTYPE));		//切换本地语言输出中文
																		//	printf("行号:%d,数据块数量:%d,字符个数:%d,内容:", nLineNumber, nBlocks, nDataSize);
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
删除该行[first,last]之间的字符
*/
Line_iterator CLine::DeleteLine(int first, int last)
{
	if (bBlankLine || first > last || first > nDataSize || last == 0)	//空行或无效的操作
		return Line_iterator(*this);
	if (first == 1 && last == nDataSize)		//删除整行
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
在position处按退格键
返回 false 表示需要换行 抛出false给外部调用函数处理
返回 true 表示正确删除字符
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
在start后面插入字符串 ：
eg		abcdef    insert "123"  start=3
------->abc123def
start==0			在行首添加字符串
start==nDataSize	在行末添加字符
start==x			在x位置后面插入字符串
返回插入的第一位字符位置
return Line_iterator(4)
*/
Line_iterator CLine::InsertStrings(int start, std::wstring String)
{
	if (String.empty())			//若传入字符串为空，认为是插入换行符
	{
		if (start == 0)			//换行
		{
			CLine* p = new CLine(*this);		//创建副本
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
		return end();
	}
	//在空行插入
	if (bBlankLine)
	{
		CreateLine(String);
		bBlankLine = false;
		return begin();
	}
	Line_iterator it_start(*this, start);
	Line_iterator it_end = this->end();
	int Size_to_add = String.size();
	if (start == nDataSize)		//直接在行尾增加字符
	{
		//若当前行为空行
		if (bBlankLine)
		{
			CreateLine(String);
			return begin();
		}
		//先将当前块补满后再添加新的strings
		int n = nBlocks * BLOCK_SIZE - it_start.nIndex;
		while (n > 0 && (!String.empty()))
		{
			wchar_t wch = String[0];
			String.erase(String.begin());
			++it_start;
			*it_start = wch;
			n--;
		}
		//若此时String仍非空，则新建行插入到本行末尾
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
	else if (start == 0)	//在行首插入
	{
		Line_iterator it_start = begin();
		CLine* pnewLine = new CLine(nLineNumber);
		pnewLine->CreateLine(String);
		//衔接
		Line_iterator NewLineEnd = pnewLine->end();
		NewLineEnd.pBlock->pNextBlock = pLineHead;
		pLineHead = pnewLine->pLineHead;
		pnewLine->pLineHead = NULL;
		nBlocks += pnewLine->nBlocks;
		//移动
		int Size_to_move = pnewLine->nBlocks*BLOCK_SIZE - pnewLine->nDataSize;
		if (Size_to_move != 0)			//需要补上空位
		{
			NewLineEnd = copy(NewLineEnd + 1, it_start, it_end);
			*(NewLineEnd + 1) = L'\0';
			if (NewLineEnd.pBlock->pNextBlock != NULL)		//移动之后有冗余数据块
			{
				DeleteSpareBlocks(NewLineEnd.pBlock->pNextBlock);
				NewLineEnd.pBlock->pNextBlock = NULL;
			}
		}
	}
	else     //在之间插入
	{
		std::wstring backstr = TransformToWString(it_start.nIndex + 1, nDataSize);		//保存start后面的字符串
																						//删除这部分
		nDataSize -= backstr.size();
		DeleteSpareBlocks(it_start.pBlock->pNextBlock);
		it_start.pBlock->pNextBlock = NULL;
		//重新连接
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
	Line_iterator It = begin() + max((nDataSize - 1), 0);
	return It;
}
/*
[first,last]内的字符转化为wstring形式
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
//返回当前行行宽[1-nDatasize/end]（像素） 第三个参数可选 
//end == 0 返回行宽 end != 0 返回[1-end]的宽度
int CLine::Line_Width(int Width,int end)
{
	if (bBlankLine)
		return 0;
	int n =( end == 0 ? nDataSize : min(end, nDataSize));
	int Width_EN = Width;
	int Width_ZH = Width * 2;
	int	Total_Width = 0;
	Line_iterator iterator(*this);
	for (int i = 1; i <= n; i++)
	{
		short int m = *iterator;		//得到字符的编码值
		if (WORD(m >> 8) > 0)		//中文
			Total_Width += Width_ZH;
		else                       //英文
			Total_Width += Width_EN;
		++iterator;
	}
	return Total_Width;
}

//删除多余的数据块
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

//更新行号
void CLine::UpDataLineNumber()
{
	CLine* pLine = pNextLine;
	int n = nLineNumber + 1;
	while (pLine != NULL)
	{
		pLine->nLineNumber = n;
		n++;
		pLine
			= pLine->pNextLine;
	}
}




Line_iterator::Line_iterator(CLine & theLine, int index) :pLine(&theLine)
{
	if (theLine.pLineHead != NULL)					//不为空行的情况
	{
		pWChar = theLine.pLineHead->Strings;		//指向行首字符
		pBlock = theLine.pLineHead;
		nIndex = 1;
		if (index)
			*this = *this + (index - 1);
		else
			nIndex = 0;		//等于0说明此行为空
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
/*更改行迭代器当前绑定的行*/
void Line_iterator::Set(CLine & theLine, int index)
{
	pLine = &theLine;
	if (theLine.pLineHead != NULL)
	{
		pWChar = theLine.pLineHead->Strings;		//指向行首字符
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
//返回当前迭代器指向的字符所在位置
int Line_iterator::CurrentPosition() const
{
	return nIndex;
}
//返回当前行指针
CLine * Line_iterator::GetLinePointer()
{
	return pLine;
}

Line_iterator  Line_iterator::operator+(int n)
{
	Line_iterator temp(*this);
	int BeginIndex = temp.nIndex % BLOCK_SIZE - 1;		//当前所在块中下标
	int Blocks_to_add = n / BLOCK_SIZE;
	int Remainder = n % BLOCK_SIZE;					//相对偏移量
	temp.nIndex += Blocks_to_add * BLOCK_SIZE;
	while (Blocks_to_add)
	{
		temp.pBlock = temp.pBlock->pNextBlock;
		Blocks_to_add--;
	}
	pWChar = &(pBlock->Strings[BeginIndex]);
	while (Remainder)
	{
		++(temp);
		--Remainder;
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
返回迭代器所指位置的字符引用
注意：鉴于存在空行的情况下，使用时应当保证pWChar有效
*/
TCHAR& Line_iterator::operator*()
{
	return *pWChar;
}
/*
主要用于*访问前判断
返回值：
true		当前迭代器指向有效字符
false		当前迭代器未绑定或当前行为空行
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
	return	pBlock == m.pBlock&&nIndex == m.nIndex;		//若指向的字符位于同一区域块的同一位置 则相等
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

//P1<P2 表示p1 在 P2之前 
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
