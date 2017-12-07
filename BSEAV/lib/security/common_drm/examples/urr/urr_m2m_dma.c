/******************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 *****************************************************************************/
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

BDBG_MODULE(playpump_two_pes_files2);

/* the following define the input file and its characteristics -- these will vary by input file */
#define VIDEO_FILE_NAME "videos/cnnticker.video.pes"
#define AUDIO_FILE_NAME "videos/cnnticker.audio.pes"
#define TRANSPORT_TYPE NEXUS_TransportType_eMpeg2Pes
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define AUDIO_CODEC NEXUS_AudioCodec_eMpeg
#define VIDEO_PID 0xe0
#define AUDIO_PID 0xc0


/* MEMC0 */
#define WRITE_OFFSET 0x02f800000
#define WRITE_LENGTH 0x82DB000
#define WRITE_PATTERN 42

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

    BSAGElib_State sage_platform_status;
    SRAI_PlatformHandle sage_platformHandle;
    BSAGElib_InOutContainer *container;

    BERR_Code sage_rc = BERR_SUCCESS;

    write_mem_pattern(WRITE_OFFSET, WRITE_LENGTH, WRITE_PATTERN);
    write_mem_pattern(WRITE_OFFSET, 32, 0xCD);

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    /* Open Sage */

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

    {
        NEXUS_MemoryStatus memc0Status;
        NEXUS_MemoryStatus defaultstatus;
        NEXUS_MemoryStatus memc1Status;
        NEXUS_MemoryStatus memc2Status;
        NEXUS_Error rc;

        NEXUS_DmaJobBlockOffsetSettings blkSettingsOffset;
        NEXUS_DmaJobBlockSettings blkSettings;

        NEXUS_DmaJobHandle dmaJobHandle;
        NEXUS_DmaHandle  dmaHandle;
        NEXUS_DmaJobStatus jobStatus;
        uint8_t *pBuffer;
        NEXUS_Addr bufferAddr;

        rc = NEXUS_Memory_Allocate(64, NULL, (void **)(&pBuffer));
        if (rc)
        {
            printf("%s: NEXUS_Memory_Allocate(failed)\n", BSTD_FUNCTION);
            goto error;
        }

        bufferAddr = NEXUS_AddrToOffset(pBuffer);
        printf("bufferAddr(high) 0x%x\n", (uint32_t)((bufferAddr >> 32) & 0xFFFFFFFF));
        printf("bufferAddr(low)  0x%x\n", (uint32_t)(bufferAddr & 0xFFFFFFFF));

/*!+!hla should avoid using index now */
        NEXUS_Heap_GetStatus(platformConfig.heap[NEXUS_PLATFORM_DEFAULT_HEAP], &defaultstatus);
        NEXUS_Heap_GetStatus(platformConfig.heap[NEXUS_MEMC0_PICTURE_BUFFER_HEAP], &memc0Status);
        NEXUS_Heap_GetStatus(platformConfig.heap[NEXUS_MEMC1_PICTURE_BUFFER_HEAP], &memc1Status);
        NEXUS_Heap_GetStatus(platformConfig.heap[NEXUS_MEMC2_PICTURE_BUFFER_HEAP], &memc2Status);


        dmaHandle = NEXUS_Dma_Open(0 , NULL);
        if(dmaHandle == NULL)
        {
            printf("%s: NEXUS_Dma_Open(failed)\n", BSTD_FUNCTION);
            goto error;
        }

        dmaJobHandle = NEXUS_DmaJob_Create(dmaHandle, NULL);
        if(dmaJobHandle == NULL)
        {
            printf("%s: NEXUS_DmaJob_Create(failed)\n", BSTD_FUNCTION);
            goto error;
        }

        BDBG_ERR(("memc0Status.offset: "BDBG_UINT64_FMT"\n", BDBG_UINT64_ARG(memc0Status.offset)));
        BDBG_ERR(("memc1Status.offset: "BDBG_UINT64_FMT"\n", BDBG_UINT64_ARG(memc1Status.offset)));
        BDBG_ERR(("memc2Status.offset: "BDBG_UINT64_FMT"\n", BDBG_UINT64_ARG(memc2Status.offset)));


        NEXUS_DmaJob_GetDefaultBlockSettings(&blkSettings);
        blkSettings.pSrcAddr = (void *)(pBuffer + 32);
        blkSettings.pDestAddr = (void *)pBuffer;
        blkSettings.blockSize = 32;
        blkSettings.resetCrypto = true;
        blkSettings.scatterGatherCryptoStart = true;
        blkSettings.scatterGatherCryptoEnd = true;

        BKNI_Memset((void *)blkSettings.pSrcAddr, 0x75, 32);

        printf("(virtual addr) blkSettings.pSrcAddr  %x\n", (unsigned int)blkSettings.pSrcAddr );
        printf("(virtual addr) blkSettings.pDestAddr %x\n", (unsigned int)blkSettings.pDestAddr );

        rc = NEXUS_DmaJob_ProcessBlocks(dmaJobHandle, &blkSettings, 1);
        if(rc != NEXUS_SUCCESS){
            if(rc == NEXUS_DMA_QUEUED){
                for(;;)
                {
                    rc = NEXUS_DmaJob_GetStatus(dmaJobHandle, &jobStatus);
                    if(rc != NEXUS_SUCCESS) {
                        printf("BAD rc %x\n", rc);
                        goto error;
                    }

                    if(jobStatus.currentState == NEXUS_DmaJobState_eComplete )
                    {
                        printf("NEXUS_DmaJob_ProcessBlocksOffset status NEXUS_DmaJobState_eComplete\n");
                        break;
                    }
                    BKNI_Delay(1);
                }
             }
             else {
                 BDBG_ERR(("%s - NEXUS_DmaJob_ProcessBlocks failed, rc = %d\n", BSTD_FUNCTION, rc));
                 goto error;
             }
        }

        printf("NEXUS_DmaJob_ProcessBlocksOffset, read/copy outside of ARCH address range, rc %x\n", rc);
        printf("data : %x %x %x %x %x %x %x %x\n", pBuffer[0], pBuffer[1], pBuffer[2], pBuffer[3], pBuffer[4], pBuffer[5], pBuffer[6], pBuffer[7]);
        printf("data : %x %x %x %x %x %x %x %x\n", pBuffer[8], pBuffer[9], pBuffer[10], pBuffer[11], pBuffer[12], pBuffer[13], pBuffer[14], pBuffer[15]);

        NEXUS_DmaJob_GetDefaultBlockOffsetSettings(&blkSettingsOffset);
        blkSettingsOffset.srcOffset = memc0Status.offset;
        /*blkSettingsOffset.destOffset = memc0Status.offset + 32;*/
        blkSettingsOffset.destOffset = (uint32_t)(bufferAddr & 0xFFFFFFFF);

        blkSettingsOffset.blockSize = 32;
        blkSettingsOffset.resetCrypto = true;
        blkSettingsOffset.scatterGatherCryptoStart = true;
        blkSettingsOffset.scatterGatherCryptoEnd = true;

        printf("blkSettings.destOffset  %x\n", blkSettingsOffset.destOffset );
        printf("blkSettings.srcOffset  %x\n", blkSettingsOffset.srcOffset );


        rc = NEXUS_DmaJob_ProcessBlocksOffset(dmaJobHandle, &blkSettingsOffset, 1 );
        if(rc != NEXUS_SUCCESS){
            if(rc == NEXUS_DMA_QUEUED){
                for(;;)
                {
                    rc = NEXUS_DmaJob_GetStatus(dmaJobHandle, &jobStatus);
                    if(rc != NEXUS_SUCCESS) {
                        printf("BAD %x\n", rc);
                        BDBG_ERR(("%s - NEXUS_DmaJob_ProcessBlocks failed, rc = %d\n", BSTD_FUNCTION, rc));
                        goto error;
                    }

                    if(jobStatus.currentState == NEXUS_DmaJobState_eComplete )
                    {
                        printf("NEXUS_DmaJob_ProcessBlocksOffset status NEXUS_DmaJobState_eComplete\n");
                        break;
                    }
                    BKNI_Delay(1);
                }
             }
             else {
                 BDBG_ERR(("%s - NEXUS_DmaJob_ProcessBlocks failed, rc = %d\n", BSTD_FUNCTION, rc));
                 goto error;
             }
        }

        printf("NEXUS_DmaJob_ProcessBlocksOffset, read inside of ARCH address range and copy into unsercure region, rc %x\n", rc);
        printf("data : %x %x %x %x %x %x %x %x\n", pBuffer[0], pBuffer[1], pBuffer[2], pBuffer[3], pBuffer[4], pBuffer[5], pBuffer[6], pBuffer[7]);
        printf("data : %x %x %x %x %x %x %x %x\n", pBuffer[8], pBuffer[9], pBuffer[10], pBuffer[11], pBuffer[12], pBuffer[13], pBuffer[14], pBuffer[15]);

        NEXUS_DmaJob_Destroy(dmaJobHandle);
        NEXUS_Dma_Close(dmaHandle);
    }

    return 0;

error:
    return 1;
}
