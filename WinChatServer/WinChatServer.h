#pragma once

#include "resource.h"
#include <windows.h>

extern HWND hWndLog;                                   // ��־��Ϣ���ھ��
void LogPrintf(HWND hWndUser, const TCHAR* szFormat, ...); // ��־���
