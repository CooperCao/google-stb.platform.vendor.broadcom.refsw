/***************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 * Module Description:
 *
 **************************************************************************/
#ifndef NEXUS_VIDEO_DECODER_MODULE_H__
#define NEXUS_VIDEO_DECODER_MODULE_H__

#include "nexus_video_decoder_thunks.h"
#include "nexus_base.h"
#include "nexus_video_decoder_trick.h"
#include "nexus_video_decoder_primer.h"
#include "nexus_still_decoder.h"
#include "nexus_video_decoder.h"
#include "nexus_video_decoder_extra.h"
#include "nexus_video_decoder_private.h"
#include "nexus_mosaic_video_decoder.h" /* standard on 740x */
#include "priv/nexus_core_video.h"
#include "priv/nexus_video_decoder_priv.h"
#include "priv/nexus_rave_priv.h"
#include "priv/nexus_core_img.h"
#include "nexus_video_decoder_init.h"
#include "priv/nexus_video_decoder_standby_priv.h"
#include "nexus_platform_features.h"
#include "nexus_core_utils.h"
#include "bxvd.h"
#include "budp_dccparse.h"
#include "budp_jp3dparse.h"
#include "budp_dccparse_dss.h"
#include "priv/nexus_stc_channel_priv.h"
#include "bxdm_dih.h"
#include "bxdm_pp.h"
#include "bfifo.h"
#if NEXUS_OTFPVR
#include "botf.h"
#endif
#if NEXUS_VIDEO_DECODER_EXTENSION_PREPROCESS_PICTURE
#include "nexus_video_decoder_preprocess_picture.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef NEXUS_MODULE_SELF
#error Cant be in two modules at the same time
#endif

#define NEXUS_MODULE_NAME videodecoder
#define NEXUS_MODULE_SELF g_NEXUS_videoDecoderModule

#define NEXUS_NUM_XVD_CHANNELS 16

struct NEXUS_VideoDecoderDevice;

typedef struct NEXUS_VideoDecoder_P_Interface {
    void (*Close)( NEXUS_VideoDecoderHandle handle);
    void (*GetOpenSettings)( NEXUS_VideoDecoderHandle handle, NEXUS_VideoDecoderOpenSettings *pOpenSettings);
    void (*GetSettings)( NEXUS_VideoDecoderHandle handle, NEXUS_VideoDecoderSettings *pSettings );
    NEXUS_Error (*SetSettings)( NEXUS_VideoDecoderHandle handle, const NEXUS_VideoDecoderSettings *pSettings);
    NEXUS_Error (*Start)( NEXUS_VideoDecoderHandle handle, const NEXUS_VideoDecoderStartSettings *pSettings);
    void (*Stop)( NEXUS_VideoDecoderHandle handle);
    void (*Flush)( NEXUS_VideoDecoderHandle handle);
    NEXUS_Error (*GetStatus)( NEXUS_VideoDecoderHandle handle, NEXUS_VideoDecoderStatus *pStatus);
    NEXUS_VideoInput (*GetConnector)(NEXUS_VideoDecoderHandle handle);
    NEXUS_Error (*GetStreamInformation)(NEXUS_VideoDecoderHandle handle, NEXUS_VideoDecoderStreamInformation *pStreamInformation);
    NEXUS_Error (*SetStartPts)( NEXUS_VideoDecoderHandle handle, uint32_t pts);
    void (*IsCodecSupported)( NEXUS_VideoDecoderHandle videoDecoder, NEXUS_VideoCodec codec, bool *pSupported);
    NEXUS_Error (*SetPowerState)( NEXUS_VideoDecoderHandle handle, bool powerUp);
    void (*Reset)( NEXUS_VideoDecoderHandle handle);
    NEXUS_Error (*GetExtendedStatus)( NEXUS_VideoDecoderHandle handle, NEXUS_VideoDecoderExtendedStatus *pStatus);
    void (*GetExtendedSettings)( NEXUS_VideoDecoderHandle handle, NEXUS_VideoDecoderExtendedSettings *pSettings);
    NEXUS_Error (*SetExtendedSettings)( NEXUS_VideoDecoderHandle handle, const NEXUS_VideoDecoderExtendedSettings *pSettings);
    NEXUS_StripedSurfaceHandle (*CreateStripedSurface)( NEXUS_VideoDecoderHandle handle);
    void (*DestroyStripedSurface)( NEXUS_VideoDecoderHandle videoDecoder, NEXUS_StripedSurfaceHandle stripedSurface);
    NEXUS_Error (*CreateStripedMosaicSurfaces)( NEXUS_VideoDecoderHandle videoDecoder, NEXUS_StripedSurfaceHandle *pStripedSurfaces, unsigned int maxSurfaces, unsigned int *pSurfaceCount) ;
    NEXUS_Error (*GetMostRecentPts)( NEXUS_VideoDecoderHandle handle, uint32_t *pPts);
    void (*GetTrickState)( NEXUS_VideoDecoderHandle handle, NEXUS_VideoDecoderTrickState *pState);
    NEXUS_Error (*SetTrickState)( NEXUS_VideoDecoderHandle handle, const NEXUS_VideoDecoderTrickState *pState);
    NEXUS_Error (*FrameAdvance)(NEXUS_VideoDecoderHandle videoDecoder);
    NEXUS_Error (*GetNextPts)( NEXUS_VideoDecoderHandle handle, uint32_t *pNextPts);
    void (*GetPlaybackSettings)( NEXUS_VideoDecoderHandle handle, NEXUS_VideoDecoderPlaybackSettings *pSettings);
    NEXUS_Error (*SetPlaybackSettings)( NEXUS_VideoDecoderHandle handle, const NEXUS_VideoDecoderPlaybackSettings *pSettings);
    NEXUS_StillDecoderHandle (*StillDecoder_Open)( NEXUS_VideoDecoderHandle parentDecoder, unsigned index, const NEXUS_StillDecoderOpenSettings *pSettings );
#if NEXUS_HAS_ASTM
    void (*GetAstmSettings_priv)( NEXUS_VideoDecoderHandle videoDecoder, NEXUS_VideoDecoderAstmSettings * pAstmSettings);
    NEXUS_Error (*SetAstmSettings_priv)(NEXUS_VideoDecoderHandle videoDecoder, const NEXUS_VideoDecoderAstmSettings * pAstmSettings);
    NEXUS_Error (*GetAstmStatus_isr)( NEXUS_VideoDecoderHandle videoDecoder, NEXUS_VideoDecoderAstmStatus * pAstmStatus);
#endif /* NEXUS_HAS_ASTM */

    void (*GetDisplayConnection_priv)( NEXUS_VideoDecoderHandle handle, NEXUS_VideoDecoderDisplayConnection *pConnection);
    NEXUS_Error (*SetDisplayConnection_priv)( NEXUS_VideoDecoderHandle handle, const NEXUS_VideoDecoderDisplayConnection *pConnection);
    void (*GetSourceId_priv)( NEXUS_VideoDecoderHandle handle, BAVC_SourceId *pSource);
    void (*GetHeap_priv)( NEXUS_VideoDecoderHandle handle, NEXUS_HeapHandle *pHeap);

#if NEXUS_HAS_SYNC_CHANNEL
    void (*GetSyncSettings_priv)( NEXUS_VideoDecoderHandle videoDecoder, NEXUS_VideoInputSyncSettings *pSyncSettings);
    NEXUS_Error (*SetSyncSettings_priv)(NEXUS_VideoDecoderHandle videoDecoder, const NEXUS_VideoInputSyncSettings *pSyncSettings);
    NEXUS_Error (*GetSyncStatus_isr)(NEXUS_VideoDecoderHandle videoDecoder, NEXUS_VideoInputSyncStatus *pSyncStatus );
#endif

    void (*UpdateDisplayInformation_priv)( NEXUS_VideoDecoderHandle videoDecoder, const NEXUS_VideoDecoder_DisplayInformation *displayInformation);

    NEXUS_Error (*GetDecodedFrames)(NEXUS_VideoDecoderHandle handle, NEXUS_VideoDecoderFrameStatus *pStatus, unsigned numEntries, unsigned *pNumEntriesReturned);
    NEXUS_Error (*ReturnDecodedFrames)(NEXUS_VideoDecoderHandle handle, const NEXUS_VideoDecoderReturnFrameSettings *pSettings, unsigned numFrames);
} NEXUS_VideoDecoder_P_Interface ;

