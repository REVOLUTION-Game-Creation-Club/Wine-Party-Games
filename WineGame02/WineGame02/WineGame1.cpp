#include <windows.h>
#include <GdiPlus.h>
#include <shlobj.h>
#include <stdlib.h>
#include <time.h>

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


class SBlock
{
public:
	bool ok;
	int nID;
	WCHAR strFileName[MAX_PATH];
};


SBlock g_cBlock[36];
int	g_nSelectBlockID[2] = {-1,-1};


CachedBitmap* g_pCachedBitmap;
WCHAR	g_strBGFileName[256];
int		g_BlockSize = 4;

WCHAR	g_strDefaultPath[MAX_PATH];
WCHAR	g_strImgPath[MAX_PATH];


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
	WndClass.lpszClassName=L"대구와인동호회게임2";
	WndClass.lpszMenuName=NULL;
	WndClass.style=CS_HREDRAW | CS_VREDRAW;
	RegisterClass(&WndClass);

	hWnd=CreateWindow(L"대구와인동호회게임2",L"대구와인동호회게임2",WS_OVERLAPPEDWINDOW,
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
	Bitmap* pObjBit[36];
	

	for(int i = 0; i < g_BlockSize*g_BlockSize; i++)
	{
		pObjBit[i] = (Bitmap*)Image::FromFile(g_cBlock[i].strFileName);
	}


	Graphics* pMemGraphic = new Graphics(pBGBit);

	pMemGraphic->DrawImage(pBGBit,0,0);
	

	for(int j = 0; j < g_BlockSize; j++)
	{
		for(int i = 0; i <g_BlockSize; i++)
		{
			pMemGraphic->DrawImage(pObjBit[g_BlockSize*j+i],Rect(25+129*i,129*j,128,128),0,0,
				pObjBit[g_BlockSize*j+i]->GetWidth(),pObjBit[g_BlockSize*j+i]->GetHeight(), UnitPixel,NULL);
		}
	}


	if(g_pCachedBitmap)
	{
		delete g_pCachedBitmap;
	}

	Graphics graphic(g_hWnd);
	g_pCachedBitmap = new CachedBitmap(pBGBit, &graphic);


	for(int i = 0; i < g_BlockSize*g_BlockSize; i++)
		delete pObjBit[i];

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

	Pen pen(Color(255,0,0,0), 5);

	for(int i = 0; i <= g_BlockSize; i++)
	{
		graphic.DrawLine(&pen,25+(128+1)*i,0,25+(128+1)*i,g_BlockSize*(128+1));
	}

	for(int j = 0; j <= g_BlockSize; j++)
	{
		graphic.DrawLine(&pen,25,0+(128+1)*j,25+g_BlockSize*(128+1),0+(128+1)*j);
	}

}

int CALLBACK FolderCallbackProc(HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData) { 
	//콜백 함수 
	if(uMsg == BFFM_INITIALIZED) 
	{ 
		SendMessage(hWnd, BFFM_SETSELECTION, TRUE, lpData); 
	} 
	return 0; 
} 

void MixingBlock()
{
	srand((unsigned int)time(NULL));

	for(int i = 0; i < g_BlockSize*g_BlockSize; i++)
	{
		int nRand = rand()%(g_BlockSize*g_BlockSize);


		// 랜덤으로 선택된 블럭과 순차적으로 스왑한다.
		int nTempID;
		WCHAR strTemp[MAX_PATH];

		nTempID = g_cBlock[nRand].nID;
		ZeroMemory(strTemp,MAX_PATH);
		wcscpy_s(strTemp,MAX_PATH,g_cBlock[nRand].strFileName);

		g_cBlock[nRand].nID = g_cBlock[i].nID;
		ZeroMemory(g_cBlock[nRand].strFileName,MAX_PATH);
		wcscpy_s(g_cBlock[nRand].strFileName,MAX_PATH,g_cBlock[i].strFileName);

		g_cBlock[i].nID = nTempID;
		ZeroMemory(g_cBlock[i].strFileName,MAX_PATH);
		wcscpy_s(g_cBlock[i].strFileName,MAX_PATH,strTemp);
	}
}

