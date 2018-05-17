#include"stdafx.h"

CText::CText()
{
	pFirstLineHead = NULL;
	nLineNumbers = 1;
	FileName = "";
	hAuto_Save = NULL;
}

CText::~CText()
{
	ClearAll();
	delete pFirstLineHead;
}
/*新建文本 保存默认文件名*/
void CText::NewFile()
{
	ClearAll();
	nLineNumbers = 1;
	/*待修正  增设默认文件名 */
	FileName = "";		//默认文件名及其类型
	bSave = 1;
	 // hAuto_Save = (HANDLE)_beginthreadex(NULL, 0, Auto_Save_Timer_Thread, NULL, 0, NULL);
}
//传递文件名路径及其后缀
void CText::ReadText(std::string filename)
{
	ClearAll();
	// hAuto_Save = (HANDLE)_beginthreadex(NULL, 0, Auto_Save_Timer_Thread, NULL, 0, NULL);
	FileName = filename;
	std::ifstream	File_r(FileName, std::wifstream::in);		//以读的方式打开txt文件
	/*读取文件失败*/
	if (!File_r)
		throw Read_Text_Failed("找不到文件");
	std::string LineStr;
	std::wstring LineWStr;
	CLine* p = NULL;
	nLineNumbers = 0;
	while (std::getline(File_r, LineStr))
	{
		nLineNumbers++;
		LineWStr = StringToWString(LineStr);
		if (nLineNumbers == 1)
		{
			pFirstLineHead->CreateLine(LineWStr);
			p = pFirstLineHead;
		}
		else
		{
			p->pNextLine = new CLine(nLineNumbers);
			p = p->pNextLine;
			p->CreateLine(LineWStr);
		}
	}
	File_r.close();
	bSave = 1;
}
/*
清空文本和定时器
*/
void CText::ClearAll()
{
	while (pFirstLineHead != NULL)
	{
		CLine* temp = pFirstLineHead;
		pFirstLineHead = pFirstLineHead->pNextLine;
		delete temp;
	}
	FileName = "";
	pFirstLineHead = new CLine(1);
	nLineNumbers = 1;
	bSave = 0;
	CloseHandle(hAuto_Save);
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
}
/*
删除行号范围在[first,last]内的所有行
*/
void CText::DeleteLines(int first, int last)
{
	if (first > last || last > nLineNumbers)
		return;
	bSave = 0;
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
		CLine* p = GetLinePointer(first - 1);
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
/*
块删除
返回first前一个位置
*/
Position CText::Delete(Position first, Position last)
{
	bSave = 0;
	//删除中间完整行
	if (last.LineNumber - first.LineNumber > 1)
		DeleteLines(first.LineNumber + 1, last.LineNumber - 1);
	//删除剩余部分	首先定位起始行位置
	CLine* p = GetLinePointer(first.LineNumber);
	//first last位于相邻两行
	if (first.LineNumber < last.LineNumber)
	{
		p->DeleteLine(first.Sequence, p->nDataSize);
		CLine* pNext = p->pNextLine;
		std::wstring Str = pNext->TransformToWString(last.Sequence + 1, pNext->nDataSize);
		DeleteLines(pNext->nLineNumber, pNext->nLineNumber);
		if (!Str.empty())
			p->InsertStrings(first.Sequence - 1, Str);
	}
	//first last位于同一行
	else
	{
		p->DeleteLine(first.Sequence, last.Sequence);
	}
	return { first.LineNumber,first.Sequence - 1 };
}
/*
退格键
传入参数为将被删除的元素        abcd|ef	即d的位置
返回退格后光标前的元素位置      abc|ef  即返回c的位置
*/
Position CText::BackSpace(Position position)
{
	bSave = 0;
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
		if(!LineStr.empty())
			preLine->InsertStrings(preLine->nDataSize, LineStr);
		return { position.LineNumber - 1,preLine->nDataSize  };
	}
	else
	{
		return { position.LineNumber,position.Sequence - 1 };
	}
}

