#include "MC_VirtualDisc.h"

MC_VirtualDisc MyVirtualDisc;

void MC_VirtualDisc::ThreadProc(void * vThis)
{
	MC_VirtualDisc* tThis = (MC_VirtualDisc*)vThis;
	tThis->DealIRPList();
	return ;
}

bool MC_VirtualDisc::Init(IN PDRIVER_OBJECT vDriverObject)//完成设备对象创建，符号链接创建
{
	Debug("Enter MC_VirtualDisc::Init");
	
	//初始化一些数据
	m_ThreadAddr	= 0;
	m_Exit			= false;

	//创建设备对象
	UNICODE_STRING tDeviceName;
	RtlInitUnicodeString(&tDeviceName,DEVICE_NAME);

	PDEVICE_OBJECT tDevice;// FILE_DEVICE_DISK
	NTSTATUS tStatus = IoCreateDevice(
		vDriverObject,
		sizeof(MY_DEVOBJ_EXTENSION),
		&tDeviceName,
		FILE_DEVICE_DISK,
		FILE_REMOVABLE_MEDIA,
		false,
		&tDevice);

	if(false == NT_SUCCESS(tStatus))
	{
		Debug("MC_VirtualDisc::Init fail: IoCreateDevice");
		Debug("Leave MC_VirtualDisc::Init");
		return false;
	}

	//创建符号链接
	UNICODE_STRING tSymbolicLink;
	RtlInitUnicodeString(&tSymbolicLink,DEVICE_SYMBOLICLINK);
	tStatus = IoCreateSymbolicLink(&tSymbolicLink,&tDeviceName);
	if(false == NT_SUCCESS(tStatus))
	{
		IoDeleteDevice(tDevice);
		Debug("MC_VirtualDisc::Init fail: IoCreateSymbolicLink");
		Debug("Leave MC_VirtualDisc::Init");
		return false;
	}

	//初始化设备对象
	tDevice->Flags |= DO_DIRECT_IO;

	//初始化设备扩展
	MY_DEVOBJ_EXTENSION* tDeviceExtension = (MY_DEVOBJ_EXTENSION*)tDevice->DeviceExtension;

	//初始化其他数据，如自旋锁
	InitializeListHead(&m_IrpList);
	KeInitializeSpinLock(&m_SpinLock);
	KeInitializeEvent(&m_EventWakeUpThread,SynchronizationEvent,FALSE);

	//创建线程处理IRP List
	HANDLE tThread = 0;
	tStatus = PsCreateSystemThread(&tThread,(ACCESS_MASK)0L,0,0,0,ThreadProc,this);
	if(false == NT_SUCCESS(tStatus))
	{
		IoDeleteSymbolicLink(&tSymbolicLink);
		IoDeleteDevice(tDevice);
		Debug("MC_VirtualDisc::Init fail: PsCreateSystemThread");
		Debug("Leave MC_VirtualDisc::Init");
		return false;
	}
	//将句柄转换成实际对象指针
	tStatus = ObReferenceObjectByHandle(tThread, THREAD_ALL_ACCESS, 0, KernelMode, &m_ThreadAddr, 0);
	if(false == NT_SUCCESS(tStatus))
	{
		m_ThreadAddr = 0;
		ZwClose(tThread);
		IoDeleteSymbolicLink(&tSymbolicLink);
		IoDeleteDevice(tDevice);
		Debug("MC_VirtualDisc::Init fail: PsCreateSystemThread");
		Debug("Leave MC_VirtualDisc::Init");
		return false;
	}
	ZwClose(tThread);//hc:  能关掉吗？

	//创建数据文件
	HANDLE tFile = 0;
	OBJECT_ATTRIBUTES tFileAttri;
	UNICODE_STRING tFileName;
	IO_STATUS_BLOCK tIoStatus;
	RtlInitUnicodeString(&tFileName,FILE_NAME);
	InitializeObjectAttributes(&tFileAttri,&tFileName,0,0,0);
	tStatus = ZwCreateFile(&tFile,
		GENERIC_READ|GENERIC_WRITE|SYNCHRONIZE,
		&tFileAttri, &tIoStatus,
		NULL,NULL,
		0,
		FILE_OPEN_IF,/*之前为FILE_CREATE，导致打开文件失败进而返回错误，最后使得设备对象创建了但没有卸载*/
		FILE_NO_INTERMEDIATE_BUFFERING|FILE_SYNCHRONOUS_IO_NONALERT|FILE_NON_DIRECTORY_FILE|FILE_RANDOM_ACCESS,
		NULL,NULL);

	if(false == NT_SUCCESS(tStatus))
	{
		IoDeleteSymbolicLink(&tSymbolicLink);
		IoDeleteDevice(tDevice);
		Debug("MC_VirtualDisc::Init fail: ZwCreateFile fail");
		Debug("Leave MC_VirtualDisc::Init");
		return false;
	}
	ZwClose(tFile);
	Debug("Leave MC_VirtualDisc::Init");
	return true;
}

