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

//设备类型
#define _MyControlDevice		0
#define _FilterFsMasterDevice	1
#define _FilterFsRollDevice		2

#define IsNeedFilterType(_X_)	(_X_->DeviceType == FILE_DEVICE_DISK_FILE_SYSTEM)

struct DeviceExtend
{
	int				Type;				//应用程序控制设备; 文件系统控制设备过滤; 文件系统卷设备过滤;
	PDEVICE_OBJECT	DirctLowDevObj;
	PDEVICE_OBJECT	RealStorageDeviceObj;
};

extern PDRIVER_OBJECT gMyDriverObject;
#endif