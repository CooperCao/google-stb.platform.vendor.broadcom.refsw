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

#ifndef BAPE_PRIV_H_
#define BAPE_PRIV_H_

#include "bstd.h"
#include "bkni.h"
#include "bape.h"
#include "blst_slist.h"
#include "bape_chip_priv.h"
#include "bape_dsp_priv.h"
#include "bchp_aud_fmm_ms_ctrl.h"

/* Debug objects */
BDBG_OBJECT_ID_DECLARE(BAPE_Device);
BDBG_OBJECT_ID_DECLARE(BAPE_BufferNode);
BDBG_OBJECT_ID_DECLARE(BAPE_Decoder);
BDBG_OBJECT_ID_DECLARE(BAPE_Mixer);
BDBG_OBJECT_ID_DECLARE(BAPE_MixerOutput);
BDBG_OBJECT_ID_DECLARE(BAPE_MixerInput);
BDBG_OBJECT_ID_DECLARE(BAPE_MixerInputLink);

/***************************************************************************
Summary:
PLL Status
***************************************************************************/
typedef struct BAPE_PllStatus
{
    unsigned baseSampleRate;
    uint32_t baseAddress;
    BLST_S_HEAD(PllMixerList, BAPE_Mixer) mixerList;    /* List of mixers using this PLL */
    uint8_t fsChannel[BAPE_CHIP_MAX_FS];                /* 0 = not connected.  1=ch1, 2=ch2, 3=ch3 */
    BAPE_PllSettings settings;
} BAPE_PllStatus;

/***************************************************************************
Summary:
L3 interrupt structure
***************************************************************************/
typedef struct BAPE_L3Interrupt
{
    BINT_CallbackFunc callback_isr;
    void *pParam1;
    int param2;
} BAPE_L3Interrupt;

/***************************************************************************
Summary:
Decoder State
***************************************************************************/
typedef enum BAPE_DecoderState
{
    BAPE_DecoderState_eStopped,             /* Not running */
    BAPE_DecoderState_eStarted,             /* Running in normal operation */
    BAPE_DecoderState_ePaused,              /* Running, paused */
    BAPE_DecoderState_eDisabled,            /* Prepared for flush from started state */
    BAPE_DecoderState_eDisabledPaused,      /* Prepared for flush from paused state */
    BAPE_DecoderState_eMax
} BAPE_DecoderState;

