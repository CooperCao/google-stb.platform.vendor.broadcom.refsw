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
#include "bdsp_raaga_priv.h"

BDBG_MODULE(bdsp_raaga_fwdownload_priv);

static const char * const algoidname[]=
{
    "BDSP_AF_P_AlgoId_eMpegDecode",
    "BDSP_AF_P_AlgoId_eAc3Decode",
    "BDSP_AF_P_AlgoId_eAacDecode",
    "BDSP_AF_P_AlgoId_eAacHeLpSbrDecode",
    "BDSP_AF_P_AlgoId_eDdpDecode",
    "BDSP_AF_P_AlgoId_eDdLosslessDecode",
    "BDSP_AF_P_AlgoId_eLpcmCustomDecode",
    "BDSP_AF_P_AlgoId_eBdLpcmDecode",
    "BDSP_AF_P_AlgoId_eDvdLpcmDecode",
    "BDSP_AF_P_AlgoId_eHdDvdLpcmDecode",
    "BDSP_AF_P_AlgoId_eMpegMcDecode",
    "BDSP_AF_P_AlgoId_eWmaStdDecode",
    "BDSP_AF_P_AlgoId_eWmaProStdDecode",
    "BDSP_AF_P_AlgoId_eMlpDecode",
    "BDSP_AF_P_AlgoId_eDdp71Decode",
    "BDSP_AF_P_AlgoId_eDtsDecode",
    "BDSP_AF_P_AlgoId_eDtsLbrDecode",
    "BDSP_AF_P_AlgoId_eDtsHdDecode",
    "BDSP_AF_P_AlgoId_ePcmWavDecode",
    "BDSP_AF_P_AlgoId_eAmrDecode",
    "BDSP_AF_P_AlgoId_eDraDecode",
    "BDSP_AF_P_AlgoId_eRealAudioLbrDecode",
    "BDSP_AF_P_AlgoId_eDolbyPulseDecode",
    "BDSP_AF_P_AlgoId_eMs10DdpDecode",
    "BDSP_AF_P_AlgoId_eAdpcmDecode",
    "BDSP_AF_P_AlgoId_eG711G726Decode",
    "BDSP_AF_P_AlgoId_eG729Decode",
    "BDSP_AF_P_AlgoId_eVorbisDecode",
    "BDSP_AF_P_AlgoId_eG723_1Decode",
    "BDSP_AF_P_AlgoId_eFlacDecode",
    "BDSP_AF_P_AlgoId_eMacDecode",
    "BDSP_AF_P_AlgoId_eAmrWbDecode",
    "BDSP_AF_P_AlgoId_eiLBCDecode",
    "BDSP_AF_P_AlgoId_eiSACDecode",
    "BDSP_AF_P_AlgoId_eUdcDecode",
    "BDSP_AF_P_AlgoId_eDolbyAacheDecode",
    "BDSP_AF_P_AlgoId_eOpusDecode",
    "BDSP_AF_P_AlgoId_eALSDecode",
    "BDSP_AF_P_AlgoId_eAC4Decode",
    "BDSP_AF_P_AlgoId_eEndOfAudioDecodeAlgos",
    "BDSP_VF_P_AlgoId_eRealVideo9Decode",
    "BDSP_VF_P_AlgoId_eVP6Decode",
    "BDSP_VF_P_AlgoId_eEndOfVideoDecodeAlgos",
    /*  All the Algo Ids for Decoder Frame Sync */
    "BDSP_AF_P_AlgoId_eMpegFrameSync",
    "BDSP_AF_P_AlgoId_eMpegMcFrameSync",
    "BDSP_AF_P_AlgoId_eAdtsFrameSync",
    "BDSP_AF_P_AlgoId_eLoasFrameSync",
    "BDSP_AF_P_AlgoId_eWmaStdFrameSync",
    "BDSP_AF_P_AlgoId_eWmaProFrameSync",
    "BDSP_AF_P_AlgoId_eAc3FrameSync",
    "BDSP_AF_P_AlgoId_eDdpFrameSync",
    "BDSP_AF_P_AlgoId_eDdp71FrameSync",
    "BDSP_AF_P_AlgoId_eDtsFrameSync",
    "BDSP_AF_P_AlgoId_eDtsLbrFrameSync",
    "BDSP_AF_P_AlgoId_eDtsHdFrameSync",
    "BDSP_AF_P_AlgoId_eDtsHdFrameSync_1",
    "BDSP_AF_P_AlgoId_eDtsHdHdDvdFrameSync",
    "BDSP_AF_P_AlgoId_eDdLosslessFrameSync",
    "BDSP_AF_P_AlgoId_eMlpFrameSync",
    "BDSP_AF_P_AlgoId_eMlpHdDvdFrameSync",
    "BDSP_AF_P_AlgoId_ePesFrameSync",
    "BDSP_AF_P_AlgoId_eBdLpcmFrameSync",
    "BDSP_AF_P_AlgoId_eHdDvdLpcmFrameSync",
    "BDSP_AF_P_AlgoId_eDvdLpcmFrameSync",
    "BDSP_AF_P_AlgoId_eDvdLpcmFrameSync_1",
    "BDSP_AF_P_AlgoId_ePcmWavFrameSync",
    "BDSP_AF_P_AlgoId_eDraFrameSync",
    "BDSP_AF_P_AlgoId_eRealAudioLbrFrameSync",
    "BDSP_AF_P_AlgoId_eMs10DdpFrameSync",
    "BDSP_AF_P_AlgoId_eVorbisFrameSync",
    "BDSP_AF_P_AlgoId_eFlacFrameSync",
    "BDSP_AF_P_AlgoId_eMacFrameSync",
    "BDSP_AF_P_AlgoId_eUdcFrameSync",
    "BDSP_AF_P_AlgoId_eAC4FrameSync",
    "BDSP_AF_P_AlgoId_eALSFrameSync",
    "BDSP_AF_P_AlgoId_eEndOfAudioDecFsAlgos",
    "BDSP_VF_P_AlgoId_eRealVideo9FrameSync ",
    "BDSP_VF_P_AlgoId_eVP6FrameSync",
    "BDSP_VF_P_AlgoId_eEndOfVideoDecFsAlgos",

    /*  All the Algo Ids for the stages of encode algorithms */
    "BDSP_AF_P_AlgoId_eAc3Encode",
    "BDSP_AF_P_AlgoId_eMpegL2Encode",
    "BDSP_AF_P_AlgoId_eMpegL3Encode",
    "BDSP_AF_P_AlgoId_eAacLcEncode",
    "BDSP_AF_P_AlgoId_eAacHeEncode",
    "BDSP_AF_P_AlgoId_eDtsEncode",
    "BDSP_AF_P_AlgoId_eDtsBroadcastEncode",
    "BDSP_AF_P_AlgoId_eSbcEncode",
    "BDSP_AF_P_AlgoId_eMs10DDTranscode",
    "BDSP_AF_P_AlgoId_eG711G726Encode",
    "BDSP_AF_P_AlgoId_eG729Encode",
    "BDSP_AF_P_AlgoId_eG723_1Encode",
    "BDSP_AF_P_AlgoId_eG722Encode",
    "BDSP_AF_P_AlgoId_eAmrEncode",
    "BDSP_AF_P_AlgoId_eAmrwbEncode",
    "BDSP_AF_P_AlgoId_eiLBCEncode",
    "BDSP_AF_P_AlgoId_eiSACEncode",
    "BDSP_AF_P_AlgoId_eLpcmEncode",
    "BDSP_AF_P_AlgoId_eOpusEncode",
    "BDSP_AF_P_AlgoId_eDDPEncode",
    "BDSP_AF_P_AlgoId_eEndOfAudioEncodeAlgos",
    "BDSP_VF_P_AlgoId_eH264Encode ",
    "BDSP_VF_P_AlgoId_eX264Encode ",
    "BDSP_VF_P_AlgoId_eXVP8Encode ",
    "BDSP_VF_P_AlgoId_eEndOfVideoEncodeAlgos",


    /*  All the Algo Ids for the stages of encode Algo Frame Syncs */
    "BDSP_AF_P_AlgoId_eAc3EncFrameSync",
    "BDSP_AF_P_AlgoId_eMpegL3EncFrameSync",
    "BDSP_AF_P_AlgoId_eMpegL2EncFrameSync",
    "BDSP_AF_P_AlgoId_eAacLcEncFrameSync",
    "BDSP_AF_P_AlgoId_eAacHeEncFrameSync",
    "BDSP_AF_P_AlgoId_eDtsEncFrameSync",
    "BDSP_AF_P_AlgoId_eEndOfEncFsAlgos",

    /*  All the algo ids for the stages of passthrough */
    "BDSP_AF_P_AlgoId_ePassThru",
    "BDSP_AF_P_AlgoId_eMLPPassThru",
    "BDSP_AF_P_AlgoId_eEndOfAuxAlgos",

    /*  All the Algo Ids for the stages of Post Proc algorithms */
    "BDSP_AF_P_AlgoId_eSrsTruSurroundPostProc",
    "BDSP_AF_P_AlgoId_eSrcPostProc",
    "BDSP_AF_P_AlgoId_eDdbmPostProc",
    "BDSP_AF_P_AlgoId_eDownmixPostProc",
    "BDSP_AF_P_AlgoId_eCustomSurroundPostProc",
    "BDSP_AF_P_AlgoId_eCustomBassPostProc",
    "BDSP_AF_P_AlgoId_eKaraokeCapablePostProc",
    "BDSP_AF_P_AlgoId_eCustomVoicePostProc",
    "BDSP_AF_P_AlgoId_ePeqPostProc",
    "BDSP_AF_P_AlgoId_eAvlPostProc",
    "BDSP_AF_P_AlgoId_ePl2PostProc",
    "BDSP_AF_P_AlgoId_eXenPostProc",
    "BDSP_AF_P_AlgoId_eBbePostProc",
    "BDSP_AF_P_AlgoId_eDsolaPostProc",
    "BDSP_AF_P_AlgoId_eDtsNeoPostProc",
    "BDSP_AF_P_AlgoId_eDDConvert",
    "BDSP_AF_P_AlgoId_eAudioDescriptorFadePostProc",
    "BDSP_AF_P_AlgoId_eAudioDescriptorPanPostProc",
    "BDSP_AF_P_AlgoId_ePCMRouterPostProc",
    "BDSP_AF_P_AlgoId_eWMAPassThrough",
    "BDSP_AF_P_AlgoId_eSrsTruSurroundHDPostProc",
    "BDSP_AF_P_AlgoId_eSrsTruVolumePostProc",
    "BDSP_AF_P_AlgoId_eDolbyVolumePostProc",
    "BDSP_AF_P_AlgoId_eBrcm3DSurroundPostProc",
    "BDSP_AF_P_AlgoId_eFWMixerPostProc",
    "BDSP_AF_P_AlgoId_eMonoDownMixPostProc",
    "BDSP_AF_P_AlgoId_eMs10DDConvert",
    "BDSP_AF_P_AlgoId_eDdrePostProc",
    "BDSP_AF_P_AlgoId_eDv258PostProc",
    "BDSP_AF_P_AlgoId_eDpcmrPostProc",
    "BDSP_AF_P_AlgoId_eGenCdbItbPostProc",
    "BDSP_AF_P_AlgoId_eBtscEncoderPostProc",
    "BDSP_AF_P_AlgoId_eSpeexAECPostProc",
    "BDSP_AF_P_AlgoId_eKaraokePostProc",
    "BDSP_AF_P_AlgoId_eMixerDapv2PostProc",
    "BDSP_AF_P_AlgoId_eOutputFormatterPostProc",
    "BDSP_AF_P_AlgoId_eVocalPostProc",
    "BDSP_AF_P_AlgoId_eFadeCtrlPostProc",
    "BDSP_AF_P_AlgoId_eEndOfPpAlgos",

    /*  All algo Ids for post proc frame sync */
    "BDSP_AF_P_AlgoId_eMixerFrameSync",
    "BDSP_AF_P_AlgoId_eMixerDapv2FrameSync",
    "BDSP_AF_P_AlgoId_eEndOfPpFsAlgos",

    /* All Algo ids for libraries*/
    "BDSP_AF_P_AlgoId_eSysLib",
    "BDSP_AF_P_AlgoId_eAlgoLib",
    "BDSP_AF_P_AlgoId_eIDSCommonLib",
    "BDSP_AF_P_AlgoId_eVidIDSCommonLib",

    "BDSP_AF_P_AlgoId_eEndOfLibAlgos",

        "BDSP_AF_P_AlgoId_eScm1",
        "BDSP_AF_P_AlgoId_eScm2",
        "BDSP_AF_P_AlgoId_eScm3",
        "BDSP_AF_P_AlgoId_eEndOfScmAlgos",

    /* Algo IDs for SCM Task */
    "BDSP_AF_P_AlgoId_eScmTask",
    "BDSP_AF_P_AlgoId_eVideoDecodeTask",
    "BDSP_AF_P_AlgoId_eVideoEncodeTask",
    "BDSP_AF_P_AlgoId_eEndOfTaskAlgos",

    "BDSP_AF_P_AlgoId_eEndOfAlgos",

    "BDSP_AF_P_AlgoId_eMax"

};
/**********************************************************************************
Please keep the following debug related information in-sync with the BDSP_AlgorithmType definition
***********************************************************************************/

