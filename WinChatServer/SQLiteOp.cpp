#include "SQLiteOp.h"
#include <Windows.h>

SQLDB_OPEN_FUNC sqldb_open = NULL;
SQLDB_CLOSE_FUNC sqldb_close = NULL;
SQLDB_EXEC_FUNC sqldb_exec = NULL;
SQLDB_GET_TABLE_FUNC sqldb_get_table = NULL;
SQLDB_FREE_TABLE_FUNC sqldb_free_table = NULL;

sqlite3* pDB = NULL;
HMODULE hDLL = NULL;

char* buf[255];


/**
* �ο�
* http://t.csdn.cn/KD7G3
*/
int dll_init() {
	int res = 0;
	hDLL = LoadLibrary("sqlite3.dll");
	if (hDLL == NULL) return 0;
	sqldb_open = (SQLDB_OPEN_FUNC)GetProcAddress(hDLL, "sqlite3_open");
	sqldb_close = (SQLDB_CLOSE_FUNC)GetProcAddress(hDLL, "sqlite3_close");
	sqldb_exec = (SQLDB_EXEC_FUNC)GetProcAddress(hDLL, "sqlite3_exec");
	sqldb_get_table = (SQLDB_GET_TABLE_FUNC)GetProcAddress(hDLL, "sqlite3_get_table");
	sqldb_free_table = (SQLDB_FREE_TABLE_FUNC)GetProcAddress(hDLL, "sqlite3_free_table");
	return 0;
}

/**
* �ο�
* http://t.csdn.cn/eVNYK
* http://t.zoukankan.com/hushaojun-p-5257935.html
* http://t.csdn.cn/c8Of6
*/

int db_open(const char* filename) {
	int res = sqldb_open(filename, &pDB);
	if (res != SQLITE_OK) return -1;
	return 0;
}

int db_init(const char* filename) {
	int res = 0;
	dll_init();
	res = db_open(filename);
	return res;
}

void db_defer() {
	sqldb_close(pDB);
	FreeLibrary(hDLL);
}

int db_add_user(const char* username, const char* userpwd) {
	if (is_user_exist(username)) return -1; // �û��Ѿ�����
	char buf[255] = { 0 };
	sprintf_s(buf,255,"insert into USERINFO (USERNAME,USERPWD) values ('%s','%s');",username,userpwd);
    int nRes = sqldb_exec(pDB, buf, 0, 0,NULL);
    if (nRes != SQLITE_OK)
    {
        return -1;
    }
	return 0;
}

/**
* pres ��ѯ������ڴ��ɺ����ڲ����䣬��Ҫʹ��sqldb_free_table �ͷ�
* ��0�������� ncol - 1���������ֶ����ƣ��ӵ�ncol ������ʼ�����涼���ֶ�ֵ
* ��n�е�m�е����ݣ������pres [(nrow+ 1) * ncol + m]
* nrow: ��ѯ����������¼
* ncol: ��ѯ�����ٸ��ֶ�
*/
int db_get_useinfo(const char * username,int * nrow,int *ncol,char *** pres)
{
	int res;
	char buf[255] = "select * from userinfo";
	if(username != NULL)
		sprintf_s(buf, 255, "select * from userinfo where (USERNAME = '%s');", username);
	res = sqldb_get_table(pDB, buf, pres, nrow, ncol,NULL);
	if (res != SQLITE_OK) return -1;
	return 0;
}

int db_get_userid(const char* username) {
	int nrow, ncol;
	int userid;
	char** pres = NULL;
	db_get_useinfo(username, &nrow, &ncol, &pres);
	sqldb_free_table(pres);
	if (nrow) {
		userid = atoi(pres[(nrow + 1) * ncol]);
	}else {
		userid = -1;
	}
	sqldb_free_table(pres);
	return userid;
}

/**
* �����û�����ѯ��Ӧ���û��Ƿ����
*/
int is_user_exist(const char* username) {
	int nrow, ncol;
	char ** pres = NULL;
	db_get_useinfo(username, &nrow, &ncol, &pres);
	sqldb_free_table(pres);
	return nrow ? 1 : 0;
}


