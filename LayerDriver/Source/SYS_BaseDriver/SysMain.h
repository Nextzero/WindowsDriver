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

#define Debug(_X_)		{KdPrint(("SYS_BaseDriver:"));KdPrint((_X_)); KdPrint(("\n"));}

typedef struct _MyDeviceExtend
{
	int i;
}MyDeviceExtend;

//---------------------------------------------------------------------------
// Interface export files.
#include "../ZShareFiles/ZShareDef.h"
//---------------------------------------------------------------------------
// Share define file in here.
//---------------------------------------------------------------------------
// Myself include file in here.
//---------------------------------------------------------------------------
#endif