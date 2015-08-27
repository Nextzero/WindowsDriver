#include <stdio.h>
#include "LoadDriver.h"

int main(int vArgc, char** vArg)
{


	if(LoadNTDriverA("HelloDDKDeviceRW","HelloDDKDeviceRW.sys"))
		printf("LoadNTDriverA\n");
	else
		printf("LoadNTDriverA fail ,%d\n",GetLastError());
	getchar();
/*
	HANDLE tDevice = CreateFile("\\\\.\\HelloDDKDeviceRW",GENERIC_WRITE | GENERIC_READ, 0,NULL,OPEN_EXISTING,0,NULL);
	if(tDevice != INVALID_HANDLE_VALUE)
		printf("Createfile success!\n");
	else
		printf("CreateFile fail,%d\n",GetLastError());
	getchar();

	char tBuff[64] = {"My name is CZHC"};
	DWORD  tlen = sizeof(tBuff);

	if(WriteFile(tDevice,tBuff,sizeof(tBuff),&tlen,0))
	{
		printf("Write success\n");
		printf("i write :%s  ",tBuff);
	}
	else
	{
		printf("Write fail\n");
	}

	memset(tBuff,0,sizeof(tBuff));
	if(ReadFile(tDevice,tBuff,sizeof(tBuff),&tlen,0))
	{
		printf("Read success\n");
		printf("i read :%s\n",tBuff);
	}
	else
	{
		printf("read fail\n");
	}

	CloseHandle(tDevice);
*/
	if(UnloadNTDriverA("HelloDDKDeviceRW"))
		printf("UnloadNTDriverA\n");
	else
		printf("UnloadNTDriverA fail ,%d\n",GetLastError());

	getchar();
	return 0;
}