static const char * const BDSP_P_AlgoTypeName[]=
{
    "Audio Decode",
    "Audio Passthrough",
    "Audio Encode",
    "Audio Mixer",
    "Audio EchoCanceller",
    "Audio Processing",
    "Video Decode",
    "Video Encode",
    "Security",
    "Custom"
};

#ifdef FWDWNLD_DBG
void BDSP_Raaga_P_FwDwnldBuf_Dump( void *pDeviceHandle)
{
    BDSP_Raaga *pDevice = pDeviceHandle;
    unsigned i=0,j=0;

    BDBG_OBJECT_ASSERT(pDevice, BDSP_Raaga);

    BDBG_MSG((" Image Cache buffer log"));
    for (i=0; i<BDSP_IMG_ID_MAX; i++)
    {
        BDBG_MSG((" ImageId = %d ptr = %p (off = 0x%x) size = %lu",
            i, pDevice->imgCache[i].pMemory,
            pDevice->imgCache[i].offset, (unsigned long)pDevice->imgCache[i].size));
    }
    BDBG_MSG(("--"));
    BDBG_MSG((" Download buffer log - Used when image is not preloaded "));
    for(i=0;i<BDSP_AlgorithmType_eMax; i++)
    {
        BDBG_MSG(( "AlgoType = %s (%d) bufsize = %d",BDSP_P_AlgoTypeName[i], i, pDevice->memInfo.sDwnldMemInfo.AlgoTypeBufs[i].ui32Size));
        for(j=0; j< pDevice->settings.maxAlgorithms[i]; j++)
            BDBG_MSG((" ptr = %p numuser = %d", pDevice->memInfo.sDwnldMemInfo.AlgoTypeBufs[i].DwnldBufUsageInfo[j].pAddr ,
            pDevice->memInfo.sDwnldMemInfo.AlgoTypeBufs[i].DwnldBufUsageInfo[j].numUser));
    }

}
#endif /*FWDWNLD_DBG*/

