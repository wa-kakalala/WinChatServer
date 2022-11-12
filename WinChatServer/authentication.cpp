#include "authentication.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <winsock2.h>
#define RAND_NUM_MIN 10
#define RAND_NUM_MAX 32


int generate_auth_data(char* databuf) {
	srand((unsigned int)time(NULL));
	*databuf = (char)(rand() % (RAND_NUM_MAX - RAND_NUM_MIN) + RAND_NUM_MIN);
	int bitnums = (*databuf / 8) + (*databuf %8?1:0);
	int bitindex = 0;
	int randdata_l= 0;
	short randdata_s = 0;
	char* startpos = databuf + 1 + bitnums;
	for (int i = 0; i < *databuf; i++) {
		if (rand() % 2) {  // 1 -> 4�ֽ�����
			*(databuf + 1+bitindex / 8) = *(databuf+1+ + bitindex / 8) | (0x01 << bitindex % 8);
			randdata_l = (int)(rand()<<16 + rand());
			*((int*)startpos) = htonl(randdata_l);
			bitindex++;
			startpos += 4;
		}else { // 0 -> 2 �ֽ�����
			*(databuf +1+ bitindex / 8) = *(databuf+1 + bitindex / 8) & (~(0x01 << bitindex % 8));
			randdata_s = (short)(rand());
			*((short*)startpos) = htons(randdata_s);
			bitindex++;
			startpos += 2;
		}
	}
	return startpos - databuf; // ���س���
}

int get_auth_code(const char* authdata, const char* username) {
	unsigned char N = *authdata;
	unsigned int sumdata = 0;
	int   data_l = 0;
	short data_s = 0;
	const char * startpos = authdata + 1 + (N / 8) + (N % 8 ? 1 : 0);
	for (int i = 0; i < N; i++) {
		if (*(authdata + 1 + i / 8) & ((0x01 << i % 8))) {  // 1 -> 4�ֽ�
			data_l = *(int*)startpos;
			sumdata += ntohl(data_l);
			startpos += 4;
		}else {  // 0 -> 2 �ֽ�
			data_s = *(int*)startpos;
			sumdata += ntohs(data_s);
			startpos += 2;
		}
	}
	return sumdata;
}

