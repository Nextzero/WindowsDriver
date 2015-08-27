#include "LoadDriver.h"

//װ��NT��������
BOOL LoadNTDriverW(WCHAR* lpszDriverName,WCHAR* lpszDriverPath)
{
	WCHAR szDriverImagePath[256];
	//�õ�����������·��
	GetFullPathNameW(lpszDriverPath, 256, szDriverImagePath, NULL);

	BOOL bRet = FALSE;

	SC_HANDLE hServiceMgr=NULL;//SCM�������ľ��
	SC_HANDLE hServiceDDK=NULL;//NT��������ķ�����

	//�򿪷�����ƹ�����
	hServiceMgr = OpenSCManagerW( NULL, NULL, SC_MANAGER_ALL_ACCESS );

	if( hServiceMgr == NULL )  
		{
		//OpenSCManagerʧ��
		printf( "OpenSCManager() Faild %d ! \n", GetLastError() );
		bRet = FALSE;
		goto BeforeLeave;
		}
	else
		{
		////OpenSCManager�ɹ�
		printf( "OpenSCManager() ok ! \n" );  
		}

	//������������Ӧ�ķ���
	hServiceDDK = CreateServiceW( hServiceMgr,
		lpszDriverName, //�����������ע����е�����  
		lpszDriverName, // ע������������ DisplayName ֵ  
		SERVICE_ALL_ACCESS, // ������������ķ���Ȩ��  
		SERVICE_KERNEL_DRIVER,// ��ʾ���صķ�������������  
		SERVICE_DEMAND_START, // ע������������ Start ֵ  
		SERVICE_ERROR_IGNORE, // ע������������ ErrorControl ֵ  
		szDriverImagePath, // ע������������ ImagePath ֵ  
		NULL,  
		NULL,  
		NULL,  
		NULL,  
		NULL);  

	DWORD dwRtn;
	//�жϷ����Ƿ�ʧ��
	if( hServiceDDK == NULL )  
		{  
		dwRtn = GetLastError();
		if( dwRtn != ERROR_IO_PENDING && dwRtn != ERROR_SERVICE_EXISTS )  
			{  
			printf( "CrateService() Faild %d ! \n", dwRtn );  
			bRet = FALSE;
			goto BeforeLeave;
			}  
		else  
			{
			printf( "CrateService() Faild Service is ERROR_IO_PENDING or ERROR_SERVICE_EXISTS! \n" );  
			}

		// ���������Ѿ����أ�ֻ��Ҫ��  
		hServiceDDK = OpenServiceW( hServiceMgr, lpszDriverName, SERVICE_ALL_ACCESS );  
		if( hServiceDDK == NULL )  
			{
			dwRtn = GetLastError();  
			printf( "OpenService() Faild %d ! \n", dwRtn );  
			bRet = FALSE;
			goto BeforeLeave;
			}  
		else 
			{
			printf( "OpenService() ok ! \n" );
			}
		}  
	else  
		{
		printf( "CrateService() ok ! \n" );
		}

	//�����������
	bRet= StartServiceW( hServiceDDK, NULL, NULL );  
	if( !bRet )  
		{  
		DWORD dwRtn = GetLastError();  
		if( dwRtn != ERROR_IO_PENDING && dwRtn != ERROR_SERVICE_ALREADY_RUNNING )  
			{  
			printf( "StartService() Faild %d ! \n", dwRtn );  
			bRet = FALSE;
			goto BeforeLeave;
			}  
		else  
			{  
			if( dwRtn == ERROR_IO_PENDING )  
				{  
				printf( "StartService() Faild ERROR_IO_PENDING ! \n");
				bRet = FALSE;
				goto BeforeLeave;
				}  
			else  
				{  
				printf( "StartService() Faild ERROR_SERVICE_ALREADY_RUNNING ! \n");
				bRet = TRUE;
				goto BeforeLeave;
				}  
			}  
		}
	bRet = TRUE;
BeforeLeave:
	if(hServiceDDK)
		{
		CloseServiceHandle(hServiceDDK);
		}
	if(hServiceMgr)
		{
		CloseServiceHandle(hServiceMgr);
		}
	return bRet;
}

