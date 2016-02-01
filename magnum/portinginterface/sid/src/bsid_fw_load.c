/******************************************************************************
* (c) 2004-2015 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its
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
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/

/*
   This code provides the functionality to load the FW image into memory
   and boot the arc
   Eventually, this is likely to be replaced by the common BAFL loader
*/
#include "bstd.h"
#include "bdbg.h"

#include "bsid.h"
#include "bsid_platform.h"
#include "bsid_priv.h"
#include "bsid_dbg.h"
#include "bsid_fw_load.h"
#include "bsid_img.h"   /* for BSID_IMG_FIRMWARE_ID & CHUNK_SIZE */

#ifdef BSID_P_DEBUG_FW_DUMP_TO_FILE
#include <stdio.h>
#endif

BDBG_MODULE(BSID_FW_LOAD);

/* Size of data chunk used in DRAM self-test */
#define BSID_P_TEST_CHUNK                                                  16

/******************************************************************************
* Function name: LoadOne
*
* Comments:
*     Loads a FW chunk into memory at the addresses indicated in the array
*     of the size specified in the array
*
*  - data points to the block/chunk to load
*  - size is the chunk size in 32-bit words
*  -
*
******************************************************************************/
static BERR_Code LoadOne(BSID_Handle hSid, const uint32_t *data, unsigned size, bool *pbEOF)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint32_t  i;
    uint32_t  len, addr;
    uint32_t  testData[BSID_P_TEST_CHUNK];
    void *AuxVirtAddr = NULL;
    *pbEOF = false;

    size /= 4;
    while(size)
    {
        addr = *data++;
        size--;

        if (addr==BIMG_SKIP) {
            continue;
        }
        if (addr==BIMG_EOF) {
            *pbEOF = true;
            return BERR_SUCCESS;
        }
        len = *data++;
        size--;

        if (len>size) {
            return BERR_TRACE(BERR_OS_ERROR);
        }

        /* NOTE: addr is a byte address */
        AuxVirtAddr = (void *)((uint8_t *)(hSid->sFwHwConfig.sCodeMemory.pv_CachedAddr) + addr);

        /* Accumulate the size of the boot segment and text segment together.
           The combined segment will be authenticate by security module. */
        if (addr >= BSID_FW_TEXT_SEGMENT_OFFSET && addr < BSID_FW_DATA_SEGMENT_OFFSET)
        {
            /* NOTE: We only want to modify the loaded code size the first time it is loaded
            if this is a re-load, we dont do anything
            Also, we only care about this boot data if an authentication callback is installed */
            if ((!hSid->bArcReLoad) && (NULL != hSid->pExternalBootCallback))
            {
               hSid->sBootInfo.uiSize += (len << 2);
            }
        }
        if (addr < BSID_FW_DATA_SEGMENT_OFFSET)
        {
            if (true == hSid->bArcReLoad)
            {
               /* if we're re-loading only data, skip sections that are prior to the
               data section (i.e. code sections) */
               size -= len;
               data += len;
               continue;
            }
        }

#ifdef BSID_P_DEBUG_FW_DUMP_TO_FILE
        if (!hSid->bArcReLoad)
        {
            FILE *SIDFwCode = NULL;
            SIDFwCode = fopen("firmware", "wb");
            if (!SIDFwCode)
            {
                BDBG_ERR(("Could not open file for dumping the SID firmware. Make sure to do chmod 777 -R <directory from which the app executable is being run>"));
            }
            else
            {
                fwrite((void*)data, 1, (len * 4), SIDFwCode);
                fclose(SIDFwCode);
            }
        }
#endif

        BKNI_Memcpy(AuxVirtAddr, data, (len * 4));

        if (hSid->sFwHwConfig.bSelfTest)
        {
            for(i=0;i<len;)
            {
                unsigned testSize;
                unsigned j;

                if (i+BSID_P_TEST_CHUNK <= len) {
                    testSize = BSID_P_TEST_CHUNK;
                } else {
                    testSize = len - i;
                }

                {
                    uint32_t *tempAddr = (uint32_t *)((uint8_t *)AuxVirtAddr + (uint32_t)(i*4));
                    BKNI_Memcpy((void*)testData, (void*)(tempAddr), (4 * testSize));
                }

                for(j=0;j<testSize;j++) {
                    if ( testData[j] !=  data[i+j]) {
                        BDBG_ERR(("DDRAM failure at[%d] %#x->%#x(%#x)", i, addr+(i+j)*4, testData[j], data[i+j]));
                        return BERR_TRACE(BERR_UNKNOWN);
                    }
                }
                i+=testSize;
            }
        }

        size -= len;
        data += len;
    }
    return BERR_TRACE(retCode);

}

