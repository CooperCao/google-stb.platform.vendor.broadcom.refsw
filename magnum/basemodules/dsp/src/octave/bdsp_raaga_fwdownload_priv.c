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

#include "bdsp_raaga_priv_include.h"

BDBG_MODULE(bdsp_raaga_fwdownload);

BERR_Code BDSP_Raaga_P_ComputeResidentSection(
	BDSP_Raaga_P_CodeDownloadInfo *pCodeInfo,
	unsigned *pMemReqd
)
{
	BERR_Code errCode = BERR_SUCCESS;
	unsigned i = 0;
	BDBG_ENTER(BDSP_Raaga_P_ComputeResidentSection);
	*pMemReqd = 0;
    for (i = 0; i < BDSP_SystemImgId_eMax; i++ )
    {
        *pMemReqd += pCodeInfo->imgInfo[i].ui32Size;
    }

	BDBG_LEAVE(BDSP_Raaga_P_ComputeResidentSection);
	return errCode;
}

BERR_Code BDSP_Raaga_P_ComputeLoadbleSection(
	void     *pDevice,
	unsigned *pMemReqd
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_Algorithm algorithm;
	BDSP_Raaga *pRaaga = (BDSP_Raaga *)pDevice;
	const BDSP_P_AlgorithmSupportInfo *pAlgoSupportInfo;
	unsigned totalSupportedMemory = 0, minRequiredMemory = 0;
	BDBG_ENTER(BDSP_Raaga_P_ComputeLoadbleSection);

	for(algorithm = 0; algorithm < BDSP_Algorithm_eMax; algorithm++)
	{
		pAlgoSupportInfo = BDSP_Raaga_P_LookupAlgorithmSupportInfo(algorithm);
		if(pAlgoSupportInfo->supported)
		{
			totalSupportedMemory += pRaaga->codeInfo.imgInfo[BDSP_IMG_ID_CODE(algorithm)].ui32Size;
			totalSupportedMemory += pRaaga->codeInfo.imgInfo[BDSP_IMG_ID_IDS(algorithm)].ui32Size;
			totalSupportedMemory += pRaaga->codeInfo.imgInfo[BDSP_IMG_ID_TABLE(algorithm)].ui32Size;
			totalSupportedMemory += pRaaga->codeInfo.imgInfo[BDSP_IMG_ID_IFRAME(algorithm)].ui32Size;
		}
	}

	if(pRaaga->codeInfo.preloadImages == true)
	{
		BDBG_MSG(("BDSP_Raaga_P_ComputeLoadbleSection: Authentication is enabled hence preloading total requirement %d",totalSupportedMemory));
		*pMemReqd = totalSupportedMemory;
		minRequiredMemory = totalSupportedMemory;
	}
	else
	{
		unsigned AlgorithmSize = 0;
		BDSP_Raaga_P_LoadableImageInfo *pLoadableImageInfo;
		BDSP_AlgorithmType algoType;
		const BDSP_P_AlgorithmInfo *pAlgoInfo;
		pLoadableImageInfo = &pRaaga->codeInfo.sLoadableImageInfo;
		BKNI_Memset(pLoadableImageInfo, 0, sizeof(BDSP_Raaga_P_LoadableImageInfo));
		for(algoType=0; algoType<BDSP_AlgorithmType_eMax; algoType++)
		{
			pLoadableImageInfo->sAlgoTypeSplitInfo[algoType].numImageBlock = pRaaga->deviceSettings.maxAlgorithms[algoType];
		}

		for(algorithm = 0; algorithm < BDSP_Algorithm_eMax; algorithm++)
		{
			AlgorithmSize = 0;
			pAlgoSupportInfo = BDSP_Raaga_P_LookupAlgorithmSupportInfo(algorithm);
			if(pAlgoSupportInfo->supported)
			{
				pAlgoInfo = BDSP_Raaga_P_LookupAlgorithmInfo(algorithm);
				AlgorithmSize += pRaaga->codeInfo.imgInfo[BDSP_IMG_ID_CODE(algorithm)].ui32Size;
				AlgorithmSize += pRaaga->codeInfo.imgInfo[BDSP_IMG_ID_IDS(algorithm)].ui32Size;
				AlgorithmSize += pRaaga->codeInfo.imgInfo[BDSP_IMG_ID_TABLE(algorithm)].ui32Size;
				AlgorithmSize += pRaaga->codeInfo.imgInfo[BDSP_IMG_ID_IFRAME(algorithm)].ui32Size;
				BDBG_MSG(("Algo Size for Algorithm (%d) %s = %d",algorithm,pAlgoSupportInfo->pName, AlgorithmSize));
				if(pAlgoInfo->type == BDSP_AlgorithmType_eAudioProcessing)
				{
					/* All PP are downloaded together as single Image, hence adding */
					pLoadableImageInfo->sAlgoTypeSplitInfo[pAlgoInfo->type].maxImageSize += AlgorithmSize;
				}
				else if(pAlgoInfo->type < BDSP_AlgorithmType_eMax)
				{
					if(pLoadableImageInfo->sAlgoTypeSplitInfo[pAlgoInfo->type].maxImageSize < AlgorithmSize)
						pLoadableImageInfo->sAlgoTypeSplitInfo[pAlgoInfo->type].maxImageSize = AlgorithmSize;
				}
			}
		}

		/*Calculate the minimum memory required */
		for(algoType=0; algoType<BDSP_AlgorithmType_eMax; algoType++)
		{
			BDBG_MSG(("BDSP_Raaga_P_ComputeLoadbleSection: Algo Type (%d) - Max Image Size (%d) and Number of Image containers(%d)",
				algoType, pLoadableImageInfo->sAlgoTypeSplitInfo[algoType].maxImageSize, pLoadableImageInfo->sAlgoTypeSplitInfo[algoType].numImageBlock));
			minRequiredMemory += (pLoadableImageInfo->sAlgoTypeSplitInfo[algoType].maxImageSize *
				pLoadableImageInfo->sAlgoTypeSplitInfo[algoType].numImageBlock);
		}

		if(minRequiredMemory >= totalSupportedMemory)
		{
			BDBG_MSG(("BDSP_Raaga_P_ComputeLoadbleSection: Total supported Memory is less than allocation requirement, hence enabling preloading"));
			minRequiredMemory = totalSupportedMemory;
			pRaaga->codeInfo.preloadImages = true;
		}
		*pMemReqd = minRequiredMemory;
	}
	pRaaga->codeInfo.sLoadableImageInfo.allocatedSize = minRequiredMemory;
	pRaaga->codeInfo.sLoadableImageInfo.supportedSize = totalSupportedMemory;

	BDBG_MSG(("BDSP_Raaga_P_ComputeLoadbleSection: Minimun Reqd Memory (%d) Total Supported Memory (%d)",minRequiredMemory,totalSupportedMemory));
	BDBG_LEAVE(BDSP_Raaga_P_ComputeLoadbleSection);
	return errCode;
}

