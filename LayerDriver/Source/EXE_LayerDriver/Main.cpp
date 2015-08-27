#include <stdio.h>
#include "../ZShareFiles/ZShareDef.h"
#include "LoadDriver.h"

int main(int vArgc, char** vArg)
{
	
	if(LoadNTDriverA("SYS_BaseDriver","SYS_BaseDriver.sys"))
		printf("SYS_BaseDriver LoadNTDriverA\n");
	else
		printf("SYS_BaseDriver LoadNTDriverA fail ,%d\n",GetLastError());
	getchar();

/*
	if(LoadNTDriverA("SYS_Example","SYS_Example.sys"))
		printf("SYS_BaseDriver LoadNTDriverA\n");
	else
		printf("SYS_BaseDriver LoadNTDriverA fail ,%d\n",GetLastError());
	getchar();*/

	//¹ýÂËÇý¶¯
	if(LoadNTDriverA("SYS_FilterDriver","SYS_FilterDriver.sys"))
		printf("SYS_FilterDriver LoadNTDriverA\n");
	else
		printf("SYS_FilterDriver LoadNTDriverA fail ,%d\n",GetLastError());
	getchar();


	//Ä£Äâ¶ÁÐ´BaseDriver
	HANDLE tBaseDevice = CreateFile("\\\\.\\BaseSymbolic",GENERIC_READ|GENERIC_WRITE,0,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
	if(INVALID_HANDLE_VALUE == tBaseDevice)
		printf("create file fail, %d\n",GetLastError());
	else
		printf("create file success\n");

	char	tBuf[10] = {0};
	DWORD	tLen = sizeof(tBuf);
	if(FALSE == ReadFile(tBaseDevice,tBuf,sizeof(tBuf),&tLen,0))
		printf("readfile fail,%d\n",GetLastError());
	else
		printf("read file success\n");

	if(INVALID_HANDLE_VALUE != tBaseDevice)
		CloseHandle(tBaseDevice);
	getchar();

	//Ð¶ÔØ¸½¼ÓÇý¶¯
	if(UnloadNTDriverA("SYS_FilterDriver"))
		printf("SYS_FilterDriver UnloadNTDriverA\n");
	else
		printf("SYS_FilterDriver UnloadNTDriverA fail ,%d\n",GetLastError());
	getchar();


	if(UnloadNTDriverA("SYS_BaseDriver"))
		printf("SYS_BaseDriver UnloadNTDriverA\n");
	else
		printf("SYS_BaseDriver UnloadNTDriverA fail ,%d\n",GetLastError());

	getchar();

	return 0;
}