/***************************************************************************
Summary:
Device Handle
***************************************************************************/
typedef struct BAPE_Device
{
    BDBG_OBJECT(BAPE_Device)

    /* Open Parameters */
    const BIMG_Interface *pImageInterface;
    void **pImageContext;
    BCHP_Handle chpHandle;
    BREG_Handle regHandle;
    BMEM_Handle memHandle;
    BINT_Handle intHandle;
    BTMR_Handle tmrHandle;
    BAPE_Settings settings;
    
    /* Buffer Sizes */
    unsigned pcmBufferSize;
    unsigned compressedBufferSize;

    /* Free Buffer Lists */
    BLST_S_HEAD(PcmBufferList, BAPE_BufferNode) pcmBufferList;
    BLST_S_HEAD(CompressedBufferList, BAPE_BufferNode) compressedBufferList;

    /* Software resource allocation */
#if BAPE_CHIP_MAX_DECODERS > 0
    BAPE_DecoderHandle       decoders[BAPE_CHIP_MAX_DECODERS];
    struct 
    {
        BAPE_DecoderState state;
        BAPE_DecoderStartSettings startSettings;
    } decoderWatchdogInfo[BAPE_CHIP_MAX_DECODERS];
#endif
#if BAPE_CHIP_MAX_INPUT_CAPTURES > 0
    BAPE_InputCaptureHandle  inputCaptures[BAPE_CHIP_MAX_INPUT_CAPTURES];
#endif
#if BAPE_CHIP_MAX_PLAYBACKS > 0
    BAPE_PlaybackHandle      playbacks[BAPE_CHIP_MAX_PLAYBACKS];
#endif
#if BAPE_CHIP_MAX_DACS > 0
    BAPE_DacHandle           dacs[BAPE_CHIP_MAX_DACS];
#endif
#if BAPE_CHIP_MAX_I2S_OUTPUTS > 0
    BAPE_I2sOutputHandle     i2sOutputs[BAPE_CHIP_MAX_I2S_OUTPUTS];
#endif
#if BAPE_CHIP_MAX_SPDIF_OUTPUTS > 0
    BAPE_SpdifOutputHandle   spdifOutputs[BAPE_CHIP_MAX_SPDIF_OUTPUTS];
#endif
#if BAPE_CHIP_MAX_MAI_OUTPUTS > 0
    BAPE_MaiOutputHandle     maiOutputs[BAPE_CHIP_MAX_MAI_OUTPUTS];
#endif
#if BAPE_CHIP_MAX_OUTPUT_CAPTURES > 0
    BAPE_OutputCaptureHandle outputCaptures[BAPE_CHIP_MAX_OUTPUT_CAPTURES];
#endif
#if BAPE_CHIP_MAX_I2S_INPUTS > 0
    BAPE_I2sInputHandle      i2sInputs[BAPE_CHIP_MAX_I2S_INPUTS];
#endif
#if BAPE_CHIP_MAX_RFMODS > 0
    BAPE_RfModHandle         rfmods[BAPE_CHIP_MAX_RFMODS];
#endif

    BAPE_MixerHandle         mixers[BAPE_CHIP_MAX_MIXERS];

    BLST_S_HEAD(InputGroupList, BAPE_InputGroup) inputGroupList;
    
    /* PLL Status */
    BAPE_PllStatus   pllStatus[BAPE_CHIP_MAX_PLLS];

    /* Interrupts */
    BINT_CallbackHandle isrBfEsr1;
    BINT_CallbackHandle isrBfEsr2;
    BAPE_L3Interrupt sourceRbufFreemark[BAPE_CHIP_MAX_SOURCE_CHANNELS];
    BAPE_L3Interrupt destRbufFullmark[BAPE_CHIP_MAX_DESTINATION_CHANNELS];
    BAPE_L3Interrupt destRbufOverflow[BAPE_CHIP_MAX_DESTINATION_CHANNELS];

    /* DSP */
    BAPE_DspSettings dspSettings;
    BAPE_DspHandle dsp;

    /* Hardware resource allocation map */
    bool sourceChannelAllocated[BAPE_CHIP_MAX_SOURCE_CHANNELS];
    bool destinationChannelAllocated[BAPE_CHIP_MAX_DESTINATION_CHANNELS];
    bool srcAllocated[BAPE_CHIP_MAX_SRCS];
    bool mixerAllocated[BAPE_CHIP_MAX_MIXERS];
    bool playbackAllocated[BAPE_CHIP_MAX_MIXER_PLAYBACKS];
    bool dummySinkAllocated[BAPE_CHIP_MAX_DUMMYSINKS];
    bool loopbackAllocated[BAPE_CHIP_MAX_LOOPBACKS];
    bool fsAllocated[BAPE_CHIP_MAX_FS];
    bool adaptRateAllocated[BAPE_CHIP_MAX_ADAPTRATE_CONTROLLERS];

    /* Interrupts */
    BAPE_InterruptHandlers interrupts;
} BAPE_Device;

/***************************************************************************
Summary:
Buffer Node
***************************************************************************/
typedef struct BAPE_BufferNode
{
    BDBG_OBJECT(BAPE_BufferNode)
    BLST_S_ENTRY(BAPE_BufferNode) node;
    void *pMemory;
    uint32_t offset;
    unsigned bufferSize;
} BAPE_BufferNode;

/***************************************************************************
Summary:
Invalid Stream ID
***************************************************************************/
#define BAPE_STREAM_ID_INVALID (0xff)

