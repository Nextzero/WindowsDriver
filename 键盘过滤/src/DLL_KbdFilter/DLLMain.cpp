#include "DLLMain.h"
//---------------------------------------------------------------------------
HINSTANCE __MyInstance__ = 0;
//---------------------------------------------------------------------------
BOOL WINAPI DllMain(HINSTANCE hInstDll, DWORD fdwReason, LPVOID lpvReserved)
{
	switch(fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		__MyInstance__	= hInstDll;
		DisableThreadLibraryCalls(__MyInstance__);
		break;

	case DLL_PROCESS_DETACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	}

	return true;
}
//---------------------------------------------------------------------------
int main(void)
{
	
	MC_KernelWork tKernelWork;
	if(false == tKernelWork.OnLoad())
	{
		printf("onload fail\n");
		tKernelWork.OnExit();
		return 0;
	}
	getchar();
	printf("enable test \n");
	printf("cur status: %d\n",tKernelWork.m_CurStatus);
	/*
	tKernelWork.Disabled();
		printf("after disable status: %d\n",tKernelWork.m_CurStatus);
		getchar();*/
	

	tKernelWork.Enable();
	printf("after enable status: %d\n",tKernelWork.m_CurStatus);
	getchar();

	tKernelWork.CancelIntercept();
	printf("after CancelIntercept status: %d\n",tKernelWork.m_CurStatus);
	getchar();

	unsigned short tBuf[10] = {0};
	int tLen = sizeof(tBuf);
	int total = tKernelWork.Intercept(tBuf,&tLen);
	getchar();
	tLen = sizeof(tBuf);
	total = tKernelWork.Intercept(tBuf,&tLen);
	printf("after Intercept status: %d\n",tKernelWork.m_CurStatus);
	getchar();

	printf("total:%d,tLen:%d\n",total,tLen);
	for(int i=0; i<tKernelWork.m_MakeCode.size(); i++)
	{
		printf("%x,",tBuf[i]);
	}
	tKernelWork.OnExit();
	getchar();
	return 0;
	
}