/******************************************************************************
* Function name: BSID_P_LoadUnit
*
* Comments:
*
******************************************************************************/
BERR_Code BSID_P_LoadCode(BSID_Handle hSid)
{
    BERR_Code retCode;
    uint32_t  uiChunk;
    bool bEOF = false;
    void *pImage = NULL;
    const BIMG_Interface *pImgInterface = hSid->pImgInterface;
    const void *pImageData;

    /* If ARC is being reloaded during resume, do not change boot info,
       nor clear the code block */
    if (!hSid->bArcReLoad)
    {
       /* Don't need to set boot info, nor clear memory, if we're not authenticating FW */
       if (NULL != hSid->pExternalBootCallback)
       {
          hSid->sBootInfo.pStartAddress = hSid->sFwHwConfig.sCodeMemory.pv_CachedAddr;
          hSid->sBootInfo.uiSize = BSID_FW_BOOT_SEGMENT_SIZE;
          /* Init to zero boot plus text segment region. In some platform this region is authenticated,
          the default zeroes will help avoid incorrect math in the gap area between the boot segment
          and the text segment due to otherwise random values.
          The value in BSID_IMG_TEXT_SEGMENT_EST_SIZE is a a pure estimate, it was initially twice as
          much the real combined size between boot and text segments. */
          BKNI_Memset(hSid->sFwHwConfig.sCodeMemory.pv_CachedAddr, 0x00, BSID_FW_BOOT_AND_TEXT_SEGMENT_EST_SIZE);
       }
    }

    retCode = pImgInterface->open((void *)hSid->pImgContext, &pImage, BSID_IMG_FIRMWARE_ID);
    if (BERR_SUCCESS != retCode)
    {
       BDBG_ERR(("Unable to open BIMG interface"));
       return BERR_TRACE(retCode);
    }

    /* don't access chunk 0 - reserved for future header */
    for(uiChunk = 1; /* read until hit EOF marker */; uiChunk++)
    {
        /* NOTE: returned pImageData can be NULL (e.g. at end of firmware image) */
        retCode = pImgInterface->next(pImage, uiChunk, &pImageData, BSID_IMG_CHUNK_SIZE);
        if (retCode != BERR_SUCCESS)
        {
            BDBG_ERR(("Unable to obtain chunk %d from BIMG interface", uiChunk));
            return BERR_TRACE(retCode);
        }
        if (NULL == pImageData)
            break;  /* end of FW chunk array */
        /* FIXME: ideally, this should ensure that pImageData points to a 32-bit aligned memory location */
        retCode = LoadOne(hSid, (uint32_t *)pImageData, BSID_IMG_CHUNK_SIZE, &bEOF);
        if (retCode != BERR_SUCCESS)
        {
            return BERR_TRACE(retCode);
        }
        if (bEOF)
            break;
    }
    pImgInterface->close(pImage);

    return BERR_TRACE(BERR_SUCCESS);
}


/******************************************************************************
* Function name: BSID_P_BootArc
*
* Comments:
*
******************************************************************************/
BERR_Code BSID_P_BootArc(BSID_Handle hSid)
{
    BERR_Code retCode;

    /* SWSTB-403 - always halt the ARC prior to booting */
    BSID_P_HaltArc(hSid->hReg);

    /* sid sw reset */
    BSID_P_ArcSoftwareReset(hSid->hReg);

    /* load sid code onto its memory */
    retCode = BSID_P_LoadCode(hSid);
    if (retCode != BERR_SUCCESS)
    {
        BDBG_ERR(("Error in BSID_P_LoadCode 0x%x", retCode));
        return BERR_TRACE(retCode);
    }
    if (!hSid->bArcReLoad)
    {
       /* This flushes both code and data */
       BMMA_FlushCache( hSid->sFwHwConfig.sCodeMemory.hBlock, hSid->sFwHwConfig.sCodeMemory.pv_CachedAddr, BSID_ARC_CODE_SIZE);
    }
    else
    {
       /* re-load so only flush data */
       BMMA_FlushCache( hSid->sFwHwConfig.sCodeMemory.hBlock, (uint8_t *)(hSid->sFwHwConfig.sCodeMemory.pv_CachedAddr)+BSID_FW_DATA_SEGMENT_OFFSET, BSID_ARC_CODE_SIZE-BSID_FW_DATA_SEGMENT_OFFSET);
    }

    /* SID fw code authenticate (locks down the code region as read-only memory) */
    /* NOTE: This process differs from the description given in bsid.h
       Even if this callback is installed, the PI still boots the ARC */
    /* NOTE: This callback is invoked even if the code region is not altered
       since the callback serves as region verification control regardless of
       whether re-authentication is actually required */
    if (hSid->pExternalBootCallback)
    {
       retCode = (*hSid->pExternalBootCallback)(hSid->pExternalBootCallbackData, &hSid->sBootInfo);

       if (retCode != BERR_SUCCESS)
       {
           BDBG_ERR(("Error in SID fw authentication 0x%x", retCode));
           return BERR_TRACE(retCode);
       }
    }

    /* NOTE: it is unclear whether the PI is supposed to boot the arc after authentication.
       or does security do that? Security team indicate that SID does not require authentication
       so we must boot.  Seems if security were enabled, we wouldn't be *able* to do this */
    BSID_P_ChipEnable(hSid->hReg, hSid->sFwHwConfig.sCodeMemory.ui32_PhysAddr);

    hSid->bArcReLoad = true;  /* subsequent ARC boot during resume are data re-load only */

    /* boot sid arc */
    return BERR_TRACE(BERR_SUCCESS);
}

/***************************
   End of File
****************************/
