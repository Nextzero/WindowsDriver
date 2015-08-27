//---------------------------------------------------------------------------
#ifndef zsharedef_H_
#define zsharedef_H_
//---------------------------------------------------------------------------
#include "ZVersion.h"

#ifdef _NTDDK_
#define __T(x)      L##x
#else
#define __T(x)      x
#endif

#define DEVICE_NAME_BASE				__T("\\Device\\BaseDevice")
#define DEVICE_SYMBOLICLINK_BASE		__T("\\??\\BaseSymbolic")

#define DEVICE_NAME_FILTER				__T("\\Device\\FilterDevice")
#define DEVICE_SYMBOLICLINK_FILTER		__T("\\??\\FilterSymbolic")

//Òì²½µ×²ãÇý¶¯
#define DEVICE_NAME_SYS_EXAMPLE			__T("\\Device\\SYS_Example")
#define DEVICE_SYMBOLICLINK_EXAMPLE		__T("\\??\\SYS_Example")

//---------------------------------------------------------------------------
#endif