void MC_VirtualDisc::UnInit(IN PDRIVER_OBJECT vDriverObject)
{
	Debug("Enter MC_VirtualDisc::UnInit");
	UNICODE_STRING tSymbolicLink;
	RtlInitUnicodeString(&tSymbolicLink,DEVICE_SYMBOLICLINK);

	//回收设备对象与符号链接
	PDEVICE_OBJECT tDevice = (PDEVICE_OBJECT)vDriverObject->DeviceObject;
	while (0 != tDevice) 
	{
		PDEVICE_OBJECT tTemp = tDevice;
		tDevice = tDevice->NextDevice;
		IoDeleteSymbolicLink(&tSymbolicLink);
		IoDeleteDevice(tTemp);
	}

	//回收线程资源
	m_Exit = true;
	KeSetEvent(&m_EventWakeUpThread,IO_NO_INCREMENT,false);

	if(0 != m_ThreadAddr)
	{
		KeWaitForSingleObject(m_ThreadAddr,Executive,KernelMode,false,0);
		ObDereferenceObject(m_ThreadAddr);
		m_ThreadAddr = 0;
	}
	Debug("Leave MC_VirtualDisc::UnInit");
}

NTSTATUS MC_VirtualDisc::DriverReadWrite(IN PDEVICE_OBJECT vDeviceObject, IN PIRP vIrp)
{
	Debug("Enter MC_VirtualDisc::DriverReadWrite");

	//若IRP请求长度为0，则直接返回
	PIO_STACK_LOCATION	tStack = IoGetCurrentIrpStackLocation(vIrp); 
	if(0 == tStack->Parameters.Read.Length && 0 == tStack->Parameters.Write.Length)
	{
		vIrp->IoStatus.Information	= 0;
		vIrp->IoStatus.Status		= STATUS_SUCCESS;
		IoCompleteRequest(vIrp,IO_NO_INCREMENT);
		Debug("0 == tStack->Parameters.Read.Length && 0 == tStack->Parameters.Write.Length");
		Debug("Leave MC_VirtualDisc::DriverReadWrite");
		return STATUS_SUCCESS;
	}

	//添加到队列
	IoMarkIrpPending(vIrp);//挂起IRP
	ExInterlockedInsertTailList(&m_IrpList,&vIrp->Tail.Overlay.ListEntry,&m_SpinLock);
	KeSetEvent(&m_EventWakeUpThread,IO_NO_INCREMENT,FALSE);
	Debug("Leave MC_VirtualDisc::DriverReadWrite");
	return STATUS_PENDING;
}
NTSTATUS MC_VirtualDisc::DriverControl(IN PDEVICE_OBJECT vDeviceObject, IN PIRP vIrp)
{
	PIO_STACK_LOCATION tIoStack = IoGetCurrentIrpStackLocation(vIrp);
	ULONG tIoControlCode		= tIoStack->Parameters.DeviceIoControl.IoControlCode;
	ULONG tInputLength			= tIoStack->Parameters.DeviceIoControl.InputBufferLength;
	ULONG tOutputLength			= tIoStack->Parameters.DeviceIoControl.OutputBufferLength;
	void* tInputBuf				= vIrp->AssociatedIrp.SystemBuffer;//什么方式
	NTSTATUS tStatus			= STATUS_NOT_IMPLEMENTED;
	vIrp->IoStatus.Information	= 0;
	
	switch(tIoControlCode)
	{
	case IOCTL_DISK_CHECK_VERIFY:
	//case IOCTL_CDROM_CHECK_VERIFY:
	case IOCTL_STORAGE_CHECK_VERIFY:
	case IOCTL_STORAGE_CHECK_VERIFY2:
	case IOCTL_DISK_MEDIA_REMOVAL:
	case IOCTL_STORAGE_MEDIA_REMOVAL:
	//case IOCTL_CDROM_READ_TOC:
	case IOCTL_DISK_SET_PARTITION_INFO:
		{
			tStatus = STATUS_INVALID_DEVICE_REQUEST;
		}break;
	case IOCTL_DISK_GET_DRIVE_GEOMETRY://Returns information about the physical disk's geometry (media type, number of cylinders, tracks per cylinder, sectors per track, and bytes per sector).
		if(tOutputLength < sizeof(DISK_GEOMETRY))
			tStatus = STATUS_BUFFER_TOO_SMALL;
		else
		{
			PDISK_GEOMETRY DiskGeometry			= (PDISK_GEOMETRY)tInputBuf;
			DiskGeometry->MediaType				= FixedMedia;
			DiskGeometry->Cylinders.QuadPart	= DevSectorTotal;//总扇区数
			DiskGeometry->TracksPerCylinder		= 1;
			DiskGeometry->SectorsPerTrack		= 1;
			DiskGeometry->BytesPerSector		= 512;
			vIrp->IoStatus.Information			= sizeof(DISK_GEOMETRY);

			tStatus	= STATUS_SUCCESS;
		}break;
	case IOCTL_DISK_GET_LENGTH_INFO://Returns the length, in bytes, of the disk, partition, or volume associated with the device object that is the target of the request. 
		if(tOutputLength < sizeof(GET_LENGTH_INFORMATION))
			tStatus = STATUS_BUFFER_TOO_SMALL;
		else
		{
			PGET_LENGTH_INFORMATION tInfo	= (PGET_LENGTH_INFORMATION)tInputBuf;
			tInfo->Length.QuadPart			= 512*1024*1024;
			vIrp->IoStatus.Information		= sizeof(GET_LENGTH_INFORMATION);

			tStatus = STATUS_SUCCESS;
		}break;
	case IOCTL_DISK_GET_PARTITION_INFO://Returns information about the type, size, and nature of a disk partition. (Floppy drivers need not handle this request.)
		if(tOutputLength < sizeof(PARTITION_INFORMATION))
			tStatus = STATUS_BUFFER_TOO_SMALL;
		else
		{
			PPARTITION_INFORMATION partition_information = (PPARTITION_INFORMATION)tInputBuf;
			partition_information->StartingOffset.QuadPart = 0;
			partition_information->PartitionLength.QuadPart = (LONGLONG)DevSectorTotal*512;
			partition_information->HiddenSectors = 1;
			partition_information->PartitionNumber = 0;
			partition_information->PartitionType = 0;
			partition_information->BootIndicator = FALSE;
			partition_information->RecognizedPartition = FALSE;
			partition_information->RewritePartition = FALSE;

			vIrp->IoStatus.Information = sizeof(PARTITION_INFORMATION);
			tStatus = STATUS_SUCCESS;
		}break;
	case IOCTL_DISK_GET_PARTITION_INFO_EX://Returns information about the type, size, and nature of a disk partition. (Floppy drivers need not handle this request.)
		if(tOutputLength < sizeof(PARTITION_INFORMATION_EX))
			tStatus = STATUS_BUFFER_TOO_SMALL;
		else
		{
			PPARTITION_INFORMATION_EX partition_information_ex	= (PPARTITION_INFORMATION_EX)tInputBuf;
			partition_information_ex->PartitionStyle			= PARTITION_STYLE_MBR;
			partition_information_ex->StartingOffset.QuadPart	= 0;
			partition_information_ex->PartitionLength.QuadPart	= (LONGLONG)DevSectorTotal*512;
			partition_information_ex->PartitionNumber			= 0;
			partition_information_ex->RewritePartition			= FALSE;
			partition_information_ex->Mbr.HiddenSectors			= 1;
			partition_information_ex->Mbr.PartitionType			= 0;
			partition_information_ex->Mbr.BootIndicator			= FALSE;
			partition_information_ex->Mbr.RecognizedPartition	= FALSE;
			vIrp->IoStatus.Information							= sizeof(PARTITION_INFORMATION_EX);

			tStatus = STATUS_SUCCESS;
		}break;
	case IOCTL_DISK_VERIFY://Performs a logical format of a specified extent on a disk. 
		if(tInputLength < sizeof(VERIFY_INFORMATION))
			tStatus = STATUS_INVALID_PARAMETER;
		else
		{
			PVERIFY_INFORMATION tInfo = (PVERIFY_INFORMATION)tInputBuf;
			tStatus = STATUS_SUCCESS;
			vIrp->IoStatus.Information = tInfo->Length;
		}break;
	case IOCTL_DISK_IS_WRITABLE://Determines whether a disk is writable(可写)
		{
			tStatus = STATUS_SUCCESS;
			vIrp->IoStatus.Information = 0;
		}break;
	default:
		{
			Debug(("INFO: Unknow Control Code"));
		}break;
	}
	IoCompleteRequest(vIrp,IO_NO_INCREMENT);
	return tStatus;
}