BERR_Code BDSP_Raaga_P_AssignAlgoSize(
	const BIMG_Interface *pImageInterface,
	void 			    **pImageContext,
	BDSP_P_FwBuffer      *pImgInfo
)
{
	BERR_Code errCode = BERR_SUCCESS;
	unsigned i=0;
	BDSP_Algorithm algorithm;
	BDSP_P_FwBuffer *pTempImgInfo;
	const BDSP_P_AlgorithmSupportInfo *pAlgoSupportInfo;
	const BDSP_P_AlgorithmInfo        *pAlgoInfo;

	BDBG_ENTER(BDSP_Raaga_P_AssignAlgoSize);
	BKNI_Memset(pImgInfo, 0, (sizeof(BDSP_P_FwBuffer)*BDSP_IMG_ID_MAX));
	for (i=0; i < BDSP_SystemImgId_eMax; i++)
	{
		pTempImgInfo = &pImgInfo[i];
		errCode = BDSP_Raaga_P_GetFWSize(pImageInterface, pImageContext, i, &pTempImgInfo->ui32Size);
		if(errCode != BERR_SUCCESS)
		{
			BDBG_ERR(("BDSP_Raaga_P_AssignAlgoSize: Error Deriving size for Resident Algo %d",i));
			goto end;
		}
		BDBG_MSG(("Size of Resident Algo(%d) %d", i, pTempImgInfo->ui32Size));
	}

	for(algorithm = 0; algorithm < BDSP_Algorithm_eMax; algorithm++)
	{
		pAlgoSupportInfo = BDSP_Raaga_P_LookupAlgorithmSupportInfo(algorithm);
		if(pAlgoSupportInfo->supported)
		{
			pAlgoInfo = BDSP_Raaga_P_LookupAlgorithmInfo(algorithm);
			BDBG_MSG(("Algorithm(%d) %s is supported and hence accounted for sizing and assigning",pAlgoInfo->algorithm, pAlgoInfo->pName));

			pTempImgInfo = &pImgInfo[BDSP_IMG_ID_CODE(algorithm)];
			errCode = BDSP_Raaga_P_GetFWSize(pImageInterface, pImageContext, BDSP_IMG_ID_CODE(algorithm), &pTempImgInfo->ui32Size);
			if(errCode != BERR_SUCCESS)
			{
				BDBG_ERR(("BDSP_Raaga_P_AssignAlgoSize: Error Deriving size for Code of Algorithm(%d) %s",algorithm, pAlgoInfo->pName));
				goto end;
			}
		    BDBG_MSG(("\t Code Size  %d",pTempImgInfo->ui32Size));

			pTempImgInfo = &pImgInfo[BDSP_IMG_ID_IDS(algorithm)];
			if(pAlgoInfo->idsCodeSize)
			{
				errCode = BDSP_Raaga_P_GetFWSize(pImageInterface, pImageContext, BDSP_IMG_ID_IDS(algorithm), &pTempImgInfo->ui32Size);
				if(errCode != BERR_SUCCESS)
				{
					BDBG_ERR(("BDSP_Raaga_P_AssignAlgoSize: Error Deriving size for IDS Code of Algorithm(%d) %s",algorithm, pAlgoInfo->pName));
					goto end;
				}
			}
			BDBG_MSG(("\t IDS Code Size  %d",pTempImgInfo->ui32Size));

			pTempImgInfo = &pImgInfo[BDSP_IMG_ID_TABLE(algorithm)];
			if(pAlgoInfo->romTableSize)
			{
				errCode = BDSP_Raaga_P_GetFWSize(pImageInterface, pImageContext, BDSP_IMG_ID_TABLE(algorithm), &pTempImgInfo->ui32Size);
				if(errCode != BERR_SUCCESS)
				{
					BDBG_ERR(("BDSP_Raaga_P_AssignAlgoSize: Error Deriving size for Table of Algorithm(%d) %s",algorithm, pAlgoInfo->pName));
					goto end;
				}
			}
			BDBG_MSG(("\t ROM Table Size %d",pTempImgInfo->ui32Size));

			pTempImgInfo = &pImgInfo[BDSP_IMG_ID_IFRAME(algorithm)];
			if(pAlgoInfo->compressedInterFrameSize)
			{
				errCode = BDSP_Raaga_P_GetFWSize(pImageInterface, pImageContext, BDSP_IMG_ID_IFRAME(algorithm), &pTempImgInfo->ui32Size);
				if(errCode != BERR_SUCCESS)
				{
					BDBG_ERR(("BDSP_Raaga_P_AssignAlgoSize: Error Deriving size for Interframe of Algorithm(%d) %s",algorithm, pAlgoInfo->pName));
					goto end;
				}
			}
			BDBG_MSG(("\t Compressed Interframe Size  %d",pTempImgInfo->ui32Size));
		}
	}

end:
	BDBG_LEAVE(BDSP_Raaga_P_AssignAlgoSize);
	return errCode;
}