/***************************************************************************
Summary:
Invalid Mixer ID
***************************************************************************/
#define BAPE_MIXER_ID_INVALID (0xffff)

/***************************************************************************
Summary:
Create Mixer ID
***************************************************************************/
#define BAPE_MIXER_ID_CREATE(mixer,port) ((((uint16_t)(mixer))<<8)|(port&0xff))

/***************************************************************************
Summary:
Get Mixer from ID
***************************************************************************/
#define BAPE_MIXER_ID_TO_MIXER(id) ((id)>>8)

/***************************************************************************
Summary:
Get Port from mixer ID
***************************************************************************/
#define BAPE_MIXER_ID_TO_PORT(id) ((id)&0xff)

/***************************************************************************
Summary:
Get FCI ID from mixer output
***************************************************************************/
#define BAPE_MIXER_ID_TO_FCI(id) ((0x4<<6)|(BAPE_MIXER_ID_TO_MIXER(id)*2)|BAPE_MIXER_ID_TO_PORT(id))

/***************************************************************************
Summary:
MixerInput -> Mixer Link
***************************************************************************/
typedef struct BAPE_MixerInputLink
{
    BDBG_OBJECT(BAPE_MixerInputLink)
    BAPE_MixerInput input;
    BAPE_MixerHandle mixer;
    BLST_S_ENTRY(BAPE_MixerInputLink) node;
    /* Resources valid if multiple source channels are permitted (BAPE_MixerInput.mustBranchAtSrc=false) */
    uint8_t sourceChannelId[BAPE_ChannelPair_eMax];
    uint8_t srcId[BAPE_ChannelPair_eMax];
    uint8_t playbackId[BAPE_ChannelPair_eMax];
    uint8_t adaptRateId[BAPE_ChannelPair_eMax];
    /* If active, resources are valid */
    bool active;
} BAPE_MixerInputLink;

/***************************************************************************
Summary:
Timing Source Options
***************************************************************************/
typedef enum BAPE_MclkSource
{
    BAPE_MclkSource_eNone,
    BAPE_MclkSource_ePll0,
    BAPE_MclkSource_ePll1,
    BAPE_MclkSource_ePll2,
    BAPE_MclkSource_eHifidac0,
    BAPE_MclkSource_eHifidac1,
    BAPE_MclkSource_eHifidac2,
    BAPE_MclkSource_eMax
} BAPE_MclkSource;

/***************************************************************************
Summary:
Mixer Structure
***************************************************************************/
typedef struct BAPE_Mixer
{
    BDBG_OBJECT(BAPE_Mixer)
    unsigned running;
    bool compressed;
    bool ready;
    unsigned numChannelPairs;
    unsigned index;    
    unsigned numMixerGroups;
    unsigned fs;
    BAVC_AudioSamplingRate sampleRate;
    BAPE_MclkSource mclkSource;
    uint8_t dpMixers[BAPE_ChannelPair_eMax][BAPE_CHIP_MAX_MIXERS];
    BAPE_Handle deviceHandle;
    BAPE_MixerSettings settings;
    BAPE_MixerInput master;
    BAPE_MixerInput inputs[BAPE_CHIP_MAX_MIXER_INPUTS];
    bool inputMuted[BAPE_CHIP_MAX_MIXER_INPUTS];
    BAPE_MixerInputLink *pInputLinks[BAPE_CHIP_MAX_MIXER_INPUTS];
    BAPE_MixerInputVolume inputVolume[BAPE_CHIP_MAX_MIXER_INPUTS];
    BLST_S_HEAD(OutputList, BAPE_MixerOutputObject) outputList;
    BLST_S_ENTRY(BAPE_Mixer) pllNode;
} BAPE_Mixer;

