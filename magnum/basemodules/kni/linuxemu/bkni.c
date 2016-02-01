/******************************************************************************
 * (c) 2003-2015 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *****************************************************************************/


#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "bkni_metrics.h"
#include "bkni_event_group.h"
#include "blst_list.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/time.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <linux/version.h>

BDBG_MODULE(kernelinterface);

BDBG_OBJECT_ID(BKNI_EventGroup);
BDBG_OBJECT_ID(BKNI_Event);
BDBG_OBJECT_ID(BKNI_Mutex);

struct BKNI_MutexObj
{
    BDBG_OBJECT(BKNI_Mutex)
    pthread_mutex_t mutex;
};

void * BKNI_Malloc_tagged(size_t size, const char *file, unsigned line);
void BKNI_Free_tagged(void *ptr, const char *file, unsigned line);

/* Broadcom integrated NPTL with our Linux 2.6.18 release. */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18) && 0
#define HAS_NPTL 1
#endif

#if !HAS_NPTL
#include <sys/time.h>
#endif

struct BKNI_GroupObj
{
    BDBG_OBJECT(BKNI_EventGroup)
    BLST_D_HEAD(group, BKNI_EventObj) members;
    pthread_mutex_t lock;            /* mutex for protecting signal and conditional variables */
    pthread_cond_t  cond;           /* condition to wake up from event*/
};

struct BKNI_EventObj
{
    BDBG_OBJECT(BKNI_Event)
    BLST_D_ENTRY(BKNI_EventObj) list;
    struct BKNI_GroupObj *group;
    pthread_mutex_t lock;            /* mutex for protecting signal and conditional variables */
    pthread_cond_t  cond;           /* condition to wake up from event*/
    bool signal;
};

static pthread_mutex_t g_csMutex = PTHREAD_MUTEX_INITIALIZER;

#if BDBG_DEBUG_BUILD

static pthread_t g_csOwner;

#define SET_CRITICAL() do { g_csOwner = pthread_self(); } while (0)
#define CLEAR_CRITICAL() do { g_csOwner = (pthread_t)0; } while (0)
#define CHECK_CRITICAL() ( g_csOwner == pthread_self() )

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
#define CHECK_CRITICAL() 0

#endif


#define BKNI_EMU_TICKTIME_FACTOR  10

#if BKNI_TRACK_MALLOCS
static pthread_mutex_t g_alloc_state_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif
#define B_TRACK_ALLOC_LOCK() pthread_mutex_lock(&g_alloc_state_mutex)
#define B_TRACK_ALLOC_UNLOCK() pthread_mutex_unlock(&g_alloc_state_mutex)
#define B_TRACK_ALLOC_ALLOC(size) malloc(size)
#define B_TRACK_ALLOC_FREE(ptr) free(ptr)
#define B_TRACK_ALLOC_OS "linuxuser"

#if BKNI_TRACK_MALLOCS
#define  BKNI_GARBLE_MALLOC 1

/* TODO: consider switching to linked list or hash table to remove bounds and increase performance */
struct BKNI_TrackAllocEntry {
    const void *mem;
    size_t size;
    const char *file;
    unsigned line;
};

struct BKNI_P_AllocUser {
    unsigned count;
    unsigned size;
    const char *file;
    unsigned line;
};

static struct {
    size_t allocated, freed, peak;
    unsigned allocated_cnt, freed_cnt;
    unsigned table_size;
    struct BKNI_TrackAllocEntry *table;
    struct BKNI_TrackAllocEntry dummy; /* record used when there is no space avaliable */
    struct {
        struct BKNI_TrackAllocEntry alloc;
        struct {
            const char *file;
            unsigned line;
        } free;
    } history[256]; /* FIFO of last freed objects */
    struct BKNI_P_AllocUser alloc_users[16]; /* temporary array used to account frequent users in the event of table resizing */
} g_alloc_state;

static void
b_memset32(void *ptr, size_t size, uint32_t word)
{
   unsigned i;

   if(ptr) {
       for(i=0; i+3 < size; i+=4) {
          *(uint32_t*)(((uint8_t *)ptr)+i)=word;
       }
   }
   return;
}

static const char *
b_shorten_filename(const char *pFileName)
{
    const char *s;
    unsigned i;

    if(pFileName==NULL) {
        return "unknown";
    }
    for(s=pFileName;*s != '\0';s++) { } /* search forward */

    for(i=0;s!=pFileName;s--) { /* search backward */
        if(*s=='/' || *s=='\\') {
            i++;
            if(i>4) {
                return s+1;
            }
        }
    }
    return pFileName;
}


static unsigned
b_alloc_hashkey(const void *mem)
{
    unsigned long hash = (unsigned long)mem;
    /* calculate a hash */
    hash = (hash ^ 61) ^ (hash>> 16);
    hash += (hash << 3);
    hash ^= (hash >> 4);
    hash *= 0x66635119; /* 1717784857  some prime number */
    hash ^= (hash >> 15);
    return hash;
}

