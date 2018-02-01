/***************************************************************************
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
*******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include "nexus_platform.h"
#include "nexus_memory.h"
#include "drm_secure_store.h"
#include "drm_metadata.h"
#include "drm_common.h"

#include "bstd.h"
#include "bdbg.h" 
#include "bkni.h"

int main(int argc, char* argv[])
{
	DrmRC rc = Drm_Success;
	int i = 0;
	uint8_t data[16] = {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
			            0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0xAA, 0xAA, 0xAA};
	uint8_t *pDmaBuf = NULL;
	DrmCommonInit_t commonDrmInit;
	NEXUS_PlatformSettings platformSettings;
	uint32_t decSize = 0;

	/* init Nexus */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);

    BSTD_UNUSED(argc);
    BSTD_UNUSED(argv);

    BKNI_Memset((uint8_t*)&commonDrmInit, 0x00, sizeof(DrmCommonInit_t));

	/* initialize Basic Common DRM */
	rc = DRM_Common_BasicInitialize(&commonDrmInit);
	if(rc != Drm_Success)
	{
		printf("%s - DRM_Common_BasicInitialize() failed\n", BSTD_FUNCTION);
		return 1;
	}

    /* Allocate a continuous physical address buffer for DMA */
    NEXUS_Memory_Allocate(16, NULL,(void **) &pDmaBuf);

    memcpy(pDmaBuf, data, 16);

    rc = DRM_SecureStore_AlignedFileEncWrite("./secure_store_test.bin", pDmaBuf, 16);
    if(rc != Drm_Success)
    {
    	printf("\n\t###   MAIN - Error encrypting...\n");
    	return 0;
    }

	decSize = 16;
    rc = DRM_SecureStore_AlignedFileDecRead("./secure_store_test.bin", pDmaBuf, &decSize);
    if(rc != Drm_Success)
    {
    	printf("\n\t###   MAIN - Error decrypting and reading...\n");
    	return 0;
    }

	printf("\n\tMAIN - Decrypted data :\n");
	for(i = 0; i < 16; i++)
	{
		printf("0x%02x ", pDmaBuf[i]);
	}
	printf("\n\n");

	printf("Done\n");

    /* Clean up test environment */
    NEXUS_Memory_Free(pDmaBuf);
    DRM_Common_Finalize();
    NEXUS_Platform_Uninit();

    return 0;
}
