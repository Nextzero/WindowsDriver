//---------------------------------------------------------------------------
#include "SysMain.h"
//---------------------------------------------------------------------------
void DriverUnload(IN PDRIVER_OBJECT	vDriverObject)
{
	PDEVICE_OBJECT tDevObj		= vDriverObject->DeviceObject;
	MyDeviceExtend* tDevExtent	= (MyDeviceExtend*)tDevObj->DeviceExtension;

	while(0 != tDevObj)
	{
		UNICODE_STRING tSymbolic;
		RtlInitUnicodeString(&tSymbolic,DEVICE_SYMBOLICLINK_FILTER);

		//从设备栈中弹出
		IoDetachDevice(tDevExtent->LowDevice);

		PDEVICE_OBJECT tNextObj = tDevObj->NextDevice;
		IoDeleteSymbolicLink(&tSymbolic);
		IoDeleteDevice(tDevObj);
		tDevObj = tNextObj;
	}
	return ;
}
//---------------------------------------------------------------------------
NTSTATUS DriverCreate(IN PDEVICE_OBJECT vDeviceObject, IN PIRP vIrp)
{
	Debug("Enter DriverCreate");
	IoSkipCurrentIrpStackLocation(vIrp);
	MyDeviceExtend* tDeviceExtend = (MyDeviceExtend*)vDeviceObject->DeviceExtension;
	NTSTATUS tRetStauts = IoCallDriver(tDeviceExtend->LowDevice,vIrp);
	Debug("Leave DriverCreate");
	return tRetStauts;
}
//---------------------------------------------------------------------------
NTSTATUS DriverClose(IN PDEVICE_OBJECT vDeviceObject, IN PIRP vIrp)
{
	Debug("Enter DriverClose");
	IoSkipCurrentIrpStackLocation(vIrp);
	MyDeviceExtend* tDeviceExtend = (MyDeviceExtend*)vDeviceObject->DeviceExtension;
	NTSTATUS tRetStauts = IoCallDriver(tDeviceExtend->LowDevice,vIrp);
	Debug("Leave DriverClose");
	return tRetStauts;
}
//---------------------------------------------------------------------------
NTSTATUS DriverControl(IN PDEVICE_OBJECT vDeviceObject, IN PIRP vIrp)
{
	Debug("Enter DriverControl");
	IoSkipCurrentIrpStackLocation(vIrp);
	MyDeviceExtend* tDeviceExtend = (MyDeviceExtend*)vDeviceObject->DeviceExtension;
	NTSTATUS tRetStauts = IoCallDriver(tDeviceExtend->LowDevice,vIrp);
	Debug("Leave DriverControl");
	return tRetStauts;
}
//---------------------------------------------------------------------------
NTSTATUS DriverWrite(IN PDEVICE_OBJECT vDeviceObject, IN PIRP vIrp)
{
	Debug("Enter DriverWrite");
	IoSkipCurrentIrpStackLocation(vIrp);
	MyDeviceExtend* tDeviceExtend = (MyDeviceExtend*)vDeviceObject->DeviceExtension;
	NTSTATUS tRetStauts = IoCallDriver(tDeviceExtend->LowDevice,vIrp);
	Debug("Leave DriverWrite");
	return tRetStauts;
}
//---------------------------------------------------------------------------
NTSTATUS IoCompletion(IN PDEVICE_OBJECT  DeviceObject, IN PIRP  Irp, IN PVOID  Context)
{
	Debug("Enter IoCompletion");
	MyContext* tContext = (MyContext*)Context;
	MyDeviceExtend* tDeviceExtend = (MyDeviceExtend*)DeviceObject->DeviceExtension;
	NTSTATUS tRetStatus = Irp->IoStatus.Status;

	ULONG ThisLength = 0;
	if(tContext && NT_SUCCESS(tRetStatus))
	{
		tContext->AlreadyLength += Irp->IoStatus.Information;
		if(tContext->LeftLength) //还有没读完的
		{
			//设定下一次读取 
			if(tContext->LeftLength > tDeviceExtend->LowDeviceReadLimit)
			{
				ThisLength = tDeviceExtend->LowDeviceReadLimit;
			}
			else
			{
				ThisLength = tContext->LeftLength;
			}
			MmPrepareMdlForReuse(tContext->NewMdl);//重新利用MDL
			IoBuildPartialMdl(Irp->MdlAddress,tContext->NewMdl,tContext->LeftVirtualAddress,ThisLength);
			tContext->LeftVirtualAddress = (char*)tContext->LeftVirtualAddress + ThisLength;
			tContext->LeftLength -= ThisLength;
			IoCopyCurrentIrpStackLocationToNext(Irp);
			PIO_STACK_LOCATION NextStackLocation = IoGetNextIrpStackLocation(Irp);
			NextStackLocation->Parameters.Read.Length = ThisLength;
			IoSetCompletionRoutine(Irp,IoCompletion,tContext,TRUE,TRUE,TRUE);
			IoCallDriver(tDeviceExtend->LowDevice,Irp);
			
			return STATUS_MORE_PROCESSING_REQUIRED;
		}
	}
	else
	{
		Irp->IoStatus.Information = tContext->AlreadyLength;
	}
	Debug("Leave IoCompletion");
	//return STATUS_MORE_PROCESSING_REQUIRED;
	return STATUS_SUCCESS;
}
//---------------------------------------------------------------------------
NTSTATUS DriverRead(IN PDEVICE_OBJECT vDeviceObject, IN PIRP vIrp)
{
	Debug("Enter DriverRead");

	MyDeviceExtend* tDeviceExtend = (MyDeviceExtend*)vDeviceObject->DeviceExtension;
	if(0 == tDeviceExtend)
	{
		Debug("if(0 == tDeviceExtend)");
		vIrp->IoStatus.Information = 0;
		vIrp->IoStatus.Status = STATUS_UNSUCCESSFUL;
		IoCompleteRequest(vIrp,IO_NO_INCREMENT);
		return STATUS_UNSUCCESSFUL;
	}
	
	if(!vIrp->MdlAddress)
	{
		Debug("if(!vIrp->MdlAddress)");
		vIrp->IoStatus.Information = 0;
		vIrp->IoStatus.Status = STATUS_UNSUCCESSFUL;
		IoCompleteRequest(vIrp,IO_NO_INCREMENT);
		return STATUS_UNSUCCESSFUL;
	}

	PVOID VirtualAddress = MmGetMdlVirtualAddress(vIrp->MdlAddress);
	ULONG TotalLength = MmGetMdlByteCount(vIrp->MdlAddress);
	
	ULONG ThisLength = 0;//一次读写
	if(TotalLength > tDeviceExtend->LowDeviceReadLimit)
	{
		ThisLength = tDeviceExtend->LowDeviceReadLimit;
	}
	else
	{
		ThisLength = TotalLength;
	}

	PMDL mdl = IoAllocateMdl(VirtualAddress,TotalLength,FALSE,FALSE,0);
	if(0 == mdl)
	{
		Debug("if(0 == mdl)");
		vIrp->IoStatus.Information = 0;
		vIrp->IoStatus.Status = STATUS_UNSUCCESSFUL;
		IoCompleteRequest(vIrp,IO_NO_INCREMENT);
		return STATUS_UNSUCCESSFUL;
	}

	IoBuildPartialMdl(vIrp->MdlAddress,mdl,VirtualAddress,ThisLength);
	
	MyContext* tContext = (MyContext*)ExAllocatePool(NonPagedPool,sizeof(MyContext));
	if(0 == tContext)
	{
		Debug("if(0 == tContext)");
		IoFreeMdl(mdl);
		vIrp->IoStatus.Information = 0;
		vIrp->IoStatus.Status = STATUS_UNSUCCESSFUL;
		IoCompleteRequest(vIrp,IO_NO_INCREMENT);
		return STATUS_UNSUCCESSFUL;
	}

	tContext->NewMdl = mdl;
	tContext->PreMdl = vIrp->MdlAddress;
	tContext->LeftLength = TotalLength - ThisLength;
	tContext->AlreadyLength = 0;
	tContext->LeftVirtualAddress = (char*)VirtualAddress + ThisLength;

	IoCopyCurrentIrpStackLocationToNext(vIrp);
	PIO_STACK_LOCATION NextStackLocation = IoGetNextIrpStackLocation(vIrp);
	NextStackLocation->Parameters.Read.Length = ThisLength;

	vIrp->MdlAddress = mdl;

	IoSetCompletionRoutine(vIrp,IoCompletion,tContext,TRUE,TRUE,TRUE);

	NTSTATUS tRetStatus = IoCallDriver(tDeviceExtend->LowDevice,vIrp);
	vIrp->MdlAddress = tContext->PreMdl;
	IoFreeMdl(tContext->NewMdl);

	vIrp->IoStatus.Information = TotalLength;
	vIrp->IoStatus.Status = tRetStatus;
	IoCompleteRequest(vIrp,IO_NO_INCREMENT);//为什么还要IoCom..,vIrp);
	return tRetStatus;

/*
	//底层驱动为异步完成
	MyDeviceExtend* tDeviceExtend = (MyDeviceExtend*)vDeviceObject->DeviceExtension;
	IoCopyCurrentIrpStackLocationToNext(vIrp);

	KEVENT tEvent;
	KeInitializeEvent(&tEvent,SynchronizationEvent,FALSE);

	IoSetCompletionRoutine(vIrp,IoCompletion,&tEvent,TRUE,TRUE,TRUE);

	NTSTATUS tRetStatus = IoCallDriver(tDeviceExtend->LowDevice,vIrp);
	if(tRetStatus == STATUS_PENDING)
	{

		Debug("IoCallDriver return status pending ");
		KeWaitForSingleObject(&tEvent,Executive,KernelMode,FALSE,0);//hc:做个同步点
		tRetStatus = vIrp->IoStatus.Status;
	}
	
	IoCompleteRequest(vIrp,IO_NO_INCREMENT);
	Debug("Leave DriverRead");
	return tRetStatus;*/
}
//---------------------------------------------------------------------------
extern "C" NTSTATUS DriverEntry(IN PDRIVER_OBJECT vDriverObject, IN PUNICODE_STRING vRegistryPath)
{
	Debug("Enter DriverEntry");

	vDriverObject->DriverUnload							= DriverUnload;
	vDriverObject->MajorFunction[IRP_MJ_CREATE]			= DriverCreate;
	vDriverObject->MajorFunction[IRP_MJ_CLOSE]			= DriverClose;
	vDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]	= DriverControl;
	vDriverObject->MajorFunction[IRP_MJ_READ]			= DriverRead;
	vDriverObject->MajorFunction[IRP_MJ_WRITE]			= DriverWrite;

	NTSTATUS tRetStatus;

	//得到下层设备
	UNICODE_STRING tLowDeviceName;
	RtlInitUnicodeString(&tLowDeviceName,DEVICE_NAME_BASE);

	PDEVICE_OBJECT tLowDevice;
	PFILE_OBJECT tLowFileObj;
	tRetStatus = IoGetDeviceObjectPointer(&tLowDeviceName,GENERIC_WRITE|GENERIC_READ,&tLowFileObj,&tLowDevice);
	if(false == NT_SUCCESS(tRetStatus))
	{
		Debug("IoGetDeviceObjectPointer fail");
		return tRetStatus;
	}

	//创建过滤设备对象
	UNICODE_STRING tFilterDeviceName;
	RtlInitUnicodeString(&tFilterDeviceName,DEVICE_NAME_FILTER);

	PDEVICE_OBJECT tFilterDevice;
	tRetStatus = IoCreateDevice(vDriverObject,sizeof(MyDeviceExtend),&tFilterDeviceName,FILE_DEVICE_UNKNOWN,0,false,&tFilterDevice);
	if(false == NT_SUCCESS(tRetStatus))
	{
		Debug("IoCreateDevice");
		ObDereferenceObject(tLowFileObj);
		return tRetStatus;
	}
	tFilterDevice->Flags |= DO_DIRECT_IO;
	UNICODE_STRING tFilterSymbolic;
	RtlInitUnicodeString(&tFilterSymbolic,DEVICE_SYMBOLICLINK_FILTER);

	tRetStatus = IoCreateSymbolicLink(&tFilterSymbolic,&tFilterDeviceName);
	if(false == NT_SUCCESS(tRetStatus))
	{
		Debug("IoCreateSymbolicLink");
		ObDereferenceObject(tLowFileObj);
		IoDeleteDevice(tLowDevice);
		return tRetStatus;
	}

	//附加驱动
	MyDeviceExtend* tDeviceExtend	= (MyDeviceExtend*)tFilterDevice->DeviceExtension;
	PDEVICE_OBJECT tDirectLowDevice = IoAttachDeviceToDeviceStack(tFilterDevice,tLowDevice);
	if(0 == tDirectLowDevice)
	{
		Debug("Attach fail");
		ObDereferenceObject(tLowFileObj);
		IoDeleteSymbolicLink(&tFilterSymbolic);
		IoDeleteDevice(tFilterDevice);
		return STATUS_UNSUCCESSFUL;
	}
	tDeviceExtend->LowDevice = tDirectLowDevice;
	tDeviceExtend->LowDeviceReadLimit = 1;

	ObDereferenceObject(tLowFileObj);
	Debug("Leave DriverEntry");
	return STATUS_SUCCESS;
}
//---------------------------------------------------------------------------