BERR_Code BDSP_Raaga_P_ComputeLoadbleSection_APITool(
	const BDSP_RaagaSettings      *pSettings,
	const BDSP_RaagaUsageOptions  *pUsage,
	BDSP_Raaga_P_CodeDownloadInfo *pCodeDownloadInfo,
	unsigned *pMemReqd
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_Algorithm algorithm;
	const BDSP_P_AlgorithmInfo *pAlgoInfo;
	unsigned totalSupportedMemory = 0, minRequiredMemory = 0;
	BDBG_ENTER(BDSP_Raaga_P_ComputeLoadbleSection_APITool);

	for(algorithm = 0; algorithm < BDSP_Algorithm_eMax; algorithm++)
	{
		if(pUsage->Codeclist[algorithm])
		{
			totalSupportedMemory += pCodeDownloadInfo->imgInfo[BDSP_IMG_ID_CODE(algorithm)].ui32Size;
			totalSupportedMemory += pCodeDownloadInfo->imgInfo[BDSP_IMG_ID_IDS(algorithm)].ui32Size;
			totalSupportedMemory += pCodeDownloadInfo->imgInfo[BDSP_IMG_ID_TABLE(algorithm)].ui32Size;
			totalSupportedMemory += pCodeDownloadInfo->imgInfo[BDSP_IMG_ID_IFRAME(algorithm)].ui32Size;
		}
	}

	if(pSettings->authenticationEnabled == true)
	{
		BDBG_MSG(("BDSP_Raaga_P_ComputeLoadbleSection_APITool: Authentication is enabled hence preloading total requirement %d",totalSupportedMemory));
		*pMemReqd = totalSupportedMemory;
		minRequiredMemory = totalSupportedMemory;
	}
	else
	{
		unsigned AlgorithmSize = 0;
		BDSP_Raaga_P_LoadableImageInfo *pLoadableImageInfo;
		BDSP_AlgorithmType algoType;
		pLoadableImageInfo = &pCodeDownloadInfo->sLoadableImageInfo;
		BKNI_Memset(pLoadableImageInfo, 0, sizeof(BDSP_Raaga_P_LoadableImageInfo));
		for(algoType=0; algoType<BDSP_AlgorithmType_eMax; algoType++)
		{
			pLoadableImageInfo->sAlgoTypeSplitInfo[algoType].numImageBlock = pSettings->maxAlgorithms[algoType];
		}

		for(algorithm = 0; algorithm < BDSP_Algorithm_eMax; algorithm++)
		{
			AlgorithmSize = 0;
			if(pUsage->Codeclist[algorithm])
			{
				pAlgoInfo = BDSP_Raaga_P_LookupAlgorithmInfo(algorithm);
				AlgorithmSize += pCodeDownloadInfo->imgInfo[BDSP_IMG_ID_CODE(algorithm)].ui32Size;
				AlgorithmSize += pCodeDownloadInfo->imgInfo[BDSP_IMG_ID_IDS(algorithm)].ui32Size;
				AlgorithmSize += pCodeDownloadInfo->imgInfo[BDSP_IMG_ID_TABLE(algorithm)].ui32Size;
				AlgorithmSize += pCodeDownloadInfo->imgInfo[BDSP_IMG_ID_IFRAME(algorithm)].ui32Size;
				BDBG_MSG(("Algo Size for Algorithm (%d) %s = %d",algorithm,pAlgoInfo->pName, AlgorithmSize));
				if(pAlgoInfo->type == BDSP_AlgorithmType_eAudioProcessing)
				{
					/* All PP are downloaded together as single Image, hence adding */
					pLoadableImageInfo->sAlgoTypeSplitInfo[pAlgoInfo->type].maxImageSize += AlgorithmSize;
				}
				else if(pAlgoInfo->type < BDSP_AlgorithmType_eMax)
				{
					if(pLoadableImageInfo->sAlgoTypeSplitInfo[pAlgoInfo->type].maxImageSize < AlgorithmSize)
						pLoadableImageInfo->sAlgoTypeSplitInfo[pAlgoInfo->type].maxImageSize = AlgorithmSize;
				}
			}
		}

		/*Calculate the minimum memory required */
		for(algoType=0; algoType<BDSP_AlgorithmType_eMax; algoType++)
		{
			BDBG_MSG(("BDSP_Raaga_P_ComputeLoadbleSection_APITool: Algo Type (%d) - Max Image Size (%d) and Number of Image containers(%d)",
				algoType, pLoadableImageInfo->sAlgoTypeSplitInfo[algoType].maxImageSize, pLoadableImageInfo->sAlgoTypeSplitInfo[algoType].numImageBlock));
			minRequiredMemory += (pLoadableImageInfo->sAlgoTypeSplitInfo[algoType].maxImageSize *
				pLoadableImageInfo->sAlgoTypeSplitInfo[algoType].numImageBlock);
		}

		if(minRequiredMemory >= totalSupportedMemory)
		{
			BDBG_MSG(("BDSP_Raaga_P_ComputeLoadbleSection_APITool: Total supported Memory is less than allocation requirement, hence enabling preloading"));
			minRequiredMemory = totalSupportedMemory;
		}
		*pMemReqd = minRequiredMemory;
	}
	pCodeDownloadInfo->sLoadableImageInfo.allocatedSize = minRequiredMemory;
	pCodeDownloadInfo->sLoadableImageInfo.supportedSize = totalSupportedMemory;

	BDBG_MSG(("BDSP_Raaga_P_ComputeLoadbleSection_APITool: Minimun Reqd Memory (%d) Total Supported Memory (%d)",minRequiredMemory,totalSupportedMemory));
	BDBG_LEAVE(BDSP_Raaga_P_ComputeLoadbleSection_APITool);
	return errCode;
}

