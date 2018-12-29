// GUIChatServer.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "GUIChatServer.h"

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "winsock2.h"

#define MAX_LOADSTRING 100

#define WM_SOCKET WM_USER + 1

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

char* ids[64];
SOCKET registeredClients[64];
int numRegisteredClients = 0;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void				removeSocket(HWND, SOCKET);

SOCKET listener;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(9000);

	listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	bind(listener, (SOCKADDR *)&addr, sizeof(addr));
	listen(listener, 5);


    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_GUICHATSERVER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GUICHATSERVER));

    MSG msg;

    // Main message loop:
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
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GUICHATSERVER));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_GUICHATSERVER);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 550, 420, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   WSAAsyncSelect(listener, hWnd, WM_SOCKET, FD_ACCEPT);

   CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("LISTBOX"), TEXT(""), 
	   WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOVSCROLL, 
	   10, 10, 200, 350, hWnd, (HMENU)IDC_LIST_CLIENT, GetModuleHandle(NULL), NULL);

   CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("LISTBOX"), TEXT(""),
	   WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOVSCROLL,
	   220, 10, 300, 350, hWnd, (HMENU)IDC_LIST_MESSAGE, GetModuleHandle(NULL), NULL);

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
	case WM_SOCKET:
	{
		if (WSAGETSELECTERROR(lParam))
		{
			// Xoa client
			removeSocket(hWnd, (SOCKET)wParam);
			closesocket((SOCKET)wParam);
			return DefWindowProc(hWnd, message, wParam, lParam);
		}

		if (WSAGETSELECTEVENT(lParam) == FD_ACCEPT)
		{
			SOCKET client = accept((SOCKET)wParam, NULL, NULL);

			WSAAsyncSelect(client, hWnd, WM_SOCKET, FD_READ | FD_CLOSE);
		}
		else if (WSAGETSELECTEVENT(lParam) == FD_READ)
		{
			SOCKET client = (SOCKET)wParam;
			char buf[256];
			int ret = recv(client, buf, sizeof(buf), 0);

			if (ret <= 0)
			{
				// Xoa client
				removeSocket(hWnd, client);

				closesocket(client);
				return DefWindowProc(hWnd, message, wParam, lParam);
			}

			buf[ret] = 0;
			SendDlgItemMessageA(hWnd, IDC_LIST_MESSAGE, LB_ADDSTRING, 0, (LPARAM)buf);
			SendDlgItemMessageA(hWnd, IDC_LIST_MESSAGE, WM_VSCROLL, SB_BOTTOM, 0);

			int found = 0;
			int j = 0;
			for (; j < numRegisteredClients; j++)
				if (client == registeredClients[j])
				{
					found = 1;
					break;
				}

			if (found == 0)	// chua dang nhap
			{
				char cmd[16], id[64], tmp[64];
				char *msg = "Sai cu phap. Hay nhap lai.";

				ret = sscanf(buf, "%s %s %s", cmd, id, tmp);
				if (ret == 2)
				{
					if (strcmp(cmd, "client_id:") == 0)
					{
						send(client, "OK\n", 3, 0);

						ids[numRegisteredClients] = (char *)malloc(64);
						memcpy(ids[numRegisteredClients], id, strlen(id) + 1);
						registeredClients[numRegisteredClients] = client;
						numRegisteredClients++;

						SendDlgItemMessageA(hWnd, IDC_LIST_CLIENT, LB_ADDSTRING, 0, (LPARAM)id);
						SendDlgItemMessageA(hWnd, IDC_LIST_CLIENT, WM_VSCROLL, SB_BOTTOM, 0);
					}
					else
						send(client, msg, strlen(msg), 0);
				}
				else
					send(client, msg, strlen(msg), 0);
			}
			else // found == 1 => da dang nhap
			{
				char targetId[64];
				char sendbuf[256];

				// tim id ma client muon gui
				sscanf(buf, "%s", targetId);

				sprintf(sendbuf, "%s: %s", ids[j], buf + strlen(targetId) + 1);

				if (strcmp(targetId, "@all") == 0)
				{
					// forward den tat ca cac client khac
					for (int j = 0; j < numRegisteredClients; j++)
						if (client != registeredClients[j])
							send(registeredClients[j], sendbuf, strlen(sendbuf), 0);
				}
				else
				{
					// forward den client co id la targetId
					for (int j = 0; j < numRegisteredClients; j++)
						if (strcmp(targetId + 1, ids[j]) == 0)
							send(registeredClients[j], sendbuf, strlen(sendbuf), 0);
				}
			}
		}
		else if (WSAGETSELECTEVENT(lParam) == FD_CLOSE)
		{
			// Xoa socket
			removeSocket(hWnd, (SOCKET)wParam);
			closesocket((SOCKET)wParam);
		}
	}
	break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
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
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
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

void removeSocket(HWND hWnd, SOCKET s)
{
	int i = 0;
	for (; i < numRegisteredClients; i++)
		if (registeredClients[i] == s)
			break;

	if (i < numRegisteredClients)
	{
		if (i < numRegisteredClients - 1)
		{
			registeredClients[i] = registeredClients[numRegisteredClients - 1];
			ids[i] = ids[numRegisteredClients - 1];
		}

		numRegisteredClients--;

		// Xoa id khoi LISTBOX
		SendDlgItemMessageA(hWnd, IDC_LIST_CLIENT, LB_DELETESTRING, i, 0);
	}
}