/*
在start 后面 插入字符串(含有换行符)
返回插入的最后一个字符位置
eg.
22		abcdef    insert "123"  start={22,3}
------->abc123def
return  {22,6}
*/
Position CText::Insert(Position start, std::wstring String)
{
	bSave = 0;
	std::queue<std::wstring> dq = WStrToLineWStr(String);
	CLine* p = GetLinePointer(start.LineNumber);
	if (p == NULL)
		throw std::invalid_argument("位置传入错误");
	int n = start.LineNumber;
	int prenumbers = 0;
	if (!dq.empty())					//对首行的特殊处理
	{
		String = dq.front();
		dq.pop();
		p->InsertStrings(start.Sequence, String);

		if (String.size() != 0 && !dq.empty())
			EnterNewLine({ n, start.Sequence + (int)String.size() });
		if (!dq.empty())
			n++;
		else
			prenumbers = start.Sequence;
		p = p->pNextLine;
	}
	while (dq.size() > 1)
	{
		String = dq.front();
		dq.pop();

		p->InsertStrings(0, String);

		if (String.size() != 0)
			EnterNewLine({ n, (int)String.size() });

		n++;
		p = p->pNextLine;
	}
	if (dq.size() == 1)
	{
		String = dq.front();
		dq.pop();
		p->InsertStrings(0, String);
	}
	return { n,(int)(String.size() + prenumbers) };
}
/*拷贝Position在[start ,end]之间的字符串 保存在wstring中 并加上换行符*/
std::wstring CText::Copy(Position start, Position end)
{
	std::wstring Str;
	std::wstring LineStr;
	TCHAR Flag = L'\n';			//换行符
	CLine* p = GetLinePointer(start.LineNumber);
	while (p != NULL && p->nLineNumber < end.LineNumber)
	{
		if (p->nLineNumber == start.LineNumber)
			LineStr = p->TransformToWString(start.Sequence, p->nDataSize);
		else
			LineStr = p->TransformToWString(1, p->nDataSize);
		LineStr.push_back(Flag);
		Str += LineStr;
		p = p->pNextLine;
	}
	if (p == NULL)
		throw std::invalid_argument("拷贝：错误的结束位置");
	if (p->nLineNumber != start.LineNumber)
		LineStr = p->TransformToWString(1, end.Sequence);
	else
		LineStr = p->TransformToWString(start.Sequence, end.Sequence);
	
	Str += LineStr;
	return Str;
}
/*
在position处的字符 后 按下回车键
特殊情况 position.Sequence==0表示在本行上加一行
eg.	1	abc|def  {1,3}
--> 1   abc
2   |def
return {2,1}
返回新行首字符位置 (若无则为0)
*/
Position CText::EnterNewLine(Position position)
{
	bSave = 0;
	CLine* p = GetLinePointer(position.LineNumber);
	//特殊情况
	if (position.Sequence == 0)
	{
		InsertLine(position.LineNumber - 1);
		return { position.LineNumber + 1,1 };
	}

	InsertLine(position.LineNumber);
	if (p->bBlankLine || p->nDataSize == position.Sequence)		//新增空行
	{
		return { position.LineNumber + 1,0 };
	}
	else
	{
		std::wstring Str = p->TransformToWString(position.Sequence + 1, p->nDataSize);
		p->DeleteLine(position.Sequence + 1, p->nDataSize);
		p = p->pNextLine;
		p->CreateLine(Str);
		return { position.LineNumber + 1,1 };
	}
}

/*
查找字符串 返回值表示是否查找到 将该字符串的起点和终点赋值给参数

提供的功能：
1 全字符匹配
2 区分大小写下的全字符匹配
3 不区分大小写下的全字符匹配
4 向前查找
5 向后查找
传入参数start用于指明开始位置 最终被赋值为查找到的字符串第一个字符位置
传入参数end用于指明结束位置 最终被赋值为查找到的字符串最后一个字符位置
参数 upper_lower 取值：
true	区分大小写（默认情况）
false	不区分大小写
*/
bool CText::SeekStrings(std::wstring Str, Position& start, Position& end, bool upper_lower)
{
	//模式匹配预处理
	int* pNext = GetNextValArray(Str);
	int nSize = Str.size();
	Text_iterator iterator(*this);
	Text_iterator TextEnd(*this);
	iterator.GoPosition(start);
	TextEnd.GoPosition(end);
	//匹配
	int j = 0;
	while (iterator <= TextEnd)
	{
		if (j == -1 || upper_lower_match(*iterator, Str[j], upper_lower))
		{
			++j;
			++iterator;
		}
		else
			j = pNext[j];
		if (j == nSize)				//成功的一次匹配
		{

			start = (iterator - nSize).GetCurPositin();
			end = (iterator - 1).GetCurPositin();
			j = -1;
			delete pNext;
			return true;
		}
	}

	delete pNext;
	return false;
}