BERR_Code BDSP_Raaga_P_AssignAlgoSize_APITool(
	const BDSP_RaagaUsageOptions  *pUsage,
	BDSP_P_FwBuffer 	          *pImgInfo
)
{
	BERR_Code errCode = BERR_SUCCESS;
	unsigned i = 0;
	BDSP_Algorithm algorithm;
	BDSP_P_FwBuffer *pTempImgInfo;
	const BDSP_P_AlgorithmInfo *pAlgoInfo;
	BDBG_ENTER(BDSP_Raaga_P_AssignAlgoSize_APITool);

	BKNI_Memset(pImgInfo, 0, (sizeof(BDSP_P_FwBuffer)*BDSP_IMG_ID_MAX));
	for (i=0; i < BDSP_SystemImgId_eMax; i++)
	{
		pTempImgInfo = &pImgInfo[i];
		pTempImgInfo->ui32Size = BDSP_SystemID_MemoryReqd[i];
		BDBG_MSG(("Size of Resident Algo(%d) %d", i, pTempImgInfo->ui32Size));
	}

	for(algorithm = 0; algorithm < BDSP_Algorithm_eMax; algorithm++)
	{
		if(pUsage->Codeclist[algorithm])
		{
			pAlgoInfo = BDSP_Raaga_P_LookupAlgorithmInfo(algorithm);
			BDBG_MSG(("Algorithm(%d) %s is supported and hence accounted for sizing and assigning",pAlgoInfo->algorithm, pAlgoInfo->pName));
			pTempImgInfo = &pImgInfo[BDSP_IMG_ID_CODE(algorithm)];
			pTempImgInfo->ui32Size = pAlgoInfo->algoCodeSize;
			BDBG_MSG(("\t Code Size  %d",pTempImgInfo->ui32Size));

			pTempImgInfo = &pImgInfo[BDSP_IMG_ID_IDS(algorithm)];
			pTempImgInfo->ui32Size = pAlgoInfo->idsCodeSize;
			BDBG_MSG(("\t IDS Code Size  %d",pTempImgInfo->ui32Size));

			pTempImgInfo = &pImgInfo[BDSP_IMG_ID_TABLE(algorithm)];
			pTempImgInfo->ui32Size = pAlgoInfo->romTableSize;
			BDBG_MSG(("\t ROM Table Size %d",pTempImgInfo->ui32Size));

			pTempImgInfo = &pImgInfo[BDSP_IMG_ID_IFRAME(algorithm)];
			pTempImgInfo->ui32Size = pAlgoInfo->compressedInterFrameSize;
			BDBG_MSG(("\t Interframe Size  %d",pTempImgInfo->ui32Size));
		}
	}

	BDBG_LEAVE(BDSP_Raaga_P_AssignAlgoSize_APITool);
	return errCode;
}

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

   BDBG_MSG(("Downloading Algorithm at PhyAddr = "BDSP_MSG_FMT,BDSP_MSG_ARG(pMemory->offset)));

    BDBG_MSG(("Total Size = %d",ui32Size));
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
      BDBG_MSG(("ui32Count = %d, ui32numOfChunks = %d , ui32ChunkLen =%d,pAddress = %p",
                ui32Count,ui32numOfChunks,ui32ChunkLen,pMemAddr));

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
    BDSP_P_FwBuffer  *pImgCache,
    unsigned imageId,
    bool bDownload,
    BDSP_MMA_Memory *pMemory
)
{
    BERR_Code errCode=BERR_SUCCESS;
    BDBG_ASSERT(imageId < BDSP_IMG_ID_MAX);
    if (pImgCache->ui32Size > 0)
    {
        BDBG_ASSERT( pMemory->pAddr != NULL );
        pImgCache->Buffer = *pMemory;
        if( bDownload == true )
        {
            errCode = BDSP_Raaga_P_CopyFWImageToMem(pImageInterface,
                                                    pImageContext,
                                                    pMemory,
                                                    imageId);
            if (errCode != BERR_SUCCESS)
            {
				BDBG_ERR(("BDSP_Raaga_P_RequestImg: Error in copying the firmware Image %d",imageId));
				pImgCache->Buffer.pAddr   = NULL;
				pImgCache->Buffer.offset  = 0;
				pImgCache->Buffer.hBlock  = NULL;
				return BERR_TRACE(errCode);
            }
        }
    }
    return BERR_SUCCESS;
}

