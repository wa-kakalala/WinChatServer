#include "endianCov.h"

// �ο��� https://blog.csdn.net/weixin_62042704/article/details/120994827
unsigned long long toNetEndian(unsigned long long num ) {
	unsigned short test = 1;
	char* ptest = (char*)(&test);
	if (*ptest == 0) {
		return num; // ���ģʽ
	}
	else {
		unsigned long long res = 0;
		ptest = (char*)(&num);
		int numlen = sizeof(unsigned long);
		for (int i = 0; i < numlen; i++) {
			res += ((unsigned long long)(*(ptest + i))) << ((numlen - 1 - i)*8);
		}
	}
}

unsigned long long toHostEndian(unsigned long long num) {
	unsigned short test = 1;
	char* ptest = (char*)(&test);
	if (*ptest == 0) {
		return num; // ���ģʽ
	}
	else {
		unsigned long long res = 0;
		ptest = (char*)(&num);
		int numlen = sizeof(unsigned long);
		for (int i = 0; i < numlen; i++) {
			res += ((unsigned long)(*(ptest + i))) >> ((numlen - 1 - i) * 8);
		}
	}
}