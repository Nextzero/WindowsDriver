#include "FuncHelper.h"

NTSTATUS PrintDeviceName(PVOID vDeviceObject)
{
	POBJECT_NAME_INFORMATION tObjName;
	tObjName = (POBJECT_NAME_INFORMATION)ExAllocatePool(NonPagedPool,1024);
	ULONG	tRetLen;
	NTSTATUS tRetStatus = ObQueryNameString(vDeviceObject,tObjName,1024,&tRetLen);
	if(false == NT_SUCCESS(tRetStatus))
	{
		KdPrint(("obquerynamestring failed\n"));
	}
	KdPrint(("i want to print the obj name\n"));
	KdPrint(("%wZ\n",&(tObjName->Name)));
	ExFreePool(tObjName);	
	return STATUS_SUCCESS;
}

NTSTATUS PrintIrp(PIRP vIrp)
{
	//´òÓ¡IRP±àºÅ
	PIO_STACK_LOCATION tIoStack = IoGetCurrentIrpStackLocation(vIrp);
	KdPrint(("MajorFunction:"));
	switch(tIoStack->MajorFunction)
	{
	case IRP_MJ_CREATE:
		{
			KdPrint(("IRP_MJ_CREATE\n"));
		}break;
	case IRP_MJ_CREATE_NAMED_PIPE:
		{
			KdPrint(("IRP_MJ_CREATE_NAMED_PIPE\n"));
		}break;
	case IRP_MJ_CLOSE:
		{
			KdPrint(("IRP_MJ_CLOSE\n"));
		}break;
	case IRP_MJ_READ:
		{
			KdPrint(("IRP_MJ_READ\n"));
		}break;
	case IRP_MJ_WRITE:
		{
			KdPrint(("IRP_MJ_WRITE\n"));
		}break;
	case IRP_MJ_QUERY_INFORMATION:
		{
			KdPrint(("IRP_MJ_QUERY_INFORMATION\n"));
		}break;
	case IRP_MJ_SET_INFORMATION:
		{
			KdPrint(("IRP_MJ_SET_INFORMATION\n"));
		}break;
	case IRP_MJ_QUERY_EA:
		{
			KdPrint(("IRP_MJ_QUERY_EA\n"));
		}break;
	case IRP_MJ_SET_EA:
		{
			KdPrint(("IRP_MJ_SET_EA\n"));
		}break;
	case IRP_MJ_FLUSH_BUFFERS:
		{
			KdPrint(("IRP_MJ_FLUSH_BUFFERS\n"));
		}break;
	case IRP_MJ_QUERY_VOLUME_INFORMATION:
		{
			KdPrint(("IRP_MJ_QUERY_VOLUME_INFORMATION\n"));
		}break;
	case IRP_MJ_SET_VOLUME_INFORMATION:
		{
			KdPrint(("IRP_MJ_SET_VOLUME_INFORMATION\n"));
		}break;
	case IRP_MJ_DIRECTORY_CONTROL:
		{
			KdPrint(("IRP_MJ_DIRECTORY_CONTROL\n"));
		}break;
	case IRP_MJ_FILE_SYSTEM_CONTROL:
		{
			KdPrint(("IRP_MJ_FILE_SYSTEM_CONTROL\n"));
		}break;
	case IRP_MJ_DEVICE_CONTROL:
		{
			KdPrint(("IRP_MJ_DEVICE_CONTROL\n"));
		}break;
	case IRP_MJ_INTERNAL_DEVICE_CONTROL:
		{
			KdPrint(("IRP_MJ_INTERNAL_DEVICE_CONTROL\n"));
			if(TDI_CONNECT == tIoStack->MinorFunction)
			{
				KdPrint(("TDI_CONNECT\n"));
			}
			if(TDI_ASSOCIATE_ADDRESS == tIoStack->MinorFunction)
			{
				KdPrint(("TDI_ASSOCIATE_ADDRESS\n"));
			}
			if(TDI_DISASSOCIATE_ADDRESS== tIoStack->MinorFunction)
			{
				KdPrint(("TDI_DISASSOCIATE_ADDRESS\n"));
			}
		}break;
	case IRP_MJ_SHUTDOWN:
		{
			KdPrint(("IRP_MJ_SHUTDOWN\n"));
		}break;
	case IRP_MJ_LOCK_CONTROL:
		{
			KdPrint(("IRP_MJ_LOCK_CONTROL\n"));
		}break;
	case IRP_MJ_CLEANUP:
		{
			KdPrint(("IRP_MJ_CLEANUP\n"));
		}break;
	case IRP_MJ_CREATE_MAILSLOT:
		{
			KdPrint(("IRP_MJ_CREATE_MAILSLOT\n"));
		}break;
	case IRP_MJ_QUERY_SECURITY:
		{
			KdPrint(("IRP_MJ_QUERY_SECURITY\n"));
		}break;
	case IRP_MJ_SET_SECURITY:
		{
			KdPrint(("IRP_MJ_SET_SECURITY\n"));
		}break;
	case IRP_MJ_POWER:
		{
			KdPrint(("IRP_MJ_POWER\n"));
		}break;
	case IRP_MJ_SYSTEM_CONTROL:
		{
			KdPrint(("IRP_MJ_SYSTEM_CONTROL\n"));
		}break;
	case IRP_MJ_DEVICE_CHANGE:
		{
			KdPrint(("IRP_MJ_DEVICE_CHANGE\n"));
		}break;
	case IRP_MJ_QUERY_QUOTA:
		{
			KdPrint(("IRP_MJ_QUERY_QUOTA\n"));
		}break;
	case IRP_MJ_SET_QUOTA:
		{
			KdPrint(("IRP_MJ_SET_QUOTA\n"));
		}break;
	case IRP_MJ_PNP:
		{
			KdPrint(("IRP_MJ_PNP\n"));
		}break;
	default:
		{
			KdPrint(("error\n"));
		}break;
	}

	return STATUS_SUCCESS;
}