/**********************************************************
Function: BDSP_Raaga_P_DownloadResidentCode

Description : Resident code is downloaded into Raaga code and data space

**********************************************************/
static BERR_Code BDSP_Raaga_P_DownloadResidentCode(void *pDevice)
{
	BERR_Code errCode = BERR_SUCCESS;
	unsigned imageId =0;
	BDSP_Raaga *pRaaga = (BDSP_Raaga *)pDevice;
	BDSP_MMA_Memory Memory;
	BDSP_P_FwBuffer *pImgInfo;

	BDBG_ENTER(BDSP_Raaga_P_DownloadResidentCode);
	BDBG_OBJECT_ASSERT(pRaaga, BDSP_Raaga);

	for(imageId = 0; imageId < BDSP_SystemImgId_eMax; imageId++)
	{
		pImgInfo = &pRaaga->codeInfo.imgInfo[imageId];
		if(pImgInfo->ui32Size == 0 )
		{
			continue;
		}
		BDBG_MSG(("BDSP_Raaga_P_DownloadResidentCode: Memory Requested for Image (%d) is %d",imageId, pImgInfo->ui32Size));
		errCode = BDSP_Raaga_P_RequestMemory(&pRaaga->memInfo.sROMemoryPool, pImgInfo->ui32Size, &Memory);
		if(errCode != BERR_SUCCESS)
		{
			BDBG_ERR(("BDSP_Raaga_P_DownloadResidentCode: Unable to get RO memory to download Image %d",imageId));
			goto end;
		}

		errCode = BDSP_Raaga_P_RequestImg(pRaaga->deviceSettings.pImageInterface,
			pRaaga->deviceSettings.pImageContext,
			pImgInfo,
			imageId,
			true,
			&Memory);
		if(errCode != BERR_SUCCESS)
		{
			BDBG_ERR(("BDSP_Raaga_P_DownloadResidentCode: Unable to Download image %d",imageId));
			goto end;
		}
		BDSP_MMA_P_FlushCache(pImgInfo->Buffer, pImgInfo->ui32Size);
	}

end:
	BDBG_LEAVE(BDSP_Raaga_P_DownloadResidentCode);
	return errCode;
}

static BERR_Code BDSP_Raaga_P_PreLoadFirmwareImages(void *pDevice)
{
	BERR_Code errCode = BERR_SUCCESS;
	unsigned imageId =0;
	BDSP_Raaga *pRaaga = (BDSP_Raaga *)pDevice;
	BDSP_MMA_Memory Memory;
	BDSP_P_FwBuffer *pImgInfo;

	BDBG_ENTER(BDSP_Raaga_P_PreLoadFirmwareImages);
	BDBG_OBJECT_ASSERT(pRaaga, BDSP_Raaga);

	for(imageId = BDSP_SystemImgId_eMax; imageId < BDSP_IMG_ID_MAX; imageId++)
	{
		pImgInfo = &pRaaga->codeInfo.imgInfo[imageId];
		if(pImgInfo->ui32Size == 0 )
		{
			continue;
		}
		BDBG_MSG(("BDSP_Raaga_P_PreLoadFirmwareImages: Memory Requested for Image (%d) is %d",imageId, pImgInfo->ui32Size));
		errCode = BDSP_Raaga_P_RequestMemory(&pRaaga->memInfo.sROMemoryPool, pImgInfo->ui32Size, &Memory);
		if(errCode != BERR_SUCCESS)
		{
			BDBG_ERR(("BDSP_Raaga_P_PreLoadFirmwareImages: Unable to get RO memory to download Image %d",imageId));
			goto end;
		}

		errCode = BDSP_Raaga_P_RequestImg(pRaaga->deviceSettings.pImageInterface,
			pRaaga->deviceSettings.pImageContext,
			pImgInfo,
			imageId,
			true,
			&Memory);
		if(errCode != BERR_SUCCESS)
		{
			BDBG_ERR(("BDSP_Raaga_P_PreLoadFirmwareImages: Unable to Download image %d",imageId));
			goto end;
		}
		BDSP_MMA_P_FlushCache(pImgInfo->Buffer, pImgInfo->ui32Size);
	}

end:
	BDBG_LEAVE(BDSP_Raaga_P_PreLoadFirmwareImages);
	return errCode;
}

