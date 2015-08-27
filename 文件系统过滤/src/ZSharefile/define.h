/*
 *  ”…SYS_KbdFilter,DLL_KbdFilterπ≤œÌ°£
 */
#ifndef __DEFINEH_
#define __DEFINEH_

/*
extern "C" 
{
#include <ntddk.h>
#include <ntddkbd.h>
}*/

#define ___T(x)		L##x 
#define __T(x)      ___T(x)

//---------------------------------------------------------------------------
#define SERVER_NAME_KBDFILTER				"SYS_KbdFilter"
#define DEVICE_NAME_KBDFILTER_FILENAME		"SYS_KbdFilter.sys"

#define DEVICE_NAME_KBDFILTER 				"\\Device\\SYS_KbdFilter"
#define DEVICE_NAME_KBDFILTER_SYMBOL 		"\\??\\SYS_KbdFilterSymbol"
#define DEVICE_NAME_KBDFILTER_API 			"\\\\.\\SYS_KbdFilterSymbol"
//---------------------------------------------------------------------------
//master
#define IOCTL_KBDFILTER_ENABLE				CTL_CODE(FILE_DEVICE_UNKNOWN, 0x7700, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_KBDFILTER_DISABLE				CTL_CODE(FILE_DEVICE_UNKNOWN, 0x7701, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_KBDFILTER_INTERCEPT			CTL_CODE(FILE_DEVICE_UNKNOWN, 0x7702, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_KBDFILTER_CANCELINTERCEPT		CTL_CODE(FILE_DEVICE_UNKNOWN, 0x7703, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_KBDFILTER_GETSTATUS			CTL_CODE(FILE_DEVICE_UNKNOWN, 0x7704, METHOD_BUFFERED, FILE_ANY_ACCESS)
//data
#define IOCTL_KBDFILTER_SHAREEVENT			CTL_CODE(FILE_DEVICE_UNKNOWN, 0x7705, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_KBDFILTER_GETMAKECODE			CTL_CODE(FILE_DEVICE_UNKNOWN, 0x7706, METHOD_BUFFERED, FILE_ANY_ACCESS)
//---------------------------------------------------------------------------

#endif