/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

// Line buffering RETAILMSG instead of printf
// This reduces the number of KITL messages emitted
// (but will redirect all BKNI_Printf output to debug log)
#define USE_CONSOLIDATING_RETAILMSG

#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "bkni_event_group.h"
#include "blst_list.h"

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <strsafe.h>

// If defined, check heap validity on every alloc or free
//#define BKNI_HEAP_VALIDATE

// If nonzero, add guards, file name tracking
#ifdef BKNI_TRACK_MALLOCS
# define BKNI_POISON_ON_FREE       1
#endif


// If defined, track total allocations of system memory, print out high-water marks
#if defined(BKNI_TRACK_MALLOCS)

#define HIGHWATER_MASK    0xFFFF
#define BKNI_SIGNATURE_ALLOC    0x4D435242
#define BKNI_SIGNATURE_FREE     0x44455246
// non-static for debugger help
DWORD   s_HighWaterMark = 0;
DWORD   s_TotalAllocation = 0;
typedef struct BKNIHEADER
{
    int     signature;
    size_t  size;
#ifndef BKNI_GUARDS_ONLY
    int     line;
    const char* file_name;
#endif
    int     guard;
} BKNIHEADER, *PBKNIHEADER;

#endif


// If defined, BKNI_Malloc uses private heap instead of application general heap
#define BKNI_USE_PRIVATE_HEAP
#define BKNI_HEAP_OPTIONS 0
#define BKNI_HEAP_INITIAL_SIZE  0x00200000  /*Initial heap size 2MB*/
#define BKNI_HEAP_MAX_SIZE      0x00600000  /*Maximum heap size 6MB*/

#define POISON_VALUE 0xC5
BDBG_MODULE(kernelinterface);

typedef struct BKNI_EventObj
{
	BLST_D_ENTRY(BKNI_EventObj) list;
	HANDLE hEvent;
	struct BKNI_GroupObj *group;
} BKNI_EventObj;

#ifdef MAGNUM_EXECUTES_IN_SINGLE_PROCESS
// use CS instead of mutex for improved performance
typedef struct BKNI_MutexObj
{
	CRITICAL_SECTION mutex;
	int ownerTaskId;
} BKNI_MutexObj;
#else
typedef HANDLE BKNI_MutexObj;
#endif

struct BKNI_GroupObj 
{
	BLST_D_HEAD(group, BKNI_EventObj) members;
	CRITICAL_SECTION hLock;
	HANDLE hEvent;
};


struct {
#ifdef MAGNUM_EXECUTES_IN_SINGLE_PROCESS
    CRITICAL_SECTION global_cs;
#else
    // use named mutex for global CS
    HANDLE  global_cs;
#endif
    LARGE_INTEGER sys_freq;
    BOOL use_PerfomanceCounter;
} g_kni;


/* this is disabled for now (011609)
 * */
#if BDBG_DEBUG_BUILD

static DWORD g_csOwner;

#define SET_CRITICAL() do { g_csOwner = GetCurrentThreadId(); } while (0)
#define CLEAR_CRITICAL() do { g_csOwner = (DWORD)NULL; } while (0)
#define CHECK_CRITICAL() ( g_csOwner == GetCurrentThreadId() )

#define ASSERT_CRITICAL() do \
{\
    if ( !CHECK_CRITICAL() )\
    {\
        BKNI_Printf("Error, must be in critical section to call %s\n", __FUNCTION__);\
        BKNI_Fail();\
    }\
} while (0)

#define ASSERT_NOT_CRITICAL() do \
{\
    if ( CHECK_CRITICAL() )\
    {\
        BKNI_Printf("Error, must not be in critical section to call %s\n", __FUNCTION__);\
        BKNI_Fail();\
    }\
} while (0)

#else

#define ASSERT_CRITICAL() (void)0
#define ASSERT_NOT_CRITICAL() (void)0
#define SET_CRITICAL() (void)0
#define CLEAR_CRITICAL() (void)0
#define CHECK_CRITICAL() 1

#endif

#ifdef BKNI_USE_PRIVATE_HEAP
static HANDLE s_hKniHeap=NULL;
#endif