#ifdef __cplusplus
}
#endif

/* NEXUS_VideoDecoder_P_Interface  should be already defined */

#if NEXUS_NUM_DSP_VIDEO_DECODERS
#include "nexus_video_decoder_module_dsp.h"
#endif
#if NEXUS_NUM_ZSP_VIDEO_DECODERS
#include "nexus_video_decoder_module_zsp.h"
#endif

#if NEXUS_NUM_SOFT_VIDEO_DECODERS
#include "nexus_video_decoder_module_soft.h"
#endif

#if NEXUS_HAS_PICTURE_DECODER
#include "nexus_video_decoder_module_sid.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern const NEXUS_VideoDecoder_P_Interface NEXUS_VideoDecoder_P_Interface_Avd;

BDBG_OBJECT_ID_DECLARE(NEXUS_VideoDecoder_P_Xdm);

typedef struct NEXUS_VideoDecoder_P_Xdm {
    BDBG_OBJECT(NEXUS_VideoDecoder_P_Xdm)
    BXDM_PictureProvider_Handle pictureProvider;
} NEXUS_VideoDecoder_P_Xdm;

#define NEXUS_P_MAX_ELEMENTS_IN_VIDEO_DECODER_PIC_QUEUE 64

typedef struct NEXUS_VideoDecoderPictureContext
{

   unsigned serialNumber;
   BXDM_Picture *pUnifiedPicture;
} NEXUS_VideoDecoderPictureContext;

typedef struct NEXUS_VideoDisplayPictureContext
{

   unsigned serialNumber;
   bool dropPicture;

   BXDM_Picture *pUnifiedPicture;

} NEXUS_VideoDisplayPictureContext;


typedef struct NEXUS_VideoDecoderPictureQueue
{
   BFIFO_HEAD(DecodeFifo, NEXUS_VideoDecoderPictureContext) decodeFifo;

   uint32_t pictureCounter;

   NEXUS_VideoDecoderPictureContext pictureContexts[NEXUS_P_MAX_ELEMENTS_IN_VIDEO_DECODER_PIC_QUEUE];

} NEXUS_VideoDecoderPictureQueue;

typedef struct NEXUS_VideoDisplayPictureQueue
{
    BFIFO_HEAD(DisplayFifo, NEXUS_VideoDisplayPictureContext) displayFifo;

    uint32_t pictureCounter;

    NEXUS_VideoDisplayPictureContext pictureContexts[NEXUS_P_MAX_ELEMENTS_IN_VIDEO_DECODER_PIC_QUEUE];

} NEXUS_VideoDisplayPictureQueue;

/* DM Lite Data Structures */
typedef struct
{
	BXDM_Picture *pDispPicture;
	bool first_pic;
    bool repeat_field;
	bool valid;
} DML_DispPicStruct;

typedef struct NEXUS_VideoDecoder_Xvd {
    BMMA_Block_Handle pictureMemory;
    BMMA_Block_Handle secondaryPictureMemory;
    BMMA_Block_Handle cabacMemory;
} NEXUS_VideoDecoder_Xvd;

typedef struct NEXUS_VideoDecoderExternalTsmData
{
    NEXUS_VideoDecoderPictureQueue decoderPictureQueue;
    NEXUS_VideoDisplayPictureQueue displayPictureQueue;

    BXDM_Decoder_Interface decoderInterface;
    void* pDecoderPrivateContext;

    DML_DispPicStruct displayPic;
    BAVC_MFD_Picture *pMFDPicture;
    BAVC_MFD_Picture MFDPicture;
	bool stopped;

    unsigned numDecoded;      /* total number of decoded pictures since Start */
    unsigned numDisplayed;    /* total number of display pictures since Start */
    unsigned numIFramesDisplayed; /* total number of displayed I-Frames pictures since Start */
} NEXUS_VideoDecoderExternalTsmData;

