#include <stdio.h>
#include "LoadDriver.h"

int main(int vArgc, char** vArg)
{


	if(LoadNTDriverA("HelloDDK","HelloDDK.sys"))
		printf("LoadNTDriverA\n");
	else
		printf("LoadNTDriverA fail ,%d\n",GetLastError());
	getchar();

	HANDLE tDevice = CreateFile("\\\\.\\HelloDDK",GENERIC_WRITE | GENERIC_READ, 0,NULL,OPEN_EXISTING,0,NULL);
	if(tDevice != INVALID_HANDLE_VALUE)
	{
		CloseHandle(tDevice);
		printf("Createfile success!\n");
	}
	else
		printf("CreateFile fail,%d\n",GetLastError());
	getchar();
	getchar();
	getchar();

	if(UnloadNTDriverA("HelloDDK"))
		printf("UnloadNTDriverA\n");
	else
		printf("UnloadNTDriverA fail ,%d\n",GetLastError());

	getchar();
/*
	if(0 == strcmp(vArg[1],"install"))
	{
		printf("命令:%s, %s, %s\n",vArg[1],vArg[2],vArg[3]);
		if(TRUE == LoadNTDriverA(vArg[2],vArg[3]))
			printf("安装成功\n");
		else
			printf("安装失败,%d\n",GetLastError());
	}

	if(0 == strcmp(vArg[1],"uninstall"))
	{
		printf("命令:%s, %s, %s\n",vArg[1],vArg[2],vArg[3]);
		if(TRUE == UnloadNTDriverA(vArg[2]))
			printf("卸载成功\n");
		else
			printf("卸载失败%d\n",GetLastError());
	}*/

	return 0;
}