/*
将Position[start,end]之间的字符串替换为Str
返回值：替换元素最后一个字符的位置
*/
Position CText::Replace(Position start, Position end, std::wstring Str)
{
	if (start > end || start.Sequence == 0)
		throw std::invalid_argument("传入错误的位置");
	return Insert(Delete(start, end), Str);
}
//保存 若传入文件名参数，则更改内置文本的绑定文件名 否则将内容保存到文本中
void CText::Save(std::string filename, bool isChange)
{
	if (!filename.empty() && isChange == true)
		FileName = filename;
	std::fstream wFile(filename, std::fstream::out);
	std::wstring wString;		//从内存中获取的宽字符
	std::string String;			//写入文件的短字符
	CLine* p = pFirstLineHead;
	while (p != NULL)
	{
		wString = p->TransformToWString(1, p->nDataSize);
		String = WStringToString(wString);
		wFile << String << std::endl;
		p = p->pNextLine;
	}
	bSave = 1;
	wFile.close();
}

bool CText::isSaved()
{
	return bSave;
}

std::string CText::FilePath()
{
	if (FileName.empty())
		return "";
	else
		return	FileName;
}

int CText::Line_Size(int LineNumber)
{
	CLine* p = GetLinePointer(LineNumber);
	return p->nDataSize;
}
//返回输出屏幕上本行所有字符所占宽度 其中Width为标准单位宽度
//第三个参数可选 end==0 返回行宽 end != 0  返回当前行[1-end]宽度
int CText::Line_Width(int LineNumber, int Width, int end)
{
	CLine* p = GetLinePointer(LineNumber);
	return p->Line_Width(Width, end);
}
int CText::Max_Line_Width(int Width)
{
	int Max_Width = 0;
	CLine* p = pFirstLineHead;
	while (p != NULL)
	{
		int n = p->Line_Width(Width);
		Max_Width = max(Max_Width, n);
		p = p->pNextLine;
	}
	return Max_Width;
}
/*返回当前总行数*/
int CText::Line_Number() const
{
	return nLineNumbers;
}


/*
更新行号
以p指针所指行号为Start开始
*/
void CText::UpDataLineNumber(CLine * p, int Start)
{
	while (p != NULL)
	{
		p->SetLineNumber(Start);
		p = p->pNextLine;
		Start++;
	}
	nLineNumbers = max(1, Start - 1);
}
/*
插入空行
在传入参数行号之后插入空行
注：AfterLineNumber==0 则在首行前插入空行
*/
void CText::InsertLine(int AfterLineNumber)
{
	if (AfterLineNumber == 0)
	{
		CLine* pLine = new CLine(1);
		pLine->pNextLine = pFirstLineHead;
		pFirstLineHead = pLine;
		UpDataLineNumber(pFirstLineHead, 1);
	}
	else
	{
		CLine* p = pFirstLineHead;
		while (p != NULL && p->nLineNumber != AfterLineNumber)
		{
			p = p->pNextLine;
		}
		if (p != NULL)
		{
			CLine* pLine = new CLine(AfterLineNumber + 1);
			pLine->pNextLine = p->pNextLine;
			p->pNextLine = pLine;
			UpDataLineNumber(pLine, pLine->nLineNumber);
		}
	}
}

/*
判断两字符是否相当
参数3 upper_lower 取值：
true	区分大小写
false	不区分大小写
*/
bool CText::upper_lower_match(TCHAR ch1, TCHAR ch2, bool upper_lower)
{
	if ((iswalpha(ch1) && iswalpha(ch2)) && (upper_lower == false))			//两字符均为字母 且不区分大小写
	{
		return towlower(ch1) == towlower(ch2);
	}
	else
		return ch1 == ch2;
}
/*
UINT __stdcall CText::Auto_Save_Timer_Thread(LPVOID)
{
	while (true)
	{
		if (isSaved)
		{
			Start_Time = clock() / 1000;
			continue;
		}
		int Current_time = clock() / 1000;							//当前时间(s)
		if (Current_time - Start_Time >= AUTO_SAVE_TIME)
			Save();
	}
	return 0;
}*/

