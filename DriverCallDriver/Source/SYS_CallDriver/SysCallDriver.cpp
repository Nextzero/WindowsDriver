#ifdef __cplusplus
extern "C"
{
#endif
#include <NTDDK.h>
#ifdef __cplusplus
}
#endif 

#include "../ZShareFiles/ZShareDef.h"

#define Debug(_X_)		{KdPrint(("SYS_CallDriver,"));KdPrint((_X_)); KdPrint(("\n"));}

typedef struct _MyDeviceExtend
{
	int i;
}MyDeviceExtend;

/*
使用Zw系列函数做驱动同步、异步调用驱动
同步：在ZwCreateFile时加上标志位
异步：去掉ZwCreateFile上的相关标志位。1.通过设置完成回调函数；2.通过FILE_OBJECT的event域作为同步点

FileObject由谁创建的? FileObject.event由谁创建?
*/

//Zw函数同步读
NTSTATUS ZwReadSynchronism(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp)
{
	Debug("Enter ZwReadSynchronism");
	UNICODE_STRING tDeviceName;
	RtlInitUnicodeString(&tDeviceName,DEVICE_NAME);

	OBJECT_ATTRIBUTES tDevObjAtt;
	InitializeObjectAttributes(	&tDevObjAtt,
								&tDeviceName,
								OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE,
								0,
								0);
	NTSTATUS		tRetStatus;
	HANDLE			tDeviceHandle;
	IO_STATUS_BLOCK	tIoStatusBlock;
	tRetStatus = ZwCreateFile(	&tDeviceHandle,
								GENERIC_READ|GENERIC_WRITE|SYNCHRONIZE,
								&tDevObjAtt,
								&tIoStatusBlock,
								0,
								FILE_ATTRIBUTE_NORMAL,
								0,
								FILE_OPEN,
								FILE_NO_INTERMEDIATE_BUFFERING|FILE_SYNCHRONOUS_IO_NONALERT|FILE_NON_DIRECTORY_FILE|FILE_RANDOM_ACCESS,
								0,
								0);
	if(false == NT_SUCCESS(tRetStatus))
	{
		Debug("ZwCreateFile fail");
		return tRetStatus;
	}

	char tBuf[10] = {0};
	tRetStatus = ZwReadFile(tDeviceHandle,0,0,0,&tIoStatusBlock,tBuf,sizeof(tBuf),0,0);//hc:得证
	ZwClose(tDeviceHandle);

	pIrp->IoStatus.Information	= 0;
	pIrp->IoStatus.Status		= STATUS_SUCCESS;
	IoCompleteRequest(pIrp,IO_NO_INCREMENT);
	Debug("Leave ZwReadSynchronism");
	return STATUS_SUCCESS;
}



//Zw函数异步读，设置回调
void CompleteReadCallback(PVOID vContext,PIO_STATUS_BLOCK vIoStatusBlock,ULONG t)
{
	Debug("CompleteReadCallback");
	KeSetEvent((PKEVENT)vContext,IO_NO_INCREMENT,FALSE);
}