void LoadBlockImage()
{
	// 불러올 갯수는 현재 선택된 블럭 총 개수의 반이다.
	int nLoadImageNum = g_BlockSize*g_BlockSize/2;
	
	
	for(int i = 0; i <nLoadImageNum; i++)
	{
		WCHAR strFilePath[MAX_PATH];
		WCHAR strImgNum[10];

		ZeroMemory(strFilePath,MAX_PATH);
		wcscat_s(strFilePath,MAX_PATH,g_strImgPath);
		wcscat_s(strFilePath,L"\\");
		_itow_s(i+1,strImgNum,10,10);
		wcscat_s(strFilePath,MAX_PATH,strImgNum);
		wcscat_s(strFilePath,L".png");
		
		g_cBlock[i*2].nID = i;
		wcscpy_s(g_cBlock[i*2].strFileName,MAX_PATH,strFilePath);

		g_cBlock[i*2+1].nID = i;
		wcscpy_s(g_cBlock[i*2+1].strFileName,MAX_PATH,strFilePath);
	}
}

void FileOpen(HWND hWnd)
{
	BROWSEINFO BrInfo;
	memset( &BrInfo, 0, sizeof( BrInfo ) ); 

	BrInfo.hwndOwner = hWnd;
	BrInfo.pidlRoot = NULL; 
	BrInfo.pszDisplayName = g_strImgPath; 
	BrInfo.lpszTitle = L"이미지 파일경로";             //창에 표시될 안내문구
	BrInfo.ulFlags = BIF_RETURNONLYFSDIRS; 

	wcscpy_s(g_strDefaultPath,MAX_PATH,L"C:\\"); 
	// 추가
	BrInfo.lParam = (LPARAM)g_strDefaultPath; 

	BrInfo.lpfn = FolderCallbackProc; 

	ITEMIDLIST* pidlBrowse; 
	pidlBrowse = ::SHBrowseForFolder( &BrInfo ); 

	if(pidlBrowse != NULL)
	{ 
		SHGetPathFromIDList( pidlBrowse, g_strImgPath ); 
		//pszPathname에 선택한 폴더 경로가 들어가 있습니다.

		// 선택된 경로에서 이미지 파일을 가져온다.
		LoadBlockImage();
		MixingBlock();

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
				25+(blockWidth+1)*i,(blockHeight+1)*j, blockWidth, blockHeight, hWnd,(HMENU)(ID_BLOCK_BTN+blockSize*j+i),g_hInst, NULL);
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
			DestroyWindow(GetDlgItem(hWnd,ID_BLOCK_BTN+(blockSize*j+i)));
		}
	}

	return 0;
}

int AllBlockReset(HWND hWnd)
{
	for(int j = 0; j < g_BlockSize; j++)
	{
		for(int i = 0; i < g_BlockSize; i++)
		{
			ShowWindow(GetDlgItem(hWnd,ID_BLOCK_BTN+g_BlockSize*j+i),TRUE);
			g_cBlock[g_BlockSize*j+i].ok = false;
		}
	}

	return 0;
}

int AllBlockOpen(HWND hWnd)
{
	for(int j = 0; j < g_BlockSize; j++)
	{
		for(int i = 0; i < g_BlockSize; i++)
		{
			ShowWindow(GetDlgItem(hWnd,ID_BLOCK_BTN+g_BlockSize*j+i),FALSE);
			g_cBlock[g_BlockSize*j+i].ok = true;
		}
	}
	return 0;
}
bool AllBlockOK()
{
	for(int j = 0; j < g_BlockSize; j++)
	{
		for(int i = 0; i < g_BlockSize; i++)
		{
			if(g_cBlock[g_BlockSize*j+i].ok == false)
				return false;
		}
	}

	return true;
}

