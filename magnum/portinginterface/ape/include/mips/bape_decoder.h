/***************************************************************************
 *     Copyright (c) 2006-2010, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description: Audio Decoder Interface
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef BAPE_DECODER_H_
#define BAPE_DECODER_H_

#include "bavc.h"
#include "bape_types.h"
#include "bape_input.h"
#include "bape_codec_types.h"

/***************************************************************************
Summary:
Decoder Handle
***************************************************************************/
typedef struct BAPE_Decoder *BAPE_DecoderHandle;

/***************************************************************************
Summary:
Decoder Open settings
***************************************************************************/
typedef struct BAPE_DecoderOpenSettings
{
    bool independentDelay;      /* If true, independent delay is enabled.  
                                   Outputs will use larger buffers to accomodate 
                                   the additional delay. */
} BAPE_DecoderOpenSettings;

/***************************************************************************
Summary:
Get default open settings for an audio decoder
***************************************************************************/
void BAPE_Decoder_GetDefaultOpenSettings(
    BAPE_DecoderOpenSettings *pSettings     /* [out] */
    );

/***************************************************************************
Summary:
Open an audio decoder
***************************************************************************/
BERR_Code BAPE_Decoder_Open(
    BAPE_Handle deviceHandle,
    unsigned index,
    const BAPE_DecoderOpenSettings *pSettings, 
    BAPE_DecoderHandle *pHandle                 /* [out] */
    );

/***************************************************************************
Summary:
Close an audio decoder
***************************************************************************/
void BAPE_Decoder_Close(
    BAPE_DecoderHandle handle
    );

/***************************************************************************
Summary:
Start-time settings for an audio decoder
***************************************************************************/
typedef struct BAPE_DecoderStartSettings
{
    BAVC_AudioCompressionStd codec;
    BAVC_StreamType streamType;         /* Required for TSM computation */

    unsigned stcIndex;                  /* Which STC index should be used */

    const BAVC_XptContextMap *pContextMap;  /* What RAVE context should be read while decoding.  
                                               If decoding from an input port, pass NULL for this 
                                               setting */
    BAPE_InputPort inputPort;               /* What input port to read data from.  If decoding from
                                               RAVE, pass NULL for this setting */

    bool ppmCorrection;                 /* If true, PPM correction will be performed 
                                           for 2ms lipsync precision on PCM outputs */
} BAPE_DecoderStartSettings;

/***************************************************************************
Summary:
Get default start settings for an audio decoder
***************************************************************************/
void BAPE_Decoder_GetDefaultStartSettings(
    BAPE_DecoderStartSettings *pSettings    /* [out] */
    );

/***************************************************************************
Summary:
Start an audio decoder
***************************************************************************/
BERR_Code BAPE_Decoder_Start(
    BAPE_DecoderHandle handle,
    const BAPE_DecoderStartSettings *pSettings
    );

/***************************************************************************
Summary:
Stop an audio decoder
***************************************************************************/
void BAPE_Decoder_Stop(
    BAPE_DecoderHandle handle
    );

/***************************************************************************
Summary:
Pause an audio decoder
***************************************************************************/
BERR_Code BAPE_Decoder_Pause(
    BAPE_DecoderHandle handle
    );

/***************************************************************************
Summary:
Resume an audio decoder from a paused state
***************************************************************************/
BERR_Code BAPE_Decoder_Resume(
    BAPE_DecoderHandle handle
    );

/***************************************************************************
Summary:
Advance an audio decoder in ms units.  Must be paused first.
***************************************************************************/
BERR_Code BAPE_Decoder_Advance(
    BAPE_DecoderHandle handle,
    unsigned milliseconds           /* Milliseconds to advance */
    );

/***************************************************************************
Summary:
Prepare the decoder to flush (called prior to flushing RAVE context) 
***************************************************************************/
BERR_Code BAPE_Decoder_DisableForFlush(
    BAPE_DecoderHandle handle
    );

/***************************************************************************
Summary:
Flush the decoder's buffers (called after flushing RAVE context) 
***************************************************************************/
BERR_Code BAPE_Decoder_Flush(
    BAPE_DecoderHandle handle
    );

