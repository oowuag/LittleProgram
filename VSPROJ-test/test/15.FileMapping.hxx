//Poperty->Preprocessor->PreprocessorDefinitions  设为_WINDOWS
//Linker->System->SubSystem 设为Windows

#include "Windows.h"
#include "tchar.h"

#pragma comment(lib, "User32.lib");
#pragma comment(lib, "Comdlg32.lib");

TCHAR ClassName[] = _T("Win32FileMappingClass");
TCHAR AppName[] = _T("Win32 File Mapping Example");
TCHAR MenuName[] = _T("FirstMenu");
HINSTANCE g_hInstance;

#define IDM_OPEN 1
#define IDM_SAVE 2
#define IDM_EXIT 3
#define MAXSIZE 260

OPENFILENAME g_ofn;
TCHAR FilterString[] = _T("All Files\0*.*\0Text Files\0*.txt\0");
TCHAR buffer[MAXSIZE];
HANDLE hMapFile;

HANDLE hFileRead;
HANDLE hFileWrite;
HMENU hMenu;
void *pMemory;
DWORD SizeWritten;

void CloseMapFile()
{
	CloseHandle(hMapFile);
	hMapFile = NULL;
	CloseHandle(hFileRead);
}
INT_PTR CALLBACK ProcWinMain(   HWND hWnd, 
							 UINT Msg, 
							 WPARAM wParam, 
							 LPARAM lParam 
							 )
{
	switch(Msg)
	{
	case WM_CREATE:
		hMenu = GetMenu(hWnd);
		g_ofn.lStructSize = sizeof(g_ofn);
		g_ofn.hwndOwner = hWnd;
		g_ofn.hInstance = g_hInstance;
		g_ofn.lpstrFilter = FilterString;
		g_ofn.lpstrFile = buffer;
		g_ofn.nMaxFile = MAXSIZE;
		break;

	case WM_DESTROY:
		if(hMapFile !=0)
			CloseMapFile();
		PostQuitMessage(0);
		break;
	//case WM_COMMAND:
		//if(lParam == 0)
	case WM_LBUTTONUP:
			{
				g_ofn.Flags = OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST |OFN_LONGNAMES|OFN_EXPLORER|OFN_HIDEREADONLY;
				if(GetOpenFileName(&g_ofn))
				{
					hFileRead = CreateFile(buffer,GENERIC_READ,0,NULL,OPEN_EXISTING,
						FILE_ATTRIBUTE_ARCHIVE,NULL);

					hMapFile = CreateFileMapping(hFileRead,NULL,PAGE_READONLY,0,0,NULL);
					SetWindowText(hWnd,buffer + g_ofn.nFileOffset);

					EnableMenuItem(hMenu,IDM_OPEN,MF_GRAYED);
					EnableMenuItem(hMenu,IDM_SAVE,MF_ENABLED);
				}
				break;
			}
	case WM_RBUTTONDOWN:
			{
				g_ofn.Flags = OFN_LONGNAMES|OFN_EXPLORER|OFN_HIDEREADONLY;
				if(GetSaveFileName(&g_ofn))
				{
					hFileWrite = CreateFile(buffer,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,
						CREATE_NEW,FILE_ATTRIBUTE_ARCHIVE,NULL);

					pMemory = MapViewOfFile(hMapFile,FILE_MAP_READ,0,0,0);
					int nSize = GetFileSize(hFileRead,NULL);
					WriteFile(hFileWrite,pMemory,nSize,&SizeWritten,NULL);
					UnmapViewOfFile(pMemory);
					CloseMapFile();
					CloseHandle(hFileWrite);
					SetWindowText(hWnd,AppName);
					EnableMenuItem(hMenu,IDM_OPEN,MF_ENABLED);
					EnableMenuItem(hMenu,IDM_SAVE,MF_GRAYED);

				}
				break;
			}
			//else
			//	DestroyWindow(hWnd);
	default:
		return DefWindowProc(hWnd,Msg,wParam,lParam);
	}
	return 0;
}
//int WINAPI WinMain(HINSTANCE hInstance,
//				   HINSTANCE hPrevInstance,
//				   LPSTR lpCmdLine,
//				   int nCmdShow
//				   )
int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	WNDCLASSEX wc;
	MSG msg;
	HWND hWnd;
	g_hInstance = hInstance;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = (WNDPROC)ProcWinMain;
	wc.cbClsExtra = NULL;
	wc.cbWndExtra = NULL;
	wc.hInstance = hInstance;
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = MenuName;
	wc.lpszClassName = ClassName;
	wc.hIcon = wc.hIconSm = LoadIcon(NULL,IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL,IDC_ARROW);
	RegisterClassEx(&wc);

	hWnd = CreateWindowEx(WS_EX_CLIENTEDGE,ClassName,AppName,WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,CW_USEDEFAULT,300,200,NULL,NULL,hInstance,NULL);

	ShowWindow(hWnd,SW_SHOWNORMAL);
	UpdateWindow(hWnd);

	while(GetMessage(&msg,NULL,0,0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}