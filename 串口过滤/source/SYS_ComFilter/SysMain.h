#ifndef SysMainH
#define SysMainH

#ifdef __cplusplus
extern "C"
{
#endif
#include <NTDDK.h>
#ifdef __cplusplus
}
#endif 

#define Debug(_X_) { KdPrint(("SYS_ComFilter:"));KdPrint((_X_));KdPrint(("\n")); }

typedef struct _MyDEVICE_EXTENSION 
{
	PDEVICE_OBJECT LowDeviceObj;
}MYDEVICE_EXTENSION, *PMYDEVICE_EXTENSION;

#endif