static BOOL s_IsInitialized=FALSE;

#if defined(USE_CONSOLIDATING_RETAILMSG)
// Gather console output into complete lines to reduce KITL traffic
// (BDBG_XXX messages generate 3 printf calls, which are emitted in idividual KITL packets)

#define OUTPUT_LINE_MAX     ( 256 )
#define OUTPUT_MAX          ( 1024 )

static CRITICAL_SECTION s_csOutputLock;
// Unicode line buffer (includes space for CR and NUL inserts) and current index
static _TCHAR   s_OutputLineBuffer[OUTPUT_LINE_MAX+2];
static DWORD    s_OutputIndex;
// working char buffer
static char     s_OutputBuffer[OUTPUT_MAX];

// Replacement for vprintf, creates formatted char string, then outputs line-by-line
// Newlines are converted to CR/NL to keep formatting tidy on serial output devices
int output_vprintf(const char *format, va_list argptr) 
{
    int retValue;

    // Lock buffers and index
    EnterCriticalSection(&s_csOutputLock);

    // New text into (static) char buffer
    retValue = _vsnprintf(s_OutputBuffer, sizeof(s_OutputBuffer), format, argptr);

    // No message output on release builds
#ifndef SHIP_BUILD
    if(retValue) {
        int remCount = retValue;
        char *pTxt = s_OutputBuffer;
        BOOL lastWasCR = FALSE;

        while(remCount-- ){
            char c =  *pTxt++;
            // insert CR preceding newlines for pretty formatting
            switch(c) {
                case '\n':
                    if(!lastWasCR)
                      s_OutputLineBuffer[ s_OutputIndex++ ] = _T('\r');
                    // fall through to treat subsequent NL like a CR 
                case '\r':
                    lastWasCR=TRUE;
                    break;

                default:
                    lastWasCR=FALSE;
            }
            // convert current character to Unicode and tuck in
            s_OutputLineBuffer[ s_OutputIndex++ ] = (_TCHAR) (c);

            // null-terminate and ouput on EOL (or overflow)
            if(( c == '\n') || (s_OutputIndex >= OUTPUT_LINE_MAX) ) {
                s_OutputLineBuffer[ s_OutputIndex++ ] = 0;  
                OutputDebugString(s_OutputLineBuffer);
                s_OutputIndex=0;
            }
        }
    }
#endif
    // release lock and return count
    LeaveCriticalSection(&s_csOutputLock);
    return retValue;
}
#endif


BERR_Code 
BKNI_Init(void)
{
    if(s_IsInitialized)
    {
        ASSERT(0);
        return BERR_SUCCESS;
    }
#ifdef MAGNUM_EXECUTES_IN_SINGLE_PROCESS
    InitializeCriticalSection(&g_kni.global_cs);
#else
    g_kni.global_cs = CreateMutex( NULL, FALSE, TEXT("MagnumGlobalCS"));
    if(g_kni.global_cs == NULL)
        return BERR_TRACE(BERR_INVALID_PARAMETER);   

#endif
    g_kni.use_PerfomanceCounter = QueryPerformanceFrequency(&g_kni.sys_freq);

#if defined(USE_CONSOLIDATING_RETAILMSG)
    InitializeCriticalSection(&s_csOutputLock);
    s_OutputIndex=0;
#endif        
    s_IsInitialized=TRUE;
    return  BERR_SUCCESS;
}

void 
BKNI_Uninit(void)
{
    if(!s_IsInitialized)
    {
        ASSERT(0);
        return ;
    }
#ifdef MAGNUM_EXECUTES_IN_SINGLE_PROCESS
    DeleteCriticalSection(&g_kni.global_cs);
#else
    CloseHandle(g_kni.global_cs);
    g_kni.global_cs=NULL;
#endif

#ifdef BKNI_USE_PRIVATE_HEAP
    HeapDestroy(s_hKniHeap);
    s_hKniHeap=NULL;
#endif
#if defined(USE_CONSOLIDATING_RETAILMSG)
    DeleteCriticalSection(&s_csOutputLock);
#endif        
    s_IsInitialized=FALSE;
    s_HighWaterMark = 0;
    s_TotalAllocation = 0;


    return;
}