static struct BKNI_TrackAllocEntry *
BKNI_P_GetTrackAllocEntry_one(const void *mem, const void *match, struct BKNI_TrackAllocEntry *entries, unsigned table_size)
{
    unsigned i;
    unsigned index;
    for(index=i=b_alloc_hashkey(mem)%table_size;i<2*table_size;i++,index++) {
        if(index>=table_size) {
            index = 0;
        }
        if(entries[index].mem==match) {
#if 0
            /* some profiling code to measure effectivness of the hash lookup */
            static unsigned max_count=0;
            static unsigned total_count=0;
            unsigned count = i-(b_alloc_hashkey(mem)%table_size);
            total_count+=count+1;
            if(count>max_count) {
                unsigned avg = (100*total_count)/(g_alloc_state.allocated_cnt+g_alloc_state.freed_cnt+1);
                max_count=count;
                BKNI_Printf("BKNI_GetTrackAllocEntry: scan_count:%u(%u.%02u avg) addr:%#lx key:%u elements:%u(%u)\n", count,  avg/100, avg%100, (unsigned long)mem, (b_alloc_hashkey(mem)%table_size), g_alloc_state.allocated_cnt-g_alloc_state.freed_cnt, table_size);
            }
#endif
            return  entries+index;
        }
    }
    return NULL;
}

static struct BKNI_TrackAllocEntry *
BKNI_P_GetTrackAllocEntry_resize(const struct BKNI_TrackAllocEntry *entries, unsigned table_size, unsigned new_table_size)
{
    unsigned i;
    struct BKNI_TrackAllocEntry *new_entries;

    new_entries = B_TRACK_ALLOC_ALLOC(new_table_size*sizeof(*new_entries));
    if(new_entries==NULL) { goto error;}

    for(i=0;i<new_table_size;i++) {
        new_entries[i].mem = NULL;
        new_entries[i].size = 0;
        new_entries[i].file = NULL;
        new_entries[i].line = 0;
    }
    if(entries) {
        struct BKNI_P_AllocUser *alloc_users = g_alloc_state.alloc_users;
        const unsigned alloc_users_size = sizeof(g_alloc_state.alloc_users)/sizeof(g_alloc_state.alloc_users[0]);

        BKNI_Printf("BKNI_P_GetTrackAllocEntry_resize: resizing from %u->%u\n", table_size, new_table_size);
        b_memset32(alloc_users, sizeof(*alloc_users)*alloc_users_size, 0);
        for(i=0;i<table_size;i++) {
            if(entries[i].mem) {
                struct BKNI_TrackAllocEntry *entry = BKNI_P_GetTrackAllocEntry_one(entries[i].mem, NULL, new_entries, new_table_size);
                unsigned j;
                if(!entry) {
                    goto error;
                }
                *entry = entries[i];
                for(j=0;j<alloc_users_size;j++) {
                    if(alloc_users[j].count==0) {
                        /* found empty slot, just use it */
                        alloc_users[j].count = 1;
                        alloc_users[j].size = entry->size;
                        alloc_users[j].file = entry->file;
                        alloc_users[j].line = entry->line;
                        break;
                    } else if(alloc_users[j].file == entry->file && alloc_users[j].line == entry->line) {
                        /* found existing entry, update it and move it up to keep array sorted */
                        alloc_users[j].count++;
                        alloc_users[j].size += entry->size;
                        for(;j>0;j--) {
                            struct BKNI_P_AllocUser tmp;
                            if(alloc_users[j].count <= alloc_users[j-1].count) {
                                break;
                            }
                            tmp = alloc_users[j];
                            alloc_users[j] = alloc_users[j-1];
                            alloc_users[j-1] = tmp;
                        }
                        break;
                    }
                }
                if(j==alloc_users_size) { /* no empty slots */
                    j=alloc_users_size/2; /* wipe out half of old entries */
                    alloc_users[j].count = 1;
                    alloc_users[j].size = entry->size;
                    alloc_users[j].file = entry->file;
                    alloc_users[j].line = entry->line;
                    for(j=j+1;j<alloc_users_size;j++) {
                        alloc_users[j].count = 0;
                        alloc_users[j].size = 0;
                        alloc_users[j].file = 0;
                        alloc_users[j].line = 0;
                    }
                }
            }
        }
        BKNI_Printf("BKNI_Malloc(%s) top users:\n%10s, %7s, filename:line\n", B_TRACK_ALLOC_OS,  "blocks", "bytes");
        for(i=0;i<alloc_users_size;i++) {
            if(alloc_users[i].count==0) {
                break;
            }
            BKNI_Printf("%#10u, %7u, %s:%u\n", alloc_users[i].count, alloc_users[i].size,b_shorten_filename(alloc_users[i].file), alloc_users[i].line);
        }
    }
    return new_entries;
error:
    BKNI_Printf("BKNI_P_GetTrackAllocEntry_resize: can't reallocate alloc table %u->%u\n", table_size, new_table_size);
    return NULL;
}

static struct BKNI_TrackAllocEntry *
BKNI_P_GetTrackAllocEntry(const void *mem, const void *match)
{
    struct BKNI_TrackAllocEntry *entries = g_alloc_state.table;
    struct BKNI_TrackAllocEntry *entry=NULL;
    if(entries) {
        entry = BKNI_P_GetTrackAllocEntry_one(mem, match, entries, g_alloc_state.table_size);
        if(!entry)  {
            unsigned new_table_size =  2*g_alloc_state.table_size;
            struct BKNI_TrackAllocEntry *new_entries = BKNI_P_GetTrackAllocEntry_resize(entries, g_alloc_state.table_size, new_table_size);
            B_TRACK_ALLOC_FREE(entries);
            entries = new_entries;
            g_alloc_state.table = new_entries;
            if(new_entries) {
                g_alloc_state.table_size = new_table_size;
                entry = BKNI_P_GetTrackAllocEntry_one(mem, match, entries, new_table_size);
            } else {
                g_alloc_state.table_size = 0;
            }
        }
    }
    if(entry==NULL && entries==NULL) {
        entry = &g_alloc_state.dummy; /* if there is no space, just return something */
    }
    return entry;
}


