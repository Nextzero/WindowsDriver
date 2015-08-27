#include "SysMain.h"
//---------------------------------------------------------------
int ProcessNameOffset = 0;
PFLT_FILTER	g_FltFilterHandle = 0;
//-------------------×¢²á½á¹¹-----------------------------------
const FLT_OPERATION_REGISTRATION g_OpCallbacks[]= 
{
	{
		IRP_MJ_CREATE,
		0,
		FltPreCreate,
		FltPostCreate
	},

	{
		IRP_MJ_OPERATION_END
	}
};

const FLT_REGISTRATION g_FltRegistration = 
{
	sizeof(FLT_REGISTRATION ),
	FLT_REGISTRATION_VERSION,
	0,
	0,
	g_OpCallbacks,
	FltFilterUnload,
	0,
	0,
	0,
	0,
	0,
	0,
	0	
};
//---------------------------------------------------------------
FLT_PREOP_CALLBACK_STATUS FltPreCreate(IN OUT PFLT_CALLBACK_DATA Data,
									   IN PCFLT_RELATED_OBJECTS FltObjects,
									   OUT PVOID *CompletionContext)
{
	char tProcessName[512] = {0};
	PEPROCESS tCurProc	= PsGetCurrentProcess();
	char* tNamePtr		= (char*)tCurProc + ProcessNameOffset;
	memcpy(tProcessName, tNamePtr, 15);
	tProcessName[15] = 0;
	KdPrint(("process name:%s\n",tProcessName));
	
	NTSTATUS tRetStatus;
	char FileName[512] = {0};
	PFLT_FILE_NAME_INFORMATION tNameInfo;
	PAGED_CODE();
	tRetStatus = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &tNameInfo);
	if(NT_SUCCESS(tRetStatus))
	{
		FltParseFileNameInformation(tNameInfo);
		KdPrint(("<Pre> file name: %wZ\n", &(tNameInfo->Name)));
		FltReleaseFileNameInformation(tNameInfo);
	}
	//	return FLT_PREOP_COMPLETE;
	return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}
//---------------------------------------------------------------
FLT_POSTOP_CALLBACK_STATUS FltPostCreate(
										__inout PFLT_CALLBACK_DATA Data,
										__in PCFLT_RELATED_OBJECTS FltObjects,
										__in_opt PVOID CompletionContext,
										__in FLT_POST_OPERATION_FLAGS Flags )
{
	NTSTATUS tRetStatus;
	char FileName[512] = {0};
	PFLT_FILE_NAME_INFORMATION tNameInfo;
	PAGED_CODE();
	tRetStatus = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &tNameInfo);
	if(NT_SUCCESS(tRetStatus))
	{
		FltParseFileNameInformation(tNameInfo);
		KdPrint(("<post> file name: %wZ\n", &(tNameInfo->Name)));
		FltReleaseFileNameInformation(tNameInfo);
	}
	return FLT_POSTOP_FINISHED_PROCESSING;
}
//---------------------------------------------------------------
NTSTATUS FltFilterUnload(FLT_FILTER_UNLOAD_FLAGS vFlags)
{
	Debug("Enter FltFilterUnload");
	FltUnregisterFilter(g_FltFilterHandle);
	Debug("Leave FltFilterUnload");
	return STATUS_SUCCESS;
}
//---------------------------------------------------------------
NTSTATUS FltInstanceSetup(IN PCFLT_RELATED_OBJECTS  FltObjects,
						  IN FLT_INSTANCE_SETUP_FLAGS  Flags,
						  IN DEVICE_TYPE  VolumeDeviceType,
						  IN FLT_FILESYSTEM_TYPE  VolumeFilesystemType)
{
	Debug("FltInstanceSetup");
	return STATUS_SUCCESS;
}

//---------------------------------------------------------------
VOID DriverUnload(IN PDRIVER_OBJECT pDriverObject)
{
}
//---------------------------------------------------------------
extern "C" NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject,IN PUNICODE_STRING pRegistryPath) 
{
	NTSTATUS tRetStatus;
	PEPROCESS tCurProcess = PsGetCurrentProcess();
	if(0 == tCurProcess)
		return 0;

	for(int i=0; i<3*PAGE_SIZE; i++)
	{
		if(0 == _strnicmp("System", (char*)tCurProcess + i, strlen("System"))) 
		{
			ProcessNameOffset	= i;
			break;
		}			
	}
	if(0 == ProcessNameOffset)
	{
		return STATUS_UNSUCCESSFUL;
	}

	tRetStatus = FltRegisterFilter(pDriverObject, &g_FltRegistration, &g_FltFilterHandle);
	if(false == NT_SUCCESS(tRetStatus))
	{
		Debug("FltRegisterFilter failed");
		return STATUS_UNSUCCESSFUL;
	}

	tRetStatus = FltStartFiltering(g_FltFilterHandle);
	if(false == NT_SUCCESS(tRetStatus))
	{
		Debug("FltStartFiltering failed");
		FltUnregisterFilter(g_FltFilterHandle);
		return STATUS_UNSUCCESSFUL;
	}

	return STATUS_SUCCESS;
}
//---------------------------------------------------------------