NTSTATUS ZwReadAsynchronous(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp)
{
	Debug("Enter ZwReadAsynchronous");
	UNICODE_STRING tDeviceName;
	RtlInitUnicodeString(&tDeviceName,DEVICE_NAME);

	OBJECT_ATTRIBUTES tDevObjAtt;
	InitializeObjectAttributes(	&tDevObjAtt,
		&tDeviceName,
		OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE,
		0,
		0);
	NTSTATUS		tRetStatus;
	HANDLE			tDeviceHandle;
	IO_STATUS_BLOCK	tIoStatusBlock;
	//在ZwCreateFile只有标志位的不同
	tRetStatus = ZwCreateFile(	&tDeviceHandle,
		GENERIC_READ|GENERIC_WRITE,
		&tDevObjAtt,
		&tIoStatusBlock,
		0,
		FILE_ATTRIBUTE_NORMAL,
		0,
		FILE_OPEN,
		0,
		0,
		0);
	if(false == NT_SUCCESS(tRetStatus))
	{
		Debug("ZwCreateFile fail");
		return tRetStatus;
	}
	
	KEVENT tEvent;
	KeInitializeEvent(&tEvent,SynchronizationEvent,FALSE);
	//设置完成回调函数
	LARGE_INTEGER tOffset = RtlConvertLongToLargeInteger(0);
	tRetStatus = ZwReadFile(tDeviceHandle,0,CompleteReadCallback,&tEvent,&tIoStatusBlock,0,0,&tOffset,0);//tOffset必须要
	Debug("ZwReadFile return ");
	if(STATUS_PENDING == tRetStatus)
	{
		Debug("ZwReadFile return STATUS_PENDING");
		KeWaitForSingleObject(&tEvent,Executive,KernelMode,FALSE,0);
		Debug("has wait success ");
	}
	else if( STATUS_SUCCESS == tRetStatus)
	{
		Debug("zwreadfile return status_success");
	}
	else
	{
		Debug("zereadfile return unknow");
		KdPrint(("%d",tRetStatus));
	}
		
	ZwClose(tDeviceHandle);

	pIrp->IoStatus.Information	= 0;
	pIrp->IoStatus.Status		= STATUS_SUCCESS;
	IoCompleteRequest(pIrp,IO_NO_INCREMENT);
	Debug("Leave ZwReadAsynchronous");
	return STATUS_SUCCESS;
}


NTSTATUS ZwReadAsynchronous_FileObject(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp)
{
	Debug("Enter ZwReadAsynchronous_FileObject");
	UNICODE_STRING tDeviceName;
	RtlInitUnicodeString(&tDeviceName,DEVICE_NAME);

	OBJECT_ATTRIBUTES tDevObjAtt;
	InitializeObjectAttributes(	&tDevObjAtt,
		&tDeviceName,
		OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE,
		0,
		0);
	NTSTATUS		tRetStatus;
	HANDLE			tDeviceHandle;
	IO_STATUS_BLOCK	tIoStatusBlock;
	//在ZwCreateFile只有标志位的不同
	tRetStatus = ZwCreateFile(	&tDeviceHandle,
		GENERIC_READ|GENERIC_WRITE,
		&tDevObjAtt,
		&tIoStatusBlock,
		0,
		FILE_ATTRIBUTE_NORMAL,
		0,
		FILE_OPEN,
		0,
		0,
		0);
	if(false == NT_SUCCESS(tRetStatus))
	{
		Debug("ZwCreateFile fail");
		return tRetStatus;
	}

	KEVENT tEvent;
	KeInitializeEvent(&tEvent,SynchronizationEvent,FALSE);
	//设置完成回调函数
	LARGE_INTEGER tOffset = RtlConvertLongToLargeInteger(0);
	tRetStatus = ZwReadFile(tDeviceHandle,0,0,0,&tIoStatusBlock,0,0,&tOffset,0);//tOffset必须要
	Debug("ZwReadFile return ");
	if(STATUS_PENDING == tRetStatus)
	{
		Debug("ZwReadFile return STATUS_PENDING");
		PFILE_OBJECT tFileObj;
		//书中第三个参数有误
		NTSTATUS tTemRet = ObReferenceObjectByHandle(tDeviceHandle,EVENT_MODIFY_STATE,*IoFileObjectType,KernelMode,(PVOID*)&tFileObj,0);
		if(NT_SUCCESS(tTemRet))
		{
			KeWaitForSingleObject(&tFileObj->Event,Executive,KernelMode,FALSE,0);
			Debug("has wait success ");
			ObDereferenceObject(tFileObj);
		}
		else
		{
			Debug("Byhandle fail");
			KdPrint(("%d",tTemRet));
		}
	}
	else if( STATUS_SUCCESS == tRetStatus)
	{
		Debug("zwreadfile return status_success");
	}
	else
	{
		Debug("zereadfile return unknow");
		KdPrint(("%d",tRetStatus));
	}

	ZwClose(tDeviceHandle);

	pIrp->IoStatus.Information	= 0;
	pIrp->IoStatus.Status		= STATUS_SUCCESS;
	IoCompleteRequest(pIrp,IO_NO_INCREMENT);
	Debug("Leave ZwReadAsynchronous_FileObject");
	return STATUS_SUCCESS;
}