void *
BKNI_Malloc_tagged(size_t size, const char *file, unsigned line)
{
    void *mem;
    ASSERT_NOT_CRITICAL();
    mem = B_TRACK_ALLOC_ALLOC(size);

    if(mem) {
        struct BKNI_TrackAllocEntry *entry;
        size_t used;
#if BKNI_GARBLE_MALLOC
        b_memset32(mem, size, 0xDEADDA7A);
#endif
        B_TRACK_ALLOC_LOCK();
        entry = BKNI_P_GetTrackAllocEntry(mem, NULL);
        entry->mem = mem;
        entry->size = size;
        entry->file = file;
        entry->line = line;
        g_alloc_state.allocated += size;
        g_alloc_state.allocated_cnt ++;
        used = g_alloc_state.allocated - g_alloc_state.freed;
        if(used>g_alloc_state.peak) {
            g_alloc_state.peak = used;
        }
        B_TRACK_ALLOC_UNLOCK();
    } else {
        /*
        BKNI_DumpMallocs_Size(0);
        */
        BKNI_Printf("BKNI_Alloc(%u): returned NULL at %s:%u\n", size, b_shorten_filename(file), line);
        BKNI_Fail();
    }
    return mem;
}

void
BKNI_Free_tagged(void *ptr, const char *file, unsigned line)
{
    ASSERT_NOT_CRITICAL();

    if(ptr) {
        struct BKNI_TrackAllocEntry *entry;
        size_t size=0;
        B_TRACK_ALLOC_LOCK();
        entry = BKNI_P_GetTrackAllocEntry(ptr, ptr);
        if(entry) {
            unsigned history_index=g_alloc_state.freed_cnt%sizeof(g_alloc_state.history)/sizeof(g_alloc_state.history[0]);
            g_alloc_state.history[history_index].alloc = *entry;
            g_alloc_state.history[history_index].free.file = file;
            g_alloc_state.history[history_index].free.line = line;
            g_alloc_state.freed += entry->size;
            g_alloc_state.freed_cnt++;
            size = entry->size;
            entry->mem = NULL;
        } else {
            BKNI_Printf("BKNI_Free of unknown ptr: %#lx, %s:%u\n", (unsigned long)ptr, b_shorten_filename(file), line);
            BKNI_Fail();
        }
        B_TRACK_ALLOC_UNLOCK();
#if BKNI_GARBLE_MALLOC
        b_memset32(ptr, size, 0xDA7ADEAD);
#endif
    } else {
        BKNI_Printf("BKNI_Free of NULL ptr: %#lx, %s:%u\n", (unsigned long)ptr, b_shorten_filename(file), line);
        BKNI_Fail();
    }
    free(ptr);
    return;
}



#define BKNI_DUMP_SIZE_THRESHOLD        (1024)

static void
BKNI_DumpMallocs_Size(size_t threshold)
{
    unsigned i;
    bool header;
    struct BKNI_TrackAllocEntry *entries = g_alloc_state.table;

    B_TRACK_ALLOC_LOCK();
    for (header=false,i=0;entries && i<g_alloc_state.table_size;i++) {
        if ((entries[i].mem) &&
            (entries[i].size > threshold)) {
            if(!header) {
                header=true;
                BKNI_Printf("BKNI_Malloc(%s) report:\n%10s, %7s, filename:line\n", B_TRACK_ALLOC_OS, "address", "size");
            }
            BKNI_Printf("%#10lx, %7u, %s:%u\n",
                (unsigned long)entries[i].mem,
                entries[i].size,
                b_shorten_filename(entries[i].file),
                entries[i].line);
        }
    }
    B_TRACK_ALLOC_UNLOCK();
    BKNI_Printf("BKNI_Malloc(%s): allocated:%u(%u) freed:%u(%u) peak:%u\n", B_TRACK_ALLOC_OS, g_alloc_state.allocated, g_alloc_state.allocated_cnt, g_alloc_state.freed, g_alloc_state.freed_cnt, g_alloc_state.peak);
    return;
}

void
BKNI_DumpMallocs(void)
{
    BKNI_DumpMallocs_Size(BKNI_DUMP_SIZE_THRESHOLD);
}

BERR_Code
BKNI_GetMallocEntryInfo( const void *mem, struct BKNI_MallocEntryInfo *entry)
{
    const struct BKNI_TrackAllocEntry *allocated;

    entry->free_file = NULL;
    entry->free_line = 0;
    entry->alive = true;
    B_TRACK_ALLOC_LOCK();
    allocated = BKNI_P_GetTrackAllocEntry(mem, mem);
    if(!allocated) {
        unsigned i;
        for(i=0;i<sizeof(g_alloc_state.history)/sizeof(g_alloc_state.history[0]);i++) {
            if(g_alloc_state.history[i].alloc.mem == mem) {
                allocated = &g_alloc_state.history[i].alloc;
                entry->free_file = g_alloc_state.history[i].free.file;
                entry->free_line = g_alloc_state.history[i].free.line;
                entry->alive = false;
                break;
            }
        }
    }
    if(allocated) {
        entry->mem = allocated->mem;
        entry->size = allocated->size;
        entry->malloc_file = allocated->file;
        entry->malloc_line = allocated->line;
    }
    B_TRACK_ALLOC_UNLOCK();
    if(allocated) {
        entry->malloc_file = b_shorten_filename(entry->malloc_file);
        entry->free_file = b_shorten_filename(entry->free_file);
        return BERR_SUCCESS;
    }
    return BERR_NOT_SUPPORTED;
}