/***************************************************************************
Summary:
Generic Mixer Output Structure
***************************************************************************/
typedef struct BAPE_MixerOutputObject
{
    BDBG_OBJECT(BAPE_MixerOutput)
    void *pHandle;
    BAPE_MixerHandle mixer; /* NULL if not connected */
    BLST_S_ENTRY(BAPE_MixerOutputObject) node;
    unsigned maxChannelPairs;
    BAPE_MclkSource mclkOutput;     /* If this output generates an MCLK (e.g. DAC), which one is it? */
    bool compressedSupported;
    bool muteInMixer;
    bool fsTiming;                  /* If this output slaves to an Fs source, this is true. */
    bool pllRequired;               /* If this output should always use a PLL this is true. */
    uint16_t mixerId[BAPE_ChannelPair_eMax];
    uint8_t streamId[BAPE_ChannelPair_eMax];
    uint16_t dummyMixerId;
    uint8_t dummyStreamId;
    void (*setTimingParams_isr)(void *pHandle, BAVC_AudioSamplingRate sampleRate, BAVC_Timebase timebase);
    void (*setMclk_isr)(void *pHandle, BAPE_MclkSource mclkSource, unsigned multiplier);
    void (*setFs)(void *pHandle, unsigned fsNum);
    void (*setMute)(void *pHandle, bool muted, bool sync);
    void (*enable)(void *pHandle);
    void (*disable)(void *pHandle);
    BAPE_OutputVolume volume;
    const char *pName;
} BAPE_MixerOutputObject;

/***************************************************************************
Summary:
Init a mixer output
***************************************************************************/
#define BAPE_P_InitMixerOutput(output, handle) \
   do {\
         unsigned i;\
         BKNI_Memset((output),0,sizeof(BAPE_MixerOutputObject));\
         BDBG_OBJECT_SET((output),BAPE_MixerOutput);\
         (output)->pHandle=handle;\
         for ( i = 0; i < BAPE_Channel_eMax; i++ ) { (output)->volume.volume[i] = 0x800000; }\
         for ( i = 0; i < BAPE_ChannelPair_eMax; i++ ) { (output)->mixerId[i] = BAPE_MIXER_ID_INVALID; (output)->streamId[i] = BAPE_STREAM_ID_INVALID; }\
         (output)->dummyMixerId = BAPE_MIXER_ID_INVALID;\
         (output)->dummyStreamId = BAPE_STREAM_ID_INVALID;\
   } while(0)

/***************************************************************************
Summary:
Mixer Input Types
***************************************************************************/
typedef enum BAPE_MixerInputType
{
    BAPE_MixerInputType_eDecoder,
    BAPE_MixerInputType_ePlayback,
    BAPE_MixerInputType_eInputCapture,
    BAPE_MixerInputType_eProcessor,
    BAPE_MixerInputType_eMax
} BAPE_MixerInputType;

/***************************************************************************
Summary:
Generic Mixer Input Structure
***************************************************************************/
typedef struct BAPE_MixerInputObject
{
    BDBG_OBJECT(BAPE_MixerInput)
    void *pHandle;
    BAPE_MixerInputType type;
    bool compressed;
    bool mustBranchAtSrc;       /* If true, multiple MixerInput->Mixer links branch between SRC & DP.  If false, multiple links use master/slave source channels */
    bool useBufferPool;         /* If true, buffers are allocated from the buffer pool.  If false, they are handled externally (e.g. playback). */
    unsigned numChannelPairs;
    unsigned activeLinks;
    BAVC_AudioSamplingRate sampleRate;
    /* Links to mixer(s) */
    BLST_S_HEAD(InputMixerList, BAPE_MixerInputLink) mixerList;
    /* Post-processing */
    BAPE_MixerInput parent;
    BLST_S_ENTRY(BAPE_MixerInputObject) childNode;
    BLST_S_HEAD(ChildList, BAPE_MixerInputObject) childList;
    /* Grouping */
    BAPE_InputGroupHandle group;
    /* Resources if branched at SRC */
    uint8_t sourceChannelId[BAPE_ChannelPair_eMax];
    uint8_t srcId[BAPE_ChannelPair_eMax];
    uint8_t playbackId[BAPE_ChannelPair_eMax];
    uint8_t adaptRateId[BAPE_ChannelPair_eMax];
    /* Buffer resources if useBufferBool is true */
    BAPE_BufferNode *pBuffers[BAPE_ChannelPair_eMax];
    const char *pName;
} BAPE_MixerInputObject;

