/***************************************************************************
*     Copyright (c) 2004-2013, Broadcom Corporation
*     All Rights Reserved
*     Confidential Property of Broadcom Corporation
*
*  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
*  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
*  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
*
***************************************************************************/
#if NEXUS_HAS_SAGE
#include "nxclient.h"
#include "nexus_platform.h"
#include "nexus_base_mmap.h"
#include "bstd.h"
#include "bkni.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

/* This nxclient app will connect to server and show/verify current heap
* configuration. Any buffer that is secure should be not be visible to this app.
* A failure indicates that host can access secure memory */
/* NOTE that URR toggling can complicate the results of this app */

BDBG_MODULE(heap_check);

#define MAP_LEN sizeof(uint32_t)
#define TEST_VALUE 0x1234dead

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
        {NEXUS_HEAP_TYPE_MAIN,"MAIN"},
        {NEXUS_HEAP_TYPE_PICTURE_BUFFERS,"PICTURE"},
        {NEXUS_HEAP_TYPE_SAGE_RESTRICTED_REGION,"SAGE"},
        {NEXUS_HEAP_TYPE_COMPRESSED_RESTRICTED_REGION,"CRR"},
        {NEXUS_HEAP_TYPE_GRAPHICS,"GFX"},
        {NEXUS_HEAP_TYPE_SECONDARY_GRAPHICS,"GFX2"}};
    unsigned n = 0, i;
    for (i=0;i<sizeof(g_mapping)/sizeof(g_mapping[0]) && n<buf_size;i++) {
        if (heapType & g_mapping[i].value) {
            n += BKNI_Snprintf(&buf[n], buf_size-n, "%s%s", n?",":"", g_mapping[i].name);
        }
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
        memory_type_to_string(status.memoryType, buf[0], sizeof(buf[0])),
        heap_type_to_string(status.heapType, buf[1], sizeof(buf[1])));

    if(status.heapType&NEXUS_HEAP_TYPE_PICTURE_BUFFERS)
    {
        *total+=status.size;
        if(status.memoryType&NEXUS_MEMORY_TYPE_SECURE)
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
    NEXUS_ClientConfiguration clientConfig;
    NEXUS_PlatformConfiguration platformConfig;
    unsigned picture_heap_total=0;
    unsigned picture_heap_unsecure=0;
    unsigned picture_heap_secure=0;

    NEXUS_Platform_GetClientConfiguration(&clientConfig);
    NEXUS_Platform_GetConfiguration(&platformConfig);
    printf("heap\toffset\t\tmemc\tsize@offset\t\t\tused/peak\tserver\tinfo\n");
    for (i=0;i<NEXUS_MAX_HEAPS;i++) {
        if (platformConfig.heap[i]) {
            print_heap(i, platformConfig.heap[i], &platformConfig, &picture_heap_total, &picture_heap_unsecure, &picture_heap_secure);
        }
    }
    printf("PICTURE HEAP TOTAL: 0x%08x (%dMB)\n", picture_heap_total, picture_heap_total>>20);
    printf("PICTURE HEAP (SECURE): 0x%08x (%dMB)\n", picture_heap_secure, picture_heap_secure>>20);
    printf("PICTURE HEAP (UNSECURE): 0x%08x (%dMB)\n", picture_heap_unsecure, picture_heap_unsecure>>20);
}

static int test_heaps(void)
{
    int i;
    int rc = 0;
    void *test_addr;
    uint32_t value;
    NEXUS_MemoryStatus status;
    NEXUS_Error nexus_rc;
    bool secure, picture;
    int memfd;
    NEXUS_PlatformConfiguration platformConfig;

    NEXUS_Platform_GetConfiguration(&platformConfig);

    memfd = open("/dev/mem", O_RDWR|O_SYNC);
    if (memfd < 0)
    {
        fprintf(stderr, "Cannot open /dev/mem\n");
        return -1;
    }

    for (i=0;i<NEXUS_MAX_HEAPS;i++)
    {
        if(!platformConfig.heap[i])
            continue;

        nexus_rc = NEXUS_Heap_GetStatus(platformConfig.heap[i], &status);
        if(nexus_rc != NEXUS_SUCCESS)
        {
            printf("Failed to get information on heap %d, SKIPPING\n", i);
            continue;
        }

        secure=(status.memoryType & NEXUS_MEMORY_TYPE_SECURE) ? true : false;
        picture=(status.heapType & NEXUS_HEAP_TYPE_PICTURE_BUFFERS) ? true : false;

        if (picture || secure)
        {
            printf("Checking HEAP[%d]: 0x%08x@0x%08x - %s - %s\n", i, status.size, (uint32_t)status.offset,
                picture ? "Picture Buffer" : "OTHER", secure ? "SECURE" : "UNsecure");

            test_addr = (void *)mmap(NULL, MAP_LEN, PROT_READ|PROT_WRITE, MAP_SHARED, memfd, status.offset);
            if(test_addr == MAP_FAILED)
            {
                printf("Failed to map 0x%x@0x%x\n", MAP_LEN, (uint32_t)status.offset);
                continue;
            }

            printf("\tWrite - 0x%08x : ", TEST_VALUE);
            value=TEST_VALUE;
            memcpy(test_addr, &value, sizeof(value));

            memcpy(&value, test_addr, sizeof(value));
            printf("Read - 0x%08x : ", value);

            if((secure) && (value!=0))
            {
                /* TODO: Check if toggling enabled */
                rc = NEXUS_UNKNOWN;
                printf("FAIL!\n");
            }
            else
            {
                printf("PASS\n");
            }

            munmap(test_addr, MAP_LEN);
        }
    }

    close(memfd);
    return rc;
}

int main(int argc, char **argv)
{
    NxClient_JoinSettings joinSettings;
    NEXUS_Error rc = NEXUS_SUCCESS;

    BSTD_UNUSED(argc);

    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s", argv[0]);
    joinSettings.mode = NEXUS_ClientMode_eVerified;
    rc = NxClient_Join(&joinSettings);
    if (rc) return -1;

    print_heaps();

    rc = test_heaps();

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