#undef BKNI_Malloc
void *
BKNI_Malloc(size_t size)
{
    return BKNI_Malloc_tagged(size, NULL, 0);
}

#undef BKNI_Free
void
BKNI_Free(void *ptr)
{
    BKNI_Free_tagged(ptr, NULL, 0);
    return;
}

static void
BKNI_P_TrackAlloc_Init(void)
{
    b_memset32(&g_alloc_state, sizeof(g_alloc_state), 0);
    g_alloc_state.table_size = 4096;
    g_alloc_state.table = BKNI_P_GetTrackAllocEntry_resize(NULL, 0, g_alloc_state.table_size);
    return;
}

static void
BKNI_P_TrackAlloc_Uninit(void)
{
    BKNI_DumpMallocs_Size(0);
    if(g_alloc_state.table) {
        B_TRACK_ALLOC_FREE(g_alloc_state.table);
    }
    return;
}

#else /* BKNI_TRACK_MALLOCS */

void *
BKNI_Malloc(size_t size)
{
    void *ptr;
    ASSERT_NOT_CRITICAL();
    ptr = malloc(size);
    return ptr;
}

void
BKNI_Free(void *ptr)
{
    ASSERT_NOT_CRITICAL();
    free(ptr);
    return;
}

void *
BKNI_Malloc_tagged(size_t size, const char *file, unsigned line)
{
    BSTD_UNUSED(file);
    BSTD_UNUSED(line);
    return BKNI_Malloc(size);
}

void
BKNI_Free_tagged(void *ptr, const char *file, unsigned line)
{
    BSTD_UNUSED(file);
    BSTD_UNUSED(line);
    BKNI_Free(ptr);
    return;
}

BERR_Code BKNI_GetMallocEntryInfo( const void *mem, struct BKNI_MallocEntryInfo *entry)
{
    BSTD_UNUSED(mem);
    BSTD_UNUSED(entry);
    return BERR_NOT_SUPPORTED;
}

static void
BKNI_P_TrackAlloc_Init(void)
{
    return;
}

static void
BKNI_P_TrackAlloc_Uninit(void)
{
    return;
}

void
BKNI_DumpMallocs(void)
{
    return;
}

#endif /* BKNI_TRACK_MALLOCS */


BERR_Code BKNI_Init(void)
{
#if BDBG_DEBUG_BUILD
    if (pthread_self() == 0) {
        /* If this fails, a library outside of magnum has failed. KNI requires this to work. */
        BKNI_Fail();
    }
#endif
    BKNI_P_TrackAlloc_Init();

    return BERR_SUCCESS;
}

/* coverity[+kill]  */
void BKNI_Fail(void)
{
    #if 1 /*BCHP_CHIP==7400*/
    /* JDG - abort() is prodcing bad core dumps on 7400. */
    /* Segmentation faults are not, this will force SIGSEGV */
    BKNI_Printf("BKNI_Fail is intentionally causing a segfault. Please inspect any prior error messages or get a core dump stack trace to determine the cause of failure.\n");
    *(volatile unsigned char *)0;

    /* The declaration includes the 'noreturn' attribute. Some compilers complain that the function actually can return, so we explicity prevent that. */
    while(1);
    #else
    abort();
    #endif
}

BERR_Code BKNI_CreateMutex_tagged(BKNI_MutexHandle *handle, const char *file, int line)
{
    ASSERT_NOT_CRITICAL();

    *handle = (BKNI_MutexHandle)BKNI_Malloc_tagged(sizeof(**handle), file, line);
    if (!*handle) {
        return BERR_TRACE(BERR_OS_ERROR);
    }
    BDBG_OBJECT_SET(*handle, BKNI_Mutex);

    if (pthread_mutex_init(&(*handle)->mutex, NULL)) {
        BDBG_OBJECT_DESTROY(*handle, BKNI_Mutex);
        free(*handle);
        return BERR_TRACE(BERR_OS_ERROR);
    } else {
        return BERR_SUCCESS;
    }
}

void
BKNI_DestroyMutex_tagged(BKNI_MutexHandle handle, const char *file, int line)
{
    ASSERT_NOT_CRITICAL();
    BDBG_OBJECT_ASSERT(handle, BKNI_Mutex);

    pthread_mutex_destroy(&handle->mutex);
    BDBG_OBJECT_DESTROY(handle, BKNI_Mutex);
    BKNI_Free_tagged(handle, file, line);
    return ;
}

BERR_Code
BKNI_TryAcquireMutex_tagged(BKNI_MutexHandle handle,const char *file, int line)
{
    int rc;

    ASSERT_NOT_CRITICAL();
    BDBG_OBJECT_ASSERT(handle, BKNI_Mutex);
    BSTD_UNUSED(file);
    BSTD_UNUSED(line);

    rc = pthread_mutex_trylock(&handle->mutex);
    if (rc==0) {
        return BERR_SUCCESS;
    } else if (rc==EBUSY) {
        return BERR_TIMEOUT;
    } else {
        return BERR_TRACE(BERR_OS_ERROR);
    }
}

BERR_Code
BKNI_AcquireMutex_tagged(BKNI_MutexHandle handle,const char *file, int line)
{
    ASSERT_NOT_CRITICAL();
    BDBG_OBJECT_ASSERT(handle, BKNI_Mutex);
    BSTD_UNUSED(file);
    BSTD_UNUSED(line);


    if (pthread_mutex_lock(&handle->mutex))
        return BERR_TRACE(BERR_OS_ERROR);
    else
        return BERR_SUCCESS;
}
#if !BKNI_TRACK_MALLOCS
BERR_Code BKNI_CreateMutex(BKNI_MutexHandle *handle)
{
    return BKNI_CreateMutex_tagged(handle, NULL, 0);
}