static BERR_Code BDSP_Raaga_P_PreLoadPostProcessImages(void *pDevice)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_Raaga *pRaaga = (BDSP_Raaga *)pDevice;
	const BDSP_P_AlgorithmSupportInfo *pAlgoSupportInfo;
	BDSP_Algorithm algorithm;
	BDSP_MMA_Memory Memory;
	unsigned imageId;
	BDSP_P_FwBuffer *pImgInfo;
	BDSP_Raaga_P_AlgoTypeSplitInfo *pAlgoTypeSplitInfo;

	BDBG_ENTER(BDSP_Raaga_P_PreLoadPostProcessImages);
	BDBG_OBJECT_ASSERT(pRaaga, BDSP_Raaga);

	pAlgoTypeSplitInfo = &pRaaga->codeInfo.sLoadableImageInfo.sAlgoTypeSplitInfo[BDSP_AlgorithmType_eAudioProcessing];
	if(pAlgoTypeSplitInfo->numImageBlock > 1)
	{
		BDBG_ERR(("BDSP_Raaga_P_PreLoadPostProcessImages:ERROR Number of Image blocks is more than One!!!!!"));
		BDBG_ASSERT(0);
	}
	Memory = pAlgoTypeSplitInfo->sImageBlockInfo[0].Memory;
    for(algorithm = BDSP_Algorithm_eAudioProcessing_StrtIdx; algorithm <= BDSP_Algorithm_eAudioProcessing_EndIdx; algorithm++)
	{
		pAlgoSupportInfo = BDSP_Raaga_P_LookupAlgorithmSupportInfo(algorithm);
		BDBG_MSG(("BDSP_Raaga_P_PreLoadPostProcessImages: Algorithm(%d) %s", algorithm, pAlgoSupportInfo->pName));
		if(pAlgoSupportInfo->supported)
		{
			for(imageId = BDSP_IMG_ID_CODE(algorithm); imageId <= BDSP_IMG_ID_IDS(algorithm); imageId++)
			{
				pImgInfo = &pRaaga->codeInfo.imgInfo[imageId];
				if(pImgInfo->ui32Size == 0 )
				{
					continue;
				}
				errCode = BDSP_Raaga_P_RequestImg(pRaaga->deviceSettings.pImageInterface,
					pRaaga->deviceSettings.pImageContext,
					pImgInfo,
					imageId,
					true,
					&Memory);
				if(errCode != BERR_SUCCESS)
				{
					BDBG_ERR(("BDSP_Raaga_P_PreLoadPostProcessImages: Unable to Download image %d",imageId));
					goto end;
				}
				BDSP_MMA_P_FlushCache(pImgInfo->Buffer, pImgInfo->ui32Size);
				Memory.pAddr = (void *)((uint8_t *)Memory.pAddr+pImgInfo->ui32Size);
				Memory.offset= Memory.offset + pImgInfo->ui32Size;
			}
			BDBG_MSG(("Algorithm %s: CODE pAddr = %p Offset = "BDSP_MSG_FMT,pAlgoSupportInfo->pName,
				pRaaga->codeInfo.imgInfo[BDSP_IMG_ID_CODE(algorithm)].Buffer.pAddr,BDSP_MSG_ARG(pRaaga->codeInfo.imgInfo[BDSP_IMG_ID_CODE(algorithm)].Buffer.offset)));
			BDBG_MSG(("Algorithm %s: INTERFRAME pAddr = %p Offset = "BDSP_MSG_FMT,pAlgoSupportInfo->pName,
				pRaaga->codeInfo.imgInfo[BDSP_IMG_ID_IFRAME(algorithm)].Buffer.pAddr,BDSP_MSG_ARG(pRaaga->codeInfo.imgInfo[BDSP_IMG_ID_IFRAME(algorithm)].Buffer.offset)));
			BDBG_MSG(("Algorithm %s: ROM TABLE pAddr = %p Offset = "BDSP_MSG_FMT,pAlgoSupportInfo->pName,
				pRaaga->codeInfo.imgInfo[BDSP_IMG_ID_TABLE(algorithm)].Buffer.pAddr,BDSP_MSG_ARG(pRaaga->codeInfo.imgInfo[BDSP_IMG_ID_TABLE(algorithm)].Buffer.offset)));
			BDBG_MSG(("Algorithm %s: IDS pAddr = %p Offset = "BDSP_MSG_FMT,pAlgoSupportInfo->pName,
				pRaaga->codeInfo.imgInfo[BDSP_IMG_ID_IDS(algorithm)].Buffer.pAddr,BDSP_MSG_ARG(pRaaga->codeInfo.imgInfo[BDSP_IMG_ID_IDS(algorithm)].Buffer.offset)));
		}
		if(Memory.pAddr > (void *)((uint8_t *)pAlgoTypeSplitInfo->sImageBlockInfo[0].Memory.pAddr+
					pAlgoTypeSplitInfo->maxImageSize))
		{
			BDBG_MSG(("BDSP_Raaga_P_PreLoadPostProcessImages: Binary Downloaded is more than allocated"));
			errCode= BERR_INVALID_PARAMETER;
			goto end;
		}
	}
end:
	BDBG_LEAVE(BDSP_Raaga_P_PreLoadPostProcessImages);
	return errCode;
}

static BERR_Code BDSP_Raaga_P_AssignMemoryForDynamicDownload(void *pDevice)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_Raaga *pRaaga = (BDSP_Raaga *)pDevice;
	BDSP_MMA_Memory Memory;
	unsigned i = 0;
	BDSP_Raaga_P_AlgoTypeSplitInfo *pAlgoTypeSplitInfo;
	BDSP_AlgorithmType algoType;

	BDBG_ENTER(BDSP_Raaga_P_AssignMemoryForDynamicDownload);
	BDBG_OBJECT_ASSERT(pRaaga, BDSP_Raaga);

	for(algoType = BDSP_AlgorithmType_eAudioDecode; algoType < BDSP_AlgorithmType_eMax; algoType++)
	{
		pAlgoTypeSplitInfo = &pRaaga->codeInfo.sLoadableImageInfo.sAlgoTypeSplitInfo[algoType];
		for(i=0;i<pAlgoTypeSplitInfo->numImageBlock;i++)
		{
			errCode = BDSP_Raaga_P_RequestMemory(&pRaaga->memInfo.sROMemoryPool, pAlgoTypeSplitInfo->maxImageSize, &Memory);
			if(errCode != BERR_SUCCESS)
			{
				BDBG_ERR(("BDSP_Raaga_P_AssignMemoryForDynamicDownload: Unable to get RO memory to split for Algotype %d, size = %d, Number of Images = %d",
					algoType, pAlgoTypeSplitInfo->maxImageSize,pAlgoTypeSplitInfo->numImageBlock ));
				goto end;
			}
			pAlgoTypeSplitInfo->sImageBlockInfo[i].Memory = Memory;
			pAlgoTypeSplitInfo->sImageBlockInfo[i].algorithm = BDSP_Algorithm_eMax;
			pAlgoTypeSplitInfo->sImageBlockInfo[i].numUser   = 0;
			pAlgoTypeSplitInfo->sImageBlockInfo[i].bDownloadValid = false;
		}
	}
end:
	BDBG_LEAVE(BDSP_Raaga_P_AssignMemoryForDynamicDownload);
	return errCode;
}

BERR_Code BDSP_Raaga_P_DownloadCode(void *pDevice)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_Raaga *pRaaga = (BDSP_Raaga *)pDevice;

	BDBG_ENTER(BDSP_Raaga_P_DownloadCode);
	BDBG_OBJECT_ASSERT(pRaaga, BDSP_Raaga);

	errCode = BDSP_Raaga_P_DownloadResidentCode(pDevice);
	if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_DownloadCode: Unable to complete Resident code download"));
		goto end;
	}

	if(pRaaga->codeInfo.preloadImages == true)
	{
		errCode = BDSP_Raaga_P_PreLoadFirmwareImages(pDevice);
		if(errCode != BERR_SUCCESS)
		{
			BDBG_ERR(("BDSP_Raaga_P_DownloadCode: Unable to complete Preload of firmware Image"));
			goto end;
		}
	}
	else
	{
		errCode = BDSP_Raaga_P_AssignMemoryForDynamicDownload(pDevice);
		if(errCode != BERR_SUCCESS)
		{
			BDBG_ERR(("BDSP_Raaga_P_DownloadCode: Unable to Split Memory for Dynamic Download"));
			goto end;
		}

		errCode = BDSP_Raaga_P_PreLoadPostProcessImages(pDevice);
		if(errCode != BERR_SUCCESS)
		{
			BDBG_ERR(("BDSP_Raaga_P_DownloadCode: Unable to Download all the PostProcesses"));
			goto end;
		}
	}

