/************************************************************************
* �ļ�����:Driver.cpp                                                 
* ��    ��:�ŷ�
* �������:2007-11-1
*************************************************************************/

#include "Driver.h"


// add by hc in 2013/08/13
void MyFileTest()
{
	OBJECT_ATTRIBUTES tFileAttri;
	HANDLE tFile = 0;
	IO_STATUS_BLOCK tIoStatus;
	UNICODE_STRING tFileNameUnicode;
	RtlInitUnicodeString(&tFileNameUnicode,L"\\??\\C:\\1.txt");

	InitializeObjectAttributes(&tFileAttri,&tFileNameUnicode,OBJ_CASE_INSENSITIVE,NULL,NULL);

	NTSTATUS tStatus = ZwCreateFile(
		&tFile,
		GENERIC_WRITE|GENERIC_READ,
		&tFileAttri,
		&tIoStatus,
		0,
		FILE_ATTRIBUTE_NORMAL,
		FILE_SHARE_READ|FILE_SHARE_WRITE,
		FILE_OPEN_IF,
		FILE_SYNCHRONOUS_IO_NONALERT,
		0,
		0);

	if(NT_SUCCESS(tStatus))
		KdPrint(("CreateFile success\n"));
	else
		KdPrint(("CreateFile fail\n"));

	//д�ļ�
/*
	IO_STATUS_BLOCK tStatusWrite;
	char tBuf[64] = {"i am ZwWriteFile\n"};
	int tBufLen = sizeof(tBuf);
	NTSTATUS tRetStatus = ZwWriteFile(tFile,0,0,0,&tStatusWrite,tBuf,tBufLen,0,0);
	if(NT_SUCCESS(tRetStatus))
		KdPrint(("ZwWriteFile success\n"));
	else
		KdPrint(("ZwWriteFile fail\n"));*/


	//���ļ�
	IO_STATUS_BLOCK tStatusRead;
	char tBuf[64];

	memset(tBuf,0,sizeof(tBuf));
	NTSTATUS tRetStatus = ZwReadFile(tFile,0,0,0,&tStatusRead,tBuf,sizeof(tBuf),0,0);
	if(NT_SUCCESS(tRetStatus))
	{
		KdPrint(("ZwReadFile success\n"));
		KdPrint((tBuf));
	}
	else
		KdPrint(("ZwReadFile fail\n"));

	ZwClose(tFile);
}

/************************************************************************
* ��������:DriverEntry
* ��������:��ʼ���������򣬶�λ������Ӳ����Դ�������ں˶���
* �����б�:
      pDriverObject:��I/O�������д���������������
      pRegistryPath:����������ע�����е�·��
* ���� ֵ:���س�ʼ������״̬
*************************************************************************/

#pragma INITCODE
extern "C" NTSTATUS DriverEntry (
			IN PDRIVER_OBJECT pDriverObject,
			IN PUNICODE_STRING pRegistryPath	) 
{
	NTSTATUS status = STATUS_SUCCESS;
	KdPrint(("Enter DriverEntry\n"));

	//ע�������������ú������
	pDriverObject->DriverUnload = HelloDDKUnload;
	pDriverObject->MajorFunction[IRP_MJ_CREATE] = HelloDDKDispatchRoutine;
	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = HelloDDKDispatchRoutine;
	pDriverObject->MajorFunction[IRP_MJ_WRITE] = HelloDDKDispatchRoutine;
	pDriverObject->MajorFunction[IRP_MJ_READ] = HelloDDKDispatchRoutine;
	
	//���������豸����
	status = CreateDevice(pDriverObject);

	UNICODE_STRING devName;
	RtlInitUnicodeString(&devName,L"\\Device\\MyDDKDevice");//ǳ����
	UNICODE_STRING tTestUnicode;
	RtlInitUnicodeString(&tTestUnicode,devName.Buffer);

	KdPrint(("DriverEntry end\n"));
	//MyFileTest();
	return status;
}

/************************************************************************
* ��������:CreateDevice
* ��������:��ʼ���豸����
* �����б�:
      pDriverObject:��I/O�������д���������������
* ���� ֵ:���س�ʼ��״̬
*************************************************************************/
#pragma INITCODE
NTSTATUS CreateDevice(
		IN PDRIVER_OBJECT	pDriverObject) 
{
	NTSTATUS status;
	PDEVICE_OBJECT pDevObj;
	PDEVICE_EXTENSION pDevExt;
	
	//�����豸����
	UNICODE_STRING devName;
	RtlInitUnicodeString(&devName,L"\\Device\\MyDDKDevice");
	
	//�����豸
	status = IoCreateDevice( pDriverObject,
						sizeof(DEVICE_EXTENSION),
						&(UNICODE_STRING)devName,
						FILE_DEVICE_UNKNOWN,
						0, TRUE,
						&pDevObj);
	if (!NT_SUCCESS(status))
		return status;

	pDevObj->Flags |= DO_BUFFERED_IO;
	pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;
	pDevExt->pDevice = pDevObj;
	pDevExt->ustrDeviceName = devName;
	//������������
	UNICODE_STRING symLinkName;
	RtlInitUnicodeString(&symLinkName,L"\\??\\X:");
	pDevExt->ustrSymLinkName = symLinkName;
	status = IoCreateSymbolicLink( &symLinkName,&devName );
	if (!NT_SUCCESS(status)) 
	{
		IoDeleteDevice( pDevObj );
		return status;
	}
	return STATUS_SUCCESS;
}

/************************************************************************
* ��������:HelloDDKUnload
* ��������:�������������ж�ز���
* �����б�:
      pDriverObject:��������
* ���� ֵ:����״̬
*************************************************************************/
#pragma PAGEDCODE
VOID HelloDDKUnload (IN PDRIVER_OBJECT pDriverObject) 
{
	PDEVICE_OBJECT	pNextObj;
	KdPrint(("Enter DriverUnload\n"));
	pNextObj = pDriverObject->DeviceObject;
	while (pNextObj != NULL) 
	{
		PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)
			pNextObj->DeviceExtension;

		//ɾ����������
		UNICODE_STRING pLinkName = pDevExt->ustrSymLinkName;
		IoDeleteSymbolicLink(&pLinkName);
		pNextObj = pNextObj->NextDevice;
		IoDeleteDevice( pDevExt->pDevice );
	}
}

/************************************************************************
* ��������:HelloDDKDispatchRoutine
* ��������:�Զ�IRP���д���
* �����б�:
      pDevObj:�����豸����
      pIrp:��IO�����
* ���� ֵ:����״̬
*************************************************************************/
#pragma PAGEDCODE
NTSTATUS HelloDDKDispatchRoutine(IN PDEVICE_OBJECT pDevObj,
								 IN PIRP pIrp) 
{
	KdPrint(("Enter HelloDDKDispatchRoutine\n"));
	NTSTATUS status = STATUS_SUCCESS;
	// ���IRP
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = 0;	// bytes xfered
	IoCompleteRequest( pIrp, IO_NO_INCREMENT );
	KdPrint(("Leave HelloDDKDispatchRoutine\n"));

	return status;
}
