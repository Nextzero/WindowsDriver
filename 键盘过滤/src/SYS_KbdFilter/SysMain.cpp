#include "SysMain.h"

static int			    g_Status			= (ENABLE & (~INTERCEPT));
static USHORT			g_CurMakeCode		= 0;
static PVOID			g_ShareEvent		= 0;
static PDEVICE_OBJECT	g_MasterDeviceObj	= 0;

VOID DriverUnload(IN PDRIVER_OBJECT pDriverObject)
{
	PDEVICE_OBJECT pNextDevObj = pDriverObject->DeviceObject;
	while(0 != pNextDevObj)
	{
		PMYDEVICE_EXTENSION pDevExtend = (PMYDEVICE_EXTENSION)pNextDevObj->DeviceExtension;
		
		if(pNextDevObj == g_MasterDeviceObj)
		{
			PDEVICE_OBJECT pTempDevObj = pNextDevObj->NextDevice;
			UNICODE_STRING tMasterDevObjSymbol;
			RtlInitUnicodeString(&tMasterDevObjSymbol,__T(DEVICE_NAME_KBDFILTER_SYMBOL));
			IoDeleteSymbolicLink(&tMasterDevObjSymbol);
			IoDeleteDevice(pNextDevObj);
			pNextDevObj = pTempDevObj;
			continue;
		}
		
		PIRP tCurIrp		= pDevExtend->CurIrp;
		BOOLEAN tCancelRet	= FALSE;
		if(0 != tCurIrp)
		{
			 tCancelRet = IoCancelIrp(tCurIrp);
		}

		if(0 != pDevExtend->LowDeviceObj)
		{
			IoDetachDevice(pDevExtend->LowDeviceObj);
		}

		LARGE_INTEGER tDelay;
		tDelay.QuadPart = 1000*10000*(-1);
		KeDelayExecutionThread(KernelMode,FALSE,&tDelay);
	
		PDEVICE_OBJECT pTempDevObj = pNextDevObj->NextDevice;
		IoDeleteDevice(pNextDevObj);
		pNextDevObj = pTempDevObj;
	}
}

NTSTATUS DriverCommonDispatch(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp)
{
	Debug("Enter DriverCommonDispatch");
	PMYDEVICE_EXTENSION tDevExtend = (PMYDEVICE_EXTENSION)pDevObj->DeviceExtension;
	PIO_STACK_LOCATION tIrpStack = IoGetCurrentIrpStackLocation(pIrp);
	KdPrint(("%x\n",tIrpStack->MajorFunction));
	KdPrint(("%x\n",tIrpStack->MinorFunction));
	if(pDevObj == g_MasterDeviceObj)
	{
		pIrp->IoStatus.Information	= 0;
		pIrp->IoStatus.Status		= STATUS_SUCCESS;
		IoCompleteRequest(pIrp,IO_NO_INCREMENT);
		Debug("g_MasterDeviceObj,Dispatch");
		return STATUS_SUCCESS;
	}

	IoSkipCurrentIrpStackLocation(pIrp);
	NTSTATUS tStatus = IoCallDriver(tDevExtend->LowDeviceObj,pIrp);
	Debug("Leave DriverCommonDispatch");
	return tStatus;
}

NTSTATUS DriverPower(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp)
{
	Debug("Enter DriverPower");
	
	if(pDevObj == g_MasterDeviceObj)
	{
		pIrp->IoStatus.Information	= 0;
		pIrp->IoStatus.Status		= STATUS_SUCCESS;
		IoCompleteRequest(pIrp,IO_NO_INCREMENT);
		Debug("g_MasterDeviceObj,DriverPower");
		return STATUS_SUCCESS;
	}

	PMYDEVICE_EXTENSION tDevExtend = (PMYDEVICE_EXTENSION)pDevObj->DeviceExtension;
	PoStartNextPowerIrp(pIrp);
	IoSkipCurrentIrpStackLocation(pIrp);
	NTSTATUS tStatus = PoCallDriver(tDevExtend->LowDeviceObj,pIrp);
	Debug("Leave DriverPower");
	return tStatus;
}

