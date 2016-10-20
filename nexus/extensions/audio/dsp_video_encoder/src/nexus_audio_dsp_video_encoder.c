/***************************************************************************
*  Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
* API Description:
*
***************************************************************************/

#include "nexus_audio_module.h"
#include "bchp_xpt_pcroffset.h"
#include "bvee.h"

BDBG_MODULE(nexus_audio_dsp_video_encoder);

BDBG_OBJECT_ID(NEXUS_DspVideoEncoder);

/* Global variable declarations */
extern BVEE_Handle g_NEXUS_veeHandle;

struct NEXUS_VideoEncoder_P_Device;

struct NEXUS_DspVideoEncoder {
    BDBG_OBJECT(NEXUS_DspVideoEncoder)
    bool opened;
    bool started;
    NEXUS_RaveStatus raveStatus;
    BVEE_ChannelHandle veeChannel;
    BVEE_ChannelStartSettings veeStartSettings;
    NEXUS_DspVideoEncoderStartSettings startSettings;
    BVEE_ExtInterruptHandle veeIntHandle;
    struct { /* mapping between MMA and nexus memory blocks, there is an assumption that DSP encoder will return the _same_ memory blocks for as long as it opened */
        BMMA_Block_Handle mma;
        NEXUS_MemoryBlockHandle nexus;
    } frame,meta;
    struct NEXUS_VideoEncoder_P_Device *device;
};

struct NEXUS_DspVideoEncoder_P_Device {
    struct NEXUS_DspVideoEncoder encoders[NEXUS_NUM_DSP_VIDEO_ENCODERS];
    NEXUS_IsrCallbackHandle watchdogCallback;
     bool watchdog;
};

static struct NEXUS_DspVideoEncoder_P_Device g_encoder;

static void NEXUS_DspVideoEncoder_P_Watchdog_isr(void *pParam1, int param2);

#define LOCK_TRANSPORT()    NEXUS_Module_Lock(g_NEXUS_audioModuleData.internalSettings.modules.transport)
#define UNLOCK_TRANSPORT()  NEXUS_Module_Unlock(g_NEXUS_audioModuleData.internalSettings.modules.transport)

#define STC_CONTEXT_STEP ( BCHP_XPT_PCROFFSET_STC1_CTRL - BCHP_XPT_PCROFFSET_STC0_CTRL )

uint32_t NEXUS_DspVideoEncoder_P_GetStcRegAddr(unsigned index)
{
    return  BCHP_XPT_PCROFFSET_STC0_LO + (STC_CONTEXT_STEP * index);
}

void NEXUS_DspVideoEncoder_GetDefaultOpenSettings_priv( NEXUS_DspVideoEncoderOpenSettings *pSettings )
{
    NEXUS_ASSERT_MODULE();
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    return;
}

NEXUS_DspVideoEncoderHandle NEXUS_DspVideoEncoder_Open_priv(unsigned index, const NEXUS_DspVideoEncoderOpenSettings *openSettings)
{
    NEXUS_DspVideoEncoderHandle  encoder;
    BVEE_ChannelOpenSettings channelOpenSettings;
    NEXUS_Error rc;

    NEXUS_ASSERT_MODULE();
    BSTD_UNUSED(openSettings);

    if(index>=NEXUS_NUM_DSP_VIDEO_ENCODERS) { rc=BERR_TRACE(NEXUS_INVALID_PARAMETER); goto err_args;}

    encoder = &g_encoder.encoders[index];
    if ( encoder->opened )
    {
        BDBG_ERR(("DSP Video Encoder %u already opened", index));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto err_args;
    }

    BKNI_Memset(encoder, 0, sizeof(*encoder));
    BDBG_OBJECT_SET(encoder, NEXUS_DspVideoEncoder);
    encoder->frame.mma = NULL;
    encoder->frame.nexus = NULL;
    encoder->meta.mma = NULL;
    encoder->meta.nexus = NULL;

    BVEE_Channel_GetDefaultOpenSettings(&channelOpenSettings);
    channelOpenSettings.maxQueuedPictures = 24;
    channelOpenSettings.resolution.width=1280;
    channelOpenSettings.resolution.height=720;
    rc = BVEE_Channel_Open(g_NEXUS_veeHandle, index, &channelOpenSettings, &encoder->veeChannel);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto err_channel_open;}

    rc = BVEE_Channel_AllocExtInterrupt(encoder->veeChannel, &encoder->veeIntHandle);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto err_int_alloc;}

    encoder->opened = true;

    return encoder;