end:
	BDBG_LEAVE(BDSP_Raaga_P_DownloadCode);
	return errCode;
}

static BDSP_Raaga_P_ImageBlockInfo* BDSP_Raaga_P_GetFreeImageBlock(
	BDSP_Algorithm algorithm,
	BDSP_Raaga_P_AlgoTypeSplitInfo *pAlgoTypeSplitInfo
	)
{
	BDSP_Raaga_P_ImageBlockInfo *pImageBlockInfo = NULL;
	unsigned freshIndex=BDSP_RAAGA_MAX_DOWNLOAD_BUFFERS, ReUseIndex=BDSP_RAAGA_MAX_DOWNLOAD_BUFFERS;
	unsigned index=0;
	BDBG_ENTER(BDSP_Raaga_P_GetFreeImageBlock);

	for(index = 0; index<pAlgoTypeSplitInfo->numImageBlock; index++)
	{
		pImageBlockInfo = &pAlgoTypeSplitInfo->sImageBlockInfo[index];
		if(algorithm == pImageBlockInfo->algorithm)
		{
			BDBG_MSG(("BDSP_Raaga_P_GetFreeImageBlock: Using the already downloaded Image Block (%d)", index));
			goto end;
		}
		if(pImageBlockInfo->algorithm == BDSP_Algorithm_eMax)
		{
			freshIndex = index;
		}
		if(pImageBlockInfo->numUser == 0)
		{
			ReUseIndex = index;
		}
	}

	if(freshIndex != BDSP_RAAGA_MAX_DOWNLOAD_BUFFERS)
	{
		BDBG_MSG(("BDSP_Raaga_P_GetFreeImageBlock: Using the Fresh Image Block (%d)", freshIndex));
		pImageBlockInfo = &pAlgoTypeSplitInfo->sImageBlockInfo[freshIndex];
	}
	else if(ReUseIndex != BDSP_RAAGA_MAX_DOWNLOAD_BUFFERS)
	{
		BDBG_MSG(("BDSP_Raaga_P_GetFreeImageBlock: Reusing the Image Block (%d)", ReUseIndex));
		pImageBlockInfo = &pAlgoTypeSplitInfo->sImageBlockInfo[ReUseIndex];
	}
	else
	{
		pImageBlockInfo = NULL;
	}

end:
	BDBG_LEAVE(BDSP_Raaga_P_GetFreeImageBlock);
	return pImageBlockInfo;
}