/***************************************************************************
Summary:
Init a mixer input
***************************************************************************/
#define BAPE_P_InitMixerInput(input, inputType, handle) \
   do {\
         unsigned i;\
         BKNI_Memset((input),0,sizeof(BAPE_MixerInputObject));\
         BDBG_OBJECT_SET((input),BAPE_MixerInput);\
         (input)->pHandle=(handle);\
         (input)->type=(inputType);\
         (input)->sampleRate=BAVC_AudioSamplingRate_eUnknown;\
         BLST_S_INIT(&(input)->mixerList); \
         BLST_S_INIT(&(input)->childList); \
         for ( i = 0; i < BAPE_ChannelPair_eMax; i++ ) (input)->sourceChannelId[i] = (input)->srcId[i] = (input)->playbackId[i] = (input)->adaptRateId[i] = 0xff;\
   } while(0)

/***************************************************************************
Summary:
Release a mixer's resources if not in use
***************************************************************************/
#if 0
void BAPE_Mixer_P_ReleaseResources(
    BAPE_MixerHandle handle
    );
#else
#define BAPE_Mixer_P_ReleaseResources(h) BAPE_Mixer_P_ReleaseResources_impl((h), __FILE__, __LINE__)
void BAPE_Mixer_P_ReleaseResources_impl(
    BAPE_MixerHandle handle,
    const char *pFilename,
    unsigned line
    );
#endif

/***************************************************************************
Summary:
Prepare to close a MixerInput and release all resources
***************************************************************************/
void BAPE_MixerInput_P_Shutdown(
    BAPE_MixerInput input
    );

/***************************************************************************
Summary:
Get an input's source channel ID for a channel pair
***************************************************************************/
uint8_t BAPE_MixerInput_P_GetSourceChannel(
    BAPE_MixerInput input,
    BAPE_ChannelPair channelPair
    );

/***************************************************************************
Summary:
Prepare the mixer to start an input
***************************************************************************/
BERR_Code BAPE_MixerInput_P_PrepareToStart(
    BAPE_MixerInput input,
    bool adaptiveRateControl
    );

/***************************************************************************
Summary:
Start an input
***************************************************************************/
BERR_Code BAPE_MixerInput_P_Start(
    BAPE_MixerInput input,
    BAVC_AudioSamplingRate sampleRate
    );

/***************************************************************************
Summary:
Set an input's sample rate on the fly (used with decoders)
***************************************************************************/
void BAPE_MixerInput_P_SetSampleRate_isr(
    BAPE_MixerInput input, 
    BAVC_AudioSamplingRate sampleRate
    );

/***************************************************************************
Summary:
Stop an input
***************************************************************************/
void BAPE_MixerInput_P_Stop(
    BAPE_MixerInput input
    );

/***************************************************************************
Summary:
Get number of outputs connected to a mixer
***************************************************************************/
unsigned BAPE_MixerInput_P_GetNumOutputs(
    BAPE_MixerInput input
    );

/***************************************************************************
Summary:
Set the input mute state
***************************************************************************/
void BAPE_MixerInput_P_SetMute(
    BAPE_MixerInput input,
    bool muted
    );

