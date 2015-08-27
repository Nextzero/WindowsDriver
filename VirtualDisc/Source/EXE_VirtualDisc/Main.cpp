#include <stdio.h>
#include "../ZShareFiles/ZShareDef.h"
#include "LoadDriver.h"

int main(int vArgc, char** vArg)
{
	if(LoadNTDriverA("SYS_VirtualDisc","SYS_VirtualDisc.sys"))
		printf("LoadNTDriverA\n");
	else
		printf("LoadNTDriverA fail ,%d\n",GetLastError());
	getchar();

	//ӳ���̷�
	if(FALSE == DefineDosDeviceA(DDD_RAW_TARGET_PATH,"F:",DEVICE_NAME))//����֮ǰд����"\\??\\F:"��ʾ��������
		printf("DefineDosDevice create fail %d\n",GetLastError());
	else
		printf("DefineDoDevice create success\n");

	getchar();

	//��������
	if(LoadNTDriverA("SYS_LayerDriver","SYS_LayerDriver.sys"))
		printf("LoadNTDriverA\n");
	else
		printf("LoadNTDriverA fail ,%d\n",GetLastError());
	getchar();


	//�򿪾�,���´���Ϊ������������֮�ϵ��ļ�ϵͳ�Ƴ�
	DWORD tRet = 0;
	HANDLE tVolume = CreateFile("\\\\.\\VirtualDisc",GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if(INVALID_HANDLE_VALUE == tVolume)
	{
		printf("open volume fail %d\n",GetLastError());
	}
	else
		printf("open volume\n");


	getchar();
	if(FALSE == DeviceIoControl(tVolume,FSCTL_LOCK_VOLUME,0,0,0,0,&tRet,0))
	{
		printf("FSCTL_LOCK_VOLUME fail %d\n",GetLastError());
	}
	else
	{
		printf("FSCTL_LOCK_VOLUME \n");
	}
	getchar();
	if(FALSE == DeviceIoControl(tVolume,FSCTL_DISMOUNT_VOLUME,0,0,0,0,&tRet,0))
	{
		printf("FSCTL_DISMOUNT_VOLUME fail %d\n",GetLastError());
	}
	else
	{
		printf("FSCTL_DISMOUNT_VOLUME\n");
	}
	getchar();
	if(FALSE == DeviceIoControl(tVolume,FSCTL_UNLOCK_VOLUME ,0,0,0,0,&tRet,0))
	{
		printf("FSCTL_UNLOCK_VOLUME fail %d\n",GetLastError());
	}
	else
	{
		printf("FSCTL_UNLOCK_VOLUME\n");
	}
	getchar();
	CloseHandle(tVolume);

	//ȡ���̷�ӳ��
	if(FALSE == DefineDosDeviceA(DDD_REMOVE_DEFINITION,"F:",0))
		printf("DefineDosDevice remove fail %d\n",GetLastError());
	else
		printf("DefineDoDevice remove success\n");


	//ж�ظ�������
	if(UnloadNTDriverA("SYS_LayerDriver"))
		printf("UnloadNTDriverA\n");
	else
		printf("UnloadNTDriverA fail ,%d\n",GetLastError());
	getchar();


	if(UnloadNTDriverA("SYS_VirtualDisc"))
		printf("UnloadNTDriverA\n");
	else
		printf("UnloadNTDriverA fail ,%d\n",GetLastError());

	getchar();

	return 0;
}