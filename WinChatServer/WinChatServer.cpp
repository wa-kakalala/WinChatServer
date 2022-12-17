// WinChatServer.cpp : 定义应用程序的入口点。
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "framework.h"
#include "WinChatServer.h"
#include <stdio.h>
#include <winsock2.h>
#include "types.h"
#include "SQLiteOp.h"
#include "UserInfoArray.h"
#include "UserDataArray.h"
#include "authentication.h"
#include "endianCov.h"

#pragma comment(lib, "ws2_32.lib") /* WinSock使用的库函数 */

#define MAX_LOADSTRING   100
#define ID_EDIT_LOG      1    // 日志控件标识
#define EDIT_STYLE       (WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | \
                         ES_MULTILINE | WS_HSCROLL | WS_VSCROLL)
#define WINCHAT_MAX_BUF  512
#define WINCHAT_TEMP_BUF 128
#define WINCHAT_MAX_DATA 2048

#define WIN_CHAT_NOTIFY  (WM_USER + 10) /* 自定义socket消息 */
#define WINCHAT_UDP_PORT 6666
#define WINCHAT_MAX_PWD  128
#define DB_FILENAME       ("./model/WinChatDB")


// 全局变量:
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名
HWND hWndLog;                                   // 日志信息窗口句柄
HWND hWndUser;                                  // 用户信息窗口句柄
HWND hWndFile;                                  // 文件信息窗口句柄
SOCKET udpSoc = INVALID_SOCKET;                 // Ser updSoc
char  WinChatBuf[WINCHAT_MAX_DATA];             // 接收数据缓冲区
WC_MSG_HDR msg_hdr;                             // 消息通用头部
char WinChatPwdBuf[WINCHAT_MAX_PWD];            //  用户密码缓冲区

// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void LogPrintf(HWND hWndUser, const TCHAR* szFormat, ...); // 日志输出
SOCKET WinChatCreateUdpSoc(HWND hWnd, unsigned short port); // 创建udp套接字
void WinChatUDPSocketNotify(WPARAM wParam, LPARAM lParam);  // 处理WSAAsyncSelect 
void WinChatUDPController(SOCKET udpsoc);                   // 消息控制器
void WinChatShowAllUsers();

int WinChatLoginProc(const char* data, unsigned short datalen, struct sockaddr_in* peer_addr); // 登陆事件处理函数
int LoginProcUser(const char* namedata, struct sockaddr_in* peer_addr, char* userpwd, int pwdlen);

