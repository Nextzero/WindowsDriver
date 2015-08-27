#ifndef MC_VirtualDiscH
#define MC_VirtualDiscH

#include "SysMain.h"

class MC_VirtualDisc
{
public:
	//为什么不能申明默认构造函数

	bool		Init(IN PDRIVER_OBJECT vDriverObject);
	void		UnInit(IN PDRIVER_OBJECT vDriverObject);
	
	NTSTATUS	DriverReadWrite(IN PDEVICE_OBJECT vDeviceObject, IN PIRP vIrp);
	NTSTATUS	DriverControl(IN PDEVICE_OBJECT vDeviceObject, IN PIRP vIrp);
	NTSTATUS	DriverClose(IN PDEVICE_OBJECT vDeviceObject, IN PIRP vIrp);

	static void ThreadProc(void * vThis);
	void DealIRPList();

private:
	bool		Open();
	bool		Read(PIO_STATUS_BLOCK vIoStatus,char * vBuf, ULONG vLen, LONGLONG vOffset);
	bool		Write(PIO_STATUS_BLOCK vIoStatus,char * vBuf, ULONG vLen, LONGLONG vOffset);

private:
	KSPIN_LOCK	m_SpinLock;
	KEVENT		m_EventWakeUpThread;
	LIST_ENTRY	m_IrpList;//pIrp		= CONTAINING_RECORD(Request, IRP, Tail.Overlay.ListEntry);
	HANDLE		m_ThreadAddr;

	bool		m_Exit;
	HANDLE		m_FileHandle;
};

extern MC_VirtualDisc MyVirtualDisc;

#endif

