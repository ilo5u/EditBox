//EditBox.cpp: 定义应用程序的入口点。
//

#include "stdafx.h"
#include "editbox.h"

#pragma comment(lib, "Shell32.lib")

#define MAX_LOADSTRING 100

// 全局变量: 
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名
HTEXT hText;									// 文本对象

// 此代码模块中包含的函数的前向声明: 
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	Find(HWND, UINT, WPARAM, LPARAM);
BOOL				AdjustWindowSize(HWND, POINT&, const POINT&, const POINT&);
BOOL				AdjustClientPos(HWND, POINT&, const POINT&, const POINT&, const POINT&);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_EDITBOX, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化: 
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_EDITBOX));

    MSG msg;

    // 主消息循环: 
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目的: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_EDITBOX));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_EDITBOX);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目的: 保存实例句柄并创建主窗口
//
//   注释: 
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中

   HWND hWnd = CreateWindowW(szWindowClass, szTitle,
	   WS_CAPTION |		// 标题栏
	   WS_SYSMENU |		// 窗口调整按钮
	   WS_MAXIMIZE |	// 初始最大化
	   WS_HSCROLL |		// 水平滚动条
	   WS_MINIMIZEBOX |	// 最小化按钮
	   WS_VSCROLL		// 竖直滚动条
	   ,		
      CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, SW_SHOW);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的:    处理主窗口的消息。
