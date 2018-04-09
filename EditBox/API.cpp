#include "stdafx.h"
#include <stack>

Cursor* pCursor = NULL;
wchar_t* pTChar = NULL;
std::stack<Record*>* pRecord = NULL;
RVALUE _stdcall UserMessageProc(HTEXT hText, int x, int y, UINT message, FPARAM fParam, SPARAM sParam)
{
	pCursor->SethText(hText);
	int Width_EN = Install::Width;			//字符宽度（英文、特殊符号单位宽度 汉字、汉化符号占两个Width）	
	int Width_ZH = 2 * Width_EN;			//汉字模式下符号宽度
	int Height = Install::Height;			//字符高度
	int LineNumber = y / Height + 1;
	//对于鼠标点击文本下方空白区域的行号计算
	if (LineNumber > hText->Line_Number() && message != UM_TEXT)
	{
		LineNumber = hText->Line_Number();
		y = (LineNumber - 1) * Height;
	}
	//规格化y坐标
	y = y - y % Height;
	//对消息做出的处理
	switch (message)
	{
		case UM_UP:
		{
			if (LineNumber > 1)
			{
				y -= Height;
				int LastLineWidth = hText->Line_Width(LineNumber - 1, Width_EN);
				x = min(LastLineWidth, x);
				if (!(pCursor->isLegalCursor(LineNumber-1, x)))			//光标位置合法性检测 防止因上下移动使光标在汉字中间显示
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
				if (LineNumber != hText->Line_Number())		//普通行尾
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
		case UM_END:			//当前行尾时x y坐标
		{
			x = hText->Line_Width(LineNumber, Width_EN);
			return RVALUE(y) << 32 | RVALUE(x);
		}		
		case UM_RETURN:		//回车换行
		{
			Position position; 
			/*对文本内的处理*/
			position.LineNumber = LineNumber;
			position.Sequence = pCursor->Characters_before_Cursor(LineNumber, x);
			position = hText->EnterNewLine(position);
			int New_Width = hText->Max_Line_Width(Width_EN);			//处理后文本最大宽度
			//记录撤销信息
			Record* rd = new Record(RD_RETURN);
			rd->start = position;
			pRecord->push(rd);
			/*对光标的处理*/;
			//高字节文本范围横向宽度（单位像素）低字节纵向宽度
			return  RVALUE(Height*hText->Line_Number()) << 32 | RVALUE(New_Width);
		}
		case UM_DELETE:
		{
			Record* rd = new Record(RD_DELETE);
			//选段结束横纵坐标
			int end_x = LODWORD(sParam);
			int end_y = HIDWORD(sParam);
			Position start_position;
			Position end_position;
			//删除start光标后的字符到end光标前的所有字符
			try 
			{
				start_position = pCursor->CursorToPosition_After(x, y);
				
			}
			catch (std::invalid_argument &e)
			{
				//光标位于文本末尾
				//新文本长宽
				int New_Width_X = hText->Max_Line_Width(Width_EN);
				int New_Height_Y = hText->Line_Number()*Height;
				delete rd;
				return RVALUE(New_Height_Y) << 32 | RVALUE(New_Width_X);
			}
			end_position = pCursor->CursorToPosition(end_x, end_y);
			//位置相同表示退格键
			if (end_position == start_position)
			{
				rd->Save_Delete_Data(hText,start_position, start_position);
				pRecord->push(rd);
				hText->BackSpace(start_position);
			}
			//坐标相等表示向后删除
			if (end_x == x && end_y == y)
			{
				try
				{
					end_position = pCursor->CursorToPosition_After(end_x, end_y);
					hText->BackSpace(end_position);
					rd->Save_Delete_Data(hText, end_position, end_position);
					pRecord->push(rd);
				}
				catch (std::invalid_argument& e)
				{
					//收到异常 说明当前光标位置文本末尾 对向后删除不做处理
					return RVALUE(0);
				}
			}
			else
			{
				hText->Delete(start_position, end_position);
				rd->Save_Delete_Data(hText, start_position, end_position);
				pRecord->push(rd);
			}
			//新文本长宽
			int New_Width_X = hText->Max_Line_Width(Width_EN);
			int New_Height_Y = hText->Line_Number()*Height;
			return RVALUE(New_Height_Y) << 32 | RVALUE(New_Width_X);
		}
		case UM_CURSOR:		//定位合法光标
		{
			x = pCursor->CursorLocation(LineNumber, x);
			return RVALUE(y) << 32 | RVALUE(x);

		}
		case UM_HOME:		//Home键
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
			TCHAR wch = (TCHAR)LODWORD(sParam);		//带插入字符
			ws.push_back(wch);
			Position position = pCursor->CursorToPosition(x, y);
			hText->Insert(position, ws);
			rd->Set_Choose_Data(position, position);
			pRecord->push(rd);
			int New_Max_Width = hText->Max_Line_Width(Install::Width);
			return RVALUE(New_Max_Width);
		}
		case UM_TEXT:
		{
			wmemset(pTChar, L' ', TEXT_SIZE);
			*(TCHAR**)sParam = pTChar;
			//非文本区域
			if (hText->Line_Number() < LineNumber)
			{
				return RVALUE(TEXT_SIZE) << 32;
			}
			int iCount = 0;
			short int iStart = 0;			//高亮部分的开始点
			short int iEnd = 0;				//高亮部分结束点
			int end_x = LODWORD(fParam);
			CLine* pLine = hText->GetLinePointer(LineNumber);
			Position position_start;		//字符显示起点
			Position position_end;			//字符显示终点
			end_x = pCursor->CursorLocation(LineNumber, end_x);
			position_end = pCursor->CursorToPosition(end_x, y);
			try
			{
				position_start = pCursor->CursorToPosition_After(x, y);		//获得光标后字符
				if (position_start.Sequence == 0)
				{
					//位于行尾 不用显示
					throw std::invalid_argument("LineRear");
				}
			}
			catch (std::invalid_argument& e)
			{
				//起始光标位于文本末尾
				iCount = (end_x - pLine->Line_Width(Install::Width)) / Install::Width;
				return RVALUE(TEXT_SIZE) << 32;
			}
			std::wstring WStr = pLine->TransformToWString(position_start.Sequence, position_end.Sequence);
			WStringToWch(WStr,pTChar);
			iCount = WStr.size() + (end_x - hText->Line_Width(LineNumber, Install::Width)) / Install::Width;
			//高亮部分待完成
			return RVALUE(iCount) << 32 | RVALUE(iEnd) << 16 | RVALUE(iStart);
		}
		case UM_ALL:
		{
			int MaxLineNumber = hText->Line_Number();
			y = (MaxLineNumber - 1)*Height;
			x = hText->Line_Width(MaxLineNumber, Width_EN);
			return RVALUE(y) << 32 | RVALUE(x);
		}
		case UM_CHOOSE:		//待定
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
			Record* p = pRecord->top();
			pRecord->pop();
			p->ReDo(hText);
			if (p->ACT == RD_RETURN)		//撤销换行
			{	
				(*(POINT*)fParam) = (*(POINT*)sParam) = pCursor->PositionToCursor(p->start);
			}
			else                           //撤销删除、插入
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
//创建文本函数
HTEXT _stdcall CreateText(int iCharWidth, int iCharHeight)
{
	CText* p = new CText;
	p->NewFile();
	pTChar = new wchar_t[TEXT_SIZE];
	wmemset(pTChar, L' ', TEXT_SIZE);
	Install::Height = iCharHeight;
	Install::Width = iCharWidth;
	pCursor = Initialize_Cursor(p, iCharWidth, iCharHeight);			//申请鼠标类
	pRecord = new std::stack<Record*>;									//申请撤销类
	return p;
}
//销毁文本函数
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
