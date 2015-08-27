#include "SysMain.h"
//---------------------------------------------------------------
PDRIVER_OBJECT gMyDriverObject = 0;
//---------------------------------------------------------------
VOID DriverUnload(IN PDRIVER_OBJECT pDriverObject)
{
	PDEVICE_OBJECT tDevObj = pDriverObject->DeviceObject;
	while(tDevObj)
	{
		PDEVICE_OBJECT	tNext	= tDevObj->NextDevice;
		DeviceExtend*	tExtend	= (DeviceExtend*)tDevObj->DeviceExtension;
		
		if(_MyControlDevice == tExtend->Type)
		{
			Debug("Delete MyControlDevice");
			IoDeleteDevice(tDevObj);
		}
		if(_FilterFsMasterDevice == tExtend->Type) 
		{
			if(0 != tExtend->DirctLowDevObj)
			{
				Debug("DetachDevice _FilterFsMasterDevice");
				IoDetachDevice(tExtend->DirctLowDevObj);
			}
	
			Debug("Delete _FilterFsMasterDevice");
			IoDeleteDevice(tDevObj);
		}
		if(_FilterFsRollDevice == tExtend->Type)
		{
			if(0 != tExtend->DirctLowDevObj)
			{
				Debug("DetachDevice _FilterFsRollDevice ");
				IoDetachDevice(tExtend->DirctLowDevObj);
			}

			Debug("Delete _FilterFsRollDevice");
			IoDeleteDevice(tDevObj);
		}
		
		tDevObj = tNext;
	}

	DestoryFastIo(pDriverObject);
	return ;
}
//---------------------------------------------------------------
VOID DriverNotificationRoutine(struct _DEVICE_OBJECT *DeviceObject,BOOLEAN FsActive)
{
	Debug("Enter DriverNotificationRoutine");
	PrintDeviceName(DeviceObject);

	if(!IsNeedFilterType(DeviceObject))
	{
		Debug("I Can't Filter this DeviceType");
		return ;
	}

	NTSTATUS tRetStatus;

	if(TRUE == FsActive)
	{	
		//����
		Debug("�ļ�ϵͳ����");
		
		PDEVICE_OBJECT	tFsFilterMasterObj;
		tRetStatus = IoCreateDevice(gMyDriverObject,sizeof(DeviceExtend),0,DeviceObject->Type,0,FALSE,&tFsFilterMasterObj);//no name
		if(false == NT_SUCCESS(tRetStatus))
		{
			Debug("�ļ�ϵͳ�����豸�����豸IoCreateDevice failed\n");
			return ;
		}
		if(DeviceObject->Flags & DO_BUFFERED_IO)
			tFsFilterMasterObj->Flags |= DO_BUFFERED_IO;
		if(DeviceObject->Flags & DO_DIRECT_IO)
			tFsFilterMasterObj->Flags |= DO_DIRECT_IO;
		if(DeviceObject->Characteristics & FILE_DEVICE_SECURE_OPEN)
			tFsFilterMasterObj->Characteristics |= FILE_DEVICE_SECURE_OPEN;
		tFsFilterMasterObj->Flags &= ~DO_DEVICE_INITIALIZING;	//http://blog.csdn.net/wzsy/article/details/6191451

		DeviceExtend* tExtend	= (DeviceExtend*)tFsFilterMasterObj->DeviceExtension;
		tExtend->Type			= _FilterFsMasterDevice;
		tExtend->DirctLowDevObj	= IoAttachDeviceToDeviceStack(tFsFilterMasterObj,DeviceObject);
		if(0 == tExtend->DirctLowDevObj)
		{
			Debug("_FilterFsMasterDevice Attach failed");
			IoDeleteDevice(tFsFilterMasterObj);
			return ;
		}
		Debug("_FilterFsMasterDevice Attach Success");
	}
	else
	{
		//ע��
		Debug("ע���ļ�ϵͳ");
	}
	Debug("Leave DriverNotificationRoutine");
}
//---------------------------------------------------------------
NTSTATUS CommonDispatch(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp)
{
	Debug("Enter CommonDispatch");

	PrintDeviceName(pDevObj);
	PrintIrp(pIrp);

	DeviceExtend* tDevExtend	= (DeviceExtend*)pDevObj->DeviceExtension;
	if(_MyControlDevice == tDevExtend->Type)
	{
		pIrp->IoStatus.Information	= 0;
		pIrp->IoStatus.Status		= STATUS_SUCCESS;
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		Debug("Leave CommonDispatch");
		return STATUS_SUCCESS;
	}
	else //_FilterFsMasterDevice or _FilterFsRollDevice
	{
		IoSkipCurrentIrpStackLocation(pIrp);
		NTSTATUS tRetStatus = IoCallDriver(tDevExtend->DirctLowDevObj,pIrp);
		Debug("leave CommonDispath");
		return tRetStatus;
	}
}
//---------------------------------------------------------------
NTSTATUS FsFilterCreate_COMPLETION_ROUTINE(IN PDEVICE_OBJECT DeviceObject,IN PIRP Irp,IN PVOID Context)
{
	PKEVENT tEvent = (PKEVENT)Context;
	PIO_STACK_LOCATION tIoStack = IoGetCurrentIrpStackLocation(Irp);
	Debug("FsFilterCreate_COMPLETION_ROUTINE");
	if(NT_SUCCESS(Irp->IoStatus.Status))
	{
		if( (0 != tIoStack->FileObject) && tIoStack->Parameters.Create.Options & FILE_DIRECTORY_FILE)
		{
			Debug("this is a dir");
		}
		KdPrint(("%wZ\n",&(tIoStack->FileObject->FileName)));
		PrintDeviceName(tIoStack->FileObject);
	}
	KeSetEvent(tEvent, IO_NO_INCREMENT, FALSE);
	return STATUS_MORE_PROCESSING_REQUIRED;
}
//---------------------------------------------------------------
NTSTATUS FsFilterCreate(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp)
{
	Debug("Enter FsFilterCreate");
	NTSTATUS tRetStatus;
	DeviceExtend* tExtend = (DeviceExtend*)pDevObj->DeviceExtension;
	if(_MyControlDevice == tExtend->Type)		//�Լ��Ŀ����豸
	{
		Debug("FsFilterCreate,_MyControlDevice ");
		pIrp->IoStatus.Information	= 0;
		pIrp->IoStatus.Status		= STATUS_SUCCESS;
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		return STATUS_SUCCESS;
	}
	if(_FilterFsMasterDevice == tExtend->Type)	//�ļ�ϵͳ�����豸�Ĺ����豸
	{
		Debug("FsFilterCreate,_FilterFsMasterDevice");
		IoSkipCurrentIrpStackLocation(pIrp);
		return IoCallDriver(tExtend->DirctLowDevObj, pIrp);
	}
	if(_FilterFsRollDevice == tExtend->Type)	//�ļ�ϵͳ���豸�Ĺ����豸
	{
		Debug("FsFilterCreate,_FilterFsRollDevice");

		PIO_STACK_LOCATION tIoStack = IoGetCurrentIrpStackLocation(pIrp);
		KdPrint(("file name when enter create function:%wZ\n",&(tIoStack->FileObject->FileName)));
		PrintDeviceName(tIoStack->FileObject);

		KEVENT tEvent;
		KeInitializeEvent(&tEvent, NotificationEvent, FALSE);
		IoCopyCurrentIrpStackLocationToNext(pIrp);
		IoSetCompletionRoutine(pIrp, FsFilterCreate_COMPLETION_ROUTINE, &tEvent, TRUE, TRUE, TRUE);
		tRetStatus = IoCallDriver(tExtend->DirctLowDevObj, pIrp);
		if(STATUS_PENDING == tRetStatus)
		{
			KeWaitForSingleObject(&tEvent,Executive,KernelMode,FALSE, 0);
		}

		tRetStatus = pIrp->IoStatus.Status;
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		Debug("Leave FsFilterCreate");
		return tRetStatus;
	}
	
	//error
	pIrp->IoStatus.Information	= 0;
	pIrp->IoStatus.Status		= STATUS_UNSUCCESSFUL;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return pIrp->IoStatus.Status;
}
//---------------------------------------------------------------
NTSTATUS FsFilterClose(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp)
{
	Debug("Enter FsFilterClose");
	NTSTATUS tRetStatus = CommonDispatch(pDevObj,pIrp);
	Debug("Leave FsFilterClose");
	return tRetStatus;
}
//---------------------------------------------------------------
NTSTATUS FsFilterDeviceControl(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp)
{
	Debug("Enter FsFilterDeviceControl");
	NTSTATUS tRetStatus;
	PIO_STACK_LOCATION	tIoStack = IoGetCurrentIrpStackLocation(pIrp);
	LONG tControlCode = tIoStack->Parameters.DeviceIoControl.IoControlCode;
	tRetStatus = CommonDispatch(pDevObj, pIrp);
	Debug("Leave FsFilterDeviceControl");
	return tRetStatus;
}
//---------------------------------------------------------------
NTSTATUS FsFilterFileSystemControl_COMPLETION_ROUTINE(IN PDEVICE_OBJECT DeviceObject,IN PIRP Irp,IN PVOID Context)
{
	PKEVENT tEvent = (PKEVENT)Context;
	KeSetEvent(tEvent, IO_NO_INCREMENT, FALSE);
	return STATUS_MORE_PROCESSING_REQUIRED;
}
//---------------------------------------------------------------
NTSTATUS FsFilterFileSystemControl(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp)
{
	Debug("Enter FsFilterFileSystemControl");
	PrintDeviceName(pDevObj);
	DeviceExtend* tExtend = (DeviceExtend*)pDevObj->DeviceExtension; //������ļ�ϵͳ�����豸�Ĺ����豸���豸��չ

	//�������ļ�ϵͳ�����豸�Ĺ����豸����ֱ�ӷ���
	if(_FilterFsMasterDevice != tExtend->Type)
	{
		Debug("Just _FilterFsMasterDevice need to handle FsFilterFileSystemControl");
		pIrp->IoStatus.Information	= 0;
		pIrp->IoStatus.Status		= STATUS_UNSUCCESSFUL;
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		Debug("Leave FsFilterFileSystemControl\n");
		return STATUS_UNSUCCESSFUL;
	}

	NTSTATUS tRetStatus;
	PIO_STACK_LOCATION tIrpStack = IoGetCurrentIrpStackLocation(pIrp);

	//�󶨾�����ļ�ϵͳ���豸
	if(IRP_MN_MOUNT_VOLUME == tIrpStack->MinorFunction)
	{
		//�����ļ�ϵͳ���豸�Ĺ����豸
		PDEVICE_OBJECT	tFilterRollDeviceObj;
		tRetStatus = IoCreateDevice(gMyDriverObject, sizeof(DeviceExtend), 0, 0, 0, FALSE, &tFilterRollDeviceObj);
		if(false == NT_SUCCESS(tRetStatus))
		{
			Debug("create Filter Roll Device failed, call next driver directly");
			IoSkipCurrentIrpStackLocation(pIrp);
			return IoCallDriver(tExtend->DirctLowDevObj, pIrp);
		}

		DeviceExtend* tFilterRollExtend				= (DeviceExtend*)tFilterRollDeviceObj->DeviceExtension; //�����ļ�ϵͳ���豸�Ĺ����豸���豸��չ
		tFilterRollExtend->Type						= _FilterFsRollDevice; 
		tFilterRollExtend ->RealStorageDeviceObj	= tIrpStack->Parameters.MountVolume.Vpb->RealDevice;	//����ʵ�ʴ洢���豸�������ļ�ϵͳ���豸

		//������ɺ���
		KEVENT tEvent;
		KeInitializeEvent(&tEvent,NotificationEvent,FALSE);
		IoCopyCurrentIrpStackLocationToNext(pIrp);
		IoSetCompletionRoutine(pIrp, FsFilterFileSystemControl_COMPLETION_ROUTINE,&tEvent,TRUE,TRUE,TRUE);
		tRetStatus =  IoCallDriver(tExtend->DirctLowDevObj, pIrp);
		if(STATUS_PENDING == tRetStatus)
		{
			tRetStatus = KeWaitForSingleObject(&tEvent,Executive, KernelMode, FALSE, NULL);
		}
		if(false == NT_SUCCESS(tRetStatus))
		{
			Debug("Mount Device IoCallDriver failed");
			tRetStatus	= pIrp->IoStatus.Status;
			IoCompleteRequest(pIrp, IO_NO_INCREMENT);   // need Complete this IRP again ?
			IoDeleteDevice(tFilterRollDeviceObj);
			return tRetStatus;
		}
		
		//��
		PDEVICE_OBJECT tFsRollDeviceObj = tFilterRollExtend->RealStorageDeviceObj->Vpb->DeviceObject; //IoCallDriver��ɺ����ɵ��ļ�ϵͳ���豸
		tFilterRollExtend->DirctLowDevObj = IoAttachDeviceToDeviceStack(tFilterRollDeviceObj, tFsRollDeviceObj);
		if(0 == tFilterRollExtend->DirctLowDevObj)
		{
			Debug("�ļ�ϵͳ���豸 attach failed");
			IoDeleteDevice(tFilterRollDeviceObj);
			tRetStatus	= pIrp->IoStatus.Status;
			IoCompleteRequest(pIrp, IO_NO_INCREMENT);  
			return tRetStatus;
		}
		Debug("Attach FS Roll Device Success");

		if(tFsRollDeviceObj->Flags & DO_BUFFERED_IO)
			tFilterRollDeviceObj->Flags |= DO_BUFFERED_IO;
		if(tFsRollDeviceObj->Flags & DO_DIRECT_IO)
			tFilterRollDeviceObj->Flags |= DO_DIRECT_IO;
		if(tFsRollDeviceObj->Characteristics & FILE_DEVICE_SECURE_OPEN)
			tFilterRollDeviceObj->Characteristics |= FILE_DEVICE_SECURE_OPEN;
		tFilterRollDeviceObj->Flags &= ~DO_DEVICE_INITIALIZING;

		tRetStatus	= pIrp->IoStatus.Status;
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);   // need Complete this IRP again ?
		return tRetStatus;
	}
	if(IRP_MN_LOAD_FILE_SYSTEM == tIrpStack->MinorFunction)
	{
		//�ļ�ϵͳʶ����������ʵ�ʵ��ļ�ϵͳ�����ƻ���ͨ���ļ�ϵͳע�ắ��ʵ���ж�
		Debug("Load real file system");

		KEVENT tEventLoadFs;
		KeInitializeEvent(&tEventLoadFs,NotificationEvent,FALSE);
		IoCopyCurrentIrpStackLocationToNext(pIrp);
		IoSetCompletionRoutine(pIrp, FsFilterFileSystemControl_COMPLETION_ROUTINE,&tEventLoadFs,TRUE,TRUE,TRUE);
		tRetStatus =  IoCallDriver(tExtend->DirctLowDevObj, pIrp);
		if(STATUS_PENDING == tRetStatus)
		{
			tRetStatus = KeWaitForSingleObject(&tEventLoadFs,Executive, KernelMode, FALSE, NULL);
		}
		if(false == NT_SUCCESS(tRetStatus))
		{
			Debug("�ļ�ϵͳʶ����LoadFs failed");
			tRetStatus	= pIrp->IoStatus.Status;
			IoCompleteRequest(pIrp, IO_NO_INCREMENT);   // need Complete this IRP again ?
			return tRetStatus;
		}

		if(!NT_SUCCESS(pIrp->IoStatus.Status) && STATUS_IMAGE_ALREADY_LOADED != pIrp->IoStatus.Status)
		{
			//error
			Debug("error");
		}
		else
		{
			//����ļ�ʶ�����İ�
			Debug("����ļ�ʶ�����İ�");
			IoDetachDevice(tExtend->DirctLowDevObj);
			IoDeleteDevice(pDevObj);
		}

		tRetStatus = pIrp->IoStatus.Status;
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		return tRetStatus;
	}

	if(IRP_MN_USER_FS_REQUEST == tIrpStack->MinorFunction)
	{
		tIrpStack->Parameters
	}
	
	//����IRP����
	IoSkipCurrentIrpStackLocation(pIrp);
	tRetStatus = IoCallDriver(tExtend->DirctLowDevObj, pIrp);
	Debug("Leave FsFilterFileSystemControl");
	return tRetStatus;
}
//---------------------------------------------------------------
NTSTATUS FsFilterRead_COMPLETION_ROUTINE(IN PDEVICE_OBJECT DeviceObject,IN PIRP Irp,IN PVOID Context)
{
	PKEVENT tEvent = (PKEVENT)Context;
	KeSetEvent(tEvent, IO_NO_INCREMENT, FALSE);
	return STATUS_MORE_PROCESSING_REQUIRED;
}
//---------------------------------------------------------------
NTSTATUS FsFilterRead(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp)
{
	Debug("Enter FsFilterRead");
	NTSTATUS tRetStatus;
	DeviceExtend* tExtend = (DeviceExtend*)pDevObj->DeviceExtension;
	if(_MyControlDevice == tExtend->Type)	//Ӧ�ò�����豸
	{
		Debug("FsFilterRead, _MyControlDevice");
		pIrp->IoStatus.Status		= STATUS_SUCCESS;
		pIrp->IoStatus.Information	= 0;
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		return STATUS_SUCCESS;
	}
	if(_FilterFsMasterDevice == tExtend->Type)	//�ļ�ϵͳ�����豸
	{
		Debug("FsFilterRead, _FilterFsMasterDevice");
		IoSkipCurrentIrpStackLocation(pIrp);
		return IoCallDriver(tExtend->DirctLowDevObj, pIrp);
	}
	if(_FilterFsRollDevice == tExtend->Type)	//�ļ�ϵͳ���豸
	{
		KEVENT tEventRead;
		KeInitializeEvent(&tEventRead, SynchronizationEvent, FALSE);
		IoCopyCurrentIrpStackLocationToNext(pIrp);
		IoSetCompletionRoutine(pIrp, FsFilterRead_COMPLETION_ROUTINE, &tEventRead, TRUE, TRUE, TRUE);
		tRetStatus = IoCallDriver(tExtend->DirctLowDevObj, pIrp);
		if(STATUS_PENDING == tRetStatus)
		{
			KeWaitForSingleObject(&tEventRead, Executive, KernelMode, FALSE, 0);
		}
		if(!NT_SUCCESS(tRetStatus))
		{
			Debug("FsFilterRead IoCallDriverFailed");
			IoCompleteRequest(pIrp, IO_NO_INCREMENT);
			return tRetStatus;
		}
		else
		{
			Debug("Read success");
			IoCompleteRequest(pIrp, IO_NO_INCREMENT);
			return tRetStatus;
		}
	}
	
	Debug("FsFilterRead error");		//error
	pIrp->IoStatus.Information	= 0;
	pIrp->IoStatus.Status		= STATUS_UNSUCCESSFUL;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	Debug("Leave FsFilterRead");
	return STATUS_UNSUCCESSFUL;
}
//---------------------------------------------------------------
NTSTATUS FsFilterWrite_COMPLETION_ROUTINE(IN PDEVICE_OBJECT DeviceObject,IN PIRP Irp,IN PVOID Context)
{
	PKEVENT tEvent = (PKEVENT)Context;
	KeSetEvent(tEvent, IO_NO_INCREMENT, FALSE);
	return STATUS_MORE_PROCESSING_REQUIRED;
}
//---------------------------------------------------------------
NTSTATUS FsFilterWrite(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp)
{
	Debug("Enter FsFilterWrite");
	NTSTATUS tRetStatus;
	DeviceExtend* tExtend = (DeviceExtend*)pDevObj->DeviceExtension;
	if(_MyControlDevice == tExtend->Type)	//Ӧ�ò�����豸
	{
		Debug("FsFilterWrite, _MyControlDevice");
		pIrp->IoStatus.Status		= STATUS_SUCCESS;
		pIrp->IoStatus.Information	= 0;
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		return STATUS_SUCCESS;
	}
	if(_FilterFsMasterDevice == tExtend->Type)	//�ļ�ϵͳ�����豸
	{
		Debug("FsFilterWrite, _FilterFsMasterDevice");
		IoSkipCurrentIrpStackLocation(pIrp);
		return IoCallDriver(tExtend->DirctLowDevObj, pIrp);
	}
	if(_FilterFsRollDevice == tExtend->Type)	//�ļ�ϵͳ���豸
	{
		return CommonDispatch(pDevObj, pIrp);
	}

	Debug("FsFilterRead error");		//error
	pIrp->IoStatus.Information	= 0;
	pIrp->IoStatus.Status		= STATUS_UNSUCCESSFUL;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	Debug("Leave FsFilterWrite");
	return STATUS_UNSUCCESSFUL;
}
//---------------------------------------------------------------
NTSTATUS FsFilterSetInformation(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp)
{
	Debug("Enter FsFilterWrite");
	NTSTATUS tRetStatus;
	DeviceExtend* tExtend = (DeviceExtend*)pDevObj->DeviceExtension;
	if(_MyControlDevice == tExtend->Type)	//Ӧ�ò�����豸
	{
		Debug("FsFilterWrite, _MyControlDevice");
		pIrp->IoStatus.Status		= STATUS_SUCCESS;
		pIrp->IoStatus.Information	= 0;
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		return STATUS_SUCCESS;
	}
	if(_FilterFsMasterDevice == tExtend->Type)	//�ļ�ϵͳ�����豸
	{
		Debug("FsFilterWrite, _FilterFsMasterDevice");
		IoSkipCurrentIrpStackLocation(pIrp);
		return IoCallDriver(tExtend->DirctLowDevObj, pIrp);
	}
	if(_FilterFsRollDevice == tExtend->Type)	//�ļ�ϵͳ���豸
	{
		PIO_STACK_LOCATION tIoStack = IoGetCurrentIrpStackLocation(pIrp);
		if(FileDispositionInformation == tIoStack->Parameters.SetFile.FileInformationClass) //shift delete
		{
			FILE_DISPOSITION_INFORMATION* tFileDelInfo = (FILE_DISPOSITION_INFORMATION*)pIrp->AssociatedIrp.SystemBuffer;
			if(TRUE == tFileDelInfo->DeleteFile)
			{
				//��ֹ
				Debug("��ֹ�ļ�ɾ��");
				pIrp->IoStatus.Information	= 0;
				pIrp->IoStatus.Status		= STATUS_UNSUCCESSFUL;
				IoCompleteRequest(pIrp, IO_NO_INCREMENT);
				return STATUS_UNSUCCESSFUL;
			}
		}
		
		if(FileRenameInformation == tIoStack->Parameters.SetFile.FileInformationClass) //delete ����վ
		{
			Debug("��ֹ�ļ�ɾ��������վ");
			pIrp->IoStatus.Information	= 0;
			pIrp->IoStatus.Status		= STATUS_UNSUCCESSFUL;
			IoCompleteRequest(pIrp, IO_NO_INCREMENT);
			return STATUS_UNSUCCESSFUL;

		/*
			FILE_DISPOSITION_INFORMATION* tFileDelInfo = (FILE_DISPOSITION_INFORMATION*)pIrp->AssociatedIrp.SystemBuffer;
					if(TRUE == tFileDelInfo->DeleteFile)
					{
						//��ֹ
						Debug("��ֹ�ļ�ɾ��������վ");
						pIrp->IoStatus.Information	= 0;
						pIrp->IoStatus.Status		= STATUS_UNSUCCESSFUL;
						IoCompleteRequest(pIrp, IO_NO_INCREMENT);
						return STATUS_UNSUCCESSFUL;
					}*/
		
		}


		IoSkipCurrentIrpStackLocation(pIrp);
		return IoCallDriver(tExtend->DirctLowDevObj, pIrp);
	}

	Debug("FsFilterRead error");		//error
	pIrp->IoStatus.Information	= 0;
	pIrp->IoStatus.Status		= STATUS_UNSUCCESSFUL;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	Debug("Leave FsFilterWrite");
	return STATUS_UNSUCCESSFUL;
}
//---------------------------------------------------------------
extern "C" NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject,IN PUNICODE_STRING pRegistryPath) 
{
	//��ʼ����ǲ����
	for(int i=0; i<IRP_MJ_MAXIMUM_FUNCTION; i++)
	{
		pDriverObject->MajorFunction[i] = CommonDispatch;
	}
	pDriverObject->MajorFunction[IRP_MJ_CREATE]						= FsFilterCreate;
	pDriverObject->MajorFunction[IRP_MJ_CLOSE]						= FsFilterClose;
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]				= FsFilterDeviceControl;
	pDriverObject->MajorFunction[IRP_MJ_FILE_SYSTEM_CONTROL]		= FsFilterFileSystemControl;
	pDriverObject->MajorFunction[IRP_MJ_READ]						= FsFilterRead;
	pDriverObject->MajorFunction[IRP_MJ_WRITE]						= FsFilterWrite;
	pDriverObject->MajorFunction[IRP_MJ_SET_INFORMATION]			= FsFilterSetInformation;

	pDriverObject->DriverUnload	= DriverUnload;

	//ע�����IO
	InitFastIo(pDriverObject);

	//����ȫ����������
	gMyDriverObject	= pDriverObject;

	//ע���ļ�ϵͳ�ص�
	NTSTATUS	tRetStatus;
	tRetStatus	= IoRegisterFsRegistrationChange(pDriverObject,DriverNotificationRoutine);
	if(false == NT_SUCCESS(tRetStatus))
	{
		DestoryFastIo(pDriverObject);
		return STATUS_UNSUCCESSFUL;
	}

	//����Ӧ�ó�������豸
	//...
	
	return STATUS_SUCCESS;
}
//---------------------------------------------------------------

