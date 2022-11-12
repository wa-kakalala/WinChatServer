#include "UserDataArray.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_DATA_NUM  100
char* UserData[MAX_DATA_NUM];
unsigned int DataNum;

/**
* Ä¬ÈÏÎªNULL
*/
void userdata_init() {
	for (int i = 0; i < MAX_DATA_NUM; i++) {
		UserData[i] = NULL;
	}
}

int add_userdata(const char* data,unsigned int datalen) {
	if (DataNum >= MAX_DATA_NUM) return -1;
	int index = 0;
	for (; index < MAX_DATA_NUM; index++) {
		if (UserData[index] == NULL) break;
	}
	UserData[index] = (char*)malloc(sizeof(char) * datalen);
	if (UserData[index] == NULL) return -1;
	memcpy_s(UserData[index],datalen, data, datalen);
	DataNum++;
	return index;
}

int delete_userdata(unsigned int index) {
	if (UserData[index] == NULL) return -1;
	free(UserData[index]);
	UserData[index] = NULL;
	DataNum = NULL;
	return index;
}

void delete_all_userdata() {
	for (int i = 0; i < MAX_DATA_NUM; i++) {
		if (UserData[i] == NULL) continue;
		delete_userdata(i);
	}
}