NTSTATUS MyIoCompletion(IN PDEVICE_OBJECT  DeviceObject, IN PIRP  Irp, IN PVOID  Context)
{
	Debug("Enter MyIoCompletion");
	
	if(NT_SUCCESS(Irp->IoStatus.Status))
	{
		PKEYBOARD_INPUT_DATA tKeyData = (PKEYBOARD_INPUT_DATA )Irp->AssociatedIrp.SystemBuffer;
		if(tKeyData->Flags)
		{
			g_CurMakeCode	= tKeyData->MakeCode;
			if(0 != g_ShareEvent && (g_Status & INTERCEPT))
			{
				KeSetEvent((PRKEVENT)g_ShareEvent,(KPRIORITY)0, false); //弹起
			}
		}
		if(!(g_Status & ENABLE))  //按下与弹起
		{
			ULONG tBufLen	= Irp->IoStatus.Information;
			ULONG tNumKeys	= tBufLen / sizeof(KEYBOARD_INPUT_DATA);

			for(int i=0;i<tNumKeys;++i)
				tKeyData[i].MakeCode = 0x00;
		}
		KdPrint(("扫描码:%x,%s\n",tKeyData->MakeCode,tKeyData->Flags?"弹起":"按下"));
	}

	if(Irp->PendingReturned)
	{
		IoMarkIrpPending(Irp);
	}

	Debug("Leave MyIoCompletion");
	return Irp->IoStatus.Status;
}


NTSTATUS DriverRead(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp)
{
	Debug("Enter DriverRead");

	if(pDevObj == g_MasterDeviceObj)
	{
		pIrp->IoStatus.Information	= 0;
		pIrp->IoStatus.Status		= STATUS_SUCCESS;
		IoCompleteRequest(pIrp,IO_NO_INCREMENT);
		Debug("g_MasterDeviceObj,DriverRead");
		return STATUS_SUCCESS;
	}

	PMYDEVICE_EXTENSION tDevExtend = (PMYDEVICE_EXTENSION)pDevObj->DeviceExtension;
	tDevExtend->CurIrp = pIrp;

	PIO_STACK_LOCATION tIrpStack = IoGetCurrentIrpStackLocation(pIrp);
	IoCopyCurrentIrpStackLocationToNext(pIrp);
	
	IoSetCompletionRoutine(pIrp,MyIoCompletion,0,TRUE,TRUE,TRUE);
	NTSTATUS tStatus = IoCallDriver(tDevExtend->LowDeviceObj,pIrp);
	Debug("Leave DriverRead");
	return tStatus;
}

NTSTATUS DriverPNP(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp)
{
	Debug("Enter DriverPNP");

	if(pDevObj == g_MasterDeviceObj)
	{
		pIrp->IoStatus.Information	= 0;
		pIrp->IoStatus.Status		= STATUS_SUCCESS;
		IoCompleteRequest(pIrp,IO_NO_INCREMENT);
		Debug("g_MasterDeviceObj,DriverPNP");
		return STATUS_SUCCESS;
	}

	PMYDEVICE_EXTENSION tDevExtend	= (PMYDEVICE_EXTENSION)pDevObj->DeviceExtension;
	PIO_STACK_LOCATION tIrpStack	= IoGetCurrentIrpStackLocation(pIrp);

	NTSTATUS tStatus;
	if(IRP_MN_REMOVE_DEVICE == tIrpStack->MinorFunction)
	{
		IoSkipCurrentIrpStackLocation(pIrp);
		tStatus = IoCallDriver(tDevExtend->LowDeviceObj,pIrp);

		IoDetachDevice(tDevExtend->LowDeviceObj);
		IoDeleteDevice(pDevObj);
		tStatus = STATUS_SUCCESS;
	}
	else
	{
		IoSkipCurrentIrpStackLocation(pIrp);
		tStatus = IoCallDriver(tDevExtend->LowDeviceObj,pIrp);
	}	
	Debug("Leave DriverPNP");
	return tStatus;
}