/***************************************************************************
Summary:
Audio Decoder TSM Settings
***************************************************************************/
typedef struct BAPE_DecoderTsmSettings
{
    bool tsmEnabled;
    bool astmEnabled;
    bool playback;
    uint32_t ptsOffset;     /* PTS offset value in 45 kHz ticks.  Will be internally converted to
                               27MHz if required. */

    /* All thresholds below are programmed in milliseconds. */
    struct 
    {
        unsigned discard;           /* Threshold (ms) beyond which frames will be discarded */
        unsigned grossAdjustment;   /* Threshold (ms) beyond which gross adjustments will be performed */
        unsigned syncLimit;         /* Sets the sync limit for audio master mode (in ms).  Set to 0 to disable audio master mode */
    } thresholds;
} BAPE_DecoderTsmSettings;

/***************************************************************************
Summary:
Get Audio Decoder TSM Settings
***************************************************************************/
void BAPE_Decoder_GetTsmSettings(
    BAPE_DecoderHandle handle,
    BAPE_DecoderTsmSettings *pSettings  /* [out] */
    );

/***************************************************************************
Summary:
Get Audio Decoder TSM Settings in isr context
***************************************************************************/
void BAPE_Decoder_GetTsmSettings_isr(
    BAPE_DecoderHandle handle,
    BAPE_DecoderTsmSettings *pSettings  /* [out] */
    );

/***************************************************************************
Summary:
Set Audio Decoder TSM Settings
***************************************************************************/
BERR_Code BAPE_Decoder_SetTsmSettings(
    BAPE_DecoderHandle handle,
    const BAPE_DecoderTsmSettings *pSettings
    );

/***************************************************************************
Summary:
Set Audio Decoder TSM Settings in isr context
***************************************************************************/
BERR_Code BAPE_Decoder_SetTsmSettings_isr(
    BAPE_DecoderHandle handle,
    const BAPE_DecoderTsmSettings *pSettings
    );

/***************************************************************************
Summary:
Indicate a playback STC is valid to the decoder in isr context
***************************************************************************/
BERR_Code BAPE_Decoder_SetStcValid_isr(
    BAPE_DecoderHandle handle
    );

/***************************************************************************
Summary:
Audio Decoder TSM Status
***************************************************************************/
typedef struct BAPE_DecoderTsmStatus
{
    BAVC_PTSInfo ptsInfo;
    int32_t ptsStcDifference;   /* PTS - STC value */
} BAPE_DecoderTsmStatus;

/***************************************************************************
Summary:
Get Audio Decoder TSM Status
***************************************************************************/
BERR_Code BAPE_Decoder_GetTsmStatus(
    BAPE_DecoderHandle handle,
    BAPE_DecoderTsmStatus *pStatus /* [out] */
    );

/***************************************************************************
Summary:
Get Audio Decoder TSM Status in isr context
***************************************************************************/
BERR_Code BAPE_Decoder_GetTsmStatus_isr(
    BAPE_DecoderHandle handle,
    BAPE_DecoderTsmStatus *pStatus  /* [out] */
    );

/***************************************************************************
Summary:
Audio Decoder Settings that are changeable on the fly
***************************************************************************/
typedef struct BAPE_DecoderSettings
{
    BAPE_DualMonoMode dualMonoMode;
    BAPE_ChannelMode outputMode;
    bool outputLfe;    
} BAPE_DecoderSettings;

/***************************************************************************
Summary:
Get Audio Decoder Settings
***************************************************************************/
void BAPE_Decoder_GetSettings(
    BAPE_DecoderHandle handle,
    BAPE_DecoderSettings *pSettings     /* [out] */
    );

/***************************************************************************
Summary:
Set Audio Decoder Settings
***************************************************************************/
BERR_Code BAPE_Decoder_SetSettings(
    BAPE_DecoderHandle handle,
    const BAPE_DecoderSettings *pSettings
    );

/***************************************************************************
Summary:
Audio Decoder Status
***************************************************************************/
typedef struct BAPE_DecoderStatus
{
    BAVC_AudioCompressionStd codec;
    BAPE_DecoderTsmStatus tsmStatus;
    BAVC_AudioSamplingRate sampleRate;
    unsigned framesDecoded;
    unsigned frameErrors;
    unsigned dummyFrames;
    union
    {
        BAPE_MpegStatus mpeg;
        BAPE_Ac3Status  ac3;
        BAPE_AacStatus  aac;
        BAPE_WmaStatus  wma;
    } codecStatus;
} BAPE_DecoderStatus;

/***************************************************************************
Summary:
Get Audio Decoder Status
***************************************************************************/
void BAPE_Decoder_GetStatus(
    BAPE_DecoderHandle handle,
    BAPE_DecoderStatus *pStatus     /* [out] */
    );