BERR_Code BDSP_Raaga_P_Dwnld_AudioProc_Algos(void* pDeviceHandle, BDSP_MMA_Memory *pMemory)
{
    const BDSP_Raaga_P_AlgorithmInfo *pInfo;
    unsigned i=0;
    BDSP_Algorithm algorithm;
    bool bDownload = true;
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_Raaga *pDevice = pDeviceHandle;
    BDSP_MMA_Memory MemoryInit = *pMemory;

    BDBG_OBJECT_ASSERT(pDevice, BDSP_Raaga);

    for(algorithm= BDSP_Algorithm_eAudioProcessing_StrtIdx ; algorithm <= BDSP_Algorithm_eAudioProcessing_EndIdx ;algorithm++)
    {
        pInfo = BDSP_Raaga_P_LookupAlgorithmInfo(algorithm);
        BDBG_MSG(("Requesting algorithm %s (%u) num nodes %d at %p", pInfo->pName, algorithm,pInfo->algoExecInfo.NumNodes, pMemory->pAddr));
        if ( pInfo->supported )
        {
            for ( i = 0; i < pInfo->algoExecInfo.NumNodes; i++ )
            {
                BDSP_AF_P_AlgoId algoId = pInfo->algoExecInfo.eAlgoIds[i];
                bDownload = true;

                if ( algoId < BDSP_AF_P_AlgoId_eMax )
                {
                    BDBG_MSG((" AlgoId (%d) %s", algoId, algoidname[algoId]));
                    errCode = BDSP_Raaga_P_RequestImg(pDevice->settings.pImageInterface,pDevice->settings.pImageContext,\
                                                        pDevice->imgCache, BDSP_IMG_ID_CODE(algoId), bDownload, pMemory);
                    IF_ERR_GOTO_error;
                    BDSP_MMA_P_FlushCache((*pMemory), pDevice->imgCache[BDSP_IMG_ID_CODE(algoId)].size);
                    pMemory->pAddr = (void *)((uint8_t *)pMemory->pAddr + pDevice->imgCache[BDSP_IMG_ID_CODE(algoId)].size);
                    pMemory->offset= pMemory->offset + pDevice->imgCache[BDSP_IMG_ID_CODE(algoId)].size;

                    errCode = BDSP_Raaga_P_RequestImg(pDevice->settings.pImageInterface,pDevice->settings.pImageContext,\
                                                        pDevice->imgCache, BDSP_IMG_ID_IFRAME(algoId), bDownload, pMemory);
                    IF_ERR_GOTO_error;
                    BDSP_MMA_P_FlushCache((*pMemory), pDevice->imgCache[BDSP_IMG_ID_IFRAME(algoId)].size);
                    pMemory->pAddr = (void *)((uint8_t *)pMemory->pAddr + pDevice->imgCache[BDSP_IMG_ID_IFRAME(algoId)].size);
                    pMemory->offset= pMemory->offset + pDevice->imgCache[BDSP_IMG_ID_IFRAME(algoId)].size;

                    if ( BDSP_Raaga_P_AlgoHasTables(algoId) )
                    {
                        errCode = BDSP_Raaga_P_RequestImg(pDevice->settings.pImageInterface,pDevice->settings.pImageContext,\
                                                            pDevice->imgCache, BDSP_IMG_ID_TABLE(algoId), bDownload, pMemory);
                        IF_ERR_GOTO_error;
                        BDSP_MMA_P_FlushCache((*pMemory), pDevice->imgCache[BDSP_IMG_ID_TABLE(algoId)].size);
                        pMemory->pAddr = (void *)((uint8_t *)pMemory->pAddr + pDevice->imgCache[BDSP_IMG_ID_TABLE(algoId)].size);
                        pMemory->offset= pMemory->offset + pDevice->imgCache[BDSP_IMG_ID_TABLE(algoId)].size;
                    }

                }

#ifdef FWDWNLD_DBG
                BDBG_MSG((" code ptr = %p (off = %p)",pDevice->imgCache[BDSP_IMG_ID_CODE(algoId)].pMemory, pDevice->imgCache[BDSP_IMG_ID_CODE(algoId)].offset));
                BDBG_MSG((" iframe ptr = %p (off = %p)",pDevice->imgCache[BDSP_IMG_ID_IFRAME(algoId)].pMemory, pDevice->imgCache[BDSP_IMG_ID_IFRAME(algoId)].offset));
                BDBG_MSG((" table ptr = %p (off = %p)",pDevice->imgCache[BDSP_IMG_ID_TABLE(algoId)].pMemory, pDevice->imgCache[BDSP_IMG_ID_TABLE(algoId)].offset));
#endif
            }
        }
    }
    if(pMemory->pAddr > (void *)((uint8_t *)MemoryInit.pAddr + pDevice->memInfo.sDwnldMemInfo.AlgoTypeBufs[BDSP_AlgorithmType_eAudioProcessing].ui32Size))
    {
        BDBG_ERR(("Used memory more than allocated memory.MemInfo size parameter might be \
            corrupted. Used till %p allocated till %p -- exclusive", pMemory->pAddr, \
            (void *)((uint8_t *)MemoryInit.pAddr + pDevice->memInfo.sDwnldMemInfo.AlgoTypeBufs[BDSP_AlgorithmType_eAudioProcessing].ui32Size) ));
        errCode = BERR_INVALID_PARAMETER;
        goto error;
    }

error:
/* All the memory will be freed in the parent function "BDSP_Raaga_P_Alloc_DwnldFwExec" */
 return errCode;
}

