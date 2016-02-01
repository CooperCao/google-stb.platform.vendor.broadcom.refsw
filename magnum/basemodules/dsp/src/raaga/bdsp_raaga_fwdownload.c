/******************************************************************************
 * (c) 2006-2015 Broadcom Corporation
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
 *
 *****************************************************************************/


#include "bdsp_raaga_fwdownload_priv.h"
#include "bdsp_raaga_fw_algo.h"
#include "bimg.h"


#ifdef BDSP_AUTH
#define BDSP_MEM_P_ConvertAddressToOffset(a,b,c)
#define BDSP_MEM_P_FlushCache(a,b,c) BSTD_UNUSED(a)
#else
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
        void *pAddress,     /* NULL , If its a offline utility */
        unsigned firmware_id,
        BMEM_Handle hHeap
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
      BDBG_ERR(("\nError in fetching next chunk in Image Interface\n"));
      iface->close(image);
      return BERR_TRACE(rc);
    }

    ui32Size =((uint32_t *) data)[0];
    ui32numOfChunks = ((uint32_t *) data)[1];

    pMemAddr = (uint8_t *)pAddress;

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
          BDBG_ERR(("\nError in fetching next chunk in Image Interface\n"));;
          iface->close(image);
          return BERR_TRACE(rc);
      }


      BKNI_Memcpy((void *)pMemAddr,data,ui32ChunkLen);
      pMemAddr +=ui32ChunkLen;
      uiSizeCopied +=  ui32ChunkLen;
    }

    BDSP_MEM_P_FlushCache(hHeap,pAddress,uiSizeCopied);

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
    void * memHandle,
    void * ptr
    )
{
    BERR_Code errCode=BERR_SUCCESS;


    BDBG_ASSERT(imageId < BDSP_IMG_ID_MAX);

    if ( imgCache[imageId].size > 0 )
    {
    BDBG_ASSERT( ptr != NULL );


        imgCache[imageId].pMemory = ptr;

            BDSP_MEM_P_ConvertAddressToOffset((BMEM_Handle )memHandle,
                                            imgCache[imageId].pMemory,
                                            &imgCache[imageId].offset);
        if( bDownload == true )
        {
            errCode = BDSP_Raaga_P_CopyFWImageToMem(pImageInterface,
                                                    pImageContext,
                                                    imgCache[imageId].pMemory,
                                                    imageId,
                                                    (BMEM_Handle )memHandle);
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
                                            void *ptr,
                                            uint32_t ui32AllocatedSize,
                                            void *memHandle)
{
    unsigned imageId;
    void *ptrInit = ptr;
    bool bDownload = true;
    size_t size_check=0;
    BERR_Code errCode=BERR_SUCCESS;


    for (imageId=0; imageId<BDSP_IMG_ID_MAX; imageId++)
    {
        if(imgCache[imageId].size == 0 ){
            continue;
        }
        errCode = BDSP_Raaga_P_RequestImg(pImageInterface,pImageContext, imgCache, imageId, bDownload, memHandle, ptr);
        ptr = (void *)((uint8_t *)ptr + imgCache[imageId].size);
        if ( errCode ){
            errCode = BERR_TRACE(errCode);
            goto error;
        }
        size_check+=imgCache[imageId].size;

    }

    /* Error Check */
    if(ptr > (void *)((uint8_t *) ptrInit + ui32AllocatedSize))
    {
        BDBG_ERR(("Used memory more than allocated memory.MemInfo size parameter might be corrupted.\
        Used till %p Allocated till %p -- exclusive", ptr, \
        (void *)((uint8_t *)ptrInit + ui32AllocatedSize)));
        errCode = BERR_INVALID_PARAMETER;
        goto error;
    }

error:

    return errCode;
}

unsigned BDSP_Raaga_P_AssignAlgoSizes(
                const BIMG_Interface *pImageInterface,
                void **pImageContext,
                BDSP_RaagaImgCacheEntry * pImgCache,
                const BDSP_RaagaUsageOptions *pUsage,
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