#ifdef BKNI_TRACK_MALLOCS
// struct defined in PRIVATE\WINCEOS\COREOS\CORE\LMEM\heap.h
// In DEBUG coreos build,  HEAP_SENTINELS will defined as 4.
// However, it really does not matter if we trck the .size or .cbTrueSize
// which is a good thing as we may be watching a DEBUG build of this code 
// running on a RETAIL kernel.
// Also, we are not concerned with other members of the union, which are ptr types
struct item {
    int         size;       // size of item (including header and trailer)
    union {
#if THESE_THINGS_ARE_DEFINED
        pregion prgn;       // ptr to region item is contained in
        pheap   php;        // OR ptr to heap for VirtualAlloc'ed items
#endif
        int     cbTail;     // OR # of bytes left in region beyond the tail
    };
#if HEAP_SENTINELS
    int         cbTrueSize; // actual requested size in bytes
    DWORD       dwSig;      // item header guard bytes
#endif
};
typedef struct item item, *pitem;

#endif //BKNI_TRACK_MALLOCS

#define BKNI_VMEM_ALLOC_USEG_THRESHOLD	0x00200000
LPVOID BKNI_P_MemAlloc (LPVOID pAddr, DWORD cbSize, DWORD fdwAction, LPDWORD pdwData)
{
	LPVOID lpRet = NULL;
	
	if (cbSize >= BKNI_VMEM_ALLOC_USEG_THRESHOLD) {
		switch(fdwAction) {
			case MEM_RESERVE:
				lpRet = VirtualAlloc (pAddr, cbSize, MEM_RESERVE, PAGE_NOACCESS);
				break;

			case MEM_COMMIT:
				lpRet = VirtualAlloc (pAddr, cbSize, MEM_COMMIT, PAGE_READWRITE);
				break;
			
			default:
				break;
		}
		return lpRet;
	}

	return VirtualAlloc (pAddr, cbSize, fdwAction, PAGE_READWRITE);
}

BOOL BKNI_P_MemFree(LPVOID pAddr, DWORD cbSize, DWORD fdwAction, DWORD dwData)
{
	return VirtualFree (pAddr, cbSize, fdwAction);
}


// Save Label
void *BKNI_Malloc_tagged(
	size_t reqSize,		            /* Number of bytes to allocate */
	const char*       pchFile,     /* source filename where block is allocated from */
	unsigned          iLine        /* line number in file where allocation occurs */
	)
{
    char *  ptr;
    DWORD size;
#if BKNI_TRACK_MALLOCS
    PBKNIHEADER ph;
    PDWORD pSouthGuard;
    // add in header and south guard, make sure southie can be DWORD aligned
    size = (reqSize + sizeof(BKNIHEADER) + 2*sizeof (DWORD) - 1) & ~(sizeof(DWORD)-1);
#else
    size = reqSize;
#endif
#ifdef BKNI_USE_PRIVATE_HEAP
    if(s_hKniHeap == NULL) {
		s_hKniHeap = CeHeapCreate(BKNI_HEAP_OPTIONS, BKNI_HEAP_INITIAL_SIZE, BKNI_HEAP_MAX_SIZE, BKNI_P_MemAlloc, BKNI_P_MemFree);
		if (!s_hKniHeap) {
			RETAILMSG(1,(L"%S() CeHeapCreate(0x%08X, 0x%08X, 0x%08X, ..) FAILS",
				__FUNCTION__, BKNI_HEAP_OPTIONS, BKNI_HEAP_INITIAL_SIZE, BKNI_HEAP_MAX_SIZE));
		}
	}

#ifdef BKNI_HEAP_VALIDATE
    ASSERT(HeapValidate(s_hKniHeap, 0, NULL));    
#endif
    ptr = (char *) HeapAlloc(s_hKniHeap, 0, size);

#ifdef BKNI_TRACK_MALLOCS
    if( ptr != NULL)
    {
        pitem pit = (pitem)((char*)ptr - sizeof(item));
        s_TotalAllocation += pit->size;
        if(s_HighWaterMark < s_TotalAllocation)
        {
            s_HighWaterMark = s_TotalAllocation;
            RETAILMSG(1,(L"BKNI_Malloc high-water mark=%u\n", s_HighWaterMark));
            // set next HW at page boundary above current for less chatter
            s_HighWaterMark = (s_HighWaterMark+HIGHWATER_MASK)&~HIGHWATER_MASK;
        }
    }
#endif //BKNI_TRACK_MALLOCS
#else
     ptr = (char *) malloc(size);
#endif //BKNI_USE_PRIVATE_HEAP


    if( ptr == NULL)
    {
        RETAILMSG(1,(L"BKNI_Malloc_Tag(%u) FAILS, GetLastError=%u. Called from %S:%u\r\n",
                                reqSize, GetLastError(), pchFile, iLine));
        DBGCHK(TEXT("BKNI"),0);
    }
#if BKNI_TRACK_MALLOCS
    else
    {
        // add the tracking tags
        ph = (PBKNIHEADER) ptr;
        pSouthGuard = (PDWORD)(ptr + size - sizeof(DWORD));
        ptr += sizeof(BKNIHEADER);

        // construct a prologue
        BKNI_Memset(ph, 0, sizeof(BKNIHEADER));
        *pSouthGuard = ph->guard = ph->signature = BKNI_SIGNATURE_ALLOC;
        ph->size = size;

# ifndef BKNI_GUARDS_ONLY
        ph->line = iLine;
        ph->file_name = pchFile;
# endif
    }
#endif
    return (void *) ptr;
}

