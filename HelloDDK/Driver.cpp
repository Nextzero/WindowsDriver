/************************************************************************
* �ļ�����:Driver.cpp                                                 
* ��    ��:�ŷ�
* �������:2007-11-1
*************************************************************************/

#include "Driver.h"

/*Add by CZHC 2013/8/3 13:12*/
#pragma PAGEDCODE
void TestLookaside()
{
	KdPrint(("enter TestLookaside ......\n"));
	PAGED_LOOKASIDE_LIST tLookaside;
	ExInitializePagedLookasideList(&tLookaside,NULL,NULL,0,sizeof(MyListNode),'1234',0);

	MyListNode* MyList[10] = {0};
	for(int i=0; i<10; i++)
	{
		MyList[i] = (MyListNode*)ExAllocateFromPagedLookasideList(&tLookaside);
		if(MyList[i] == NULL)
		{
			KdPrint(("i fuck ..\n"));
			continue;
		}
		MyList[i]->data = i;
	}

	for(int k=0; k<10; k++)
	{
		if(MyList[k] == NULL)
		{
			KdPrint(("i fuck too \n"));
			continue;
		}
		KdPrint(("%d   ",MyList[k]->data));
		ExFreeToPagedLookasideList(&tLookaside,MyList[k]);
		MyList[k] = NULL;
	}

	ExDeletePagedLookasideList(&tLookaside);
	
	KdPrint(("leave TestLookaside ......\n"));

}

/*Add by CZHC 2013/8/1 21:54*/
#pragma PAGEDCODE
void TestList()
{
	LIST_ENTRY tHead;
	InitializeListHead(&tHead);

	MyListNode* pData;
	ULONG i=0;

	KSPIN_LOCK tLock;
	KeInitializeSpinLock(&tLock);

	KdPrint(("begin insert data to list \n"));
	for(i=0; i<10; i++)
	{
		KdPrint(("insert one %d\n",i));
		pData = (MyListNode*)ExAllocatePool(PagedPool,sizeof(MyListNode));
		pData->data = i;
		ExInterlockedInsertTailList(&tHead,&pData->ListEntry,&tLock);
	}

	KdPrint(("begin remove from link and print"));
	while(!IsListEmpty(&tHead))
	{
		MyListNode* pCurNode = (MyListNode*)ExInterlockedRemoveHeadList(&tHead,&tLock);
		KdPrint(("%d  ",pCurNode->data));
		ExFreePool(pCurNode);
	}
}

/*Add by CZHC 2013/8/1 20:20*/
#pragma PAGEDCODE
void PrintInfo(IN PDRIVER_OBJECT pDriverObject)
{
	KdPrint(("------------------------------\n"));
	KdPrint(("Enter PrintInfo\n"));
	NTSTATUS tStatus;
	PDEVICE_OBJECT pDevice = pDriverObject->DeviceObject;
	int i = 0;
	for(; pDevice != NULL; pDevice = pDevice->NextDevice)
	{
		KdPrint(("The %d Device\n",i++));
		KdPrint(("Device AttachDevice:0x%08X\n",pDevice->AttachedDevice));
		KdPrint(("Next Device:0x%08X\n",pDevice->NextDevice));
		KdPrint(("Device stacksize:%d\n",pDevice->StackSize));
		KdPrint(("Device's DriverObject 0X%08X\n",pDevice->DriverObject));
	}
	KdPrint(("Leaver PrintInfo\n"));
	KdPrint(("------------------------------\n"));

	KdPrint(("------------------------------\n"));
	PEPROCESS pEProcess = PsGetCurrentProcess();
	PTSTR ProcessName = (PTSTR)((ULONG)pEProcess + 0x174);
	KdPrint(("%s\n",ProcessName));
	KdPrint(("------------------------------\n"));
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

	PrintInfo(pDriverObject);
	KdPrint(("DriverEntry end\n"));

	TestList();
	TestLookaside();
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
	RtlInitUnicodeString(&symLinkName,L"\\??\\HelloDDK");
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