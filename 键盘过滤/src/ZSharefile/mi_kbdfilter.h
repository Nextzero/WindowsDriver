#ifndef __MI_KBDFILTERH_
#define __MI_KBDFILTERH_
//---------------------------------------------------------------------------
#define DLL_NAME_KBDFILTER "DLL_KbdFilter"
//---------------------------------------------------------------------------
//#define KBDSUCCESS	0
#define KBDERROR		-1
//---------------------------------------------------------------------------
#define ENABLE			1
#define INTERCEPT		2		/* nRet & INTERCEPT*/
//---------------------------------------------------------------------------
struct MI_KbdFilter
{
	virtual bool	OnLoad()		= 0;
	virtual void	OnExit()		= 0;

	virtual int		Enable()		= 0;
	virtual int		Disabled()		= 0;

	virtual int		GetStatus()		= 0;

	virtual int		Intercept(unsigned short* tBuf, int* tLen)	= 0;
	virtual int		CancelIntercept()							= 0;
};
//---------------------------------------------------------------------------
#endif