// need to preserve this symbol for precompiled code
#ifdef BKNI_Malloc
# undef BKNI_Malloc
#endif
void *
BKNI_Malloc(size_t size)
{
    return  BKNI_Malloc_tagged(size,__FILE__, __LINE__);
}

void BKNI_Free_tagged(
        void *ptr,          /* Pointer to memory allocated by BKNI_Malloc */
        const char *file,
        unsigned line
    )
{

#if BKNI_TRACK_MALLOCS
    if( ptr )
    {
        PDWORD pSouthGuard;
        char * optr = (char *) ptr;
        PBKNIHEADER ph = (PBKNIHEADER) (optr - sizeof(BKNIHEADER));
        ptr = (void *)ph;
        pSouthGuard = (PDWORD) (((char *) ptr) + ph->size - sizeof(DWORD));

        if((ph->signature != BKNI_SIGNATURE_ALLOC) || (ph->guard != BKNI_SIGNATURE_ALLOC) || (*pSouthGuard != BKNI_SIGNATURE_ALLOC) )
        {
            RETAILMSG(1,(L"BKNI_Free(0x%08X) Invalid Pointer, Signature=0x%X, guard=0x%X, south guard=0x%X\n", ptr,
                    ph->signature, ph->guard, *pSouthGuard));
# ifndef BKNI_GUARDS_ONLY
            RETAILMSG(1,(L"Allocation was from %S:%u\n",ph->file_name, ph->line));
# endif
            DBGCHK(TEXT("BKNI"),0);
        }
#if defined(BKNI_POISON_ON_FREE)
        else
            memset(ptr, POISON_VALUE, ph->size-sizeof(DWORD)-sizeof(BKNIHEADER));
#endif
        *pSouthGuard = ph->guard = ph->signature = BKNI_SIGNATURE_FREE;
    }
#endif

#ifdef BKNI_USE_PRIVATE_HEAP
#ifdef BKNI_HEAP_VALIDATE
    ASSERT(HeapValidate(s_hKniHeap, 0, ptr));
#endif
#ifdef BKNI_TRACK_MALLOCS
    if(ptr != NULL)
    {
        pitem pit = (pitem)((char*)ptr - sizeof(item));
        s_TotalAllocation -= pit->size;
#if defined(BKNI_POISON_ON_FREE) && !defined(BKNI_TRACK_MALLOCS)
        memset(ptr, POISON_VALUE, pit->size);
#endif
    }
#endif
    HeapFree(s_hKniHeap, 0, ptr);
#else
    free(ptr);
#endif
    return;
}