void
BKNI_DestroyMutex(BKNI_MutexHandle handle)
{
    BKNI_DestroyMutex_tagged(handle, NULL, 0);
}
BERR_Code
BKNI_TryAcquireMutex(BKNI_MutexHandle handle)
{
    BKNI_TryAcquireMutex_tagged(handle,NULL,0);
}
BERR_Code
BKNI_AcquireMutex(BKNI_MutexHandle handle)
{
    BKNI_AcquireMutex_tagged(handle,NULL,0);
}

#endif



void
BKNI_ReleaseMutex(BKNI_MutexHandle handle)
{
    ASSERT_NOT_CRITICAL();
    BDBG_OBJECT_ASSERT(handle, BKNI_Mutex);

    if (pthread_mutex_unlock(&handle->mutex)) {
        BDBG_ERR(("pthread_mutex_unlock failed"));
        BDBG_ASSERT(false);
    }
    return ;
}

void
BKNI_EnterCriticalSection(void)
{
    ASSERT_NOT_CRITICAL();

    if (pthread_mutex_lock(&g_csMutex)!=0)
    {
        BDBG_ERR(("pthread_mutex_lock failed"));
        BDBG_ASSERT(false);
        return;
    }

    SET_CRITICAL();
}

void
BKNI_LeaveCriticalSection(void)
{
    ASSERT_CRITICAL();
    CLEAR_CRITICAL();

    if (pthread_mutex_unlock(&g_csMutex))
    {
        BDBG_ERR(("pthread_mutex_unlock failed"));
        BDBG_ASSERT(false);
    }
    return;
}

int
BKNI_Printf(const char *fmt, ...)
{
    va_list arglist;
    int rc;

    va_start( arglist, fmt );
    rc = vfprintf(stderr, fmt, arglist);
    va_end(arglist);

    return rc;
}


int
BKNI_Snprintf(char *str, size_t len, const char *fmt, ...)
{
    va_list arglist;
    int rc;

    va_start( arglist, fmt );
    rc = vsnprintf(str, len, fmt, arglist);
    va_end(arglist);

    return rc;
}

int
BKNI_Vprintf(const char *fmt, va_list ap)
{
    return vfprintf(stderr, fmt, ap);
}

/**
BKNI_Delay impl notes:
This is an incredibly inefficient implementation...which is exactly
the point. Because the linux scheduler has a 10 milisecond clock tick,
this function should not hit the scheduler. It must use a busy loop.
sleep and usleep use the scheduler. nanasleep will use the scheduler
unless the pthread priority is high, which we cannot assume in this function.
Therefore a busy loop with a fine-grain time syscall does the job.
*/
void
BKNI_Delay(unsigned int microsec)
{
    struct timeval start, end;
    unsigned long diff;
    gettimeofday(&start, NULL);

    microsec /= 1000; /* cnvert to msec */
    microsec *= BKNI_EMU_TICKTIME_FACTOR;
    do {
        gettimeofday(&end, NULL);
        diff = (end.tv_sec - start.tv_sec)*1000000 + (end.tv_usec - start.tv_usec);
    } while (diff < microsec);
    return;
}

BERR_Code
BKNI_Sleep(unsigned int millisec)
{
    struct timespec delay;
    struct timespec rem;
    int rc;

    ASSERT_NOT_CRITICAL();

    millisec *= BKNI_EMU_TICKTIME_FACTOR;

    delay.tv_sec = millisec/1000;
    delay.tv_nsec = 1000 * 1000 * (millisec%1000);

    for(;;) {
        rc = nanosleep(&delay, &rem); /* [u]sleep can't be used because it uses SIGALRM */
        if (rc!=0) {
            if (errno==EINTR) {
                delay = rem; /* sleep again */
                continue;
            }
            return BERR_TRACE(BERR_OS_ERROR);
        }
        break; /* done */
    }

    return BERR_SUCCESS;
}


BERR_Code
BKNI_CreateEvent_tagged(BKNI_EventHandle *pEvent, const char *file, int line)
{
    BKNI_EventHandle event;
    int rc;
    BERR_Code result=BERR_SUCCESS;
    /* coverity[var_decl: FALSE] */
    pthread_condattr_t attr;

    ASSERT_NOT_CRITICAL();

    event = BKNI_Malloc_tagged(sizeof(*event), file, line);
    *pEvent = event;
    if ( !event) {
        result = BERR_TRACE(BERR_OS_ERROR);
        goto err_no_memory;
    }
    BDBG_OBJECT_SET(event, BKNI_Event);

    rc = pthread_mutex_init (&event->lock, NULL /* default attributes */);
    if (rc!=0) {
        result = BERR_TRACE(BERR_OS_ERROR);
        goto err_mutex;
    }

    /* coverity[uninit_use_in_call: FALSE] */
    rc = pthread_condattr_init(&attr);
    if (rc!=0) {
        result = BERR_TRACE(BERR_OS_ERROR);
        goto err_condvar;
    }

#if HAS_NPTL
    rc = pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
    if (rc!=0) {
        result = BERR_TRACE(BERR_OS_ERROR);
        goto err_condvar;
    }
#endif

    rc = pthread_cond_init( &event->cond, &attr);
    if (rc!=0) {
        result = BERR_TRACE(BERR_OS_ERROR);
        goto err_condvar;
    }
    event->signal = false;
    event->group = NULL;

    return result;

err_condvar:
    pthread_mutex_destroy(&event->lock);
err_mutex:
    BDBG_OBJECT_DESTROY(event, BKNI_Event);
    free(event);
err_no_memory:
    return result;
}

