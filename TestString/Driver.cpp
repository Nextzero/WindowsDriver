#include "Driver.h"

void test()
{
	NTSTATUS tRetStatus;

	KdPrint(("enter teest\n"));

	KdPrint(("WCHAR and CHAR"));
	CHAR tSrc[]		= "abcd";
	CHAR tDes[10]	= {0};
	strcpy(tDes,tSrc);
	KdPrint(("%s\n",tDes));

	WCHAR tWSrc[]	= L"abcde";
	WCHAR tWDes[10]	= {0};
	wcscpy(tWDes,tWSrc);
	KdPrint(("%S\n",tWDes));

	KdPrint(("\n"));

	KdPrint(("ANSI_STRING and UNICODE_STRING\n"));
	ANSI_STRING tAnsiStringSrc;
	KdPrint(("printf ansi string(no init):%Z\n",&tAnsiStringSrc));

	UNICODE_STRING tUnicodeStringSrc;
	KdPrint(("printf Unicode string(no init):%wZ\n",&tUnicodeStringSrc));

	//RtlInitAnsiString()简单指针赋值
	tUnicodeStringSrc.MaximumLength	= 100;
	tUnicodeStringSrc.Buffer = (PWSTR)ExAllocatePool(PagedPool, tUnicodeStringSrc.MaximumLength);
	WCHAR tWTemp[] = L"example unicodestring";
	tUnicodeStringSrc.Length	= wcslen(tWTemp) * 2;	//单位字节 
	RtlCopyMemory(tUnicodeStringSrc.Buffer, tWTemp, tUnicodeStringSrc.Length);	//字节
	KdPrint(("length:%d,maxlen:%d,str:%wZ\n",tUnicodeStringSrc.Length, tUnicodeStringSrc.MaximumLength, &tUnicodeStringSrc));
	
	//RtlCopyUnicodeString()和RtlCopyMemory相似，目的缓存需要自己分配,自己清空
	KdPrint(("compare string\n"));

	UNICODE_STRING tCompareStr;
	RtlInitUnicodeString(&tCompareStr,tWTemp);
	KdPrint(("length:%d,maxlen:%d,str:%wZ\n",tCompareStr.Length, tCompareStr.MaximumLength, &tCompareStr));

	if(0 == RtlCompareUnicodeString(&tUnicodeStringSrc, &tCompareStr, FALSE))	//maxlength不同，不影响比较结果
		KdPrint(("equal\n"));
	else
		KdPrint(("not equal\n"));

	ExFreePool(tUnicodeStringSrc.Buffer);
	tUnicodeStringSrc.Buffer		= 0;
	tUnicodeStringSrc.Length		= 0;
	tUnicodeStringSrc.MaximumLength	= 0;

	KdPrint(("to up\n"));
	UNICODE_STRING	tUpCaseString;
	tRetStatus = RtlUpcaseUnicodeString(&tUpCaseString, &tUnicodeStringSrc, TRUE);
	if(false == NT_SUCCESS(tRetStatus))
		KdPrint(("UpcaseUnicodestring failed\n"));

	KdPrint(("%wZ\n",&tUpCaseString));
	RtlFreeUnicodeString(&tUpCaseString);

	KdPrint(("oh on?\n"));
	UNICODE_STRING tTestUnicodeString;
	RtlInitUnicodeString(&tTestUnicodeString, L"aabbccdd");
	KdPrint(("%wZ\n",&tTestUnicodeString));
	RtlUpcaseUnicodeString(&tTestUnicodeString,&tTestUnicodeString,FALSE);
	KdPrint(("%wZ\n",&tTestUnicodeString));

	KdPrint(("string to int\n"));
	UNICODE_STRING tString;
	RtlInitUnicodeString(&tString, L"123");
	ULONG t = 0;
	RtlUnicodeStringToInteger(&tString, 10, &t);
	t = t+1;
	KdPrint(("convert to int:%d\n",t));

	KdPrint(("level test\n"));
}


VOID HelloDDKUnload (IN PDRIVER_OBJECT pDriverObject) 
{
	
}

NTSTATUS HelloDDKDispatchRoutine(IN PDEVICE_OBJECT pDevObj,
								 IN PIRP pIrp) 
{
	KdPrint(("Enter HelloDDKDispatchRoutine\n"));
	NTSTATUS status = STATUS_SUCCESS;
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = 0;	
	IoCompleteRequest( pIrp, IO_NO_INCREMENT );
	KdPrint(("Leave HelloDDKDispatchRoutine\n"));
	return status;
}

extern "C" NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject,IN PUNICODE_STRING pRegistryPath) 
{
	NTSTATUS status = STATUS_SUCCESS;
	for(int i=0; i<IRP_MJ_MAXIMUM_FUNCTION; i++)
		pDriverObject->MajorFunction[i] = HelloDDKDispatchRoutine;
	pDriverObject->DriverUnload	= HelloDDKUnload;
	test();
	return status;
}