err_int_alloc:
    BVEE_Channel_Close(encoder->veeChannel);
err_channel_open:
    BKNI_Memset(encoder, 0, sizeof(*encoder));
err_args:
    return NULL;
}

void NEXUS_DspVideoEncoder_Release_priv(NEXUS_DspVideoEncoderHandle encoder)
{
    NEXUS_ASSERT_MODULE();
    BDBG_OBJECT_ASSERT(encoder, NEXUS_DspVideoEncoder);
    if(encoder->frame.nexus) {
        NEXUS_OBJECT_UNREGISTER(NEXUS_MemoryBlock, encoder->frame.nexus, Release);
        NEXUS_MemoryBlock_Free(encoder->frame.nexus);
        encoder->frame.nexus=NULL;
    }
    if(encoder->meta.nexus) {
        NEXUS_OBJECT_UNREGISTER(NEXUS_MemoryBlock, encoder->meta.nexus, Release);
        NEXUS_MemoryBlock_Free(encoder->meta.nexus);
        encoder->meta.nexus=NULL;
    }
}

void NEXUS_DspVideoEncoder_Close_priv(NEXUS_DspVideoEncoderHandle encoder)
{
    NEXUS_ASSERT_MODULE();
    BDBG_OBJECT_ASSERT(encoder, NEXUS_DspVideoEncoder);
    BVEE_Channel_FreeExtInterrupt(encoder->veeChannel, encoder->veeIntHandle);
    BVEE_Channel_Close(encoder->veeChannel);
    BKNI_Memset(encoder, 0, sizeof(*encoder));  /* Also invalidates object ID */
    return;
}

NEXUS_Error NEXUS_DspVideoEncoder_GetExtInterruptInfo_priv(NEXUS_DspVideoEncoderHandle encoder, NEXUS_DspVideoEncoderExtInterruptInfo *extIntInfo)
{
    BERR_Code rc=NEXUS_SUCCESS;
    BVEE_ExtInterruptInfo veeExtIntInfo;

    rc = BVEE_Channel_GetExtInterruptInfo(encoder->veeChannel, encoder->veeIntHandle, &veeExtIntInfo);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto error;}

    extIntInfo->address = veeExtIntInfo.address;
    extIntInfo->bit_num = veeExtIntInfo.bit_num;

error:
    return rc;
}

void NEXUS_DspVideoEncoder_GetUserDataSettings_priv(NEXUS_DspVideoEncoderHandle encoder, NEXUS_DspVideoEncoderUserDataSettings *userDataSettings)
{
    userDataSettings->xudSettings.sinkInterface.pPrivateSinkContext = (void*)encoder->veeChannel;
    userDataSettings->xudSettings.sinkInterface.userDataAdd_isr = (BXUDlib_UserDataSink_Add)BVEE_Channel_UserData_AddBuffers_isr;
    userDataSettings->xudSettings.sinkInterface.userDataStatus_isr = (BXUDlib_UserDataSink_Status)BVEE_Channel_UserData_GetStatus_isr;

    return;
}

void NEXUS_DspVideoEncoder_GetDefaultStartSettings_priv( NEXUS_DspVideoEncoderStartSettings *pSettings )
{
    BVEE_ChannelStartSettings startSettings;

    NEXUS_ASSERT_MODULE();

    BVEE_Channel_GetDefaultStartSettings(&startSettings);

    BKNI_Memset(pSettings, 0, sizeof(*pSettings));

    pSettings->nonRealTime = startSettings.nonRealTime;
    pSettings->codec = NEXUS_P_VideoCodec_FromMagnum(startSettings.codec);
    pSettings->framerate = NEXUS_P_FrameRate_FromMagnum_isrsafe(startSettings.frameRate);
    pSettings->width = startSettings.ui32EncodPicWidth;
    pSettings->height = startSettings.ui32EncodPicHeight;

    return;
}

