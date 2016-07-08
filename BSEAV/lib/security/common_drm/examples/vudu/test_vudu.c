/******************************************************************************
* Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
*
* This program is the proprietary software of Broadcom and/or its
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
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. , WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "nexus_platform.h"
#include "drm_vudu.h"
#include "drm_metadata.h"
#include "drm_common.h"
#include "drm_types.h"

int test_checkVUDU(void)
{
    int i = 0;
    int keyLen = 16;
    int ivLen = 16;
    int ret = 0;

    uint8_t* srcBuf = NULL;
    uint8_t* dstBuf = NULL;
    int length = 80;


    uint8_t key3Data[] = {0x5e, 0xc4, 0x79, 0xb5, 0xa6, 0x14, 0x56, 0x8b,
                          0x28, 0x14, 0xa7, 0x0a, 0x66, 0x1e, 0xef, 0x53};

    uint8_t key4Data[] = {0x22, 0x67, 0x9e, 0x5a, 0x24, 0xf3, 0xb4, 0xbb,
                          0x4a, 0x29, 0x95, 0x25, 0x36, 0x26, 0x2e, 0xac};

    uint8_t ivData[] = {0x02, 0x12, 0xf7, 0x5d, 0x7b, 0x36, 0xb0, 0x55,
                        0x10, 0x3a, 0x00, 0x7a, 0x6a, 0xfc, 0xde, 0x2b};

    uint8_t clear[] = {
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55};

  uint8_t src[] = {
    0x5A, 0xFD, 0x4C, 0xD9, 0x10, 0x2F, 0x76, 0x7B, 0xB4, 0x65, 0xAC, 0xC5, 0x58, 0x4E, 0x01, 0xC2,
    0x28, 0x4E, 0x81, 0xC4, 0x8C, 0x45, 0x34, 0x08, 0x83, 0x52, 0x0C, 0xA1, 0xB3, 0xC6, 0x8D, 0x25,
    0xF5, 0xA8, 0x32, 0xDE, 0xFB, 0x91, 0x98, 0xEB, 0xCE, 0xAB, 0x78, 0xF0, 0xFF, 0xC4, 0x02, 0xC2,
    0x23, 0x93, 0xAF, 0xCF, 0xA7, 0x7F, 0xCE, 0x75, 0x2E, 0x50, 0xAE, 0x21, 0x37, 0x32, 0xA0, 0x4F,
    0x34, 0xB3, 0x63, 0x2B, 0xD0, 0xF7, 0x6C, 0x64, 0x4B, 0xBB, 0xB8, 0x59, 0x20, 0x28, 0x66, 0xA3};


    uint8_t dst[80] = {0, };

    if (DRM_Vudu_Initialize() != Drm_Success)
    {
        fprintf(stderr, "VUDU error: Failed to initialize DRM module.\n");
		return 1;
    }

    DRM_Vudu_SetConfigAlgParams(DrmSecurityAlgorithm_eAes, DrmSecurityAlgorithmVariant_eCbc, DrmSecurityOperation_eDecrypt);

    DRM_VuduContentKeyParams_t contentKeyParams;
    BKNI_Memset((uint8_t *)&contentKeyParams, 0x00, sizeof(DRM_VuduContentKeyParams_t));


    BKNI_Memcpy(contentKeyParams.procInForKey3, key3Data, keyLen);
    BKNI_Memcpy(contentKeyParams.procInForKey4, key4Data, keyLen);
    BKNI_Memcpy(contentKeyParams.Iv, ivData, ivLen);

    contentKeyParams.procInLength  = keyLen;
    contentKeyParams.IvLength      = ivLen;
    contentKeyParams.CustKeySelect = 0x47;
    contentKeyParams.KeyVarHigh    = 0x03;
    contentKeyParams.KeyVarLow     = 0x2e;

    if (DRM_Vudu_LoadContentKey(&contentKeyParams) != Drm_Success)
    {
        fprintf(stderr, "DRM_Vudu_LoadContentKey failed\n");
        ret = 1;
		goto ErrorExit;
    }

    if ((DRM_Common_MemoryAllocate(&srcBuf, length) != Drm_Success) || (srcBuf == NULL))
    {
      fprintf(stderr, "DRM_Common_MemoryAllocate failed\n");
      ret = 2;
    }

    if ((DRM_Common_MemoryAllocate(&dstBuf, length) != Drm_Success) || (dstBuf == NULL))
    {
        fprintf(stderr, "DRM_Common_MemoryAllocate failed\n");
        ret = 3;
		goto ErrorExit;
    }

    BKNI_Memcpy(srcBuf, src, length);

    if (DRM_Vudu_Decrypt((uint8_t*)srcBuf, length, (uint8_t*)dstBuf) != Drm_Success)
    {
        fprintf(stderr, "DRM_Vudu_Decrypt failed\n");
        ret = 4;
		goto ErrorExit;
    }

    BKNI_Memcpy(dst, dstBuf, length);

    fprintf(stderr, " Enc input:\n");
    for ( i = 0; i < length; i++)
    {
        if ((i != 0) && ((i % 16) == 0))
        {
            fprintf(stderr, "\n");
        }
        fprintf(stderr, "%02X ", src[i]);
    }
    fprintf(stderr, "\n");

    fprintf(stderr, "Decrypt output :\n");
    for ( i = 0; i < length; i++)
    {
        if ((i != 0) && ((i % 16) == 0))
        {
            fprintf(stderr, "\n");
        }
        fprintf(stderr, "%02X ", dst[i]);
    }
    fprintf(stderr, "\n");

    /*Compare data*/
    for(i = 0; i < length; i++)
    {
        if(dst[i] != 0x55)
        {
            fprintf(stderr, "Data Mismatch!!!!!!!!!!!!\n");
            ret = 4;
			goto ErrorExit;
        }
    }
ErrorExit:
	if (srcBuf)
		DRM_Common_MemoryFree(srcBuf);

	if (dstBuf)
		DRM_Common_MemoryFree(dstBuf);

    DRM_Vudu_Finalize();

    return ret;
}


int main(int argc, char *argv[])
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    int err = 0;

    err = NEXUS_Platform_Init(&platformSettings);
    if (err)
    {
        printf("NEXUS_Platform_Init() failed\n");
        return -1;
    }

    err = test_checkVUDU();
    if (err)
    {
        printf("Vudu Unittest failed :(\n");
    }
    else
    {
        printf("Vudu Unittest Passed :) \n");
    }


    NEXUS_Platform_Uninit();

    return err;
}
