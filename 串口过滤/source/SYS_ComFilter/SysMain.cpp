#include "SysMain.h"

VOID DriverUnload(IN PDRIVER_OBJECT pDriverObject)
{
	PDEVICE_OBJECT tDeviceObj = pDriverObject->DeviceObject;
	PMYDEVICE_EXTENSION tDevExtend = (PMYDEVICE_EXTENSION)tDeviceObj->DeviceExtension;
	UNICODE_STRING tMySymbol;
	RtlInitUnicodeString(&tMySymbol,L"\\??\\SYS_ComFilterSymbol");

	if(0 != tDevExtend->LowDeviceObj)
	{
		IoDetachDevice(tDevExtend->LowDeviceObj);
	}

	LARGE_INTEGER tDelay;
	tDelay.QuadPart = 5000*10000*(-1);
	KeDelayExecutionThread(KernelMode,FALSE,&tDelay);

	
	IoDeleteSymbolicLink(&tMySymbol);
	IoDeleteDevice(tDeviceObj);
}

NTSTATUS DriverCommonDispatch(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp)
{
	Debug("Entry DriverCommonDispatch");
	PMYDEVICE_EXTENSION tDevExtend = (PMYDEVICE_EXTENSION)pDevObj->DeviceExtension;
	IoSkipCurrentIrpStackLocation(pIrp);
	NTSTATUS tStatus = IoCallDriver(tDevExtend->LowDeviceObj, pIrp);	
	Debug("Leave DriverCommonDipatch");
	return tStatus;
}

NTSTATUS DriverPower(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp)
{
	Debug("Entry DriverPower");
	PoStartNextPowerIrp(pIrp);
	IoSkipCurrentIrpStackLocation(pIrp);
	PMYDEVICE_EXTENSION tDevExtend = (PMYDEVICE_EXTENSION)pDevObj->DeviceExtension;
	NTSTATUS tStatus = PoCallDriver(tDevExtend->LowDeviceObj, pIrp);
	Debug("Leave DriverPower");
	return tStatus;
}

NTSTATUS DriverWrite(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp)
{
	Debug("Enter DriverWrite");
	PIO_STACK_LOCATION tStackLocal = IoGetCurrentIrpStackLocation(pIrp);
	PMYDEVICE_EXTENSION tDevExtend = (PMYDEVICE_EXTENSION)pDevObj->DeviceExtension;
	PUCHAR tBuf = 0;
	ULONG tLen = tStackLocal->Parameters.Write.Length;
	if(0 != pIrp->MdlAddress)
	{
		tBuf = (PUCHAR)MmGetMdlVirtualAddress(pIrp->MdlAddress);
	}
	else if(0 != pIrp->UserBuffer)
	{
		tBuf = (PUCHAR)pIrp->UserBuffer;
	}
	else
	{
		tBuf = (PUCHAR)pIrp->AssociatedIrp.SystemBuffer;
	}

	for(int i=0; i<tLen; i++)
	{
		KdPrint(("%c,",tBuf[i]));
	}
	IoSkipCurrentIrpStackLocation(pIrp);
	NTSTATUS tStatus = IoCallDriver(tDevExtend->LowDeviceObj,pIrp);
	Debug("Leave DriverWrite");
	return tStatus;
}

extern "C" NTSTATUS DriverEntry (IN PDRIVER_OBJECT pDriverObject,IN PUNICODE_STRING pRegistryPath) 
{
	for(int i=0; i<IRP_MJ_MAXIMUM_FUNCTION; i++)
	{
		pDriverObject->MajorFunction[i] = DriverCommonDispatch;
	}
	pDriverObject->MajorFunction[IRP_MJ_POWER] = DriverPower;
	pDriverObject->MajorFunction[IRP_MJ_WRITE] = DriverWrite;

	pDriverObject->DriverUnload = DriverUnload;

	PDEVICE_OBJECT	tLowDeviceObj	= 0;
	PFILE_OBJECT	tFileObj		= 0;
	UNICODE_STRING tLowDeviceName;
	RtlInitUnicodeString(&tLowDeviceName,L"\\Device\\serial0");
	NTSTATUS tStatus = IoGetDeviceObjectPointer(&tLowDeviceName,FILE_ALL_ACCESS,&tFileObj,&tLowDeviceObj);
	if(false == NT_SUCCESS(tStatus))
	{
		return tStatus;
	}
	else
	{
		ObDereferenceObject(tFileObj);
	}

	UNICODE_STRING tMyDeviceName;
	RtlInitUnicodeString(&tMyDeviceName,L"\\Device\\SYS_ComFilter");
	PDEVICE_OBJECT tMyDeviceObj = 0;
	tStatus = IoCreateDevice(pDriverObject,sizeof(MYDEVICE_EXTENSION),&tMyDeviceName,tLowDeviceObj->DeviceType,0,FALSE,&tMyDeviceObj);
	if(false == NT_SUCCESS(tStatus))
	{
		return tStatus;
	}
	

	UNICODE_STRING tMySymbol;
	RtlInitUnicodeString(&tMySymbol,L"\\??\\SYS_ComFilterSymbol");
	tStatus = IoCreateSymbolicLink(&tMySymbol,&tMyDeviceName);
	if(false == NT_SUCCESS(tStatus))
	{
		IoDeleteDevice(tMyDeviceObj);
		return tStatus;
	}
	
	//
	if(tLowDeviceObj->Flags & DO_DIRECT_IO)
		tMyDeviceObj->Flags |= DO_DIRECT_IO;
	if(tLowDeviceObj->Flags & DO_BUFFERED_IO)
		tMyDeviceObj->Flags |= DO_BUFFERED_IO;
	if(tLowDeviceObj->Characteristics & FILE_DEVICE_SECURE_OPEN)
		tMyDeviceObj->Characteristics |= FILE_DEVICE_SECURE_OPEN;
	tMyDeviceObj->Flags |= DO_POWER_PAGABLE;

	PMYDEVICE_EXTENSION tDevExtend = (PMYDEVICE_EXTENSION)tMyDeviceObj->DeviceExtension;
	tDevExtend->LowDeviceObj = IoAttachDeviceToDeviceStack(tMyDeviceObj,tLowDeviceObj);
	if(0 == tDevExtend->LowDeviceObj)
	{
		IoDeleteSymbolicLink(&tMySymbol);
		IoDeleteDevice(tMyDeviceObj);
		return STATUS_UNSUCCESSFUL;
	}
	tMyDeviceObj->Flags = tMyDeviceObj->Flags & ~DO_DEVICE_INITIALIZING;
	return STATUS_SUCCESS;
}

