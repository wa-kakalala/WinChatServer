// WinChatServer.cpp : 定义应用程序的入口点。
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "framework.h"
#include "WinChatServer.h"
#include <stdio.h>
#include <winsock2.h>
#include "types.h"

#pragma comment(lib, "ws2_32.lib") /* WinSock使用的库函数 */

#define MAX_LOADSTRING   100
#define ID_EDIT_LOG      1    // 日志控件标识
#define EDIT_STYLE       (WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | \
                         ES_MULTILINE | WS_HSCROLL | WS_VSCROLL)
#define WINCHAT_MAX_BUF  512
#define WINCHAT_MAX_DATA 1024
#define MAX_USER_NUM     100
#define WIN_CHAT_NOTIFY  (WM_USER + 10) /* 自定义socket消息 */
#define WINCHAT_UDP_PORT 6666

// 全局变量:
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名
HWND hWndLog;                                   // 日志信息窗口句柄
SOCKET udpSoc = INVALID_SOCKET;                 // Ser updSoc
char  WinChatBuf[WINCHAT_MAX_DATA];            // 接收数据缓冲区
WC_USER_INFO UserInfo[MAX_USER_NUM];            // 记录用户信息

// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void LogPrintf(const TCHAR* szFormat, ...); // 日志输出
SOCKET WinChatCreateUdpSoc(HWND hWnd, unsigned short port); // 创建udp套接字
void WinChatUDPSocketNotify(WPARAM wParam, LPARAM lParam);  // 处理WSAAsyncSelect 
void WinChatUDPController(SOCKET udpsoc);                   // 消息控制器

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    int err;
    WSADATA     wsaData;
    err = WSAStartup(WINSOCK_VERSION, &wsaData); /* 初始化 */
    if (err) return 0;
    
    // 初始化全局字符串
    LoadString(hInstance, IDS_APP_TITLE, (LPTSTR)szTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_WINCHATSERVER, (LPTSTR)szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINCHATSERVER));

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
    closesocket(udpSoc);
    WSACleanup();
    return (int) msg.wParam;
}

/**
注册窗口类
*/
ATOM MyRegisterClass(HINSTANCE hInstance){
    WNDCLASSEXW wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINCHATSERVER));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WINCHATSERVER);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
    return RegisterClassExW(&wcex);
}


BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int cxClient, cyClient;
    switch (message){
    case WM_CREATE:
        hWndLog = CreateWindow(TEXT("edit"), NULL, EDIT_STYLE, 0, 0, 0,0, hWnd, (HMENU)ID_EDIT_LOG, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
        return 0;
    case WIN_CHAT_NOTIFY:
        WinChatUDPSocketNotify(wParam, lParam);
        return 0;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            case IDM_START:
                udpSoc = WinChatCreateUdpSoc(hWnd, WINCHAT_UDP_PORT);
                if (udpSoc != INVALID_SOCKET) {
                    LogPrintf(TEXT("WinChat server is running ...\r\n"));
                }else {
                    LogPrintf(TEXT("WinChat server starts error ...\r\n"));
                }
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_SIZE:
        cxClient = LOWORD(lParam);
        cyClient = HIWORD(lParam);
        MoveWindow(hWndLog, 0, 0, cxClient, cyClient, FALSE);
        return 0;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 在此处添加使用 hdc 的任何绘图代码...
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

void LogPrintf(const TCHAR* szFormat, ...)
{
    int iBufLen = 0, iIndex;
    TCHAR szBuffer[WINCHAT_MAX_BUF];
    va_list pVaList;
    va_start(pVaList, szFormat);
#ifdef UNICODE
    iBufLen = _vsnwprintf(szBuffer, FINGER_MAX_BUF, szFormat, pVaList);
#else
    iBufLen = _vsnprintf_s(szBuffer, WINCHAT_MAX_BUF, WINCHAT_MAX_BUF,szFormat, pVaList);
#endif
    va_end(pVaList);
    iIndex = GetWindowTextLength(hWndLog);
    SendMessage(hWndLog, EM_SETSEL, (WPARAM)iIndex, (LPARAM)iIndex);
    SendMessage(hWndLog, EM_REPLACESEL, FALSE, (LPARAM)szBuffer);
    SendMessage(hWndLog, EM_SCROLLCARET, 0, 0);
}

SOCKET WinChatCreateUdpSoc(HWND hWnd, unsigned short port) {
    struct sockaddr_in udpsoc_addr; 
    SOCKET udpsoc;
    int result;

    udpsoc = socket(AF_INET, SOCK_DGRAM, 0);
    udpsoc_addr.sin_family = AF_INET;
    udpsoc_addr.sin_port = htons(port);
    udpsoc_addr.sin_addr.s_addr = INADDR_ANY;

    result = bind(udpsoc, (struct sockaddr*)&udpsoc_addr, sizeof(udpsoc_addr));
    if (result == SOCKET_ERROR)
    {
        closesocket(udpsoc);
        return INVALID_SOCKET;
    }
    result = WSAAsyncSelect(udpsoc, hWnd, WIN_CHAT_NOTIFY, FD_READ);
    return udpsoc;
}

void WinChatUDPSocketNotify(WPARAM wParam, LPARAM lParam) {
    WORD wEvent, wError;
    wEvent = WSAGETSELECTEVENT(lParam); /* LOWORD */
    wError = WSAGETSELECTERROR(lParam); /*HIWORD */
    switch (wEvent) {
    case FD_READ:
        if (wError) {
            LogPrintf(TEXT("FD_READ error #%i."), wError);
            return;
        }
        WinChatUDPController(wParam);
        break;
    default:
        LogPrintf(TEXT("Error event type\r\n"));
    }
}

void WinChatUDPController(SOCKET udpsoc) {
    struct sockaddr_in peer_addr;
    int datalen,addr_len = sizeof(peer_addr);
    datalen = recvfrom(udpsoc, WinChatBuf, WINCHAT_MAX_DATA, 0,
        (struct sockaddr*)&peer_addr, &addr_len);
    WinChatBuf[datalen] = 0;
    LogPrintf("%s\r\n", WinChatBuf);
}
