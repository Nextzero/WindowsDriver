#ifndef SysMainH
#define SysMainH

#ifdef __cplusplus
extern "C"
{
#endif
#include <ntifs.h>
#include <NTDDK.h>
#ifdef __cplusplus
}
#endif 

#include "fastIO.h"
#include "FuncHelper.h"

#define Debug(_X_) { KdPrint(("SYS_FSFilter-->"));KdPrint((_X_));KdPrint(("\n")); }

//�豸����
#define _MyControlDevice		0
#define _FilterFsMasterDevice	1
#define _FilterFsRollDevice		2

#define IsNeedFilterType(_X_)	(_X_->DeviceType == FILE_DEVICE_DISK_FILE_SYSTEM)

struct DeviceExtend
{
	int				Type;				//Ӧ�ó�������豸; �ļ�ϵͳ�����豸����; �ļ�ϵͳ���豸����;
	PDEVICE_OBJECT	DirctLowDevObj;
	PDEVICE_OBJECT	RealStorageDeviceObj;
};

extern PDRIVER_OBJECT gMyDriverObject;
#endif