//---------------------------------------------------------------------------
#ifndef zsharedef_H_
#define zsharedef_H_
//---------------------------------------------------------------------------
#include "ZVersion.h"

#ifdef _NTDDK_
#define __T(x)      L##x
#else
#define __T(x)      x
#endif

#define DEVICE_NAME				__T("\\Device\\VirtualDisc")
#define DEVICE_SYMBOLICLINK		__T("\\??\\VirtualDisc")
#define FILE_NAME				__T("\\??\\D:\\VistualDisc.disc")

#define DEVICE_NAME_FILTER			__T("\\Device\\FilterDevice")
#define DEVICE_SYMBOLICLINK_FILTER	__T("\\??\\FilterSymbolic")

#define DevSectorTotal 1048576   //ÉÈÇø×ÜÊý

typedef struct 
{
	int i;
}MY_DEVOBJ_EXTENSION;
//---------------------------------------------------------------------------
#endif