NEXUS_Error NEXUS_DspVideoEncoder_Start_priv(NEXUS_DspVideoEncoderHandle encoder, const NEXUS_DspVideoEncoderStartSettings *startSettings)
{
    BERR_Code rc;
    unsigned stcChannelIndex;


    NEXUS_ASSERT_MODULE();
    BDBG_OBJECT_ASSERT(encoder, NEXUS_DspVideoEncoder);

    if ( encoder->started )
    {
        BDBG_ERR(("Already running."));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    BVEE_Channel_GetDefaultStartSettings(&encoder->veeStartSettings);
    encoder->veeStartSettings.nonRealTime = startSettings->nonRealTime;
    encoder->veeStartSettings.codec = NEXUS_P_VideoCodec_ToMagnum(startSettings->codec, NEXUS_TransportType_eEs);
    rc = NEXUS_P_FrameRate_ToMagnum_isrsafe(startSettings->framerate, &encoder->veeStartSettings.frameRate);
    encoder->veeStartSettings.ui32EncodPicWidth = startSettings->width;
    encoder->veeStartSettings.ui32EncodPicHeight = startSettings->height;
    encoder->veeStartSettings.ui32TargetBitRate = startSettings->bitrate;
    encoder->veeStartSettings.ui32End2EndDelay = startSettings->encoderDelay;
    encoder->veeStartSettings.pxlformat = BAVC_YCbCrType_e4_2_2; /* TODO : Do not hardcode */

    if(startSettings->width < 1280 && startSettings->height < 720) {
        encoder->veeStartSettings.bDblkEnable = true;
        encoder->veeStartSettings.bSubPelMvEnable = true;
    }

    /* Picture Quality Settings */
    encoder->veeStartSettings.ui32IntraPeriod = 30;
    encoder->veeStartSettings.ui32IDRPeriod = 30;
    encoder->veeStartSettings.bRateControlEnable = 1;

    /*External Interrupt Info */
    encoder->veeStartSettings.extIntCfg.enableInterrupts = true;
    encoder->veeStartSettings.extIntCfg.numInterrupts = 1;
    encoder->veeStartSettings.extIntCfg.interruptInfo[0].interruptRegAddr = startSettings->extIntInfo.address;
    encoder->veeStartSettings.extIntCfg.interruptInfo[0].interruptBit = startSettings->extIntInfo.bit_num;

    encoder->veeStartSettings.sendMetadata = true;
    encoder->veeStartSettings.bSendCC = startSettings->userDataSettings.encodeUserData;

    if(startSettings->raveContext) {
        LOCK_TRANSPORT();
        rc = NEXUS_Rave_GetStatus_priv(startSettings->raveContext, &encoder->raveStatus);
        UNLOCK_TRANSPORT();
        if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto  err_rave_status;}
        encoder->veeStartSettings.pContextMap = &encoder->raveStatus.xptContextMap;
    } else {
        BDBG_WRN(("Encoder start requires Rave Context"));
        goto err_rave_status;
    }

    LOCK_TRANSPORT();
    NEXUS_StcChannel_GetIndex_priv(startSettings->stcChannel, &stcChannelIndex);
    UNLOCK_TRANSPORT();
    encoder->veeStartSettings.stcIndx = (int) stcChannelIndex;

    BDBG_MSG(("Starting VEE Channel %p", (void *)encoder->veeChannel));
    rc = BVEE_Channel_Start(encoder->veeChannel, &encoder->veeStartSettings);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto err_start;}
    encoder->startSettings = *startSettings;
    encoder->started = true;

    return NEXUS_SUCCESS;

err_start:
err_rave_status:
    return BERR_TRACE(rc);
}

void NEXUS_DspVideoEncoder_Stop_priv(NEXUS_DspVideoEncoderHandle encoder)
{

    NEXUS_ASSERT_MODULE();
    BDBG_OBJECT_ASSERT(encoder, NEXUS_DspVideoEncoder);

    BDBG_MSG(("Stopping VEE Channel %p", (void *)encoder->veeChannel));
    BVEE_Channel_Stop(encoder->veeChannel);
    encoder->started = false;

    return;
}

