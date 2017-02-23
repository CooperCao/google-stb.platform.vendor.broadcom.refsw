/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
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

 ******************************************************************************/
#if NEXUS_HAS_SAGE
#include "nxclient.h"
#include "nexus_platform.h"
#include "nexus_base_mmap.h"
#include "nexus_sage.h"
#include "bstd.h"
#include "bkni.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

/* This nxclient app will connect to server and show current heap
* configuration. It can also toggle URR's between secure and non-secure */

BDBG_MODULE(heap_toggle);

enum testOp_t {
    eTestOpQueryOnly,
    eTestOpSetUrr,
    eTestOpSetGfx,
    eTestOpSetExt,
    eTestOpSetAll,
    eTestOpLoop,
    eTestOpDebug,
    eTestOpVerify
};

enum toggleOp_t {
    eToggleIgnore,
    eToggleEnable,
    eToggleDisable
};

struct Heap_Info_t
{
    NEXUS_HeapHandle heaps[NEXUS_MAX_HEAPS];
    NEXUS_MemoryStatus status[NEXUS_MAX_HEAPS];
    uint8_t index[NEXUS_MAX_HEAPS];
    uint8_t number;
    bool secureGfx;
    bool secureExt;
};

static struct Heap_Info_t heapInfo;
static int iterations = -1;

static const char *memory_type_to_string(unsigned memoryType, char *buf, size_t buf_size)
{
    if(memoryType==0)
    {
        BKNI_Snprintf(buf, buf_size, "%s", "eDevice");
    }
    else
    {
        static const struct {unsigned value; const char *name;} g_mapping[] = {
            {NEXUS_MEMORY_TYPE_DRIVER_CACHED,"DRIVER"},
            {NEXUS_MEMORY_TYPE_APPLICATION_CACHED,"APP"},
            {NEXUS_MEMORY_TYPE_SECURE,"SECURE"},
            {NEXUS_MEMORY_TYPE_SECURE_OFF, "RUNTIME/UNSECURE"},
            {NEXUS_MEMORY_TYPE_RESERVED,"RESERVED"},
            {NEXUS_MEMORY_TYPE_MANAGED,"MANAGED"},
            {NEXUS_MEMORY_TYPE_NOT_MAPPED,"NOT_MAPPED"},
            {NEXUS_MEMORY_TYPE_HIGH_MEMORY,"HIGH_MEMORY"},
            {NEXUS_MEMORY_TYPE_ONDEMAND_MAPPED,"ONDEMAND_MAPPED"}};
        unsigned n = 0, i;
        for (i=0;i<sizeof(g_mapping)/sizeof(g_mapping[0]) && n<buf_size;i++) {
            if (memoryType & g_mapping[i].value) {
                n += BKNI_Snprintf(&buf[n], buf_size-n, "%s%s", n?",":"", g_mapping[i].name);
            }
        }
    }
    return buf;
}

static const char *heap_type_to_string(unsigned heapType, char *buf, size_t buf_size)
{
    static const struct {unsigned value; const char *name;} g_mapping[] = {
        {NEXUS_HEAP_TYPE_MAIN,"MAIN   "},
        {NEXUS_HEAP_TYPE_PICTURE_BUFFERS,"PICTURE"},
        {NEXUS_HEAP_TYPE_SAGE_RESTRICTED_REGION,"SAGE   "},
        {NEXUS_HEAP_TYPE_COMPRESSED_RESTRICTED_REGION,"CRR    "},
        {NEXUS_HEAP_TYPE_GRAPHICS,"GFX    "},
        {NEXUS_HEAP_TYPE_SECONDARY_GRAPHICS,"GFX2   "},
        {NEXUS_HEAP_TYPE_PICTURE_BUFFER_EXT,"PIC EXT"},
        {NEXUS_HEAP_TYPE_SECURE_GRAPHICS,"SEC GFX"},
        {NEXUS_HEAP_TYPE_EXPORT_REGION,"XRR    "}};
    unsigned n = 0, i;
    for (i=0;i<sizeof(g_mapping)/sizeof(g_mapping[0]) && n<buf_size;i++) {
        if (heapType & g_mapping[i].value) {
            n += BKNI_Snprintf(&buf[n], buf_size-n, "%s%s", n?",":"", g_mapping[i].name);
        }
    }
    if(n==0)
    {
        n += BKNI_Snprintf(&buf[n], buf_size-n, "N/A    ");
    }

    return buf;
}

