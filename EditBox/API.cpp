#include "stdafx.h"
#include<stack>
#include "editbox.h"


Cursor* pCursor = NULL;
wchar_t* pTChar = NULL;
wchar_t* pDataBuffer = NULL;
size_t MaxBufferSize = 0;
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
			if (!(pCursor->isLegalCursor(LineNumber - 1, x)))			//光标位置合法性检测 防止因上下移动使光标在汉字中间显示
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
				x += Width_ZH;
		}
		pCursor->ResetChoose();
		return RVALUE(y) << 32 | RVALUE(x);
	}
	case UM_END:			//当前行尾时x y坐标
	{
		x = hText->Line_Width(LineNumber, Width_EN);
		pCursor->ResetChoose();
		return RVALUE(y) << 32 | RVALUE(x);
	}
	case UM_RETURN:			//回车换行
	{
		Position position;
		/*对文本内的处理*/
		position.LineNumber = LineNumber;
		position.Sequence = pCursor->Characters_before_Cursor(LineNumber, x);
		position = hText->EnterNewLine(position);
		int New_Width = hText->Max_Line_Width(Width_EN);			//处理后文本最大宽度
																	//记录撤销信息
		Record* rd = new Record(RD_RETURN);
		rd->Set_Choose_Data(position, position);
		pRecord->push(rd);
		//高字节文本范围横向宽度（单位像素）低字节纵向宽度
		pCursor->ResetChoose();
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
			start_position = pCursor->CursorToPosition_After(x, y);		//获得删除的起点

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
			if (start_position.Sequence == 0)
			{
				rd->ACT = RD_MERGE_LINE;
				if (start_position.LineNumber > 1)			//不记录无效的删除
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
		//坐标相等表示向后删除
		else if (end_x == x && end_y == y)
		{
			try
			{
				end_position = pCursor->CursorToPosition_After(end_x, end_y);
				if (end_position.LineNumber != LineNumber)			//有换行的情况  即  光标位于行尾
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
				//收到异常 说明当前光标位置文本末尾 对向后删除不做处理
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
		//新文本长宽
		int New_Width_X = hText->Max_Line_Width(Width_EN);
		int New_Height_Y = hText->Line_Number()*Height;
		pCursor->ResetChoose();
		return RVALUE(New_Height_Y) << 32 | RVALUE(New_Width_X);
	}
	case UM_CURSOR:		//定位合法光标
	{
		x = pCursor->CursorLocation(LineNumber, x);
		pCursor->ResetChoose();
		return RVALUE(y) << 32 | RVALUE(x);

	}
	case UM_HOME:		//Home键
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
			throw std::invalid_argument("没有选中信息 拷贝失败");
		Position copy_start = pCursor->start;
		Position copy_end = pCursor->end;
		std::wstring wstr_copy = hText->Copy(copy_start, copy_end);
		Alloc_Buffer(pTChar, MaxBufferSize, wstr_copy.size());
		WStringToWch(wstr_copy, pTChar);
		*((LPWSTR*)sParam) = pTChar;
		return RVALUE(wstr_copy.size());
	}
	case UM_CHAR:
	{
		pCursor->ResetChoose();
		Record* rd = new Record(RD_INSERT);
		std::wstring ws;
		TCHAR wch = (TCHAR)LODWORD(sParam);			//待插入字符
		Position position = pCursor->CursorToPosition(x, y);
		if (wch == L'\t')							//处理Tab
		{
			int PreWidth = hText->Line_Width(position.LineNumber, Width_EN, position.Sequence);		
			int n = TAB_SIZE - (PreWidth / Width_EN) % TAB_SIZE;			//n表示需要插入空格的数量
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

			position = hText->Insert(position, ws);				//在Position后面插入一个字符  position记录插入后的字符位置

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
		//非文本区域
		if (hText->Line_Number() < LineNumber)	
		{
			pTChar[0] = L'\0';
			return RVALUE(0);
		}
		pCursor->CursorLocation(LineNumber, x);
		int iCount = 0;					//显示的字符数量
		short int iStart = 0;			//高亮部分的开始点
		short int iEnd = 0;				//高亮部分结束点
		int end_x = LODWORD(fParam);
		CLine* pLine = hText->GetLinePointer(LineNumber);
		Position position_start;		//字符显示起点
		Position position_end;			//字符显示终点
		try
		{
			position_start = pCursor->CursorToPosition_After(x, y);		//获得光标后字符
		}
		catch (std::invalid_argument& e)
		{
			//起始光标位于文本末尾
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
		//设置高亮部分  从0开始
		if(pCursor->isChoose())
			Set_Height_Light(position_start.LineNumber, position_start, position_end, iStart, iEnd);
		return RVALUE(iCount) << 32 | RVALUE(iEnd) << 16 | RVALUE(iStart);
	}
	case UM_ALL:
	{
		int MaxLineNumber = hText->Line_Number();
		y = (MaxLineNumber - 1)*Height;
		x = hText->Line_Width(MaxLineNumber, Width_EN);
		pCursor->Choose(hText->First_Position(), hText->End_Position());		//全选
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
			pCursor->Choose(start, end);					//记录选中信息 （start 在前 end 在后）
		}
		catch (std::invalid_argument& e)
		{
			pCursor->ResetChoose();
		}				
		
		return RVALUE(p.y) << 32 | RVALUE(p.x);
	}
	case UM_CANCEL:
	{

		if (pRecord->empty())			//当前没有待撤销的步骤
			return UR_NOTCANCEL;
		Record* p = pRecord->top();
		//取出最近一次的操作
		p->ReDo(hText);
		pCursor->ResetChoose();			//清空之前的选中信息
		switch (p->ACT)
		{
		case RD_RETURN | RD_INSERT | RD_MERGE_LINE:
		{
			(*(POINT*)fParam) = (*(POINT*)sParam) = pCursor->PositionToCursor(p->start);
			break;
		}
		case RD_DELETE:
		{
			(*(POINT*)fParam) = pCursor->PositionToCursor({ p->start.LineNumber,p->start.Sequence - 1 });
			(*(POINT*)sParam) = pCursor->PositionToCursor(p->end);
			pCursor->Choose(p->start, p->end);			//鼠标设置选段信息
			break;
		}
		default:break;
		}


		pRecord->pop();
		delete p;
		return RVALUE((hText->Line_Number() - 1)*Height) << 32 | RVALUE(hText->Max_Line_Width(Width_EN));
	}
	case UM_PASTE:
	{
		Record* rd = new Record(RD_INSERT);
		std::string SText = wchTostring((TCHAR*)sParam);
		std::wstring WSText = StringToWString(SText);		//待插入的内容
		Position start;
		if (x == 0)			//在行首插入
			start = { LineNumber,0 };
		else
			start = pCursor->CursorToPosition(x, y);
		Position s = { start.LineNumber,start.Sequence + 1 };	//插入后的首字符位置
		start = hText->Insert(start, WSText);					//插入字符 start得到插入的最后一个字符的位置

		rd->Set_Choose_Data(s, start);
		pRecord->push(rd);
		pCursor->Choose(s, start);

		*(POINT*)fParam = pCursor->PositionToCursor(start);
		int Text_Width = hText->Max_Line_Width(Width_EN);
		int Text_Height = hText->Line_Number()*Height;

		return RVALUE(Text_Height) << 32 | RVALUE(Text_Width);
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

	delete pRecord;
	pRecord = NULL;
	delete pCursor;
	pCursor = NULL;

	Free_Buffer(pTChar);
	Free_Buffer(pDataBuffer);
	return TRUE;
}

Cursor * Initialize_Cursor(CText * p, int Width, int Height)
{
	Cursor* pCursor = new Cursor(p, Width, Height);
	return pCursor;
}


//为数据存储申请缓冲区 存在周期为整个程序
void Alloc_Buffer(wchar_t *& p, size_t & Old_Size, size_t New_Size)
{
	if (p == NULL)
	{
		p = new wchar_t[New_Size];
		Old_Size = New_Size;
		return;
	}
	if (New_Size > Old_Size)				//缓冲区需要扩容
	{
		delete[]p;
		p = new wchar_t[New_Size];
		Old_Size = New_Size;
	}
	else
	{
		if (Old_Size > 1000)				//缓冲区太大 减小缓冲区长度
		{
			delete[]p;
			int n = max(New_Size, Old_Size / 3);
			p = new wchar_t[n];
			Old_Size = n;
		}
	}
}




//释放分配的数据缓冲区
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
	int Len = pe.Sequence - ps.Sequence + 1;			//选段长度
	if (LineNumber<s.LineNumber || LineNumber>e.LineNumber)			//选段不在选中范围
	{
		start = end = 0;
		return;
	}
	if (s.LineNumber == e.LineNumber)					//选中范围只有一行
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





