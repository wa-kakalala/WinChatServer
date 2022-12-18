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
	LOGIN_TYPE,
	LOGIN_CHALLENGE_TYPE,
	LOGIN_FAILED,
	LOGIN_SUCCESS
};

enum WinChatChallengeType {
	NO_USER_TYPE,
	CHALLENGE_TYPE
};

// common format
typedef struct WC_MSG_HDR {
	unsigned char type;
	unsigned short len;
}WC_MSG_HDR;// WC_USER_INFO
#define WC_MSG_HDR_LEN (sizeof(unsigned char ) + sizeof(unsigned short))

// common login fromat
typedef struct WC_LOGIN_COMMON_HDR {
	unsigned char login_type;
}WC_LOGIN_COMMON_HDR; // WC_LOGIN_COMMON_HDR
#define WC_LOGIN_COMMON_HDR_LEN ( sizeof(unsigned char) )

// login format
typedef struct WC_LOGIN_HDR {
	unsigned char len;
}WC_LOGIN_HDR; // WC_LOGIN_HDR
#define WC_LOGIN_HDR_LEN ( sizeof(unsigned char) )


// msg txt format
typedef struct WC_MSG_TXT_HDR {
	unsigned int fromID;
	unsigned int toID;
	unsigned short len;
}WC_MSG_TXT_HDR;
#define WC_MSG_TXT_HDR_LEN ( sizeof(unsigned int) *2 + sizeof(unsigned short) )

// grp lst format
typedef struct WC_GRP_LIST_HDR {
	unsigned char	type;
	unsigned int	groupID;
	unsigned short  itemlen;
};
#define WC_GRP_LIST_HDR_LEN  ( sizeof(unsigned char) + sizeof(unsigned int) + sizeof(unsigned short))

// item id
typedef struct WC_GRP_ITEM {
	unsigned long long crc64;
	unsigned short     len;
};
#define WC_GRP_ITEM_LEN (sizeof(unsigned long long) + sizeof(unsigned short))

enum GrpListType {
	WC_LIST_MEM		,
	WC_LIST_FILE	,
	WC_LIST_ONLINE  ,
	WC_LIST_NEWMSG
};


