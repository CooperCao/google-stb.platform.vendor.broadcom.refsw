
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <io.h>
#include "epivers.h"
#include "epiioctl.h"
#include "epictrl.h"
#include "wmiprocs.h"
#include "vendor.h"

#if VERBOSE
#define VPRINTF printf
#else
#define VPRINTF
#endif

#define WMI_DLL     "wmi.dll"

#ifndef ARRAYSIZE
#define ARRAYSIZE(a)  (sizeof(a)/sizeof(a[0]))
#endif
#define snprintf _snprintf
    

extern void GetOSVer();

struct ProcEntry {
    PVOID addr;
    char *name;
};

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
			GetOSVer();
            break;

        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            break;
            
        case DLL_PROCESS_DETACH:
            break;
    }       
    
    return TRUE;
}
