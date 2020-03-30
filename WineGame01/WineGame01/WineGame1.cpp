#include <windows.h>
#include <GdiPlus.h>
#include "resource.h"

using namespace Gdiplus;

#pragma comment(lib,"gdiplus")

HINSTANCE	g_hInst;
HWND		g_hWnd;

#define ID_CLOSE_BTN	100
#define ID_RESET_BTN	101
#define ID_IMGFILE_BTN	102
#define ID_ALLOPEN_BTN	103

#define ID_4BY4			200
#define ID_5BY5			201
#define ID_6BY6			202
#define ID_7BY7			203
#define ID_8BY8			204

#define ID_BLOCK_BTN	300

CachedBitmap* g_pCachedBitmap;
WCHAR	g_strBGFileName[256];
int		g_BlockSize = 8;
int     g_nSelectBlockID = -1;

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow);
LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
	g_hInst = hInstance;

	ULONG_PTR g_GdiPlusTokenBoxData;
	GdiplusStartupInput GdiplusStartupInput;
	GdiplusStartup(&g_GdiPlusTokenBoxData, &GdiplusStartupInput, NULL);

	HWND	 hWnd;	
	WNDCLASS WndClass;

	WndClass.cbClsExtra=0;
	WndClass.cbWndExtra=0;
	WndClass.hbrBackground=(HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.hCursor=LoadCursor(NULL,IDC_ARROW);
	WndClass.hIcon=LoadIcon(NULL,IDI_APPLICATION);
	WndClass.hInstance=hInstance;
	WndClass.lpfnWndProc=(WNDPROC)WndProc;
	WndClass.lpszClassName=L"대구와인동호회게임1";
	WndClass.lpszMenuName=NULL;
	WndClass.style=CS_HREDRAW | CS_VREDRAW;
	RegisterClass(&WndClass);

	hWnd=CreateWindow(L"대구와인동호회게임1",L"대구와인동호회게임1",WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,CW_USEDEFAULT,1024,768,
		NULL,(HMENU)NULL,hInstance,NULL);
	ShowWindow(hWnd,nCmdShow);

	MSG Message;
	while(GetMessage(&Message,0,0,0)) {
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
	GdiplusShutdown(g_GdiPlusTokenBoxData);
	return (int)Message.wParam;
}

void UpdateScreen()
{	
	Bitmap* pBGBit = (Bitmap*)Image::FromFile(L"wine.jpg");
	Bitmap* pObjBit= (Bitmap*)Image::FromFile(g_strBGFileName);


//	ImageAttributes imgAttr; 
//	imgAttr.SetColorKey(Color(255,255,255),Color(255,255,255));

	Graphics* pMemGraphic = new Graphics(pBGBit);

	pMemGraphic->DrawImage(pBGBit,0,0);
	
	Rect rect(25,25,640,640);
	pMemGraphic->DrawImage(pObjBit,rect,25,25,pObjBit->GetWidth(),pObjBit->GetHeight(),
		UnitPixel,NULL);

	if(g_pCachedBitmap)
	{
		delete g_pCachedBitmap;
	}

	Graphics graphic(g_hWnd);
	g_pCachedBitmap = new CachedBitmap(pBGBit, &graphic);


//	delete pObjBit;
	delete pBGBit;
	delete pMemGraphic;
	InvalidateRect(g_hWnd,NULL,TRUE);
}

void OnPaint(HDC hdc)
{
	Graphics graphic(hdc);
	if(g_pCachedBitmap == NULL)
	{
		UpdateScreen();
	}
	graphic.DrawCachedBitmap(g_pCachedBitmap,0,0);
}

void FileOpen(HWND hWnd)
{
	OPENFILENAME OFN;
	WCHAR lpstrFile[256] = L"";

	memset(&OFN, 0, sizeof(OPENFILENAME));
	OFN.lStructSize = sizeof(OPENFILENAME);
	OFN.hwndOwner = hWnd;
	OFN.lpstrFilter = L"그림 파일(*.jpg;*.bmp;*.gif)\0*.jpg;*.gif;*.bmp\0";
	OFN.lpstrFile = lpstrFile;
	OFN.nMaxFile = 256;

	if(GetOpenFileName(&OFN) != 0)
	{
		// TODO : 불러온 파일을 화면에 뿌린다.
		
		wcscpy_s(g_strBGFileName,256,OFN.lpstrFile);
		UpdateScreen();
	}
}

int CreateBlock(HWND hWnd, int blockSize, int blockWidth, int blockHeight)
{
	for(int j = 0; j < blockSize; j++)
	{
		for(int i = 0; i < blockSize; i++)
		{
			CreateWindow(L"button",NULL,WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
				25+blockWidth*i,25+blockHeight*j, blockWidth, blockHeight, hWnd,(HMENU)(ID_BLOCK_BTN+(blockSize*j+i)),g_hInst, NULL);
		}
	}

	return 0;
}

int DestroyBlock(HWND hWnd, int blockSize)
{
	for(int j = 0; j < blockSize; j++)
	{
		for(int i = 0; i < blockSize; i++)
		{
			DestroyWindow(GetDlgItem(hWnd,ID_BLOCK_BTN+blockSize*j+i));
		}
	}

	return 0;
}

int AllBlockShow(HWND hWnd)
{
	for(int j = 0; j < g_BlockSize; j++)
	{
		for(int i = 0; i < g_BlockSize; i++)
		{
			ShowWindow(GetDlgItem(hWnd,ID_BLOCK_BTN+g_BlockSize*j+i),TRUE);
		}
	}

	return 0;
}

int AllBlockHide(HWND hWnd)
{
	for(int j = 0; j < g_BlockSize; j++)
	{
		for(int i = 0; i < g_BlockSize; i++)
		{
			ShowWindow(GetDlgItem(hWnd,ID_BLOCK_BTN+g_BlockSize*j+i),FALSE);
		}
	}

	return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd,UINT iMessage,WPARAM wParam,LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	LPDRAWITEMSTRUCT lpdis;

	static HBITMAP hBlockDC;
	static HBITMAP hAllOpenDC;
	static HBITMAP hResetDC;
	static HBITMAP hCloseDC;
	static HBITMAP hLoadImgDC;

	HDC hMemDC;

	int CHILD_ID = LOWORD(wParam);
	int x = LOWORD(lParam);
	int y = HIWORD(lParam);
	
	int pos_x,pos_y;

	switch(iMessage)
	{
	case WM_CREATE:
		g_hWnd=hWnd;
		// 닫기 버튼
		CreateWindow(L"button",NULL,WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
			700,10, 150, 50, hWnd,(HMENU)ID_CLOSE_BTN,g_hInst, NULL);

		// 모두열기 버튼
		CreateWindow(L"button",NULL,WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
			700,80, 150, 50, hWnd,(HMENU)ID_ALLOPEN_BTN,g_hInst, NULL);

		// 리셋 버튼
		CreateWindow(L"button",NULL,WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
			700,160, 150, 50, hWnd,(HMENU)ID_RESET_BTN,g_hInst, NULL);
		
		// 이미지 불러오기 버튼
		CreateWindow(L"button",NULL,WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
			700,230, 150, 50, hWnd,(HMENU)ID_IMGFILE_BTN,g_hInst, NULL);

		// 4x4 버튼
		CreateWindow(L"button",L"4x4", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP,
			700, 300,100,30,hWnd,(HMENU)ID_4BY4,g_hInst, NULL);
		// 5x5 버튼
		CreateWindow(L"button",L"5x5", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
			700, 350,100,30,hWnd,(HMENU)ID_5BY5,g_hInst, NULL);
		// 6x6 버튼
		CreateWindow(L"button",L"6x6", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
			700, 400,100,30,hWnd,(HMENU)ID_6BY6,g_hInst, NULL);
		// 7x7 버튼
		CreateWindow(L"button",L"7x7", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
			700, 450,100,30,hWnd,(HMENU)ID_7BY7,g_hInst, NULL);
		// 8x8 버튼
		CreateWindow(L"button",L"8x8", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
			700, 500,100,30,hWnd,(HMENU)ID_8BY8,g_hInst, NULL);
		CheckRadioButton(hWnd,ID_4BY4,ID_8BY8,ID_8BY8);		

		// 블럭 버튼을 생성한다.
		CreateBlock(hWnd, g_BlockSize, 80, 80);


		//wcscpy_s(g_strBGFileName,256,L"taeyun.jpg");

		hBlockDC = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BLOCK));
		hAllOpenDC = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_ALLOPEN));
		hResetDC = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_RESET));
		hLoadImgDC = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_LOADIMG));
		hCloseDC = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_CLOSE));

		return 0;
	case WM_RBUTTONDOWN:

		switch(g_BlockSize)
		{
		case 4:
			if( (25 > x) || (g_BlockSize*160 < x) ||
				(25 > y) || (g_BlockSize*160 < y) )
				return 0;

			pos_x = (x - 25)/160;
			pos_y = (y - 25)/160;
			break;
		case 5:
			if( (25 > x) || (g_BlockSize*128 < x) ||
				(25 > y) || (g_BlockSize*128 < y) )
				return 0;

			pos_x = (x - 25)/128;
			pos_y = (y - 25)/128;
			break;
		case 6:
			if( (25 > x) || (g_BlockSize*107 < x) ||
				(25 > y) || (g_BlockSize*107 < y) )
				return 0;

			pos_x = (x - 25)/107;
			pos_y = (y - 25)/107;
			break;
		case 7:
			if( (25 > x) || (g_BlockSize*91 < x) ||
				(25 > y) || (g_BlockSize*91 < y) )
				return 0;

			pos_x = (x - 25)/91;
			pos_y = (y - 25)/91;
			break;
		case 8:
			if( (25 > x) || ((g_BlockSize*80) < x) ||
				(25 > y) || ((g_BlockSize*80) < y) )
				return 0;
			pos_x = (x - 25)/80;
			pos_y = (y - 25)/80;
			break;
		}
		
		g_nSelectBlockID = 300 + pos_y*g_BlockSize+pos_x;
		SendMessage(hWnd,WM_COMMAND,ID_CLOSE_BTN,NULL);
		return 0;

	case WM_DRAWITEM:
		if( ( CHILD_ID >=ID_BLOCK_BTN)&&
			(CHILD_ID < (ID_BLOCK_BTN+120)) )
		{
			lpdis = (LPDRAWITEMSTRUCT)lParam;
			//SelectObject(lpdis->hDC, GetStockObject(GRAY_BRUSH));
			//Rectangle(lpdis->hDC,0,0,50,50);
			hMemDC = CreateCompatibleDC(lpdis->hDC);
			HBITMAP OldBitmap = (HBITMAP)SelectObject(hMemDC, hBlockDC);

			WCHAR strNum[10];
			_itow_s(CHILD_ID-299,strNum,10,10);
			switch(g_BlockSize)
			{
			case 4:
				BitBlt(lpdis->hDC, 0, 0, 160, 160, hMemDC, 0, 0, SRCCOPY);
				TextOut(lpdis->hDC,60,50,strNum,wcslen(strNum));
				DeleteDC(hMemDC);	
				break;
			case 5:
				BitBlt(lpdis->hDC, 0, 0, 128, 128, hMemDC, 0, 0, SRCCOPY);
				TextOut(lpdis->hDC,54,54,strNum,wcslen(strNum));

				DeleteDC(hMemDC);
				break;
			case 6:
				BitBlt(lpdis->hDC, 0, 0, 107, 107, hMemDC, 0, 0, SRCCOPY);
				TextOut(lpdis->hDC,43,43,strNum,wcslen(strNum));

				DeleteDC(hMemDC);
				break;
			case 7:
				BitBlt(lpdis->hDC, 0, 0, 91, 91, hMemDC, 0, 0, SRCCOPY);
				TextOut(lpdis->hDC,36,36,strNum,wcslen(strNum));

				DeleteDC(hMemDC);
				break;
			case 8:
				BitBlt(lpdis->hDC, 0, 0, 80, 80, hMemDC, 0, 0, SRCCOPY);
				TextOut(lpdis->hDC,30,30,strNum,wcslen(strNum));

				DeleteDC(hMemDC);
				break;
			}

			return 0;
		}

		switch(LOWORD(wParam))
		{
			case ID_CLOSE_BTN:
				lpdis = (LPDRAWITEMSTRUCT)lParam;
//				SelectObject(lpdis->hDC, GetStockObject(LTGRAY_BRUSH));
//				Rectangle(lpdis->hDC,0,0,150,50);
				hMemDC = CreateCompatibleDC(lpdis->hDC);
				SelectObject(hMemDC, hCloseDC);

				if(lpdis->itemState & ODS_SELECTED)
				{
					TextOut(hMemDC,6,6,L"닫기",2);
				}
				else
				{
					TextOut(hMemDC,5,5,L"닫기",2);
				}
				BitBlt(lpdis->hDC, 0, 0, 150, 50, hMemDC, 0, 0, SRCCOPY);
				DeleteDC(hMemDC);


				return 0;
			case ID_ALLOPEN_BTN:
				lpdis = (LPDRAWITEMSTRUCT)lParam;
//				SelectObject(lpdis->hDC, GetStockObject(DKGRAY_BRUSH));
//				Rectangle(lpdis->hDC,0,0,150,50);
				hMemDC = CreateCompatibleDC(lpdis->hDC);
				SelectObject(hMemDC, hAllOpenDC);

				if(lpdis->itemState & ODS_SELECTED)
				{
					TextOut(hMemDC,6,6,L"모두열기",4);
				}
				else
				{
					TextOut(hMemDC,5,5,L"모두열기",4);
				}
				BitBlt(lpdis->hDC, 0, 0, 150, 50, hMemDC, 0, 0, SRCCOPY);
				DeleteDC(hMemDC);

				return 0;

			case ID_RESET_BTN:
				lpdis = (LPDRAWITEMSTRUCT)lParam;
//				SelectObject(lpdis->hDC, GetStockObject(DKGRAY_BRUSH));
//				Rectangle(lpdis->hDC,0,0,150,50);
				hMemDC = CreateCompatibleDC(lpdis->hDC);
				SelectObject(hMemDC, hResetDC);

				if(lpdis->itemState & ODS_SELECTED)
				{
					TextOut(hMemDC,6,6,L"리셋",2);
				}
				else
				{
					TextOut(hMemDC,5,5,L"리셋",2);
				}
				BitBlt(lpdis->hDC, 0, 0, 150, 50, hMemDC, 0, 0, SRCCOPY);
				DeleteDC(hMemDC);

				return 0;

			case ID_IMGFILE_BTN:
				lpdis = (LPDRAWITEMSTRUCT)lParam;
//				SelectObject(lpdis->hDC, GetStockObject(GRAY_BRUSH));
//				Rectangle(lpdis->hDC,0,0,150,50);
				hMemDC = CreateCompatibleDC(lpdis->hDC);
				SelectObject(hMemDC, hLoadImgDC);

				if(lpdis->itemState & ODS_SELECTED)
				{
					TextOut(hMemDC,6,6,L"이미지불러오기",7);
				}
				else
				{
					TextOut(hMemDC,5,5,L"이미지불러오기",7);
				}
				BitBlt(lpdis->hDC, 0, 0, 150, 50, hMemDC, 0, 0, SRCCOPY);
				DeleteDC(hMemDC);

				return 0;
		}
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case ID_CLOSE_BTN:
		//	MessageBox(hWnd,L"재시작 버튼을 눌렀습니다.",L"확인", MB_OK);
			if(g_nSelectBlockID != -1)
			{
				ShowWindow(GetDlgItem(hWnd,g_nSelectBlockID),TRUE);
				g_nSelectBlockID = -1;
			}

			break;
		case ID_ALLOPEN_BTN:
		//	MessageBox(hWnd,L"리셋 버튼을 눌렀습니다.",L"확인", MB_OK);
			AllBlockHide(hWnd);
			g_nSelectBlockID = -1;
			break;

		case ID_RESET_BTN:
		//	MessageBox(hWnd,L"리셋 버튼을 눌렀습니다.",L"확인", MB_OK);
			AllBlockShow(hWnd);
			g_nSelectBlockID = -1;
			break;
		case ID_IMGFILE_BTN:
			FileOpen(hWnd);
			g_nSelectBlockID = -1;
			break;
		}

		if( ( CHILD_ID >=ID_BLOCK_BTN)&&
			(CHILD_ID < (ID_BLOCK_BTN+g_BlockSize*g_BlockSize)) )
		{
			ShowWindow(GetDlgItem(hWnd,CHILD_ID),FALSE);
			g_nSelectBlockID = CHILD_ID;
			return 0;
		}

		// 라디오 버튼 클릭 처리
		if(HIWORD(wParam) == BN_CLICKED)
		{
			switch(LOWORD(wParam))
			{
			case ID_4BY4:
				DestroyBlock(hWnd,g_BlockSize);
				g_BlockSize = 4;
				CreateBlock(hWnd,g_BlockSize,160,160);
				g_nSelectBlockID = -1;
				break;
			case ID_5BY5:
				DestroyBlock(hWnd,g_BlockSize);
				g_BlockSize = 5;
				CreateBlock(hWnd,g_BlockSize,128,128);
				g_nSelectBlockID = -1;
				break;
			case ID_6BY6:
				DestroyBlock(hWnd,g_BlockSize);
				g_BlockSize = 6;
				CreateBlock(hWnd,g_BlockSize,107,107);
				g_nSelectBlockID = -1;
				break;
			case ID_7BY7:
				DestroyBlock(hWnd,g_BlockSize);
				g_BlockSize = 7;
				CreateBlock(hWnd,g_BlockSize,91,91);
				g_nSelectBlockID = -1;
				break;
			case ID_8BY8:
				DestroyBlock(hWnd,g_BlockSize);
				g_BlockSize = 8;
				CreateBlock(hWnd,g_BlockSize,80,80);
				g_nSelectBlockID = -1;
				break;
			}
		}

	case WM_PAINT:
		hdc=BeginPaint(hWnd, &ps);
		OnPaint(hdc);
		EndPaint(hWnd, &ps);
		return 0;

	case WM_DESTROY:
		if (g_pCachedBitmap) 
		{
			delete g_pCachedBitmap;
		}
		PostQuitMessage(0);
		return 0;
	}

	return(DefWindowProc(hWnd,iMessage,wParam,lParam));
}