#ifdef BKNI_Free
# undef BKNI_Free
#endif
void
BKNI_Free(void *ptr)
{
    BKNI_Free_tagged(ptr,__FILE__, __LINE__);
}

// TBD
BERR_Code
BKNI_GetMallocEntryInfo( const void *mem, struct BKNI_MallocEntryInfo *entry)
{
    memset(entry, 0, sizeof(*entry));
    return BERR_NOT_SUPPORTED;
}

#ifdef MAGNUM_EXECUTES_IN_SINGLE_PROCESS
// Using critical section as mutex

BERR_Code BKNI_CreateMutex_tagged(
    BKNI_MutexHandle *pMutex,
    const char *file,
    int line
    )
{
    BKNI_MutexHandle mutex;

    ASSERT_NOT_CRITICAL();

    mutex = (BKNI_MutexHandle) BKNI_Malloc_tagged(sizeof(*mutex), file, line);
    *pMutex = mutex;
    if (!mutex) {
        return BERR_TRACE(BERR_OS_ERROR);
    }   
    InitializeCriticalSection(&mutex->mutex);

    return BERR_SUCCESS;
}

void BKNI_DestroyMutex_tagged(
    BKNI_MutexHandle mutex,
    const char *file,
    int line
    )
{
	ASSERT_NOT_CRITICAL();

    DeleteCriticalSection(&mutex->mutex);
    BKNI_Free_tagged(mutex, file, line);
    return;
}

BERR_Code 
BKNI_TryAcquireMutex_tagged(
    BKNI_MutexHandle mutex,
    const char *file,
    int line
    )
{
	ASSERT_NOT_CRITICAL();

    if (!TryEnterCriticalSection(&mutex->mutex)) {
       return BERR_TIMEOUT; /* no BERR_TRACE here */
    }
    return BERR_SUCCESS;
}

BERR_Code 
BKNI_AcquireMutex_tagged(
    BKNI_MutexHandle mutex,
    const char *file,
    int line
    )
{
	ASSERT_NOT_CRITICAL();

    EnterCriticalSection(&mutex->mutex);
    return BERR_SUCCESS;
}

void
BKNI_ReleaseMutex_tagged(
    BKNI_MutexHandle mutex,
    const char *file,
    int line
    )
{
	ASSERT_NOT_CRITICAL();

    LeaveCriticalSection(&mutex->mutex);
    return ;
}
#else // MAGNUM_EXECUTES_IN_SINGLE_PROCESS
// Using native mutex suitable for MP

BERR_Code BKNI_CreateMutex_tagged(
    BKNI_MutexHandle *pMutex,
    const char *file,
    int line
    )
{
    BKNI_MutexHandle mutex;
	ASSERT_NOT_CRITICAL();

    mutex = CreateMutex(NULL, FALSE, NULL);
    *pMutex = mutex;
    if (!mutex) {
        return BERR_TRACE(BERR_OS_ERROR);
    }   

    return BERR_SUCCESS;
}

void BKNI_DestroyMutex_tagged(
    BKNI_MutexHandle mutex,
    const char *file,
    int line
    )
{
	ASSERT_NOT_CRITICAL();

    if(!CloseHandle(mutex))
    {
        BERR_TRACE(BERR_INVALID_PARAMETER);   
    }
    return;
}

BERR_Code 
BKNI_AcquireMutex_tagged(
    BKNI_MutexHandle mutex,
    const char *file,
    int line
    )
{
	ASSERT_NOT_CRITICAL();
    if(WAIT_OBJECT_0 == WaitForSingleObject(mutex, INFINITE))
        return BERR_SUCCESS;

    return BERR_TRACE(BERR_INVALID_PARAMETER);   
}

BERR_Code 
BKNI_TryAcquireMutex_tagged(
    BKNI_MutexHandle mutex,
    const char *file,
    int line
    )
{
	ASSERT_NOT_CRITICAL();
    switch(WaitForSingleObject(mutex, 0))
    {
        case WAIT_OBJECT_0:
            return BERR_SUCCESS;
        case WAIT_TIMEOUT:
           return BERR_TIMEOUT; /* no BERR_TRACE here */
        default:
            break;
    }
    return BERR_TRACE(BERR_INVALID_PARAMETER);   
}

