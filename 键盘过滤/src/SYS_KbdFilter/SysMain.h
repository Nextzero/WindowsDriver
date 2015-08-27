#ifndef SysMainH
#define SysMainH

#ifdef __cplusplus
extern "C"
{
#endif
#include <NTDDK.h>
#include <ntddkbd.h>
NTKERNELAPI
NTSTATUS ObReferenceObjectByName(
		IN PUNICODE_STRING ObjectName,
		IN ULONG Attributes,
		IN PACCESS_STATE PassAccesState OPTIONAL,
		IN ACCESS_MASK DesiredAccess OPTIONAL,
		IN POBJECT_TYPE ObjectType,
		IN KPROCESSOR_MODE AccessMode,
		IN OUT PVOID PareseContext OPTIONAL,
		OUT PVOID * Object
		);
extern POBJECT_TYPE IoDeviceObjectType;
#ifdef __cplusplus
}
#endif 

#include "../ZSharefile/define.h"
#include "../ZSharefile/mi_kbdfilter.h"

#define Debug(_X_) { KdPrint(("SYS_ComFilter:"));KdPrint((_X_));KdPrint(("\n")); }

typedef struct _MyDEVICE_EXTENSION 
{
	PDEVICE_OBJECT LowDeviceObj;
	PIRP	CurIrp;
}MYDEVICE_EXTENSION, *PMYDEVICE_EXTENSION;

#endif