/***************************************************************************
Summary:
Helpers to get ringbuffer register addresses from source channel id
***************************************************************************/
#define BAPE_P_SOURCE_CHANNEL_TO_RDADDR_REG(id) (BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR + ((BCHP_AUD_FMM_BF_CTRL_RINGBUF_2_RDADDR-BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR)*((uint32_t)(id))))
#define BAPE_P_SOURCE_CHANNEL_TO_WRADDR_REG(id) (BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR + ((BCHP_AUD_FMM_BF_CTRL_RINGBUF_2_WRADDR-BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR)*((uint32_t)(id))))
#define BAPE_P_SOURCE_CHANNEL_TO_BASEADDR_REG(id) (BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_BASEADDR + ((BCHP_AUD_FMM_BF_CTRL_RINGBUF_2_BASEADDR-BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_BASEADDR)*((uint32_t)(id))))
#define BAPE_P_SOURCE_CHANNEL_TO_ENDADDR_REG(id) (BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_ENDADDR + ((BCHP_AUD_FMM_BF_CTRL_RINGBUF_2_ENDADDR-BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_ENDADDR)*((uint32_t)(id))))
#define BAPE_P_SOURCE_CHANNEL_TO_FREEFULL_REG(id) (BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_FREEFULL_MARK + ((BCHP_AUD_FMM_BF_CTRL_RINGBUF_2_FREEFULL_MARK-BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_FREEFULL_MARK)*((uint32_t)(id))))
#define BAPE_P_SOURCE_CHANNEL_TO_WRPOINT_REG(id) (BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_START_WRPOINT + ((BCHP_AUD_FMM_BF_CTRL_RINGBUF_2_START_WRPOINT-BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_START_WRPOINT)*((uint32_t)(id))))

/***************************************************************************
Summary:
Helpers to get ringbuffer register addresses from destination channel id
***************************************************************************/
#define BAPE_P_DEST_CHANNEL_TO_RDADDR_REG(id) (BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR + ((BCHP_AUD_FMM_BF_CTRL_RINGBUF_2_RDADDR-BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR)*((uint32_t)(id)+BAPE_CHIP_MAX_SOURCE_CHANNELS)))
#define BAPE_P_DEST_CHANNEL_TO_WRADDR_REG(id) (BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR + ((BCHP_AUD_FMM_BF_CTRL_RINGBUF_2_WRADDR-BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR)*((uint32_t)(id)+BAPE_CHIP_MAX_SOURCE_CHANNELS)))
#define BAPE_P_DEST_CHANNEL_TO_BASEADDR_REG(id) (BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_BASEADDR + ((BCHP_AUD_FMM_BF_CTRL_RINGBUF_2_BASEADDR-BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_BASEADDR)*((uint32_t)(id)+BAPE_CHIP_MAX_SOURCE_CHANNELS)))
#define BAPE_P_DEST_CHANNEL_TO_ENDADDR_REG(id) (BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_ENDADDR + ((BCHP_AUD_FMM_BF_CTRL_RINGBUF_2_ENDADDR-BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_ENDADDR)*((uint32_t)(id)+BAPE_CHIP_MAX_SOURCE_CHANNELS)))
#define BAPE_P_DEST_CHANNEL_TO_FREEFULL_REG(id) (BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_FREEFULL_MARK + ((BCHP_AUD_FMM_BF_CTRL_RINGBUF_2_FREEFULL_MARK-BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_FREEFULL_MARK)*((uint32_t)(id)+BAPE_CHIP_MAX_SOURCE_CHANNELS)))
#define BAPE_P_DEST_CHANNEL_TO_WRPOINT_REG(id) (BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_START_WRPOINT + ((BCHP_AUD_FMM_BF_CTRL_RINGBUF_2_START_WRPOINT-BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_START_WRPOINT)*((uint32_t)(id)+BAPE_CHIP_MAX_SOURCE_CHANNELS)))

/***************************************************************************
Summary:
Init the L3 interrupt handlers
***************************************************************************/
BERR_Code BAPE_P_InitInterrupts(
    BAPE_Handle handle
    );

/***************************************************************************
Summary:
Uninit the L3 interrupt handlers
***************************************************************************/
void BAPE_P_UninitInterrupts(
    BAPE_Handle handle
    );

/***************************************************************************
Summary:
Enable/Disable a source channel freemark interrupt
***************************************************************************/
BERR_Code BAPE_P_SetSourceChannelFreemarkInterrupt(
    BAPE_Handle handle,
    unsigned sourceChannelId,
    BINT_CallbackFunc callback_isr,
    void *pParam1,
    int param2
    );