//ж����������  
BOOL UnloadNTDriverW( WCHAR * szSvrName )  
{
	BOOL bRet = FALSE;
	SC_HANDLE hServiceMgr=NULL;//SCM�������ľ��
	SC_HANDLE hServiceDDK=NULL;//NT��������ķ�����
	SERVICE_STATUS SvrSta;
	
	hServiceMgr = OpenSCManagerW( NULL, NULL, SC_MANAGER_ALL_ACCESS );  
	if( hServiceMgr == NULL )  
		{
		printf( "OpenSCManager() Faild %d ! \n", GetLastError() );  
		bRet = FALSE;
		goto BeforeLeave;
		}  
	else  
		{
		printf( "OpenSCManager() ok ! \n" );  
		}
	hServiceDDK = OpenServiceW( hServiceMgr, szSvrName, SERVICE_ALL_ACCESS );  

	if( hServiceDDK == NULL )  
		{
		printf( "OpenService() Faild %d ! \n", GetLastError() );  
		bRet = FALSE;
		goto BeforeLeave;
		}  
	else  
		{  
		printf( "OpenService() ok ! \n" );  
		}  
	if( !ControlService( hServiceDDK, SERVICE_CONTROL_STOP , &SvrSta ) )  
		{  
		printf( "ControlService() Faild %d !\n", GetLastError() );  
		}  
	else  
		{
		printf( "ControlService() ok !\n" );  
		}  
	if( !DeleteService( hServiceDDK ) )  
		{
		printf( "DeleteSrevice() Faild %d !\n", GetLastError() );  
		}  
	else  
		{  
		printf( "DelServer:eleteSrevice() ok !\n" );  
		}  
	bRet = TRUE;
BeforeLeave:
	if(hServiceDDK)
		{
		CloseServiceHandle(hServiceDDK);
		}
	if(hServiceMgr)
		{
		CloseServiceHandle(hServiceMgr);
		}
	return bRet;	
} 

//װ��NT��������
BOOL LoadNTDriverA(char* lpszDriverName,char* lpszDriverPath)
{
	char szDriverImagePath[256];
	//�õ�����������·��
	GetFullPathNameA(lpszDriverPath, 256, szDriverImagePath, NULL);

	BOOL bRet = FALSE;

	SC_HANDLE hServiceMgr=NULL;//SCM�������ľ��
	SC_HANDLE hServiceDDK=NULL;//NT��������ķ�����

	//�򿪷�����ƹ�����
	hServiceMgr = OpenSCManagerA( NULL, NULL, SC_MANAGER_ALL_ACCESS );

	if( hServiceMgr == NULL )  
	{
		//OpenSCManagerʧ��
		bRet = FALSE;
		goto BeforeLeave;
	}
	else
	{
		////OpenSCManager�ɹ�
		//printf( "OpenSCManager() ok ! \n" );  
	}

	//������������Ӧ�ķ���
	hServiceDDK = CreateServiceA( hServiceMgr,
		lpszDriverName, //�����������ע����е�����  
		lpszDriverName, // ע������������ DisplayName ֵ  
		SERVICE_ALL_ACCESS, // ������������ķ���Ȩ��  
		SERVICE_KERNEL_DRIVER,// ��ʾ���صķ�������������  
		SERVICE_DEMAND_START, // ע������������ Start ֵ  
		SERVICE_ERROR_IGNORE, // ע������������ ErrorControl ֵ  
		szDriverImagePath, // ע������������ ImagePath ֵ  
		NULL,  
		NULL,  
		NULL,  
		NULL,  
		NULL);  

	DWORD dwRtn;
	//�жϷ����Ƿ�ʧ��
	if( hServiceDDK == NULL )  
	{  
		dwRtn = GetLastError();
		if( dwRtn != ERROR_IO_PENDING && dwRtn != ERROR_SERVICE_EXISTS )  
		{  
			printf( "CrateService() Faild %d ! \n", dwRtn );  
			bRet = FALSE;
			goto BeforeLeave;
		}  
		else  
		{
			printf( "CrateService() Faild Service is ERROR_IO_PENDING or ERROR_SERVICE_EXISTS! \n" );  
		}

		hServiceDDK = OpenServiceA( hServiceMgr, lpszDriverName, SERVICE_ALL_ACCESS );  
		if( hServiceDDK == NULL )  
		{
			dwRtn = GetLastError();  
			printf( "OpenService() Faild %d ! \n", dwRtn );  
			bRet = FALSE;
			goto BeforeLeave;
		}  
		else 
		{
			printf( "OpenService() ok ! \n" );
		}
	}  
	else  
	{
		printf( "CrateService() ok ! \n" );
	}

	//�����������
	bRet= StartServiceA( hServiceDDK, NULL, NULL );  
	if( !bRet )  
	{  
		DWORD dwRtn = GetLastError();  
		if( dwRtn != ERROR_IO_PENDING && dwRtn != ERROR_SERVICE_ALREADY_RUNNING )  
		{  
			printf( "StartService() Faild %d ! \n", dwRtn );  
			bRet = FALSE;
			goto BeforeLeave;
		}  
		else  
		{  
			if( dwRtn == ERROR_IO_PENDING )  
			{  
				printf( "StartService() Faild ERROR_IO_PENDING ! \n");
				bRet = FALSE;
				goto BeforeLeave;
			}  
			else  
			{  
				printf( "StartService() Faild ERROR_SERVICE_ALREADY_RUNNING ! \n");
				bRet = TRUE;
				goto BeforeLeave;
			}  
		}  
	}
	bRet = TRUE;
BeforeLeave:
	if(hServiceDDK)
	{
		CloseServiceHandle(hServiceDDK);
	}
	if(hServiceMgr)
	{
		CloseServiceHandle(hServiceMgr);
	}
	return bRet;
}

