#ifndef __FuncHelper_H
#define __FuncHelper_H

#ifdef __cplusplus
extern "C"
{
#endif
#include <ntifs.h>
#include <NTDDK.h>
#ifdef __cplusplus
}
#endif 

NTSTATUS PrintDeviceName	(PVOID vDeviceObject);
NTSTATUS PrintIrp			(PIRP vIrp);
#endif