NTSTATUS MC_VirtualDisc::DriverClose(IN PDEVICE_OBJECT vDeviceObject, IN PIRP vIrp)
{
	return STATUS_SUCCESS;
}

void MC_VirtualDisc::DealIRPList()
{
	Debug("Enter MC_VirtualDisc::DealIRPList");

	while(1)
	{
		NTSTATUS tStatus = KeWaitForSingleObject(&m_EventWakeUpThread,Executive,KernelMode,false,0);
		if(true == m_Exit)
		{
			PsTerminateSystemThread(STATUS_SUCCESS);
			return ;
		}
		if(false == NT_SUCCESS(tStatus))
			continue;//Wait函数发生其他情况

		PIO_STACK_LOCATION	tStack;
		PLIST_ENTRY			tListNode = 0;
		PIRP				tIrp;
		while(tListNode = ExInterlockedRemoveHeadList(&m_IrpList,&m_SpinLock))
		{
			tIrp = CONTAINING_RECORD(tListNode, IRP, Tail.Overlay.ListEntry);
			tStack = IoGetCurrentIrpStackLocation(tIrp);

			bool tIsFail = false;
			switch(tStack->MajorFunction)
			{
			case IRP_MJ_READ:
				{
					if(false == Read(
						&tIrp->IoStatus,
						(char*)MmGetSystemAddressForMdlSafe(tIrp->MdlAddress,NormalPagePriority),
						tStack->Parameters.Read.Length,
						tStack->Parameters.Read.ByteOffset.QuadPart))
					{
						tIsFail = true;
					}
				}break;
			case IRP_MJ_WRITE:
				{
					if(false == Write(
						&tIrp->IoStatus,
						(char*)MmGetSystemAddressForMdlSafe(tIrp->MdlAddress,NormalPagePriority),
						tStack->Parameters.Write.Length,
						tStack->Parameters.Write.ByteOffset.QuadPart))
					{
						tIsFail = true;
					}
				}break;
			default:
				{
					tIrp->IoStatus.Status		= STATUS_DRIVER_INTERNAL_ERROR;
					tIrp->IoStatus.Information	= 0;
				}break;
			}
			IoCompleteRequest(tIrp,IO_NO_INCREMENT);
			if(true == tIsFail)
				break;//break以后，链表中还留有数据怎么办，可能造成事件不被触发而线程一直等待下去
		}
	}
}

