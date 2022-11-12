#pragma once
#include <stdio.h>
#include "sqlite3.h"
/**
*  用户表结构
*  USERID
*  USERNAME
*  USERPWD
*  USERSEX
*  USRAGE
*  USERDES
*/

typedef int (*SQLDB_OPEN_FUNC)(
	const char* filename,   /* Database filename (UTF-8) */
	sqlite3** ppDb          /* OUT: SQLite db handle */
	);
typedef int (*SQLDB_CLOSE_FUNC)(sqlite3*);
typedef int (*SQLDB_EXEC_FUNC)(
	sqlite3*,                                  /* An open database */
	const char* sql,                           /* SQL to be evaluated */
	int (*callback)(void*, int, char**, char**),  /* Callback function */
	void*,                                    /* 1st argument to callback */
	char** errmsg                              /* Error msg written here */
);

typedef int (*SQLDB_GET_TABLE_FUNC)(
	sqlite3* db,          /* An open database */
	const char* zSql,     /* SQL to be evaluated */
	char*** pazResult,    /* Results of the query */
	int* pnRow,           /* Number of result rows written here */
	int* pnColumn,        /* Number of result columns written here */
	char** pzErrmsg       /* Error msg written here */
);
typedef void (*SQLDB_FREE_TABLE_FUNC)(char** result);

extern SQLDB_FREE_TABLE_FUNC sqldb_free_table;


int db_init(const char* filename);
void db_defer();
int db_add_user(const char* username, const char* userpwd);
int is_user_exist(const char* username);
int db_get_useinfo(const char* username, int* nrow, int* ncol, char*** pres);
int db_get_userid(const char* username);

