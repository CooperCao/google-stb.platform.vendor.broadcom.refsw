/***************************************************************************
 *    (c)2015 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
*******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "nexus_platform.h"
#include "nexus_memory.h"
#include "drm_edrm_tl.h"
#include "drm_metadata.h"
#include "drm_common.h"

#include "bstd.h"
#include "bdbg.h"
#include "bkni.h"

int main(int argc, char* argv[])
{
    DrmRC rc = Drm_Success;
    DRM_EdrmTlHandle hEdrmTl;
    uint32_t result;
    uint8_t *pCert = NULL;
    int i;
    NEXUS_PlatformSettings platformSettings;

    if(argc != 2)
    {
        printf("\n\t USAGE: test_edrm [path of type3 drm bin file]\n");
        return 1;
    }

    /* init Nexus */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);

    /* initialize Edrm */
    rc = DRM_EdrmTl_Initialize(argv[1], &hEdrmTl);
    if(rc != Drm_Success)
    {
        printf("\n\t###   MAIN - Error calling DRM_EdrmTl_Initialize...\n");
        return 0;
    }

    rc = DRM_Common_MemoryAllocate(&pCert, EDRM_CERT_SIZE);
    if(rc == Drm_Success)
    {
        rc = DRM_EdrmTl_GetDeviceCertificate(hEdrmTl, pCert, EDRM_CERT_SIZE);
        if(rc == Drm_Success)
        {
            printf("----------<EDRM_Certificate>-------------------");
            for(i=0; i<EDRM_CERT_SIZE; i++)
            {
            if(i%32==0) printf("\n");
            printf(" %02x", pCert[i]);
            }
            printf("\n---------------------------------------------\n");
        }
        DRM_Common_MemoryFree(pCert);
    }

    DRM_EdrmTl_VerifyPublicKey(hEdrmTl, &result);
    printf("[result=%d] Public key is valid.\n", result);

    DRM_EdrmTl_Finalize(hEdrmTl);

    NEXUS_Platform_Uninit();

    return 0;
}