/******************************************************************************
Summary:
    This function downloads following at device open time.
            -- Resident Code
            if(Preload == true)
                - Download all algos
            else
                -Just allocated memory for the internal mem in memInfo struct

Input Parameter:
    void *pDeviceHandle:    RAAGA Handle (Device Handle)

*******************************************************************************/
BERR_Code BDSP_Raaga_P_Alloc_DwnldFwExec(
    void *pDeviceHandle
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_Raaga *pDevice = pDeviceHandle;
    BDSP_Raaga_P_DwnldMemInfo *pDwnldMemInfo;
    unsigned imageId,i=0,j=0;
    BDSP_MMA_Memory Memory;
    bool bDownload = true;
    BDSP_Raaga_P_AlgoTypeImgBuf *pAlgoTypeBuf;

    BDBG_ENTER(BDSP_Raaga_P_Alloc_DwnldFwExec);

    BDBG_OBJECT_ASSERT(pDevice, BDSP_Raaga);

    pDwnldMemInfo = &pDevice->memInfo.sDwnldMemInfo;

    if(pDevice->deviceWatchdogFlag == true)
    {
        /* This implementation is specific to watchdog recovery.
             Traverse through all the tasks which were registered on any DSP
             and release all the algorithms one by one. Unregister themselves from the Device*/
        BDSP_RaagaTask *pRaagaTask = NULL;
        for(i=0;i<pDevice->numDsp;i++)
        {
            for(j=0;j<BDSP_RAAGA_MAX_FW_TASK_PER_DSP;j++)
            {
                if(pDevice->taskDetails.pRaagaTask[i][j]!=NULL)
                {
                    pRaagaTask = (BDSP_RaagaTask *)pDevice->taskDetails.pRaagaTask[i][j];
                    /* Traverse through all stages in the task and reset the running flag and task handle */
                    BDSP_STAGE_TRAVERSE_LOOP_BEGIN(pRaagaTask->startSettings.primaryStage->pStageHandle, pStageIterator)
                    BSTD_UNUSED(macroStId);
                    BSTD_UNUSED(macroBrId);
                    {
                        BDSP_Raaga_P_ReleaseAlgorithm(pDevice, pStageIterator->algorithm);
                    }
                    BDSP_STAGE_TRAVERSE_LOOP_END(pStageIterator)

                    pDevice->taskDetails.pRaagaTask[i][BDSP_RAAGA_GET_TASK_INDEX(pRaagaTask->taskId)]  = NULL;
                    /* coverity[double_lock: FALSE] */
                    BKNI_AcquireMutex(pDevice->taskDetails.taskIdMutex);
                    pDevice->taskDetails.taskId[i][BDSP_RAAGA_GET_TASK_INDEX(pRaagaTask->taskId)]   = true;
                    BKNI_ReleaseMutex(pDevice->taskDetails.taskIdMutex);
                    if( pRaagaTask->isStopped == true)
                    {
                        /* This memclear is called to ensure that start Settings for all the task which were stopped before Raaga Open in
                             watchdog recovery are cleared */
                        BKNI_Memset((void *)&pRaagaTask->startSettings, 0, sizeof(pRaagaTask->startSettings));
                    }
                }
            }
        }
    }
    BDBG_MSG(("**************Allocated size %d, ptr = %p Preloaded = %d", pDwnldMemInfo->ui32AllocatedBinSize, pDwnldMemInfo->ImgBuf.pAddr, pDwnldMemInfo->IsImagePreLoaded));

    if(pDwnldMemInfo->ImgBuf.pAddr == NULL ){
            return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    /*Init*/
    BKNI_Memset(pDwnldMemInfo->ImgBuf.pAddr, 0, pDwnldMemInfo->ui32AllocwithGuardBand);
    BDSP_MMA_P_FlushCache(pDwnldMemInfo->ImgBuf,pDwnldMemInfo->ui32AllocwithGuardBand);
    for (imageId=0; imageId<BDSP_IMG_ID_MAX; imageId++)
    {
        pDevice->imgCache[imageId].pMemory = NULL;
        pDevice->imgCache[imageId].offset= 0;
    }
    for(i=0;i<BDSP_AlgorithmType_eMax ; i++ )
    {
        pAlgoTypeBuf = &pDevice->memInfo.sDwnldMemInfo.AlgoTypeBufs[i];
        for(j=0; j< BDSP_RAAGA_MAX_DWNLD_BUFS ; j++ )
        {
            BDSP_INIT_DWNLDBUF( &pAlgoTypeBuf->DwnldBufUsageInfo[j]);
        }
    }
    Memory = pDwnldMemInfo->ImgBuf;

    /*Rest of the Image Code download/allocation */
    if( pDevice->memInfo.sDwnldMemInfo.IsImagePreLoaded == true )
    {
        errCode = BDSP_Raaga_P_PreLoadFwImages(
                        pDevice->settings.pImageInterface,
                        pDevice->settings.pImageContext,
                        pDevice->imgCache,
                        &Memory,
                        pDwnldMemInfo->ui32AllocatedBinSize
                        );
        IF_ERR_GOTO_error;
        BDSP_MMA_P_FlushCache(pDwnldMemInfo->ImgBuf, pDwnldMemInfo->ui32AllocatedBinSize);
    }
    else
    {
        /* System Code download*/
        for ( imageId = 0; imageId < BDSP_SystemImgId_eMax; imageId++ )
        {
            if(pDevice->imgCache[imageId].size == 0 )
            {
                continue;
            }
            errCode = BDSP_Raaga_P_RequestImg(pDevice->settings.pImageInterface,pDevice->settings.pImageContext, pDevice->imgCache, imageId, bDownload, &Memory);
            IF_ERR_GOTO_error;
            BDSP_MMA_P_FlushCache(Memory, pDevice->imgCache[imageId].size);
            Memory.pAddr = (void *)((uint8_t *)Memory.pAddr + pDevice->imgCache[imageId].size);
            Memory.offset= Memory.offset + pDevice->imgCache[imageId].size;
        }
        /* Code download happens during start task when images are not
        preloaded. Only pointers are allocated here.*/
        errCode = BDSP_Raaga_P_AssignMem_DwnldBuf(pDeviceHandle, &Memory);
        IF_ERR_GOTO_error;

        errCode = BDSP_Raaga_P_Dwnld_AudioProc_Algos(pDeviceHandle, &(pDevice->memInfo.sDwnldMemInfo.AlgoTypeBufs[BDSP_AlgorithmType_eAudioProcessing].DwnldBufUsageInfo[0].Memory));
        IF_ERR_GOTO_error;
    }

#ifdef FWDWNLD_DBG
    BDSP_Raaga_P_FwDwnldBuf_Dump(pDeviceHandle);
#endif

    return errCode;

error:
    BDSP_Raaga_P_FreeFwExec(pDeviceHandle);

    BDBG_LEAVE(BDSP_Raaga_P_Alloc_DwnldFwExec);
    return errCode;
}

BERR_Code BDSP_Raaga_P_DownloadStartTimeFWExec(
    void *pContextHandle,
    void *pPrimaryStageHandle
    )
{
    BERR_Code errCode;
    BDSP_RaagaContext *pRaagaContext = (BDSP_RaagaContext *)pContextHandle;
    BDSP_Raaga  *pDevice= pRaagaContext->pDevice;

    BDSP_RaagaStage *pRaagaErrorStage;
    BDSP_RaagaStage *pRaagaPrimaryStage = (BDSP_RaagaStage *)pPrimaryStageHandle;

    BDBG_ASSERT(NULL != pRaagaPrimaryStage);

    BDSP_STAGE_TRAVERSE_LOOP_BEGIN(pRaagaPrimaryStage, pStageIterator)
    BSTD_UNUSED(macroStId);
    BSTD_UNUSED(macroBrId);
    {
        errCode = BDSP_Raaga_P_RequestAlgorithm(pDevice, pStageIterator->algorithm);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            pRaagaErrorStage = pStageIterator;
            goto error;
        }
    }
    BDSP_STAGE_TRAVERSE_LOOP_END(pStageIterator)

    return BERR_SUCCESS;
error:
    BDSP_STAGE_TRAVERSE_LOOP_BEGIN(pRaagaPrimaryStage, pStageIterator)
    BSTD_UNUSED(macroBrId);
    BSTD_UNUSED(macroStId);
    {
        BDSP_Raaga_P_ReleaseAlgorithm(pDevice, pStageIterator->algorithm);
        if (pRaagaErrorStage == pStageIterator)
        {
            break;
        }
    }
    BDSP_STAGE_TRAVERSE_LOOP_END(pStageIterator)

    return errCode;
}