void
BKNI_DestroyEvent_tagged(BKNI_EventHandle event, const char *file, int line)
{
    int rc;
    BKNI_EventGroupHandle group;

    ASSERT_NOT_CRITICAL();
    BDBG_OBJECT_ASSERT(event, BKNI_Event);
    group = event->group;
    /* At this point, we may have been removed from the group and event->group is NULL.
    This would be poor application code, but KNI should protect itself. */

    if (group) {
        BDBG_WRN(("Event %#x still in the group %#x, removing it", (unsigned)(unsigned long)event, (unsigned)(unsigned long)group));
        rc = pthread_mutex_lock(&group->lock);
        if (rc!=0) {
            BDBG_ERR(("pthread_mutex_lock %d", rc));
            BDBG_ASSERT(false);
        }
        /* if the group does not match, then the caller needs to fix their code. we can't have an event being added & removed from various
        groups and being destroyed at the same time. */
        BDBG_ASSERT(event->group == group);
        BLST_D_REMOVE(&group->members, event, list);
        pthread_mutex_unlock(&group->lock);
    }
    rc = pthread_mutex_destroy(&event->lock);
    if (rc!=0) {
        BDBG_ERR(("pthread_mutex_destroy: %d", rc));
        BDBG_ASSERT(false);
    }
    rc = pthread_cond_destroy(&event->cond);
    if (rc!=0) {
        BDBG_ERR(("pthread_cond_destroy: %d", rc));
        BDBG_ASSERT(false);
    }
    BDBG_OBJECT_DESTROY(event, BKNI_Event);
    BKNI_Free_tagged(event, file, line);
    return ;
}

#if !BKNI_TRACK_MALLOCS
BERR_Code BKNI_CreateEvent(BKNI_EventHandle *pEvent)
{
    return BKNI_CreateEvent_tagged(pEvent, NULL, 0);
}
void BKNI_DestroyEvent(BKNI_EventHandle event)
{
    BKNI_DestroyEvent_tagged(event, NULL, 0);
}
#endif

BERR_Code
BKNI_WaitForEvent(BKNI_EventHandle event, int timeoutMsec)
{
    int rc;
    BERR_Code result = BERR_SUCCESS;
    struct timeval now;
    struct timespec target;

    if(timeoutMsec != BKNI_INFINITE)
    {
        timeoutMsec *= 100*BKNI_EMU_TICKTIME_FACTOR;
    }

    if ( timeoutMsec != 0 )
    {
        ASSERT_NOT_CRITICAL();
    }

    if (timeoutMsec!=0 && timeoutMsec!=BKNI_INFINITE) {
        if (timeoutMsec<100) {
            timeoutMsec=100; /* wait at least 100msec */
        }
        rc = gettimeofday(&now, NULL);
        if (rc!=0) {
            return BERR_TRACE(BERR_OS_ERROR);
        }
        target.tv_nsec = now.tv_usec * 1000 + (timeoutMsec%1000)*1000000;
        target.tv_sec = now.tv_sec + (timeoutMsec/1000);
        if (target.tv_nsec >= 1000000000) {
            target.tv_nsec -=  1000000000;
            target.tv_sec ++;
        }
    }

    rc = pthread_mutex_lock(&event->lock);
    if (rc!=0) {
        return BERR_TRACE(BERR_OS_ERROR);
    }
    if (event->signal) {
        event->signal = false;
        goto done;
    }
    if (timeoutMsec == 0) { /* wait without timeout */
        /* no BERR_TRACE here ??? */
        result = BERR_TIMEOUT;
        goto done;
    }
    do {
        if (timeoutMsec == BKNI_INFINITE) {
            rc = pthread_cond_wait(&event->cond, &event->lock);
        } else {
            rc = pthread_cond_timedwait(&event->cond, &event->lock, &target);
            if (rc==ETIMEDOUT) {
                BDBG_MSG(("BKNI_WaitForEvent(%#x): timeout", (unsigned)(unsigned long)event));
                result = BERR_TIMEOUT;
                goto done;
            }
        }
        if(rc==EINTR) {
            BDBG_MSG(("BKNI_WaitForEvent(%#x): interrupted", (unsigned)(unsigned long)event));
            continue;
        }
        if (rc!=0) {
            result = BERR_TRACE(BERR_OS_ERROR);
            goto done;
        }
    } while(!event->signal);  /* we might have been wokenup and then event has been cleared */

    event->signal = false;
done:
    pthread_mutex_unlock(&event->lock);
    return result;
}

void
BKNI_SetEvent(BKNI_EventHandle event)
{
    int rc;
    BKNI_EventGroupHandle group;

    BDBG_OBJECT_ASSERT(event, BKNI_Event);
    group = event->group;
    /* At this point, we may have been removed from the group and event->group is NULL.
    This is a real possibility because BKNI_SetEvent can be called from an ISR.
    Caching the group pointer allows us to safely unlock still. */

    if (group) {
        rc = pthread_mutex_lock(&group->lock);
        BDBG_ASSERT(0 == rc);
    }
    rc = pthread_mutex_lock(&event->lock);
    if (rc!=0) {
        BDBG_ERR(("pthread_mutex_lock: %d", rc));
        BDBG_ASSERT(false);
    }
    event->signal = true;
    rc = pthread_cond_signal(&event->cond);
    if (rc!=0) {
        BDBG_ERR(("pthread_cond_signal: %d", rc));
        BDBG_ASSERT(false);
    }
    if (group) {
        rc = pthread_cond_signal(&group->cond);
        if (rc!=0) {
            BDBG_ERR(("pthread_cond_signal: %d, ignored", rc));
        }
    }
    rc = pthread_mutex_unlock(&event->lock);
    if (rc!=0) {
        BDBG_ERR(("pthread_mutex_unlock: %d", rc));
        BDBG_ASSERT(false);
    }
    if (group) {
        pthread_mutex_unlock(&group->lock);
    }
    return ;
}

