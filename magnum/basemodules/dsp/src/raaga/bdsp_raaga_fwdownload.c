/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include "bdsp_raaga_fwdownload_priv.h"
#include "bdsp_raaga_fw_algo.h"
#include "bimg.h"
#ifndef BDSP_AUTH
#include "bdsp_raaga_mm_priv.h"
#endif

BDBG_MODULE(bdsp_raaga_fwdownload);

/******************************************************************************
Summary:
    This Function Copies the FW exec from Image interface to buffer in the DRAM.
*******************************************************************************/
BERR_Code BDSP_Raaga_P_CopyFWImageToMem(
        const BIMG_Interface *iface,
        void *pImgContext,
        BDSP_MMA_Memory *pMemory,
        unsigned firmware_id
        )
{
    void *image = NULL;
    const void *data = NULL;
    void *context = pImgContext;
    uint8_t *pMemAddr;

    uint32_t ui32Size = 0, ui32numOfChunks = 0,ui32ChunkLen = 0;
    uint32_t ui32Count = 0;
    uint32_t uiSizeCopied=0;
    BERR_Code rc = BERR_SUCCESS;

    BDBG_ASSERT(iface);
    BDBG_ASSERT(pImgContext);

     rc = iface->open(context, &image, firmware_id);
    if (rc != BERR_SUCCESS)
    {
      BDBG_ERR(("Error in Opening the Image Interface"));
      return BERR_TRACE(rc);
    }

    rc = iface->next(image, 0, &data, 8);
    if (rc != BERR_SUCCESS)
    {
      BDBG_ERR(("Error in fetching next chunk in Image Interface"));
      iface->close(image);
      return BERR_TRACE(rc);
    }

    ui32Size =((uint32_t *) data)[0];
    ui32numOfChunks = ((uint32_t *) data)[1];
    pMemAddr = (uint8_t *)pMemory->pAddr;

    /*BDBG_MSG(("Total Size = %d",ui32Size));*/
    for (ui32Count = 1;ui32Count <= ui32numOfChunks; ui32Count++)
    {
      /* The implementation of image interface has to be such that there is
        one header array and then there are firmware binary arrays each having
        bytes of size BDSP_IMG_CHUNK_SIZE. But the last array most probably will
        not have a size equal to exactly BDSP_IMG_CHUNK_SIZE. So we are testing
        for the last chunk here and getting the size based on the total firmware
        binary size and number of chunks*/

      ui32ChunkLen = (ui32Count == ui32numOfChunks) ?  \
          (ui32Size - ((ui32Count - 1)*BDSP_IMG_CHUNK_SIZE)): BDSP_IMG_CHUNK_SIZE;

      BDBG_ASSERT(ui32ChunkLen <= BDSP_IMG_CHUNK_SIZE);
        /*BDBG_MSG(("ui32Count = %d, ui32numOfChunks = %d , ui32ChunkLen =%d , "
                "pAddress = %#x",ui32Count,ui32numOfChunks,ui32ChunkLen,ui32MemAddr)); */

      rc = iface->next(image, ui32Count, &data, ui32ChunkLen);
      if (rc != BERR_SUCCESS)
      {
          BDBG_ERR(("Error in fetching next chunk in Image Interface"));;
          iface->close(image);
          return BERR_TRACE(rc);
      }

      BKNI_Memcpy((void *)pMemAddr,data,ui32ChunkLen);

      pMemAddr +=ui32ChunkLen;
      uiSizeCopied +=  ui32ChunkLen;
    }

    if(uiSizeCopied != ui32Size)
    {
      BDBG_ERR(("FW Image (Id =%#x) not downloaded properly",firmware_id));
    }

    iface->close(image);

    return BERR_SUCCESS;
}