static void print_heap(unsigned i, NEXUS_HeapHandle heap, const NEXUS_PlatformConfiguration *platformConfig,
    unsigned *total, unsigned *unsecure, unsigned *secure)
{
    NEXUS_MemoryStatus status;
    char buf[2][128];
    unsigned size_percentage;
    NEXUS_Error rc;
    unsigned serverHeapIndex;
    rc = NEXUS_Heap_GetStatus(heap, &status);
    if (rc) return;
    size_percentage = status.size/100; /* avoid overflow by dividing size by 100 to get percentage */
    for (serverHeapIndex=0;serverHeapIndex<NEXUS_MAX_HEAPS;serverHeapIndex++) {
        if (platformConfig->heap[serverHeapIndex] == heap) break;
    }
    printf("%-2d\t" BDBG_UINT64_FMT "\t%d\t0x%08x(%3dMB)@0x%08x\t%3d%%/%3d%%\t%u\t%s : %s\n",
        i, BDBG_UINT64_ARG(status.offset), status.memcIndex, status.size, status.size/(1024*1024), (unsigned)status.addr,
        size_percentage?(status.size-status.free)/size_percentage:0,
        size_percentage?status.highWatermark/size_percentage:0,
        serverHeapIndex,
        heap_type_to_string(status.heapType, buf[1], sizeof(buf[1])),
        memory_type_to_string(status.memoryType, buf[0], sizeof(buf[0])));

    if(status.heapType & NEXUS_HEAP_TYPE_PICTURE_BUFFERS)
    {
        *total+=status.size;
        if(status.memoryType & NEXUS_MEMORY_TYPE_SECURE)
        {
            *secure+=status.size;
        }
        else
        {
            *unsecure+=status.size;
        }
    }
}

static void print_heaps(void)
{
    unsigned i;
    NEXUS_PlatformConfiguration platformConfig;
    unsigned picture_heap_total=0;
    unsigned picture_heap_unsecure=0;
    unsigned picture_heap_secure=0;

    NEXUS_Platform_GetConfiguration(&platformConfig);
    printf("heap\toffset\t\tmemc\tsize@offset\t\t\tused/peak\tserver\tinfo\n");
    for (i=0;i<heapInfo.number;i++) {
        print_heap(heapInfo.index[i], heapInfo.heaps[i], &platformConfig, &picture_heap_total, &picture_heap_unsecure, &picture_heap_secure);
    }
    printf("PICTURE HEAP TOTAL: 0x%08x (%dMB)\n", picture_heap_total, picture_heap_total>>20);
    printf("PICTURE HEAP (SECURE): 0x%08x (%dMB)\n", picture_heap_secure, picture_heap_secure>>20);
    printf("PICTURE HEAP (UNSECURE): 0x%08x (%dMB)\n", picture_heap_unsecure, picture_heap_unsecure>>20);
}

static void print_secure_status(void)
{
    unsigned i;
    NEXUS_Error rc;
    NEXUS_HeapRuntimeSettings heapSettings;
    NEXUS_SageStatus sageStatus;
    NEXUS_MemoryStatus status;

    for (i=0;i<heapInfo.number;i++)
    {
        /* Only for secure buffers */
        if(!(heapInfo.status[i].memoryType & NEXUS_MEMORY_TYPE_SECURE))
            continue;

        NEXUS_Platform_GetHeapRuntimeSettings(heapInfo.heaps[i], &heapSettings);

        printf("HEAP[%d] - %s\n", heapInfo.index[i], heapSettings.secure ? "SECURE" : "NON-SECURE");
    }

    rc=NEXUS_Sage_GetStatus(&sageStatus);
    if(rc!=NEXUS_SUCCESS)
    {
        printf("Failed to get SAGE URR status!\n");
    }
    else
    {
        printf("SAGE URR - %s\n", sageStatus.urr.secured ? "SECURE" : "NON-SECURE");
    }
}

