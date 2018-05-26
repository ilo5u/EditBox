#include"stdafx.h"

std::string CText::Path = "C:\\MiniWord\\Default\\";
CText::CText()
{
	pFirstLineHead = NULL;
	nLineNumbers = 1;
	FileName = "";
	hAuto_Save = (HANDLE)_beginthreadex(NULL, 0, Auto_Save_Timer_Thread, (void*)this, 0, NULL);
}

CText::~CText()
{
	ClearAll();
	TerminateThread(hAuto_Save, 0);
	CloseHandle(hAuto_Save);
	delete pFirstLineHead;
}
/*�½��ı� ����Ĭ���ļ���*/
void CText::NewFile()
{
	ClearAll();
	nLineNumbers = 1;
	FileName = "";		//Ĭ���ļ�����������
	bSave = 1;
}
//�����ļ���·�������׺
void CText::ReadText(std::string filename)
{
	ClearAll();
	FileName = filename;
	std::ifstream	File_r(CText::Path + FileName, std::ifstream::in);		//�Զ��ķ�ʽ��txt�ļ�
																			/*��ȡ�ļ�ʧ��*/
	if (!File_r)
		throw Read_Text_Failed("�Ҳ����ļ�");
	std::string LineStr;
	std::wstring LineWStr;
	CLine* p = NULL;
	nLineNumbers = 0;
	while (std::getline(File_r, LineStr))
	{
		nLineNumbers++;
		LineWStr = StringToWString(LineStr);
		TabToSpace(LineWStr);
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
	nLineNumbers = max(1, nLineNumbers);
	File_r.close();
	bSave = 1;
}
/*
����ı��Ͷ�ʱ��
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
����firstǰһ��λ��
*/
Position CText::Delete(Position first, Position last)
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
		if (!Str.empty())
			p->InsertStrings(first.Sequence - 1, Str);
	}
	//first lastλ��ͬһ��
	else
	{
		p->DeleteLine(first.Sequence, last.Sequence);
	}
	return { first.LineNumber,first.Sequence - 1 };
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
		int n = preLine->nDataSize;
		if (!LineStr.empty())
			preLine->InsertStrings(preLine->nDataSize, LineStr);
		return { position.LineNumber - 1,n };
	}
	else
	{
		return { position.LineNumber,position.Sequence - 1 };
	}
}