void
BKNI_ResetEvent(BKNI_EventHandle event)
{
    int rc;

    BDBG_OBJECT_ASSERT(event, BKNI_Event);
    rc = pthread_mutex_lock(&event->lock);
    if (rc!=0) {
        BDBG_ERR(("pthread_mutex_lock: %d", rc));
        BDBG_ASSERT(false);
    }
    event->signal = false ;
    rc = pthread_mutex_unlock(&event->lock);
    if (rc!=0) {
        BDBG_ERR(("pthread_mutex_unlock: %d", rc));
        BDBG_ASSERT(false);
    }
    return ;
}

int
BKNI_Vsnprintf(char *s, size_t n, const char *fmt, va_list ap)
{
    return vsnprintf(s, n, fmt, ap);
}

BERR_Code
BKNI_CreateEventGroup(BKNI_EventGroupHandle *pGroup)
{
    int rc;
    BKNI_EventGroupHandle group;
    BERR_Code result;
    /* coverity[var_decl: FALSE] */
    pthread_condattr_t attr;

    ASSERT_NOT_CRITICAL();

    group = malloc(sizeof(*group));
    if (!group) {
        result = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_no_memory;
    }
    BDBG_OBJECT_SET(group, BKNI_EventGroup);

    BLST_D_INIT(&group->members);
    rc = pthread_mutex_init (&group->lock, NULL /* default attributes */);
    if (rc!=0) {
        result = BERR_TRACE(BERR_OS_ERROR);
        goto err_mutex;
    }

    /* coverity[uninit_use_in_call: FALSE] */
    rc = pthread_condattr_init(&attr);
    if (rc!=0) {
        result = BERR_TRACE(BERR_OS_ERROR);
        goto err_condvar;
    }

#if HAS_NPTL
    rc = pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
    if (rc!=0) {
        result = BERR_TRACE(BERR_OS_ERROR);
        goto err_condvar;
    }
#endif

    rc = pthread_cond_init( &group->cond, &attr);
    if (rc!=0) {
        result = BERR_TRACE(BERR_OS_ERROR);
        goto err_condvar;
    }
    *pGroup = group;

    return BERR_SUCCESS;

err_condvar:
    pthread_mutex_destroy(&group->lock);
err_mutex:
    BDBG_OBJECT_DESTROY(group, BKNI_EventGroup);
    free(group);
err_no_memory:
    return result;
}

void
BKNI_DestroyEventGroup(BKNI_EventGroupHandle group)
{
    int rc;
    BKNI_EventHandle event;

    ASSERT_NOT_CRITICAL();
    BDBG_OBJECT_ASSERT(group, BKNI_EventGroup);

    rc = pthread_mutex_lock(&group->lock);
    if (rc<0) {
        BDBG_ERR(("pthread_mutex_lock failed, rc=%d", rc));
        BDBG_ASSERT(false);
    }

    while(NULL != (event=BLST_D_FIRST(&group->members)) ) {
        BDBG_ASSERT(event->group == group);
        event->group = NULL;
        BLST_D_REMOVE_HEAD(&group->members, list);
    }
    pthread_mutex_unlock(&group->lock);
    /* NOTE: to avoid this race condition, app must ensure that no SetEvent for this group is pending at this time */
    pthread_mutex_destroy(&group->lock);
    pthread_cond_destroy(&group->cond);
    BDBG_OBJECT_DESTROY(group, BKNI_EventGroup);
    free(group);
    return;
}


BERR_Code
BKNI_AddEventGroup(BKNI_EventGroupHandle group, BKNI_EventHandle event)
{
    int rc;
    BERR_Code result = BERR_SUCCESS;

    ASSERT_NOT_CRITICAL();
    BDBG_OBJECT_ASSERT(group, BKNI_EventGroup);
    BDBG_OBJECT_ASSERT(event, BKNI_Event);

    /* IMPORTANT: group lock shall be acquired before event lock */
    rc = pthread_mutex_lock(&group->lock);
    if (rc!=0) {
        BDBG_ERR(("pthread_mutex_lock failed, rc=%d", rc));
        BDBG_ASSERT(false);
    }
    rc = pthread_mutex_lock(&event->lock);
    if (rc!=0) {
        BDBG_ERR(("pthread_mutex_lock failed, rc=%d", rc));
        BDBG_ASSERT(false);
    }
    if (event->group != NULL) {
        BDBG_ERR(("Event %#x already connected to the group %#x", (unsigned)(unsigned long)event, (unsigned)(unsigned long)group));
        result = BERR_TRACE(BERR_OS_ERROR);
    } else {
        BLST_D_INSERT_HEAD(&group->members, event, list);
        event->group = group;
        if (event->signal) {
            /* signal condition if signal already set */
            pthread_cond_signal(&group->cond);
        }
    }
    rc = pthread_mutex_unlock(&event->lock);
    if (rc!=0) {
        BDBG_ERR(("pthread_mutex_unlock failed, rc=%d", rc));
        BDBG_ASSERT(false);
    }
    rc = pthread_mutex_unlock(&group->lock);
    if (rc!=0) {
        BDBG_ERR(("pthread_mutex_unlock failed, rc=%d", rc));
        BDBG_ASSERT(false);
    }
    return result;
}