BDSP_Raaga_P_DwnldBufUsageInfo * BDSP_Raaga_P_LookUpFree_DwnldBuf(void * pDeviceHandle,BDSP_Algorithm algorithm, const BDSP_Raaga_P_AlgorithmInfo *pInfo)
{
    BDSP_Raaga *pDevice = pDeviceHandle;
    unsigned i=0;
    BDSP_Raaga_P_DwnldBufUsageInfo *pDwnldBuf = NULL;
    BDSP_Raaga_P_DwnldBufUsageInfo *pDwnldBuf_AlgoPresent = NULL;
    BDSP_Raaga_P_DwnldBufUsageInfo *pDwnldBuf_FreeMem = NULL;
    BDSP_Raaga_P_DwnldBufUsageInfo *pDwnldBuf_ReUseMem = NULL;
    BDSP_Raaga_P_AlgoTypeImgBuf *pAlgoTypeBuf = NULL;
    BDSP_AlgorithmType AlgoType = pInfo->type;

    BDBG_ENTER( BDSP_Raaga_P_LookUpFree_DwnldBuf);
    BDBG_OBJECT_ASSERT(pDevice, BDSP_Raaga);

    /* Make sure the sync between the names and the structure definition is maintained */
    BDBG_ASSERT((sizeof(BDSP_P_AlgoTypeName)/sizeof(char *) ) == BDSP_AlgorithmType_eMax);

    pAlgoTypeBuf = &pDevice->memInfo.sDwnldMemInfo.AlgoTypeBufs[AlgoType];

    /* Understand the usage of memory */
    for(i=0;i< pDevice->settings.maxAlgorithms[AlgoType]; i++)
    {
        pDwnldBuf = &pAlgoTypeBuf->DwnldBufUsageInfo[i];
        if( pDwnldBuf->algorithm== algorithm )
        {
            pDwnldBuf_AlgoPresent = pDwnldBuf;
            break;
        }
        else if (pDwnldBuf->algorithm == BDSP_Algorithm_eMax )
        {
            if( pDwnldBuf_FreeMem == NULL )
                pDwnldBuf_FreeMem = pDwnldBuf ;
        }
        else if (pDwnldBuf->numUser == 0)
        {
            if( pDwnldBuf_ReUseMem  == NULL )
                pDwnldBuf_ReUseMem = pDwnldBuf;

        }
    }

    /****************************************************************
    Decide which location you wish to use
    Priority:
    * Matching algoID ( algo already downloaded )
    * FreeMem ( unused )
    * NumUser == 0 , indicates that that memory is not being used presently

    Input: pDwnldBuf_AlgoPresent , pDwnldBuf_FreeMem, pDwnldBuf_ReUseMem
    Ouput: pDwnldBuf --> buffer to be used
    ****************************************************************/

    pDwnldBuf = NULL;

    if (pDwnldBuf_AlgoPresent != NULL ){
        pDwnldBuf = pDwnldBuf_AlgoPresent;
        pDwnldBuf->bIsExistingDwnldValid = true;
    }
    else if(pDwnldBuf_FreeMem != NULL ){
        pDwnldBuf = pDwnldBuf_FreeMem;
        BDSP_INIT_DWNLDBUF(pDwnldBuf);
    }
    else if(pDwnldBuf_ReUseMem != NULL )
    {
        pDwnldBuf = pDwnldBuf_ReUseMem;
        BDSP_INIT_DWNLDBUF(pDwnldBuf);
    }

    if( pDwnldBuf == NULL ){
        BDBG_ERR(("Insufficient memory for %u %s algorithms.  Max %s algorithms is set to %u",
            pDevice->settings.maxAlgorithms[pInfo->type]+1, BDSP_P_AlgoTypeName[pInfo->type],
            BDSP_P_AlgoTypeName[pInfo->type], pDevice->settings.maxAlgorithms[pInfo->type] ));

        goto    error;
    }

    pDwnldBuf->numUser++;
    pDwnldBuf->algorithm = algorithm;
    BDBG_MSG((" Request Mem = %p for algoId %s (%d)  numuser = %d existing download valid = %d" ,
        pDwnldBuf->Memory.pAddr, BDSP_P_AlgoTypeName[pInfo->type],algorithm , pDwnldBuf->numUser, pDwnldBuf->bIsExistingDwnldValid));

error:
    BDBG_LEAVE( BDSP_Raaga_P_LookUpFree_DwnldBuf);
    return pDwnldBuf;

}