//ж����������  
BOOL UnloadNTDriverA( char* szSvrName )  
{
	BOOL bRet				= FALSE;
	SC_HANDLE hServiceMgr	= NULL;//SCM�������ľ��
	SC_HANDLE hServiceDDK	= NULL;//NT��������ķ�����
	SERVICE_STATUS SvrSta;

	hServiceMgr = OpenSCManagerA( NULL, NULL, SC_MANAGER_ALL_ACCESS );  
	if( hServiceMgr == NULL )  
	{
		printf( "OpenSCManager() Faild %d ! \n", GetLastError() );  
		bRet = FALSE;
		goto BeforeLeave;
	}  
	else  
	{
		printf( "OpenSCManager() ok ! \n" );  
	}
	
	hServiceDDK = OpenServiceA( hServiceMgr, szSvrName, SERVICE_ALL_ACCESS );  
	if( hServiceDDK == NULL )  
	{
	
		printf( "OpenService() Faild %d ! \n", GetLastError() );  
		bRet = FALSE;
		goto BeforeLeave;
	}  
	else  
	{  
		printf( "OpenService() ok ! \n" );  
	}  

	if( !ControlService( hServiceDDK, SERVICE_CONTROL_STOP , &SvrSta ) )  
	{  
		printf( "ControlService() Faild %d !\n", GetLastError() );  
	}  
	else  
	{
		printf( "ControlService() ok !\n" );  
	}  

	//��̬ж����������  
	if( !DeleteService( hServiceDDK ) )  
	{
		printf( "DeleteSrevice() Faild %d !\n", GetLastError() );  
	}  
	else  
	{  
		printf( "DelServer:eleteSrevice() ok !\n" );  
	}  
	bRet = TRUE;
BeforeLeave:
	if(hServiceDDK)
	{
		CloseServiceHandle(hServiceDDK);
	}
	if(hServiceMgr)
	{
		CloseServiceHandle(hServiceMgr);
	}
	return bRet;	
} 