int LoingProc_SendAuth(int userinfo_index);                              // 发送认证消息
void Debug_Log(const char* data, unsigned int datalen);
void Pack_Common_Hdr(char* buf, unsigned char type, unsigned short len); //通用数据头打包
int WinChatMsgTxtProc(const char* data, unsigned short recvdatalen);
void Send_Online_UsersInfo();
int WinChatMsgBinProc(const char* data, unsigned short recvdatalen, struct sockaddr_in* peer_addr);

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
    err = db_init(DB_FILENAME);
    if (err == -1) return 0;

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
    delete_all_users();
    delete_all_userdata();
    db_defer();
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
    struct sockaddr localaddr;
    int addrlen;
    switch (message){
    case WM_CREATE:
        hWndLog = CreateWindow(TEXT("edit"), NULL, EDIT_STYLE, 0, 0, 0, 0, hWnd, (HMENU)ID_EDIT_LOG, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
        hWndUser = CreateWindow(TEXT("edit"), NULL, EDIT_STYLE, 0, 0, 0, 0, hWnd, (HMENU)ID_EDIT_LOG, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
        hWndFile = CreateWindow(TEXT("edit"), NULL, EDIT_STYLE, 0, 0, 0, 0, hWnd, (HMENU)ID_EDIT_LOG, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
        WinChatShowAllUsers();
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
                    LogPrintf(hWndLog,TEXT("WinChat server is running ...\r\n"));
                }else {
                    LogPrintf(hWndLog,TEXT("WinChat server starts error ...\r\n"));
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
        MoveWindow(hWndLog, 0, 0, cxClient/3 *2, cyClient, FALSE);
        MoveWindow(hWndUser, cxClient / 3 * 2, 0, cxClient- cxClient /3*2, cyClient/2, FALSE);
        MoveWindow(hWndFile, cxClient / 3 * 2, cyClient / 2, cxClient - cxClient / 3 * 2, cyClient-cyClient / 2, FALSE);
        
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

void LogPrintf(HWND hWnd,const TCHAR* szFormat, ...)
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
    iIndex = GetWindowTextLength(hWnd);
    SendMessage(hWnd, EM_SETSEL, (WPARAM)iIndex, (LPARAM)iIndex);
    SendMessage(hWnd, EM_REPLACESEL, FALSE, (LPARAM)szBuffer);
    SendMessage(hWnd, EM_SCROLLCARET, 0, 0);
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
            LogPrintf(hWndLog,TEXT("FD_READ error #%i."), wError);
            return;
        }
        WinChatUDPController(wParam);
        break;
    default:
        LogPrintf(hWndLog,TEXT("Error event type\r\n"));
    }
}

void WinChatUDPController(SOCKET udpsoc) {
    struct sockaddr_in peer_addr;
    int datalen,addr_len = sizeof(peer_addr);
    datalen = recvfrom(udpsoc, WinChatBuf, WINCHAT_MAX_DATA, 0,
        (struct sockaddr*)&peer_addr, &addr_len);
    WinChatBuf[datalen] = '\0'; // append end flag
    // common msg type
    msg_hdr.type = *(char*)WinChatBuf;
    // common msg len
    msg_hdr.len = ntohs( * (short*)(WinChatBuf + sizeof(char)));
    switch (msg_hdr.type) {
        case WC_TYPE_LOGIN:
            WinChatLoginProc(WinChatBuf + WC_MSG_HDR_LEN , msg_hdr.len, &peer_addr);
            break;
        case WC_TYPE_MSG_TXT:
            WinChatMsgTxtProc(WinChatBuf + WC_MSG_HDR_LEN, msg_hdr.len);
            break;
        case WC_TYPE_MSG_BIN:
            WinChatMsgBinProc(WinChatBuf + WC_MSG_HDR_LEN, msg_hdr.len, &peer_addr);
            break;
    }
    
}

int WinChatMsgBinProc(const char* data, unsigned short recvdatalen,struct sockaddr_in* peer_addr) {
    unsigned int fromID;
    unsigned int toID;
    unsigned long long crc64;
    unsigned short  filenameLen;
    fromID = ntohl(*(unsigned int*)data);
    data += sizeof(unsigned int);
    toID = ntohl(*(unsigned int*)data);
    data += sizeof(unsigned int);
    crc64 = toHostEndian(*(unsigned long long*)data);
    data += sizeof(unsigned long long);
    filenameLen = ntohs(*(unsigned short*)data);

    char* start = WinChatBuf + WC_MSG_HDR_LEN;
    *(unsigned long long*)start = toNetEndian(crc64);
    start += sizeof(unsigned long long);
    *(unsigned short*)start = htons(6789);
    int datalen = sizeof(unsigned long long) + sizeof(unsigned short);
    Pack_Common_Hdr(WinChatBuf, WC_TYPE_MSG_BIN_ACK, datalen);
    datalen = sendto(udpSoc, WinChatBuf, datalen + WC_MSG_HDR_LEN, 0, (sockaddr*)peer_addr, sizeof(sockaddr));
    return 0;
}
int WinChatMsgTxtProc(const char* data, unsigned short recvdatalen) {
    WC_MSG_TXT_HDR msg_txt_hdr;
    struct sockaddr_in to_addr;
    int datalen;
    
    int userIndex = 0;
    msg_txt_hdr.toID = ntohl(*((unsigned int*)(data + sizeof(unsigned int))));
    LogPrintf(hWndLog, "toID: %d\r\n", msg_txt_hdr.toID);
    userIndex = search_user(msg_txt_hdr.toID);
    if (userIndex == -1 ) {
        LogPrintf(hWndLog, "ID为: %u 的用户处于离线状态\r\n", msg_txt_hdr.toID);
        return -1;
    }else if (get_user_status_byindex(userIndex) != WC_USR_ON) {
        LogPrintf(hWndLog, "ID为: %u 的用户未登陆\r\n", msg_txt_hdr.toID);
        return -1;
    }

    to_addr.sin_family = AF_INET;
    to_addr.sin_port = htons(get_user_port_byindex(userIndex));
    to_addr.sin_addr.s_addr = inet_addr(get_user_ip_byindex(userIndex)); // 改进：存储时不存储字符串

    datalen = sendto(udpSoc, WinChatBuf, recvdatalen + WC_MSG_HDR_LEN, 0, (sockaddr*)&to_addr, sizeof(to_addr));
    return userIndex;
}

int WinChatLoginProc(const char* data, unsigned short recvdatalen,struct sockaddr_in* peer_addr) {
    WC_LOGIN_COMMON_HDR common_hdr;
    common_hdr.login_type = *(unsigned char*)data;
    int datalen, userIndex;
    unsigned int challenge = 0;
    char challenge_ok = 0;
    switch (common_hdr.login_type) {
        case LOGIN_TYPE:
            userIndex =LoginProcUser(data + WC_LOGIN_COMMON_HDR_LEN, peer_addr, WinChatPwdBuf,WINCHAT_MAX_PWD);
            if (userIndex == -1) {
                // 返回给客户端错误信息
                // ...
                return -1;
            }
            datalen = generate_auth_data(WinChatBuf + WC_MSG_HDR_LEN  + WC_LOGIN_COMMON_HDR_LEN);
            
            challenge = get_auth_code(WinChatBuf + WC_MSG_HDR_LEN + WC_LOGIN_COMMON_HDR_LEN, WinChatPwdBuf);
            update_user_challenge(userIndex, challenge);
            Pack_Common_Hdr(WinChatBuf, WC_TYPE_LOGIN, (unsigned short)(datalen + WC_LOGIN_COMMON_HDR_LEN));
            *(WinChatBuf + WC_MSG_HDR_LEN) = CHALLENGE_TYPE;
            datalen = sendto(udpSoc, WinChatBuf, datalen + +WC_MSG_HDR_LEN + WC_LOGIN_COMMON_HDR_LEN,0,(sockaddr*)peer_addr, sizeof(sockaddr));
            update_user_status_byIndex(userIndex, WC_USR_AUTH);
            break;
        case LOGIN_CHALLENGE_TYPE:
            userIndex = search_user_by_ip_port(inet_ntoa(peer_addr->sin_addr), ntohs(peer_addr->sin_port));
            if (userIndex == -1) {
                // 返回给客户端错误信息
                // ...
                return -1;
            }
            //Pack_Common_Hdr(WinChatBuf, WC_TYPE_LOGIN,  WC_LOGIN_COMMON_HDR_LEN + WC_LOGIN_COMMON_HDR_LEN);
            datalen = WC_MSG_HDR_LEN + WC_LOGIN_COMMON_HDR_LEN;
            Debug_Log(data + WC_LOGIN_COMMON_HDR_LEN, 4 );
            challenge = ntohl(*((unsigned int*)(data + WC_LOGIN_COMMON_HDR_LEN)));
            LogPrintf(hWndLog, "get checkcode: %u\r\n", get_challenge_byindex(userIndex));
            LogPrintf(hWndLog, "get checkcode: %u\r\n", challenge);
            
            if (challenge != get_challenge_byindex(userIndex)) {       
                LogPrintf(hWndLog, "用户: %s 认证失败，拒绝登陆\r\n", get_user_name_byindex(userIndex));
                delete_user_byindex(userIndex);
                *(WinChatBuf + WC_MSG_HDR_LEN) = LOGIN_FAILED;
                Pack_Common_Hdr(WinChatBuf, WC_TYPE_LOGIN, WC_LOGIN_COMMON_HDR_LEN);
                challenge_ok = 0;
            }else {
                LogPrintf(hWndLog, "用户: %s 认证成功\r\n", get_user_name_byindex(userIndex));
                update_user_status_byIndex(userIndex, WC_USR_ON);
                *(WinChatBuf + WC_MSG_HDR_LEN) = LOGIN_SUCCESS;
                *((unsigned int*)(WinChatBuf + datalen)) = htonl(get_userid_byindex(userIndex));
                LogPrintf(hWndLog, "用户: %s 的userid:%u\r\n", get_user_name_byindex(userIndex),get_userid_byindex(userIndex));
                datalen += sizeof(unsigned int);
                Pack_Common_Hdr(WinChatBuf, WC_TYPE_LOGIN, WC_LOGIN_COMMON_HDR_LEN+ sizeof(unsigned int));
                challenge_ok = 1;
            }
            datalen = sendto(udpSoc, WinChatBuf, datalen, 0, (sockaddr*)peer_addr, sizeof(sockaddr));
            if(challenge_ok) Send_Online_UsersInfo();
            
            //*WinChatBuf = WC_TYPE_MSG_TXT;
            //*((unsigned short*)(WinChatBuf + 1)) = htons(19); // len
            //*((unsigned int*)(WinChatBuf + 1 + 2)) = htonl(0xffffffff);
            //*((unsigned int*)(WinChatBuf + 1 + 2+4)) = htonl(get_userid_byindex(userIndex));
            //*((unsigned short*)(WinChatBuf + 1 + 2 + 4 + 4)) = htons(9);
            //strcpy_s(WinChatBuf + 1 + 2 + 4 + 4+2,256,"hello,wkk");
            //datalen = sendto(udpSoc, WinChatBuf, 22, 0, (sockaddr*)peer_addr, sizeof(sockaddr));
            break;
    }
    return 0;
}

int LoginProcUser(const char* namedata, struct sockaddr_in* peer_addr,char * userpwd,int pwdlen) {
    WC_LOGIN_HDR login_hdr;
    int userid = 0;
    int res;
    login_hdr.len = ntohs(*((unsigned char*)namedata));
    const char* username = namedata + WC_LOGIN_HDR_LEN;
    LogPrintf(hWndLog, "用户: %s 正在请求登陆...\r\n", username);
    userid = db_get_userid(username);
    if (userid == -1) {
        LogPrintf(hWndLog, "用户: %s 未注册\r\n", username);
        return -1;
    }
    db_get_userpwd(username, userpwd, pwdlen);
    
    res = search_user(userid);
    if (res != -1) delete_user_byindex(res);
    res = add_user(username, userid, inet_ntoa(peer_addr->sin_addr),
            ntohs(peer_addr->sin_port));
    return res;
}

void WinChatShowAllUsers() {
    int nrow, ncol, i;
    int userid;
    char** pres = NULL;
    db_get_useinfo(NULL, &nrow, &ncol, &pres);
    LogPrintf(hWndUser,"--**************--\r\n");
    LogPrintf(hWndUser,"%s\t%s\r\n","用户ID","用户名");
    for (i = 0; i < nrow; i++) {
        userid = atoi(*(pres + (i + 1) * ncol));
        LogPrintf(hWndUser,"%05d\t%s\r\n", userid, *(pres+(i + 1) * ncol+1));
    }
    LogPrintf(hWndUser,"--**************--\r\n");
    sqldb_free_table(pres);
}


void Debug_Log(const char* data, unsigned int datalen) {
    LogPrintf(hWndLog, "------------\r\n");
    for (unsigned short  i = 0; i < datalen; i++) {
        LogPrintf(hWndLog, "%d\r\n", *(data + i));
    }
    LogPrintf(hWndLog, "------------\r\n");
}


void Pack_Common_Hdr(char* buf, unsigned char type, unsigned short len) {
    *((unsigned char*)buf) = type;
    *((unsigned short*)(buf + sizeof(unsigned char))) = htons(len);
}


void Send_Online_UsersInfo() {
    // 可以计算出实际所需的空间
    //unsigned int space = WC_GRP_LIST_HDR_LEN + WC_MSG_HDR_LEN + WC_GRP_ITEM_LEN * get_online_usersnum() + get_online_namespace();
    
    unsigned int datalen = get_online_userinfo(WinChatBuf+WC_GRP_LIST_HDR_LEN + WC_MSG_HDR_LEN) + WC_GRP_LIST_HDR_LEN;
    Pack_Common_Hdr(WinChatBuf, WC_TYPE_GRP_LST, htons(datalen));
    char* grplist = WinChatBuf + WC_MSG_HDR_LEN;
    *(grplist) = WC_LIST_NEWMSG;
    grplist += sizeof(char);
    *(unsigned int*)grplist = 0;
    grplist += sizeof(unsigned int);
    *(unsigned short*)grplist = htons(get_online_usersnum());
    sendto_online_users(udpSoc, WinChatBuf, datalen + WC_MSG_HDR_LEN);
}