//
//	WM_CREATE			- 创建窗口时初始化
//	WM_SHOWWINDOW		- 清空窗口（文本也被清理）
//  WM_COMMAND			- 处理应用程序菜单
//	WN_CLOSE			- 处理应用程序关闭时的资源释放
//	WM_SIZE				- 用户调整大小时进行重绘处理
//	WM_SETFOCOUS		- 用户重新单击应用程序
//	WM_KILLFOCUS		- 用户放弃应用程序
//	WM_KEYDOWN			- 处理虚拟按键
//	WM_CHAR				- 处理字符按键
//	WM_VSCROLL			- 处理用户拖动竖直滚动条
//	WM_HSCROLL			- 处理用户拖动水平滚动条
//	WM_LBUTTONDOWM		- 处理用户单击鼠标左键
//	WM_NCLBUTTONDBLCLK	- 处理用户双击鼠标左键击中标题栏
//	WM_LBUTTONDBLCLK	- 处理用户双击鼠标左键击中显示区
//	WM_MOUSEMOVE		- 处理用户滑动鼠标
//  WM_PAINT			- 绘制主窗口
//  WM_DESTROY			- 发送退出消息并返回
//	
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// 字符模块
	static	POINT		pCharSize;
	static	HFONT		hFont;
	static	TEXTMETRIC	tm;
	// ...

	// 窗口模块
	static	POINT		pClientSize;
	static	POINT		pClientPos;
	static	POINT		pPageSize;
	static	POINT		pWindowSize;
	// ...

	// 光标模块
	static	POINT		pCaretSize;
	static	POINT		pCaretPos;
	static	POINT		pStartPos;					// 选段起点
	static	POINT		pEndPos;
	// ...

    switch (message)
    {
	case WM_CREATE:	// 创建窗口
		{
			HDC hdc = GetDC(hWnd);

			// 初始化字体相关信息
			hFont = CreateFont(
				16,		// 高度
				0,		// 宽度
				0,		// 水平
				0,		// 倾斜
				400,	// 粗度
				0,		// 不倾斜
				0,		// 无下划线
				0,		// 无中划线
				GB2312_CHARSET,					// 字符集
				OUT_DEFAULT_PRECIS,				// 输出精度
				CLIP_DEFAULT_PRECIS,			// 裁剪精度
				DEFAULT_QUALITY,				// 输出质量
				DEFAULT_PITCH | FF_DONTCARE,	// 间距
				TEXT("New Curier")				// 名称
			);
			// ...
			SelectObject(hdc, hFont);

			// 设置字符像素大小（英文）
			GetTextMetrics(hdc, &tm);
			pCharSize.x = tm.tmAveCharWidth;
			pCharSize.y = tm.tmHeight;
			// ...

			// 创建文本对象
			hText = CreateText(pCharSize.x, pCharSize.y);
			// ...
		}
		break;
	case WM_SHOWWINDOW:	// 显示窗口
		{
			RECT rc;
			GetClientRect(hWnd, &rc);
			pClientSize.x = rc.right - rc.left;
			pClientSize.y = rc.bottom - rc.top;
			// 设置页面栅格
			pPageSize.x = pClientSize.x / ZHWIDTH(pCharSize.x);
			pPageSize.y = pClientSize.y / ZHHEIGHT(pCharSize.y);
			// 设置显示区像素大小
			pClientSize.x = pPageSize.x * ZHWIDTH(pCharSize.x);
			pClientSize.y = pPageSize.y * ZHHEIGHT(pCharSize.y);
			// 设置显示区坐标
			pClientPos.x = 0;
			pCaretSize.y = 0;
			// 设置窗口大小
			pWindowSize.x = 0;
			pWindowSize.y = 0;
			// ...

			// 设置滚动条
			SCROLLINFO sInfo;
			sInfo.cbSize = sizeof(SCROLLINFO);
			sInfo.fMask = SIF_ALL;
			sInfo.nMax = 0;
			sInfo.nMin = 0;
			sInfo.nPage = pPageSize.x;
			sInfo.nPos = 0;
			sInfo.nTrackPos = 0;
			SetScrollInfo(hWnd, SB_HORZ, &sInfo, FALSE);
			ShowScrollBar(hWnd, SB_HORZ, TRUE);

			sInfo.cbSize = sizeof(SCROLLINFO);
			sInfo.fMask = SIF_ALL;
			sInfo.nMax = 0;
			sInfo.nMin = 0;
			sInfo.nPage = pPageSize.y;
			sInfo.nPos = 0;
			sInfo.nTrackPos = 0;
			SetScrollInfo(hWnd, SB_VERT, &sInfo, FALSE);
			ShowScrollBar(hWnd, SB_VERT, TRUE);
			// ...

			// 设置光标大小
			pCaretSize.x = 1;
			pCaretSize.y = ZHHEIGHT(pCharSize.y);
			// 设置光标位置
			pCaretPos.x = pClientPos.x;
			pCaretPos.y = pCaretPos.y;
			// 创建光标
			CreateCaret(hWnd, NULL, pCaretSize.x, pCaretSize.y);
			// 重绘光标
			SetCaretPos(pCaretPos.x - pClientPos.x, pCaretPos.y - pClientPos.y);
			ShowCaret(hWnd);
			// ...

			// 更新选段
			pStartPos.x = pCaretPos.x;
			pStartPos.y = pCaretPos.y;
			pEndPos.x = pStartPos.x;
			pEndPos.y = pStartPos.y;
			// ...
		}
		break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 分析菜单选择: 
            switch (wmId)
            {
			case IDM_NEW:
				{	// 新建文件
					RVALUE result;
					if ((result =
						UserMessageProc(hText, 0, 0, UM_NEW, NULL, NULL)) != UR_ERROR)
					{
						if (result == UR_NOTSAVED)
						{	// 未保存
							// 保存当前文件
							SendMessage(hWnd, WM_COMMAND, IDM_SAVE, NULL);
							if ((result =	// 再次校验
								UserMessageProc(hText, 0, 0, UM_NEW, NULL, NULL)) != UR_ERROR)
							{
								// 重绘显示区(初始化窗口)
								ShowWindow(hWnd, SW_SHOW);

								// 重绘显示区
								InvalidateRect(hWnd, NULL, TRUE);
								UpdateWindow(hWnd);
								// ...
							}
							else
							{	// 发生异常
								assert(!(result == UR_ERROR));
							}
						}
						else
						{	// 已保存
							// 重绘显示区(初始化窗口)
							ShowWindow(hWnd, SW_SHOW);

							// 重绘显示区
							InvalidateRect(hWnd, NULL, TRUE);
							UpdateWindow(hWnd);
							// ...
						}
					}
					else
					{	// 发生异常
						assert(!(result == UR_ERROR));
					}
				}
				break;
			case IDM_OPEN:
				{	// 打开文件
					// 弹出对话框
					TCHAR szFile[MAX_PATH] = {0};	// 文件名记录
					OPENFILENAME ofn = { 0 };
					ofn.lStructSize = sizeof(OPENFILENAME);
					ofn.hwndOwner = hWnd;
					ofn.lpstrTitle = TEXT("打开文件");
					ofn.lpstrFile = szFile;
					ofn.lpstrFilter = TEXT("txt文本文件(*.txt)\0");
					ofn.nMaxFile = MAX_PATH;
					ofn.nFilterIndex = 1;
					ofn.lpstrInitialDir = TEXT("./");	// 初始化路径
					ofn.Flags = OFN_FILEMUSTEXIST | OFN_EXPLORER | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;

					if (GetOpenFileName(&ofn) && lstrcmp(szFile, TEXT("")) != 0)
					{	// 选中文件
						RVALUE result;
						if ((result =
							UserMessageProc(hText, 0, 0, UM_OPEN, NULL, (SPARAM)szFile)) != UR_ERROR)
						{
							if (result == UR_NOTSAVED)
							{	// 未保存
								// 保存
								SendMessage(hWnd, WM_COMMAND, IDM_SAVE, NULL);
								if ((result =
									UserMessageProc(hText, 0, 0, UM_OPEN, NULL, (SPARAM)szFile)) != UR_ERROR)
								{
									pWindowSize.x = (LONG)LODWORD(result);
									pWindowSize.y = (LONG)HIDWORD(result);

									// 重绘滚动条
									SCROLLINFO sInfo;
									sInfo.fMask = SIF_RANGE | SIF_POS;
									GetScrollInfo(hWnd, SB_HORZ, &sInfo);
									sInfo.nMin = 0;
									sInfo.nMax = max((pWindowSize.x - pClientSize.x) / ZHWIDTH(pCharSize.x), sInfo.nMin);
									sInfo.nPos = 0;
									SetScrollInfo(hWnd, SB_HORZ, &sInfo, FALSE);
									ShowScrollBar(hWnd, SB_HORZ, TRUE);

									GetScrollInfo(hWnd, SB_VERT, &sInfo);
									sInfo.nMin = 0;
									sInfo.nMax = max((pWindowSize.y - pClientSize.y) / ZHHEIGHT(pCharSize.y), sInfo.nMin);
									sInfo.nPos = 0;
									SetScrollInfo(hWnd, SB_VERT, &sInfo, FALSE);
									ShowScrollBar(hWnd, SB_VERT, TRUE);
									// ...

									// 设置显示区位置
									pClientPos.x = 0;
									pClientPos.y = 0;
									// ...

									// 重绘光标
									pCaretPos.x = pClientPos.x;
									pCaretPos.y = pClientPos.y;
									SetCaretPos(pClientPos.x - pCaretPos.x, pClientPos.y - pCaretPos.y);
									// ...

									// 重置选段
									pStartPos.x = pCaretPos.x;
									pStartPos.y = pCaretPos.y;
									pEndPos.x = pStartPos.x;
									pEndPos.y = pStartPos.y;
									// ...

									// 重绘显示区
									InvalidateRect(hWnd, NULL, TRUE);
									UpdateWindow(hWnd);
									// ...
								}
								else
								{	// 发生异常
									assert(!(result == UR_ERROR));
								}
							}
							else
							{
								pWindowSize.x = (LONG)LODWORD(result);
								pWindowSize.y = (LONG)HIDWORD(result);

								// 重绘滚动条
								SCROLLINFO sInfo;
								sInfo.fMask = SIF_RANGE | SIF_POS;
								GetScrollInfo(hWnd, SB_HORZ, &sInfo);
								sInfo.nMin = 0;
								sInfo.nMax = max((pWindowSize.x - pClientSize.x) / ZHWIDTH(pCharSize.x), sInfo.nMin);
								sInfo.nPos = 0;
								SetScrollInfo(hWnd, SB_HORZ, &sInfo, FALSE);
								ShowScrollBar(hWnd, SB_HORZ, TRUE);

								GetScrollInfo(hWnd, SB_VERT, &sInfo);
								sInfo.nMin = 0;
								sInfo.nMax = max((pWindowSize.y - pClientSize.y) / ZHHEIGHT(pCharSize.y), sInfo.nMin);
								sInfo.nPos = 0;
								SetScrollInfo(hWnd, SB_VERT, &sInfo, FALSE);
								ShowScrollBar(hWnd, SB_VERT, TRUE);
								// ...

								// 设置显示区位置
								pClientPos.x = 0;
								pClientPos.y = 0;
								// ...

								// 重绘光标
								pCaretPos.x = pClientPos.x;
								pCaretPos.y = pClientPos.y;
								SetCaretPos(pClientPos.x - pCaretPos.x, pClientPos.y - pCaretPos.y);
								// ...

								// 重置选段
								pStartPos.x = pCaretPos.x;
								pStartPos.y = pCaretPos.y;
								pEndPos.x = pStartPos.x;
								pEndPos.y = pStartPos.y;
								// ...

								// 重绘显示区
								InvalidateRect(hWnd, NULL, TRUE);
								UpdateWindow(hWnd);
								// ...
							}
						}
						else
						{	// 发生异常
							assert(!(result == UR_ERROR));
						}
					}
					else
					{
						assert(0);
					}
				}
				break;
			case IDM_SAVE:
				{	// 保存文件
					RVALUE result;
					if ((result =
						UserMessageProc(hText, 0, 0, UM_SAVE, NULL, NULL)) != UR_ERROR)
					{
						if (result == UR_NOPATH)
						{	// 未设置保存路径
							// 另存为
							SendMessage(hWnd, WM_COMMAND, IDM_SAVEAS, NULL);
						}
						else
						{
							// 无操作
						}
					}
					else
					{	// 发生异常
						assert(!(result == UR_ERROR));
					}
				}
				break;
			case IDM_SAVEAS:
				{	// 另存为
					TCHAR szFile[MAX_PATH] = {0};	// 路径记录
					OPENFILENAME ofn;
					ofn.lStructSize = sizeof(OPENFILENAME);
					ofn.hwndOwner = hWnd;
					ofn.hInstance = NULL;
					ofn.lpstrFilter = TEXT("txt文本文件(*.txt)\0");
					ofn.nFilterIndex = 1;
					ofn.lpstrCustomFilter = NULL;
					ofn.nMaxCustFilter = 0;
					ofn.lpstrFile = szFile;
					ofn.nMaxFile = MAX_PATH;
					ofn.lpstrFileTitle = NULL;
					ofn.nMaxFileTitle = 0;
					ofn.lpstrInitialDir = NULL;
					ofn.lpstrTitle = TEXT("保存为指定的文件... ");
					ofn.Flags = OFN_HIDEREADONLY;
					ofn.nFileOffset = 0;
					ofn.nFileExtension = 0;
					ofn.lpstrDefExt = TEXT("txt ");
					ofn.lCustData = 0L;
					ofn.lpfnHook = NULL;
					ofn.lpTemplateName = NULL;

					if (GetSaveFileName(&ofn) && lstrcmp(szFile, TEXT("")) != 0)
					{
						RVALUE result;
						if ((result =
							UserMessageProc(hText, 0, 0, UM_SAVE, NULL, (SPARAM)szFile)) != UR_ERROR)
						{
							// 无界面操作
							UserMessageProc(hText, 0, 0, UM_OPEN, NULL, (SPARAM)szFile);
						}
						else
						{	// 发生异常
							assert(!(result == UR_ERROR));
						}
					}
					else
					{
						assert(0);
					}
				}
				break;
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
				SendMessage(hWnd, WM_CLOSE, NULL, NULL);
                break;
			case IDM_FIND:
				{
					DialogBox(hInst, MAKEINTRESOURCE(IDD_FINDDIALOG), hWnd, Find);
				}
				break;
			case IDM_CUT:
				break;
			case IDM_COPY:
				break;
			case IDM_PASTE:
				break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
	case WM_CLOSE:
		{
			RVALUE result;
			if ((result =
				UserMessageProc(hText, 0, 0, UM_CLOSE, NULL, NULL)) != UR_ERROR)
			{
				if (result == UR_NOTSAVED)
				{	// 未保存
					// 保存
					SendMessage(hWnd, WM_COMMAND, IDM_SAVE, NULL);
				}
				{
					// 无操作
				}
			}
			else
			{	// 发生异常
				assert(!(result == UR_ERROR));
			}
			// 销毁文本对象
			DestroyText(hText);
			// ...

			DestroyWindow(hWnd);
		}
		break;
	case WM_SIZE:
		{
			// 永不被调用
		}
		break;
	case WM_SETFOCUS:	// 获得焦点
		{
			// 新建光标
			CreateCaret(hWnd, NULL, pCaretSize.x, pCaretSize.y);
			// 重绘光标
			SetCaretPos(pClientPos.x - pCaretPos.x, pClientPos.y - pCaretPos.y);
			ShowCaret(hWnd);
			// ...
		}
		break;
	case WM_KILLFOCUS:	// 释放焦点
		{
			// 隐藏光标
			HideCaret(hWnd);
			DestroyCaret();
			// ...
		}
		break;
	case WM_KEYDOWN:
		{
			switch (wParam)
			{
				case VK_HOME:
					{	// HOME键
						RVALUE result;
						if ((result =
							UserMessageProc(hText, NULL, NULL, UM_HOME, NULL, NULL)) != UR_ERROR)
						{
							// 设置光标位置
							pCaretPos.x = (LONG)LODWORD(result);
							pCaretPos.y = (LONG)HIDWORD(result);
							// ...

							// 滑动显示区
							pClientPos.x = pCaretPos.x;
							AdjustClientPos(hWnd, pClientPos, pCaretPos, pPageSize, pCharSize);
							// ...

							// 重绘光标
							SetCaretPos(pCaretPos.x - pClientPos.x, pCaretPos.y - pClientPos.y);
							// ...

							// 更新选段
							pStartPos.x = pCaretPos.x;
							pStartPos.y = pCaretPos.y;
							pEndPos.x = pStartPos.x;
							pEndPos.y = pStartPos.y;
							// ...
						}
						else
						{	// 发生异常
							assert(!(result == UR_ERROR));
						}
					}
					break;
				case VK_END:
					{	// END键
						RVALUE result;
						if ((result = 
							UserMessageProc(hText, pCaretPos.x, pCaretPos.y, UM_END, NULL, NULL)) != UR_ERROR)
						{
							// 设置光标位置
							pCaretPos.x = (LONG)LODWORD(result);
							pCaretPos.y = (LONG)HIDWORD(result);
							// ...

							// 滑动显示区
							AdjustClientPos(hWnd, pClientPos, pCaretPos, pPageSize, pCharSize);
							// ...

							// 重绘光标
							SetCaretPos(pCaretPos.x - pClientPos.x, pCaretPos.y - pClientPos.y);
							// ...

							// 更新选段
							pStartPos.x = pCaretPos.x;
							pStartPos.y = pCaretPos.y;
							pEndPos.x = pStartPos.x;
							pEndPos.y = pStartPos.y;
							// ...
						}
						else
						{	// 异常
							assert(!(result == UR_ERROR));
						}
					}
					break;
				case VK_UP:
					{
						RVALUE result;
						if ((result = 
							UserMessageProc(hText, pCaretPos.x, pCaretPos.y, UM_UP, NULL, NULL)) != UR_ERROR)
						{
							// 设置光标位置
							pCaretPos.x = (LONG)LODWORD(result);
							pCaretPos.y = (LONG)HIDWORD(result);
							// ...

							// 向上滑动一行
							if (pCaretPos.y < pClientPos.y)
							{
								SendMessage(hWnd, WM_VSCROLL, SB_LINEUP, NULL);
							}
							// ...

							AdjustClientPos(hWnd, pClientPos, pCaretPos, pPageSize, pCharSize);		// 检测光标是否在显示区内并调整显示区的位置
							SetCaretPos(pCaretPos.x - pClientPos.x, pCaretPos.y - pClientPos.y);	// 重绘光标

							// 更新选段
							pStartPos.x = pCaretPos.x;
							pStartPos.y = pCaretPos.y;
							pEndPos.x = pStartPos.x;
							pEndPos.y = pStartPos.y;
							// ...
						}
						else
						{
							assert(!(result == UR_ERROR));
						}
					}
					break;
				case VK_DOWN:
					{
						RVALUE result;
						if ((result = 
							UserMessageProc(hText, pCaretPos.x, pCaretPos.y, UM_DOWN, NULL, NULL)) != UR_ERROR)
						{
							// 设置光标
							pCaretPos.x = (LONG)LODWORD(result);
							pCaretPos.y = (LONG)HIDWORD(result);
							// ...

							// 向下滑动一行
							if (pCaretPos.y > pClientPos.y + pPageSize.y * ZHHEIGHT(pCharSize.y))
							{
								SendMessage(hWnd, WM_VSCROLL, SB_LINEDOWN, NULL);
							}
							// ...

							AdjustClientPos(hWnd, pClientPos, pCaretPos, pPageSize, pCharSize);		// 检测光标是否在显示区内并调整显示区的位置
							SetCaretPos(pCaretPos.x - pClientPos.x, pCaretPos.y - pClientPos.y);	// 重绘光标

							// 更新选段
							pStartPos.x = pCaretPos.x;
							pStartPos.y = pCaretPos.y;
							pEndPos.x = pStartPos.x;
							pEndPos.y = pStartPos.y;
							// ...
						}
						else
						{	// 光标处于底层
							assert(!(result == UR_ERROR));
						}
					}
					break;
				case VK_RIGHT:
					{
						RVALUE result;
						if ((result =
							UserMessageProc(hText, pCaretPos.x, pCaretPos.y, UM_RIGHT, NULL, NULL)) != UR_ERROR)
						{
							// 设置光标位置
							pCaretPos.x = (LONG)LODWORD(result);
							pCaretPos.y = (LONG)HIDWORD(result);
							// ...

							AdjustClientPos(hWnd, pClientPos, pCaretPos, pPageSize, pCharSize); // 滑动显示区
							SetCaretPos(pCaretPos.x - pClientPos.x, pCaretPos.y - pClientPos.y); // 重绘光标

							// 更新选段
							pStartPos.x = pCaretPos.x;
							pStartPos.y = pCaretPos.y;
							pEndPos.x = pStartPos.x;
							pEndPos.y = pStartPos.y;
							// ...
						}
						else
						{	
							assert(!(result == UR_ERROR));
						}
					}
					break;
				case VK_LEFT:
					{
						RVALUE result;
						if ((result =
							UserMessageProc(hText, pCaretPos.x, pCaretPos.y, UM_LEFT, NULL, NULL)) != UR_ERROR)
						{
							// 设置光标位置
							pCaretPos.x = (LONG)LODWORD(result);
							pCaretPos.y = (LONG)HIDWORD(result);
							// ...

							AdjustClientPos(hWnd, pClientPos, pCaretPos, pPageSize, pCharSize); // 滑动显示区
							SetCaretPos(pCaretPos.x - pClientPos.x, pCaretPos.y - pClientPos.y); // 重绘光标

							// 更新选段
							pStartPos.x = pCaretPos.x;
							pStartPos.y = pCaretPos.y;
							pEndPos.x = pStartPos.x;
							pEndPos.y = pStartPos.y;
							// ...
						}
						else
						{	// 发生异常
							assert(!(result == UR_ERROR));
						}
					}
					break;
				case VK_DELETE:
					{
						RVALUE result;
						if ((result =
							UserMessageProc(hText, pStartPos.x, pStartPos.y, UM_DELETE, NULL, (SPARAM)((SPARAM)pEndPos.y << 32 | (SPARAM)pEndPos.x))) != UR_ERROR)
						{
							// 获取窗口大小
							pWindowSize.x = (LONG)LODWORD(result);
							pWindowSize.y = (LONG)HIDWORD(result);
							// ...

							AdjustWindowSize(hWnd, pWindowSize, pClientSize, pCharSize);			// 调整显示区大小
							AdjustClientPos(hWnd, pClientPos, pCaretPos, pPageSize, pCharSize);		// 滑动显示区
							SetCaretPos(pCaretPos.x - pClientPos.x, pCaretPos.y - pClientPos.y);	// 重绘光标

							// 更新选段
							pStartPos.x = pCaretPos.x;
							pStartPos.y = pCaretPos.y;
							pEndPos.x = pStartPos.x;
							pEndPos.y = pStartPos.y;
							// ...
						}
						else
						{	// 发生异常
							assert(!(result == UR_ERROR));
						}
					}
					break;
				default:
					break;
			}
		}
		break;
	case WM_CHAR:
		{
			//for (int i = 0; i < (int)LOWORD(lParam); ++i)
			//{
				switch (wParam)
				{
				case '\b':
					{
						// 调整光标 -> 挪至上侧
						if (pEndPos.y < pStartPos.y || 
							(pEndPos.y == pEndPos.y && pEndPos.x < pStartPos.x))
						{
							std::swap(pStartPos.x, pEndPos.x);
							std::swap(pStartPos.y, pEndPos.y);
						}
						pCaretPos.x = pStartPos.x;
						pCaretPos.y = pStartPos.y;
						// ...

						// 判断选段是否为空
						if (pStartPos.x == pEndPos.x &&
							pStartPos.y == pEndPos.y)
						{	// 光标左移
							POINT temp = pEndPos;
							SendMessage(hWnd, WM_KEYDOWN, VK_LEFT, NULL);
							// ...

							// 重置选段
							pEndPos.x = temp.x;
							pEndPos.y = temp.y;
							// ...
						}
						// ...

						// 若光标未在显示区 滑动显示区 -> 将光标显示在中部
						AdjustClientPos(hWnd, pClientPos, pCaretPos, pPageSize, pCharSize); // 滑动显示区
						SetCaretPos(pCaretPos.x - pClientPos.x, pCaretPos.y - pClientPos.y); // 重绘光标

						// 执行删除
						if (pStartPos.x == 0 && pEndPos.x == 0 && pStartPos.y == 0 && pEndPos.y == 0)
							return DefWindowProc(hWnd, message, wParam, lParam);
						else
							SendMessage(hWnd, WM_KEYDOWN, VK_DELETE, NULL);
						// ...

						// 更新选段
						pStartPos.x = pCaretPos.x;
						pStartPos.y = pCaretPos.y;
						pEndPos.x = pStartPos.x;
						pEndPos.y = pStartPos.y;
						// ...
					}
					break;
				case '\t':
					{
						do
						{
							SendMessage(hWnd, WM_CHAR, ' ', NULL);
						} while (pCaretPos.x % (USWIDTH(pCharSize.x) << 2) != 0);
					}
					break;
				case '\n':
					{
					}
					break;
				case '\r':
					{
						RVALUE result;
						if (result = 
							(UserMessageProc(hText, pCaretPos.x, pCaretPos.y, UM_RETURN, NULL, NULL)) != UR_ERROR)
						{
							// 获取窗口大小
							pWindowSize.x = (LONG)LODWORD(result);
							pWindowSize.y = (LONG)HIDWORD(result);
							// ...

							AdjustWindowSize(hWnd, pWindowSize, pClientSize, pCharSize);			// 调整显示区大小

							// 移动光标
							SendMessage(hWnd, WM_KEYDOWN, VK_RIGHT, NULL);
							// ...
						}
						else
						{	// 发生异常
							assert(!(result == UR_ERROR));
						}
					}
					break;
				default:
					{
						RVALUE result;
						if (GetKeyState(VK_CONTROL) < 0)
						{	// 非字符处理（控制命令/快捷键）
							if (TCHAR(wParam) == TEXT('A') || TCHAR(wParam) == TEXT('a'))
							{	// Ctrl + A 全选
								if ((result =
									UserMessageProc(hText, NULL, NULL, UM_ALL, NULL, NULL)) != UR_ERROR)
								{
									// 设置光标位置
									pCaretPos.x = (LONG)LODWORD(result);
									pCaretPos.y = (LONG)HIDWORD(result);
									// ...

									// 检测光标是否在显示区内并调整显示区的位置
									AdjustClientPos(hWnd, pClientPos, pCaretPos, pPageSize, pCharSize); // 滑动显示区
									SetCaretPos(pCaretPos.x - pClientPos.x, pCaretPos.y - pClientPos.y); // 重绘光标

																										 // 重置选段
									pStartPos.x = 0;
									pStartPos.y = 0;
									pEndPos.x = pCaretPos.x;
									pEndPos.y = pCaretPos.y;
									// ...
								}
								else
								{
									assert(!(result == UR_ERROR));
								}
								break;	// 跳出 不做字符处理
							}
							else if (TCHAR(wParam) == TEXT('C') || TCHAR(wParam) == TEXT('c'))
							{	// Ctrl + C 拷贝
								if (OpenClipboard(hWnd))
								{	// 打开剪切板
									if (EmptyClipboard())
									{	// 清空剪切板
										LPWSTR lpString = nullptr;
										if ((result =
											UserMessageProc(hText, pStartPos.x, pStartPos.y, UM_COPY, (FPARAM)((FPARAM)pEndPos.y << 32 | (FPARAM)pEndPos.x), (SPARAM)&lpString)) != UR_ERROR)
										{	// 拷贝到剪切板
											// 字节数目
											int nSize = 2 * LODWORD(result) + 1;
											HGLOBAL hMem = GlobalAlloc(GHND, nSize);
											LPSTR pData = (LPSTR)GlobalLock(hMem);

											// 拷贝
											memcpy_s(pData, nSize, lpString, (nSize - 1) / 2);
											pData[nSize - 1] = '\0';
											// ...

											GlobalUnlock(hMem);
											SetClipboardData(CF_UNICODETEXT, hMem);

											// 关闭剪切板
											CloseClipboard();
											GlobalFree(hMem);
											// ...
										}
										else
										{	// 发生异常
											assert(!(result == UR_ERROR));
										}
									}
								}
								break;	// 跳出 不做字符处理
							}
							else if (TCHAR(wParam) == TEXT('V') || TCHAR(wParam) == TEXT('v'))
							{	// Ctrl + V 粘贴
								if (OpenClipboard(hWnd))
								{	// 从剪切板拷贝
									HANDLE hClipMemory = GetClipboardData(CF_UNICODETEXT);
									// 获取字节数目
									SIZE_T dwLength = GlobalSize(hClipMemory);
									// ...

									LPBYTE lpClipMemory = (LPBYTE)GlobalLock(hClipMemory);
									LPWSTR lpString = new TCHAR[dwLength / 2 + 1];

									// 拷贝
									memcpy_s(lpString, dwLength / 2, lpClipMemory, dwLength);
									lpString[dwLength / 2] = TEXT('\0');
									// ...

									// 关闭剪切板
									GlobalUnlock(hClipMemory);
									CloseClipboard();
									// ...

									// 重置选段
									pStartPos.x = pCaretPos.x;
									pStartPos.y = pCaretPos.y;
									// ...

									if ((result =
										UserMessageProc(hText, pCaretPos.x, pCaretPos.y, UM_PASTE, (FPARAM)&pEndPos, (SPARAM)lpString)) != UR_ERROR)
									{
										// 获取窗口大小
										pWindowSize.x = (LONG)LODWORD(result);
										pWindowSize.y = (LONG)HIDWORD(result);
										// ...

										// 设置光标位置
										pCaretPos.x = pEndPos.x;
										pCaretPos.y = pEndPos.y;
										// ...

										AdjustWindowSize(hWnd, pWindowSize, pClientSize, pCharSize);		// 调整显示区大小
										AdjustClientPos(hWnd, pClientPos, pCaretPos, pPageSize, pCharSize);	// 检测光标是否在显示区内并调整显示区的位置
									}
									else
									{	// 发生异常
										assert(!(result == UR_ERROR));
									}

									delete[] lpString;	// 释放临时内存
								}
								break;	// 跳出 不做字符处理
							}
							else if (TCHAR(wParam) == TEXT('X') || TCHAR(wParam) == TEXT('x'))
							{	// Ctrl + X 剪切
									// 设置Ctrl被按下
								BYTE keyState[256] = { 0 };
								keyState[VK_CONTROL] = -1;
								SetKeyboardState(keyState);
								// ...

								SendMessage(hWnd, WM_CHAR, TEXT('C'), NULL);
								SendMessage(hWnd, WM_CHAR, TEXT('\b'), NULL);

								// 恢复Ctrl按键
								keyState[VK_CONTROL] = 0;
								SetKeyboardState(keyState);
								// ...

								break;	// 跳出 不做字符处理
							}
							else if (TCHAR(wParam) == TEXT('Z') || TCHAR(wParam) == TEXT('z'))
							{	// Ctrl + Z 撤销
								if ((result =
									UserMessageProc(hText, NULL, NULL, UM_CANCEL, (FPARAM)&pStartPos, (SPARAM)&pEndPos)) != UR_ERROR)
								{
									// 获取窗口大小
									pWindowSize.x = (LONG)LODWORD(result);
									pWindowSize.y = (LONG)HIDWORD(result);
									// ...

									// 设置光标位置
									pCaretPos.x = pEndPos.x;
									pCaretPos.y = pEndPos.y;
									// ...

									AdjustWindowSize(hWnd, pWindowSize, pClientSize, pCharSize);		// 调整显示区大小
									AdjustClientPos(hWnd, pClientPos, pCaretPos, pPageSize, pCharSize);	// 检测光标是否在显示区内并调整显示区的位置
								}
								break;	// 跳出 不做字符处理
							}
							else if (TCHAR(wParam) == TEXT('F') || TCHAR(wParam) == TEXT('f'))
							{	// Ctrl + F 查找
								SendMessage(hWnd, WM_COMMAND, IDM_FIND, NULL);
								break;	// 跳出 不做字符处理
							}
							else if (TCHAR(wParam) == TEXT('S') || TCHAR(wParam) == TEXT('s'))
							{	// Ctrl + S 保存
								SendMessage(hWnd, WM_COMMAND, IDM_SAVE, NULL);
								break;	// 跳出 不做字符处理
							}
						}
						else
						{
							// 字符处理
							if ((result =
								UserMessageProc(hText, pCaretPos.x, pCaretPos.y, UM_CHAR, NULL, (SPARAM)wParam)) != UR_ERROR)
							{
								// 获取窗口大小
								pWindowSize.x = (LONG)LODWORD(result);
								pWindowSize.y = (LONG)HIDWORD(result);
								// ...

								AdjustWindowSize(hWnd, pWindowSize, pClientSize, pCharSize); // 调整显示区大小

								// 移动光标
								SendMessage(hWnd, WM_KEYDOWN, VK_RIGHT, NULL);
								// ...
							}
							else
							{	// 发生异常
								assert(!(result == UR_ERROR));
							}
						}
					}
					break;
				}
			//}
		}
		break;
	case WM_VSCROLL:
		{
			// 获取滚动条信息
			SCROLLINFO	sInfo;
			sInfo.fMask = SIF_ALL;
			GetScrollInfo(hWnd, SB_VERT, &sInfo);
			// ...

			int iVertPos = sInfo.nPos;	// 记录初始位置
			switch (LOWORD(wParam))
			{
			case SB_LINEUP:
				{
					sInfo.nPos = max(sInfo.nPos - 1, sInfo.nMin);
				}
				break;
			case SB_LINEDOWN:
				{
					sInfo.nPos = min(sInfo.nPos + 1, sInfo.nMax);
				}
				break;
			case SB_PAGEUP:
				{
					sInfo.nPos = max(sInfo.nPos - (int)sInfo.nPage, sInfo.nMin);
				}
				break;
			case SB_PAGEDOWN:
				{
					sInfo.nPos = min(sInfo.nPos + (int)sInfo.nPage, sInfo.nMax);
				}
				break;
			case SB_TOP:
				{
					sInfo.nPos = sInfo.nMin;
				}
				break;
			case SB_BOTTOM:
				{
					sInfo.nPos = sInfo.nMax;
				}
				break;
			case SB_THUMBTRACK:
				{
					if (sInfo.nTrackPos < sInfo.nMin)
						sInfo.nPos = sInfo.nMin;
					else if (sInfo.nTrackPos > sInfo.nMax)
						sInfo.nPos = sInfo.nMax;
					else
						sInfo.nPos = sInfo.nTrackPos;
				}
			default:
				break;
			}
			// 更新显示区位置纵坐标
			pClientPos.y = pClientPos.y + (sInfo.nPos - iVertPos) * ZHHEIGHT(pCharSize.y);
			// ...

			// 更新滚动条
			sInfo.fMask = SIF_POS;
			SetScrollInfo(hWnd, SB_VERT, &sInfo, FALSE);
			ShowScrollBar(hWnd, SB_VERT, TRUE);
			// ...

			// 重绘显示区
			RECT rc;
			GetClientRect(hWnd, &rc);
			ScrollWindowEx(hWnd, 0, sInfo.nPos - iVertPos, &rc, NULL, NULL, NULL, SW_INVALIDATE | SW_ERASE);
			UpdateWindow(hWnd);
			// ...
		}
		break;
	case WM_HSCROLL:
		{
			// 获取滚动条信息
			SCROLLINFO sInfo;
			sInfo.fMask = SIF_ALL;
			GetScrollInfo(hWnd, SB_HORZ, &sInfo);
			// ...

			int iVertPos = sInfo.nPos;	// 记录初始位置
			switch (LOWORD(wParam))
			{
			case SB_LINELEFT:
				{
					sInfo.nPos = max(sInfo.nPos - 1, sInfo.nMin);
				}
				break;
			case SB_LINERIGHT:
				{
					sInfo.nPos = min(sInfo.nPos + 1, sInfo.nMax);
				}
				break;
			case SB_PAGELEFT:
				{
					sInfo.nPos = max(sInfo.nPos - (int)sInfo.nPage, sInfo.nMin);
				}
				break;
			case SB_PAGERIGHT:
				{
					sInfo.nPos = min(sInfo.nPos + (int)sInfo.nPage, sInfo.nMax);
				}
				break;
			case SB_LEFT:
				{
					sInfo.nPos = sInfo.nMin;
				}
				break;
			case SB_RIGHT:
				{
					sInfo.nPos = sInfo.nMax;
				}
				break;
			case SB_THUMBTRACK:
				{
					if (sInfo.nTrackPos < sInfo.nMin)
						sInfo.nPos = sInfo.nMin;
					else if (sInfo.nTrackPos > sInfo.nMax)
						sInfo.nPos = sInfo.nMax;
					else
						sInfo.nPos = sInfo.nTrackPos;
				}
				break;
			default:
				break;
			}
			// 更新显示区位置纵坐标
			pClientPos.x = pClientPos.x + (sInfo.nPos - iVertPos) * ZHWIDTH(pCharSize.x);
			// ...

			// 更新滚动条
			sInfo.fMask = SIF_POS;
			SetScrollInfo(hWnd, SB_HORZ, &sInfo, FALSE);
			ShowScrollBar(hWnd, SB_HORZ, TRUE);
			// ...

			// 重绘显示区
			RECT rc;
			GetClientRect(hWnd, &rc);
			ScrollWindowEx(hWnd, sInfo.nPos - iVertPos, 0, &rc, NULL, NULL, NULL, SW_INVALIDATE | SW_ERASE);
			UpdateWindow(hWnd);
			// ...
		}
		break;
	case WM_LBUTTONDOWN:
		{
			// 解析鼠标点击位置
			POINT p;
			GetCursorPos(&p);
			ScreenToClient(hWnd, &p);
			if (!INRANGE(p.x, 0, pClientSize.x) ||
				!INRANGE(p.y, 0, pClientSize.y))
			{	// 未击中显示区
				return DefWindowProc(hWnd, message, wParam, lParam);
			}
			p.x += pClientPos.x;
			p.y += pClientPos.y;
			// ...

			//* DEBUG
			HDC hdc = GetDC(hWnd);
			TCHAR out[100] = { 0 };
			wsprintf(out, TEXT("%8d %8d"), p.x, p.y);
			MessageBox(hWnd, out, TEXT("Cursor Position"), MB_OKCANCEL);
			ReleaseDC(hWnd, hdc);
			//*/

			RVALUE result;
			if ((result = 
				UserMessageProc(hText, p.x, p.y, UM_CURSOR, NULL, NULL)) != UR_ERROR)
			{	
				// 设置光标位置
				pCaretPos.x = (LONG)LODWORD(result);
				pCaretPos.y = (LONG)HIDWORD(result);
				// ...
				AdjustClientPos(hWnd, pClientPos, pCaretPos, pPageSize, pCharSize);		// 滑动显示区
				SetCaretPos(pCaretPos.x - pClientPos.x, pCaretPos.y - pClientPos.y);	// 重绘光标

				// 更新选段
				pStartPos.x = pCaretPos.x;
				pStartPos.y = pCaretPos.y;
				pEndPos.x = pStartPos.x;
				pEndPos.y = pStartPos.y;
				// ...
			}
			else
			{	// 发生异常
				assert(!(result == UR_ERROR));
			}
		}
		break;
	case WM_LBUTTONUP:
		{
		}
		break;
	case WM_NCLBUTTONDBLCLK:
		{
			if (HTCAPTION == wParam)
			{

			}
			else
			{
				return DefWindowProc(hWnd, message, wParam, lParam);
			}
		}
		break;
	case WM_NCLBUTTONDOWN:
		{
			if (HTCAPTION == wParam)
			{

			}
			else
			{
				return DefWindowProc(hWnd, message, wParam, lParam);
			}
		}
		break;
	case WM_MOUSEMOVE:
		{
			switch (wParam)
			{
			case MK_LBUTTON:
				{	// 按住左键
					POINT p;	// 用户鼠标落点
					GetCursorPos(&p);
					ScreenToClient(hWnd, &p);
					// ...

					if (p.y > pClientSize.y)
					{	// 下移
						SendMessage(hWnd, WM_VSCROLL, SB_LINEDOWN, NULL);
					}
					else if (p.x > pClientSize.x)
					{	// 右移
						SendMessage(hWnd, WM_HSCROLL, SB_LINERIGHT, NULL);
					}
					else if (p.y < 0)
					{	// 上移
						SendMessage(hWnd, WM_VSCROLL, SB_LINEUP, NULL);
					}
					else if (p.x < 0)
					{	// 下移
						SendMessage(hWnd, WM_HSCROLL, SB_LINELEFT, NULL);
					}
					// 用户鼠标落点转化为绝对坐标
					p.x += pClientPos.x;
					p.y += pClientPos.y;

					RVALUE result;
					if ((result =
						UserMessageProc(hText, p.x, p.y, UM_CHOOSE, (FPARAM)(((FPARAM)pStartPos.y) << 32 | (FPARAM)pStartPos.x), NULL)) != UR_ERROR)
					{
						// 更新光标
						pCaretPos.x = (LONG)LODWORD(result);
						pCaretPos.y = (LONG)HIDWORD(result);
						SetCaretPos(pCaretPos.x - pClientPos.x, pCaretPos.y - pClientPos.y);
						// ...

						// 更新选段
						pEndPos.x = pCaretPos.x;
						pEndPos.y = pCaretPos.y;
						// ...

						// 重绘显示区
						InvalidateRect(hWnd, NULL, TRUE);
						UpdateWindow(hWnd);
						// ...
					}
					else
					{	// 发生异常
						assert(!(result == UR_ERROR));
					}
				}
				break;
			default:
				return DefWindowProc(hWnd, message, wParam, lParam);
			}
		}
		break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 在此处添加使用 hdc 的任何绘图代码...
			SelectObject(hdc, hFont);

			// 获得无效区域
			RECT rc = ps.rcPaint;
			LPWSTR szBuffer = nullptr;
			RVALUE result;
			for (int row = rc.top; row <= rc.bottom; row += ZHHEIGHT(pCharSize.y))
			{
				if ((result = 
					UserMessageProc(hText, rc.left + pClientPos.x, row + pClientPos.y,
						UM_TEXT, (FPARAM)((FPARAM)(row + pClientPos.y) << 32 | (FPARAM)(pClientSize.x + pClientPos.x)), (SPARAM)(&szBuffer))) != UR_ERROR)
				{
					int iCount		= (int)HIDWORD(result);
					short iStart	= (short)LOWORD(LODWORD(result));
					short iEnd		= (short)HIWORD(LODWORD(result));

					MyTextOut(hdc, rc.left + pClientPos.x, row + pClientPos.y, szBuffer, iCount, iStart, iEnd, ZHWIDTH(pCharSize.x));
				}
			}

			//* DEBUG
			TCHAR out[100] = { 0 };
			memset(out, 0x0, _countof(out));
			wsprintf(out, TEXT("Caret's X=%6d Y=%6d"), pCaretPos.x, pCaretPos.y);
			TextOut(hdc, pClientSize.x - pCharSize.x * 40, pClientSize.y - pCharSize.y * 1, out, lstrlen(out));

			memset(out, 0x0, _countof(out));
			wsprintf(out, TEXT("Window's X=%6d Y=%6d"), pClientPos.x, pClientPos.y);
			TextOut(hdc, pClientSize.x - pCharSize.x * 40, pClientSize.y - pCharSize.y * 2, out, lstrlen(out));

			SCROLLINFO sInfo;
			sInfo.fMask = SIF_ALL;
			GetScrollInfo(hWnd, SB_HORZ, &sInfo);

			memset(out, 0x0, _countof(out));
			wsprintf(out, TEXT("Hscroll's pos=%6d max=%6d"), sInfo.nPos, sInfo.nMax);
			TextOut(hdc, pClientSize.x - pCharSize.x * 40, pClientSize.y - pCharSize.y * 3, out, lstrlen(out));

			GetScrollInfo(hWnd, SB_VERT, &sInfo);
			memset(out, 0x0, _countof(out));
			wsprintf(out, TEXT("Vscroll's pos=%6d max=%6d"), sInfo.nPos, sInfo.nMax);
			TextOut(hdc, pClientSize.x - pCharSize.x * 40, pClientSize.y - pCharSize.y * 4, out, lstrlen(out));
			//*/

            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
		{
			DestroyCaret();
			PostQuitMessage(0);
		}
        break;
    default:
		return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

// “查找”框的消息处理程序。
INT_PTR CALLBACK Find(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		{
			if (LOWORD(wParam) == IDOK)
			{
				EndDialog(hDlg, LOWORD(wParam));
				return (INT_PTR)TRUE;
			}
			else if (LOWORD(wParam) == IDCANCEL)
			{
				EndDialog(hDlg, LOWORD(wParam));
				return (INT_PTR)TRUE;
			}
		}
		break;
	}
	return (INT_PTR)FALSE;
}

BOOL AdjustWindowSize(HWND hWnd, POINT& pWindowSize, const POINT& pClientSize, const POINT& pCharSize)
{
	// 调整显示区大小
	SCROLLINFO sInfo;
	sInfo.fMask = SIF_RANGE;
	GetScrollInfo(hWnd, SB_HORZ, &sInfo);
	// 调整滚动条范围
	if (sInfo.nMax != pWindowSize.x / ZHWIDTH(pCharSize.x))
	{
		sInfo.nMax = max((pWindowSize.x - pClientSize.x) / ZHWIDTH(pCharSize.x), 0);
		SetScrollInfo(hWnd, SB_HORZ, &sInfo, FALSE);
		ShowScrollBar(hWnd, SB_HORZ, TRUE);
	}

	GetScrollInfo(hWnd, SB_VERT, &sInfo);
	// 调整滚动条范围
	if (sInfo.nMax != pWindowSize.y / ZHWIDTH(pCharSize.y))
	{
		sInfo.nMax = max((pWindowSize.y - pClientSize.y) / ZHWIDTH(pCharSize.y), 0);
		SetScrollInfo(hWnd, SB_VERT, &sInfo, FALSE);
		ShowScrollBar(hWnd, SB_VERT, TRUE);
	}
	// ...
	return (TRUE);
}

BOOL AdjustClientPos(HWND hWnd, POINT& pClientPos, const POINT& pCaretPos, const POINT& pPageSize, const POINT& pCharSize)
{
	if (!INRANGE(pCaretPos.x, pClientPos.x, pClientPos.x + pPageSize.x * ZHWIDTH(pCharSize.x)))
	{
		int xOffset =
			pCaretPos.x - pClientPos.x - (pPageSize.x >> 1) * ZHWIDTH(pCharSize.x);

		SCROLLINFO sInfo;
		sInfo.fMask = SIF_POS | SIF_TRACKPOS;

		GetScrollInfo(hWnd, SB_HORZ, &sInfo);
		sInfo.nTrackPos = sInfo.nPos + xOffset / ZHWIDTH(pCharSize.x);
		SetScrollInfo(hWnd, SB_HORZ, &sInfo, FALSE);
		SendMessage(hWnd, WM_HSCROLL, SB_THUMBTRACK, NULL);
	}

	if (!INRANGE(pCaretPos.y, pClientPos.y, pClientPos.y + pPageSize.y * ZHHEIGHT(pCharSize.y)))
	{
		int yOffset =
			pCaretPos.y - pClientPos.y - (pPageSize.y >> 1) * ZHHEIGHT(pCharSize.y);

		SCROLLINFO sInfo;
		sInfo.fMask = SIF_POS | SIF_TRACKPOS;

		GetScrollInfo(hWnd, SB_VERT, &sInfo);
		sInfo.nTrackPos = sInfo.nPos + yOffset / ZHHEIGHT(pCharSize.y);
		SetScrollInfo(hWnd, SB_VERT, &sInfo, FALSE);
		SendMessage(hWnd, WM_VSCROLL, SB_THUMBTRACK, NULL);
	}
	return (TRUE);
}
