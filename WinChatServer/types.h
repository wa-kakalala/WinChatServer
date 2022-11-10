#pragma once

// msg type
enum WinChatTYPE {
	WC_TYPE_UNDEF,
	WC_TYPE_LOGIN,
	WC_TYPE_MSG_TXT,
	WC_TYPE_MSG_BIN,
	WC_TYPE_MSG_BIN_ACK,
	WC_TYPE_GRP_JOIN,
	WC_TYPE_GRP_QUIT,
	WC_TYPE_BIN_GET,
	WC_TYPE_GRP_LST,
}; // WinChatTYPE

// user status
enum WinChatUsrStatus {
	WC_USR_OFF,
	WC_USR_AUTH,
	WC_USR_ON
}; // WinChatUsrStatus

// user info
typedef struct WC_USER_INFO {
	char* username;
	unsigned int userid;
	char* user_ip;
	unsigned short user_port;
	unsigned char user_status;
}WC_USER_INFO;// WC_USER_INFO

// common format
typedef struct WC_MSG_HDR {
	unsigned char type;
	unsigned short len;
}WC_MSG_HDR;// WC_USER_INFO
