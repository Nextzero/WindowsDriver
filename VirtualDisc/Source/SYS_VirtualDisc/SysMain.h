//---------------------------------------------------------------------------
#ifndef SysMainH
#define SysMainH
//---------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" 
{
#endif

#include <ntddk.h>
#include <ntdddisk.h>

#ifdef __cplusplus
}
#endif

#define Debug(_X_)		{KdPrint((_X_)); KdPrint(("\n"));}

//---------------------------------------------------------------------------
// Interface export files.
#include "../ZShareFiles/ZShareDef.h"
//---------------------------------------------------------------------------
// Share define file in here.
//---------------------------------------------------------------------------
// Myself include file in here.
//---------------------------------------------------------------------------
#endif