BERR_Code BDSP_Raaga_P_RequestImg(
    const BIMG_Interface *pImageInterface,
    void **pImageContext,
    BDSP_RaagaImgCacheEntry *imgCache,
    unsigned imageId,
    bool bDownload,
    BDSP_MMA_Memory *pMemory
)
{
    BERR_Code errCode=BERR_SUCCESS;


    BDBG_ASSERT(imageId < BDSP_IMG_ID_MAX);

    if ( imgCache[imageId].size > 0 )
    {
            BDBG_ASSERT( pMemory->pAddr != NULL );
            imgCache[imageId].pMemory = pMemory->pAddr;
            imgCache[imageId].offset  = pMemory->offset;
        if( bDownload == true )
        {
            errCode = BDSP_Raaga_P_CopyFWImageToMem(pImageInterface,
                                                    pImageContext,
                                                    pMemory,
                                                    imageId);
            if ( errCode )
            {
                imgCache[imageId].pMemory = NULL;
                imgCache[imageId].offset = 0;
                return BERR_TRACE(errCode);
            }
        }
    }
    return BERR_SUCCESS;
}

BERR_Code BDSP_Raaga_P_PreLoadFwImages(const BIMG_Interface *pImageInterface,
                                            void **pImageContext,
                                            BDSP_RaagaImgCacheEntry *imgCache,
                                            BDSP_MMA_Memory *pMemory,
                                            unsigned ui32AllocatedSize
)
{
    unsigned imageId;
    BDSP_MMA_Memory MemoryInit = *pMemory;
    bool bDownload = true;
    size_t size_check=0;
    BERR_Code errCode=BERR_SUCCESS;


    for (imageId=0; imageId<BDSP_IMG_ID_MAX; imageId++)
    {
        if(imgCache[imageId].size == 0 ){
            continue;
        }
        errCode = BDSP_Raaga_P_RequestImg(pImageInterface,pImageContext, imgCache, imageId, bDownload, pMemory);
        pMemory->pAddr = (void *)((uint8_t *)pMemory->pAddr + imgCache[imageId].size);
        pMemory->offset = pMemory->offset + imgCache[imageId].size;
        if ( errCode ){
            errCode = BERR_TRACE(errCode);
            goto error;
        }
        size_check+=imgCache[imageId].size;
    }

    /* Error Check */
    if(pMemory->pAddr > (void *)((uint8_t *)MemoryInit.pAddr + ui32AllocatedSize))
    {
        BDBG_ERR(("Used memory more than allocated memory.MemInfo size parameter might be corrupted.\
        Used till %p Allocated till %p -- exclusive", pMemory->pAddr, \
        (void *)((uint8_t *)MemoryInit.pAddr + ui32AllocatedSize)));
        errCode = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto error;
    }

error:

    return errCode;
}

