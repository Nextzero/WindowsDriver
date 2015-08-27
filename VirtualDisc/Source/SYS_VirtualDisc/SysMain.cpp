//---------------------------------------------------------------------------
#include "SysMain.h"
#include "MC_VirtualDisc.h"
//---------------------------------------------------------------------------
void DriverUnload(IN PDRIVER_OBJECT	vDriverObject)
{
	MyVirtualDisc.UnInit(vDriverObject);
}
//---------------------------------------------------------------------------
NTSTATUS DriverCreate(IN PDEVICE_OBJECT vDeviceObject, IN PIRP vIrp)
{
	vIrp->IoStatus.Status		= STATUS_SUCCESS;
	vIrp->IoStatus.Information	= 0;
	IoCompleteRequest(vIrp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}
//---------------------------------------------------------------------------
NTSTATUS DriverClose(IN PDEVICE_OBJECT vDeviceObject, IN PIRP vIrp)
{
	vIrp->IoStatus.Status		= STATUS_SUCCESS;
	vIrp->IoStatus.Information	= 0;
	IoCompleteRequest(vIrp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}
//---------------------------------------------------------------------------
NTSTATUS DriverControl(IN PDEVICE_OBJECT vDeviceObject, IN PIRP vIrp)
{
	return MyVirtualDisc.DriverControl(vDeviceObject,vIrp);
}
//---------------------------------------------------------------------------
NTSTATUS DriverReadWrite(IN PDEVICE_OBJECT vDeviceObject, IN PIRP vIrp)
{
	return MyVirtualDisc.DriverReadWrite(vDeviceObject, vIrp);
}
//---------------------------------------------------------------------------
extern "C" NTSTATUS DriverEntry(IN PDRIVER_OBJECT vDriverObject, IN PUNICODE_STRING vRegistryPath)
{
	Debug("Enter DriverEntry");

	vDriverObject->DriverUnload							= DriverUnload;
	vDriverObject->MajorFunction[IRP_MJ_CREATE]			= DriverCreate;
	vDriverObject->MajorFunction[IRP_MJ_CLOSE]			= DriverClose;
	vDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]	= DriverControl;
	vDriverObject->MajorFunction[IRP_MJ_READ]			= DriverReadWrite;
	vDriverObject->MajorFunction[IRP_MJ_WRITE]			= DriverReadWrite;

	//创建设备对象
	if(false == MyVirtualDisc.Init(vDriverObject))
	{
		Debug("MC_VirtualDisc::Init fail");
		Debug("Leave DriverEntry");
		return STATUS_UNSUCCESSFUL;
	}

	Debug("Leave DriverEntry");
	return STATUS_SUCCESS;
}
//---------------------------------------------------------------------------