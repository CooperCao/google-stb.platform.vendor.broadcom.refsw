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
 *
 ******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "nexus_platform.h"
#include "nexus_memory.h"
#include "drm_mediaroom_tl.h"
#include "sage_srai.h"

#include "bstd.h"
#include "bdbg.h"
#include "bkni.h"

#ifdef NXCLIENT_SUPPORT
#include "nxclient.h"
#endif


int main(int argc, char* argv[])
{
    DrmRC rc = Drm_Success;
    uint32_t result;
    uint8_t *pMagic = NULL;
    uint8_t *pVersion = NULL;
    uint8_t *pCert = NULL;
    int i;
#ifdef NXCLIENT_SUPPORT
    NEXUS_ClientConfiguration clientConfig;
    NxClient_JoinSettings joinSettings;
    NxClient_AllocSettings nxAllocSettings;
    NxClient_AllocResults allocResults;
    int nxc_rc = BERR_SUCCESS;
#else
    NEXUS_PlatformSettings platformSettings;
#endif

    if(argc != 2)
    {
        printf("\n\t USAGE: test_mediaroom [path of type3 drm bin file]\n");
        return 1;
    }

    /* init Nexus */
#ifdef NXCLIENT_SUPPORT
    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "test_mediaroom");
    joinSettings.ignoreStandbyRequest = true;
    nxc_rc = NxClient_Join(&joinSettings);
    if(nxc_rc)
    {
         return -1;
    }
#else
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
#endif

#ifdef NXCLIENT_SUPPORT
    /* Show heaps info */
    NEXUS_Platform_GetClientConfiguration(&clientConfig);
    {
        int g;
        printf("NxClient Heaps Info -----------------\n");
        for (g = NXCLIENT_DEFAULT_HEAP; g <= NXCLIENT_SECONDARY_GRAPHICS_HEAP; g++)
        {
            NEXUS_MemoryStatus status;
            NEXUS_Heap_GetStatus(clientConfig.heap[g], &status);

            printf("Heap[%d]: memoryType=%u, heapType=%u, offset=%u, addr=%p, size=%u\n",
                      g, status.memoryType, status.heapType, (uint32_t)status.offset, status.addr, status.size);
        }
        printf("-------------------------------------\n");
    }
#endif

    /* initialize Mediaroom */
    rc = DRM_Mediaroom_Initialize(argv[1]);
    if(rc != Drm_Success)
    {
        printf("\n\t###   MAIN - Error calling DRM_MediaroomTl_Initialize...\n");
        return 0;
    }

    /* Get Magic field and Verify it. */
    rc = NEXUS_Memory_Allocate(MEDIAROOM_MAGIC_SIZE, NULL, (void **) &pMagic);
    if(rc == Drm_Success)
    {
        rc = DRM_Mediaroom_GetMagic(pMagic, MEDIAROOM_MAGIC_SIZE);
        if(rc == NEXUS_SUCCESS)
        {
            printf("----------<Mediaroom_Magic>-------------------");
            for(i=0; i<MEDIAROOM_MAGIC_SIZE; i++)
            {
            if(i%32==0) printf("\n");
            printf(" %02x", pMagic[i]);
            }
            printf("\n---------------------------------------------\n");

            result = DRM_Mediaroom_VerifyMagic(pMagic);
            printf("[result=%d] Magic field is %svalid.\n", result, (result==1)?"":"in");
        }
        else
        {
            printf("Call to DRM_Mediaroom_GetVersion failed: rc = %d\n", rc);
        }
        NEXUS_Memory_Free(pMagic);
    }
    else
    {
        printf("Call to NEXUS_Memory_Allocate failed: rc = %d\n", rc);
    }

    /* Get Version field and Verify it. */
    rc = NEXUS_Memory_Allocate(MEDIAROOM_VERSION_SIZE, NULL, (void **) &pVersion);
    if(rc == Drm_Success)
    {
        rc = DRM_Mediaroom_GetVersion(pVersion, MEDIAROOM_VERSION_SIZE);
        if(rc == NEXUS_SUCCESS)
        {
            printf("----------<Mediaroom_Version>-------------------");
            for(i=0; i<MEDIAROOM_VERSION_SIZE; i++)
            {
            if(i%32==0) printf("\n");
            printf(" %02x", pVersion[i]);
            }
            printf("\n---------------------------------------------\n");

            result = DRM_Mediaroom_VerifyVersion(pVersion);
            printf("[result=%d] Version field is %svalid.\n", result, (result==1)?"":"in");
        }
        else
        {
            printf("Call to DRM_Mediaroom_GetVersion failed: rc = %d\n", rc);
        }
        NEXUS_Memory_Free(pVersion);
    }
    else
    {
        printf("Call to NEXUS_Memory_Allocate failed: rc = %d\n", rc);
    }


    /* Get AV Pb certificate.  Verify AV Pb in AV Pb certificate. */
    rc = NEXUS_Memory_Allocate(MEDIAROOM_CERT_SIZE, NULL, (void **) &pCert);
    if(rc == Drm_Success)
    {
        rc = DRM_Mediaroom_GetCertificate(pCert, MEDIAROOM_CERT_SIZE, true);
        if(rc == NEXUS_SUCCESS)
        {
            printf("----------<Mediaroom_Certificate>-------------------");
            for(i=0; i<MEDIAROOM_CERT_SIZE; i++)
            {
            if(i%32==0) printf("\n");
            printf(" %02x", pCert[i]);
            }
            printf("\n---------------------------------------------\n");
        }
        NEXUS_Memory_Free(pCert);
    }

    DRM_Mediaroom_VerifyPb(&result, true);
    printf("[result=%d] AV Pb is %svalid.\n", result, (result==1)?"":"in");


    /* Get Non-AV Pb certificate.  Verify Non-AV Pb in Non-AV Pb certificate. */
    rc = NEXUS_Memory_Allocate(MEDIAROOM_CERT_SIZE, NULL, (void **) &pCert);
    if(rc == Drm_Success)
    {
        rc = DRM_Mediaroom_GetCertificate(pCert, MEDIAROOM_CERT_SIZE, false);
        if(rc == NEXUS_SUCCESS)
        {
            printf("----------<Mediaroom_Certificate>-------------------");
            for(i=0; i<MEDIAROOM_CERT_SIZE; i++)
            {
            if(i%32==0) printf("\n");
            printf(" %02x", pCert[i]);
            }
            printf("\n---------------------------------------------\n");
        }
        NEXUS_Memory_Free(pCert);
    }

    DRM_Mediaroom_VerifyPb(&result, false);
    printf("[result=%d] Non-AV Pb is %svalid.\n", result, (result==1)?"":"in");

    DRM_Mediaroom_Uninit();
#ifdef NXCLIENT_SUPPORT
    NxClient_Uninit();
#else
    NEXUS_Platform_Uninit();
#endif

    return 0;
}
