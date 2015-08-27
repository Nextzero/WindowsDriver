#ifndef SysMainH
#define SysMainH

#ifdef __cplusplus
	extern "C"
	{
#endif
	#include <ntifs.h>
	#include <NTDDK.h>
	#include <Ntddstor.h>
	#include <fltKernel.h>
#ifdef __cplusplus
	}
#endif 

#define Debug(_X_) { KdPrint(("SYS_MinFsFilter:"));KdPrint((_X_));KdPrint(("\n")); }

#define IsNeedFilterType(_X_)	(_X_->DeviceType == FILE_DEVICE_DISK_FILE_SYSTEM)

NTSTATUS FltFilterUnload(FLT_FILTER_UNLOAD_FLAGS vFlags);
NTSTATUS FltInstanceSetup(
						  IN PCFLT_RELATED_OBJECTS  FltObjects,
						  IN FLT_INSTANCE_SETUP_FLAGS  Flags,
						  IN DEVICE_TYPE  VolumeDeviceType,
						  IN FLT_FILESYSTEM_TYPE  VolumeFilesystemType );

FLT_PREOP_CALLBACK_STATUS FltPreCreate(
									   IN OUT PFLT_CALLBACK_DATA Data,
									   IN PCFLT_RELATED_OBJECTS FltObjects,
									   OUT PVOID *CompletionContext );

FLT_POSTOP_CALLBACK_STATUS FltPostCreate(
										__inout PFLT_CALLBACK_DATA Data,
										__in PCFLT_RELATED_OBJECTS FltObjects,
										__in_opt PVOID CompletionContext,
										__in FLT_POST_OPERATION_FLAGS Flags );
#endif