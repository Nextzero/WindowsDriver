#ifndef _LOAD_DRIVER_H_
#define _LOAD_DRIVER_H_

#include <windows.h>  
#include <winsvc.h>  

//#include "MC_RegHelper.hpp"

#pragma comment(lib, "Advapi32.lib")

BOOL LoadNTDriverA	(char* lpszDriverName,char* lpszDriverPath);
BOOL UnloadNTDriverA(char* szSvrName);

BOOL LoadNTDriverW	(WCHAR* lpszDriverName,WCHAR* lpszDriverPath);
BOOL UnloadNTDriverW(WCHAR * szSvrName);

bool Start			(char* vServiceName);
bool Stop			(char* vServiceName);
bool IsExists		(char* vServiceName);
bool IsRunning		(char* szSvrName);
bool Remove			(char* vServiceName);
bool SafeBootUnReg	(char* vServiceName);

#endif