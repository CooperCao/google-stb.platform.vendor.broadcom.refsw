/***************************************************************************
*     (c)2008-2013 Broadcom Corporation
*  
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,
*  and may only be used, duplicated, modified or distributed pursuant to the terms and
*  conditions of a separate, written license agreement executed between you and Broadcom
*  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
*  no license (express or implied), right to use, or waiver of any kind with respect to the
*  Software, and Broadcom expressly reserves all rights in and to the Software and all
*  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
*  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
*  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.  
*   
*  Except as expressly set forth in the Authorized License,
*   
*  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
*  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
*  and to use this information only in connection with your use of Broadcom integrated circuit products.
*   
*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" 
*  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR 
*  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO 
*  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES 
*  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, 
*  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION 
*  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF 
*  USE OR PERFORMANCE OF THE SOFTWARE.
*  
*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS 
*  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR 
*  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR 
*  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF 
*  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT 
*  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE 
*  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF 
*  ANY LIMITED REMEDY.
* 
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* API Description:
* Note on ThreadInfo_Get
* Threads entring ThreadInfo_Get may be native Nexus threads, or may 
* applications threads which migrate from user process into DEVICE.EXE 
* The lmitation here is that GetOwnerProcess() is slow, and calling TlsGetValue() 
* on threads which are not part of the current process generates an assert in NK
* Defining NEXUS_CHECK_THREADINFO will enable code that checks that locks are 
* not re-entered, etc, at least on Nexus native threads. 
*
* There is a perf penaalty when NEXUS_CHECK_THREADINFO is defined.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#include "nexus_base.h"
#include "nexus_base_priv.h"
#include <windows.h>

BDBG_MODULE(nexus_base);


// Registry key to Nexus environment
static HKEY s_hEnvironmentRegistryKey = NULL;

// could be smart and read the whole key, parse, convert to ascii etc.
// Or just strdup for the few requests made and let the heap leak
// _or_ we can exploit te fact that the only code that uses this does so 
// in a serially transient fashion, so we use a static return
// #define TBD_LEAK_HEAP_MEMORY

/*******************************************************************
* Nexus OS timer functions
* 
* Implementation based on GetTickCount() as Nexus requires msec precision/resolution
* 
*/

//Return current timestamp. Units can be arbitrary
void 
NEXUS_Time_Get(NEXUS_Time *time)
{
    ASSERT(time);
    *time = GetTickCount();
	return;
}

// Return difference in milliseconds between two timestamps obtained with NEXUS_Time_Get.
long 
NEXUS_Time_Diff(const NEXUS_Time *future, const NEXUS_Time *past)
{
    ASSERT(past);
    ASSERT(future);

	return (long)(*future - *past);
}

// Add delta in ms to the timestamp
void
NEXUS_Time_Add(NEXUS_Time *time, long delta_ms)
{
    ASSERT(time);
    *time += (DWORD) delta_ms;
    return;
}

/*******************************************************************
* Nexus OS Thread functions
* 
* Implementation similar to that used under Linux. Special care is needed when 
* using Threald Local Storage from a device driver, as TLS inicies are 
* application specific, and a migrating thread may use index values differently
* 
* See Joh Spaith's note on 'Avoid TLS calls in services/device drivers while 
* processing IPC calls on WinCE ':
* http://blogs.msdn.com/cenet/archive/2007/01/05/avoid-tls-calls-in-services-device-drivers-while-processing-ipc-calls-on-wince.aspx
* 
*/

// Thread local storage index used _only_for threads created by Nexus
#ifdef NEXUS_CHECK_THREADINFO
static DWORD s_TlsIndex = (DWORD) -1;
#endif
static HANDLE s_hMainProcess = NULL;

static const NEXUS_ThreadSettings NEXUS_DefaultThreadSettings = {
    /*TBD was highest prio meant? */
    1, /* /* 0=highest, 100=lowest */
    0 /* default stackSize */
};

void 
NEXUS_Thread_GetDefaultSettings(NEXUS_ThreadSettings *pSettings)
{
    BDBG_ASSERT(pSettings);
    *pSettings = NEXUS_DefaultThreadSettings;
    return;
}

BDBG_OBJECT_ID(NEXUS_Thread);


