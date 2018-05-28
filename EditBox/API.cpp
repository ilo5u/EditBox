#include "stdafx.h"
#include "Cursor.h"


Cursor* pCursor = NULL;
wchar_t* pTChar = NULL;
wchar_t* pBuffer = NULL;
size_t MaxBufferSize = 0;
std::stack<Record*>* pRecord = NULL;
RVALUE __stdcall UserMessageProc(
	_In_ HTEXT hText,
	_In_opt_ int x, _In_opt_ int y,
	_In_ UINT message,
	_In_opt_ FPARAM fParam, _In_opt_ SPARAM sParam,
	_Out_opt_ LPKERNELINFO lpKernelInfo
)
{
	pCursor->SethText(hText);
	int Width_EN = Install::Width;			//字符宽度（英文、特殊符号单位宽度 汉字、汉化符号占两个Width）	
	int Width_ZH = 2 * Width_EN;			//汉字模式下符号宽度
	int Height = Install::Height;			//字符高度
	int LineNumber = y / Height + 1;
	//对于鼠标点击文本下方空白区域的行号计算
	if (LineNumber > hText->Line_Number() && message != UM_TEXT && message != UM_CHANGECHARSIZE)
	{
		LineNumber = hText->Line_Number();
		y = (LineNumber - 1) * Height;
	}
	//规格化y坐标
	if (message != UM_CHANGECHARSIZE)
		y = y - y % Height;
	//对消息做出的处理
	switch (message)
	{
	case UM_UP:				//上
	{
		if (LineNumber > 1)
		{
			y -= Height;
			LineNumber--;
			int LastLineWidth = hText->Line_Width(LineNumber, Width_EN);
			x = min(LastLineWidth, x);
			if (!(pCursor->isLegalCursor(LineNumber, x)))			//光标位置合法性检测 防止因上下移动使光标在汉字中间显示
			{
				x -= Width_EN;
			}
		}
		pCursor->ResetChoose();
		/*设置光标像素位置 行列号*/
		lpKernelInfo->m_pCaretPixelPos = { x,y };
		lpKernelInfo->m_cCaretCoord = { (short)pCursor->Characters_before_Cursor(LineNumber,x),(short)LineNumber };
		lpKernelInfo->m_uiCount = hText->All_Characters();
		break;
	}
	case UM_DOWN:			//下
	{
		if (LineNumber < hText->Line_Number())
		{
			y += Height;
			LineNumber++;
			int NextLineWidth = hText->Line_Width(LineNumber, Width_EN);
			x = min(NextLineWidth, x);
			if (!(pCursor->isLegalCursor(LineNumber, x)))
			{
				x -= Width_EN;
			}
		}
		pCursor->ResetChoose();
		/*设置光标像素位置 行列号*/
		lpKernelInfo->m_pCaretPixelPos = { x,y };
		lpKernelInfo->m_cCaretCoord = { (short)pCursor->Characters_before_Cursor(LineNumber,x),(short)LineNumber };
		lpKernelInfo->m_uiCount = hText->All_Characters();

		break;
	}
	case UM_LEFT:			//左
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
		/*设置光标像素位置 行列号*/
		lpKernelInfo->m_pCaretPixelPos = { x,y };
		lpKernelInfo->m_cCaretCoord = { (short)pCursor->Characters_before_Cursor(LineNumber,x) ,(short)LineNumber };
		lpKernelInfo->m_uiCount = hText->All_Characters();
	
		break;
	}
	case UM_RIGHT:			//右
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
		/*设置光标像素位置 行列号*/
		lpKernelInfo->m_pCaretPixelPos = { x,y };
		lpKernelInfo->m_cCaretCoord = { (short)pCursor->Characters_before_Cursor(LineNumber,x) ,(short)LineNumber };
		lpKernelInfo->m_uiCount = hText->All_Characters();
		break;
	}
	case UM_END:			//当前行尾时x y坐标
	{
		x = hText->Line_Width(LineNumber, Width_EN);
		pCursor->ResetChoose();
		/*设置光标像素位置 行列号*/
		lpKernelInfo->m_pCaretPixelPos = { x,y };
		lpKernelInfo->m_cCaretCoord = { (short)pCursor->Characters_before_Cursor(LineNumber,x),(short)LineNumber };
		lpKernelInfo->m_uiCount = hText->All_Characters();
		break;
	}
	case UM_RETURN:			//回车换行
	{
		Position position;
		/*对文本内的处理*/
		position.LineNumber = LineNumber;
		position.Sequence = pCursor->Characters_before_Cursor(LineNumber, x);

		Record* rd = new Record(RD_RETURN);							//记录撤销信息
		rd->Set_Choose_Data(position, position);
		pRecord->push(rd);

		position = hText->EnterNewLine(position);
		int New_Width = hText->Max_Line_Width(Width_EN);			//处理后文本最大宽度


		pCursor->ResetChoose();
		/*设置光标像素位置 行列号*/
		lpKernelInfo->m_pCaretPixelPos = { 0,y + Height };
		lpKernelInfo->m_cCaretCoord = { 0 ,(short)(LineNumber + 1) };
		/*设置换行信息 文本大小*/
		lpKernelInfo->m_bLineBreak = TRUE;
		lpKernelInfo->m_pTextPixelSize = { hText->Max_Line_Width(Width_EN),hText->Line_Number()*Height };
		lpKernelInfo->m_uiCount = hText->All_Characters();
		break;
	}
	case UM_DELETE:			//删除
	{
		int Old_Lines = hText->Line_Number();
		Record* rd = new Record(RD_DELETE);
		//选段结束横纵坐标
		int end_x = LODWORD(fParam);
		int end_y = LODWORD(sParam);
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
			delete rd;
			/*设置换行信息 文本大小*/
			lpKernelInfo->m_pCaretPixelPos = { x,y };
			lpKernelInfo->m_bLineBreak = FALSE;
			lpKernelInfo->m_pTextPixelSize = { hText->Max_Line_Width(Width_EN),hText->Line_Number()*Height };
			lpKernelInfo->m_uiCount = hText->All_Characters();
			break;
		}
		end_position = pCursor->CursorToPosition(end_x, end_y);
		//坐标相等表示向后删除
		if (pCursor->isChoose())
		{
			start_position = pCursor->start;
			end_position = pCursor->end;
			rd->Save_Delete_Data(hText, start_position, end_position);
			pRecord->push(rd);
			start_position = hText->Delete(start_position, end_position);
		}
		else if (end_x == x && end_y == y)
		{
			try
			{
				end_position = pCursor->CursorToPosition_After(end_x, end_y);
				if (end_position.LineNumber != LineNumber)			//有换行的情况  即  光标位于行尾
				{
					rd->ACT = RD_MERGE_LINE;
					rd->Save_Merge_Line_Data(hText, end_position.LineNumber);
					end_position.Sequence = 0;
				}
				else
				{
					rd->Save_Delete_Data(hText, end_position, end_position);
				}
				start_position = hText->BackSpace(end_position);
				pRecord->push(rd);
			}
			catch (std::invalid_argument& e)
			{
				//收到异常 说明当前光标位置文本末尾 对向后删除不做处理
				delete rd;
				pCursor->ResetChoose();
				/*设置换行信息 文本大小*/
				lpKernelInfo->m_pCaretPixelPos = { x,y };
				lpKernelInfo->m_bLineBreak = FALSE;
				lpKernelInfo->m_pTextPixelSize = { hText->Max_Line_Width(Width_EN),hText->Line_Number()*Height };
				lpKernelInfo->m_uiCount = hText->All_Characters();
				break;
			}
		}
		//位置相同表示退格键
		else if (end_position <= start_position)
		{
			if (end_position.Sequence == 0)
			{
				rd->ACT = RD_MERGE_LINE;
				if (end_position.LineNumber > 1)			//不记录无效的删除
				{
					rd->Save_Merge_Line_Data(hText, end_position.LineNumber);
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
			start_position = hText->BackSpace(end_position);
		}

		pCursor->ResetChoose();
		int New_Lines = hText->Line_Number();
		/*设置换行信息 文本大小*/
		lpKernelInfo->m_pCaretPixelPos = pCursor->PositionToCursor(start_position);
		lpKernelInfo->m_cCaretCoord = { short(start_position.Sequence),short(start_position.LineNumber) };
		lpKernelInfo->m_bLineBreak = (Old_Lines == New_Lines ? FALSE : TRUE);
		lpKernelInfo->m_pTextPixelSize = { hText->Max_Line_Width(Width_EN),hText->Line_Number()*Height };
		lpKernelInfo->m_uiCount = hText->All_Characters();
		break;
	}
	case UM_CURSOR:			//定位合法光标
	{
		x = pCursor->CursorLocation(LineNumber, x);
		pCursor->ResetChoose();

		/*设置光标像素位置 行列号*/
		lpKernelInfo->m_pCaretPixelPos = { x,y };
		lpKernelInfo->m_cCaretCoord = { (short)pCursor->Characters_before_Cursor(LineNumber,x) ,(short)LineNumber };
		lpKernelInfo->m_uiCount = hText->All_Characters();
		break;
	}
	case UM_HOME:		
	{
		x = 0;
		pCursor->ResetChoose();
		/*设置光标像素位置 行列号*/
		lpKernelInfo->m_pCaretPixelPos = { x,y };
		lpKernelInfo->m_cCaretCoord = { (short)pCursor->Characters_before_Cursor(LineNumber,x), (short)LineNumber };
		lpKernelInfo->m_uiCount = hText->All_Characters();
		break;
	}
	case UM_NEW:
	{

		hText->NewFile();
		pCursor->ResetChoose();
		CText::Path = wchTostring((TCHAR*)sParam);
		std::wstring DefaultName = StringToWString(Generate_Default_File_Name(CText::Path));
		DefaultName.push_back(L'\0');
		Alloc_Buffer(pBuffer, MaxBufferSize, DefaultName.size());
		WStringToWch(DefaultName, pBuffer);
		lpKernelInfo->m_lpchText = pBuffer;
		lpKernelInfo->m_uiCount = 0;
		Clear_Record(pRecord);
		break;

	}
	case UM_SAVE:
	{
		if (hText->File_Name().empty())
		{
			hText->Set_File_Name(Generate_Default_File_Name(CText::Path));
			std::wstring FileName = StringToWString(hText->File_Name());
			Alloc_Buffer(pBuffer, MaxBufferSize, FileName.size());
			WStringToWch(FileName, pBuffer);
			lpKernelInfo->m_lpchText = pBuffer;
			return UR_NOFILENAME;
		}
		else
		{
			if ((TCHAR*)sParam != NULL)
			{
				std::string FullPath = wchTostring((TCHAR*)sParam);
				std::string FileName;
				size_t pos = FullPath.rfind('\\');
				if (pos != std::string::npos)
				{
					FileName = std::string(FullPath.begin() + pos + 1, FullPath.end());
					CText::Path = std::string(FullPath.begin(), FullPath.begin() + pos + 1);
				}
				else
					return UR_ERROR;
				hText->Set_File_Name(FileName);
			}
				
			hText->Save();
		}
		break;
	}
	case UM_SAVEAS:
	{
		std::string path = wchTostring((TCHAR*)sParam);
		hText->SaveAs(path);
		break;
	}
	case UM_OPEN:
	{
		std::string FullPath = wchTostring((TCHAR*)sParam);
		std::string FileName;
		size_t pos=FullPath.rfind('\\');
		if (pos != std::string::npos)
		{
			FileName = std::string(FullPath.begin() + pos + 1, FullPath.end());
			CText::Path = std::string(FullPath.begin(), FullPath.begin() + pos + 1);
		}
		else
			return UR_ERROR;
		try
		{
			hText->ReadText(FileName);
			Clear_Record(pRecord);
		}
		catch (Read_Text_Failed e)
		{
			return UR_ERROR;
		}
		pCursor->ResetChoose();
		/*设置 文本大小 */
		lpKernelInfo->m_pTextPixelSize = { hText->Max_Line_Width(Width_EN),hText->Line_Number()*Height };
		lpKernelInfo->m_uiCount = hText->All_Characters();
		break;
		
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
		lpKernelInfo->m_lpchText = pBuffer;
		lpKernelInfo->m_uiCount = wstr_copy.size();
		break;
	}
	case UM_CHAR:
	{
		Record* rd = new Record(RD_INSERT);
		std::wstring ws;
		TCHAR wch = (TCHAR)LODWORD(sParam);			//待插入字符
		int Old_Lines = hText->Line_Number();
		//存在选中信息 插入更改为替换操作
		if (pCursor->isChoose())
		{
			ws.push_back(wch);
			rd->ACT = RD_REPLACE;
			rd->Save_Delete_Data(hText, pCursor->start, pCursor->end);	

			hText->Replace(pCursor->start, pCursor->end, ws);

			rd->Set_Choose_Data(pCursor->start, pCursor->start);
			x = (pCursor->PositionToCursor(pCursor->start)).x;
			pCursor->ResetChoose();
		}
		else
		{
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
		}
		pRecord->push(rd);
		int New_Lines = hText->Line_Number();
		/*设置光标像素位置 行列号*/
		lpKernelInfo->m_pCaretPixelPos = { x,y };
		lpKernelInfo->m_cCaretCoord = { (short)pCursor->Characters_before_Cursor(LineNumber,x) , (short)LineNumber };
		/*设置换行信息 文本大小*/
		lpKernelInfo->m_bLineBreak = (Old_Lines == New_Lines ? FALSE : TRUE);
		lpKernelInfo->m_pTextPixelSize = { hText->Max_Line_Width(Width_EN),hText->Line_Number()*Height };
		lpKernelInfo->m_uiCount = hText->All_Characters();
		break;
	}
	case UM_TEXT:
	{
		lpKernelInfo->m_lpchText = pTChar;
		//非文本区域
		if (hText->Line_Number() < LineNumber)	
		{
			pTChar[0] = L'\0';
			lpKernelInfo->m_uiCount = 0;
			break;
		}
		lpKernelInfo->m_bInside = !pCursor->isLegalCursor(LineNumber, x);
		int iCount = 0;					//显示的字符数量
		short int iStart = 0;			//高亮部分的开始点
		short int iEnd = 0;				//高亮部分结束点

		int end_x = fParam;
		x = pCursor->CursorLocation(LineNumber, x, false);
		end_x = pCursor->CursorLocation(LineNumber, end_x);
		if (x == end_x)
		{
			pTChar[0] = L'\0';
			lpKernelInfo->m_uiCount = 0;
			break;
		}
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
			lpKernelInfo->m_uiCount = 0;
			break;
		}
		position_end = pCursor->CursorToPosition(end_x, y);
		std::wstring WStr = pLine->TransformToWString(position_start.Sequence, position_end.Sequence);
		WStr.push_back(L'\0');
		WStringToWch(WStr, pTChar);
		iCount = WStr.size();
		//设置高亮部分  从0开始
		if (pCursor->isChoose())
		{
			Set_Height_Light(position_start.LineNumber, position_start, position_end, iStart, iEnd);
			lpKernelInfo->m_pStartPixelPos = pCursor->PositionToCursor_Before(pCursor->start);
			lpKernelInfo->m_pEndPixelPos = pCursor->PositionToCursor(pCursor->end);
			lpKernelInfo->m_uiCount = hText->Characters(pCursor->start, pCursor->end);
		}	
		else
			lpKernelInfo->m_uiCount = hText->All_Characters();
		lpKernelInfo->m_uiCount = iCount;
		lpKernelInfo->m_uiStart = iStart;
		lpKernelInfo->m_uiEnd = iEnd;
		break;
	}
	case UM_ALL:
	{
		pCursor->Choose(hText->First_Position(), hText->End_Position());		//全选
		/*设置光标像素位置 行列号*/
		lpKernelInfo->m_pStartPixelPos = { 0,0 };
		lpKernelInfo->m_pEndPixelPos = pCursor->PositionToCursor(pCursor->end);
		lpKernelInfo->m_pCaretPixelPos = pCursor->PositionToCursor(pCursor->end);
		lpKernelInfo->m_cCaretCoord = { short(pCursor->end.Sequence) ,short(pCursor->end.LineNumber) };
		lpKernelInfo->m_uiCount = hText->All_Characters();
		break;
	}
	case UM_CHOOSE:
	{
		Position start, end;
		int end_x = fParam;
		int end_y = sParam;
		int end_LineNumber = end_y / Height + 1;
		if (end_LineNumber > hText->Line_Number())
		{
			end_LineNumber = hText->Line_Number();
			end_y = (LineNumber - 1) * Height;
		}
		end_y -= end_y % Height;
		end_x = pCursor->CursorLocation(end_LineNumber, end_x);
		x = pCursor->CursorLocation(LineNumber, x);
		POINT p = { x, y };									//保存当前光标位置
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
			if (start > end)
				throw std::invalid_argument("无效的选中");
			else
				lpKernelInfo->m_uiCount = hText->Characters(start, end);
		}
		catch (std::invalid_argument& e)
		{
			pCursor->ResetChoose();
			lpKernelInfo->m_uiCount = hText->All_Characters();
		}				

		/*设置光标像素位置 行列号*/
		lpKernelInfo->m_pCaretPixelPos = p;
		lpKernelInfo->m_cCaretCoord = { (short)pCursor->Characters_before_Cursor(LineNumber,p.x),(short)LineNumber };
		break;
	}
	case UM_CANCEL:
	{

		if (pRecord->empty())			//当前没有待撤销的步骤
			return UR_NOTCANCEL;
		Record* p = pRecord->top();
		int Old_LineNumbers = hText->Line_Number();
		//撤销最近一次的操作
		p->ReDo(hText);
		pCursor->ResetChoose();			//清空之前的选中信息
		switch (p->ACT)
		{
			case RD_MERGE_LINE:
			{
				lpKernelInfo->m_pCaretPixelPos = pCursor->PositionToCursor_Before(p->start);
				lpKernelInfo->m_cCaretCoord = { (short)p->start.Sequence,(short)p->start.LineNumber };
				lpKernelInfo->m_uiCount = hText->All_Characters();
				break;
			}
			case RD_INSERT :
			{
				lpKernelInfo->m_pCaretPixelPos = pCursor->PositionToCursor(p->start);
				lpKernelInfo->m_cCaretCoord = { (short)p->start.Sequence,(short)p->start.LineNumber };
				lpKernelInfo->m_uiCount = hText->All_Characters();
				break;
			}
			case RD_RETURN :
			{
				lpKernelInfo->m_pCaretPixelPos = pCursor->PositionToCursor(p->start);
				lpKernelInfo->m_cCaretCoord = { (short)p->start.Sequence,(short)p->start.LineNumber };
				lpKernelInfo->m_uiCount = hText->All_Characters();
				lpKernelInfo->m_pStartPixelPos = lpKernelInfo->m_pEndPixelPos = lpKernelInfo->m_pCaretPixelPos;
				break;
			}
			case RD_DELETE:
			{
				lpKernelInfo->m_pCaretPixelPos = pCursor->PositionToCursor(p->end);
				lpKernelInfo->m_cCaretCoord = { (short)p->end.Sequence,(short)p->end.LineNumber };
				pCursor->Choose(p->start, p->end);		//鼠标设置选段信息
				lpKernelInfo->m_uiCount = hText->Characters(p->start,p->end);
				lpKernelInfo->m_pStartPixelPos = pCursor->PositionToCursor_Before(pCursor->start);
				lpKernelInfo->m_pEndPixelPos = pCursor->PositionToCursor(pCursor->end);
				break;
			}
			case RD_REPLACE:
			{
				lpKernelInfo->m_pCaretPixelPos = pCursor->PositionToCursor(p->end);
				lpKernelInfo->m_cCaretCoord = { (short)p->end.Sequence,  (short)p->end.LineNumber };
				pCursor->Choose(p->start, p->end);
				lpKernelInfo->m_uiCount = hText->Characters(p->start, p->end);
				lpKernelInfo->m_pStartPixelPos = pCursor->PositionToCursor_Before(pCursor->start);
				lpKernelInfo->m_pEndPixelPos = pCursor->PositionToCursor(pCursor->end);
				break;
			}
			default:break;
		}


		pRecord->pop();
		delete p;
		int New_LineNumbers = hText->Line_Number();
		lpKernelInfo->m_bLineBreak = (Old_LineNumbers == New_LineNumbers ? FALSE : TRUE);
		lpKernelInfo->m_pTextPixelSize = { hText->Max_Line_Width(Width_EN),hText->Line_Number()*Height };
		break;
	}
	case UM_PASTE:
	{
		int Old_Lines = hText->Line_Number();
		Record* rd = new Record(RD_INSERT);
		std::string SText = wchTostring((TCHAR*)sParam);
		std::wstring WSText = StringToWString(SText);						//待插入的内容
		TabToSpace(WSText);
		Position start, end;												//记录粘贴后最后一个字符位置
		if (pCursor->isChoose())
		{
			rd->ACT = RD_REPLACE;
			rd->Save_Delete_Data(hText, pCursor->start, pCursor->end);		//保存删除前信息
			end = hText->Replace(pCursor->start, pCursor->end, WSText);		//end记录代替后最后一个字符位置
			rd->Set_Choose_Data(pCursor->start, end);
			pCursor->ResetChoose();
		}
		else
		{
			start = pCursor->CursorToPosition(x, y);
			Position s = { start.LineNumber,start.Sequence + 1 };			//插入后的首字符位置
			end = hText->Insert(start, WSText);								//插入字符 start得到插入的最后一个字符的位置

			rd->Set_Choose_Data(s, end);
		}

		pRecord->push(rd);
		int New_Lines = hText->Line_Number();
		/*设置文本大小 当前光标位置 逻辑行列*/
		lpKernelInfo->m_pTextPixelSize = { hText->Max_Line_Width(Width_EN),hText->Line_Number()*Height };
		lpKernelInfo->m_pCaretPixelPos = { pCursor->PositionToCursor(end) };
		lpKernelInfo->m_cCaretCoord = { (short)end.Sequence,(short)end.LineNumber };
		lpKernelInfo->m_uiCount = hText->All_Characters();
		lpKernelInfo->m_bLineBreak = (Old_Lines == New_Lines ? FALSE : TRUE);
		break;
	}
	case UM_FIND:
	{
		/*
		----------------------------------------------------------------------------------------------------------------------
		|全文档查找 1 非全文档查找0	(16位)|当前光标向后查找 1 当前光标向前查找 0 （16位）|区分大小写 1 不区分大小写 0（16位）|
		---------------------------------------------------------------------------------------------------------------------
		*/

		/*设置查找模式*/
		bool upper_lower = true;
		if (WORD(sParam) == 0)
			upper_lower = false;
		Position start = hText->First_Position();
		Position end = hText->End_Position();
		/*取出待查找字符串*/
		std::string SText = wchTostring((TCHAR*)fParam);
		std::wstring Str = StringToWString(SText);
		if ((WORD(HIDWORD(sParam)) == 0))											//全文档查找
		{
			if (HIWORD(LODWORD(sParam)) > 0)										//向后查找
			{
				try 
				{
					start = pCursor->CursorToPosition_After(x, y);
				}
				catch (std::invalid_argument& e)
				{
					return UR_ERROR;					//从文本末尾开始查找 无符合匹配
				}
				if (hText->SeekStrings(Str, start, end, upper_lower))			//查找成功
				{
					pCursor->Choose(start, end);
					lpKernelInfo->m_uiCount = hText->Characters(start,end);
					lpKernelInfo->m_cCaretCoord = { (short)end.Sequence ,(short)end.LineNumber };
					lpKernelInfo->m_pStartPixelPos = pCursor->PositionToCursor_Before(pCursor->start);
					lpKernelInfo->m_pEndPixelPos = pCursor->PositionToCursor(pCursor->end);
					lpKernelInfo->m_pCaretPixelPos = lpKernelInfo->m_pEndPixelPos;
					break;
				}
			}
			else                                                                    //向前查找
			{
				end = hText->First_Position();
				start = pCursor->CursorToPosition(x, y);
				if (start.LineNumber == 1 && start.Sequence == 0)					
					return UR_ERROR;
				if (hText->ReSeekStrings(Str, start, end, upper_lower))
				{
					pCursor->Choose(start, end);
					lpKernelInfo->m_uiCount = hText->Characters(start,end);
					lpKernelInfo->m_cCaretCoord = { (short)start.Sequence ,(short)start.LineNumber };
					lpKernelInfo->m_pStartPixelPos = pCursor->PositionToCursor_Before(pCursor->start);
					lpKernelInfo->m_pEndPixelPos = pCursor->PositionToCursor(pCursor->end);
					lpKernelInfo->m_pCaretPixelPos = lpKernelInfo->m_pStartPixelPos;
					break;
				}
			}
		}		
		lpKernelInfo->m_uiCount = hText->All_Characters();
		return UR_ERROR;												//未查到
	}
	case UM_REPLACE:
	{
		/*取出替代的字符串并格式化(替换Tab)*/
		std::string SText = wchTostring((TCHAR*)sParam);
		std::wstring Str = StringToWString(SText);
		TabToSpace(Str);

		/*选中被替换的字符串起始终点位置*/
		Position start = pCursor->start;
		Position end = pCursor->end;

		/*设置撤销 保存被替换字符串*/
		Record* rd = new Record(RD_REPLACE);
		rd->Save_Delete_Data(hText, start, end);

		end = hText->Replace(start, end, Str);

		pCursor->Choose(start, end);
		lpKernelInfo->m_uiCount = hText->Characters(start, end);
		rd->Set_Choose_Data(start, end);
		pRecord->push(rd);
		/*设置 文本大小 鼠标位置 */
		lpKernelInfo->m_pTextPixelSize = { hText->Max_Line_Width(Width_EN),hText->Line_Number()*Height };
		lpKernelInfo->m_pCaretPixelPos = pCursor->PositionToCursor(end);
		lpKernelInfo->m_cCaretCoord = { short(end.Sequence),short(end.LineNumber) };
		/*设置高亮*/
		lpKernelInfo->m_pStartPixelPos = pCursor->PositionToCursor_Before(start);
		lpKernelInfo->m_pEndPixelPos = pCursor->PositionToCursor(end);
		break;
	}
	case UM_ISSAVED:
	{
		if (hText->isSaved())
			return UR_SAVED;
		else
			return UR_NOTSAVED;
	}
	case UM_CHANGECHARSIZE:
	{
		Install::Width = x;
		Install::Height = y;
		pCursor->SetWidth(x);
		pCursor->SetHeight(y);
		break;
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
	Free_Buffer(pBuffer);
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
		if (New_Size <= Old_Size / 4)		//减少缓冲区大小 避免内存占用过大
		{
			if (p != NULL)
				delete[] p;
			p = new wchar_t[New_Size];
			Old_Size = New_Size;
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





