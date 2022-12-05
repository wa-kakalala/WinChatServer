#include "UserInfoArray.h"
#include <string.h>
#include <stdlib.h>
#define MAX_USER_NUM     100

WC_USER_INFO UserInfo[MAX_USER_NUM];            // 记录用户信息

int UserNum; // 记录当前用户的数量

/**
* 默认就是这种状态
*/
void userinfo_init() {
	for (int i = 0; i < MAX_USER_NUM; i++) {
		UserInfo[i].user_status = WC_USR_INVALID;  // 初始所有的都是无效的
	}
}

int search_user(unsigned int userid) {
	for (int i = 0; i < MAX_USER_NUM; i++) {
		if (UserInfo[i].user_status == WC_USR_INVALID) continue;
		if (userid ==  UserInfo[i].userid) return i;
	}
	return -1;
}

int search_user_by_ip_port(const char * ip,unsigned short port) {
	for (int i = 0; i < MAX_USER_NUM; i++) {
		if (UserInfo[i].user_status == WC_USR_INVALID) continue;
		if ((port ==  UserInfo[i].user_port) && ( strcmp(ip, UserInfo[i].user_ip) == 0)) return i;
	}
	return -1;
}



int delete_user(unsigned int userid) {
	int index = search_user(userid);
	if (index == -1) return -1;
	delete_user_byindex(index);
	return 0;
}

int delete_user_byindex(unsigned int index) {
	if (UserInfo[index].user_status == WC_USR_INVALID) return -1;
	free(UserInfo[index].username);
	UserInfo[index].username = NULL;
	free(UserInfo[index].user_ip);
	UserInfo[index].user_ip = NULL;
	UserInfo[index].user_status = WC_USR_INVALID;
	UserInfo[index].challenge = 0;
	UserInfo[index].user_port = 0;
	UserNum--;
	return 0;
}

int add_user(const char* username, unsigned int userid, const char* user_ip, unsigned short user_port) {
	if (UserNum >= MAX_USER_NUM) return -1; // 已满
	if (username == NULL || user_ip == NULL) return -1;
	size_t ip_len = strlen(user_ip);
	size_t name_len = strlen(username);
	WC_USER_INFO user_temp;
	user_temp.userid = userid;
	user_temp.user_port = user_port;
	user_temp.user_status = WC_USR_OFF;
	user_temp.user_ip = (char*)malloc(ip_len + 1); 
	if (user_temp.user_ip == NULL) return -1;
	strcpy_s(user_temp.user_ip,ip_len+1,user_ip);
	user_temp.user_ip[ip_len] = 0;
	user_temp.username = (char*)malloc(name_len + 1);
	if (user_temp.username == NULL) {
		free(user_temp.user_ip);
		user_temp.user_ip = NULL;
		return -1;
	}
	strcpy_s(user_temp.username,name_len+1, username);
	user_temp.username[name_len] = 0;

	int index = 0;
	for ( ; index < MAX_USER_NUM; index++) {
		if (UserInfo[index].user_status == WC_USR_INVALID) {
			UserInfo[index] = user_temp;
			break;
		}
	}
	UserNum++;
	return index;
}

int update_user_status(unsigned int userid, int user_status) {
	int index = search_user(userid);
	if (index == -1) return -1;
	UserInfo[index].user_status = user_status;
	return 0;
}

int update_user_status_byIndex(unsigned int index, int user_status) {
	UserInfo[index].user_status = user_status;
	return 0;
}

int update_user_challenge(int index, unsigned int challenge) {
	UserInfo[index].challenge = challenge;
	return 0;
}

void delete_all_users() {
	for (int i = 0; i < MAX_USER_NUM; i++) {
		if (UserInfo[i].user_status == WC_USR_INVALID) continue;
		delete_user_byindex(i);
	}
}

char* get_user_name_byindex(int index) {
	return UserInfo[index].username;
}

unsigned int get_challenge_byindex(int index) {
	return UserInfo[index].challenge;
}

unsigned short get_user_port_byindex(int index) {
	return UserInfo[index].user_port;
}

char* get_user_ip_byindex(int index) {
	return UserInfo[index].user_ip;
}

unsigned int get_userid_byindex(int index) {
	return UserInfo[index].userid;
}

unsigned char get_user_status_byindex(int index) {
	return UserInfo[index].user_status;
}