unsigned BDSP_Raaga_P_AssignAlgoSizes(
                const BIMG_Interface *pImageInterface,
                void **pImageContext,
                BDSP_RaagaImgCacheEntry * pImgCache,
                const BDSP_UsageOptions *pUsage,
                bool UseBDSPMacro)
{
    unsigned i=0;
    const BDSP_Raaga_P_AlgorithmInfo *pInfo;
    const BDSP_Raaga_P_AlgorithmSupportInfo *pSupportInfo;
    BDSP_Algorithm algorithm;
    uint32_t uiCurrentSize = 0;
    size_t totalSize = 0;

    BKNI_Memset( pImgCache, 0, (sizeof(BDSP_RaagaImgCacheEntry)*BDSP_IMG_ID_MAX));

    /* Compute system image requirements */
    for ( i = 0; i < BDSP_SystemImgId_eMax; i++ )
    {
        /* TODO: There should be a better way to do this. This should probably be handled
        like SCM nodes. However in the current design this might require it to be present in
        the algo ID enumeration which will have implications to vom parse and such. */
        if (i == BDSP_SystemImgId_eScm1_Digest)
        {
            pInfo = BDSP_Raaga_P_LookupAlgorithmInfo(BDSP_Algorithm_eSecurityA);
            if (pInfo->supported == false)
                continue;
        }
        else if (i == BDSP_SystemImgId_eScm2_Digest)
        {
            pInfo = BDSP_Raaga_P_LookupAlgorithmInfo(BDSP_Algorithm_eSecurityB);
            if (pInfo->supported == false)
                continue;
        }

        if(UseBDSPMacro)
        {
            /* Normal Path */
            BDSP_Raaga_P_GetFWSize(pImageInterface, pImageContext, i, &uiCurrentSize);
        }
        else
        {
            /*Memory Estimate API Path*/
             uiCurrentSize = BDSP_SystemID_MemoryReqd[i];
        }
        pImgCache[i].size = uiCurrentSize;
        totalSize += uiCurrentSize;
    }

    /* Compute supported algorithm requirements */
    for ( algorithm = 0; algorithm < BDSP_Algorithm_eMax; algorithm++ )
    {
        bool supported = false;
        BDSP_AF_P_sALGO_EXEC_INFO algoExecInfo;
        const char               *pAlgoName;
        if(UseBDSPMacro)
        {
            pInfo     = BDSP_Raaga_P_LookupAlgorithmInfo(algorithm);
            supported = pInfo->supported;
            pAlgoName = pInfo->pName;
            algoExecInfo = pInfo->algoExecInfo;
        }
        else
        {
            pSupportInfo = BDSP_Raaga_P_LookupAlgorithmSupported(algorithm,pUsage->DolbyCodecVersion);
            supported = pUsage->Codeclist[algorithm];
            pAlgoName = pSupportInfo->pName;
            algoExecInfo = pSupportInfo->algoExecInfo;
        }
        if (supported)
        {
            for ( i = 0; i < algoExecInfo.NumNodes; i++ )
            {
                BDSP_AF_P_AlgoId algoId = algoExecInfo.eAlgoIds[i];
                if ( algoId < BDSP_AF_P_AlgoId_eMax )
                {
                    BDBG_MSG(("Sizing %s (algoid %u)", pAlgoName, algoId));
                    if ( pImgCache[BDSP_IMG_ID_CODE(algoId)].size == 0 )
                    {
                        if(UseBDSPMacro)
                        {
                            /* Normal Path */
                            BDSP_Raaga_P_GetFWSize(pImageInterface, pImageContext, BDSP_IMG_ID_CODE(algoId), &uiCurrentSize);
                        }
                        else
                        {
                            /*Memory Estimate API Path*/
                            uiCurrentSize = BDSP_sNodeInfo[algoId].ui32CodeSize;
                        }
                        pImgCache[BDSP_IMG_ID_CODE(algoId)].size = uiCurrentSize;
                        BDBG_MSG(("img %u size %u", BDSP_IMG_ID_CODE(algoId), uiCurrentSize));
                        totalSize += uiCurrentSize;
                    }

                    if ( pImgCache[BDSP_IMG_ID_IFRAME(algoId)].size == 0 )
                    {
                        if(UseBDSPMacro)
                        {
                            /* Normal Path */
                            BDSP_Raaga_P_GetFWSize(pImageInterface, pImageContext, BDSP_IMG_ID_IFRAME(algoId), &uiCurrentSize);
                        }
                        else
                        {
                            /*Memory Estimate API Path*/
                            uiCurrentSize = BDSP_sNodeInfo[algoId].ui32InterFrmBuffSize;
                        }
                        pImgCache[BDSP_IMG_ID_IFRAME(algoId)].size = uiCurrentSize;
                        BDBG_MSG(("img %u size %u", BDSP_IMG_ID_IFRAME(algoId), uiCurrentSize));
                        totalSize += uiCurrentSize;
                    }

                    if ( BDSP_Raaga_P_AlgoHasTables(algoId) )
                    {
                        if ( pImgCache[BDSP_IMG_ID_TABLE(algoId)].size == 0 )
                        {
                            if(UseBDSPMacro)
                            {
                                /* Normal Path */
                                BDSP_Raaga_P_GetFWSize(pImageInterface, pImageContext, BDSP_IMG_ID_TABLE(algoId), &uiCurrentSize);
                            }
                            else
                            {
                                /*Memory Estimate API Path*/
                                uiCurrentSize = BDSP_sNodeInfo[algoId].ui32RomTableSize;
                            }
                            pImgCache[BDSP_IMG_ID_TABLE(algoId)].size = uiCurrentSize;
                            BDBG_MSG(("img %u size %u", BDSP_IMG_ID_TABLE(algoId), uiCurrentSize));
                            totalSize += uiCurrentSize;
                        }
                    }
                }
            }
        }
    }
    return totalSize;
}