BERR_Code BDSP_Raaga_P_Release_DwnldBufUsage(void * pDeviceHandle,BDSP_Algorithm algorithm, const BDSP_Raaga_P_AlgorithmInfo *pInfo)
{
    BDSP_Raaga *pDevice = pDeviceHandle;
    unsigned i=0;
    BDSP_Raaga_P_DwnldBufUsageInfo *pDwnldBuf = NULL;
    BDSP_Raaga_P_AlgoTypeImgBuf *pAlgoTypeBuf = NULL;
    BDSP_AlgorithmType AlgoType = pInfo->type;
    BERR_Code errCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(pDevice, BDSP_Raaga);

    pAlgoTypeBuf = &pDevice->memInfo.sDwnldMemInfo.AlgoTypeBufs[AlgoType];

    for(i=0;i< pDevice->settings.maxAlgorithms[AlgoType]; i++)
    {
        pDwnldBuf = &pAlgoTypeBuf->DwnldBufUsageInfo[i];
        if( pDwnldBuf->algorithm == algorithm)
        {
            BDBG_ASSERT(pDwnldBuf->numUser > 0 );

            pDwnldBuf->numUser--;

            break;
        }
        else
        {
            pDwnldBuf = NULL;
        }
    }

    if( pDwnldBuf == NULL ){
            BDBG_ERR((" Expected match not found: For Algotype %u Algorithm: %s (%u) ",pInfo->type, pInfo->pName,pInfo->algorithm));
            errCode = BERR_TRACE(BERR_INVALID_PARAMETER);
            goto error;
    }

    BDBG_MSG((" Releasing Download buf %d type %d, algorithm (%d) numuser = %d", i, AlgoType,algorithm,pDwnldBuf->numUser ));

    error:
    return errCode;

}

