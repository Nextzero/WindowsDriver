#include <stdio.h>
#include "../ZShareFiles/ZShareDef.h"
#include "LoadDriver.h"

int main(int vArgc, char** vArg)
{
	if(LoadNTDriverA("SYS_Example","SYS_Example.sys"))
		printf("LoadNTDriverA\n");
	else
		printf("LoadNTDriverA fail ,%d\n",GetLastError());
	getchar();

	if(LoadNTDriverA("SYS_CallDriver","SYS_CallDriver.sys"))
		printf("LoadNTDriverA\n");
	else
		printf("LoadNTDriverA fail ,%d\n",GetLastError());
	getchar();

	//调用SYS_CallDriver ReadFile函数，内部将调用SYS_Example完成功能
	HANDLE tDevice = CreateFile("\\\\.\\symbolic_CallDriver",GENERIC_WRITE|GENERIC_READ,0,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
	if(INVALID_HANDLE_VALUE == tDevice)
		printf("CreateFile CallDriver fail %d\n",GetLastError());

	char tBuf[10] = {0};
	DWORD tLen = sizeof(tBuf);
	if(FALSE == ReadFile(tDevice,tBuf,sizeof(tBuf),&tLen,0))
		printf("ReadFile fail %d\n",GetLastError());

	if(INVALID_HANDLE_VALUE != tDevice)
		CloseHandle(tDevice);

	getchar();
	if(UnloadNTDriverA("SYS_CallDriver"))
		printf("UnloadNTDriverA SYS_CallDriver\n");
	else
		printf("UnloadNTDriverA SYS_CallDriver fail ,%d\n",GetLastError());

	getchar();

	if(UnloadNTDriverA("SYS_Example"))
		printf("UnloadNTDriverA SYS_Example \n");
	else
		printf("UnloadNTDriverA  SYS_Example fail ,%d\n",GetLastError());

	getchar();

	return 0;
}