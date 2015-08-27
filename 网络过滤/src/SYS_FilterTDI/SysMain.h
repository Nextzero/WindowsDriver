#ifndef SysMainH
#define SysMainH

#ifdef __cplusplus
extern "C"
{
#endif
#include <ntifs.h>
#include <NTDDK.h>
#include <TdiKrnl.h>
#ifdef __cplusplus
}
#endif 

#include <stdlib.h>
#include <string.h>
#include "FuncHelper.h"

#define TDI_ADDRESS_MAX_LENGTH	TDI_ADDRESS_LENGTH_OSI_TSAP
#define TA_ADDRESS_MAX			(sizeof(TA_ADDRESS) - 1 + TDI_ADDRESS_MAX_LENGTH)
#define TDI_ADDRESS_INFO_MAX	(sizeof(TDI_ADDRESS_INFO) - 1 + TDI_ADDRESS_MAX_LENGTH)

#define Debug(_X_) { KdPrint(("SYS_FilterTDI:"));KdPrint((_X_));KdPrint(("\n")); }

typedef struct _MyDEVICE_EXTENSION 
{
	PDEVICE_OBJECT LowDeviceObj;
}MYDEVICE_EXTENSION, *PMYDEVICE_EXTENSION;

#endif