BERR_Code
BKNI_RemoveEventGroup(BKNI_EventGroupHandle group, BKNI_EventHandle event)
{
    int rc;
    BERR_Code result = BERR_SUCCESS;

    ASSERT_NOT_CRITICAL();
    BDBG_OBJECT_ASSERT(group, BKNI_EventGroup);
    BDBG_OBJECT_ASSERT(event, BKNI_Event);

    rc = pthread_mutex_lock(&group->lock);
    if (rc!=0) {
        BDBG_ERR(("pthread_mutex_lock failed, rc=%d", rc));
        BDBG_ASSERT(false);
    }
    rc = pthread_mutex_lock(&event->lock);
    if (rc!=0) {
        BDBG_ERR(("pthread_mutex_lock failed, rc=%d", rc));
        BDBG_ASSERT(false);
    }
    if (event->group != group) {
        BDBG_ERR(("Event %#x doesn't belong to the group %#x", event, group));
        result = BERR_TRACE(BERR_OS_ERROR);
    } else {
        BLST_D_REMOVE(&group->members, event, list);
        event->group = NULL;
    }
    rc = pthread_mutex_unlock(&event->lock);
    if (rc!=0) {
        BDBG_ERR(("pthread_mutex_unlock failed, rc=%d", rc));
        BDBG_ASSERT(false);
    }
    rc = pthread_mutex_unlock(&group->lock);
    if (rc!=0) {
        BDBG_ERR(("pthread_mutex_unlock failed, rc=%d", rc));
        BDBG_ASSERT(false);
    }
    return result;
}

static unsigned
group_get_events(BKNI_EventGroupHandle group, BKNI_EventHandle *events, unsigned max_events)
{
    BKNI_EventHandle ev;
    int rc;
    unsigned event;

    BDBG_OBJECT_ASSERT(group, BKNI_EventGroup);

    for(event=0, ev=BLST_D_FIRST(&group->members); ev && event<max_events ; ev=BLST_D_NEXT(ev, list)) {
        BDBG_OBJECT_ASSERT(ev, BKNI_Event);
        rc = pthread_mutex_lock(&ev->lock);
        if (rc!=0) {
            BDBG_ERR(("pthread_mutex_lock failed, rc=%d", rc));
            BDBG_ASSERT(false);
        }
        if (ev->signal) {
            ev->signal = false;
            events[event] = ev;
            event++;
        }
        rc = pthread_mutex_unlock(&ev->lock);
        if (rc!=0) {
            BDBG_ERR(("pthread_mutex_unlock failed, rc=%d", rc));
            BDBG_ASSERT(false);
        }
    }
    return event;
}

BERR_Code
BKNI_WaitForGroup(BKNI_EventGroupHandle group, int timeoutMsec, BKNI_EventHandle *events, unsigned max_events, unsigned *nevents)
{
    int rc;
    struct timeval now;
    struct timespec target;
    BERR_Code result = BERR_SUCCESS;

    ASSERT_NOT_CRITICAL();

    timeoutMsec *= BKNI_EMU_TICKTIME_FACTOR;

    if (max_events<1) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    if (timeoutMsec!=0 && timeoutMsec!=BKNI_INFINITE) {
        if (timeoutMsec<100) {
            timeoutMsec=100; /* wait at least 100msec */
        }
        rc = gettimeofday(&now, NULL);
        if (rc!=0) {
            return BERR_TRACE(BERR_OS_ERROR);
        }
        target.tv_nsec = now.tv_usec * 1000 + (timeoutMsec%1000)*1000000;
        target.tv_sec = now.tv_sec + (timeoutMsec/1000);
        if (target.tv_nsec >= 1000000000) {
            target.tv_nsec -=  1000000000;
            target.tv_sec ++;
        }
    }
    rc = pthread_mutex_lock(&group->lock);
    if (rc!=0) {
        BDBG_ERR(("pthread_mutex_lock failed, rc=%d", rc));
        BDBG_ASSERT(false);
    }
    for(;;) {
        *nevents = group_get_events(group, events, max_events);
        if (*nevents) {
            goto done;
        }
        if (timeoutMsec == BKNI_INFINITE) {
            rc = pthread_cond_wait(&group->cond, &group->lock);
        } else {
            rc = pthread_cond_timedwait(&group->cond, &group->lock, &target);
            if (rc==ETIMEDOUT) {
                BDBG_MSG(("BKNI_WaitForGroup(%#x): timeout", (unsigned long)group));
                result = BERR_TIMEOUT;
                goto done;
            }
        }
        if (rc!=0) {
            result = BERR_TRACE(BERR_OS_ERROR);
            goto done;
        }
    }

done:
    rc = pthread_mutex_unlock(&group->lock);
    if (rc!=0) {
        BDBG_ERR(("pthread_mutex_unlock failed, rc=%d", rc));
        BDBG_ASSERT(false);
    }
    return result;
}

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

void
BKNI_AssertIsrContext_isr(const char *filename, unsigned lineno)
{
    if ( !CHECK_CRITICAL() ) {
        BDBG_P_AssertFailed("Not in critical section", filename, lineno);
    }
}

void BKNI_Uninit(void)
{
    BKNI_P_TrackAlloc_Uninit();
    return;
}