NEXUS_Error NEXUS_DspVideoEncoder_Flush_priv(NEXUS_DspVideoEncoderHandle encoder)
{
    BERR_Code rc;

    NEXUS_ASSERT_MODULE();
    BDBG_OBJECT_ASSERT(encoder, NEXUS_DspVideoEncoder);

    if ( !encoder->started )
    {
        BDBG_ERR(("Encoder is not running.  Cannot flush."));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }


    /* TODO: Do we need disable for flush/flush?  */
    BVEE_Channel_Stop(encoder->veeChannel);

    if(encoder->startSettings.raveContext) {
        LOCK_TRANSPORT();
        NEXUS_Rave_Disable_priv(encoder->startSettings.raveContext);
        NEXUS_Rave_Flush_priv(encoder->startSettings.raveContext);
        UNLOCK_TRANSPORT();
    }

    rc = BVEE_Channel_Start(encoder->veeChannel, &encoder->veeStartSettings);
    if(rc!=BERR_SUCCESS) {return BERR_TRACE(rc);}

    if(encoder->startSettings.raveContext) {
        LOCK_TRANSPORT();
        NEXUS_Rave_Enable_priv(encoder->startSettings.raveContext);
        UNLOCK_TRANSPORT();
    }

    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_DspVideoEncoder_GetBuffer_priv(NEXUS_DspVideoEncoderHandle encoder, const NEXUS_DspVideoEncoderDescriptor **pBuffer, size_t *pSize, const NEXUS_DspVideoEncoderDescriptor **pBuffer2, size_t *pSize2)
{
    NEXUS_Error rc;

    if(g_encoder.watchdog) {
        return NEXUS_NOT_AVAILABLE;
    }

    BDBG_OBJECT_ASSERT(encoder, NEXUS_DspVideoEncoder);
    BDBG_ASSERT(pBuffer);
    BDBG_ASSERT(pSize);
    BDBG_ASSERT(pBuffer2);
    BDBG_ASSERT(pSize2);

    NEXUS_ASSERT_FIELD(NEXUS_DspVideoEncoderDescriptor, flags, BAVC_VideoBufferDescriptor, stCommon.uiFlags);
    NEXUS_ASSERT_FIELD(NEXUS_DspVideoEncoderDescriptor, originalPTS, BAVC_VideoBufferDescriptor, stCommon.uiOriginalPTS);
    NEXUS_ASSERT_FIELD(NEXUS_DspVideoEncoderDescriptor, pts, BAVC_VideoBufferDescriptor, stCommon.uiPTS);
    NEXUS_ASSERT_FIELD(NEXUS_DspVideoEncoderDescriptor, escr, BAVC_VideoBufferDescriptor, stCommon.uiESCR);

    NEXUS_ASSERT_FIELD(NEXUS_DspVideoEncoderDescriptor, ticksPerBit, BAVC_VideoBufferDescriptor, stCommon.uiTicksPerBit);
    NEXUS_ASSERT_FIELD(NEXUS_DspVideoEncoderDescriptor, shr, BAVC_VideoBufferDescriptor, stCommon.iSHR);
    NEXUS_ASSERT_FIELD(NEXUS_DspVideoEncoderDescriptor, offset, BAVC_VideoBufferDescriptor, stCommon.uiOffset);
    NEXUS_ASSERT_FIELD(NEXUS_DspVideoEncoderDescriptor, length, BAVC_VideoBufferDescriptor, stCommon.uiLength);
    NEXUS_ASSERT_FIELD(NEXUS_DspVideoEncoderDescriptor, dts, BAVC_VideoBufferDescriptor, uiDTS);
    NEXUS_ASSERT_FIELD(NEXUS_DspVideoEncoderDescriptor, dataUnitType, BAVC_VideoBufferDescriptor, uiDataUnitType);
    NEXUS_ASSERT_STRUCTURE(NEXUS_DspVideoEncoderDescriptor, BAVC_VideoBufferDescriptor);

    BDBG_CASSERT(NEXUS_DSPVIDEOENCODERDESCRIPTOR_FLAG_ORIGINALPTS_VALID ==  BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_ORIGINALPTS_VALID);
    BDBG_CASSERT(NEXUS_DSPVIDEOENCODERDESCRIPTOR_FLAG_PTS_VALID == BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_PTS_VALID);
    BDBG_CASSERT(NEXUS_DSPVIDEOENCODERDESCRIPTOR_FLAG_FRAME_START ==  BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_FRAME_START);
    BDBG_CASSERT(NEXUS_DSPVIDEOENCODERDESCRIPTOR_FLAG_EOS ==  BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_EOS );
    BDBG_CASSERT(NEXUS_DSPVIDEOENCODERDESCRIPTOR_FLAG_METADATA == BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_METADATA );
    BDBG_CASSERT(NEXUS_DSPVIDEOENCODERDESCRIPTOR_VIDEOFLAG_DTS_VALID == BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_DTS_VALID);
    BDBG_CASSERT(NEXUS_DSPVIDEOENCODERDESCRIPTOR_VIDEOFLAG_RAP == BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_RAP);
    BDBG_CASSERT(NEXUS_DSPVIDEOENCODERDESCRIPTOR_VIDEOFLAG_DATA_UNIT_START == BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_DATA_UNIT_START);

    rc = BVEE_Channel_GetBufferDescriptors(encoder->veeChannel, (void *)pBuffer, pSize, (void *)pBuffer2, pSize2);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc); goto error;}

    return NEXUS_SUCCESS;
