#include "stdafx.h"
#include<stack>


Cursor* pCursor = NULL;
wchar_t* pTChar = NULL;
wchar_t* pBuffer = NULL;
size_t MaxBufferSize = 0;
std::stack<Record*>* pRecord = NULL;
RVALUE _stdcall UserMessageProc(HTEXT hText, int x, int y, UINT message, FPARAM fParam, SPARAM sParam)
{
	pCursor->SethText(hText);
	int Width_EN = Install::Width;			//�ַ���ȣ�Ӣ�ġ�������ŵ�λ��� ���֡���������ռ����Width��	
	int Width_ZH = 2 * Width_EN;			//����ģʽ�·��ſ��
	int Height = Install::Height;			//�ַ��߶�
	int LineNumber = y / Height + 1;
	//����������ı��·��հ�������кż���
	if (LineNumber > hText->Line_Number() && message != UM_TEXT)
	{
		LineNumber = hText->Line_Number();
		y = (LineNumber - 1) * Height;
	}
	//���y����
	y = y - y % Height;
	//����Ϣ�����Ĵ���
	switch (message)
	{
	case UM_UP:
	{
		if (LineNumber > 1)
		{
			y -= Height;
			int LastLineWidth = hText->Line_Width(LineNumber - 1, Width_EN);
			x = min(LastLineWidth, x);
			if (!(pCursor->isLegalCursor(LineNumber - 1, x)))			//���λ�úϷ��Լ�� ��ֹ�������ƶ�ʹ����ں����м���ʾ
			{
				x -= Width_EN;
			}
		}
		pCursor->ResetChoose();
		return RVALUE(y) << 32 | RVALUE(x);
	}
	case UM_DOWN:
	{
		if (LineNumber < hText->Line_Number())
		{
			y += Height;
			int NextLineWidth = hText->Line_Width(LineNumber + 1, Width_EN);
			x = min(NextLineWidth, x);
			if (!(pCursor->isLegalCursor(LineNumber + 1, x)))
			{
				x -= Width_EN;
			}
		}
		pCursor->ResetChoose();
		return RVALUE(y) << 32 | RVALUE(x);
	}
	case UM_LEFT:
	{
		int opt = pCursor->CharactersProperty_before_Cursor(LineNumber, x);
		switch (opt)
		{
			case 1:x -= Width_ZH; break;
			case -1:x -= Width_EN; break;
			default:
			{
				if (LineNumber != 1)
				{
					x = hText->Line_Width(LineNumber - 1, Width_EN);
					y -= Height;
				}
				break;
			}
		}
		pCursor->ResetChoose();
		return RVALUE(y) << 32 | RVALUE(x);
	}
	case UM_RIGHT:
	{
		int Size = pCursor->Characters_before_Cursor(LineNumber, x);
		if (Size == hText->Line_Size(LineNumber))
		{
			if (LineNumber != hText->Line_Number())		//��ͨ��β
			{
				y += Height;
				x = 0;
			}
		}
		else
		{
			if (pCursor->isEnAfter(LineNumber, x))
				x += Width_EN;
			else
				x += Width_ZH;
		}
		pCursor->ResetChoose();
		return RVALUE(y) << 32 | RVALUE(x);
	}
	case UM_END:			//��ǰ��βʱx y����
	{
		x = hText->Line_Width(LineNumber, Width_EN);
		pCursor->ResetChoose();
		return RVALUE(y) << 32 | RVALUE(x);
	}
	case UM_RETURN:			//�س�����
	{
		Position position;
		/*���ı��ڵĴ���*/
		position.LineNumber = LineNumber;
		position.Sequence = pCursor->Characters_before_Cursor(LineNumber, x);
		
		Record* rd = new Record(RD_RETURN);							//��¼������Ϣ
		rd->Set_Choose_Data(position, position);
		pRecord->push(rd);

		position = hText->EnterNewLine(position);
		int New_Width = hText->Max_Line_Width(Width_EN);			//������ı������
																	
		//���ֽ��ı���Χ�����ȣ���λ���أ����ֽ�������
		pCursor->ResetChoose();
		return  RVALUE(Height*hText->Line_Number()) << 32 | RVALUE(New_Width);
	}
	case UM_DELETE:
	{
		Record* rd = new Record(RD_DELETE);
		//ѡ�ν�����������
		int end_x = LODWORD(sParam);
		int end_y = HIDWORD(sParam);
		Position start_position;
		Position end_position;
		//ɾ��start������ַ���end���ǰ�������ַ�
		try
		{
			start_position = pCursor->CursorToPosition_After(x, y);		//���ɾ�������

		}
		catch (std::invalid_argument &e)
		{
			//���λ���ı�ĩβ
			//���ı�����
			int New_Width_X = hText->Max_Line_Width(Width_EN);
			int New_Height_Y = hText->Line_Number()*Height;
			delete rd;
			return RVALUE(New_Height_Y) << 32 | RVALUE(New_Width_X);
		}
		end_position = pCursor->CursorToPosition(end_x, end_y);
		//λ����ͬ��ʾ�˸��
		if (end_position == start_position)
		{
			if (start_position.Sequence == 0)
			{
				rd->ACT = RD_MERGE_LINE;
				if (start_position.LineNumber > 1)			//����¼��Ч��ɾ��
				{
					rd->Save_Merge_Line_Data(hText, start_position.LineNumber);
					pRecord->push(rd);
				}
				else
					delete rd;
			}
			else
			{
				rd->Save_Delete_Data(hText, start_position, start_position);
				pRecord->push(rd);
			}
			hText->BackSpace(start_position);
		}
		//������ȱ�ʾ���ɾ��
		else if (end_x == x && end_y == y)
		{
			try
			{
				end_position = pCursor->CursorToPosition_After(end_x, end_y);
				if (end_position.LineNumber != LineNumber)			//�л��е����  ��  ���λ����β
				{
					rd->ACT = RD_MERGE_LINE;
					rd->Save_Merge_Line_Data(hText, end_position.LineNumber);
				}
				else
				{
					rd->Save_Delete_Data(hText, end_position, end_position);
				}
				hText->BackSpace(end_position);
				pRecord->push(rd);
			}
			catch (std::invalid_argument& e)
			{
				//�յ��쳣 ˵����ǰ���λ���ı�ĩβ �����ɾ����������
				delete rd;
				pCursor->ResetChoose();
				return RVALUE(0);
			}
		}
		else
		{
			rd->Save_Delete_Data(hText, start_position, end_position);
			pRecord->push(rd);
			hText->Delete(start_position, end_position);
		}
		//���ı�����
		int New_Width_X = hText->Max_Line_Width(Width_EN);
		int New_Height_Y = hText->Line_Number()*Height;
		pCursor->ResetChoose();
		return RVALUE(New_Height_Y) << 32 | RVALUE(New_Width_X);
	}
	case UM_CURSOR:		//��λ�Ϸ����
	{
		x = pCursor->CursorLocation(LineNumber, x);
		pCursor->ResetChoose();
		return RVALUE(y) << 32 | RVALUE(x);

	}
	case UM_HOME:		//Home��
	{
		x = 0;
		pCursor->ResetChoose();
		return RVALUE(y) << 32 | RVALUE(x);
	}
	case UM_NEW:
	{
		if (hText->isSaved())
		{
			hText->ClearAll();
			pCursor->ResetChoose();
			return UR_SAVED;
		}
		return  UR_NOTSAVED;
	}
	case UM_SAVE:
	{
		if (hText->FilePath().empty())
		{
			if ((LPWSTR)sParam == NULL)
				return UR_NOPATH;
			else
			{
				std::string otherFileName = wchTostring((LPWSTR)sParam);
				hText->Save(otherFileName, true);
			}

		}
		else
			hText->Save();
		break;
	}
	case UM_CLOSE:
	{
		if (hText->isSaved())
		{
			pCursor->ResetChoose();
			return UR_SAVED;
		}
		else
			return UR_NOTSAVED;
	}
	case UM_OPEN:
	{
		if (hText->isSaved())
		{
			TCHAR* pwch = (TCHAR*)sParam;
			std::string FileName = wchTostring(pwch);
			hText->ReadText(FileName);
			int TextWidth = hText->Max_Line_Width(Width_EN);
			pCursor->ResetChoose();
			return RVALUE(hText->Line_Number()*Height) << 32 | RVALUE(TextWidth);
		}
		else
		{
			return UR_NOTSAVED;
		}
	}
	case UM_COPY:
	{
		if (!pCursor->isChoose())
			return UR_ERROR;
		Position copy_start = pCursor->start;
		Position copy_end = pCursor->end;
		std::wstring wstr_copy = hText->Copy(copy_start, copy_end);
		Alloc_Buffer(pBuffer, MaxBufferSize, wstr_copy.size());
		WStringToWch(wstr_copy, pBuffer);
		if (wstr_copy.size() < MaxBufferSize)
			pBuffer[wstr_copy.size()] = L'\0';
		*((LPWSTR*)sParam) = pBuffer;
		return RVALUE(wstr_copy.size());
	}
	case UM_CHAR:
	{
		pCursor->ResetChoose();
		Record* rd = new Record(RD_INSERT);
		std::wstring ws;
		TCHAR wch = (TCHAR)LODWORD(sParam);			//�������ַ�
		Position position = pCursor->CursorToPosition(x, y);
		if (wch == L'\t')							//����Tab
		{
			int PreWidth = hText->Line_Width(position.LineNumber, Width_EN, position.Sequence);		
			int n = TAB_SIZE - (PreWidth / Width_EN) % TAB_SIZE;			//n��ʾ��Ҫ����ո������
			x += Width_EN * n;
			for (int i = 0; i < n; i++)
				ws.push_back(L' ');
			Position temp = { position.LineNumber,position.Sequence + 1 };
			position = hText->Insert(position, ws);
			rd->Set_Choose_Data(temp, position);
		}
		else
		{
			ws.push_back(wch);

			position = hText->Insert(position, ws);				//��Position�������һ���ַ�  position��¼�������ַ�λ��

			rd->Set_Choose_Data(position, position);
			if (WORD(wch >> 8) > 0)
				x += Width_ZH;
			else
				x += Width_EN;
		}

		pRecord->push(rd);
		int New_Max_Width = hText->Max_Line_Width(Install::Width);
		return RVALUE(x) << 32 | RVALUE(New_Max_Width);
	}
	case UM_TEXT:
	{
		*(TCHAR**)sParam = pTChar;
		//���ı�����
		if (hText->Line_Number() < LineNumber)	
		{
			pTChar[0] = L'\0';
			return RVALUE(0);
		}
		pCursor->CursorLocation(LineNumber, x);
		int iCount = 0;					//��ʾ���ַ�����
		short int iStart = 0;			//�������ֵĿ�ʼ��
		short int iEnd = 0;				//�������ֽ�����
		int end_x = LODWORD(fParam);
		CLine* pLine = hText->GetLinePointer(LineNumber);
		Position position_start;		//�ַ���ʾ���
		Position position_end;			//�ַ���ʾ�յ�
		try
		{
			position_start = pCursor->CursorToPosition_After(x, y);		//��ù����ַ�
		}
		catch (std::invalid_argument& e)
		{
			//��ʼ���λ���ı�ĩβ
			pTChar[0] = L'\0';
			return RVALUE(0);
		}
		end_x = pCursor->CursorLocation(LineNumber, end_x);
		position_end = pCursor->CursorToPosition(end_x, y);
		std::wstring WStr = pLine->TransformToWString(position_start.Sequence, position_end.Sequence);
		WStringToWch(WStr, pTChar);
		iCount = WStr.size();
		if (iCount < TEXT_SIZE)
			pTChar[iCount] = L'\0';
		//���ø�������  ��0��ʼ
		if(pCursor->isChoose())
			Set_Height_Light(position_start.LineNumber, position_start, position_end, iStart, iEnd);
		return RVALUE(iCount) << 32 | RVALUE(iEnd) << 16 | RVALUE(iStart);
	}
	case UM_ALL:
	{
		int MaxLineNumber = hText->Line_Number();
		y = (MaxLineNumber - 1)*Height;
		x = hText->Line_Width(MaxLineNumber, Width_EN);
		pCursor->Choose(hText->First_Position(), hText->End_Position());		//ȫѡ
		return RVALUE(y) << 32 | RVALUE(x);
	}
	case UM_CHOOSE:
	{
		Position start, end;
		int end_x = LODWORD(fParam);
		int end_y = HIDWORD(fParam);
		end_y -= end_y % Height;
		end_x = pCursor->CursorLocation(end_y / Height + 1, end_x);
		x = pCursor->CursorLocation(LineNumber, x);
		POINT p = { x,y };
		if (y > end_y)
		{
			std::swap(x, end_x);
			std::swap(y, end_y);
		}
		else if (y == end_y && x > end_x)
			std::swap(x, end_x);
		
	
		end = pCursor->CursorToPosition(end_x, end_y);
		try
		{
			start = pCursor->CursorToPosition_After(x, y);
			pCursor->Choose(start, end);					//��¼ѡ����Ϣ ��start ��ǰ end �ں�
		}
		catch (std::invalid_argument& e)
		{
			pCursor->ResetChoose();
		}				
		
		return RVALUE(p.y) << 32 | RVALUE(p.x);
	}
	case UM_CANCEL:
	{

		if (pRecord->empty())			//��ǰû�д������Ĳ���
			return UR_NOTCANCEL;
		Record* p = pRecord->top();
		//ȡ�����һ�εĲ���
		p->ReDo(hText);
		pCursor->ResetChoose();			//���֮ǰ��ѡ����Ϣ
		switch (p->ACT)
		{
		case RD_MERGE_LINE:
		{
			(*(POINT*)fParam) = (*(POINT*)sParam) = pCursor->PositionToCursor(p->start);
			break;
		}
		case RD_INSERT :
		{
			(*(POINT*)fParam) = (*(POINT*)sParam) = pCursor->PositionToCursor(p->start);
			break;
		}
		case RD_RETURN :
		{
			(*(POINT*)fParam) = (*(POINT*)sParam) = pCursor->PositionToCursor(p->start);
			break;
		}
		case RD_DELETE:
		{
			(*(POINT*)fParam) = pCursor->PositionToCursor({ p->start.LineNumber,p->start.Sequence - 1 });
			(*(POINT*)sParam) = pCursor->PositionToCursor(p->end);
			pCursor->Choose(p->start, p->end);			//�������ѡ����Ϣ
			break;
		}
		default:break;
		}


		pRecord->pop();
		delete p;
		return RVALUE((hText->Line_Number())*Height) << 32 | RVALUE(hText->Max_Line_Width(Width_EN));
	}
	case UM_PASTE:
	{
		pCursor->ResetChoose();
		Record* rd = new Record(RD_INSERT);
		std::string SText = wchTostring((TCHAR*)sParam);
		std::wstring WSText = StringToWString(SText);		//�����������
		Position start;
		start = pCursor->CursorToPosition(x, y);
		Position s = { start.LineNumber,start.Sequence + 1 };	//���������ַ�λ��
		start = hText->Insert(start, WSText);					//�����ַ� start�õ���������һ���ַ���λ��

		rd->Set_Choose_Data(s, start);
		pRecord->push(rd);

		*(POINT*)fParam = pCursor->PositionToCursor(start);
		int Text_Width = hText->Max_Line_Width(Width_EN);
		int Text_Height = hText->Line_Number()*Height;

		return RVALUE(Text_Height) << 32 | RVALUE(Text_Width);
	}
	default: break;
	}
	return UR_SUCCESS;
}
//�����ı�����
HTEXT _stdcall CreateText(int iCharWidth, int iCharHeight)
{
	CText* p = new CText;
	p->NewFile();
	pTChar = new wchar_t[TEXT_SIZE];
	wmemset(pTChar, L' ', TEXT_SIZE);
	Install::Height = iCharHeight;
	Install::Width = iCharWidth;
	pCursor = Initialize_Cursor(p, iCharWidth, iCharHeight);			//���������
	pRecord = new std::stack<Record*>;									//���볷����
	return p;
}
//�����ı�����
BOOL _stdcall DestroyText(HTEXT &hText)
{
	if (hText != NULL)
		delete hText;
	hText = NULL;

	delete pRecord;
	pRecord = NULL;
	delete pCursor;
	pCursor = NULL;

	Free_Buffer(pTChar);
	Free_Buffer(pBuffer);
	return TRUE;
}