BERR_Code BDSP_Raaga_P_DownloadAlgorithm(
	void *pDevice,
	BDSP_Algorithm algorithm
)
{
    BERR_Code errCode = BERR_SUCCESS;
	BDSP_Raaga *pRaaga = (BDSP_Raaga *)pDevice;
	const BDSP_P_AlgorithmSupportInfo *pAlgoSupportInfo;
	const BDSP_P_AlgorithmInfo *pAlgoInfo;
	BDSP_Raaga_P_ImageBlockInfo *pImageBlockInfo;
	BDSP_MMA_Memory Memory;
	BDSP_P_FwBuffer *pImgInfo;
	unsigned imageId =0;
	bool bdownload = true;

	BDBG_ENTER(BDSP_Raaga_P_DownloadAlgorithm);
	BDBG_OBJECT_ASSERT(pRaaga, BDSP_Raaga);

	pAlgoSupportInfo = BDSP_Raaga_P_LookupAlgorithmSupportInfo(algorithm);
	BDBG_MSG(("BDSP_Raaga_P_DownloadAlgorithm: Algorithm(%d) %s", algorithm, pAlgoSupportInfo->pName));

	if(pAlgoSupportInfo->supported)
	{
		pAlgoInfo = BDSP_Raaga_P_LookupAlgorithmInfo(algorithm);
		if(pAlgoInfo->type == BDSP_AlgorithmType_eAudioProcessing)
		{
			BDBG_MSG(("BDSP_Raaga_P_DownloadAlgorithm: PostProcesses are downloaded at Open, Algorithm(%d) requested %s",algorithm,pAlgoInfo->pName));
			goto end;
		}

		pImageBlockInfo = BDSP_Raaga_P_GetFreeImageBlock(algorithm, &pRaaga->codeInfo.sLoadableImageInfo.sAlgoTypeSplitInfo[pAlgoInfo->type]);
		if(pImageBlockInfo == NULL)
		{
			BDBG_ERR(("BDSP_Raaga_P_DownloadAlgorithm: Couldn't Find a free Buffer to downaload algorithm(%d) %s",algorithm, pAlgoInfo->pName));
			errCode = BERR_NOT_SUPPORTED;
			goto end;
		}

		if(pImageBlockInfo->bDownloadValid == true)
		{
			pImageBlockInfo->numUser++;
			bdownload = false;
		}

		Memory = pImageBlockInfo->Memory;
		for(imageId = BDSP_IMG_ID_CODE(algorithm); imageId <= BDSP_IMG_ID_IDS(algorithm); imageId++)
		{
			pImgInfo = &pRaaga->codeInfo.imgInfo[imageId];
			if(pImgInfo->ui32Size == 0 )
			{
				continue;
			}
			errCode = BDSP_Raaga_P_RequestImg(pRaaga->deviceSettings.pImageInterface,
				pRaaga->deviceSettings.pImageContext,
				pImgInfo,
				imageId,
				bdownload,
				&Memory);
			if(errCode != BERR_SUCCESS)
			{
				BDBG_ERR(("BDSP_Raaga_P_DownloadAlgorithm: Unable to Download image %d",imageId));
				goto end;
			}
			BDSP_MMA_P_FlushCache(pImgInfo->Buffer, pImgInfo->ui32Size);
			Memory.pAddr = (void *)((uint8_t *)Memory.pAddr+pImgInfo->ui32Size);
			Memory.offset= Memory.offset + pImgInfo->ui32Size;
		}

		if(Memory.pAddr > (void *)((uint8_t *)pImageBlockInfo->Memory.pAddr+
					pRaaga->codeInfo.sLoadableImageInfo.sAlgoTypeSplitInfo[pAlgoInfo->type].maxImageSize))
		{
			BDBG_MSG(("BDSP_Raaga_P_DownloadAlgorithm: Binary Download is more than allocated algorithm(%d) %s",algorithm, pAlgoInfo->pName));
            errCode= BERR_INVALID_PARAMETER;
            goto end;
		}
		BDBG_MSG(("Algorithm %s: CODE pAddr = %p Offset = "BDSP_MSG_FMT,pAlgoInfo->pName,
			pRaaga->codeInfo.imgInfo[BDSP_IMG_ID_CODE(algorithm)].Buffer.pAddr,BDSP_MSG_ARG(pRaaga->codeInfo.imgInfo[BDSP_IMG_ID_CODE(algorithm)].Buffer.offset)));
		BDBG_MSG(("Algorithm %s: INTERFRAME pAddr = %p Offset = "BDSP_MSG_FMT,pAlgoInfo->pName,
			pRaaga->codeInfo.imgInfo[BDSP_IMG_ID_IFRAME(algorithm)].Buffer.pAddr,BDSP_MSG_ARG(pRaaga->codeInfo.imgInfo[BDSP_IMG_ID_IFRAME(algorithm)].Buffer.offset)));
		BDBG_MSG(("Algorithm %s: ROM TABLE pAddr = %p Offset = "BDSP_MSG_FMT,pAlgoInfo->pName,
			pRaaga->codeInfo.imgInfo[BDSP_IMG_ID_TABLE(algorithm)].Buffer.pAddr,BDSP_MSG_ARG(pRaaga->codeInfo.imgInfo[BDSP_IMG_ID_TABLE(algorithm)].Buffer.offset)));
		BDBG_MSG(("Algorithm %s: IDS pAddr = %p Offset = "BDSP_MSG_FMT,pAlgoInfo->pName,
			pRaaga->codeInfo.imgInfo[BDSP_IMG_ID_IDS(algorithm)].Buffer.pAddr,BDSP_MSG_ARG(pRaaga->codeInfo.imgInfo[BDSP_IMG_ID_IDS(algorithm)].Buffer.offset)));
	}
	else
	{
        BDBG_ERR(("Algorithm %u (%s) is not supported", algorithm, pAlgoSupportInfo->pName));
        errCode = BERR_NOT_SUPPORTED;
		goto end;
	}

end:
	BDBG_LEAVE(BDSP_Raaga_P_DownloadAlgorithm);
	return errCode;
}

BERR_Code BDSP_Raaga_P_ReleaseAlgorithm(
	void *pDevice,
	BDSP_Algorithm algorithm
)
{
    BERR_Code errCode = BERR_SUCCESS;
	BDSP_Raaga *pRaaga = (BDSP_Raaga *)pDevice;
	const BDSP_P_AlgorithmInfo *pAlgoInfo;
	BDSP_Raaga_P_AlgoTypeSplitInfo *pAlgoTypeSplitInfo;
	BDSP_Raaga_P_ImageBlockInfo *pImageBlockInfo;
	unsigned index = 0;

	BDBG_ENTER(BDSP_Raaga_P_ReleaseAlgorithm);
	BDBG_OBJECT_ASSERT(pRaaga, BDSP_Raaga);

	pAlgoInfo = BDSP_Raaga_P_LookupAlgorithmInfo(algorithm);
	BDBG_MSG(("BDSP_Raaga_P_ReleaseAlgorithm: Algorithm(%d) %s", algorithm, pAlgoInfo->pName));
	if(pAlgoInfo->type == BDSP_AlgorithmType_eAudioProcessing)
	{
		BDBG_MSG(("BDSP_Raaga_P_ReleaseAlgorithm: PostProcesses are never released Algorithm(%d) requested %s",algorithm,pAlgoInfo->pName));
		goto end;
	}

	pAlgoTypeSplitInfo = &pRaaga->codeInfo.sLoadableImageInfo.sAlgoTypeSplitInfo[pAlgoInfo->type];
	for(index = 0; index<pAlgoTypeSplitInfo->numImageBlock; index++)
	{
		pImageBlockInfo = &pAlgoTypeSplitInfo->sImageBlockInfo[index];
		if(algorithm == pImageBlockInfo->algorithm)
		{
			BDBG_ASSERT(pImageBlockInfo->numUser > 0 );
			pImageBlockInfo->numUser--;
			goto end;
		}
	}

	if(index >= pAlgoTypeSplitInfo->numImageBlock)
	{
		BDBG_ERR(("BDSP_Raaga_P_ReleaseAlgorithm: Couldn't Release the Algorithm(%d) %s", algorithm, pAlgoInfo->pName));
		errCode = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto end;
	}

end:
	BDBG_LEAVE(BDSP_Raaga_P_ReleaseAlgorithm);
	return errCode;
}