/* This structure provides the implementation of the NEXUS_VideoDecoderHandle.
It corresponds to one XVD channel. */
struct NEXUS_VideoDecoder {
    NEXUS_OBJECT(NEXUS_VideoDecoder);
    const NEXUS_VideoDecoder_P_Interface *intf;
    struct NEXUS_VideoDecoderDevice *device;
    unsigned channelIndex; /* index to device->channel[], send to XVD */
    unsigned mosaicIndex; /* mosaic index, packed to avoid extra MFD bandwidth. set at OpenChannel time. */
    unsigned index; /* if not mosaic, index passed to NEXUS_VideoDecoder_Open. if mosaic, the mosaic subindex (not parentIndex). */
    unsigned parentIndex; /* if not mosaic, same as index. if mosaic, the parent. This is the memconfig VideoDecoder index. */
    BXVD_ChannelHandle dec;
    BKNI_EventHandle source_changed_event;
    NEXUS_RaveHandle rave;
    bool raveDetached;
    bool enhancementRaveDetached;
    NEXUS_RaveHandle savedRave; /* the primer interface may replace the main RAVE context with a primed context. */
    NEXUS_VideoDecoderPrimerHandle primer;
    NEXUS_RaveHandle enhancementRave; /* rave context for  enhancementPidChannel */
    NEXUS_VideoDecoderStartSettings startSettings;
    NEXUS_VideoDecoderExtendedSettings extendedSettings;
    bool skipFlushOnStop;

    struct {
        BXVD_Userdata_Handle handle;
        BMMA_Block_Handle mem;
        unsigned char *buf; /* size is settings.userDataBufferSize */
        unsigned rptr, wptr, wrap_ptr;
        NEXUS_VideoDecoderUserDataStatus status;
        NEXUS_DigitalVbiDataCallback vbiDataCallback_isr;
        NEXUS_VideoInput vbiVideoInput;
        unsigned lastGetBufferSize;
    } userdata;

    BXVD_StillPictureBuffers still_picture_buffer;
    BKNI_EventHandle still_picture_event;
    BAVC_MFD_Picture last_field; /* current picture going to display */
    unsigned last_getStripedSurfaceSerialNumber;
    bool last_field_flag;
    NEXUS_VideoDecoderStreamInformation streamInfo; /* newest picture coming from decoder when starting */
    NEXUS_VideoDecoderStreamInformation lastStreamInfo; /* last picture coming from decoder. stored after stop decode. */
    struct {
        BXVD_PictureCoding pictureCoding;
    } pictureParameterInfo; /* TODO: consider storing whole BXVD_PictureParameterInfo at isr time, then building NEXUS_VideoDecoderStreamInformation at get time */
    NEXUS_TransportType transportType;
    NEXUS_VideoDecoderOpenMosaicSettings openSettings; /* superset */
    NEXUS_VideoDecoderMosaicSettings mosaicSettings;
    NEXUS_VideoDecoderSettings settings;
    NEXUS_IsrCallbackHandle userdataCallback;
    NEXUS_IsrCallbackHandle afdChangedCallback;
    NEXUS_IsrCallbackHandle streamChangedCallback;
    NEXUS_IsrCallbackHandle sourceChangedCallback;
    NEXUS_IsrCallbackHandle ptsErrorCallback;
    NEXUS_IsrCallbackHandle firstPtsCallback;
    NEXUS_IsrCallbackHandle firstPtsPassedCallback;
    NEXUS_IsrCallbackHandle decodeErrorCallback;
    NEXUS_IsrCallbackHandle fnrtChunkDoneCallback;
    struct {
        BXVD_DQTStatus status;
        bool set;
    } dqt;
    struct {
        NEXUS_IsrCallbackHandle firstPtsCallback;
        NEXUS_IsrCallbackHandle firstPtsPassedCallback;
        NEXUS_VideoDecoderPlaybackSettings videoDecoderPlaybackSettings;
    } playback;
    unsigned dataReadyCount;
    unsigned numPicturesReceivedToFlush;
    NEXUS_IsrCallbackHandle dataReadyCallback;
    NEXUS_VideoDecoderDisplayConnection displayConnection;
    bool videoAsGraphics;
    NEXUS_IsrCallbackHandle s3DTVChangedCallback;
    NEXUS_VideoDecoder3DTVStatus s3DTVStatus;
    BKNI_EventHandle s3DTVStatusEvent;
    NEXUS_EventCallbackHandle s3DTVStatusEventHandler;
    NEXUS_TimerHandle s3DTVStatusTimer;
    uint32_t s3DTVStatusPts;
    struct NEXUS_VideoDecoderDevice *linkedDevice;

    BUDP_DCCparse_Format currentUserDataFormat;
    NEXUS_UserDataFormat userDataFormat;
    bool useUserDataFormat;
    unsigned userdataAnyFilterCnt;
    struct {
#define B_MAX_VBI_CC_COUNT 32 /* required by UDPlib */
        BUDP_DCCparse_ccdata ccData[B_MAX_VBI_CC_COUNT];
        BUDP_DCCparse_dss_cc_subtitle dssCcData[B_MAX_VBI_CC_COUNT];
        BUDP_DCCparse_dss_cc_subtitle dssSubtitle[B_MAX_VBI_CC_COUNT];
    } udpData;

    uint32_t primerPtsOffset;
#if NEXUS_HAS_SYNC_CHANNEL
    struct
    {
        NEXUS_VideoInputSyncSettings settings;
        NEXUS_VideoInputSyncStatus status;
        bool startMuted;
        bool mute;
        unsigned delay;
        unsigned startDelay;
    } sync;
#endif

#if NEXUS_HAS_ASTM
    struct
    {
        bool permitted;
        NEXUS_VideoDecoderAstmSettings settings;
        NEXUS_VideoDecoderAstmStatus status;
    } astm;
#endif

    struct
    {
        unsigned sampleCount;
        uint32_t lastControlTime;
        uint32_t lastSampleTime;
        unsigned sumQueueDepth;
        int ptsOffset;
    } lowLatency;

    unsigned pts_error_cnt;
    bool started;
    bool tsm;
    int32_t ptsStcDifference;
    unsigned defaultDiscardThreshold; /* store the default from XVD after Start */
    unsigned defaultVeryLateThreshold;
    bool firstPtsReady;
    bool firstPtsPassed;

    unsigned pictureDeliveryCount;

    struct
    {
        unsigned count;  /* we believe the fifo is empty. this requires extra logic because internal buffers might cause false detect */
        uint64_t lastCdbValidPointer;
        uint64_t lastCdbReadPointer;
        NEXUS_IsrCallbackHandle callback;
        unsigned emptyCount;
        unsigned noLongerEmptyCount;
    } fifoEmpty;

    NEXUS_IsrCallbackHandle stateChangedCallback;