/***************************************************************************
Summary:
Enable/Disable a destination channel fullmark interrupt
***************************************************************************/
BERR_Code BAPE_P_SetDestinationChannelFullmarkInterrupt(
    BAPE_Handle handle,
    unsigned destChannelId,
    BINT_CallbackFunc callback_isr,
    void *pParam1,
    int param2
    );

/***************************************************************************
Summary:
Enable/Disable a destination channel overflow interrupt
***************************************************************************/
BERR_Code BAPE_P_SetDestinationChannelOverflowInterrupt(
    BAPE_Handle handle,
    unsigned destChannelId,
    BINT_CallbackFunc callback_isr,
    void *pParam1,
    int param2
    );

/***************************************************************************
Summary:
Helper to get a string for a sample rate code
***************************************************************************/
const char *BAPE_P_GetSampleRateString_isrsafe(BAVC_AudioSamplingRate sampleRate);

/***************************************************************************
Summary:
Helper to get a numeric value for sample rates (useful for > or < comparisons)
***************************************************************************/
unsigned BAPE_P_GetSampleRateValue_isrsafe(BAVC_AudioSamplingRate sampleRate);

/***************************************************************************
Summary:
Convert sample rate value to enum
***************************************************************************/
BAVC_AudioSamplingRate BAPE_P_GetSampleRateEnum_isrsafe(unsigned sampleRate);

/***************************************************************************
Summary:
Helper to get the SPDIF channel status code for a sample rate (IEC60958-3)
***************************************************************************/
unsigned BAPE_P_GetSampleRateCstatCode_isrsafe(BAVC_AudioSamplingRate sampleRate);

/***************************************************************************
Summary:
Initialize the SRC block
***************************************************************************/
BERR_Code BAPE_P_InitSrc(
    BAPE_Handle handle
    );

/***************************************************************************
Summary:
Helper to get a string for a sample rate code
***************************************************************************/
void BAPE_P_ProgramSRC_isr(BAPE_Handle, uint8_t srcId, BAVC_AudioSamplingRate inputRate, BAVC_AudioSamplingRate outputRate);

/***************************************************************************
Summary:
Attach a mixer to a PLL
***************************************************************************/
void BAPE_P_AttachMixerToPll(BAPE_MixerHandle mixer, BAPE_Pll pll);

/***************************************************************************
Summary:
Detach a mixer from a PLL
***************************************************************************/
void BAPE_P_DetachMixerFromPll(BAPE_MixerHandle mixer, BAPE_Pll pll);

/***************************************************************************
Summary:
Update PLL sample rate
***************************************************************************/
BERR_Code BAPE_P_UpdatePll_isr(BAPE_Handle handle, BAPE_Pll pll);

/***************************************************************************
Summary:
Allocate mixers from the resource pool.  Sets up cascade and grouping.
***************************************************************************/
BERR_Code BAPE_P_AllocateMixers(BAPE_MixerHandle mixer, unsigned numOutputs, unsigned numChannelPairs);

/***************************************************************************
Summary:
Allocate Input Buffers from the resource pool
***************************************************************************/
BERR_Code BAPE_P_AllocateInputBuffers(BAPE_Handle handle, BAPE_MixerInput input);

/***************************************************************************
Summary:
Allocate Input Resources for inputs that branch at SRC
***************************************************************************/
BERR_Code BAPE_P_AllocateInputResources(BAPE_Handle handle, BAPE_MixerInput input);

/***************************************************************************
Summary:
Allocate Input Link Resources for inputs that use multiple Source Channels
***************************************************************************/
BERR_Code BAPE_P_AllocateInputLinkResources(BAPE_Handle handle, BAPE_MixerInputLink *pLink);

/***************************************************************************
Summary:
Release mixers and playbacks to the resource pool
***************************************************************************/
void BAPE_P_FreeMixers(BAPE_MixerHandle mixer);