//获取行首指针
CLine * CText::GetLinePointer(int LineNumber)
{
	if (LineNumber > Line_Number())
		throw std::invalid_argument("传输行参数过大");
	CLine* p = pFirstLineHead;
	if (p == NULL)
		throw std::invalid_argument("未创建的文本");
	while (p->nLineNumber != LineNumber)
		p = p->pNextLine;
	return p;
}

Position CText::First_Position()
{
	if (pFirstLineHead != NULL)
		return { 1,1 };
	else
		return { 1,0 };
}

Position CText::End_Position()
{
	CLine* p = GetLinePointer(nLineNumbers);
	if (p != NULL)
		return { nLineNumbers,p->nDataSize };
	else
		return { 1,0 };
}

Text_iterator::Text_iterator(CText & Text, int LineNumber, int position)
{
	pText = &Text;		//绑定文本
	if (LineNumber <= Text.Line_Number())
	{
		CLine* p = Text.GetLinePointer(LineNumber);
		currLine.Set(*p, position);
	}
}

Text_iterator::~Text_iterator()
{
	pText = NULL;
}

TCHAR Text_iterator::operator*()
{
	if (currLine.isValid())
		return *currLine;
	else
		return L'\n';
}

Text_iterator & Text_iterator::operator++()
{
	CLine* p = currLine.GetLinePointer();			//当前行指针
	int position = currLine.CurrentPosition();		//当前位置
	int CurLineNumber = p->nLineNumber;				//获取当前行号
	if (position == p->nDataSize)					//需要换行
	{
		if (CurLineNumber != pText->Line_Number())
		{
			p = p->pNextLine;
			currLine.Set(*p);						//换入下一行行首
		}
	}
	else
		++currLine;
	return *this;
}

Text_iterator  Text_iterator::operator++(int)
{
	Text_iterator temp(*this);
	++(*this);
	return temp;
}

Text_iterator & Text_iterator::operator--()
{
	CLine* p = currLine.GetLinePointer();			//当前行指针
	int position = currLine.CurrentPosition();		//当前位置
	int CurLineNumber = p->nLineNumber;				//获取当前行号
	if (position == 1 || position == 0)
	{
		if (CurLineNumber != 1)
		{
			p = pText->GetLinePointer(CurLineNumber - 1);
			currLine.Set(*p, p->nDataSize);		//回到上一行行尾
		}
	}
	else
		--currLine;
	return *this;
}

Text_iterator Text_iterator::operator--(int)
{
	Text_iterator temp(*this);
	--(*this);
	return temp;
}
Text_iterator  Text_iterator::operator+(int n)
{
	Text_iterator temp(*this);
	while (n)
	{
		++temp;
		--n;
	}
	return temp;
}
Text_iterator  Text_iterator::operator-(int n)
{
	Text_iterator temp(*this);
	while (n)
	{
		--temp;
		--n;
	}
	return temp;
}
bool Text_iterator::operator==(const Text_iterator & Text)
{
	return currLine == Text.currLine;
}
bool Text_iterator::operator!=(const Text_iterator & Text)
{
	return currLine != Text.currLine;
}

bool Text_iterator::operator<(const Text_iterator & Text)
{
	if (currLine.pLine->nLineNumber < Text.currLine.pLine->nLineNumber)
		return true;
	if (currLine.nIndex < Text.currLine.nIndex)
		return true;
	return false;
}
bool Text_iterator::operator<=(const Text_iterator & Text)
{
	return (*this < Text) || (currLine.pLine->nLineNumber == Text.currLine.pLine->nLineNumber&&currLine.nIndex == Text.currLine.nIndex);
}

