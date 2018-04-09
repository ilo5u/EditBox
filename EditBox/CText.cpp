#include "stdafx.h"

CText::CText()
{
	pFirstLineHead = NULL;
	nLineNumbers = 0;
	FileName = "";
}

CText::~CText()
{
	ClearAll();
}
/*�½��ı� ����Ĭ���ļ���*/
void CText::NewFile()
{
	if (pFirstLineHead != NULL)
		ClearAll();
	pFirstLineHead = new CLine(1);
	nLineNumbers = 1;
	FileName = "";		//Ĭ���ļ�����������
	bSave = 1;
}
//�����ļ���·�������׺
void CText::ReadText(std::string filename)
{
	if (pFirstLineHead != NULL)
		ClearAll();
	FileName = filename;
	std::ifstream	File_r(FileName, std::wifstream::in);		//�Զ��ķ�ʽ��txt�ļ�
	std::string LineStr;
	std::wstring LineWStr;
	CLine* p = NULL;
	while (std::getline(File_r, LineStr))
	{
		nLineNumbers++;
		if (nLineNumbers == 1)
		{
			if (LineStr.empty())		//���ļ�
			{
				File_r.close();
				return;		
			}
			LineWStr = StringToWString(LineStr);
			pFirstLineHead = new CLine(nLineNumbers);
			pFirstLineHead->CreateLine(LineWStr);
			p = pFirstLineHead;
		}
		else
		{
			LineWStr = StringToWString(LineStr);
			p->pNextLine = new CLine(nLineNumbers);
			p = p->pNextLine;
			p->CreateLine(LineWStr);
		}

	}
	File_r.close();
	bSave = 1;
}
/*
����ı� 
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
	pFirstLineHead = NULL;
	nLineNumbers = 0;
	bSave = 1;
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
		std::cout << "���ļ�" << std::endl;
	}
}
/*
ɾ���кŷ�Χ��[first,last]�ڵ�������
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
��ɾ��
*/
void CText::Delete(Position first, Position last)
{
	bSave = 0;
	//ɾ���м�������
	if (last.LineNumber - first.LineNumber > 1)
		DeleteLines(first.LineNumber + 1, last.LineNumber - 1);
	//ɾ��ʣ�ಿ��	���ȶ�λ��ʼ��λ��
	CLine* p = GetLinePointer(first.LineNumber);
	//first lastλ����������
	if (first.LineNumber < last.LineNumber)
	{
		p->DeleteLine(first.Sequence, p->nDataSize);
		CLine* pNext = p->pNextLine;
		std::wstring Str = pNext->TransformToWString(last.Sequence + 1, pNext->nDataSize);
		DeleteLines(pNext->nLineNumber, pNext->nLineNumber);
		p->InsertStrings(first.Sequence - 1, Str);
	}
	//first lastλ��ͬһ��
	else
	{
		p->DeleteLine(first.Sequence, last.Sequence);
	}

}
/*
�˸��
�������Ϊ����ɾ����Ԫ��        abcd|ef	��d��λ��
�����˸����ǰ��Ԫ��λ��      abc|ef  ������c��λ��
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
	//�軻��
	if (!(p->BackSpace(position)))
	{
		std::wstring LineStr = p->TransformToWString(1, p->nDataSize);
		DeleteLines(p->nLineNumber, p->nLineNumber);
		preLine->InsertStrings(preLine->nDataSize, LineStr);
		return { position.LineNumber - 1,preLine->nDataSize + 1 };
	}
	else
	{
		return { position.LineNumber,position.Sequence - 1 };
	}
}
/*
��start��ָλ�ú����
����ֵΪ��������һ���ַ�λ��
���£�
�޸��˲�����ڸ���������������
*/
Position CText::Insert(Position start, Position first, Position last)
{
	bSave = 0;
	CLine* p = GetLinePointer(first.LineNumber);
	//���ƶ���ֻ��һ��
	if (first.LineNumber == last.LineNumber)
	{
		std::wstring Str = p->TransformToWString(first.Sequence, last.Sequence);
		p->InsertStrings(start.Sequence, Str);
		return { start.LineNumber,start.Sequence+int(Str.size())};
	}
	else
	{
		//Ԥ����
		CLine* pStart = GetLinePointer(start.LineNumber);
		std::wstring BackStr = pStart->TransformToWString(start.Sequence + 1, pStart->nDataSize);		//�����������ַ���
		std::wstring Str = p->TransformToWString(first.Sequence, p->nDataSize);
		//�ض�(�ڲ�����ڸ��Ʒ�Χ֮��ʱ�貹�䱻�ضϵ��ַ���ֵ)
		bool bAmong = false;
		//�����õ��ı���
			int L = start.LineNumber;	
			std::wstring SaveStr;
		if (start.LineNumber > first.LineNumber&&start < last)
		{
			bAmong = true;
			SaveStr = pStart->TransformToWString(start.Sequence + 1, pStart->nDataSize);
		}
		pStart->DeleteLine(start.Sequence + 1, pStart->nDataSize);									
		pStart->InsertStrings(start.Sequence, Str);
		int n = first.LineNumber + 1;
		//���εĲ���
		while (n <= last.LineNumber)
		{
			p = p->pNextLine;
			InsertLine(pStart->nLineNumber);
			pStart = pStart->pNextLine;
			if(n==last.LineNumber)
				Str = p->TransformToWString(1, last.Sequence);
			else
				Str = p->TransformToWString(1, p->nDataSize);
			//bAmong ����������Ĵ���
			if (bAmong)
			{
				if (L == n)
				{
					L = BackStr.size();
					while (L--)
						Str.pop_back();
					Str += SaveStr;
					bAmong = false;
				}
			}
			pStart->CreateLine(Str);
			n++;
		}
		pStart->InsertStrings(pStart->nDataSize, BackStr);
		return { pStart->nLineNumber,pStart->nDataSize - int(BackStr.size()) };
	}
}
/*
��start ���� �����ַ���
���ز���ĵ�һ���ַ�λ��
eg.
22		abcdef    insert "123"  start={22,3}
------->abc123def	
return  {22,4}
*/
Position CText::Insert(Position start, std::wstring String)
{
	bSave = 0;
	CLine* p = GetLinePointer(start.LineNumber);
	if (p == NULL)
		throw std::invalid_argument("error position ");
	if (p->bBlankLine) 
	{
		p->CreateLine(String);
		return { start.LineNumber,1 };
	}
	else
	{
		p->InsertStrings(start.Sequence, String);
		return { start.LineNumber,start.Sequence + 1 };
	}
}
/*
��position�����ַ����»س���
������� position.Sequence==0��ʾ�ڱ����ϼ�һ��
eg.	1	abc|def  {1,3}
--> 1   abc
	2   |def
	return {2,1}
	�����������ַ�λ�� (������Ϊ0)
*/
Position CText::EnterNewLine(Position position)
{
	bSave = 0;
	CLine* p = GetLinePointer(position.LineNumber);		
	//�������
	if (position.Sequence == 0)
	{
		InsertLine(position.LineNumber - 1);
		return { position.LineNumber + 1,1 };
	}

	InsertLine(position.LineNumber);
	if (p->bBlankLine || p->nDataSize == position.Sequence)		//��������
	{
		return { position.LineNumber + 1,0 };
	}
	else
	{
		std::wstring Str = p->TransformToWString(position.Sequence, p->nDataSize);
		p->DeleteLine(position.Sequence + 1, p->nDataSize);
		p = p->pNextLine;
		p->CreateLine(Str);
		return { position.LineNumber + 1,1 };
	}
}

