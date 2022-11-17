#include "authentication.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <winsock2.h>
#include "WinChatServer.h"
#include "crc.h"
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
		if (rand() % 2) {  // 1 -> 4字节数据
			*(databuf + 1+bitindex / 8) = *(databuf+1+ bitindex / 8) | (0x01 << bitindex % 8);
			randdata_l = (int)((rand()<<16) | rand());
			*((int*)startpos) = htonl(randdata_l);
			bitindex++;
			startpos += 4;
		}else { // 0 -> 2 字节数据
			*(databuf +1+ bitindex / 8) = *(databuf+1 + bitindex / 8) & (~(0x01 << (bitindex % 8)));
			randdata_s = (short)(rand());
			*((short*)startpos) = htons(randdata_s);
			bitindex++;
			startpos += 2;
		}
	}
	return startpos - databuf; // 返回长度
}

unsigned int  get_auth_code(const char* authdata, const char* pwd) {
	unsigned char N = *authdata;
	//LogPrintf(hWndLog, "N:%d\r\n", N);
	unsigned int sumdata = 0;
	int   data_l = 0;
	short data_s = 0;
	const char * startpos = authdata + 1 + (N / 8) + (N % 8 ? 1 : 0);
	for (int i = 0; i < N; i++) {
		if (*(authdata + 1 + i / 8) & (0x01 << (i % 8))) {  // 1 -> 4字节
			data_l = *(unsigned int*)startpos;
			sumdata += ntohl(data_l);
			//LogPrintf(hWndLog, "%u\r\n", ntohl(data_l));
			
			startpos += 4;
		}else {  // 0 -> 2 字节
			data_s = *(unsigned short*)startpos;
			sumdata += ntohs(data_s);
			//LogPrintf(hWndLog, "%u\r\n", ntohs(data_s));
			startpos += 2;
		}
	}
	//LogPrintf(hWndLog, "sumdata: %u,pwd: %s\r\n", sumdata|0x01,pwd);
	sumdata = crc32((unsigned char *)pwd, strlen(pwd), sumdata | 0x01);
	//LogPrintf(hWndLog, "sumdata: %u,pwd: %s\r\n", sumdata, pwd);
	return sumdata;
}


