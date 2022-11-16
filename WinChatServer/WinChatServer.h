#pragma once

#include "resource.h"
#include <windows.h>

extern HWND hWndLog;                                   // 日志信息窗口句柄
void LogPrintf(HWND hWndUser, const TCHAR* szFormat, ...); // 日志输出
