#ifdef __cplusplus
extern "C"
{
#endif
#include <NTDDK.h>
#ifdef __cplusplus
}
#endif 

#include "../ZShareFiles/ZShareDef.h"

#define Debug(_X_)		{KdPrint(("SYS_Example,"));KdPrint((_X_)); KdPrint(("\n"));}

typedef struct _MyDeviceExtend
{
	LARGE_INTEGER	TimeCount;
	KTIMER			Timer;
	KDPC			DPC;
	PIRP			CurIrp;
}MyDeviceExtend;

NTSTATUS DriverCreate(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp) 
{
	Debug("create");
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest( pIrp, IO_NO_INCREMENT );

	return STATUS_SUCCESS;
}

NTSTATUS DriverClose(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp) 
{
	Debug("Close");
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest( pIrp, IO_NO_INCREMENT );

	return STATUS_SUCCESS;
}

NTSTATUS DriverWrite(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp) 
{
	Debug("Write");
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest( pIrp, IO_NO_INCREMENT );
	
	return STATUS_SUCCESS;
}


NTSTATUS DriverRead(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp) 
{
	Debug("Enter DriverRead");
	
	IoMarkIrpPending(pIrp);

	MyDeviceExtend* tDevExtend = (MyDeviceExtend*)pDevObj->DeviceExtension;
	ULONG tTime = 3000000;
	tDevExtend->TimeCount = RtlConvertLongToLargeInteger(-10*tTime);
	tDevExtend->CurIrp	= pIrp;

	NTSTATUS tRetStatus;
	tRetStatus = KeSetTimer(&tDevExtend->Timer,tDevExtend->TimeCount,&tDevExtend->DPC);

	Debug("Leave DriverRead");
	return STATUS_PENDING;
}

VOID DriverUnload(IN PDRIVER_OBJECT DriverObject)
{
	PDEVICE_OBJECT tDevObj		= DriverObject->DeviceObject;
	
	while(NULL != tDevObj)
	{
		PDEVICE_OBJECT tNextDevObj	= tDevObj->NextDevice;
		UNICODE_STRING tSymbolicName;
		
		MyDeviceExtend* tDevExtend	= (MyDeviceExtend*)tDevObj->DeviceExtension;	
		KeCancelTimer(&tDevExtend->Timer);

		RtlInitUnicodeString(&tSymbolicName,DEVICE_SYMBOLICLINK);
		IoDeleteSymbolicLink(&tSymbolicName);
		IoDeleteDevice(tDevObj);
		tDevObj = tNextDevObj;
	}
}

void OnTimeDPC(IN PKDPC Dpc,IN PVOID  DeferredContext,IN PVOID  SystemArgument1,IN PVOID  SystemArgument2)
{
	Debug("Enter OnTimeDPC");
	PDEVICE_OBJECT tDevice = (PDEVICE_OBJECT)DeferredContext;
	MyDeviceExtend* tDevExtend = (MyDeviceExtend*)tDevice->DeviceExtension;
	PIRP tCurIrp = tDevExtend->CurIrp;

	tCurIrp->IoStatus.Information = 0;
	tCurIrp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(tCurIrp,IO_NO_INCREMENT);

	//KeSetTimer(&tDevExtend->Timer,tDevExtend->TimeCount,&tDevExtend->DPC);
	Debug("Leave OnTimeDPC");
}

extern "C" NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject, IN PUNICODE_STRING pRegistryPath) 
{
	Debug(("Enter DriverEntry\n"));

	pDriverObject->DriverUnload = DriverUnload;
	pDriverObject->MajorFunction[IRP_MJ_CREATE]	= DriverCreate;
	pDriverObject->MajorFunction[IRP_MJ_CLOSE]	= DriverClose;
	pDriverObject->MajorFunction[IRP_MJ_WRITE]	= DriverWrite;
	pDriverObject->MajorFunction[IRP_MJ_READ]	= DriverRead;

	//创建设备
	PDEVICE_OBJECT tDev;
	UNICODE_STRING tDevName;
	RtlInitUnicodeString(&tDevName,DEVICE_NAME);

	NTSTATUS tRetStatus;
	tRetStatus = IoCreateDevice(pDriverObject,
								sizeof(MyDeviceExtend),
								&tDevName,
								FILE_DEVICE_UNKNOWN,
								0,
								FALSE,
								&tDev);

	if(false == NT_SUCCESS(tRetStatus))
	{
		Debug("IoCreateDevice fail");
		return tRetStatus;
	}

	//创建符号链接
	UNICODE_STRING tSymbolicName;
	RtlInitUnicodeString(&tSymbolicName,DEVICE_SYMBOLICLINK);
	tRetStatus = IoCreateSymbolicLink(&tSymbolicName,&tDevName);
	if(false == NT_SUCCESS(tRetStatus))
	{
		Debug("IoCreateSymbolicLink fail");
		IoDeleteDevice(tDev);
		return tRetStatus;
	}

	tDev->Flags |= DO_BUFFERED_IO;

	MyDeviceExtend* tMyDevExtend	= (MyDeviceExtend*)tDev->DeviceExtension;

	KeInitializeTimer(&tMyDevExtend->Timer);
	KeInitializeDpc(&tMyDevExtend->DPC,OnTimeDPC,(void*)tDev);
	return STATUS_SUCCESS;
}
//#define LOCKEDCODE code_seg()