void
BKNI_ReleaseMutex_tagged(
    BKNI_MutexHandle mutex,
    const char *file,
    int line
    )
{
	ASSERT_NOT_CRITICAL();
    if(!ReleaseMutex(mutex))
    {
        BERR_TRACE(BERR_INVALID_PARAMETER);   
    }
    return;
}
#endif // MAGNUM_EXECUTES_IN_SINGLE_PROCESS


#ifdef BKNI_CreateMutex
# undef BKNI_CreateMutex
#endif
BERR_Code 
BKNI_CreateMutex(BKNI_MutexHandle *pMutex)
{
    return BKNI_CreateMutex_tagged(pMutex, __FILE__, __LINE__);
}

#ifdef BKNI_DestroyMutex
# undef BKNI_DestroyMutex
#endif
void 
BKNI_DestroyMutex(BKNI_MutexHandle pMutex)
{
    BKNI_DestroyMutex_tagged(pMutex, __FILE__, __LINE__);
}

#ifdef BKNI_AcquireMutex
# undef BKNI_AcquireMutex
#endif
BERR_Code 
BKNI_AcquireMutex(BKNI_MutexHandle mutex)
{
    return BKNI_AcquireMutex_tagged(mutex, __FILE__, __LINE__);
}
#ifdef BKNI_TryAcquireMutex
# undef BKNI_TryAcquireMutex
#endif
BERR_Code 
BKNI_TryAcquireMutex(BKNI_MutexHandle mutex)
{
    return BKNI_TryAcquireMutex_tagged(mutex, __FILE__, __LINE__);
}

#ifdef BKNI_ReleaseMutex
# undef BKNI_ReleaseMutex
#endif
void BKNI_ReleaseMutex(BKNI_MutexHandle mutex)
{
    BKNI_ReleaseMutex_tagged(mutex, __FILE__, __LINE__);
}

void 
BKNI_Fail(void)
{
    DebugBreak();
//    ExitProcess( -1);
}


int 
BKNI_Printf(const char *fmt, ...)
{
    va_list arglist;
    int rc;

    va_start( arglist, fmt );
    rc = BKNI_Vprintf(fmt,arglist);
    va_end(arglist);

    return rc;
}


int 
BKNI_Snprintf(char *str, size_t len, const char *fmt, ...)
{
    va_list arglist;
    int rc;

    va_start( arglist, fmt );
    rc = _vsnprintf(str, len, fmt, arglist);
    va_end(arglist);

    return rc;
}

int 
BKNI_Vprintf(const char *fmt, va_list ap)
{
#if defined(USE_CONSOLIDATING_RETAILMSG)
    return output_vprintf(fmt, ap);
#else
    return vprintf(fmt, ap);
#endif
}

void 
BKNI_Delay(unsigned int microsec)
{

    if (g_kni.use_PerfomanceCounter) {
    LARGE_INTEGER begin, cur, diff;
        
        diff.QuadPart = (microsec * g_kni.sys_freq.QuadPart)/(1000000);
        for (QueryPerformanceCounter(&begin);QueryPerformanceCounter(&cur) && (cur.QuadPart - begin.QuadPart)<diff.QuadPart;) { }
    } else {
      volatile long l;

      while(microsec--) {
          for(l=0;l<1000;l++);
      }
    }
    return;
}

BERR_Code 
BKNI_Sleep(unsigned int millisec)
{
	ASSERT_NOT_CRITICAL();

    Sleep( millisec);
    return BERR_SUCCESS;
}


BERR_Code 
BKNI_CreateEvent_tagged(BKNI_EventHandle *pEvent, const char* pchFile, int iLine)
{
    BKNI_EventHandle event;

	ASSERT_NOT_CRITICAL();

    event = (BKNI_EventHandle) BKNI_Malloc_tagged(sizeof(*event), pchFile, iLine);
    *pEvent = event;
    if (!event) {
        return BERR_TRACE(BERR_OS_ERROR);
    }
    memset(event, 0, sizeof(*event));
    event->hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!event->hEvent) {
        BKNI_Free(event);
        return BERR_TRACE(BERR_OS_ERROR);
    }
    return BERR_SUCCESS;
}

