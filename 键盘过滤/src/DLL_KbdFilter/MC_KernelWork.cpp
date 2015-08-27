#include "MC_KernelWork.h"

MC_KernelWork::MC_KernelWork(void)
{
	m_Avaliable		= false;
	m_ShareEvent	= 0;
	m_Thread		= 0;
	m_CurStatus		= KBDERROR;
}

MC_KernelWork::~MC_KernelWork(void)
{
	m_Avaliable		= false;
	m_ShareEvent	= 0;
	m_Thread		= 0;
	m_CurStatus		= KBDERROR;
}

bool MC_KernelWork::OnLoad()
{
	if(true == m_Avaliable)
		return true;
	
	m_Avaliable = false;
	if( !LoadNTDriverA(SERVER_NAME_KBDFILTER,DEVICE_NAME_KBDFILTER_FILENAME) )
	{
		OutputDebugStringA("LoadDriver Fail");
		return false;
	}
	
	//缓冲区
	m_MakeCode.clear();
	m_SpinLock = FALSE;

	//通信
	m_ShareEvent = CreateEvent(0,FALSE,FALSE,0);
	if(0 == m_ShareEvent)
	{
		UnloadNTDriverA(SERVER_NAME_KBDFILTER);
		return false;
	}
	m_Thread = CreateThread(0,0,ThreadProc,this,0,0);
	if(0 == m_Thread)
	{
		CloseHandle(m_ShareEvent);
		UnloadNTDriverA(SERVER_NAME_KBDFILTER);
		return false;
	}
	m_ThreadTerminate = false;
	
	m_Avaliable = true;
	GetStatus(); //赋值m_CurStatus
	return true;
}

void MC_KernelWork::OnExit()
{
	if(false == m_Avaliable)
		return ;

	m_SpinLock			= FALSE;
	m_ThreadTerminate	= true;
	if(0 != m_Thread)	
	{
		DWORD tWaitRet = WaitForSingleObject(m_Thread,1000*3);
		if(WAIT_TIMEOUT == tWaitRet || WAIT_FAILED == tWaitRet)
		{
			TerminateThread(m_Thread,0);
			OutputDebugStringA("TerminateThread");
		}
		CloseHandle(m_Thread);
	}
	if(0 != m_ShareEvent)
	{
		CloseHandle(m_ShareEvent);
	}

	UnloadNTDriverA(SERVER_NAME_KBDFILTER);

	m_Avaliable = false;
	return ;
}

int MC_KernelWork::Enable()
{
	if(false == m_Avaliable)
		return KBDERROR;

	if(m_CurStatus & ENABLE)
		return 0;

	HANDLE tFilterHandle = CreateFile(DEVICE_NAME_KBDFILTER_API,FILE_ALL_ACCESS,0,0,OPEN_EXISTING,0,0);
	if(INVALID_HANDLE_VALUE == tFilterHandle)
	{
		OutputDebugStringA("CreateFile fail");
		return KBDERROR;
	}

	DWORD	tLen = 0;
	if(FALSE == DeviceIoControl(tFilterHandle,IOCTL_KBDFILTER_ENABLE,0,0,&m_CurStatus,sizeof(m_CurStatus),&tLen,0))
	{
		CloseHandle(tFilterHandle);
		return KBDERROR;
	}
	CloseHandle(tFilterHandle);
	if(m_CurStatus & ENABLE)
		return 0;

	return KBDERROR;
}

int MC_KernelWork::Disabled()
{
	if(false == m_Avaliable)
		return KBDERROR;

	if(!(m_CurStatus & ENABLE))
		return 0;

	HANDLE tFilterHandle = CreateFile(DEVICE_NAME_KBDFILTER_API,FILE_ALL_ACCESS,0,0,OPEN_EXISTING,0,0);
	if(INVALID_HANDLE_VALUE == tFilterHandle)
	{
		OutputDebugStringA("CreateFile fail");
		return KBDERROR;
	}

	DWORD	tLen = 0;
	if(FALSE == DeviceIoControl(tFilterHandle,IOCTL_KBDFILTER_DISABLE,0,0,&m_CurStatus,sizeof(m_CurStatus),&tLen,0))
	{
		CloseHandle(tFilterHandle);
		return false;
	}
	CloseHandle(tFilterHandle);
	
	if(!(m_CurStatus & ENABLE))
		return 0;

	return KBDERROR;
}

int MC_KernelWork::GetStatus()
{
	if(false == m_Avaliable)
		return KBDERROR;

	m_CurStatus = KBDERROR;
	HANDLE tFilterHandle = CreateFile(DEVICE_NAME_KBDFILTER_API,FILE_ALL_ACCESS,0,0,OPEN_EXISTING,0,0);
	if(INVALID_HANDLE_VALUE == tFilterHandle)
	{
		OutputDebugStringA("CreateFile fail");
		return m_CurStatus;
	}

	DWORD	tLen = 0;
	if(FALSE == DeviceIoControl(tFilterHandle,IOCTL_KBDFILTER_GETSTATUS,0,0,&m_CurStatus,sizeof(m_CurStatus),&tLen,0))
	{
		CloseHandle(tFilterHandle);
		return m_CurStatus;
	}
	CloseHandle(tFilterHandle);
	return m_CurStatus;
}