Cursor * Initialize_Cursor(CText * p, int Width, int Height)
{
	Cursor* pCursor = new Cursor(p, Width, Height);
	return pCursor;
}


//Ϊ���ݴ洢���뻺���� ��������Ϊ��������
void Alloc_Buffer(wchar_t *& p, size_t & Old_Size, size_t New_Size)
{
	if (p == NULL)
	{
		p = new wchar_t[New_Size];
		Old_Size = New_Size;
		return;
	}
	if (New_Size > Old_Size)				//��������Ҫ����
	{
		delete[]p;
		p = new wchar_t[New_Size];
		Old_Size = New_Size;
	}

}




//�ͷŷ�������ݻ�����
void Free_Buffer(wchar_t *& p)
{
	if (p != NULL)
		delete[]p;
	p = NULL;
}

void Set_Height_Light(int LineNumber, Position ps, Position pe, short int & start, short int & end)
{
	Position s = pCursor->start;
	Position e = pCursor->end;
	int Len = pe.Sequence - ps.Sequence + 1;			//ѡ�γ���
	if (LineNumber<s.LineNumber || LineNumber>e.LineNumber)			//ѡ�β���ѡ�з�Χ
	{
		start = end = 0;
		return;
	}
	if (s.LineNumber == e.LineNumber)					//ѡ�з�Χֻ��һ��
	{
		start = max(0, s.Sequence - ps.Sequence);
		if (e.Sequence < ps.Sequence)
			end = 0;
		else if (e.Sequence > pe.Sequence)
			end = Len;
		else
			end = e.Sequence - ps.Sequence + 1;
		return;
	}
	if (LineNumber == s.LineNumber)
	{
		start = max(0, s.Sequence - ps.Sequence);
		end = Len;
	}
	else if (LineNumber == e.LineNumber)
	{
		start = 0;
		end = max(0, e.Sequence - ps.Sequence + 1);
	}
	else
	{
		start = 0;
		end = Len;
	}
	return;
}