BERR_Code BDSP_Raaga_P_DumpImage(
    void       *pBuffer,
    unsigned    uiBufferSize,
    void      **pvCodeStart,
    unsigned   *puiCodeSize,
    const BIMG_Interface *pImageInterface,
    void **pImageContext
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_RaagaImgCacheEntry *pImgCache, *pImgCachelocal;
    const BDSP_UsageOptions Usage;
    bool bUseBDSPMacro = true;
    unsigned i;
    unsigned uiFwBinSize, uiFwBinSizeWithGuardBand;
    BDSP_MMA_Memory Memory;
    unsigned size = 0;

    BDBG_ENTER(BDSP_Raaga_P_DumpImage);
    *pvCodeStart = NULL;
    *puiCodeSize = 0;

    pImgCache = (BDSP_RaagaImgCacheEntry *)BKNI_Malloc(sizeof(BDSP_RaagaImgCacheEntry)*BDSP_IMG_ID_MAX);
    if(pImgCache == NULL)
    {
        BDBG_ERR(("BDSP_Raaga_P_DumpImage: Cannot allocate memory"));
        errCode=BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        return errCode;
    }
    pImgCachelocal = pImgCache;

    /* Find all the sizes for supported binaries */
    BKNI_Memset(pImgCache, 0, (sizeof(BDSP_RaagaImgCacheEntry)*BDSP_IMG_ID_MAX));
    uiFwBinSize = BDSP_Raaga_P_AssignAlgoSizes(
                pImageInterface,
                pImageContext,
                pImgCache,
                &Usage,
                bUseBDSPMacro);

    for(i=0; i < BDSP_IMG_ID_MAX ; i ++ )
    {
        size = (unsigned)pImgCachelocal->size;
        BDBG_MSG((" IMG[%d] : size = %d", i ,size));
        pImgCachelocal++;
    }

    /* Guard band required for Raaga Code access */
    uiFwBinSizeWithGuardBand = uiFwBinSize + BDSP_CODE_DWNLD_GUARD_BAND_SIZE;

    if( uiFwBinSizeWithGuardBand > uiBufferSize )
    {
        BDBG_ERR((" Allocated memory for binary download less than the required memory. "));
        BDBG_ERR((" Please increase the define value at bdsp_auth.h. "));
        BDBG_ERR((" Allocated Size = %d firmware size = %d Diff(required - allocated ) = %d", uiBufferSize, uiFwBinSizeWithGuardBand, uiFwBinSizeWithGuardBand- uiBufferSize  ));
        errCode=BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto end;
    }

    BKNI_Memset( pBuffer, 0, uiFwBinSizeWithGuardBand );
    Memory.pAddr = pBuffer;
    Memory.offset = 0;
    Memory.hBlock = 0;
    errCode = BDSP_Raaga_P_PreLoadFwImages(
                pImageInterface,
                pImageContext,
                pImgCache,
                &Memory,
                uiFwBinSize);

    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_DumpImage: Preloading of the firmware not successful"));
        goto end;
    }

    *pvCodeStart = pBuffer;
    *puiCodeSize = uiFwBinSizeWithGuardBand;

end:
	BKNI_Free(pImgCache);
    BDBG_LEAVE(BDSP_Raaga_P_DumpImage);
    return errCode;
}