#ifdef BKNI_CreateEvent
#	undef BKNI_CreateEvent
#endif
BERR_Code 
BKNI_CreateEvent(BKNI_EventHandle *pEvent)
{
    return BKNI_CreateEvent_tagged(pEvent, __FILE__, __LINE__);
}

void BKNI_DestroyEvent_tagged(
        BKNI_EventHandle event,
        const char *file,
        int line
    )
{
	ASSERT_NOT_CRITICAL();

    if(event->group)
    {
        BKNI_RemoveEventGroup(event->group, event);
    }

    CloseHandle(event->hEvent);
    BKNI_Free(event);
    return;
}

#ifdef BKNI_DestroyEvent
#	undef BKNI_DestroyEvent
#endif
void BKNI_DestroyEvent(
        BKNI_EventHandle event
    )
{
    BKNI_DestroyEvent_tagged(event, __FILE__, __LINE__);
}

BERR_Code 
BKNI_WaitForEvent(BKNI_EventHandle event, int timeoutMsec)
{
    DWORD winTimeout;
    DWORD rc;
    
	if ( timeoutMsec != 0 )
    {
        ASSERT_NOT_CRITICAL();
    }
    
    winTimeout = ((timeoutMsec==BKNI_INFINITE) ? INFINITE : (DWORD)timeoutMsec);
    rc = WaitForSingleObject(event->hEvent, winTimeout);
    if (rc==WAIT_OBJECT_0) {
        return BERR_SUCCESS;
    } else if (rc==WAIT_TIMEOUT) {
        return BERR_TIMEOUT; /* don't use timeout BERR_TRACE here */
    } 
    return BERR_TRACE(BERR_OS_ERROR);
}

void
BKNI_SetEvent(BKNI_EventHandle event)
{
    if (!SetEvent(event->hEvent)) {
        BDBG_ERR(("SetEvent failed"));
        BDBG_ASSERT(false);
    }
    if(event->group)
    {
        EnterCriticalSection(&event->group->hLock);
        SetEvent(event->group->hEvent);
        LeaveCriticalSection(&event->group->hLock);
    }
    return;
}

void
BKNI_ResetEvent(BKNI_EventHandle event)
{
    if (!ResetEvent(event->hEvent)) {
        BDBG_ASSERT(false);
    }
    return;
}


void
BKNI_EnterCriticalSection(void)
{
	ASSERT_NOT_CRITICAL();

#ifdef MAGNUM_EXECUTES_IN_SINGLE_PROCESS
	{
		int retries = 0, SpinLimit = 2;
		
		while(!TryEnterCriticalSection(&g_kni.global_cs)) 
		{
			if( retries++ < SpinLimit ){
				Sleep(0);
				continue;
			}
			EnterCriticalSection(&g_kni.global_cs);
			break;
		}
	}
#else
    WaitForSingleObject(g_kni.global_cs, INFINITE);
#endif

	SET_CRITICAL();

    return ;
}

void
BKNI_LeaveCriticalSection(void)
{
	ASSERT_CRITICAL();
    CLEAR_CRITICAL();
#ifdef MAGNUM_EXECUTES_IN_SINGLE_PROCESS
    LeaveCriticalSection(&g_kni.global_cs);
#else
    ReleaseMutex(g_kni.global_cs);
#endif
    return ;
}


int 
BKNI_Vsnprintf(char *s, size_t n, const char *fmt, va_list ap)
{
    return _vsnprintf(s, n, fmt, ap);
}


BERR_Code 
BKNI_CreateEventGroup(BKNI_EventGroupHandle *pGroup)
{
	BKNI_EventGroupHandle group;
	BERR_Code result;

    ASSERT_NOT_CRITICAL();

	group = BKNI_Malloc(sizeof(*group));
	if (!group) {
		result = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		return result;
	}
    memset(group, 0, sizeof(*group));
	BLST_D_INIT(&group->members);

	InitializeCriticalSection(&group->hLock);
    group->hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if(group->hEvent == NULL)
    {
        result = BERR_TRACE(BERR_OS_ERROR);
        goto err_event;
    }
    *pGroup = group;
	return BERR_SUCCESS;

err_event:
    DeleteCriticalSection(&group->hLock);
    BKNI_Free(group);
    return result;
}