error:
    return rc;
}

NEXUS_Error NEXUS_DspVideoEncoder_ReadComplete_priv(NEXUS_DspVideoEncoderHandle encoder, unsigned descriptorsCompleted)
{
    NEXUS_Error rc;

    BDBG_OBJECT_ASSERT(encoder, NEXUS_DspVideoEncoder);

    rc = BVEE_Channel_ConsumeBufferDescriptors(encoder->veeChannel, descriptorsCompleted);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc); goto error;}

    return NEXUS_SUCCESS;
error:
    return rc;
}

NEXUS_Error NEXUS_DspVideoEncoder_GetStatus_priv(NEXUS_DspVideoEncoderHandle encoder, NEXUS_DspVideoEncoderStatus *pStatus)
{
    NEXUS_Error rc;
    BAVC_VideoBufferStatus bufferStatus;
    BVEE_ChannelStatus status;

    BDBG_OBJECT_ASSERT(encoder, NEXUS_DspVideoEncoder);
    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    rc = BVEE_Channel_GetBufferStatus(encoder->veeChannel, &bufferStatus);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc); goto error;}

    pStatus->bufferBase = bufferStatus.stCommon.pFrameBufferBaseAddress;
    pStatus->metadataBufferBase = bufferStatus.stCommon.pMetadataBufferBaseAddress;

    BVEE_Channel_GetStatus(encoder->veeChannel, &status);
    pStatus->picturesReceived = status.ui32TotalFramesRecvd;
    pStatus->picturesEncoded = status.ui32TotalFramesEncoded;
    pStatus->picturesDroppedFRC = status.ui32TotalFramesDropedForFRC;

    if ( NULL != bufferStatus.stCommon.hFrameBufferBlock ) {
        if(encoder->frame.nexus==NULL) {
            NEXUS_Module_Lock(g_NEXUS_audioModuleData.internalSettings.modules.core);
            encoder->frame.nexus = NEXUS_MemoryBlock_FromMma_priv( bufferStatus.stCommon.hFrameBufferBlock );
            NEXUS_Module_Unlock(g_NEXUS_audioModuleData.internalSettings.modules.core);
            if(encoder->frame.nexus==NULL) {
                rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);goto error;
            }
            encoder->frame.mma = bufferStatus.stCommon.hFrameBufferBlock;
            NEXUS_OBJECT_REGISTER(NEXUS_MemoryBlock, encoder->frame.nexus, Acquire);
        }
        if(encoder->frame.mma != bufferStatus.stCommon.hFrameBufferBlock) {
            rc=BERR_TRACE(NEXUS_NOT_SUPPORTED);goto error;
        }
        pStatus->bufferBlock = encoder->frame.nexus;
    }
    if ( NULL != bufferStatus.stCommon.hMetadataBufferBlock ) {
        if(encoder->meta.nexus==NULL) {
            NEXUS_Module_Lock(g_NEXUS_audioModuleData.internalSettings.modules.core);
            encoder->meta.nexus = NEXUS_MemoryBlock_FromMma_priv( bufferStatus.stCommon.hMetadataBufferBlock);
            NEXUS_Module_Unlock(g_NEXUS_audioModuleData.internalSettings.modules.core);
            if(encoder->meta.nexus==NULL) {
                rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);goto error;
            }
            encoder->meta.mma = bufferStatus.stCommon.hMetadataBufferBlock;
            NEXUS_OBJECT_REGISTER(NEXUS_MemoryBlock, encoder->meta.nexus, Acquire);
        }
        if(encoder->meta.mma != bufferStatus.stCommon.hMetadataBufferBlock) {
            rc=BERR_TRACE(NEXUS_NOT_SUPPORTED);goto error;
        }
        pStatus->metadataBufferBlock = encoder->meta.nexus;
    }
    return NEXUS_SUCCESS;

