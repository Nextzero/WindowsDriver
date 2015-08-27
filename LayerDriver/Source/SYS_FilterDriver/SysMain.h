//---------------------------------------------------------------------------
#ifndef SysMainH
#define SysMainH
//---------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" 
{
#endif

#include <ntddk.h>
#include <ntdddisk.h>

#ifdef __cplusplus
}
#endif

#define Debug(_X_)		{KdPrint(("SYS_FilterDriver: "));KdPrint((_X_)); KdPrint(("\n"));}

typedef struct _MyDeviceExtend
{
	PDEVICE_OBJECT LowDevice;
	int LowDeviceReadLimit;

}MyDeviceExtend;

typedef struct _MyContext
{
	PMDL NewMdl;
	PMDL PreMdl;
	ULONG LeftLength;
	ULONG AlreadyLength;
	PVOID LeftVirtualAddress;
}MyContext;
//---------------------------------------------------------------------------
// Interface export files.
#include "../ZShareFiles/ZShareDef.h"
//---------------------------------------------------------------------------
// Share define file in here.
//---------------------------------------------------------------------------
// Myself include file in here.
//---------------------------------------------------------------------------
#endif