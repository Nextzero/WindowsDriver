#include "SysMain.h"

ULONG ProcessNameOffset = 0;

VOID DriverUnload(IN PDRIVER_OBJECT pDriverObject)
{
	PDEVICE_OBJECT tDeviceObj = pDriverObject->DeviceObject;
	PMYDEVICE_EXTENSION tDevExtend = (PMYDEVICE_EXTENSION)tDeviceObj->DeviceExtension;
	UNICODE_STRING tMySymbol;
	RtlInitUnicodeString(&tMySymbol,L"\\??\\TcpFilter_Symbol");

	if(0 != tDevExtend->LowDeviceObj)
	{
		IoDetachDevice(tDevExtend->LowDeviceObj);
	}
	
	IoDeleteSymbolicLink(&tMySymbol);
	IoDeleteDevice(tDeviceObj);
}

NTSTATUS DriverCommonDispatch(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp)
{
	//Debug("Entry DriverCommonDispatch");

	//process name
	char tProcessName[512] = {0};
	PEPROCESS tCurProc	= PsGetCurrentProcess();
	char* tNamePtr		= (char*)tCurProc + ProcessNameOffset;
	memcpy(tProcessName, tNamePtr, 15);
	tProcessName[15] = 0;
	//KdPrint(("process name:%s\n",tProcessName));

	//PrintIrp(pIrp);
	PIO_STACK_LOCATION tIoStack = IoGetCurrentIrpStackLocation(pIrp);
	if(IRP_MJ_INTERNAL_DEVICE_CONTROL == tIoStack->MajorFunction 
		&& TDI_CONNECT == tIoStack->MinorFunction)
	{
		PrintIrp(pIrp);
		PTDI_REQUEST_KERNEL_CONNECT param = (PTDI_REQUEST_KERNEL_CONNECT )(&tIoStack->Parameters);
		TA_ADDRESS* remote_addr = ((TRANSPORT_ADDRESS*)(param->RequestConnectionInformation->RemoteAddress))->Address;
		USHORT	tPort = ((TDI_ADDRESS_IP *)(remote_addr))->sin_port;
		ULONG	tAddr = ((TDI_ADDRESS_IP *)(remote_addr))->in_addr;

		ULONG resultAddr = 0;
		((char *)&resultAddr)[0] = ((char *)&tAddr)[3];
		((char *)&resultAddr)[1] = ((char *)&tAddr)[2];
		((char *)&resultAddr)[2] = ((char *)&tAddr)[1];
		((char *)&resultAddr)[3] = ((char *)&tAddr)[0];

		USHORT resultPort = 0;
		((char *)&resultPort)[0] = ((char *)&tPort)[1];
		((char *)&resultPort)[1] = ((char *)&tPort)[0];

		KdPrint(("remote Addr:%d.%d.%d.%d, Port:%d\n",((char *)&resultAddr)[0], ((char *)&resultAddr)[1], ((char *)&resultAddr)[2], ((char *)&resultAddr)[3], resultPort));
	}


	PMYDEVICE_EXTENSION tDevExtend = (PMYDEVICE_EXTENSION)pDevObj->DeviceExtension;
	IoSkipCurrentIrpStackLocation(pIrp);
	NTSTATUS tStatus = IoCallDriver(tDevExtend->LowDeviceObj, pIrp);	
	//Debug("Leave DriverCommonDipatch");
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

NTSTATUS DriverCreate_QueryInfo_COMPLETION_ROUTINE(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN PVOID Context)
{
	Debug("Enter DriverCreate_QueryInfo_COMPLETION_ROUTINE");

	TDI_ADDRESS_INFO* tTdiAddrInfo = (TDI_ADDRESS_INFO*)Context;

	USHORT	tPort = ((TDI_ADDRESS_IP *)(tTdiAddrInfo->Address.Address->Address))->sin_port;
	ULONG	tAddr = ((TDI_ADDRESS_IP *)(tTdiAddrInfo->Address.Address->Address))->in_addr;

	ULONG resultAddr = 0;
	((char *)&resultAddr)[0] = ((char *)&tAddr)[3];
	((char *)&resultAddr)[1] = ((char *)&tAddr)[2];
	((char *)&resultAddr)[2] = ((char *)&tAddr)[1];
	((char *)&resultAddr)[3] = ((char *)&tAddr)[0];

	USHORT resultPort = 0;
	((char *)&resultPort)[0] = ((char *)&tPort)[1];
	((char *)&resultPort)[1] = ((char *)&tPort)[0];

	KdPrint(("Addr:%d, Port:%d\n",resultAddr, resultPort));

	if(Irp->MdlAddress != NULL)
	{
		//MmUnlockPages(Irp->MdlAddress);
		IoFreeMdl(Irp->MdlAddress);
		Irp->MdlAddress = NULL;
	}
	if(tTdiAddrInfo != NULL)
		ExFreePool(tTdiAddrInfo);

	Debug("Leave DriverCreate_QueryInfo_COMPLETION_ROUTINE");
	return STATUS_SUCCESS;
}

NTSTATUS DriverCreate_COMPLETION_ROUTINE(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN PVOID Context)
{
	Debug("Enter DriverCreate_COMPLETION_ROUTINE");
	NTSTATUS tRetStatus;
	PIRP tQueryIrp = (PIRP)Context;
	PIO_STACK_LOCATION tIoStack = IoGetCurrentIrpStackLocation(Irp);
	PMYDEVICE_EXTENSION tExtend = (PMYDEVICE_EXTENSION)DeviceObject->DeviceExtension;

	if(STATUS_SUCCESS != Irp->IoStatus.Status)
	{
		if (Irp->PendingReturned) 
		{
			KdPrint(("[tdi_fw] tdi_generic_complete: PENDING\n"));
			IoMarkIrpPending(Irp);
		}

		return STATUS_SUCCESS;
	}

	TDI_ADDRESS_INFO* tTdiAddrInfo = (TDI_ADDRESS_INFO*)ExAllocatePool(NonPagedPool, TDI_ADDRESS_INFO_MAX);
	PMDL mdl = IoAllocateMdl(tTdiAddrInfo, TDI_ADDRESS_INFO_MAX, FALSE, FALSE, 0);
	MmBuildMdlForNonPagedPool(mdl);

	TdiBuildQueryInformation(
						tQueryIrp, 
						tExtend->LowDeviceObj, 
						tIoStack->FileObject, 
						DriverCreate_QueryInfo_COMPLETION_ROUTINE, 
						tTdiAddrInfo,
						TDI_QUERY_ADDRESS_INFO, 
						mdl);


	tRetStatus = IoCallDriver(tExtend->LowDeviceObj, tQueryIrp);

	mdl				= 0;
	tTdiAddrInfo	= 0;
	tQueryIrp		= 0;

	// cleanup
	if (tQueryIrp != NULL)
	{
		IoCompleteRequest(tQueryIrp, IO_NO_INCREMENT);
	}

	if (mdl != NULL)
	{
		IoFreeMdl(mdl);
		mdl = 0;
	}

	if (tTdiAddrInfo != NULL)
	{
		ExFreePool(tTdiAddrInfo);
		tTdiAddrInfo = 0;
	}
	
	Irp->IoStatus.Status = tRetStatus;

	if (Irp->PendingReturned) 
	{
		KdPrint(("[tdi_fw] tdi_generic_complete: PENDING\n"));
		IoMarkIrpPending(Irp);
	}

	Debug("Leave DriverCreate_COMPLETION_ROUTINE");
	return STATUS_SUCCESS;
}

NTSTATUS DriverCreate(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp)
{
	Debug("Enter DriverCreate");

	//process name
	char tProcessName[512] = {0};
	PEPROCESS tCurProc	= PsGetCurrentProcess();
	char* tNamePtr		= (char*)tCurProc + ProcessNameOffset;
	memcpy(tProcessName, tNamePtr, 15);
	tProcessName[15] = 0;
	KdPrint(("process name:%s\n",tProcessName));

	//ready
	NTSTATUS			tRetStatus	= STATUS_SUCCESS;
	PMYDEVICE_EXTENSION tExtend		= (PMYDEVICE_EXTENSION)pDevObj->DeviceExtension;
	PIO_STACK_LOCATION	tIoStack	= IoGetCurrentIrpStackLocation(pIrp);

	//ip, port
	FILE_FULL_EA_INFORMATION* ea = (FILE_FULL_EA_INFORMATION*)pIrp->AssociatedIrp.SystemBuffer;
	if(0 != ea)
	{
		KdPrint(("eaname:%s\n",ea->EaName));

		if(0 == memcmp(ea->EaName, TdiTransportAddress, ea->EaNameLength))//传输层地址
		{
			//事先创建查询IRP
			PIRP tQueryIrp = TdiBuildInternalDeviceControlIrp(
													TDI_QUERY_INFORMATION, 
													tExtend->LowDeviceObj, 
													tIoStack->FileObject,
													0,
													0); 

			//下发到下一层设备，并等待完成
			IoCopyCurrentIrpStackLocationToNext(pIrp);
			IoSetCompletionRoutine(pIrp, DriverCreate_COMPLETION_ROUTINE, tQueryIrp, TRUE, TRUE, TRUE);
			tRetStatus = IoCallDriver(tExtend->LowDeviceObj, pIrp);

			KdPrint(("Leave DriverCreate by IoCallDriver TdiTransportAddress"));
			
			return tRetStatus;		
		} // TdiTransportAddress
		else if(0 == memcmp(ea->EaName, TdiConnectionContext, ea->EaNameLength))
		{
			//连接终端
			CONNECTION_CONTEXT tConnectCtx = *((CONNECTION_CONTEXT*)(ea->EaName + ea->EaNameLength + 1));
		}// TdiConnectionContext

	}// ea != 0

	IoSkipCurrentIrpStackLocation(pIrp);
	tRetStatus = IoCallDriver(tExtend->LowDeviceObj, pIrp);
	Debug("Leave DriverCreate");
	return tRetStatus;
}

extern "C" NTSTATUS DriverEntry (IN PDRIVER_OBJECT pDriverObject,IN PUNICODE_STRING pRegistryPath) 
{
	//获取进程名偏移
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

	//function
	for(int i=0; i<IRP_MJ_MAXIMUM_FUNCTION; i++)
	{
		pDriverObject->MajorFunction[i] = DriverCommonDispatch;
	}
	pDriverObject->DriverUnload = DriverUnload;
	pDriverObject->MajorFunction[IRP_MJ_CREATE]	= DriverCreate;

	PDEVICE_OBJECT	tLowDeviceObj	= 0;
	PFILE_OBJECT	tFileObj		= 0;
	UNICODE_STRING tLowDeviceName;
	RtlInitUnicodeString(&tLowDeviceName,L"\\Device\\Tcp");
	NTSTATUS tStatus = IoGetDeviceObjectPointer(&tLowDeviceName,FILE_ALL_ACCESS,&tFileObj,&tLowDeviceObj);
	if(false == NT_SUCCESS(tStatus))
		return tStatus;
	else
		ObDereferenceObject(tFileObj);

	UNICODE_STRING tMyDeviceName;
	RtlInitUnicodeString(&tMyDeviceName,L"\\Device\\TcpFilter");
	PDEVICE_OBJECT tMyDeviceObj = 0;
	tStatus = IoCreateDevice(pDriverObject,sizeof(MYDEVICE_EXTENSION),&tMyDeviceName,tLowDeviceObj->DeviceType,0,FALSE,&tMyDeviceObj);
	if(false == NT_SUCCESS(tStatus))
		return tStatus;

	UNICODE_STRING tMySymbol;
	RtlInitUnicodeString(&tMySymbol,L"\\??\\TcpFilter_Symbol");
	tStatus = IoCreateSymbolicLink(&tMySymbol,&tMyDeviceName);
	if(false == NT_SUCCESS(tStatus))
	{
		IoDeleteDevice(tMyDeviceObj);
		return tStatus;
	}
	tMyDeviceObj->Flags	= tLowDeviceObj->Flags;

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