error:
    return BERR_TRACE(rc);
}

void NEXUS_DspVideoEncoder_GetRaveSettings_priv(NEXUS_RaveOpenSettings *raveSettings)
{
    NEXUS_ASSERT_MODULE();

    raveSettings->config.Cdb.Length = 256*1024;
    raveSettings->config.Cdb.Alignment = 4;
    raveSettings->config.Itb.Length = 128*1024;
    raveSettings->config.Itb.Alignment = 4;
#if BSTD_CPU_ENDIAN == BSTD_ENDIAN_LITTLE
    raveSettings->config.Cdb.LittleEndian = true;
#else
    raveSettings->config.Cdb.LittleEndian = false;
#endif

    return;
}

#if NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
NEXUS_Error NEXUS_DspVideoEncoder_EnqueuePicture_isr(NEXUS_DspVideoEncoderHandle encoder, BAVC_EncodePictureBuffer *picture)
{
    BVEE_PictureDescriptor descriptor;
    BERR_Code rc;

    if(g_encoder.watchdog) {
        return NEXUS_NOT_AVAILABLE;
    }

    BKNI_Memset(&descriptor, 0, sizeof(descriptor));
    descriptor.hLumaBlock = picture->hLumaBlock;
    descriptor.ulLumaOffset = picture->ulLumaOffset;
    descriptor.hChromaBlock = picture->hChromaBlock;
    descriptor.ulChromaOffset = picture->ulChromaOffset;
    descriptor.bStriped = picture->bStriped;
    descriptor.ulStripeWidth = picture->ulStripeWidth;
    descriptor.ulLumaNMBY = picture->ulLumaNMBY;
    descriptor.ulChromaNMBY = picture->ulChromaNMBY;
    descriptor.width = picture->ulWidth;
    descriptor.height = picture->ulHeight;
    descriptor.polarity = picture->ePolarity;
    descriptor.sarHorizontal = picture->ulAspectRatioX;
    descriptor.sarVertical = picture->ulAspectRatioY;
    descriptor.frameRate = picture->eFrameRate;
    descriptor.originalPts.ui32CurrentPTS = picture->ulOriginalPTS;
    descriptor.ulSTCSnapshotLo = picture->ulSTCSnapshotLo;
    descriptor.ulSTCSnapshotHi = picture->ulSTCSnapshotHi;
    descriptor.ulPictureId = picture->ulPictureId;
    descriptor.bCadenceLocked = picture->bCadenceLocked;
    descriptor.ePicStruct = picture->ePicStruct;
    descriptor.h2H1VLumaBlock = picture->h2H1VLumaBlock;
    descriptor.ul2H1VLumaOffset = picture->ul2H1VLumaOffset;
    descriptor.h2H2VLumaBlock = picture->h2H2VLumaBlock;
    descriptor.ul2H2VLumaOffset = picture->ul2H2VLumaOffset;

    BDBG_MSG(("Enqueue %p %u %08x%08x", (void *)descriptor.hLumaBlock, descriptor.ulPictureId, descriptor.ulSTCSnapshotHi, descriptor.ulSTCSnapshotLo));

    rc = BVEE_Channel_EnqueuePicture_isr(encoder->veeChannel, &descriptor);

    return rc;
}
NEXUS_Error NEXUS_DspVideoEncoder_DequeuePicture_isr(NEXUS_DspVideoEncoderHandle encoder, BAVC_EncodePictureBuffer *picture)
{
    BVEE_PictureDescriptor descriptor;
    BERR_Code rc;

    BKNI_Memset(picture, 0, sizeof(*picture));

    if(g_encoder.watchdog) {
    return NEXUS_NOT_AVAILABLE;
    }

    rc = BVEE_Channel_DequeuePicture_isr(encoder->veeChannel, &descriptor);
    if(rc!=BERR_SUCCESS) {return BERR_TRACE(rc);}

    BDBG_MSG(("Dequeue %p:%u", (void *)descriptor.hLumaBlock, descriptor.ulPictureId));

    picture->hLumaBlock = descriptor.hLumaBlock;
    picture->ulPictureId = descriptor.ulPictureId;

    return rc;
}
#else /* !NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT */
NEXUS_Error NEXUS_DspVideoEncoder_EnqueuePicture_isr(NEXUS_DspVideoEncoderHandle encoder, NEXUS_DspVideoEncoderPicture *picture)
{
    BVEE_PictureDescriptor descriptor;
    BERR_Code rc;

    if(g_encoder.watchdog) {
        return NEXUS_NOT_AVAILABLE;
    }

    BKNI_Memset(&descriptor, 0, sizeof(descriptor));

    descriptor.hImageMmaBlock = picture->hImage;
    descriptor.offset = picture->offset;
    descriptor.width = picture->width;
    descriptor.height = picture->height;
    descriptor.polarity = picture->polarity;
    descriptor.originalPts.ui32CurrentPTS = picture->origPts;
    descriptor.bIgnorePicture = picture->ignorePicture;
    descriptor.bStallStc = picture->stallStc;
    descriptor.sarHorizontal = picture->sarHorizontal;
    descriptor.sarVertical = picture->sarVertical;
#if 0
    descriptor.frameRate = encoder->veeStartSettings.frameRate;
#else
    NEXUS_P_FrameRate_ToMagnum_isrsafe(picture->framerate, &descriptor.frameRate); /* No drop ins nexus. DSP firmware will handle frame drops */
#endif

    NEXUS_StcChannel_GetStc_isr(encoder->startSettings.stcChannel, &descriptor.STC_Lo);

    BDBG_MSG(("Enqueue %p:%u", (void *)descriptor.hImageMmaBlock, descriptor.offset));

    rc = BVEE_Channel_EnqueuePicture_isr(encoder->veeChannel, &descriptor);

    return rc;
}

