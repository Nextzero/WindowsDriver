#include "LoadDriver.h"

//装载NT驱动程序
BOOL LoadNTDriverW(WCHAR* lpszDriverName,WCHAR* lpszDriverPath)
{
	WCHAR szDriverImagePath[256];
	//得到完整的驱动路径
	GetFullPathNameW(lpszDriverPath, 256, szDriverImagePath, NULL);

	BOOL bRet = FALSE;

	SC_HANDLE hServiceMgr=NULL;//SCM管理器的句柄
	SC_HANDLE hServiceDDK=NULL;//NT驱动程序的服务句柄

	//打开服务控制管理器
	hServiceMgr = OpenSCManagerW( NULL, NULL, SC_MANAGER_ALL_ACCESS );

	if( hServiceMgr == NULL )  
		{
		//OpenSCManager失败
		printf( "OpenSCManager() Faild %d ! \n", GetLastError() );
		bRet = FALSE;
		goto BeforeLeave;
		}
	else
		{
		////OpenSCManager成功
		printf( "OpenSCManager() ok ! \n" );  
		}

	//创建驱动所对应的服务
	hServiceDDK = CreateServiceW( hServiceMgr,
		lpszDriverName, //驱动程序的在注册表中的名字  
		lpszDriverName, // 注册表驱动程序的 DisplayName 值  
		SERVICE_ALL_ACCESS, // 加载驱动程序的访问权限  
		SERVICE_KERNEL_DRIVER,// 表示加载的服务是驱动程序  
		SERVICE_DEMAND_START, // 注册表驱动程序的 Start 值  
		SERVICE_ERROR_IGNORE, // 注册表驱动程序的 ErrorControl 值  
		szDriverImagePath, // 注册表驱动程序的 ImagePath 值  
		NULL,  
		NULL,  
		NULL,  
		NULL,  
		NULL);  

	DWORD dwRtn;
	//判断服务是否失败
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

		// 驱动程序已经加载，只需要打开  
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

	//开启此项服务
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

//卸载驱动程序  
BOOL UnloadNTDriverW( WCHAR * szSvrName )  
{
	BOOL bRet = FALSE;
	SC_HANDLE hServiceMgr=NULL;//SCM管理器的句柄
	SC_HANDLE hServiceDDK=NULL;//NT驱动程序的服务句柄
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

//装载NT驱动程序
BOOL LoadNTDriverA(char* lpszDriverName,char* lpszDriverPath)
{
	char szDriverImagePath[256];
	//得到完整的驱动路径
	GetFullPathNameA(lpszDriverPath, 256, szDriverImagePath, NULL);

	BOOL bRet = FALSE;

	SC_HANDLE hServiceMgr=NULL;//SCM管理器的句柄
	SC_HANDLE hServiceDDK=NULL;//NT驱动程序的服务句柄

	//打开服务控制管理器
	hServiceMgr = OpenSCManagerA( NULL, NULL, SC_MANAGER_ALL_ACCESS );

	if( hServiceMgr == NULL )  
	{
		//OpenSCManager失败
		bRet = FALSE;
		goto BeforeLeave;
	}
	else
	{
		////OpenSCManager成功
		//printf( "OpenSCManager() ok ! \n" );  
	}

	//创建驱动所对应的服务
	hServiceDDK = CreateServiceA( hServiceMgr,
		lpszDriverName, //驱动程序的在注册表中的名字  
		lpszDriverName, // 注册表驱动程序的 DisplayName 值  
		SERVICE_ALL_ACCESS, // 加载驱动程序的访问权限  
		SERVICE_KERNEL_DRIVER,// 表示加载的服务是驱动程序  
		SERVICE_DEMAND_START, // 注册表驱动程序的 Start 值  
		SERVICE_ERROR_IGNORE, // 注册表驱动程序的 ErrorControl 值  
		szDriverImagePath, // 注册表驱动程序的 ImagePath 值  
		NULL,  
		NULL,  
		NULL,  
		NULL,  
		NULL);  

	DWORD dwRtn;
	//判断服务是否失败
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

	//开启此项服务
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

//卸载驱动程序  
BOOL UnloadNTDriverA( char* szSvrName )  
{
	BOOL bRet				= FALSE;
	SC_HANDLE hServiceMgr	= NULL;//SCM管理器的句柄
	SC_HANDLE hServiceDDK	= NULL;//NT驱动程序的服务句柄
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

	//动态卸载驱动程序。  
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