#ifndef __MC_KERNELWORKH_
#define __MC_KERNELWORKH_

#include "../ZSharefile/mi_kbdfilter.h"
#include "DLLMain.h"


#include <list>

#define MAX_MAKECODE 4096

class MC_KernelWork : public MI_KbdFilter
{
public:
	MC_KernelWork(void);
	~MC_KernelWork(void);

	virtual bool	OnLoad();
	virtual void	OnExit();

	virtual int		Enable();
	virtual int		Disabled();

	virtual int		GetStatus();

	virtual int		Intercept(unsigned short* tBuf, int* tLen);
	virtual int		CancelIntercept();

public:
	static DWORD WINAPI ThreadProc(LPVOID lpParameter)
	{
		MC_KernelWork* tThis = (MC_KernelWork*)lpParameter;
		tThis->KbdThreadProc();
		return 0;
	}

private:
	int KbdThreadProc();

public:
	bool	m_Avaliable;

	HANDLE	m_ShareEvent;
	HANDLE	m_Thread;
	bool	m_ThreadTerminate;

	//InterlockedCompareExchange();
	LONG	m_SpinLock;
	std::list<unsigned short> m_MakeCode;

	int	m_CurStatus;
};

#endif