/******************************************
Downloads all the images required for an algorithm

If the image has to be preloaded, function will get a void ** which will be updated with the number of bytes downloaded

If the image has to be downloaded in the pre-allocated buffers for AlgoTypes, then the void ** input will be NULL and
that pointer will not be updated

*******************************************/
BERR_Code BDSP_Raaga_P_RequestAlgorithm(
    void *pDeviceHandle,
    BDSP_Algorithm algorithm        /*input*/
    )
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_Raaga *pDevice = pDeviceHandle;
    const BDSP_Raaga_P_AlgorithmInfo *pInfo;
    unsigned i=0;
    BDSP_Raaga_P_DwnldBufUsageInfo *pDwnldBufInfo;
    BDSP_MMA_Memory Memory;
    BDSP_MMA_Memory MemoryInit;
    bool bDownload=true;


    BDBG_OBJECT_ASSERT(pDevice, BDSP_Raaga);


    pInfo = BDSP_Raaga_P_LookupAlgorithmInfo(algorithm);
    if ( pInfo->supported )
    {
        if( pInfo->type == BDSP_AlgorithmType_eAudioProcessing){
            BDBG_MSG(("For Algorithm Type = AudioProcessing, Memory is already allocated statically"));
            /* All the BDSP_AlgorithmType_eAudioProcessing type of algorithms are downloaded beforehand in the function "BDSP_Raaga_P_Alloc_DwnldFwExec" */
            return BERR_SUCCESS;
        }

        pDwnldBufInfo = BDSP_Raaga_P_LookUpFree_DwnldBuf(pDeviceHandle, algorithm, pInfo);

        if(pDwnldBufInfo == NULL )
        {
            errCode = BERR_INVALID_PARAMETER;
            errCode = BERR_TRACE(errCode);
            goto error;
        }

        if(pDwnldBufInfo->Memory.pAddr== NULL )
        {
            errCode = BERR_INVALID_PARAMETER;
            errCode = BERR_TRACE(errCode);
            goto error;
        }

        Memory = pDwnldBufInfo->Memory;
        MemoryInit = pDwnldBufInfo->Memory;

        if( pDwnldBufInfo->bIsExistingDwnldValid == true){
             bDownload = false;
        }

        BDBG_MSG(("Requesting algorithm %s (%u) ptr = %p num nodes %d", pInfo->pName, algorithm, Memory.pAddr, pInfo->algoExecInfo.NumNodes));
        for ( i = 0; i < pInfo->algoExecInfo.NumNodes; i++ )
        {
            BDSP_AF_P_AlgoId algoId = pInfo->algoExecInfo.eAlgoIds[i];
            /*BDBG_MSG((" AlgoId (%d) %s", algoId, algoidname[algoId]));*/


            if ( algoId < BDSP_AF_P_AlgoId_eMax )
            {
                errCode = BDSP_Raaga_P_RequestImg(pDevice->settings.pImageInterface,pDevice->settings.pImageContext, pDevice->imgCache, BDSP_IMG_ID_CODE(algoId), bDownload, &Memory);
                IF_ERR_GOTO_error;
                BDSP_MMA_P_FlushCache(Memory, pDevice->imgCache[BDSP_IMG_ID_CODE(algoId)].size);
                Memory.pAddr = (void *)((uint8_t *)Memory.pAddr + pDevice->imgCache[BDSP_IMG_ID_CODE(algoId)].size);
                Memory.offset= Memory.offset + pDevice->imgCache[BDSP_IMG_ID_CODE(algoId)].size;

                errCode = BDSP_Raaga_P_RequestImg(pDevice->settings.pImageInterface,pDevice->settings.pImageContext, pDevice->imgCache, BDSP_IMG_ID_IFRAME(algoId), bDownload, &Memory);
                IF_ERR_GOTO_error;
                BDSP_MMA_P_FlushCache(Memory, pDevice->imgCache[BDSP_IMG_ID_IFRAME(algoId)].size);
                Memory.pAddr = (void *)((uint8_t *)Memory.pAddr + pDevice->imgCache[BDSP_IMG_ID_IFRAME(algoId)].size);
                Memory.offset= Memory.offset + pDevice->imgCache[BDSP_IMG_ID_IFRAME(algoId)].size;

                if ( BDSP_Raaga_P_AlgoHasTables(algoId) )
                {
                    errCode = BDSP_Raaga_P_RequestImg(pDevice->settings.pImageInterface,pDevice->settings.pImageContext, pDevice->imgCache, BDSP_IMG_ID_TABLE(algoId), bDownload, &Memory);
                    IF_ERR_GOTO_error;
                    BDSP_MMA_P_FlushCache(Memory, pDevice->imgCache[BDSP_IMG_ID_TABLE(algoId)].size);
                    Memory.pAddr = (void *)((uint8_t *)Memory.pAddr + pDevice->imgCache[BDSP_IMG_ID_TABLE(algoId)].size);
                    Memory.offset= Memory.offset + pDevice->imgCache[BDSP_IMG_ID_TABLE(algoId)].size;
                }

            }
#ifdef FWDWNLD_DBG
            BDBG_MSG(("Code ptr = %p (off = %p)",pDevice->imgCache[BDSP_IMG_ID_CODE(algoId)].pMemory, pDevice->imgCache[BDSP_IMG_ID_CODE(algoId)].offset));
            BDBG_MSG(("iframe ptr = %p (off = %p)",pDevice->imgCache[BDSP_IMG_ID_IFRAME(algoId)].pMemory,pDevice->imgCache[BDSP_IMG_ID_IFRAME(algoId)].offset));
            BDBG_MSG(("table ptr = %p (off = %p)",pDevice->imgCache[BDSP_IMG_ID_TABLE(algoId)].pMemory, pDevice->imgCache[BDSP_IMG_ID_TABLE(algoId)].offset));
#endif
            }
        if(Memory.pAddr > (void*)((uint8_t *)MemoryInit.pAddr+pDevice->memInfo.sDwnldMemInfo.AlgoTypeBufs[pInfo->type].ui32Size)){
            BDBG_ERR((" Binary downloaded is more than the size allocated for the type %s (%d) ", BDSP_P_AlgoTypeName[pInfo->type], pInfo->type));
            errCode= BERR_INVALID_PARAMETER;
            goto error;
        }

    }
    else
    {
        BDBG_ERR(("Algorithm %u (%s) is not supported", algorithm, pInfo->pName));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    return BERR_SUCCESS;

error:

    return errCode;
}