void show_usage(char *app)
{
    printf("%s usage:\n", app);
    printf("\t -P \t Set picture buffer(s) to secure\n");
    printf("\t -p \t Set picture buffer(s) to unsecure\n");
    printf("\t -G \t Set secure GFX to secure\n");
    printf("\t -g \t Set secure GFX to unsecure\n");
    printf("\t -E \t Set secure extension to secure\n");
    printf("\t -e \t Set secure extension to unsecure\n");
    printf("\t -A \t set all to secure\n");
    printf("\t -a \t set all to unsecure\n");
    printf("\t -l \t Infinite loop (Press enter key to quit)\n");
    printf("\t -t \t Test/Verify scrubbing operation\n");
}

/* Cache the heaps. This prevents a lot of spew */
static unsigned cacheHeap(void)
{
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_MemoryStatus status;
    int i;
    NEXUS_Error rc;

    if(!heapInfo.number)
    {
        NEXUS_Platform_GetConfiguration(&platformConfig);
        for (i=0;i<NEXUS_MAX_HEAPS;i++)
        {
            if(!platformConfig.heap[i])
                continue;

            rc=NEXUS_Heap_GetStatus(platformConfig.heap[i], &status);
            if(rc)
                continue;

            /* Only care about secure buffers and/or picture buffers */
            if(!(status.memoryType & NEXUS_MEMORY_TYPE_SECURE) && !(status.heapType & NEXUS_HEAP_TYPE_PICTURE_BUFFERS))
                continue;

            if(status.heapType & NEXUS_HEAP_TYPE_SECURE_GRAPHICS)
            {
                heapInfo.secureGfx=true;
            }

            if(status.heapType & NEXUS_HEAP_TYPE_PICTURE_BUFFER_EXT)
            {
                heapInfo.secureExt=true;
            }

            heapInfo.heaps[heapInfo.number]=platformConfig.heap[i];
            heapInfo.status[heapInfo.number]=status;
            heapInfo.index[heapInfo.number]=i;
            heapInfo.number++;
        }
    }

    if(!heapInfo.number)
        return -1;

    return 0;
}

static NEXUS_Error setHeapsSecure(enum toggleOp_t urrSecure, enum toggleOp_t gfxSecure, enum toggleOp_t extSecure)
{
    int i;
    NEXUS_HeapRuntimeSettings heapSettings;
    NEXUS_Error rc;
    NEXUS_Error ret=NEXUS_SUCCESS;
    enum toggleOp_t action;
    char buf[128];
    bool markSecure;

    for (i=0;i<heapInfo.number;i++)
    {
        /* Only for secure buffers */
        if(!(heapInfo.status[i].memoryType & NEXUS_MEMORY_TYPE_SECURE))
        {
            continue;
        }

        action=eToggleIgnore;
        if(heapInfo.status[i].heapType & NEXUS_HEAP_TYPE_PICTURE_BUFFERS)
        {
            action=urrSecure;
        }
        else if(heapInfo.status[i].heapType & NEXUS_HEAP_TYPE_SECURE_GRAPHICS)
        {
            action=gfxSecure;
        }
        else if(heapInfo.status[i].heapType & NEXUS_HEAP_TYPE_PICTURE_BUFFER_EXT)
        {
            action=extSecure;
        }

        if(action==eToggleIgnore)
        {
            continue;
        }

        markSecure=(action==eToggleEnable) ? true : false;

        /* This heap is a secure picture buffer heap, update it's status */
        NEXUS_Platform_GetHeapRuntimeSettings(heapInfo.heaps[i], &heapSettings);

        if(heapSettings.secure==markSecure)
            continue;

        BDBG_WRN(("Updating HEAP[%d] (%s) from %s to %s", heapInfo.index[i],
                  heap_type_to_string(heapInfo.status[i].heapType, buf, sizeof(buf)),
                  heapSettings.secure ? "SECURE" : "CLEAR",
                  markSecure ? "SECURE" : "CLEAR"));

        heapSettings.secure=markSecure;
        rc=NEXUS_Platform_SetHeapRuntimeSettings(heapInfo.heaps[i], &heapSettings);
        if(rc)
        {
            ret=BERR_TRACE(rc);
            BDBG_ERR(("Failed NEXUS_Platform_SetHeapRuntimeSettings (0x%x)", rc));
            goto EXIT;
        }
    }

EXIT:
    return ret;
}