bool Start(char* vServiceName)
{
	if(0 == vServiceName)
		return false;

	SC_HANDLE schSCManager = OpenSCManager (0, 0, SC_MANAGER_ALL_ACCESS); 
	if(0 == schSCManager)
		return false;

	SC_HANDLE schService = OpenService(schSCManager, vServiceName, SERVICE_ALL_ACCESS);
	if(0 == schService)
	{
		CloseServiceHandle(schSCManager);
		return false;
	}

	if(false == StartService(schService, 0, 0))
	{
		if(ERROR_SERVICE_ALREADY_RUNNING == GetLastError())
		{
			CloseServiceHandle(schService);
			CloseServiceHandle(schSCManager);
			return true;
		}
		else
		{
			CloseServiceHandle(schService);
			CloseServiceHandle(schSCManager);
			return false; 
		}
	}

	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);
	return true;
}

bool Stop(char* vServiceName)
{
	if(0 == vServiceName)
		return false;

	SC_HANDLE schSCManager = OpenSCManager (0, 0, SC_MANAGER_ALL_ACCESS); 
	if(0 == schSCManager)
		return false;

	SC_HANDLE schService = OpenService(schSCManager, vServiceName, SERVICE_ALL_ACCESS);
	if(0 == schService)
	{
		CloseServiceHandle(schSCManager);
		return false;
	}

	SERVICE_STATUS status;
	if(false == ControlService(schService, SERVICE_CONTROL_STOP, &status))
	{
		CloseServiceHandle(schService);
		CloseServiceHandle(schSCManager);
		return false;
	}

	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);
	return true;
}

bool IsExists(char* vServiceName)
{
	if(0 == vServiceName)
		return false;

	SC_HANDLE schSCManager = OpenSCManager (0, 0, SC_MANAGER_CONNECT); 
	if(0 == schSCManager)
		return false;

	SC_HANDLE schService = OpenService(schSCManager, vServiceName, SERVICE_ALL_ACCESS);
	if(0 == schService)
	{
		CloseServiceHandle(schSCManager);
		return false;
	}

	if(1060 == GetLastError())
	{
		CloseServiceHandle(schSCManager);
		return false;
	}

	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);
	return true;
}
//---------------------------------------------------------------------------
bool IsRunning(char* vServiceName)
{
	if(0 == vServiceName)
		return false;

	SC_HANDLE schSCManager = OpenSCManager (0, 0, SC_MANAGER_CONNECT); 
	if(0 == schSCManager)
		return false;

	SC_HANDLE schService = OpenService(schSCManager, vServiceName, SERVICE_ALL_ACCESS);
	if(0 == schService)
	{
		CloseServiceHandle(schSCManager);
		return false;
	}

	SERVICE_STATUS status;
	QueryServiceStatus(schService, &status);
	if(SERVICE_RUNNING != status.dwCurrentState)
	{
		CloseServiceHandle(schService);
		CloseServiceHandle(schSCManager);
		return false;
	}

	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);
	return true;
}

bool SafeBootUnReg(char* vServiceName)
{
	char tKeyPath[1024];
/*
	sprintf(tKeyPath, "HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\Minimal\\%s", vServiceName);
	MC_RegHelper::RegKeyDel(tKeyPath);

	sprintf(tKeyPath, "HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\Minimal\\%s.sys", vServiceName);
	MC_RegHelper::RegKeyDel(tKeyPath);

	sprintf(tKeyPath, "HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\Network\\%s", vServiceName);
	MC_RegHelper::RegKeyDel(tKeyPath);

	sprintf(tKeyPath, "HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\Network\\%s.sys", vServiceName);
	MC_RegHelper::RegKeyDel(tKeyPath);*/

	return true;
}


bool Remove(char* vServiceName)
{
	if(0 == vServiceName)
		return false;

	SC_HANDLE schSCManager = OpenSCManager (0, 0, SC_MANAGER_ALL_ACCESS); 
	if(0 == schSCManager)
		return false;

	SC_HANDLE schService = OpenService(schSCManager, vServiceName, SERVICE_ALL_ACCESS);
	if(0 == schService)
	{
		CloseServiceHandle(schSCManager);
		return false;
	}

	if(false == DeleteService(schService))
	{
		CloseServiceHandle(schService);
		CloseServiceHandle(schSCManager);
		return false;
	}

	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);

	SafeBootUnReg(vServiceName);
	return true;
}