std::queue<Position> CText::SeekStrings(std::wstring Str, Position start)
{
	//ģʽƥ��Ԥ����
	int* pNext = GetNextValArray(Str);
	int nSize = Str.size();
	Text_iterator iterator(*this);
	Text_iterator TextEnd(*this);
	iterator.GoPosition(start);
	TextEnd.GoEnd();
	std::queue<Position>	Container;		//�洢����
	//ƥ��
	for (int j=-1; iterator != TextEnd; )
	{
		if (j == nSize)				//�ɹ���һ��ƥ��
		{
			Container.push((iterator - nSize).GetCurPositin());
			j = -1;
		}
		if (j == -1 || *iterator == Str[j])
		{
			++j;
			++iterator;
		}
		else
			j = pNext[j];
	}
	delete pNext;
	return Container;
}
//���� �������ļ�������������������ı��İ��ļ��� �������ݱ��浽�ı���
void CText::Save(std::string filename, bool isChange)
{
	if (!filename.empty()&&isChange==true)
		FileName = filename;
	std::fstream wFile(filename, std::fstream::out);
	std::wstring wString;		//���ڴ��л�ȡ�Ŀ��ַ�
	std::string String;			//д���ļ��Ķ��ַ�
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
//���������Ļ�ϱ��������ַ���ռ��� ����WidthΪ��׼��λ���
int CText::Line_Width(int LineNumber, int Width)
{
	CLine* p = GetLinePointer(LineNumber);
	return p->Line_Width(Width);
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
/*���ص�ǰ������*/
int CText::Line_Number() const
{
	return nLineNumbers;
}


/*
�����к� 
��pָ����ָ�к�ΪStart��ʼ
*/
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
/*
�������
�ڴ�������к�֮��������
ע��AfterLineNumber==0 ��������ǰ�������
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
		while (p != NULL&&p->nLineNumber!=AfterLineNumber)
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
//��ȡ����ָ��
CLine * CText::GetLinePointer(int LineNumber)
{
	CLine* p = pFirstLineHead;
	if (p == NULL)
		return NULL;
	while (p->nLineNumber != LineNumber)
		p = p->pNextLine;
	return p;
}

Text_iterator::Text_iterator(CText & Text, int LineNumber, int position)
{
	pText = &Text;		//���ı�
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
	CLine* p = currLine.GetLinePointer();			//��ǰ��ָ��
	int position = currLine.CurrentPosition();		//��ǰλ��
	int CurLineNumber = p->nLineNumber;				//��ȡ��ǰ�к�
	if (position == p->nDataSize)					//��Ҫ����
	{
		if (CurLineNumber != pText->Line_Number())	
		{
			p = p->pNextLine;
			currLine.Set(*p);						//������һ������
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
	CLine* p = currLine.GetLinePointer();			//��ǰ��ָ��
	int position = currLine.CurrentPosition();		//��ǰλ��
	int CurLineNumber = p->nLineNumber;				//��ȡ��ǰ�к�
	if (position == 1 || position == 0)
	{
		if (CurLineNumber != 1)
		{
			p = pText->GetLinePointer(CurLineNumber - 1);
			currLine.Set(*p, p->nDataSize);		//�ص���һ����β
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
Text_iterator & Text_iterator::operator+(int n)
{
	while (n)
	{
		++(*this);
		--n;
	}
	return *this;
}
Text_iterator & Text_iterator::operator-(int n)
{
	while (n)
	{
		--(*this);
		--n;
	}
	return *this;
}
bool Text_iterator::operator==(const Text_iterator & Text)
{
	return currLine == Text.currLine;
}
bool Text_iterator::operator!=(const Text_iterator & Text)
{
	return currLine != Text.currLine;
}
//�жϵ�ǰ�ı��������Ƿ�����ĩ
bool Text_iterator::isEnd()
{
	int CurLineNumber = currLine.GetLinePointer()->nLineNumber;
	int position = currLine.CurrentPosition();
	CLine* pLastLine = pText->GetLinePointer(pText->Line_Number());
	//��ĩ�е����һ���ַ���
	return (CurLineNumber == pLastLine->nLineNumber) && (position == pLastLine->nDataSize);
}
//��λ�ı���������ĳλ��
void Text_iterator::GoPosition(Position position)
{
	CLine* pLine = pText->GetLinePointer(position.LineNumber);
	currLine.Set(*pLine, position.Sequence);
}
//�ı���������λ���ı�ĩ
void Text_iterator::GoEnd()
{
	CLine* pLastLine = pText->GetLinePointer(pText->Line_Number());
	int nRear = pLastLine->nDataSize;
	currLine.Set(*pLastLine, nRear);
}
//���ص�ǰ���������ı��е�λ��
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

/*KMP�㷨�е�Next����*/
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
		if (WORD(m >> 8) > 0)			//�ж�������
			n++;
	}
	return n;
}

//ANSI������
/*ʵ�ֽ�charת��Ϊwchar_t*/
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
//ANSI������
/*ʵ�ֽ�wchar_tת��Ϊchar*/
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