    struct
    {
        unsigned staticCount; /* fifo has not moved */
        unsigned lastPictureDeliveryCount;
        uint64_t lastCdbValidPointer;
        uint64_t lastCdbReadPointer;
        uint32_t lastPts;
        uint32_t lastPtsValid;
        NEXUS_TimerHandle timer;
        NEXUS_StcChannelDecoderFifoWatchdogStatus status;
        bool isFull;
    } fifoWatchdog;

    struct
    {
        unsigned cnt; /* check status every 2 seconds */
        unsigned avdStatusBlock; /* last block. only print on changes. */
        /* piggy back of fifoWatchdog.timer for now */
    } status;

    unsigned overflowCount;

    struct {
        bool wasActive; /* set to true if OTF PVR was activated between start and stop */
#if NEXUS_OTFPVR
        bool active; /* set to true if OTF PVR currently active */
        bool eos;
        NEXUS_TimerHandle timer;
        NEXUS_RaveHandle rave; /* rave context that is used by the OTFPVR to feed ITB entires into the decoder */
        BAVC_XptContextMap xptContextMap; /* we need to keep this since XVD expects pointer to BAVC_XptContextMap, and we can't use automatic variable */
        BOTF_Handle otf;
        BOTF_Status status;
#endif
    } otfPvr; /* state specific for the OTF PVR operations */

    NEXUS_VideoDecoderTrickState trickState;
    struct {
        unsigned pictureId;
        unsigned pictureDisplayCount;
    } maxFrameRepeat; /* state if trickState.maxFrameRepeat != 0 */

    unsigned xdmIndex; /* XDM index. Each XDM instance is mapped with a decoder object and an MFD interrupt. */
    unsigned mfdIndex; /* MFD index. This is the apparent "main index" to users of VideoDecoder (e.g. Display).
                           For 7405, AVD0 services MFD0 and MFD1, so mainIndex can be 0 or 1.
                           For 7420, AVD0 services MFD0 and AVD1 services MFD1, so mainIndex can be 0 or 1.
                           For 3548, AVD0 services MFD0, so mainIndex can be 0. */
    bool isInterruptChannel; /* For mosaic mode, only the first channel on a device gives interrupts. */
    NEXUS_VideoDecoder_DisplayInformation displayInformation;
    NEXUS_VideoInputObject input;
    unsigned additionalPtsOffset; /* stores additional pts offset that is applied for certain codecs (MPEG2 needs additional 120msec of delay */
    bool errorHandlingOverride; /* if true should use the errorHandlingMode instead of the user supplied value */
    NEXUS_VideoDecoderErrorHandling errorHandlingMode;

    bool mosaicMode;
    bool mosaicParent; /* set true if this is the mosaic parent */

    BKNI_EventHandle channelChangeReportEvent;
    NEXUS_EventCallbackHandle channelChangeReportEventHandler;
    bool validOutputPic;
    unsigned cdbLength; /* actual size of CDB buffer */
    unsigned itbLength; /* actual size of ITB buffer */
    NEXUS_VideoDecoder_P_Xdm xdm;
    union {
#if NEXUS_NUM_DSP_VIDEO_DECODERS
        NEXUS_VideoDecoder_Dsp dsp;
#endif
#if NEXUS_NUM_ZSP_VIDEO_DECODERS
        NEXUS_VideoDecoder_Zsp zsp;
#endif
#if NEXUS_NUM_SOFT_VIDEO_DECODERS
        NEXUS_VideoDecoder_Soft soft;
#endif
#if NEXUS_HAS_PICTURE_DECODER
        NEXUS_VideoDecoder_Sid sid;
#endif
        NEXUS_VideoDecoder_Xvd xvd;
    } decoder;

    struct {
        NEXUS_VideoDecoderCrc *data;
        unsigned size, wptr, rptr;
    } crc;

    struct
    {
        NEXUS_StcChannelDecoderConnectionHandle connector;
        unsigned priority;
        BKNI_EventHandle statusChangeEvent;
        NEXUS_EventCallbackHandle statusChangeEventHandler;
    } stc;

    NEXUS_VideoDecoderExternalTsmData externalTsm;
    struct {
        unsigned maxWidth, maxHeight, refreshRate; /* derived from memconfig's maxFormat, cached here */
    } memconfig;
    struct {
        NEXUS_MemoryBlockHandle  block;
        BMMA_Block_Handle mma;
    } memoryBlockHeap[16];
    struct {
        NEXUS_VideoDecoderPrivateSettings settings;
        NEXUS_IsrCallbackHandle streamChangedCallback;
    } private;
};

/* The NEXUS_VideoDecoderDevice corresponds to an AVD HW block. */
struct NEXUS_VideoDecoderDevice {
    BXVD_Handle xvd;
    BXVD_HardwareCapabilities cap;
    unsigned index;
    NEXUS_VideoDecoderDisplayConnection defaultConnection;
    BKNI_EventHandle watchdog_event;
    NEXUS_EventCallbackHandle watchdogEventHandler;
    unsigned numWatchdogs;
    struct NEXUS_VideoDecoder *channel[NEXUS_NUM_XVD_CHANNELS];
    BMMA_Heap_Handle mem; /* heap used by AVD */
    struct NEXUS_VideoDecoderDevice *slaveLinkedDevice; /* crosslink to device which has set device->linkedDevice to this one */

    /* image interface */
    void * img_context;
    BIMG_Interface img_interface;

    unsigned mosaicCount;
    BXDM_DisplayInterruptHandler_Handle hXdmDih[BXVD_DisplayInterrupt_eMax];
    bool xdmInUse[BXVD_DisplayInterrupt_eMax];
};

/* global instance data */
extern NEXUS_ModuleHandle g_NEXUS_videoDecoderModule;
extern NEXUS_VideoDecoderModuleSettings g_NEXUS_videoDecoderModuleSettings;
extern NEXUS_VideoDecoderModuleInternalSettings g_NEXUS_videoDecoderModuleInternalSettings;
extern struct NEXUS_VideoDecoderDevice g_NEXUS_videoDecoderXvdDevices[NEXUS_MAX_XVD_DEVICES];
extern NEXUS_VideoDecoderCapabilities g_NEXUS_videoDecoderCapabilities;

#define LOCK_TRANSPORT()    NEXUS_Module_Lock(g_NEXUS_videoDecoderModuleInternalSettings.transport)
#define UNLOCK_TRANSPORT()  NEXUS_Module_Unlock(g_NEXUS_videoDecoderModuleInternalSettings.transport)