//通过ObjectPointer调用驱动
NTSTATUS ObjPtrReadSynchronism(IN PDEVICE_OBJECT pDevObj,IN PIRP pIrp)
{
	Debug("Enter ObjPtrReadSynchronism");
	UNICODE_STRING tDeviceName;
	RtlInitUnicodeString(&tDeviceName,DEVICE_NAME);
	PFILE_OBJECT tFileObj;
	PDEVICE_OBJECT tDevObj;
	NTSTATUS tRetStatus;
	tRetStatus = IoGetDeviceObjectPointer(&tDeviceName,GENERIC_READ|GENERIC_WRITE,&tFileObj,&tDevObj);
	if(false == NT_SUCCESS(tRetStatus))
	{
		Debug("IoGetDeviceObjectPointer fail");
		pIrp->IoStatus.Information = 0;
		pIrp->IoStatus.Status = STATUS_SUCCESS;
		return STATUS_SUCCESS;
	}

	IoAllocateIrp(pDevObj->StackSize)
	KEVENT tEvent;
	KeInitializeEvent(&tEvent,SynchronizationEvent,FALSE);
	IO_STATUS_BLOCK tIoStatusBlock;
	LARGE_INTEGER tOffset = RtlConvertLongToLargeInteger(0);

	PIRP tNewIrp = IoBuildSynchronousFsdRequest(IRP_MJ_READ,pDevObj,0,0,&tOffset,&tEvent,&tIoStatusBlock);
	if(0 == tNewIrp)
		Debug("build irp fail");

	PIO_STACK_LOCATION tStackLoc = IoGetNextIrpStackLocation(tNewIrp);
	//tStackLoc->FileObject = tFileObj;//这是为什么

	tRetStatus = IoCallDriver(tDevObj,tNewIrp);
	Debug("IoCallDriver return ");
	if(STATUS_PENDING == tRetStatus)
	{
		Debug("IoCallDriver return status_pending");
		KeWaitForSingleObject(&tEvent,Executive,KernelMode,FALSE,0);
		Debug("wait success");
	}
	ObDereferenceObject(tFileObj);

	pIrp->IoStatus.Information = 0;
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(pIrp,IO_NO_INCREMENT);
	Debug("Leave ObjPtrReadSynchronism");
	return STATUS_SUCCESS;
}

NTSTATUS DispatchRoutine(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp) 
{
	Debug("Enter DispatchRoutine");
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest( pIrp, IO_NO_INCREMENT );
	Debug("Leave DispatchRoutine");
	return STATUS_SUCCESS;
}


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
	return ObjPtrReadSynchronism(pDevObj,pIrp);
}

VOID DriverUnload(IN PDRIVER_OBJECT DriverObject)
{
	PDEVICE_OBJECT tDevObj		= DriverObject->DeviceObject;
	
	while(NULL != tDevObj)
	{
		PDEVICE_OBJECT tNextDevObj	= tDevObj->NextDevice;
		UNICODE_STRING tSymbolicName;
		
		MyDeviceExtend* tDevExtend	= (MyDeviceExtend*)tDevObj->DeviceExtension;	
	
		RtlInitUnicodeString(&tSymbolicName,DEVICE_SYMBOLICLINK__CALLDRIVER);
		IoDeleteSymbolicLink(&tSymbolicName);
		IoDeleteDevice(tDevObj);
		tDevObj = tNextDevObj;
	}
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
	RtlInitUnicodeString(&tDevName,DEVICE_NAME_CALLDRIVER);

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
	RtlInitUnicodeString(&tSymbolicName,DEVICE_SYMBOLICLINK__CALLDRIVER);
	tRetStatus = IoCreateSymbolicLink(&tSymbolicName,&tDevName);
	if(false == NT_SUCCESS(tRetStatus))
	{
		Debug("IoCreateSymbolicLink fail");
		IoDeleteDevice(tDev);
		return tRetStatus;
	}

	tDev->Flags |= DO_BUFFERED_IO;

	MyDeviceExtend* tMyDevExtend	= (MyDeviceExtend*)tDev->DeviceExtension;

	return STATUS_SUCCESS;
}
//#define LOCKEDCODE code_seg()