NTSTATUS DriverControl(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp)
{
	Debug("Enter DriverControl");

	PMYDEVICE_EXTENSION tDevExtend = (PMYDEVICE_EXTENSION)pDevObj->DeviceExtension;
	PIO_STACK_LOCATION	tIrpStack	= IoGetCurrentIrpStackLocation(pIrp);

	if(pDevObj != g_MasterDeviceObj || 0 == g_MasterDeviceObj)
	{
		IoSkipCurrentIrpStackLocation(pIrp);
		Debug("not master device");
		return IoCallDriver(tDevExtend->LowDeviceObj,pIrp);
	}
	
	ULONG tInBufLen		= tIrpStack->Parameters.DeviceIoControl.InputBufferLength;
	ULONG tOutBufLen	= tIrpStack->Parameters.DeviceIoControl.OutputBufferLength;
	PVOID tInBuf		= pIrp->AssociatedIrp.SystemBuffer;
	int*  tOutBuf		= (int*)pIrp->AssociatedIrp.SystemBuffer;
	ULONG tControlCode	= tIrpStack->Parameters.DeviceIoControl.IoControlCode;
	
	ULONG	 tRetLen = 0;
	NTSTATUS tStatus = STATUS_SUCCESS;
	switch(tControlCode)
	{
	case IOCTL_KBDFILTER_ENABLE:
		{
			g_Status	|= ENABLE;
			(*tOutBuf)	= g_Status;
			tRetLen		= sizeof(g_Status);
		}break;
	case IOCTL_KBDFILTER_DISABLE:
		{
			g_Status	&= (~ENABLE);
			(*tOutBuf)	= g_Status;
			tRetLen		= sizeof(g_Status);
		}break;
	case IOCTL_KBDFILTER_INTERCEPT:
		{
			g_Status	|= INTERCEPT;
			(*tOutBuf)	= g_Status;
			tRetLen		= sizeof(g_Status);
		}break;
	case IOCTL_KBDFILTER_CANCELINTERCEPT:
		{
			g_Status	&= (~INTERCEPT);
			(*tOutBuf)	= g_Status;
			tRetLen		= sizeof(g_Status);
		}break;
	case IOCTL_KBDFILTER_GETSTATUS:
		{
			(*tOutBuf)	= g_Status;
			tRetLen		= sizeof(g_Status);
		}break;
	case IOCTL_KBDFILTER_SHAREEVENT:
		{
			HANDLE tShareEvent = (*(HANDLE*)tInBuf);
			ObReferenceObjectByHandle(tShareEvent,FILE_ALL_ACCESS,0,KernelMode,&g_ShareEvent,0);
		}break;
	case IOCTL_KBDFILTER_GETMAKECODE:
		{
			PUSHORT tCh = (PUSHORT)tOutBuf;
			(*tCh)	= g_CurMakeCode;
			tRetLen = sizeof(g_CurMakeCode);
		}break;
	default:
		{
			pIrp->IoStatus.Information	= 0;
			pIrp->IoStatus.Status		= STATUS_UNSUCCESSFUL;
			return STATUS_UNSUCCESSFUL;
		}break;
	}
	pIrp->IoStatus.Information	= tRetLen;
	pIrp->IoStatus.Status		= tStatus;
	IoCompleteRequest(pIrp,IO_NO_INCREMENT);
	Debug("Leave DriverControl");
	return tStatus;
}