//判断当前文本迭代器是否到了文末
bool Text_iterator::isEnd()
{
	int CurLineNumber = currLine.GetLinePointer()->nLineNumber;
	int position = currLine.CurrentPosition();
	CLine* pLastLine = pText->GetLinePointer(pText->Line_Number());
	//在末行的最后一个字符处
	return (CurLineNumber == pLastLine->nLineNumber) && (position == pLastLine->nDataSize);
}
//定位文本迭代器于某位置
void Text_iterator::GoPosition(Position position)
{
	CLine* pLine = pText->GetLinePointer(position.LineNumber);
	currLine.Set(*pLine, position.Sequence);
}
//文本迭代器定位到文本末
void Text_iterator::GoEnd()
{
	CLine* pLastLine = pText->GetLinePointer(pText->Line_Number());
	int nRear = pLastLine->nDataSize;
	currLine.Set(*pLastLine, nRear);
}
//返回当前迭代器在文本中的位置
Position Text_iterator::GetCurPositin()
{
	int LineNumber = currLine.pLine->nLineNumber;
	int Sequence = currLine.nIndex;
	return { LineNumber,Sequence };
}

int Text_iterator::CurrentLineNumber()
{
	return currLine.pLine->nLineNumber;
}

/*KMP算法中的Next数组*/
int * GetNextValArray(std::wstring SubStr)
{
	int* pNextVal = new int[SubStr.size()];
	pNextVal[0] = -1;
	int j = 0;
	int k = -1;
	while (j < SubStr.size() - 1)
	{
		if (k == -1 || SubStr[j] == SubStr[k])
		{
			k++;
			j++;
			if (SubStr[j] != SubStr[k])
				pNextVal[j] = k;
			else
				pNextVal[j] = pNextVal[k];
		}
		else
			k = pNextVal[k];
	}
	return pNextVal;
}

size_t NumberOfZH(const std::wstring & wstr)
{
	size_t n = 0;
	for (auto wch : wstr)
	{
		short int m = wch;
		if (WORD(m >> 8) > 0)			//判断是中文
			n++;
	}
	return n;
}

//ANSI编码下
/*实现将char转化为wchar_t*/
std::wstring StringToWString(const std::string & s)
{
	std::string strLocale = setlocale(LC_ALL, "");
	const char* chSrc = s.c_str();
	size_t nDestSize = strnlen_s(chSrc, s.size()) + 1;
	wchar_t* wchDest = new wchar_t[nDestSize];
	wmemset(wchDest, 0, nDestSize);
	mbstowcs_s(&nDestSize, wchDest, nDestSize, chSrc, _TRUNCATE);
	std::wstring wstrResult = wchDest;
	delete[]wchDest;
	setlocale(LC_ALL, strLocale.c_str());
	return wstrResult;
}
//ANSI编码下
/*实现将wchar_t转化为char*/
std::string WStringToString(const std::wstring & ws)
{
	std::string strLocale = setlocale(LC_ALL, "");
	const wchar_t* wchSrc = ws.c_str();
	size_t need_to_add = NumberOfZH(ws);
	size_t nDestSize = wcsnlen_s(wchSrc, ws.size()) + need_to_add + 1;
	char *chDest = new char[nDestSize];
	memset(chDest, 0, nDestSize);
	wcstombs_s(&nDestSize, chDest, nDestSize, wchSrc, _TRUNCATE);
	std::string strResult = chDest;
	delete[]chDest;
	setlocale(LC_ALL, strLocale.c_str());
	return strResult;
}

std::string wchTostring(TCHAR * pwch)
{
	int Len = lstrlen(pwch);
	std::wstring WStr;
	for (int i = 0; i < Len; i++)
	{
		WStr.push_back(pwch[i]);
	}
	return WStringToString(WStr);
}

void WStringToWch(const std::wstring & ws, TCHAR* &pwch)
{
	int n = ws.size();
	for (int i = 0; i < n; i++)
	{
		pwch[i] = ws[i];
	}
}

std::queue<std::wstring> WStrToLineWStr(std::wstring WSTR)
{
	std::queue<std::wstring> dq;
	std::wstring Str;
	TCHAR Flag = L'\n';		//换行符
	for (TCHAR ch : WSTR)
	{
		if (ch == Flag)		//遇到行尾换行符 换下一行
		{
			dq.push(Str);	//注：Str为空说明次此行仅有换行符		
			Str.clear();	//清空
		}
		else
			Str.push_back(ch);
	}
	if (!Str.empty())
		dq.push(Str);
	return dq;
}