// Thread context info. 
// This is allocated and attached to threads created in Nexus
// Migrating threads use the 'main' context
struct NEXUS_Thread {
    char name[16];
    BDBG_OBJECT(NEXUS_Thread)
	HANDLE thread;
    void (*pThreadFunc)(void *);
    void *pContext;
    NEXUS_ThreadSettings settings;
    NEXUS_P_ThreadInfo info;
    HANDLE thread_handle;
};


// Thread context for main thread (in test app case)
static struct {
    struct NEXUS_Thread mainThread;
} NEXUS_P_Os_State;


// Entry point for threads created within Nexus
static DWORD
NEXUS_P_ThreadEntry(void *t)
{
    NEXUS_ThreadHandle thread = t;

    ASSERT(t);

#ifdef NEXUS_CHECK_THREADINFO
    // save the 'thread' in allocated TLS slot
    TlsSetValue(s_TlsIndex, t);
#endif
	thread->pThreadFunc(thread->pContext);
	return 0;
}

NEXUS_ThreadHandle 
NEXUS_P_Thread_Create(const char *pThreadName, void (*pThreadFunc)(void *), void *pContext, const NEXUS_ThreadSettings *pSettings)
{
    NEXUS_ThreadHandle  thread;
    BERR_Code mrc;
    DWORD   dwCreateFlags;

    BDBG_ASSERT(pThreadName);
    BDBG_ASSERT(pThreadFunc);

    if(!pSettings) {
        pSettings = &NEXUS_DefaultThreadSettings;
    }

    thread = BKNI_Malloc(sizeof(*thread));
    if(!thread) {
        mrc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    BDBG_OBJECT_INIT(thread, NEXUS_Thread);
    strncpy(thread->name, pThreadName,sizeof(thread->name)-1);
    thread->name[sizeof(thread->name)-1]='\0';
    thread->pThreadFunc = pThreadFunc;
    thread->pContext = pContext;
    thread->settings = *pSettings;
    NEXUS_P_ThreadInfo_Init(&thread->info);
    thread->info.pThreadName = thread->name;
                      
    // Start the thread suspended so that the prio can be set gracefully
    dwCreateFlags = CREATE_SUSPENDED;
    if(pSettings->stackSize != 0)
    {
        dwCreateFlags = CREATE_SUSPENDED | STACK_SIZE_PARAM_IS_A_RESERVATION;
    }


    thread->thread_handle = CreateThread(NULL, pSettings->stackSize,
                                        NEXUS_P_ThreadEntry, (LPVOID) thread, 
                                        dwCreateFlags, NULL);


    if(thread->thread_handle == NULL)
    {
        BKNI_Free(thread);
		mrc = BERR_TRACE(BERR_OS_ERROR);
        return NULL;
    }

    // set the thread priority and wake it up
    CeSetThreadPriority(thread->thread_handle, pSettings->priority + NX_THREAD_BASE_PRIORITY);
    ResumeThread(thread->thread_handle);

    return thread;
}

void 
NEXUS_Thread_Destroy(NEXUS_ThreadHandle thread)
{
    BDBG_OBJECT_ASSERT(thread, NEXUS_Thread);

    WaitForSingleObject(thread->thread_handle, INFINITE) ;

    BDBG_OBJECT_DESTROY(thread, NEXUS_Thread);
    BKNI_Free(thread);
    return;
}


BERR_Code 
NEXUS_P_Base_Os_Init(void)
{
    long	lRes;
    NEXUS_ThreadHandle  thread;
	DWORD dispo;

    RETAILMSG(1,(L"NEXUS_P_Base_Os_Init\n"));

	// remember our process (is not calling process for driver)
	s_hMainProcess = (HANDLE) GetCurrentProcessId();

    // open the key to Nexus environment vars. Create if it does not exist
    lRes = RegCreateKeyEx(HKEY_LOCAL_MACHINE, NEXUS_ENV_KEY_NAME, 0, NULL, REG_OPTION_VOLATILE, 0, NULL, &s_hEnvironmentRegistryKey, &dispo);
    if(lRes != ERROR_SUCCESS)
    {
        DEBUGMSG(1,(TEXT("NEXUS_P_Base_Os_Init: error opening registry key:%s\n"),NEXUS_ENV_KEY_NAME));
        return BERR_TRACE(BERR_OS_ERROR);
    }

#ifdef NEXUS_CHECK_THREADINFO
    // Allocate the TLSIndex for trheads we create
    s_TlsIndex = TlsAlloc();
    if(s_TlsIndex == (DWORD)-1)
    {
        return BERR_TRACE(BERR_OS_ERROR);
    }
#endif
    
    /* initialiaze main/guest pseudo thread context */
    thread = &NEXUS_P_Os_State.mainThread;
    BDBG_OBJECT_INIT(thread, NEXUS_Thread);
    strncpy(thread->name, "main",sizeof(thread->name)-1);
    NEXUS_P_ThreadInfo_Init(&thread->info);
    thread->info.pThreadName = thread->name;
    thread->pThreadFunc = NULL;
    thread->pContext = NULL;
    thread->settings = NEXUS_DefaultThreadSettings;

#ifdef NEXUS_CHECK_THREADINFO
    // save the 'thread' in allocated TLS slot
    // In driver mode, calling thread may not be in our porcess
    if(s_hMainProcess == GetOwnerProcess())
    {
        TlsSetValue(s_TlsIndex, (LPVOID) thread);
    }
#endif

    return BERR_SUCCESS;
}

void 
NEXUS_P_Base_Os_Uninit(void)
{
    RETAILMSG(1,(L"NEXUS_P_Base_Os_Uninit\n"));

    // done with registry environment key
    if(s_hEnvironmentRegistryKey != NULL)
    {
        RegCloseKey(s_hEnvironmentRegistryKey);
        s_hEnvironmentRegistryKey = NULL;
    }
#ifdef NEXUS_CHECK_THREADINFO
    if( s_TlsIndex != -1)
    {
        TlsFree(s_TlsIndex);
        s_TlsIndex = -1;
    }
#endif
    return;
}

/*********************************************************************
*/
 
NEXUS_P_ThreadInfo *
NEXUS_P_ThreadInfo_Get(void)
{
    NEXUS_ThreadHandle  thread=NULL;

#ifdef NEXUS_CHECK_THREADINFO
	HANDLE hOwner = GetOwnerProcess();
	
//	RETAILMSG(1,(TEXT("NEXUS_P_ThreadInfo_Get: Owner=0x%X, main=0x%X\n"), hOwner, s_hMainProcess));
    if( hOwner == s_hMainProcess)
    {
        thread = (NEXUS_ThreadHandle) TlsGetValue(s_TlsIndex);

		//MPC: Thread could be NULL
		if(thread)
        {
        	return &thread->info;
        }
    }
#endif
    return NULL;
}

/*********************************************************************
* Nexus environment functions 
*   WinCE doesn't support environment variables, we'll use registry for that.
*  See <project>.reg file, under [HKEY_LOCAL_MACHINE\Nexus\Environment] 
* 
* Implementation restricts use of the staticly declared buffer 
* 
*/

const char *
NEXUS_GetEnv(const char *name)
{
    long	lRes;
    DWORD   dwSize;
    TCHAR   wchName[NX_GETENV_NAME_MAXLEN];
    static TCHAR   wchValue[NX_GETENV_VALUE_MAXLEN];
    int     str_size;
    char *  retValue = NULL;
#ifdef TBD_LEAK_HEAP_MEMORY
    // So far GetEnv is called on start-up thread only, so storing result
    // in a static buffer is OK
    char	retString[NX_GETENV_VALUE_MAXLEN];
#else
    static char	retString[NX_GETENV_VALUE_MAXLEN];
#endif

    ASSERT(name);
    str_size = strlen(name);
    if(str_size >= NX_GETENV_NAME_MAXLEN)
    {
        ASSERT(0);
        return NULL;
    }

    // Empty for debug message
    retString[0]=0;

    // convert name to Unicode key name
    mbstowcs(wchName, name, str_size+1);
    
    // Query returns BYTES
    dwSize = sizeof(wchValue) - sizeof(TCHAR);
    lRes = RegQueryValueEx(s_hEnvironmentRegistryKey, wchName, 0, NULL, (LPBYTE)wchValue, &dwSize);

    if(lRes == ERROR_MORE_DATA)
    {
        wchValue[NX_GETENV_VALUE_MAXLEN-1] = 0;
        RETAILMSG(1,(TEXT("NEXUS_GetEnv(\"%a\") Result truncated at %u chars, %u available\n"), 
                    name, sizeof(retString)-1, dwSize/sizeof(TCHAR)) );

        lRes = ERROR_SUCCESS;
    }

    if(lRes == ERROR_SUCCESS)
    {
        // convert value from Unicode back to ASCII
        wcstombs(retString, wchValue, sizeof(retString)-1);
#ifdef TBD_LEAK_HEAP_MEMORY
        retValue = LocalAlloc(LMEM_ZEROINIT,1+strlen(retString));
        strcpy(retValue, retString);
#else
        retValue = retString;
#endif
    }
    
    DEBUGMSG(0,(TEXT("NEXUS_GetEnv(\"%a\") = \"%a\", Query returns 0x%X\n"), 
                    name, retString, lRes));
    
    return retValue;
}

void 
NEXUS_SetEnv(const char *name, const char *value)
{
    long	lRes;
    DWORD   dwSize;
    TCHAR   wchName[NX_GETENV_NAME_MAXLEN];
    TCHAR   wchValue[NX_GETENV_VALUE_MAXLEN];
    int     str_size;
    char *  retValue = NULL;

    ASSERT(name);
    ASSERT(value);

    // convert name to Unicode
    str_size = strlen(name);
    if(str_size >= NX_GETENV_NAME_MAXLEN)
    {
        DEBUGMSG(1,(TEXT("NEXUS_SetEnv:invalid name\n")));
        ASSERT(0);
        return;
    }
    // copy and expand name, ask fer termination
    mbstowcs(wchName, name, str_size+1);

    // convert value to Unicode
    str_size = strlen(value);
    if(str_size >= NX_GETENV_VALUE_MAXLEN)
    {
        DEBUGMSG(1,(TEXT("NEXUS_SetEnv:invalid value\n")));
        ASSERT(0);
        return;
    }
    // return value is the number of converted multibyte characters or -1
    lRes = mbstowcs(wchValue, value, str_size+1);
    if(lRes < 0)
    {
        DEBUGMSG(1,(TEXT("NEXUS_SetEnv:invalid value\n")));
        ASSERT(0);
        return;
    }

    // size including terminating NULL
    dwSize = (str_size+1) * sizeof (TCHAR);
    lRes = RegSetValueEx(s_hEnvironmentRegistryKey, wchName, 0,
                REG_SZ, (BYTE *) wchValue, dwSize); 

    ASSERT(lRes == ERROR_SUCCESS);

    // push the change out if being cached
    RegFlushKey(s_hEnvironmentRegistryKey);

    return;
}

void 
NEXUS_FlushCache(const void *pvAddr, size_t ulNumBytes)
{
	if(ulNumBytes)  {
		/* Address must not be in FAKE range */
		if(((unsigned) pvAddr) < 0xE0000000)  {
		    /* flush and invalidate the address range provided */
			CacheRangeFlush((LPVOID)pvAddr, ulNumBytes, CACHE_SYNC_DISCARD);
		}
	}
	return;
}

void
NEXUS_Base_GetDefaultSettings(NEXUS_Base_Settings *pSettings)
{
    unsigned i;
    BDBG_ASSERT(pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    for(i=0;i<sizeof(pSettings->threadSettings)/sizeof(pSettings->threadSettings[0]);i++) {
        NEXUS_Thread_GetDefaultSettings(&pSettings->threadSettings[i]);
    }
    return;
}

int 
NEXUS_atoi(const char *str)
{
    return atoi(str);
}

int
NEXUS_P_Base_StrCmp(const char *str1, const char *str2)
{
    return strcmp(str1, str2);
}


/**
NEXUS_P_Base_Os_MarkThread allows nexus to store bookkeeping information
about external threads. This function does not need to be called for threads
nexus creates using NEXUS_Thread_Create.
threads.
**/
void
NEXUS_P_Base_Os_MarkThread(const char *name)
{
    /* NEXUS_LockModule can't be uses since NEXUS_P_Base_Os_MarkThread_locked would create key, and Unlock would see then Unlock without Lock */
    /* profiling not supporting in wince os */
    BSTD_UNUSED(name);
    return;
}

void NEXUS_Platform_P_StopCallbacks(void *interfaceHandle)
{
    NEXUS_Base_P_StopCallbacks(interfaceHandle);
    return;
}

void NEXUS_Platform_P_StartCallbacks(void *interfaceHandle)
{
    NEXUS_Base_P_StartCallbacks(interfaceHandle);
    return;
}