void
BKNI_DestroyEventGroup(BKNI_EventGroupHandle group)
{
	ASSERT_NOT_CRITICAL();
    DeleteCriticalSection(&group->hLock);
    CloseHandle(group->hEvent);
    BKNI_Free(group);
}

BERR_Code
BKNI_AddEventGroup(BKNI_EventGroupHandle group, BKNI_EventHandle event)
{
	BERR_Code result = BERR_SUCCESS;
 	ASSERT_NOT_CRITICAL();
    
    EnterCriticalSection(&group->hLock);
	if (event->group != NULL) {
		BDBG_ERR(("Event %#x already connected to the group %#x", (unsigned)event, (unsigned)group));
		result = BERR_TRACE(BERR_OS_ERROR);
	} else {
		BLST_D_INSERT_HEAD(&group->members, event, list);
		event->group = group;
        /* Signal event addition to wait routine */
        SetEvent(group->hEvent);
	}
    LeaveCriticalSection(&group->hLock);    
    return result;
}

BERR_Code
BKNI_RemoveEventGroup(BKNI_EventGroupHandle group, BKNI_EventHandle event)
{
	BERR_Code result = BERR_SUCCESS;
	ASSERT_NOT_CRITICAL();

    EnterCriticalSection(&group->hLock);
    
    if(event->group != group)
    {
		BDBG_ERR(("Event %#x doesn't belong to group %#x", (unsigned)event, (unsigned)group));
		result = BERR_TRACE(BERR_OS_ERROR);
    }
    else
    {
        BLST_D_REMOVE(&group->members, event, list);
        event->group = NULL;
        /* Signal event removal to wait routine */
        SetEvent(group->hEvent);
    }
    LeaveCriticalSection(&group->hLock);    
    return result;
}


static unsigned 
group_get_events(BKNI_EventGroupHandle group, BKNI_EventHandle *events, unsigned max_events)
{
	BKNI_EventHandle ev;
	int rc;
	unsigned event;

    EnterCriticalSection(&group->hLock);
	for(event=0, ev=BLST_D_FIRST(&group->members); ev && event<max_events ; ev=BLST_D_NEXT(ev, list)) {
			rc = WaitForSingleObject(ev->hEvent, 0);
			if (rc==WAIT_OBJECT_0) {
				events[event] = ev;
				event++;
			}
	}
    LeaveCriticalSection(&group->hLock);    
	return event;
}


BERR_Code 
BKNI_WaitForGroup(BKNI_EventGroupHandle group, int timeoutMsec, BKNI_EventHandle *events, unsigned max_events, unsigned *nevents)
{
	BERR_Code result = BERR_SUCCESS;
    unsigned  wait_result=0;
    
    ASSERT_NOT_CRITICAL();

   	if (max_events<1) {
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}
    
    do {
		wait_result = WaitForSingleObject(group->hEvent, timeoutMsec);
    
		if(wait_result != WAIT_TIMEOUT)
		{
			*nevents = group_get_events(group, events, max_events);
		}
		else
			return BERR_TIMEOUT;
	}while(*nevents == 0);

    return result;
}

void
BKNI_AssertIsrContext_isr(const char *filename, unsigned lineno)
{
    if ( !CHECK_CRITICAL() ) {
        BDBG_P_AssertFailed("Not in critical section", filename, lineno);
    }
}

/* used functions from depricated ISO */
void *
BKNI_Memset(void *b, int c, size_t len)
{
    return memset(b, c, len);
}

void *
BKNI_Memcpy(void *dst, const void *src, size_t len)
{
    return memcpy(dst, src, len);
}

int
BKNI_Memcmp(const void *b1, const void *b2, size_t len)
{
    return memcmp(b1, b2, len);
}

void *
BKNI_Memchr(const void *b, int c, size_t len)
{
    return memchr(b, c, len);

}

void *
BKNI_Memmove(void *dst, const void *src, size_t len)
{
    return memmove(dst, src, len);
}