/***************************************************************************
Summary:
Audio Decoder Codec-Specific Settings
***************************************************************************/
typedef struct BAPE_DecoderCodecSettings
{
    BAVC_AudioCompressionStd codec;
    union
    {
        BAPE_Ac3Settings  ac3;
        BAPE_Ac3Settings  ac3Plus;
        BAPE_AacSettings  aac;       /* Applies to both ADTS/LOAS */
        BAPE_AacSettings  aacPlus;   /* Applies to both ADTS/LOAS */
    } codecSettings;
} BAPE_DecoderCodecSettings;

/***************************************************************************
Summary:
Get Audio Decoder Codec-Specific Settings
***************************************************************************/
void BAPE_Decoder_GetCodecSettings(
    BAPE_DecoderHandle handle,
    BAVC_AudioCompressionStd codec,
    BAPE_DecoderCodecSettings *pSettings     /* [out] */
    );

/***************************************************************************
Summary:
Set Audio Decoder Codec-Specific Settings
***************************************************************************/
BERR_Code BAPE_Decoder_SetCodecSettings(
    BAPE_DecoderHandle handle,
    const BAPE_DecoderCodecSettings *pSettings
    );

/***************************************************************************
Summary:
Get Audio Source Connector for stereo output data
***************************************************************************/
void BAPE_Decoder_GetStereoPcmConnector(
    BAPE_DecoderHandle handle,
    BAPE_MixerInput *pConnector /* [out] */
    );

/***************************************************************************
Summary:
Get Audio Source Connector for multichannel output data
***************************************************************************/
void BAPE_Decoder_GetMultichannelPcmConnector(
    BAPE_DecoderHandle handle,
    BAPE_MixerInput *pConnector /* [out] */
    );

/***************************************************************************
Summary:
Get Audio Source Connector for compressed output data
***************************************************************************/
void BAPE_Decoder_GetCompressedConnector(
    BAPE_DecoderHandle handle,
    BAPE_MixerInput *pConnector /* [out] */
    );

/***************************************************************************
Summary:
Decoder Interrupt Handlers
***************************************************************************/
typedef struct BAPE_DecoderInterruptHandlers
{
    /* Interrupt fires when first PTS is received */
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2, BAPE_DecoderTsmStatus *pTsmStatus);
        void *pParam1;
        int param2;
    } firstPts;
    /* Interrupt fires when TSM Fail (PTS Error) occurs */
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2, BAPE_DecoderTsmStatus *pTsmStatus);
        void *pParam1;
        int param2;    
    } tsmFail;
    /* Interrupt fires when TSM transitions from fail -> pass in ASTM mode */
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2, BAPE_DecoderTsmStatus *pTsmStatus);
        void *pParam1;
        int param2;
    } tsmPass;
    /* Interrupt fires when the decoder receives the first or any new sample rate in the stream */
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2, BAVC_AudioSamplingRate sampleRate);
        void *pParam1;
        int param2;
    } sampleRateChange;
    /* Interrupt fires when the decoder achieves frame lock */
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2);
        void *pParam1;
        int param2;
    } lock;
    /* Interrupt fires when the decoder loses frame lock */
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2);
        void *pParam1;
        int param2;
    } unlock;
    /* Interrupt fires when the decoder status is ready for the host to read.
       Typically, this occurs after the first frame is successfully decoded. */
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2);
        void *pParam1;
        int param2;
    } statusReady;
} BAPE_DecoderInterruptHandlers;

/***************************************************************************
Summary:
Get Currently Registered Interrupt Handlers
***************************************************************************/
void BAPE_Decoder_GetInterruptHandlers(
    BAPE_DecoderHandle handle,
    BAPE_DecoderInterruptHandlers *pInterrupts     /* [out] */
    );

/***************************************************************************
Summary:
Set Interrupt Handlers 
 
Description: 
To disable any unwanted interrupt, pass NULL for its callback routine
***************************************************************************/
BERR_Code BAPE_Decoder_SetInterruptHandlers(
    BAPE_DecoderHandle handle,
    const BAPE_DecoderInterruptHandlers *pInterrupts
    );

/***************************************************************************
Summary:
Get Default CDB/ITB configuration for decoding
***************************************************************************/
void BAPE_Decoder_GetDefaultCdbItbConfig(
    BAPE_DecoderHandle handle,
    BAVC_CdbItbConfig *pConfig  /* [out] */
    );

#endif