NEXUS_Error NEXUS_DspVideoEncoder_DequeuePicture_isr(NEXUS_DspVideoEncoderHandle encoder, NEXUS_DspVideoEncoderPicture *picture)
{
    BVEE_PictureDescriptor descriptor;
    BERR_Code rc;

    BKNI_Memset(picture, 0, sizeof(*picture));

    if(g_encoder.watchdog) {
    return NEXUS_NOT_AVAILABLE;
    }

    rc = BVEE_Channel_DequeuePicture_isr(encoder->veeChannel, &descriptor);
    if(rc!=BERR_SUCCESS) {return BERR_TRACE(rc);}

    BDBG_MSG(("Dequeue %p:%u", (void *)descriptor.hImageMmaBlock, descriptor.offset));

    picture->hImage = descriptor.hImageMmaBlock;
    picture->offset = descriptor.offset;

    return rc;
}
#endif /* NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT */

NEXUS_Error NEXUS_DspVideoEncoder_GetDelayRange_priv(NEXUS_DspVideoEncoderHandle encoder, uint32_t width, uint32_t height, NEXUS_VideoFrameRate framerate, uint32_t bitrate, uint32_t *delay)
{
    BERR_Code rc;
    BVEE_Resolution resolution;
    BAVC_FrameRateCode veeframerate;

    BSTD_UNUSED(encoder);

    resolution.width = width;
    resolution.height = height;

    rc = NEXUS_P_FrameRate_ToMagnum_isrsafe(framerate, &veeframerate);
    if(rc!=BERR_SUCCESS) {return BERR_TRACE(rc);}

    rc = BVEE_GetA2PDelay(delay, resolution, veeframerate, bitrate);
    if(rc!=BERR_SUCCESS) {return BERR_TRACE(rc);}

    return rc;
}

