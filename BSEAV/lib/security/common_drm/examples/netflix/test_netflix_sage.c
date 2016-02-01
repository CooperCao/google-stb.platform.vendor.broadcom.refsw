/***************************************************************************
 *    (c)2012 Broadcom Corporation
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
#include "drm_netflix.h"
#include "drm_metadata.h"
#include "drm_common.h"

#include "bstd.h"
#include "bdbg.h"
#include "bkni.h"

#define DUMP_DATA_HEX(string,data,size) {        \
   char tmp[512]= "\0";                          \
   uint32_t i=0, l=strlen(string);               \
   sprintf(tmp,"%s",string);                     \
   while( i<size && l < 512) {                   \
    sprintf(tmp+l," %02x", data[i]); ++i; l+=3;} \
   printf(tmp); printf("\n");                    \
}


int main(int argc, char* argv[])
{
    NEXUS_Error                     nrc = NEXUS_SUCCESS;
	DrmRC rc = Drm_Success;
	NEXUS_PlatformSettings platformSettings;
    NEXUS_MemoryAllocationSettings allocationSettings;
    DrmNetFlixSageHandle netflixSageCtx=NULL;
    uint32_t esnSize = 0;
    char * pStr = NULL;
    uint8_t clearBuf[] = { 0x4B, 0x45, 0x59, 0x00, 0x4C, 0x00, 0x45, 0x00, 0x4E, 0x00, 0x3E, 0x00, 0x3C, 0x41, 0x4C };
    uint8_t * pClearBuf = NULL;
    uint8_t * pEncBuf = NULL;
    size_t  allocSize = 0;
    uint32_t encSize = 0;
    uint32_t outSize = 0;

	/* init Nexus */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);

	/* initialize */
    rc = DRM_Netflix_Initialize( "drm.bin", (DrmNetFlixSageHandle *) &netflixSageCtx);
    if((rc != Drm_Success) || (netflixSageCtx == NULL))
    {
        printf("%s:%d - DRM_Netflix_Initialize() failed\n",__FUNCTION__,__LINE__);
        return -1;
    }
    else
    {
        printf("%s:%d - DRM_Netflix_Initialize() succeeded.\n",__FUNCTION__,__LINE__);
    }

    /********************************************************************
     * Get ESN
    ********************************************************************/
    rc = DRM_Netflix_Get_EsnSize((DrmNetFlixSageHandle)netflixSageCtx, &esnSize);
    if( rc != Drm_Success) {
        printf("%s:%d - DRM_Netflix_Get_EsnSize() failed\n",__FUNCTION__,__LINE__);
        goto ErrExit;
    }

    printf("%s:%d - Size of the ESN %d.\n",__FUNCTION__,__LINE__,esnSize);

    NEXUS_Memory_GetDefaultAllocationSettings(&allocationSettings);
    allocSize = esnSize+1;
    allocSize += (16 - (allocSize % 16));
    allocationSettings.alignment = 16;
    nrc = NEXUS_Memory_Allocate(allocSize, &allocationSettings, (void**)&pStr);
    if(nrc != NEXUS_SUCCESS) {
        printf("%s - NEXUS_Memory_Allocate failed for  for the ESN buffer, rc = %d\n", __FUNCTION__, nrc);
        (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto ErrExit;
    }
    memset(pStr,0,allocSize);
    rc = DRM_Netflix_Get_Esn((DrmNetFlixSageHandle)netflixSageCtx, (uint8_t *) pStr, &esnSize);
    if( rc != Drm_Success) {
        printf("%s:%d - DRM_Netflix_Get_Esn() failed\n",__FUNCTION__,__LINE__);
        goto ErrExit;
    }
    pStr[esnSize] = '\0';
    printf("%s:%d - Esn : %s \n",__FUNCTION__,__LINE__,pStr);

    NEXUS_Memory_Free(pStr);

    /********************************************************************/
    /* Netflix_Encrypt                                                  */
    /********************************************************************/
    allocSize = sizeof(clearBuf);
    allocSize += (16 - (allocSize % 16));
    NEXUS_Memory_GetDefaultAllocationSettings(&allocationSettings);
    allocationSettings.alignment = 16;

    NEXUS_Memory_Allocate(allocSize+32, &allocationSettings, (void**)&pEncBuf);
    memset(pEncBuf,0,allocSize+32);
    memcpy((uint8_t*)pEncBuf, clearBuf,sizeof(clearBuf));
    rc = DRM_Netflix_Encrypt((DrmNetFlixSageHandle)netflixSageCtx,
                              pEncBuf,
                              allocSize+32,  /* buffer length including 16-byte alignment + 32-byte digest*/
                             &encSize);     /* result encrypted data size */
    if( rc != Drm_Success) {
        printf("%s:%d - DRM_Netflix_Encrypt() failed\n",__FUNCTION__,__LINE__);
        goto ErrExit;
    }
    printf("%s:%d - DRM_Netflix_Encrypt() success with size %d.\n",__FUNCTION__,__LINE__,encSize);

    /********************************************************************/
    /* Netflix_Decrypt                                                  */
    /********************************************************************/
    allocSize = encSize;
    allocSize += (16 - (allocSize % 16));

    NEXUS_Memory_GetDefaultAllocationSettings(&allocationSettings);
    allocationSettings.alignment = 16;
    NEXUS_Memory_Allocate(encSize, &allocationSettings, (void**)&pClearBuf);
    memcpy((uint8_t*)pClearBuf, pEncBuf, encSize);
    rc = DRM_Netflix_Decrypt((DrmNetFlixSageHandle)netflixSageCtx,
                                    pClearBuf,
                                    encSize,
                                    &outSize);     /* result encrypted data size */
    if( rc != Drm_Success) {
        printf("%s:%d - DRM_Netflix_Decrypt() failed\n",__FUNCTION__,__LINE__);
        goto ErrExit;
    }
    printf("%s:%d - DRM_Netflix_Decrypt() success with size %d\n",__FUNCTION__,__LINE__,outSize);

    if( memcmp( clearBuf,pClearBuf,sizeof(clearBuf)) != 0)
    {
        printf("%s:%d - DRM_Netflix_Decrypt() failed\n",__FUNCTION__,__LINE__);
        goto ErrExit;
    }


ErrExit:

    if(pStr != NULL)
    {
        NEXUS_Memory_Free(pStr);
        pStr = NULL;
    }
    if(pEncBuf != NULL)
    {
        NEXUS_Memory_Free(pEncBuf);
        pEncBuf = NULL;
    }
    if(pClearBuf != NULL)
    {
        NEXUS_Memory_Free(pClearBuf);
        pClearBuf = NULL;
    }

    DRM_Netflix_Finalize((DrmNetFlixSageHandle)netflixSageCtx);
    NEXUS_Platform_Uninit();
	printf("\n\tMAIN - Done\n");
    return 0;
}