/***************************************************************************
Summary:
Free Input Buffers to the resource pool
***************************************************************************/
void BAPE_P_FreeInputBuffers(BAPE_Handle handle, BAPE_MixerInput input);

/***************************************************************************
Summary:
Free Input Resources for inputs that branch at SRC
***************************************************************************/
void BAPE_P_FreeInputResources(BAPE_Handle handle, BAPE_MixerInput input);

/***************************************************************************
Summary:
Free Input Link Resources for inputs that use multiple Source Channels
***************************************************************************/
void BAPE_P_FreeInputLinkResources(BAPE_Handle handle, BAPE_MixerInputLink *pLink);

/***************************************************************************
Summary:
Microsequencer FW version
***************************************************************************/
extern const uint32_t g_BAPE_MS_FirmwareVersion[4];

/***************************************************************************
Summary:
Microsequencer FW
***************************************************************************/
extern const uint32_t g_BAPE_MS_Firmware[BCHP_AUD_FMM_MS_CTRL_USEQ_INSTi_ARRAY_END + 1];

/***************************************************************************
Summary:
Load image into the preallocated memory and returns an entry point
***************************************************************************/
BERR_Code BAPE_P_Image_Load(BAPE_Handle handle, BAPE_Img_Id id, void *mem, size_t memSize, size_t *memUsed, void **pEntry);
BERR_Code BAPE_P_Image_Run(BAPE_Handle handle, void *mem, size_t memSize, size_t memUsed, void *pEntry);
void BAPE_P_Image_Reset(BAPE_Handle handle);

/***************************************************************************
Summary:
Decoder Handle
***************************************************************************/
typedef struct BAPE_Decoder
{
    BDBG_OBJECT(BAPE_Decoder)
    BAPE_Handle deviceHandle;
    bool independentDelay;
    bool passthrough;
    unsigned index;
    BAPE_MixerInputObject stereoConnector;
    BAPE_MixerInputObject multichannelConnector;
    BAPE_MixerInputObject compressedConnector;
    BAPE_DspContextHandle dspContext;
    char stereoName[13]; /* Decoder %d %c */
    char multichannelName[13]; /* Decoder %d %c */
    char compressedName[13]; /* Decoder %d %c */
    bool stereoOutputs, multichannelOutputs, compressedOutputs;
    BAPE_DecoderState state;
    BAPE_DecoderSettings settings;
    BAPE_DecoderStartSettings startSettings;
    BAPE_DecoderTsmSettings tsmSettings;    
    BAPE_DecoderCodecSettings ac3Settings, ac3PlusSettings;
    BAPE_DecoderCodecSettings aacSettings, aacPlusSettings;
    BAPE_DecoderInterruptHandlers interrupts;
} BAPE_Decoder;

/***************************************************************************
Summary:
Set a decoder's sample rate
***************************************************************************/
void BAPE_Decoder_P_SetSampleRate_isr(BAPE_DecoderHandle handle, BAVC_AudioSamplingRate sampleRate);

/***************************************************************************
Summary:
Get the address of a mixer's config register
***************************************************************************/
uint32_t BAPE_P_GetMixerConfigAddress(unsigned mixerId);

/***************************************************************************
Summary:
Invalid Fs handle
***************************************************************************/
#define BAPE_FS_INVALID ((unsigned)-1)

/***************************************************************************
Summary:
Allocate an Fs timing source
***************************************************************************/
unsigned BAPE_P_AllocateFs(BAPE_Handle handle);

/***************************************************************************
Summary:
Free an Fs timing source
***************************************************************************/
void BAPE_P_FreeFs(BAPE_Handle handle, unsigned fs);

/***************************************************************************
Summary:
Set the timing parameters for an Fs timing source
***************************************************************************/
void BAPE_P_SetFsTiming_isr(
    BAPE_Handle handle, 
    unsigned fs, 
    BAPE_MclkSource clkSource, 
    unsigned multiplier
    );

#endif /* #ifndef BAPE_PRIV_H_ */