/**
Priv functions
**/
NEXUS_Error NEXUS_VideoDecoder_P_OpenChannel(NEXUS_VideoDecoderHandle videoDecoder);
void NEXUS_VideoDecoder_P_CloseChannel(NEXUS_VideoDecoderHandle videoDecoder);
void NEXUS_VideoDecoder_P_WatchdogHandler(void *data);
void NEXUS_VideoDecoder_P_Watchdog_isr(void *data, int not_used, void *not_used2);
void NEXUS_VideoDecoder_P_DataReady_isr(void *data, int chIndex, void *field);
void NEXUS_VideoDecoder_P_RequestStc_isr(void *data, int chIndex, void *pts_info);
void NEXUS_VideoDecoder_P_PtsError_isr(void *data, int chIndex, void *pts_info);
void NEXUS_VideoDecoder_P_TsmPass_isr(void *data, int chIndex, void *pts_info);
void NEXUS_VideoDecoder_P_PictureParams_isr(void *data, int chIndex, void *info_);
void NEXUS_VideoDecoder_P_FirstPtsReady_isr(void *data, int chIndex, void *pts_info);
void NEXUS_VideoDecoder_P_FirstPtsPassed_isr(void *data, int chIndex, void *pts_info);
void NEXUS_VideoDecoder_P_UserdataReady_isr(void *data, int chInt, void *not_used);
void NEXUS_VideoDecoder_P_PtsStcOffset_isr(void *data, int unused, void *pOffset);
void NEXUS_VideoDecoder_P_DecodeError_isr(void *data, int unused, void *unused2);
void NEXUS_VideoDecoder_P_3DTVTimer(void* context);
void NEXUS_VideoDecoder_P_Jp3dSignal_isr(NEXUS_VideoDecoderHandle videoDecoder, uint16_t type);
void NEXUS_VideoDecoder_P_PictureExtensionData_isr(void *data, int unused, void *pData);
void NEXUS_VideoDecoder_P_FnrtChunkDone_isr(void *data, int unused, void *unused2);
void NEXUS_VideoDecoder_P_EndOfGOP_isr(void *data, int unused, void *param);
BERR_Code NEXUS_VideoDecoder_P_GetCdbLevelCallback_isr(void *pContext, unsigned *pCdbLevel);
BERR_Code NEXUS_VideoDecoder_P_GetPtsCallback_isr(void *pContext, BAVC_PTSInfo *pPTSInfo);
BERR_Code NEXUS_VideoDecoder_P_StcValidCallback_isr(void *pContext);
BERR_Code NEXUS_VideoDecoder_P_SetPcrOffset_isr(void *pContext, uint32_t pcrOffset);
BERR_Code NEXUS_VideoDecoder_P_GetPcrOffset_isr(void *pContext, uint32_t *pPcrOffset);

NEXUS_Error NEXUS_VideoDecoder_P_SetCrcFifoSize(NEXUS_VideoDecoderHandle videoDecoder, bool forceDisable);
NEXUS_Error NEXUS_VideoDecoder_P_SetPtsOffset(NEXUS_VideoDecoderHandle videoDecoder);
NEXUS_Error NEXUS_VideoDecoder_P_SetPtsOffset_isr(NEXUS_VideoDecoderHandle videoDecoder);
NEXUS_Error NEXUS_VideoDecoder_P_SetMute(NEXUS_VideoDecoderHandle videoDecoder);
NEXUS_Error NEXUS_VideoDecoder_P_SetFreeze(NEXUS_VideoDecoderHandle videoDecoder);
NEXUS_Error NEXUS_VideoDecoder_P_SetUserdata(NEXUS_VideoDecoderHandle videoDecoder);
NEXUS_Error NEXUS_VideoDecoder_P_SetTsm(NEXUS_VideoDecoderHandle videoDecoder);
void NEXUS_VideoDecoder_P_ChannelChangeReport(void *context);
/* SetChannelChangeMode, priv Start and Stop  are used to implement controllable video decoder Flush*/
void NEXUS_VideoDecoder_P_Stop_priv(NEXUS_VideoDecoderHandle videoDecoder);
NEXUS_Error NEXUS_VideoDecoder_P_Start_priv(NEXUS_VideoDecoderHandle videoDecoder, const NEXUS_VideoDecoderStartSettings *pStartSettings, bool otfPvr);
NEXUS_Error NEXUS_VideoDecoder_P_SetChannelChangeMode(NEXUS_VideoDecoderHandle videoDecoder, NEXUS_VideoDecoder_ChannelChangeMode mode);
void NEXUS_VideoDecoder_P_GetRaveSettings(unsigned avdIndex, NEXUS_RaveOpenSettings *pRaveSettings, const NEXUS_VideoDecoderOpenMosaicSettings *pOpenSettings);
void NEXUS_VideoDecoder_P_ApplyDisplayInformation( NEXUS_VideoDecoderHandle videoDecoder);
void NEXUS_VideoDecoder_P_ApplyDisplayInformation_Common( NEXUS_VideoDecoderHandle videoDecoder);
NEXUS_Error NEXUS_VideoDecoder_P_getUnusedMfd(bool dsp, BAVC_SourceId *pId, unsigned *pHeapIndex);

#if NEXUS_OTFPVR
void NEXUS_VideoDecoder_P_OtfPvr_Stop(NEXUS_VideoDecoderHandle videoDecoder);
void NEXUS_VideoDecoder_P_OtfPvr_DisableForFlush(NEXUS_VideoDecoderHandle videoDecoder);
void NEXUS_VideoDecoder_P_OtfPvr_Flush(NEXUS_VideoDecoderHandle videoDecoder);
NEXUS_Error NEXUS_VideoDecoder_P_OtfPvr_Activate(NEXUS_VideoDecoderHandle videoDecoder, const NEXUS_RaveStatus *raveStatus, BXVD_DecodeSettings *xvdCfg);
NEXUS_Error NEXUS_VideoDecoder_P_OtfPvr_SetTrickState(NEXUS_VideoDecoderHandle videoDecoder, const NEXUS_VideoDecoderTrickState *pState, bool *complete);
void NEXUS_VideoDecoder_P_OtfPvr_UpdateStatus(NEXUS_VideoDecoderHandle videoDecoder, NEXUS_VideoDecoderStatus *pStatus);
#endif