LRESULT CALLBACK WndProc(HWND hWnd,UINT iMessage,WPARAM wParam,LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	LPDRAWITEMSTRUCT lpdis;

	static HBITMAP hBlcokDC;
	static HBITMAP hAllOpenDC;
	static HBITMAP hResetDC;
	static HBITMAP hCloseDC;
	static HBITMAP hLoadImgDC;

	HDC hMemDC;

	int CHILD_ID;

	switch(iMessage)
	{
	case WM_CREATE:
		g_hWnd=hWnd;

		// 닫기 버튼
		CreateWindow(L"button",NULL,WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
			800,10, 150, 50, hWnd,(HMENU)ID_CLOSE_BTN,g_hInst, NULL);

		// 모두열기 버튼
		CreateWindow(L"button",NULL,WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
			800,110, 150, 50, hWnd,(HMENU)ID_ALLOPEN_BTN,g_hInst, NULL);

		// 리셋 버튼
		CreateWindow(L"button",NULL,WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
			800,210, 150, 50, hWnd,(HMENU)ID_RESET_BTN,g_hInst, NULL);

		// 이미지 불러오기 버튼
		CreateWindow(L"button",NULL,WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
			800,310, 150, 50, hWnd,(HMENU)ID_IMGFILE_BTN,g_hInst, NULL);

		// 4x4 버튼
		CreateWindow(L"button",L"4x4", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP,
			800, 400,100,30,hWnd,(HMENU)ID_4BY4,g_hInst, NULL);
		// 5x5 버튼
		CreateWindow(L"button",L"5x5", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
			800, 450,100,30,hWnd,(HMENU)ID_5BY5,g_hInst, NULL);
		// 6x6 버튼
		CreateWindow(L"button",L"6x6", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
			800, 500,100,30,hWnd,(HMENU)ID_6BY6,g_hInst, NULL);
		//// 7x7 버튼
		//CreateWindow(L"button",L"7x7", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
		//	800, 450,100,30,hWnd,(HMENU)ID_7BY7,g_hInst, NULL);
		//// 8x8 버튼
		//CreateWindow(L"button",L"8x8", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
		//	800, 500,100,30,hWnd,(HMENU)ID_8BY8,g_hInst, NULL);
		CheckRadioButton(hWnd,ID_4BY4,ID_6BY6,ID_4BY4);		

		// 블럭 버튼을 생성한다.
		CreateBlock(hWnd, g_BlockSize, 128, 128);


		hBlcokDC = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BLOCK));
		hAllOpenDC = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_ALLOPEN));
		hResetDC = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_RESET));
		hLoadImgDC = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_LOADIMG));
		hCloseDC = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_CLOSE));

		return 0;
	case WM_RBUTTONDOWN:
		SendMessage(hWnd,WM_COMMAND,ID_CLOSE_BTN,NULL);
		return 0;

	case WM_DRAWITEM:
		CHILD_ID = LOWORD(wParam);
		if( ( CHILD_ID >=ID_BLOCK_BTN)&&
			(CHILD_ID < (ID_BLOCK_BTN+64)) )
		{
			lpdis = (LPDRAWITEMSTRUCT)lParam;

			hMemDC = CreateCompatibleDC(lpdis->hDC);

			SelectObject(hMemDC, hBlcokDC);

			//SelectObject(hMemDC, GetStockObject(GRAY_BRUSH));
			//Rectangle(hMemDC,0,0,50,50);

			BitBlt(lpdis->hDC, 0, 0, 128, 128, hMemDC, 0, 0, SRCCOPY);

			WCHAR strNum[10];
			int blockno= CHILD_ID-ID_BLOCK_BTN+2;

			_itow_s(blockno,strNum,10,10);
			
			if(lpdis->itemState & ODS_SELECTED)
			{
				TextOut(lpdis->hDC,54,54,strNum,wcsnlen(strNum, 10));
			}
			else
			{
				TextOut(lpdis->hDC,54,54,strNum,wcsnlen(strNum,10));
			}
			
			DeleteDC(hMemDC);
			return 0;
		}

		switch(LOWORD(wParam))
		{
			case ID_CLOSE_BTN:
				lpdis = (LPDRAWITEMSTRUCT)lParam;
				//SelectObject(lpdis->hDC, GetStockObject(LTGRAY_BRUSH));
				//Rectangle(lpdis->hDC,0,0,150,50);

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
		CHILD_ID = LOWORD(wParam);
		switch(LOWORD(wParam))
		{
		case ID_CLOSE_BTN:
			//MessageBox(hWnd,L"닫기 버튼을 눌렀습니다.",L"확인", MB_OK);
			// TODO: 두 아이디를 찾아서 닫는다.
			ShowWindow(GetDlgItem(hWnd,g_nSelectBlockID[0]),TRUE);
			ShowWindow(GetDlgItem(hWnd,g_nSelectBlockID[1]),TRUE);

			g_nSelectBlockID[0] = -1;
			g_nSelectBlockID[1] = -1;

			break;
		case ID_ALLOPEN_BTN:
			AllBlockOpen(hWnd);
			g_nSelectBlockID[0] = -1;
			g_nSelectBlockID[1] = -1;
			break;
		case ID_RESET_BTN:
		//	MessageBox(hWnd,L"리셋 버튼을 눌렀습니다.",L"확인", MB_OK);
			AllBlockReset(hWnd);
			g_nSelectBlockID[0] = -1;
			g_nSelectBlockID[1] = -1;
			break;
		case ID_IMGFILE_BTN:
			AllBlockReset(hWnd);
			FileOpen(hWnd);
			break;
		}

		if( ( CHILD_ID >=ID_BLOCK_BTN)&&
			(CHILD_ID < (ID_BLOCK_BTN+g_BlockSize*g_BlockSize)) )
		{
			
			if(g_nSelectBlockID[0] == -1)
			{
				g_nSelectBlockID[0] = CHILD_ID;
				ShowWindow(GetDlgItem(hWnd,CHILD_ID),FALSE);

				//WCHAR strMessage[128];
				//WCHAR strChildID[10];

				//_itow_s(g_cBlock[CHILD_ID-300].nID,strChildID,10,10);
				//wcscpy_s(strMessage,128,strChildID);
				//MessageBox(hWnd,strMessage,L"확인", MB_OK);
			}
			else if(g_nSelectBlockID[1] == -1)
			{
				g_nSelectBlockID[1] = CHILD_ID;
				ShowWindow(GetDlgItem(hWnd,CHILD_ID),FALSE);

				//WCHAR strMessage[128];
				//WCHAR strChildID[10];

				//_itow_s(g_cBlock[CHILD_ID-300].nID,strChildID,10,10);
				//wcscpy_s(strMessage,128,strChildID);
				//MessageBox(hWnd,strMessage,L"확인", MB_OK);

				int childID0 = g_nSelectBlockID[0] - 300;
				int childID1 = g_nSelectBlockID[1] - 300;
				if(g_cBlock[childID0].nID == g_cBlock[childID1].nID)
				{
					g_cBlock[childID0].ok = true;
					g_cBlock[childID1].ok = true;
					g_nSelectBlockID[0] = -1;
					g_nSelectBlockID[1] = -1;

					if(AllBlockOK() == true)
					{
						MessageBox(hWnd,L"축하합니다. 게임이 끝났습니다!!!",L"확인", MB_OK);
					}
					else
					{
						MessageBox(hWnd,L"같은 그림을 찾았습니다.",L"확인", MB_OK);
					}
				}
			}
			else
			{
				MessageBox(hWnd,L"닫기를 눌러주세요.",L"확인", MB_OK);
			}

		}

		// 라디오 버튼 클릭 처리
		if(HIWORD(wParam) == BN_CLICKED)
		{
			switch(LOWORD(wParam))
			{
			case ID_4BY4:
				AllBlockReset(hWnd);
				DestroyBlock(hWnd,g_BlockSize);
				g_BlockSize = 4;
				CreateBlock(hWnd,g_BlockSize,128,128);
				g_nSelectBlockID[0] = -1;
				g_nSelectBlockID[1] = -1;
				FileOpen(hWnd);

				break;
			case ID_5BY5:
				AllBlockReset(hWnd);
				DestroyBlock(hWnd,g_BlockSize);
				g_BlockSize = 5;
				CreateBlock(hWnd,g_BlockSize,128,128);
				g_nSelectBlockID[0] = -1;
				g_nSelectBlockID[1] = -1;
				FileOpen(hWnd);

				break;
			case ID_6BY6:
				AllBlockReset(hWnd);
				DestroyBlock(hWnd,g_BlockSize);
				g_BlockSize = 6;
				CreateBlock(hWnd,g_BlockSize,128,128);
				g_nSelectBlockID[0] = -1;
				g_nSelectBlockID[1] = -1;
				FileOpen(hWnd);
				break;
			//case ID_7BY7:
			//	DestroyBlock(hWnd,g_BlockSize);
			//	g_BlockSize = 7;
			//	CreateBlock(hWnd,g_BlockSize,91,91);
			//	break;
			//case ID_8BY8:
			//	DestroyBlock(hWnd,g_BlockSize);
			//	g_BlockSize = 8;
			//	CreateBlock(hWnd,g_BlockSize,80,80);
			//	break;
			}
		}

		return 0;

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

		DeleteObject(hBlcokDC);

		PostQuitMessage(0);
		return 0;
	}

	return(DefWindowProc(hWnd,iMessage,wParam,lParam));
}