void BDSP_Raaga_P_ReleaseAlgorithm(
    void *pDeviceHandle,
    BDSP_Algorithm algorithm
    )
{
    BDSP_Raaga *pDevice = pDeviceHandle;
    const BDSP_Raaga_P_AlgorithmInfo *pInfo;
    BERR_Code errCode = BERR_SUCCESS;

    BDBG_ENTER(BDSP_Raaga_P_ReleaseAlgorithm);
    BDBG_OBJECT_ASSERT(pDevice, BDSP_Raaga);

    pInfo = BDSP_Raaga_P_LookupAlgorithmInfo(algorithm);

    if( pDevice->memInfo.sDwnldMemInfo.IsImagePreLoaded == false )
    {
        if ( pInfo->supported )
        {
            if(pInfo->type == BDSP_AlgorithmType_eAudioProcessing)
            {
                /* All the post processing are allocated at the open time and will be just freed at the end.
                Usage count is of no use. So no need to release */
                return;
            }
            BDBG_MSG(("Releasing algorithm %s (%u)", pInfo->pName, algorithm));
            errCode = BDSP_Raaga_P_Release_DwnldBufUsage(pDeviceHandle, algorithm, pInfo);
            if(errCode)
            {
                BDBG_ERR(("Algorithm %s (%u) could not be released properly",pInfo->pName, algorithm));
                errCode = BERR_TRACE(errCode);
            }
        }
    }

    /* We dont release the images in case of secure mode all of them are resident always until Raaga_Close*/

    BDBG_LEAVE(BDSP_Raaga_P_ReleaseAlgorithm);
}