extern "C" NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject,IN PUNICODE_STRING pRegistryPath) 
{
	for(int i_Dispatch = 0; i_Dispatch < IRP_MJ_MAXIMUM_FUNCTION; i_Dispatch++)
	{
		pDriverObject->MajorFunction[i_Dispatch] = DriverCommonDispatch;
	}
	pDriverObject->MajorFunction[IRP_MJ_POWER]			= DriverPower;
	pDriverObject->MajorFunction[IRP_MJ_PNP]			= DriverPNP;
	pDriverObject->MajorFunction[IRP_MJ_READ]			= DriverRead;
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]	= DriverControl;

	pDriverObject->DriverUnload = DriverUnload;

	//打开kbdclass驱动对象
	PDRIVER_OBJECT tDriObjKdbclass;
	UNICODE_STRING tKbdclassName;
	RtlInitUnicodeString(&tKbdclassName,L"\\Driver\\Kbdclass");
	NTSTATUS tStatus = ObReferenceObjectByName(&tKbdclassName,OBJ_CASE_INSENSITIVE,0,FILE_ALL_ACCESS,IoDeviceObjectType,KernelMode,0,(PVOID*)&tDriObjKdbclass);
	if(false == NT_SUCCESS(tStatus))
	{
		return STATUS_UNSUCCESSFUL;
	}
	else
	{
		ObDereferenceObject(tDriObjKdbclass);
	}

	//创建并绑定设备
	bool tAttch = false;
	PDEVICE_OBJECT tTargetDevObj = tDriObjKdbclass->DeviceObject; //kbdclass下的设备对象
	while(0 != tTargetDevObj)
	{
		tAttch = true;
		PDEVICE_OBJECT tMyDeviceObject;
		tStatus = IoCreateDevice(pDriverObject,sizeof(MYDEVICE_EXTENSION),0,tTargetDevObj->DeviceType,tTargetDevObj->Characteristics,FALSE,&tMyDeviceObject);
		if(false == NT_SUCCESS(tStatus))
		{
			return STATUS_UNSUCCESSFUL; //之前的设备要不要删掉？
		}
		tMyDeviceObject->Flags |= tTargetDevObj->Flags & (DO_BUFFERED_IO | DO_DIRECT_IO | DO_POWER_PAGABLE);

		PMYDEVICE_EXTENSION tDevExtend = (PMYDEVICE_EXTENSION)tMyDeviceObject->DeviceExtension;
		tDevExtend->LowDeviceObj = IoAttachDeviceToDeviceStack(tMyDeviceObject,tTargetDevObj);
		if(0 == tDevExtend->LowDeviceObj)
		{
			IoDeleteDevice(tMyDeviceObject);
			return STATUS_UNSUCCESSFUL;
		}
		tMyDeviceObject->Flags = tMyDeviceObject->Flags & ~DO_DEVICE_INITIALIZING;

		tTargetDevObj = tTargetDevObj->NextDevice;
	}
	
	if(false == tAttch)
	{
		return STATUS_UNSUCCESSFUL;
	}

	//创建主控设备
	UNICODE_STRING tMasterDevObjName;
	RtlInitUnicodeString(&tMasterDevObjName,__T(DEVICE_NAME_KBDFILTER));
	tStatus = IoCreateDevice(pDriverObject,sizeof(MYDEVICE_EXTENSION),&tMasterDevObjName,FILE_DEVICE_UNKNOWN,0,FALSE,&g_MasterDeviceObj);
	if(false == NT_SUCCESS(tStatus))
	{
		return STATUS_UNSUCCESSFUL;
	}

	UNICODE_STRING tMasterDevObjSymbol;
	RtlInitUnicodeString(&tMasterDevObjSymbol,__T(DEVICE_NAME_KBDFILTER_SYMBOL));
	tStatus = IoCreateSymbolicLink(&tMasterDevObjSymbol,&tMasterDevObjName);
	if(false == NT_SUCCESS(tStatus))
	{
		return STATUS_UNSUCCESSFUL;
	}
	g_MasterDeviceObj->Flags |= (DO_BUFFERED_IO | DO_DIRECT_IO);
	return STATUS_SUCCESS;
}