static inline struct timeval timeDelta(struct timeval start, struct timeval stop)
{
    struct timeval delta;

    delta.tv_sec=stop.tv_sec-start.tv_sec;
    if(stop.tv_sec==start.tv_sec)
    {
        delta.tv_usec=stop.tv_usec-start.tv_usec;
    }
    else
    {
        delta.tv_usec=1000000-start.tv_usec;
        delta.tv_usec+=stop.tv_usec;
        if(delta.tv_usec>=1000000)
        {
            delta.tv_usec-=1000000;
            delta.tv_sec++;
        }
    }

    return delta;
}

void *toggle_thread(void *context)
{
    bool *done=(bool *)context;
    bool secure=false;
    unsigned delay;
    struct timeval start, stop, delta, worst;
    uint32_t count=0;
    NEXUS_Error rc=NEXUS_SUCCESS;

    memset(&worst, 0, sizeof(worst));

    srand(time(NULL)); /* seed */

    while(!*done)
    {
        delay=rand() % 2000;
        gettimeofday(&start, NULL);

        if(!secure)
        {
            /* If disabling, must disable gfx/ext first */
            rc=setHeapsSecure(eToggleIgnore, eToggleDisable, eToggleDisable);
            if(rc!=NEXUS_SUCCESS)
            {
                goto EXIT;
            }
            rc=setHeapsSecure(eToggleDisable, eToggleIgnore, eToggleIgnore);
            if(rc!=NEXUS_SUCCESS)
            {
                goto EXIT;
            }
        }
        else
        {
            /* If enabling, must enable urr first */
            rc=setHeapsSecure(eToggleEnable, eToggleIgnore, eToggleIgnore);
            if(rc!=NEXUS_SUCCESS)
            {
                goto EXIT;
            }
            rc=setHeapsSecure(eToggleIgnore, eToggleEnable, eToggleEnable);
            if(rc!=NEXUS_SUCCESS)
            {
                goto EXIT;
            }
        }

        gettimeofday(&stop, NULL);
        delta=timeDelta(start, stop);
        if(delta.tv_sec>worst.tv_sec)
        {
            worst=delta;
        } else if(delta.tv_sec==worst.tv_sec)
        {
            if(delta.tv_usec>worst.tv_usec)
            {
                worst=delta;
            }
        }

        secure=!secure;
        count++;
        printf("Toggle[%d] (to %s) took %u.%07u seconds (Worst: %u.%07u)\n", count, !secure ? "secure" : "UNsecure",
               (unsigned int)delta.tv_sec, (unsigned int)delta.tv_usec, (unsigned int)worst.tv_sec, (unsigned int)worst.tv_usec);
        BKNI_Sleep(delay);

        if(iterations>=0)
        {
            iterations--;
            if(iterations==0)
            {
                *done=true;
            }
        }
    }

EXIT:
    printf("Exiting toggle loop (%u toggle commands)\n", count);
    printf("Worst toggle time was %u.%07u seconds\n", (unsigned int)worst.tv_sec, (unsigned int)worst.tv_usec);

    return (void *)rc;
}