/**
Common
**/
NEXUS_VideoFormat NEXUS_VideoDecoder_P_DeriveFormat(unsigned height, unsigned frameRate, bool interlaced);

void NEXUS_VideoDecoder_P_Trick_Reset_Generic(NEXUS_VideoDecoderHandle videoDecoder);


NEXUS_VideoDecoderHandle NEXUS_VideoDecoder_P_Open_Avd( unsigned index, const NEXUS_VideoDecoderOpenSettings *pOpenSettings);
void NEXUS_VideoDecoder_P_Close_Avd( NEXUS_VideoDecoderHandle handle);
NEXUS_Error NEXUS_VideoDecoder_P_SetSettings_Avd( NEXUS_VideoDecoderHandle handle, const NEXUS_VideoDecoderSettings *pSettings);
NEXUS_Error NEXUS_VideoDecoder_P_Start_Avd( NEXUS_VideoDecoderHandle handle, const NEXUS_VideoDecoderStartSettings *pSettings);
void NEXUS_VideoDecoder_P_Stop_Avd( NEXUS_VideoDecoderHandle handle);
void NEXUS_VideoDecoder_P_Flush_Avd( NEXUS_VideoDecoderHandle handle);
void NEXUS_VideoDecoder_P_FlushFifoEmpty(NEXUS_VideoDecoderHandle videoDecoder);
NEXUS_Error NEXUS_VideoDecoder_P_GetStatus_Avd( NEXUS_VideoDecoderHandle handle, NEXUS_VideoDecoderStatus *pStatus);
NEXUS_Error NEXUS_VideoDecoder_P_GetStreamInformation_Avd(NEXUS_VideoDecoderHandle handle, NEXUS_VideoDecoderStreamInformation *pStreamInformation);
NEXUS_Error NEXUS_VideoDecoder_P_SetStartPts_Avd( NEXUS_VideoDecoderHandle handle, uint32_t pts);
NEXUS_Error NEXUS_VideoDecoder_P_SetPowerState_Avd( NEXUS_VideoDecoderHandle handle, bool powerUp);
NEXUS_VideoDecoderHandle NEXUS_VideoDecoder_P_OpenMosaic_Avd( unsigned parentIndex, unsigned index, const NEXUS_VideoDecoderOpenMosaicSettings *pOpenSettings );
void NEXUS_VideoDecoder_P_Reset_Avd( NEXUS_VideoDecoderHandle handle);
NEXUS_Error NEXUS_VideoDecoder_P_GetExtendedStatus_Avd( NEXUS_VideoDecoderHandle handle, NEXUS_VideoDecoderExtendedStatus *pStatus);
void NEXUS_VideoDecoder_P_GetExtendedSettings_Avd( NEXUS_VideoDecoderHandle handle, NEXUS_VideoDecoderExtendedSettings *pSettings);
NEXUS_Error NEXUS_VideoDecoder_P_SetExtendedSettings_Avd( NEXUS_VideoDecoderHandle handle, const NEXUS_VideoDecoderExtendedSettings *pSettings);
NEXUS_Error NEXUS_VideoDecoder_P_GetStripedSurfaceCreateSettings( NEXUS_VideoDecoderHandle videoDecoder, const BAVC_MFD_Picture *pPicture, NEXUS_StripedSurfaceCreateSettings *pCreateSettings );
NEXUS_StripedSurfaceHandle NEXUS_VideoDecoder_P_CreateStripedSurface_Avd( NEXUS_VideoDecoderHandle handle);
void NEXUS_VideoDecoder_P_DestroyStripedSurface_Avd( NEXUS_VideoDecoderHandle videoDecoder, NEXUS_StripedSurfaceHandle stripedSurface);
NEXUS_Error NEXUS_VideoDecoder_P_CreateStripedMosaicSurfaces_Avd( NEXUS_VideoDecoderHandle videoDecoder, NEXUS_StripedSurfaceHandle *pStripedSurfaces, unsigned int maxSurfaces, unsigned int *pSurfaceCount) ;
NEXUS_Error NEXUS_VideoDecoder_P_GetMostRecentPts_Avd( NEXUS_VideoDecoderHandle handle, uint32_t *pPts);

NEXUS_Error NEXUS_VideoDecoder_P_SetTrickState_Avd( NEXUS_VideoDecoderHandle handle, const NEXUS_VideoDecoderTrickState *pState);
NEXUS_Error NEXUS_VideoDecoder_P_GetNextPts_Avd( NEXUS_VideoDecoderHandle handle, uint32_t *pNextPts);
NEXUS_Error NEXUS_VideoDecoder_P_FrameAdvance_Avd(NEXUS_VideoDecoderHandle videoDecoder);
void NEXUS_VideoDecoder_P_GetPlaybackSettings_Common( NEXUS_VideoDecoderHandle handle, NEXUS_VideoDecoderPlaybackSettings *pSettings);
NEXUS_Error NEXUS_VideoDecoder_P_SetPlaybackSettings_Common( NEXUS_VideoDecoderHandle handle, const NEXUS_VideoDecoderPlaybackSettings *pSettings);

NEXUS_ModuleHandle NEXUS_VideoDecoderModule_P_Init_Avd(const NEXUS_VideoDecoderModuleInternalSettings *pModuleSettings, const NEXUS_VideoDecoderModuleSettings *pSettings);
void NEXUS_VideoDecoderModule_P_Uninit_Avd(void);