bool MC_VirtualDisc::Open()
{
	UNICODE_STRING tFileName;
	RtlInitUnicodeString(&tFileName,FILE_NAME);

	OBJECT_ATTRIBUTES tFileAttri;
	InitializeObjectAttributes(&tFileAttri,&tFileName,OBJ_CASE_INSENSITIVE,0,0);

	HANDLE tFile = 0;
	IO_STATUS_BLOCK tIoStatus;
	NTSTATUS tStatus = ZwOpenFile(
		&tFile,
		GENERIC_READ|GENERIC_WRITE,
		&tFileAttri,
		&tIoStatus,
		0,
		FILE_NO_INTERMEDIATE_BUFFERING|FILE_SYNCHRONOUS_IO_NONALERT|FILE_NON_DIRECTORY_FILE|FILE_RANDOM_ACCESS);

	if(false == NT_SUCCESS(tStatus))
	{
		m_FileHandle = 0;
		return false;
	}

	m_FileHandle = tFile;
	return true;
}
bool MC_VirtualDisc::Read(PIO_STATUS_BLOCK vIoStatus,char * vBuf, ULONG vLen, LONGLONG vOffset)
{
	Debug("Enter MC_VirtualDisc::Read");
	if(false == Open())
	{
		Debug("Open fail");
		vIoStatus->Information	= 0;
		vIoStatus->Status		= STATUS_UNSUCCESSFUL;
		return false;
	}

	//读取文件
	NTSTATUS tStatus = ZwReadFile(m_FileHandle,0,0,0,vIoStatus,vBuf,vLen,(PLARGE_INTEGER)&vOffset,0);
	if(false == NT_SUCCESS(tStatus))
	{
		Debug("Read fail");
		vIoStatus->Information	= 0;
		vIoStatus->Status		= STATUS_UNSUCCESSFUL;
		ZwClose(m_FileHandle);
		return false;
	}
	ZwClose(m_FileHandle);
	return true;
}
bool MC_VirtualDisc::Write(PIO_STATUS_BLOCK vIoStatus,char * vBuf, ULONG vLen, LONGLONG vOffset)
{
	Debug("Enter MC_VirtualDisc::Write");
	if(false == Open())
	{
		Debug("Open fail");
		vIoStatus->Information	= 0;
		vIoStatus->Status		= STATUS_UNSUCCESSFUL;
		return false;
	}

	//读取文件
	NTSTATUS tStatus = ZwWriteFile(m_FileHandle,0,0,0,vIoStatus,vBuf,vLen,(PLARGE_INTEGER)&vOffset,0);
	if(false == NT_SUCCESS(tStatus))
	{
		Debug("Write fail");
		vIoStatus->Information	= 0;
		vIoStatus->Status		= STATUS_UNSUCCESSFUL;
		ZwClose(m_FileHandle);
		return false;
	}
	ZwClose(m_FileHandle);
	return true;
}