static int scrub_verify(void)
{
    int memfd;
    int i, j;
    unsigned char *addr;
    NEXUS_Error rc;
    unsigned char pattern[] = {
        0x00, 0x01, 0x02, 0x03,
        0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b,
        0x0c, 0x0d, 0x0e, 0x0f,
        0x21, 0x10, 0x20, 0x30,
        0x40, 0x50, 0x60, 0x70,
        0x80, 0x90, 0xa0, 0xb0,
        0xc0, 0xd0, 0xe0, 0xf0
    };
    unsigned patternSize=sizeof(pattern);

    memfd = open("/dev/mem", O_RDWR|O_SYNC);
    if (memfd < 0)
    {
        fprintf(stderr, "Cannot open /dev/mem\n");
        return -1;
    }

    /* Make sure in URR off mode */
    /* When disabling, must disable gfx/ext first */
    rc=setHeapsSecure(eToggleIgnore, eToggleDisable, eToggleDisable);
    if(rc!=NEXUS_SUCCESS)
    {
        goto EXIT;
    }
    rc=setHeapsSecure(eToggleDisable, eToggleIgnore, eToggleIgnore);
    if(rc!=NEXUS_SUCCESS)
    {
        goto EXIT;
    }

    /* Write pattern to URR */
    for (i=0;i<heapInfo.number;i++)
    {
        /* Only for secure buffers  */
        if(!(heapInfo.status[i].memoryType & NEXUS_MEMORY_TYPE_SECURE))
            continue;

        /* Only for picture buffers, secure gfx, or secure extension */
        if(!(heapInfo.status[i].heapType & NEXUS_HEAP_TYPE_PICTURE_BUFFERS) &&
           !(heapInfo.status[i].heapType & NEXUS_HEAP_TYPE_SECURE_GRAPHICS) &&
           !(heapInfo.status[i].heapType & NEXUS_HEAP_TYPE_PICTURE_BUFFER_EXT))
        {
            continue;
        }

        if(heapInfo.status[i].size % patternSize)
        {
            printf("Please adjust alignment of pattern..\n");
            printf("patternSize is %d, secure heap size is %d\n", patternSize, heapInfo.status[i].size);
            goto EXIT;
        }

        addr = (unsigned char *)mmap(NULL, heapInfo.status[i].size, PROT_READ|PROT_WRITE, MAP_SHARED, memfd, heapInfo.status[i].offset);
        if(addr == MAP_FAILED)
        {
            printf("Failed to map 0x%x@0x%x\n", heapInfo.status[i].size, (uint32_t)heapInfo.status[i].offset);
            continue;
        }

        printf("Writing test pattern to heap[%d] (0x%x @ 0x%016llx)\n", heapInfo.index[i], heapInfo.status[i].size, heapInfo.status[i].offset);
        for(j=0;j<heapInfo.status[i].size;j+=patternSize)
        {
            memcpy(&addr[j], pattern, patternSize);
        }
        NEXUS_FlushCache(addr, heapInfo.status[i].size);

        /* Verify write access */
        if(memcmp(&addr[0], pattern, patternSize)!=0)
        {
            munmap((void *)addr, heapInfo.status[i].size);
            printf("Writing test pattern FAILED. Cannot continue\n");
            goto EXIT;
        }

        munmap((void *)addr, heapInfo.status[i].size);
    }

    /* Toggle URR to on mode */
    /* When enable, must enable URR first */
    rc=setHeapsSecure(eToggleEnable, eToggleIgnore, eToggleIgnore);
    if(rc!=NEXUS_SUCCESS)
    {
        goto EXIT;
    }
    rc=setHeapsSecure(eToggleIgnore, eToggleEnable, eToggleEnable);
    if(rc!=NEXUS_SUCCESS)
    {
        goto EXIT;
    }

    /* Toggle back to off (will scrub) */
    /* When disabling, must disable gfx/ext first */
    rc=setHeapsSecure(eToggleIgnore, eToggleDisable, eToggleDisable);
    if(rc!=NEXUS_SUCCESS)
    {
        goto EXIT;
    }
    rc=setHeapsSecure(eToggleDisable, eToggleIgnore, eToggleIgnore);
    if(rc!=NEXUS_SUCCESS)
    {
        goto EXIT;
    }

    /* Verify pattern is gone */
    for (i=0;i<heapInfo.number;i++)
    {
        /* Only for secure buffers */
        if(!(heapInfo.status[i].memoryType & NEXUS_MEMORY_TYPE_SECURE))
            continue;

        /* Only for picture buffers, secure gfx, or secure extension */
        if(!(heapInfo.status[i].heapType & NEXUS_HEAP_TYPE_PICTURE_BUFFERS) &&
           !(heapInfo.status[i].heapType & NEXUS_HEAP_TYPE_SECURE_GRAPHICS) &&
           !(heapInfo.status[i].heapType & NEXUS_HEAP_TYPE_PICTURE_BUFFER_EXT))
        {
            continue;
        }

        addr = (unsigned char *)mmap(NULL, heapInfo.status[i].size, PROT_READ|PROT_WRITE, MAP_SHARED, memfd, heapInfo.status[i].offset);
        if(addr == MAP_FAILED)
        {
            printf("Failed to map 0x%x@0x%x\n", heapInfo.status[i].size, (uint32_t)heapInfo.status[i].offset);
            continue;
        }

        NEXUS_FlushCache(addr, heapInfo.status[i].size);

        printf("Checking for test pattern in secure buffer (0x%x @ 0x%016llx)\n", heapInfo.status[i].size, heapInfo.status[i].offset);
        for(j=0;j<heapInfo.status[i].size;j+=patternSize)
        {
            int k;
            if(memcmp(&addr[j], pattern, patternSize)==0)
            {
                printf("STALE DATA PRESENT!!!!! SCRUBBING FAILED! (0x%016llx)\n", heapInfo.status[i].offset+j);

                printf("DATA:\n\t0x");
                for(k=0;k<patternSize;k++)
                    printf("%02x", addr[j+k]);
                printf("\n\t0x");
                for(k=0;k<patternSize;k++)
                    printf("%02x", pattern[k]);
                printf("\n");

                rc=BERR_TRACE(NEXUS_UNKNOWN);
                break;
            }
        }

        munmap((void *)addr, heapInfo.status[i].size);
    }

EXIT:
    close(memfd);

    printf("Scrub verify operation %s\n", (rc==NEXUS_SUCCESS) ? "PASSES" : "FAILS");

    return rc;
}

