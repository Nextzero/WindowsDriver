//---------------------------------------------------------------------------
#include "SysMain.h"
//---------------------------------------------------------------------------
void DriverUnload(IN PDRIVER_OBJECT	vDriverObject)
{
	PDEVICE_OBJECT tDevObj		= vDriverObject->DeviceObject;

	while(0 != tDevObj)
	{
		UNICODE_STRING tSymbolic;
		RtlInitUnicodeString(&tSymbolic,DEVICE_SYMBOLICLINK_BASE);
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
	vIrp->IoStatus.Information	= 0;
	vIrp->IoStatus.Status		= STATUS_SUCCESS;
	IoCompleteRequest(vIrp,IO_NO_INCREMENT);
	Debug("Leave DriverCreate");
	return STATUS_SUCCESS;
}
//---------------------------------------------------------------------------
NTSTATUS DriverClose(IN PDEVICE_OBJECT vDeviceObject, IN PIRP vIrp)
{
	Debug("Enter DriverClose");
	vIrp->IoStatus.Information	= 0;
	vIrp->IoStatus.Status		= STATUS_SUCCESS;
	IoCompleteRequest(vIrp,IO_NO_INCREMENT);
	Debug("Leave DriverClose");
	return STATUS_SUCCESS;
}
//---------------------------------------------------------------------------
NTSTATUS DriverControl(IN PDEVICE_OBJECT vDeviceObject, IN PIRP vIrp)
{
	Debug("Enter DriverControl");
	vIrp->IoStatus.Information	= 0;
	vIrp->IoStatus.Status		= STATUS_SUCCESS;
	IoCompleteRequest(vIrp,IO_NO_INCREMENT);
	Debug("Leave DriverControl");
	return STATUS_SUCCESS;
}
//---------------------------------------------------------------------------
NTSTATUS DriverWrite(IN PDEVICE_OBJECT vDeviceObject, IN PIRP vIrp)
{
	Debug("Enter DriverWrite");
	vIrp->IoStatus.Information	= 0;
	vIrp->IoStatus.Status		= STATUS_SUCCESS;
	IoCompleteRequest(vIrp,IO_NO_INCREMENT);
	Debug("Leave DriverWrite");
	return STATUS_SUCCESS;
}
//---------------------------------------------------------------------------
NTSTATUS DriverRead(IN PDEVICE_OBJECT vDeviceObject, IN PIRP vIrp)
{
	Debug("Enter DriverRead");
	if(0 == vIrp->MdlAddress)
	{
		vIrp->IoStatus.Information	= 0;
		vIrp->IoStatus.Status		= STATUS_UNSUCCESSFUL;
		IoCompleteRequest(vIrp,IO_NO_INCREMENT);
		return STATUS_UNSUCCESSFUL;
	}

	PVOID tVirtualAddress = MmGetMdlVirtualAddress(vIrp->MdlAddress);
	ULONG tLength = MmGetMdlByteCount(vIrp->MdlAddress);
	RtlFillMemory(tVirtualAddress,tLength,'A');

	vIrp->IoStatus.Information	= tLength;
	vIrp->IoStatus.Status		= STATUS_SUCCESS;
	IoCompleteRequest(vIrp,IO_NO_INCREMENT);
	Debug("Leave DriverRead");
	return STATUS_SUCCESS;
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

	NTSTATUS tRetStatus = STATUS_UNSUCCESSFUL;

	//创建设备对象
	UNICODE_STRING tBaseDeviceName;
	RtlInitUnicodeString(&tBaseDeviceName,DEVICE_NAME_BASE);

	PDEVICE_OBJECT tBaseDevice;
	tRetStatus = IoCreateDevice(vDriverObject,sizeof(MyDeviceExtend),&tBaseDeviceName,FILE_DEVICE_UNKNOWN,0,false,&tBaseDevice);
	if(false == NT_SUCCESS(tRetStatus))
	{
		Debug("IoCreateDevice fail");
		return tRetStatus;
	}

	UNICODE_STRING tBaseSymbolic;
	RtlInitUnicodeString(&tBaseSymbolic,DEVICE_SYMBOLICLINK_BASE);

	tRetStatus = IoCreateSymbolicLink(&tBaseSymbolic,&tBaseDeviceName);
	if(false == NT_SUCCESS(tRetStatus))
	{
		Debug("IoCreateSymbolicLink fail");
		IoDeleteDevice(tBaseDevice);
		return tRetStatus;
	}
	tBaseDevice->Flags |= DO_DIRECT_IO;
	Debug("Leave DriverEntry");
	return STATUS_SUCCESS;
}
//---------------------------------------------------------------------------