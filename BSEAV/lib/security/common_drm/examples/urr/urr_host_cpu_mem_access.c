/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

/* Nexus example app: two PES files (one video, one audio) using two playpumps with lipsync */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "nexus_platform.h"
#include "nexus_base_mmap.h"

#include <assert.h>
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "nexus_parser_band.h"
#include "nexus_pid_channel.h"
#include "sage_srai.h"
#include "common_crypto.h"
#include "drm_prdy.h"

BDBG_MODULE(urr_host_cpu_mem_access);


#define WRITE_PATTERN 42
#define LENGTH 0x1000


int mem_cmp(unsigned offset, const void * pRefBuffer, unsigned length, int *pCompareResult)
{
    void *baseAddr = NULL;
    int memfd = -1;
    int rc = 0;

    memfd = open("/dev/mem", O_RDWR|O_SYNC);
    if (memfd < 0) {
        fprintf(stderr, "Cannot open /dev/mem , error #%d '%s'\n", memfd, strerror(memfd));
        rc = -1;
        goto errorExit;
    }
    baseAddr = (void *)mmap(0, length, PROT_READ|PROT_WRITE, MAP_SHARED, memfd, offset);
    if (baseAddr == MAP_FAILED) {
        fprintf(stderr, "Cannot mmap [0x%08x, 0x%08x[ MAP_FAILED\n", offset, offset+length);
        rc = -2;
        goto errorExit;
    }

    *pCompareResult =  memcmp(baseAddr, pRefBuffer, length);

    fflush(NULL);

errorExit:
    if (baseAddr) {
        munmap(baseAddr, length);
        baseAddr = NULL;
    }
    if (memfd >= 0) {
        close(memfd);
        memfd = -1;
    }

    return rc;
}


static void *base_addr = NULL;
static int memfd = -1;

static int _write_init(unsigned offset, unsigned length)
{
    memfd = open("/dev/mem", O_RDWR|O_SYNC);
    if (memfd < 0) {
        fprintf(stderr, "Cannot open /dev/mem , error #%d '%s'\n", memfd, strerror(memfd));
        return -1;
    }
    base_addr = (void *)mmap(0, length, PROT_READ|PROT_WRITE, MAP_SHARED, memfd, offset);
    if (base_addr == MAP_FAILED) {
        fprintf(stderr, "Cannot mmap [0x%08x, 0x%08x[ MAP_FAILED\n", offset, offset+length);
        return -2;
    }
    return 0;
}
static void _write_uninit(unsigned length)
{
    if (base_addr) {
        munmap(base_addr, length);
        base_addr = NULL;
    }
    if (memfd >= 0) {
        close(memfd);
        memfd = -1;
    }
}

int write_mem_pattern(unsigned offset, unsigned length, unsigned pattern)
{
    int rc = 0;

    if (_write_init(offset, length)) {
        rc = -3;
        goto end;
    }

    {
        unsigned i;
        unsigned char *p = (unsigned char *)base_addr;
        printf("WRITE %u bytes [0x%08x, 0x%08x[\n", length, offset, offset + length);
        for (i = 0; i < length; i++)
        {
            p[i] = pattern;
        }
        printf("\n   ---   END\n\n");
        fflush(NULL);
    }

end:
    _write_uninit(length);
    return rc;
}


int main(void)
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_MemoryConfigurationSettings memConfigSettings;

    BSAGElib_State sage_platform_status;
    SRAI_PlatformHandle sage_platformHandle;
    BSAGElib_InOutContainer *container;
    unsigned heapIndex;
    NEXUS_MemoryStatus status;
    BERR_Code sage_rc = BERR_SUCCESS;
    char refBuffer[LENGTH];
    int i,j;

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    NEXUS_GetDefaultMemoryConfigurationSettings(&memConfigSettings);
    platformSettings.openFrontend = false;

    /* Request secure picture buffers, i.e. URR
    * Should only do this if SAGE is in use, and when SAGE_SECURE_MODE is NOT 1 */
    /* For now default to SVP2.0 type configuration (i.e. ALL buffers are
    * secure ONLY */
    for (i=0;i<NEXUS_NUM_VIDEO_DECODERS;i++)
    {
        memConfigSettings.videoDecoder[i].secure = NEXUS_SecureVideo_eSecure;
    }
    for (i=0;i<NEXUS_NUM_DISPLAYS;i++)
    {
        for (j=0;j<NEXUS_NUM_VIDEO_WINDOWS;j++)
        {
            memConfigSettings.display[i].window[j].secure = NEXUS_SecureVideo_eSecure;
        }
    }

    NEXUS_Platform_MemConfigInit(&platformSettings, &memConfigSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    memset(refBuffer, 0, LENGTH);

    for (heapIndex=0; heapIndex < NEXUS_MAX_HEAPS; heapIndex++) {
        NEXUS_Heap_GetStatus(platformConfig.heap[heapIndex], &status);
        if (status.heapType & NEXUS_HEAP_TYPE_PICTURE_BUFFERS)
        {
             write_mem_pattern(status.offset, LENGTH, WRITE_PATTERN);
        }
    }

    sage_rc = SRAI_Platform_Open(BSAGE_PLATFORM_ID_COMMONDRM, &sage_platform_status, &sage_platformHandle);
    if (sage_rc != BERR_SUCCESS)
    {
        printf("%s - Error calling platform_open", BSTD_FUNCTION);
        assert(sage_rc == BERR_SUCCESS);
    }
    if(sage_platform_status == BSAGElib_State_eUninit)
    {
        container = SRAI_Container_Allocate();
        printf("%s - container %p\n", BSTD_FUNCTION, (void *)container);
        if(container == NULL)
        {
            printf("%s - Error fetching container", BSTD_FUNCTION);
            assert(container);
        }
        sage_rc = SRAI_Platform_Init(sage_platformHandle, container);
        if (sage_rc != BERR_SUCCESS)
        {
            printf("%s - Error calling platform init", BSTD_FUNCTION);
            assert(sage_rc == BERR_SUCCESS);
        }
    }

    for (heapIndex=0; heapIndex < NEXUS_MAX_HEAPS; heapIndex++) {
        NEXUS_Heap_GetStatus(platformConfig.heap[heapIndex], &status);
        if ((status.heapType & NEXUS_HEAP_TYPE_PICTURE_BUFFERS) && (status.memoryType & NEXUS_MEMORY_TYPE_SECURE))
        {
            int result;
            if(mem_cmp(status.offset, (const void *)refBuffer, LENGTH, &result) == 0){
                if(result == 0) {
                    printf("MEMC Access Test: SUCCEEDED\n");
                }
                else
                    printf("MEMC Access Test: FAILED\n");
             }
            else {
                printf("mapping failure\n");
            }
        }
    }

    NEXUS_Platform_Uninit();

    return 0;
}