void debugOutput(void)
{
    int i;

    for (i=0;i<heapInfo.number;i++) {
        if(heapInfo.status[i].heapType & NEXUS_HEAP_TYPE_COMPRESSED_RESTRICTED_REGION)
        {
            printf("CRR 0x%016llx 0x%08x\n", heapInfo.status[i].offset, heapInfo.status[i].size);
            continue;
        }
        if(heapInfo.status[i].heapType & NEXUS_HEAP_TYPE_EXPORT_REGION)
        {
            printf("XRR 0x%016llx 0x%08x\n", heapInfo.status[i].offset, heapInfo.status[i].size);
            continue;
        }
        if(heapInfo.status[i].heapType & NEXUS_HEAP_TYPE_SECURE_GRAPHICS)
        {
            printf("GFX%d 0x%016llx 0x%08x\n", heapInfo.status[i].memcIndex, heapInfo.status[i].offset, heapInfo.status[i].size);
            continue;
        }
        if(heapInfo.status[i].heapType & NEXUS_HEAP_TYPE_PICTURE_BUFFER_EXT)
        {
            printf("EXT%d 0x%016llx 0x%08x\n", heapInfo.status[i].memcIndex, heapInfo.status[i].offset, heapInfo.status[i].size);
            continue;
        }
        if((heapInfo.status[i].heapType & NEXUS_HEAP_TYPE_PICTURE_BUFFERS) && (heapInfo.status[i].memoryType & NEXUS_MEMORY_TYPE_SECURE))
        {
            printf("URR%d 0x%016llx 0x%08x\n", heapInfo.status[i].memcIndex, heapInfo.status[i].offset, heapInfo.status[i].size);
            continue;
        }
    }
}

