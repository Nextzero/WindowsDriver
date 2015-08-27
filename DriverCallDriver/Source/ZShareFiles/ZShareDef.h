//---------------------------------------------------------------------------
#ifndef zsharedef_H_
#define zsharedef_H_
//---------------------------------------------------------------------------

#ifdef _NTDDK_
#define __T(x)      L##x
#else
#define __T(x)      x
#endif

#define DEVICE_NAME				__T("\\Device\\SYS_Example")
#define DEVICE_SYMBOLICLINK		__T("\\??\\symbolic_Example")

#define DEVICE_NAME_CALLDRIVER				__T("\\Device\\SYS_CallDriver")
#define DEVICE_SYMBOLICLINK__CALLDRIVER		__T("\\??\\symbolic_CallDriver")
//---------------------------------------------------------------------------
#endif