void NEXUS_VideoDecoder_GetDisplayConnection_priv_Avd( NEXUS_VideoDecoderHandle handle, NEXUS_VideoDecoderDisplayConnection *pConnection);
NEXUS_Error NEXUS_VideoDecoder_SetDisplayConnection_priv_Avd( NEXUS_VideoDecoderHandle handle, const NEXUS_VideoDecoderDisplayConnection *pConnection);
void NEXUS_VideoDecoder_GetSourceId_priv_Avd( NEXUS_VideoDecoderHandle handle, BAVC_SourceId *pSource);
void NEXUS_VideoDecoder_GetHeap_priv_Common( NEXUS_VideoDecoderHandle handle, NEXUS_HeapHandle *pHeap);
#if NEXUS_HAS_SYNC_CHANNEL
void NEXUS_VideoDecoder_GetSyncSettings_priv_Common( NEXUS_VideoDecoderHandle videoDecoder, NEXUS_VideoInputSyncSettings *pSyncSettings);
NEXUS_Error NEXUS_VideoDecoder_SetSyncSettings_priv_Avd(NEXUS_VideoDecoderHandle videoDecoder, const NEXUS_VideoInputSyncSettings *pSyncSettings);
NEXUS_Error NEXUS_VideoDecoder_GetSyncStatus_Common_isr(NEXUS_VideoDecoderHandle videoDecoder, NEXUS_VideoInputSyncStatus *pSyncStatus );
void NEXUS_VideoDecoder_P_SetCustomSyncPtsOffset(NEXUS_VideoDecoderHandle videoDecoder);
#endif
void NEXUS_VideoDecoder_UpdateDisplayInformation_priv_Avd( NEXUS_VideoDecoderHandle videoDecoder, const NEXUS_VideoDecoder_DisplayInformation *displayInformation);
void NEXUS_VideoDecoder_UpdateDisplayInformation_priv_Common( NEXUS_VideoDecoderHandle videoDecoder, const NEXUS_VideoDecoder_DisplayInformation *pDisplayInformation);
#if NEXUS_HAS_ASTM
NEXUS_Error NEXUS_VideoDecoder_SetAstmSettings_priv_Xdm(NEXUS_VideoDecoderHandle videoDecoder, const NEXUS_VideoDecoderAstmSettings *pAstmSettings);
void NEXUS_VideoDecoder_GetAstmSettings_priv_Common( NEXUS_VideoDecoderHandle videoDecoder, NEXUS_VideoDecoderAstmSettings * pAstmSettings);
NEXUS_Error NEXUS_VideoDecoder_SetAstmSettings_priv_Avd(NEXUS_VideoDecoderHandle videoDecoder, const NEXUS_VideoDecoderAstmSettings * pAstmSettings);
NEXUS_Error NEXUS_VideoDecoder_GetAstmStatus_Common_isr( NEXUS_VideoDecoderHandle videoDecoder, NEXUS_VideoDecoderAstmStatus * pAstmStatus);
#endif /* NEXUS_HAS_ASTM */

NEXUS_Error NEXUS_VideoDecoder_GetDecodedFrames_Avd(
    NEXUS_VideoDecoderHandle handle,
    NEXUS_VideoDecoderFrameStatus *pStatus,  /* attr{nelem=numEntries;nelem_out=pNumEntriesReturned;null_allowed=y} [out] */
    unsigned numEntries,
    unsigned *pNumEntriesReturned /* [out] */
    );

NEXUS_Error NEXUS_VideoDecoder_ReturnDecodedFrames_Avd(
    NEXUS_VideoDecoderHandle handle,
    const NEXUS_VideoDecoderReturnFrameSettings *pSettings, /* attr{null_allowed=y, nelem=numFrames} Settings for each returned frame.  Pass NULL for defaults. */
    unsigned numFrames
    );

#if VIDEO_PRIMER_STATS
void NEXUS_VideoDecoder_DataReady_P_PrimerHook_isr_Avd(NEXUS_VideoDecoderHandle videoDecoder, BAVC_MFD_Picture *pFieldData);
void NEXUS_VideoDecoder_Stop_P_PrimerHook_Avd(NEXUS_VideoDecoderHandle videoDecoder);
#endif


void NEXUS_VideoDecoder_P_GetOpenSettings_Common( NEXUS_VideoDecoderHandle videoDecoder, NEXUS_VideoDecoderOpenSettings *pOpenSettings);
void NEXUS_VideoDecoder_P_GetSettings_Common(NEXUS_VideoDecoderHandle videoDecoder, NEXUS_VideoDecoderSettings *pSettings);
void NEXUS_VideoDecoder_P_GetTrickState_Common( NEXUS_VideoDecoderHandle handle, NEXUS_VideoDecoderTrickState *pState);
NEXUS_VideoInput NEXUS_VideoDecoder_P_GetConnector_Common( NEXUS_VideoDecoderHandle videoDecoder);


NEXUS_Error NEXUS_VideoDecoder_P_Init_Generic(NEXUS_VideoDecoderHandle videoDecoder, const NEXUS_RaveOpenSettings *raveSettings, const NEXUS_VideoDecoderOpenMosaicSettings *openSettings);
void NEXUS_VideoDecoder_P_GetMosaicOpenSettings(NEXUS_VideoDecoderOpenMosaicSettings *openMosaicSettings, const NEXUS_VideoDecoderOpenSettings *openSettings);
void NEXUS_VideoDecoder_P_Close_Generic(NEXUS_VideoDecoderHandle videoDecoder);
NEXUS_Error NEXUS_VideoDecoder_P_Start_Generic_Prologue(NEXUS_VideoDecoderHandle videoDecoder, const NEXUS_VideoDecoderStartSettings *pStartSettings);
NEXUS_Error NEXUS_VideoDecoder_P_Start_Priv_Generic_Epilogue(NEXUS_VideoDecoderHandle videoDecoder, const NEXUS_VideoDecoderStartSettings *pStartSettings);
NEXUS_Error NEXUS_VideoDecoder_P_Start_Generic_Epilogue(NEXUS_VideoDecoderHandle videoDecoder, const NEXUS_VideoDecoderStartSettings *pStartSettings);
NEXUS_Error NEXUS_VideoDecoder_P_Start_Generic_Body(NEXUS_VideoDecoderHandle videoDecoder, const NEXUS_VideoDecoderStartSettings *pStartSettings, bool otfPvr, BAVC_VideoCompressionStd *pVideoCmprStd, NEXUS_RaveStatus *raveStatus, const NEXUS_StcChannelDecoderConnectionSettings *stcChannelConnectionSettings, bool *pPlayback, unsigned *stcChannelIndex);
NEXUS_Error NEXUS_VideoDecoder_P_Stop_Generic_Prologue(NEXUS_VideoDecoderHandle videoDecoder);
void NEXUS_VideoDecoder_P_Stop_Priv_Generic_Prologue(NEXUS_VideoDecoderHandle videoDecoder);
void NEXUS_VideoDecoder_P_Stop_Priv_Generic_Epilogue(NEXUS_VideoDecoderHandle videoDecoder);
void NEXUS_VideoDecoder_P_Stop_Generic_Epilogue(NEXUS_VideoDecoderHandle videoDecoder);
void NEXUS_VideoDecoder_P_GetStatus_Generic(NEXUS_VideoDecoderHandle videoDecoder, NEXUS_VideoDecoderStatus *pStatus);
const BAVC_MFD_Picture * NEXUS_VideoDecoder_P_DataReady_PreprocessFieldData_isr(NEXUS_VideoDecoderHandle videoDecoder, const BAVC_MFD_Picture *pFieldData, BAVC_MFD_Picture *pModifiedFieldData);
void NEXUS_VideoDecoder_P_DataReady_Generic_Prologue_isr(NEXUS_VideoDecoderHandle videoDecoder, const BAVC_MFD_Picture *pFieldData);
void NEXUS_VideoDecoder_P_DataReady_Generic_Epilogue_isr(NEXUS_VideoDecoderHandle videoDecoder, const BAVC_MFD_Picture *pFieldData, unsigned pictureDeliveryCount);
void NEXUS_VideoDecoder_P_IsCodecSupported_Generic( NEXUS_VideoDecoderHandle videoDecoder, NEXUS_VideoCodec codec, bool *pSupported);

