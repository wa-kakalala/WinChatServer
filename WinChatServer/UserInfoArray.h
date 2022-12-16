#pragma once
#include <stdio.h>
#include <winsock2.h>

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
	unsigned int challenge;
}WC_USER_INFO;// WC_USER_INFO

int add_user(const char* username, unsigned int userid, const char* user_ip, unsigned short user_port);
int update_user_status(unsigned int userid, int user_status);
void delete_all_users();
int delete_user_byindex(unsigned int index);
int update_user_challenge(int index, unsigned int challenge);
char* get_user_name_byindex(int index);
unsigned int get_challenge_byindex(int index);
int update_user_status_byIndex(unsigned int index, int user_status);
int search_user_by_ip_port(const char* ip, unsigned short port);
unsigned int get_userid_byindex(int index);
int search_user(unsigned int userid);
unsigned char get_user_status_byindex(int index);
char* get_user_ip_byindex(int index);
unsigned short get_user_port_byindex(int index);
int get_online_usersnum();
unsigned int get_online_namespace();
unsigned int get_online_userinfo(char* buf);
int sendto_online_users(SOCKET udpSoc, char* buf, int buflen);