/*
��start ���� �����ַ���(���л��з�)
���ز�������һ���ַ�λ��
�����к�
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
		throw std::invalid_argument("λ�ô������");
	int n = start.LineNumber;
	int prenumbers = 0;
	if (!dq.empty())					//�����е����⴦��
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
	UpDataLineNumber(pFirstLineHead, 1);
	return { n,(int)(String.size() + prenumbers) };
}
/*����Position��[start ,end]֮����ַ��� ������wstring�� �����ϻ��з�*/
std::wstring CText::Copy(Position start, Position end)
{
	std::wstring Str;
	std::wstring LineStr;
	TCHAR Flag = L'\n';			//���з�
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
		throw std::invalid_argument("����������Ľ���λ��");
	if (p->nLineNumber != start.LineNumber)
		LineStr = p->TransformToWString(1, end.Sequence);
	else
		LineStr = p->TransformToWString(start.Sequence, end.Sequence);

	Str += LineStr;
	return Str;
}
/*
��position�����ַ� �� ���»س���
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
		std::wstring Str = p->TransformToWString(position.Sequence + 1, p->nDataSize);
		p->DeleteLine(position.Sequence + 1, p->nDataSize);
		p = p->pNextLine;
		p->CreateLine(Str);
		return { position.LineNumber + 1,1 };
	}
}

/*
�����ַ��� ����ֵ��ʾ�Ƿ���ҵ� �����ַ����������յ㸳ֵ������

�ṩ�Ĺ��ܣ�
1 ȫ�ַ�ƥ��
2 ���ִ�Сд�µ�ȫ�ַ�ƥ��
3 �����ִ�Сд�µ�ȫ�ַ�ƥ��
4 ��ǰ����
5 ������
�������start����ָ����ʼλ�� ���ձ���ֵΪ���ҵ����ַ�����һ���ַ�λ��
�������end����ָ������λ�� ���ձ���ֵΪ���ҵ����ַ������һ���ַ�λ��
���� upper_lower ȡֵ��
true	���ִ�Сд��Ĭ�������
false	�����ִ�Сд
*/
bool CText::SeekStrings(std::wstring Str, Position& start, Position& end, bool upper_lower)
{
	//ģʽƥ��Ԥ����
	int* pNext = GetNextValArray(Str);
	int nSize = Str.size();
	Text_iterator iterator(*this);
	Text_iterator TextEnd(*this);
	iterator.GoPosition(start);
	TextEnd.GoPosition(end);
	//ƥ��
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
		if (j == nSize)				//�ɹ���һ��ƥ��
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

bool CText::ReSeekStrings(std::wstring Str, Position & start, Position & end, bool upper_lower)
{
	std::wstring R_Str(Str.rbegin(), Str.rend());			//������
															//ģʽƥ��Ԥ����
	int* pNext = GetNextValArray(R_Str);
	int nSize = R_Str.size();
	Text_iterator iterator(*this);
	Text_iterator TextEnd(*this);
	iterator.GoPosition(start);
	TextEnd.GoPosition(end);
	//ƥ��
	int j = 0;
	while (!(iterator < TextEnd))
	{
		if (j == -1 || upper_lower_match(*iterator, R_Str[j], upper_lower))
		{
			++j;
			--iterator;
		}
		else
			j = pNext[j];
		if (j == nSize)				//�ɹ���һ��ƥ��
		{

			start = (iterator + 1).GetCurPositin();
			end = (iterator + nSize).GetCurPositin();
			j = -1;
			delete pNext;
			return true;
		}
	}

	delete pNext;
	return false;
}

/*
��Position[start,end]֮����ַ����滻ΪStr
����ֵ���滻Ԫ�����һ���ַ���λ��
*/
Position CText::Replace(Position start, Position end, std::wstring Str)
{
	if (start > end || start.Sequence == 0)
		throw std::invalid_argument("��������λ��");
	return Insert(Delete(start, end), Str);
}
//���� �������ļ�������������������ı��İ��ļ��� �������ݱ��浽�ı���
void CText::Save()
{
	if (FileName.empty())
	{
		throw No_File_Name("��ǰ�ı�δ����");
	}
	std::fstream wFile(Path + FileName, std::fstream::out);
	std::wstring wString;					//���ڴ��л�ȡ�Ŀ��ַ�
	std::string String;						//д���ļ��Ķ��ַ�
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

void CText::SaveAs(std::string FullPath)
{
	std::fstream wFile(FullPath, std::fstream::out);
	CLine* p = pFirstLineHead;
	std::wstring wString;				//���ڴ��л�ȡ�Ŀ��ַ�
	std::string String;					//д���ļ��Ķ��ַ�
	while (p != NULL)
	{
		wString = p->TransformToWString(1, p->nDataSize);
		String = WStringToString(wString);
		wFile << String << std::endl;
		p = p->pNextLine;
	}
	wFile.close();
}

bool CText::isSaved()
{
	return bSave;
}

std::string CText::File_Name()
{
	return	FileName;
}


int CText::Line_Size(int LineNumber)
{
	CLine* p = GetLinePointer(LineNumber);
	return p->nDataSize;
}
//���������Ļ�ϱ��������ַ���ռ��� ����WidthΪ��׼��λ���
//������������ѡ end==0 �����п� end != 0  ���ص�ǰ��[1-end]���
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
/*���ص�ǰ������*/
int CText::Line_Number() const
{
	return nLineNumbers;
}

int CText::Characters(Position start, Position end)
{
	if (start.LineNumber == end.LineNumber)
		return end.Sequence - start.Sequence + 1;
	int Sum = 0;
	CLine* p = GetLinePointer(start.LineNumber);
	Sum += p->nDataSize - start.Sequence + 1;
	p = p->pNextLine;
	while (p->nLineNumber != end.LineNumber)
	{
		Sum += p->nLineNumber;
		p = p->pNextLine;
	}
	Sum += end.Sequence;
	return Sum;
}

int CText::All_Characters()
{
	return Characters(First_Position(), End_Position());
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
	nLineNumbers = max(1, Start - 1);
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
�ж����ַ��Ƿ��൱
����3 upper_lower ȡֵ��
true	���ִ�Сд
false	�����ִ�Сд
*/
bool CText::upper_lower_match(TCHAR ch1, TCHAR ch2, bool upper_lower)
{
	if ((iswalpha(ch1) && iswalpha(ch2)) && (upper_lower == false))			//���ַ���Ϊ��ĸ �Ҳ����ִ�Сд
	{
		return towlower(ch1) == towlower(ch2);
	}
	else
		return ch1 == ch2;
}

UINT __stdcall Auto_Save_Timer_Thread(LPVOID LP)
{
	CText* pText = (CText*)LP;
	while (true)
	{
		Sleep(1000 * 150);
		if (pText->isSaved())
		{
			pText->Start_Time = clock() / 1000;
			continue;
		}
		int Current_time = clock() / 1000;							//��ǰʱ��(s)
		if (Current_time - pText->Start_Time >= AUTO_SAVE_TIME && !pText->FileName.empty())
		{
			pText->Save();
			pText->Start_Time = clock() / 1000;
		}

	}
	return 0;
}

/*��Ĭ��·���ļ�������Ĭ���ı��ĵ��ļ���*/
std::string Generate_Default_File_Name(const std::string & Path)
{
	std::string FileName = "�½��ı��ĵ�";
	int n = 0;
	WIN32_FIND_DATAA findData;
	HANDLE hFindFile;
	std::string File_Style = Path + "*.txt";				//������ǰ�ļ���������txt�ļ�

	char* szFileName = (char*)File_Style.c_str();
	hFindFile = FindFirstFileA(szFileName, &findData);
	if (hFindFile != INVALID_HANDLE_VALUE)
	{
		do
		{
			n = max(n, Match_File_Name(std::string(findData.cFileName)));
		} while (FindNextFileA(hFindFile, &findData));

		FindClose(hFindFile);
	}
	if (n > 0)
		FileName = FileName + " (" + std::to_string(n) + ")";
	return FileName + ".txt";
}
/*���ص�ǰĬ���ļ���*/
int Match_File_Name(const std::string & FileName)
{
	if (FileName.size() < 12)
		return 0;
	std::string s("�½��ı��ĵ�.txt");
	if (FileName == s)
		return 1;
	if (std::string(FileName.begin(), FileName.begin() + 13) == std::string("�½��ı��ĵ� "))
	{
		auto l = std::find(FileName.begin(), FileName.end(), '(');
		auto r = std::find(FileName.begin(), FileName.end(), ')');
		if (l != FileName.end() && r != FileName.end())
		{
			return stoi(std::string(l + 1, r)) + 1;
		}
		else
			return 0;
	}
	return 0;
}

//��ȡ����ָ��
CLine * CText::GetLinePointer(int LineNumber)
{
	if (LineNumber > Line_Number())
		throw std::invalid_argument("�����в�������");
	CLine* p = pFirstLineHead;
	if (p == NULL)
		throw std::invalid_argument("δ�������ı�");
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

void CText::Set_File_Name(const std::string Name)
{
	FileName = Name;
}

void CText::Set_Path(const std::string path)
{
	CText::Path = path;
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
		else
			++currLine;								//�����ı�β��
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
		if (CurLineNumber > 1)
		{
			p = pText->GetLinePointer(CurLineNumber - 1);
			currLine.Set(*p, p->nDataSize);		//�ص���һ����β
		}
		else if (position == 1)
			--currLine;							//�ص��ı�ǰ
		
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
	return currLine < Text.currLine;
}
bool Text_iterator::operator<=(const Text_iterator & Text)
{
	return currLine <= Text.currLine;
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

std::queue<std::wstring> WStrToLineWStr(std::wstring WSTR)
{
	std::queue<std::wstring> dq;
	std::wstring Str;
	TCHAR Flag = L'\n';		//���з�
	for (TCHAR ch : WSTR)
	{
		if (ch == Flag)		//������β���з� ����һ��
		{
			dq.push(Str);	//ע��StrΪ��˵���δ��н��л��з�		
			Str.clear();	//���
		}
		else
			Str.push_back(ch);
	}
	if (!Str.empty())
		dq.push(Str);
	return dq;
}

void TabToSpace(std::wstring & Str)
{
	TCHAR tab = L'\t';
	if (Str.find(tab) == std::wstring::npos)		//�������Ʊ�� ����
		return;
	std::wstring Spaces;
	for (int i = 0; i < Str.size(); i++)
	{
		if (Str[i] == tab)
		{
			int n = TAB_SIZE - i % TAB_SIZE;
			std::wstring Spaces(n, L' ');
			Str.replace(i, 1, Spaces);
		}
	}
}