void NEXUS_VideoDecoder_P_GetExtendedSettings_NotImplemented( NEXUS_VideoDecoderHandle handle, NEXUS_VideoDecoderExtendedSettings *pSettings);
NEXUS_Error NEXUS_VideoDecoder_P_SetExtendedSettings_NotImplemented( NEXUS_VideoDecoderHandle handle, const NEXUS_VideoDecoderExtendedSettings *pSettings);


NEXUS_Error NEXUS_VideoDecoder_P_Xdm_Initialize(NEXUS_VideoDecoderHandle decoder, const BXDM_Decoder_Interface *decoderInterface, void *decoderContext);
void NEXUS_VideoDecoder_P_Xdm_Shutdown(NEXUS_VideoDecoderHandle decoder);
NEXUS_Error NEXUS_VideoDecoder_P_Xdm_Start(NEXUS_VideoDecoderHandle decoder);
void NEXUS_VideoDecoder_P_Xdm_Stop(NEXUS_VideoDecoderHandle decoder);
NEXUS_Error NEXUS_VideoDecoder_P_Xdm_ApplySettings(NEXUS_VideoDecoderHandle decoder, const NEXUS_VideoDecoderSettings *pSettings, bool force);
NEXUS_Error NEXUS_VideoDecoder_P_Xdm_SetChannelChangeMode(NEXUS_VideoDecoderHandle videoDecoder, NEXUS_VideoDecoder_ChannelChangeMode channelChangeMode);
NEXUS_Error NEXUS_VideoDecoder_P_Xdm_SetMute(NEXUS_VideoDecoderHandle videoDecoder);
NEXUS_Error NEXUS_VideoDecoder_P_Xdm_SetFreeze(NEXUS_VideoDecoderHandle videoDecoder);
NEXUS_Error NEXUS_VideoDecoder_SetSyncSettings_priv_Xdm(NEXUS_VideoDecoderHandle videoDecoder, const NEXUS_VideoInputSyncSettings *pSyncSettings);
NEXUS_Error NEXUS_VideoDecoder_P_Xdm_SetPtsOffset(NEXUS_VideoDecoderHandle videoDecoder);
void NEXUS_VideoDecoder_P_ApplyDisplayInformation_Xdm( NEXUS_VideoDecoderHandle videoDecoder);

NEXUS_Error NEXUS_VideoDecoder_SetDisplayConnection_priv_Xdm(NEXUS_VideoDecoderHandle decoder, const NEXUS_VideoDecoderDisplayConnection *connection);
void NEXUS_VideoDecoder_UpdateDisplayInformation_priv_Xdm(NEXUS_VideoDecoderHandle decoder, const NEXUS_VideoDecoder_DisplayInformation *pDisplayInformation);
NEXUS_Error NEXUS_VideoDecoder_P_GetStatus_Generic_Xdm( NEXUS_VideoDecoderHandle decoder, NEXUS_VideoDecoderStatus *pStatus);
BERR_Code NEXUS_VideoDecoder_P_GetPtsCallback_Xdm_isr(void *pContext, BAVC_PTSInfo *pPTSInfo);
BERR_Code NEXUS_VideoDecoder_P_StcValidCallback_Xdm_isr(void *pContext);
BERR_Code NEXUS_VideoDecoder_P_SetPcrOffset_Xdm_isr(void *pContext, uint32_t pcrOffset);
BERR_Code NEXUS_VideoDecoder_P_GetPcrOffset_Xdm_isr(void *pContext, uint32_t *pPcrOffset);

NEXUS_Error NEXUS_VideoDecoder_P_SetXvdDisplayInterrupt(NEXUS_VideoDecoderHandle videoDecoder, const NEXUS_VideoDecoderDisplayConnection *connection, BXVD_DisplayInterrupt interrupt);
NEXUS_Error NEXUS_VideoDecoder_P_SetSimulatedStc_isr(NEXUS_VideoDecoderHandle videoDecoder, uint32_t pts);
NEXUS_Error NEXUS_VideoDecoder_P_SetUnderflowMode_isr(NEXUS_VideoDecoderHandle videoDecoder, bool underflowMode);

NEXUS_Error NEXUS_VideoDecoder_P_SetLowLatencySettings(NEXUS_VideoDecoderHandle videoDecoder);
void NEXUS_VideoDecoderPrimer_P_Link(NEXUS_VideoDecoderPrimerHandle primer, NEXUS_VideoDecoderHandle videoDecoder);
void NEXUS_VideoDecoderPrimer_P_Unlink(NEXUS_VideoDecoderPrimerHandle primer, NEXUS_VideoDecoderHandle videoDecoder);
NEXUS_Error NEXUS_VideoDecoder_P_SetDiscardThreshold_Avd(NEXUS_VideoDecoderHandle videoDecoder);

NEXUS_MemoryBlockHandle NEXUS_VideoDecoder_P_MemoryBlockFromMma(NEXUS_VideoDecoderHandle decoder, BMMA_Block_Handle mma);
bool nexus_p_use_secure_picbuf(NEXUS_VideoDecoderHandle videoDecoder);
struct NEXUS_VideoDecoderDevice *nexus_video_decoder_p_any_device(void);

NEXUS_OBJECT_CLASS_DECLARE(NEXUS_StillDecoder);
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_VideoDecoderPrimer);

#ifdef __cplusplus
}
#endif

#endif