int MC_KernelWork::Intercept(unsigned short* tBuf, int* tLen)
{
	if(false == m_Avaliable)
		return KBDERROR;

	if(0 == tBuf || 0 == tLen)
		return KBDERROR;
	
	if(!(m_CurStatus & INTERCEPT))
	{
		HANDLE tFilterHandle = CreateFile(DEVICE_NAME_KBDFILTER_API,FILE_ALL_ACCESS,0,0,OPEN_EXISTING,0,0);
		if(INVALID_HANDLE_VALUE == tFilterHandle)
		{
			OutputDebugStringA("CreateFile fail");
			return m_CurStatus;
		}

		DWORD	tLen = 0;
		if(FALSE == DeviceIoControl(tFilterHandle,IOCTL_KBDFILTER_INTERCEPT,0,0,&m_CurStatus,sizeof(m_CurStatus),&tLen,0))
		{
			CloseHandle(tFilterHandle);
			return m_CurStatus;
		}
		CloseHandle(tFilterHandle);
	}

	//锁
	OutputDebugStringA("mark\n");
	printf("mark");
	while(InterlockedExchange(&m_SpinLock,TRUE) == TRUE)
	{
		Sleep(0);
	}	
	int i = 0;
	for( ; i<*tLen && !m_MakeCode.empty(); i++)
	{
		unsigned short temp = m_MakeCode.front();
		printf("front:%x\n",temp);
		tBuf[i] = m_MakeCode.front();
		m_MakeCode.pop_front();
	}
	InterlockedExchange(&m_SpinLock,FALSE);

	*tLen = i;
	return m_MakeCode.size(); //返回缓冲区内剩余字符数
}

int MC_KernelWork::CancelIntercept()
{
	if(false == m_Avaliable)
		return KBDERROR;

	if(!(m_CurStatus & INTERCEPT))
		return 0;

	HANDLE tFilterHandle = CreateFile(DEVICE_NAME_KBDFILTER_API,FILE_ALL_ACCESS,0,0,OPEN_EXISTING,0,0);
	if(INVALID_HANDLE_VALUE == tFilterHandle)
	{
		OutputDebugStringA("CreateFile fail");
		return KBDERROR;
	}

	DWORD	tLen = 0;
	if(FALSE == DeviceIoControl(tFilterHandle,IOCTL_KBDFILTER_CANCELINTERCEPT,0,0,&m_CurStatus,sizeof(m_CurStatus),&tLen,0))
	{
		CloseHandle(tFilterHandle);
		return KBDERROR;
	}
	CloseHandle(tFilterHandle);
	if(!(m_CurStatus & INTERCEPT))
		return 0;

	return KBDERROR;
}

int MC_KernelWork::KbdThreadProc()
{
	// share event 
	HANDLE tFilterHandle = CreateFile(DEVICE_NAME_KBDFILTER_API,FILE_ALL_ACCESS,0,0,OPEN_EXISTING,0,0);
	if(INVALID_HANDLE_VALUE == tFilterHandle)
	{
		OutputDebugStringA("CreateFile fail");
		return KBDERROR;
	}

	DWORD	tLen = 0;
	if(FALSE == DeviceIoControl(tFilterHandle,IOCTL_KBDFILTER_SHAREEVENT,&m_ShareEvent,sizeof(m_ShareEvent),&m_CurStatus,sizeof(m_CurStatus),&tLen,0))
	{
		CloseHandle(tFilterHandle);
		return KBDERROR;
	}

	//loop
	while(1)
	{
		if(true == m_ThreadTerminate)
		{
			CloseHandle(tFilterHandle);
			OutputDebugStringA("thread exit success\n");
			return 0;
		}

		DWORD tWaitRet = WaitForSingleObject(m_ShareEvent,1000);
		if(WAIT_TIMEOUT == tWaitRet || WAIT_FAILED == tWaitRet)
			continue;

		printf("wait success\n");
		unsigned short tBuf = 0;
		if(TRUE == DeviceIoControl(tFilterHandle,IOCTL_KBDFILTER_GETMAKECODE,0,0,&tBuf,sizeof(tBuf),&tLen,0))
		{
			while(InterlockedExchange(&m_SpinLock,TRUE) == TRUE)
			{
				Sleep(0);
			}	
			
			printf("mymymy:%x\n",tBuf);
			m_MakeCode.push_back(tBuf);
			if(m_MakeCode.size() > MAX_MAKECODE)
			{
				m_MakeCode.pop_front();
			}
			InterlockedExchange(&m_SpinLock,FALSE);
		}
	}
}