int main(int argc, char **argv)
{
    int i;
    NxClient_JoinSettings joinSettings;
    NEXUS_Error rc = NEXUS_SUCCESS;
    enum testOp_t op = eTestOpQueryOnly;
    pthread_t thread;
    bool done=false;
    enum toggleOp_t toggle=eToggleIgnore;

    for(i=1;i<argc;i++)
    {
        if(argv[i][0]=='-')
        {
            switch(argv[i][1])
            {
                case 'h':
                    show_usage(argv[0]);
                    return 0;
                    break;
                case 'A':
                    op = eTestOpSetAll;
                    toggle = eToggleEnable;
                    break;
                case 'a':
                    op = eTestOpSetAll;
                    toggle = eToggleDisable;
                    break;
                case 'P':
                    op = eTestOpSetUrr;
                    toggle = eToggleEnable;
                    break;
                case 'p':
                    op = eTestOpSetUrr;
                    toggle = eToggleDisable;
                    break;
                case 'G':
                    op = eTestOpSetGfx;
                    toggle = eToggleEnable;
                    break;
                case 'g':
                    op = eTestOpSetGfx;
                    toggle = eToggleDisable;
                    break;
                case 'E':
                    op = eTestOpSetExt;
                    toggle = eToggleEnable;
                    break;
                case 'e':
                    op = eTestOpSetExt;
                    toggle = eToggleDisable;
                    break;
                case 'l': /* Infinite loop */
                    op = eTestOpLoop;
                    break;
                case 'd': /* internal test output. Non-described option */
                    op = eTestOpDebug;
                    break;
                case 'c': /* finite loop */
                    op = eTestOpLoop;

                    iterations = strtoul(argv[++i], NULL, 0);
                    if(iterations <= 0)
                    {
                        printf("Supply an iteration count greater than 0");
                        return -1;
                    }
                    else
                    {
                        printf("Will run for %d iterations", iterations );
                    }
                    break;
                case 't': /* Test scrubbing */
                    op = eTestOpVerify;
                    break;
                default:
                    break;
            }
        }
    }

    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s", argv[0]);
    joinSettings.mode = NEXUS_ClientMode_eVerified;
    rc = NxClient_Join(&joinSettings);
    if (rc) return -1;

    if(cacheHeap())
    {
        printf("Failed to obtain secure picture buffer heap information\n");
        printf("or no secure picture buffers exist\n");
        return -1;
    }

    if(op==eTestOpDebug)
    {
        debugOutput();
        goto EXIT;
    }

    print_heaps();
    print_secure_status();

    switch(op)
    {
        case eTestOpSetAll:
            if(toggle==eToggleEnable)
            {
                /* If enabling, must enable urr first */
                rc=setHeapsSecure(toggle, eToggleIgnore, eToggleIgnore);
                if(rc!=NEXUS_SUCCESS)
                {
                    goto EXIT;
                }
                rc=setHeapsSecure(eToggleIgnore, toggle, toggle);
                if(rc!=NEXUS_SUCCESS)
                {
                    goto EXIT;
                }
            }
            else
            {
                /* If disabling, must disable gfx/ext first */
                rc=setHeapsSecure(eToggleIgnore, toggle, toggle);
                if(rc!=NEXUS_SUCCESS)
                {
                    goto EXIT;
                }
                rc=setHeapsSecure(toggle, eToggleIgnore, eToggleIgnore);
                if(rc!=NEXUS_SUCCESS)
                {
                    goto EXIT;
                }
            }
            break;
        case eTestOpSetUrr:
            rc=setHeapsSecure(toggle, eToggleIgnore, eToggleIgnore);
            if(rc!=NEXUS_SUCCESS)
            {
                goto EXIT;
            }
            break;
        case eTestOpSetGfx:
            if(!heapInfo.secureGfx)
            {
                printf("No secure gfx buffers exist\n");
                rc=BERR_TRACE(NEXUS_INVALID_PARAMETER);
                goto EXIT;
            }
            rc=setHeapsSecure(eToggleIgnore, toggle, eToggleIgnore);
            if(rc!=NEXUS_SUCCESS)
            {
               goto EXIT;
            }
            break;
        case eTestOpSetExt:
            if(!heapInfo.secureExt)
            {
                printf("No secure extension buffers exist\n");
                rc=BERR_TRACE(NEXUS_INVALID_PARAMETER);
                goto EXIT;
            }
            rc=setHeapsSecure(eToggleIgnore, eToggleIgnore, toggle);
            if(rc!=NEXUS_SUCCESS)
            {
               goto EXIT;
            }
            break;
        case eTestOpLoop:
            printf("Launching toggle loop... \n");
            pthread_create(&thread, NULL, toggle_thread, &done);
            if(iterations<=0)
            {
                printf("\n\nInfinite loop started.... press enter to quit\n\n");
                getchar();
                done=true;
            }
            printf("Waiting for thread to exit\n");
            pthread_join(thread, (void **)&rc);
            break;
        case eTestOpVerify:
            rc = scrub_verify();
            break;
        case eTestOpQueryOnly:
        default:
            goto EXIT;
            break;
    }

    print_secure_status();

EXIT:
    NxClient_Uninit();
    return rc;
}
#else
#include <stdio.h>
int main(void)
{
    printf("test not supported\n");
    return 0;
}
#endif
