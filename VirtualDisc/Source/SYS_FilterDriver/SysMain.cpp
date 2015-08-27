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
NTSTATUS DriverRead(IN PDEVICE_OBJECT vDeviceObject, IN PIRP vIrp)
{
	Debug("Enter DriverRead");
	IoSkipCurrentIrpStackLocation(vIrp);
	MyDeviceExtend* tDeviceExtend = (MyDeviceExtend*)vDeviceObject->DeviceExtension;
	NTSTATUS tRetStauts = IoCallDriver(tDeviceExtend->LowDevice,vIrp);
	Debug("Leave DriverRead");
	return tRetStauts;
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
	RtlInitUnicodeString(&tLowDeviceName,DEVICE_NAME);

	PDEVICE_OBJECT tLowDevice;
	PFILE_OBJECT tLowFileObj;
	tRetStatus = IoGetDeviceObjectPointer(&tLowDeviceName,GENERIC_WRITE|GENERIC_READ,&tLowFileObj,&tLowDevice);
	if(false == NT_SUCCESS(tRetStatus))
		return tRetStatus;

	//创建过滤设备对象
	UNICODE_STRING tFilterDeviceName;
	RtlInitUnicodeString(&tFilterDeviceName,DEVICE_NAME_FILTER);

	PDEVICE_OBJECT tFilterDevice;
	tRetStatus = IoCreateDevice(vDriverObject,sizeof(MyDeviceExtend),&tFilterDeviceName,FILE_DEVICE_UNKNOWN,0,false,&tFilterDevice);
	if(false == NT_SUCCESS(tRetStatus))
	{
		ObDereferenceObject(tLowFileObj);
		return tRetStatus;
	}

	UNICODE_STRING tFilterSymbolic;
	RtlInitUnicodeString(&tFilterSymbolic,DEVICE_SYMBOLICLINK_FILTER);

	tRetStatus = IoCreateSymbolicLink(&tFilterSymbolic,&tFilterDeviceName);
	if(false == NT_SUCCESS(tRetStatus))
	{
		ObDereferenceObject(tLowFileObj);
		IoDeleteDevice(tLowDevice);
		return tRetStatus;
	}

	//附加驱动
	MyDeviceExtend* tDeviceExtend = (MyDeviceExtend*)tFilterDevice->DeviceExtension;
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

	ObDereferenceObject(tLowFileObj);
	Debug("Leave DriverEntry");
	return STATUS_SUCCESS;
}
//---------------------------------------------------------------------------