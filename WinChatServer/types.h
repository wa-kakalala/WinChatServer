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

enum WinChatLoginType {
	LOGIN_ACK,
	LOING_TYPE,
	AUTH_0_TYPE, // server to client
	AUTH_2_TYPE, // client to server

	LOGIN_ERR_NOUSER = 10,
	LOGIN_ERR_PWD,
};

// common format
typedef struct WC_MSG_HDR {
	unsigned char type;
	unsigned short len;
}WC_MSG_HDR;// WC_USER_INFO


// common login fromat
typedef struct WC_LOGIN_COMMON_HDR {
	unsigned char login_type;
}; // WC_LOGIN_COMMON_HDR

// login format
typedef struct WC_LOGIN_HDR {
	unsigned short len;
}WC_LOGIN_HDR; // WC_LOGIN_HDR
