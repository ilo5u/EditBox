#include "stdafx.h"
#include<stack>
Cursor* pCursor = NULL;
wchar_t* pTChar = NULL;
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
				if (!(pCursor->isLegalCursor(LineNumber-1, x)))			//���λ�úϷ��Լ�� ��ֹ�������ƶ�ʹ����ں����м���ʾ
				{
					x -= Width_EN;
				}
			}
			return RVALUE(y) << 32 | RVALUE(x);
		}
		case UM_DOWN:
		{
			if (LineNumber < hText->Line_Number())
			{
				y += Height;
				int NextLineWidth = hText->Line_Width(LineNumber + 1, Width_EN);
				x = min(NextLineWidth, x);
				if (!(pCursor->isLegalCursor(LineNumber+1, x)))
				{
					x -= Width_EN;
				}
			}
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
			return RVALUE(y) << 32 | RVALUE(x);
		}
		case UM_RIGHT :
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
					x+=Width_ZH;
			}
			return RVALUE(y) << 32 | RVALUE(x);
		}
		case UM_END:			//��ǰ��βʱx y����
		{
			x = hText->Line_Width(LineNumber, Width_EN);
			return RVALUE(y) << 32 | RVALUE(x);
		}		
		case UM_RETURN:		//�س�����
		{
			Position position; 
			/*���ı��ڵĴ���*/
			position.LineNumber = LineNumber;
			position.Sequence = pCursor->Characters_before_Cursor(LineNumber, x);
			position = hText->EnterNewLine(position);
			int New_Width = hText->Max_Line_Width(Width_EN);			//������ı������
			//��¼������Ϣ
			Record* rd = new Record(RD_RETURN);
			rd->Set_Choose_Data(position, position);
			pRecord->push(rd);
			//���ֽ��ı���Χ�����ȣ���λ���أ����ֽ�������
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
				rd->Save_Delete_Data(hText,start_position, start_position);
				pRecord->push(rd);
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
						end_position.Sequence--;
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
			return RVALUE(New_Height_Y) << 32 | RVALUE(New_Width_X);
		}
		case UM_CURSOR:		//��λ�Ϸ����
		{
			x = pCursor->CursorLocation(LineNumber, x);
			return RVALUE(y) << 32 | RVALUE(x);

		}
		case UM_HOME:		//Home��
		{
			x = 0;
			return RVALUE(y) << 32 | RVALUE(x);
		}
		case UM_NEW:
		{
			if (hText->isSaved())
			{
				hText->ClearAll();
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
				return UR_SAVED;
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
				return RVALUE(hText->Line_Number()*Height) << 32 | RVALUE(TextWidth);
			}
			else
			{
				return UR_NOTSAVED;
			}
		}
		case UM_COPY:
		{
			int end_x = LODWORD(fParam);
			int end_y = HIDWORD(fParam);
			Position copy_start = pCursor->CursorToPosition_After(x, y);
			Position copy_end = pCursor->CursorToPosition(end_x, end_y);
			int Size = 0;
			if (copy_start == copy_end)
			{
				return RVALUE(0);
			}
			else
			{
				std::wstring wstr_copy = pCursor->Copy(copy_start, copy_end);
				TCHAR* pwch = new TCHAR[wstr_copy.size()];
				WStringToWch(wstr_copy, pwch);
				*((LPWSTR*)sParam) = pwch;
				return RVALUE(wstr_copy.size());
			}
		}
		case UM_CHAR:
		{
			Record* rd = new Record(RD_INSERT);
			std::wstring ws;
			TCHAR wch = (TCHAR)LODWORD(sParam);		//�������ַ�
			ws.push_back(wch);
			Position position = pCursor->CursorToPosition(x, y);
			hText->Insert(position, ws);			//��Position�������һ���ַ�
			position = { position.LineNumber,position.Sequence + 1 };
			rd->Set_Choose_Data(position, position);
			pRecord->push(rd);
			int New_Max_Width = hText->Max_Line_Width(Install::Width);
			if (WORD(wch >> 8) > 0)
				x += Width_ZH;
			else
				x += Width_EN;
			return RVALUE(x) << 32 | RVALUE(New_Max_Width);				
		}
		case UM_TEXT:
		{
			wmemset(pTChar, L' ', TEXT_SIZE);
			*(TCHAR**)sParam = pTChar;
			//���ı�����
			if (hText->Line_Number() < LineNumber)
			{
				return RVALUE(TEXT_SIZE) << 32;
			}
			int iCount = 0;
			short int iStart = 0;			//�������ֵĿ�ʼ��
			short int iEnd = 0;				//�������ֽ�����
			int end_x = LODWORD(fParam);
			CLine* pLine = hText->GetLinePointer(LineNumber);
			Position position_start;		//�ַ���ʾ���
			Position position_end;			//�ַ���ʾ�յ�
			try
			{
				position_start = pCursor->CursorToPosition_After(x, y);		//��ù����ַ�
				if (position_start.Sequence == 0)
				{
					//λ����β ������ʾ
					throw std::invalid_argument("LineRear");
				}
			}
			catch (std::invalid_argument& e)
			{
				//��ʼ���λ���ı�ĩβ
				iCount = (end_x - pLine->Line_Width(Install::Width)) / Install::Width;
				return RVALUE(TEXT_SIZE) << 32;
			}
			end_x = pCursor->CursorLocation(LineNumber, end_x);
			position_end = pCursor->CursorToPosition(end_x, y);
			std::wstring WStr = pLine->TransformToWString(position_start.Sequence, position_end.Sequence);
			WStringToWch(WStr,pTChar);
			iCount = WStr.size() + (end_x - hText->Line_Width(LineNumber, Install::Width)) / Install::Width;
			//�������ִ����
			return RVALUE(iCount) << 32 | RVALUE(iEnd) << 16 | RVALUE(iStart);
		}
		case UM_ALL:
		{
			int MaxLineNumber = hText->Line_Number();
			y = (MaxLineNumber - 1)*Height;
			x = hText->Line_Width(MaxLineNumber, Width_EN);
			return RVALUE(y) << 32 | RVALUE(x);
		}
		case UM_CHOOSE:		//����
		{
			Position start, end;
			int end_x = LODWORD(fParam);
			int end_y = HIDWORD(fParam);
			end_y -= end_y % Height;
			end.LineNumber = end_y / Height + 1;
			end_x = pCursor->CursorLocation(end.LineNumber, end_x);
			end = pCursor->CursorToPosition(end_x, end_y);
			start = pCursor->CursorToPosition_After(x, y);
			return RVALUE(y) << 32 | RVALUE(x);
		}
		case UM_CANCEL:
		{
			if (pRecord->empty())			//��ǰû�д������Ĳ���
				return UR_NOTCANCEL;	
			Record* p = pRecord->top();
			//ȡ����һ����ѡ����Ϣ
			while (p->ACT != RD_CHOOSE)
			{
				pRecord->pop();
				if(pRecord->empty())		//��ǰ����ջ������ѡ����Ϣ��û�д������Ĳ���
					return UR_NOTCANCEL;
				p = pRecord->top();
			}
			pRecord->pop();
			p->ReDo(hText);
			if (p->ACT == RD_RETURN)		//��������
			{	
				(*(POINT*)fParam) = (*(POINT*)sParam) = pCursor->PositionToCursor(p->start);
			}
			else                           //����ɾ��������
			{
				(*(POINT*)fParam) = pCursor->PositionToCursor({ p->start.LineNumber,p->start.Sequence - 1 });
				(*(POINT*)sParam) = pCursor->PositionToCursor(p->end);
			}
			delete p;
			return RVALUE((hText->Line_Number() - 1)*Height) << 32 | RVALUE(hText->Max_Line_Width(Width_EN));
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
	delete[]pTChar;
	pTChar = NULL;
	Free_Cursor(pCursor);
	pCursor = NULL;
	delete pRecord;
	pRecord = NULL;
	return TRUE;
}

Cursor * Initialize_Cursor(CText * p, int Width, int Height)
{
	Cursor* pCursor = new Cursor(p, Width, Height);
	return pCursor;
}

void Free_Cursor(Cursor * &p)
{
	if (p != NULL)
		delete p;
	p = NULL;
}
