#pragma once
#include <stdio.h>

// user status
enum WinChatUsrStatus {
	WC_USR_INVALID,
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

int add_user(const char* username, unsigned int userid, const char* user_ip, unsigned short user_port);
int update_user_status(unsigned int userid, int user_status);
void delete_all_users();