NEXUS_Error NEXUS_DspVideoEncoder_GetSettings_priv(NEXUS_DspVideoEncoderHandle encoder, NEXUS_DspVideoEncoderSettings *pSettings)
{
    NEXUS_Error rc;
    BVEE_ChannelSettings settings;

    BDBG_OBJECT_ASSERT(encoder, NEXUS_DspVideoEncoder);

    rc = BVEE_Channel_GetSettings(encoder->veeChannel, &settings);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc); return rc;}

    pSettings->bitrate = settings.ui32TargetBitRate;
    pSettings->frameRate = settings.frameRate;

    return rc;
}

NEXUS_Error NEXUS_DspVideoEncoder_SetSettings_priv(NEXUS_DspVideoEncoderHandle encoder, NEXUS_DspVideoEncoderSettings *pSettings)
{
    NEXUS_Error rc;
    BVEE_ChannelSettings settings;

    BDBG_OBJECT_ASSERT(encoder, NEXUS_DspVideoEncoder);

    settings.ui32TargetBitRate = pSettings->bitrate;
    settings.frameRate = pSettings->frameRate;

   rc = BVEE_Channel_SetSettings(encoder->veeChannel, &settings);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);}

    return rc;
}

NEXUS_Error NEXUS_DspVideoEncoder_P_InitWatchdog(void)
{
    BERR_Code errCode;
    BVEE_InterruptHandlers interrupts;

    g_encoder.watchdogCallback = NEXUS_IsrCallback_Create(&g_encoder, NULL);
    if ( g_encoder.watchdogCallback == NULL )
    {
        return BERR_TRACE(BERR_OS_ERROR);
    }

    /* Allow watchdog to be disabled for debugging */
    if ( !NEXUS_GetEnv("no_watchdog") )
    {
        BVEE_GetInterruptHandlers(g_NEXUS_veeHandle, &interrupts);
        interrupts.watchdog.pCallback_isr = NEXUS_DspVideoEncoder_P_Watchdog_isr;
        errCode = BVEE_SetInterruptHandlers(g_NEXUS_veeHandle, &interrupts);
        if ( errCode )
        {
            NEXUS_IsrCallback_Destroy(g_encoder.watchdogCallback);
            return BERR_TRACE(errCode);
        }
    }


    return BERR_SUCCESS;
}

void NEXUS_DspVideoEncoder_P_UninitWatchdog(void)
{
    BVEE_InterruptHandlers interrupts;

    BVEE_GetInterruptHandlers(g_NEXUS_veeHandle, &interrupts);
    interrupts.watchdog.pCallback_isr = NULL;
    BVEE_SetInterruptHandlers(g_NEXUS_veeHandle, &interrupts);
    NEXUS_IsrCallback_Destroy(g_encoder.watchdogCallback);
}

static void NEXUS_DspVideoEncoder_P_Watchdog_isr(void *pParam1, int param2)
{
    BSTD_UNUSED(pParam1);
    BSTD_UNUSED(param2);
    BDBG_ERR(("DSP Video Watchdog Interrupt Received"));

    g_encoder.watchdog = true;
    NEXUS_IsrCallback_Fire_isr(g_encoder.watchdogCallback);
}

void NEXUS_DspVideoEncoder_SetWatchdogCallback_priv(NEXUS_CallbackDesc *watchdog)
{
    BDBG_MSG(("NEXUS_DspVideoEncoder_SetWatchdogCallback_priv"));
    if(g_encoder.watchdogCallback)
    NEXUS_IsrCallback_Set(g_encoder.watchdogCallback, watchdog);
}

void NEXUS_DspVideoEncoder_Watchdog_priv(void)
{
    BDBG_WRN(("NEXUS_DspVideoEncoder_Watchdog_priv"));
    /* Process watchdog event */
    BVEE_ProcessWatchdogInterrupt(g_NEXUS_veeHandle);

    g_encoder.watchdog = false;
}
