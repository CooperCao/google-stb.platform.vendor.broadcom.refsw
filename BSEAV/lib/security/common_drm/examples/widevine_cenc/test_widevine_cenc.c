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
 *****************************************************************************/
#ifndef NEXUS_HAS_SAGE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include "bstd.h"
#include "bdbg.h"
#include "bkni.h"

#include "nexus_platform.h"
#include "nexus_memory.h"
#include "drm_widevine_cenc.h"
#include "drm_common.h"
#include "drm_metadata.h"

#if 0
--------------------------------------------------
Subkey Generation
K              2b7e1516 28aed2a6 abf71588 09cf4f3c
AES-128(key,0) 7df76b0c 1ab899b3 3e42f047 b91b546f
K1             fbeed618 35713366 7c85e08f 7236a8de
K2             f7ddac30 6ae266cc f90bc11e e46d513b
--------------------------------------------------

--------------------------------------------------
Example 1: len = 0
M              <empty string>
AES-CMAC       bb1d6929 e9593728 7fa37d12 9b756746
--------------------------------------------------

Example 2: len = 16
M              6bc1bee2 2e409f96 e93d7e11 7393172a
AES-CMAC       070a16b4 6b4d4144 f79bdd9d d04a287c
--------------------------------------------------
#endif


/* Test Vectors */
uint8_t input_tc_0[16] =  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t expected_message_tc_0[16] =  {0xbb, 0x1d, 0x69, 0x29, 0xe9, 0x59, 0x37, 0x28, 0x7f, 0xa3, 0x7d, 0x12, 0x9b, 0x75, 0x67, 0x46};

uint8_t input_tc_1[16] =  {0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96, 0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a};
uint8_t expected_message_tc_1[16] =  {0x07, 0x0a, 0x16, 0xb4, 0x6b, 0x4d, 0x41, 0x44, 0xf7, 0x9b, 0xdd, 0x9d, 0xd0, 0x4a, 0x28, 0x7c};

unsigned char M[64] = {
    0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,    0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a,
    0xae, 0x2d, 0x8a, 0x57, 0x1e, 0x03, 0xac, 0x9c,    0x9e, 0xb7, 0x6f, 0xac, 0x45, 0xaf, 0x8e, 0x51,
    0x30, 0xc8, 0x1c, 0x46, 0xa3, 0x5c, 0xe4, 0x11,    0xe5, 0xfb, 0xc1, 0x19, 0x1a, 0x0a, 0x52, 0xef,
    0xf6, 0x9f, 0x24, 0x45, 0xdf, 0x4f, 0x9b, 0x17,    0xad, 0x2b, 0x41, 0x7b, 0xe6, 0x6c, 0x37, 0x10
};
uint8_t expected_message_tc_2[16] =  {0xdf, 0xa6, 0x67, 0x47, 0xde, 0x9a, 0xe6, 0x30, 0x30, 0xca, 0x32, 0x61, 0x14, 0x97, 0xc8, 0x27};
uint8_t expected_message_tc_3[16] =  {0x51, 0xf0, 0xbe, 0xbf, 0x7e, 0x3b, 0x9d, 0x92, 0xfc, 0x49, 0x74, 0x17, 0x79, 0x36, 0x3c, 0xfe};

int main(int argc, char* argv[])
{
    int zz = 0;
    DRM_WidevineCenc_Settings inputWvInitStruct;
    uint8_t wrappedKey[16] = {0x00};
    uint8_t *output_cmac = NULL;

    NEXUS_PlatformSettings platformSettings;
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;

    NEXUS_Platform_Init(&platformSettings);

    BSTD_UNUSED(argc);

    inputWvInitStruct.key_file = argv[1];

    if(DRM_WidevineCenc_Initialize(&inputWvInitStruct) != Drm_Success)
    {
        printf("\n\tMAIN - failed to Initialize Widevine CENC DRM module **************\n");
        return -1;
    }

    /* get wrapped key */
    if(DRM_WidevineCenc_GetKey(wrappedKey) != Drm_Success)
    {
        printf("\n\tMAIN - failed to get key data **************\n");
        return -1;
    }

    if(DRM_Common_MemoryAllocate(&output_cmac, 16) != Drm_Success){
        printf("%s - Allocate memory failed (%u) ", BSTD_FUNCTION, __LINE__);
        return -1;
    }

    for(zz = 0; zz <3;zz++)
    {
        uint8_t *ptrToCompare = NULL;
        uint32_t size = 0;
        switch(zz){
        case 0:
            ptrToCompare = expected_message_tc_1;
            size = 16;
            break;
        case 1:
            ptrToCompare = expected_message_tc_2;
            size = 40;
            break;
        case 2:
            ptrToCompare = expected_message_tc_3;
            size = 64;
            break;
        }
        /* call aes-cmac */
        if(DRM_WidevineCenc_AesCMAC(wrappedKey,
                                 16,
                                 M,
                                 size,
                                 output_cmac) != Drm_Success)
        {
            printf("\n\tMAIN - Error AES-CMAC *******\n");
            return -1;
        }

        if(BKNI_Memcmp(ptrToCompare, output_cmac, 16) != 0)
        {
           printf("%s - AES-CMAC comparison failed (iteration = %u)\n", BSTD_FUNCTION, zz);
           printf("%s - AES-CMAC comparison failed (iteration = %u)\n", BSTD_FUNCTION, zz);
        }
        else
        {
            printf("%s - AES-CMAC MATCH!!!! (iteration = %u)\n", BSTD_FUNCTION, zz);
            printf("%s - AES-CMAC MATCH!!!! (iteration = %u)\n", BSTD_FUNCTION, zz);
        }
    }

    if(output_cmac != NULL)
    {
        DRM_Common_MemoryFree(output_cmac);
    }

    DRM_WidevineCenc_Finalize();

    NEXUS_Platform_Uninit();

    printf("\n\tMAIN - exiting application\n");
    return 0;
}

#else /* ifndef NEXUS_HAS_SAGE */
#include <stdio.h>
#include "bstd.h"
int main(int argc, char* argv[])
{
    BSTD_UNUSED(argc);
    BSTD_UNUSED(argv);
    printf("\n\nThis application is not supported on SAGE-based platforms.\n");
    printf("Please use the DRM_WVOemCrypto API for Widevine support on SAGE-based platforms.\n\n");
    return -1;
}
#endif
