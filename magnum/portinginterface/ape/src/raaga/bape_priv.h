/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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

#ifndef BAPE_PRIV_H_
#define BAPE_PRIV_H_

#include "bstd.h"
#include "bkni.h"
#include "bape.h"
#include "blst_slist.h"
#include "blst_squeue.h"
#include "bape_chip_priv.h"
#include "bape_reg_priv.h"
#include "blst_queue.h"
#if defined BCHP_HDMI_RCVR_CTRL_REG_START
#include "bchp_hdmi_rcvr_ctrl.h"
#elif defined BCHP_AUD_FMM_IOP_IN_HDMI_0_REG_START
#include "bchp_aud_fmm_iop_in_hdmi_0.h"
#endif
#if defined BCHP_SPDIF_RCVR_CTRL_REG_START
#include "bchp_spdif_rcvr_ctrl.h"
#include "bchp_spdif_rcvr_esr.h"
#include "bchp_hdmi_rcvr_ctrl.h"
#include "bchp_hdmi_rcvr_esr.h"
#elif defined BCHP_SPDIF_RCVR_CTRL0_REG_START
#include "bchp_spdif_rcvr_ctrl0.h"
#include "bchp_spdif_rcvr_ctrl1.h"
#include "bchp_spdif_rcvr_esr0.h"
#include "bchp_spdif_rcvr_esr1.h"
#elif defined BCHP_AUD_FMM_IOP_IN_SPDIF_0_REG_START
#include "bchp_aud_fmm_iop_in_spdif_0.h"
#endif

/* Debug objects */
BDBG_OBJECT_ID_DECLARE(BAPE_Device);
BDBG_OBJECT_ID_DECLARE(BAPE_BufferNode);
BDBG_OBJECT_ID_DECLARE(BAPE_Decoder);
BDBG_OBJECT_ID_DECLARE(BAPE_Mixer);
BDBG_OBJECT_ID_DECLARE(BAPE_MixerInputCapture);
BDBG_OBJECT_ID_DECLARE(BAPE_OutputPort);
BDBG_OBJECT_ID_DECLARE(BAPE_PathConnector);
BDBG_OBJECT_ID_DECLARE(BAPE_PathConnection);
BDBG_OBJECT_ID_DECLARE(BAPE_InputPort);
BDBG_OBJECT_ID_DECLARE(BAPE_PathNode);
BDBG_OBJECT_ID_DECLARE(BAPE_Playback);
BDBG_OBJECT_ID_DECLARE(BAPE_InputCapture);
BDBG_OBJECT_ID_DECLARE(BAPE_EchoCanceller);

/* Shorthand to indicate the number of elements in an array. */
#define BAPE_P_NUM_ELEMS(x)  (sizeof(x) / sizeof (x)[0])
#define BAPE_P_MAX(A,B) ((A)>(B)?(A):(B))

/***************************************************************************
Summary:
A macro to compare two values and print a BDBG message that contains one
value if they match, or both values if they differ.

For example, this code:
    BAPE_LOG_CHANGE(BDBG_LOG, "  Sample Rate     ", "%u",
                        oldSampleRate, oldSampleRate,
                        newSampleRate, newSampleRate );

Will print this if both oldSampleRate and newSampleRate are 48000.
    00:00:01.003 bape_fmt_priv:   Sample Rate     : 48000

And will print this if oldSampleRate is 48000 and newSampleRate is 768000:
    00:00:01.003 bape_fmt_priv:   Sample Rate     : 48000 -> 768000

***************************************************************************/
#define BAPE_LOG_CHANGE(bdbg_log_macro, heading, pfmt, oldval, oldprint, newval, newprint)           \
        (  (oldval) != (newval)                                                                      \
            ?      bdbg_log_macro(("%s: " pfmt " -> " pfmt , heading,    (oldprint), (newprint )))   \
            :      bdbg_log_macro(("%s: " pfmt, heading, (newprint) ))                               \
        )

/***************************************************************************
Summary:
PLL Status
***************************************************************************/
typedef struct BAPE_AudioPll
{
    unsigned baseSampleRate;
    unsigned freqCh1;
    BLST_S_HEAD(PllMixerList, BAPE_Mixer) mixerList;    /* List of mixers using this PLL */
    BAPE_PllSettings settings;
} BAPE_AudioPll;

/***************************************************************************
Summary:
PLL Status
***************************************************************************/
typedef struct BAPE_AudioNco
{
    unsigned baseSampleRate;
    unsigned long ncoFreq;
    BAVC_Timebase timebase;
    BLST_S_HEAD(NcoMixerList, BAPE_Mixer) mixerList;    /* List of mixers using this NCO */
} BAPE_AudioNco;

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
Invalid DSP Mixer Input
***************************************************************************/
#define BAPE_DSPMIXER_INPUT_INVALID ((unsigned)-1)

/***************************************************************************
Summary:
Invalid Branch/Stage ID's
***************************************************************************/
#define BAPE_BRANCH_ID_INVALID ((unsigned)-1)
#define BAPE_STAGE_ID_INVALID ((unsigned)-1)

/***************************************************************************
Summary:
Invalid DSP ID
***************************************************************************/
#define BAPE_DSP_ID_INVALID ((unsigned)-1)

/***************************************************************************
Summary:
FCI ID Type
***************************************************************************/
typedef uint16_t BAPE_FciId;

/***************************************************************************
Summary:
Invalid FCI ID
***************************************************************************/
#define BAPE_FCI_ID_INVALID (0x3ff)

/***************************************************************************
Summary:
FCI ID Group
***************************************************************************/
typedef struct BAPE_FciIdGroup
{
    BAPE_FciId ids[BAPE_ChannelPair_eMax];
} BAPE_FciIdGroup;

/***************************************************************************
Summary:
Init FCI ID Group
***************************************************************************/
#define BAPE_FciIdGroup_Init(pGroup) \
do \
{ \
    unsigned i; \
    for ( i = 0; i < BAPE_ChannelPair_eMax; i++ ) \
    { \
        (pGroup)->ids[i] = BAPE_FCI_ID_INVALID; \
    } \
} while (0)

/***************************************************************************
Summary:
Determine number of active channel pairs in an FCI group
***************************************************************************/
#define BAPE_FciIdGroup_GetNumChannelPairs(group) (((group).ids[3] != BAPE_FCI_ID_INVALID)?4:((group).ids[2] != BAPE_FCI_ID_INVALID)?3:((group).ids[1] != BAPE_FCI_ID_INVALID)?2:((group).ids[0] != BAPE_FCI_ID_INVALID)?1:0)

/***************************************************************************
Summary:
Determine number of active channel pairs in an FCI group
***************************************************************************/
#define BAPE_FciIdGroup_IsEqual(group1,group2) (((group1).ids[0] == (group2).ids[0]) && ((group1).ids[1] == (group2).ids[1]) && ((group1).ids[2] == (group2).ids[2]) && ((group1).ids[3] == (group2).ids[3]))

/***************************************************************************
Summary:
Source FIFO Group Handle
***************************************************************************/
typedef struct BAPE_SfifoGroup *BAPE_SfifoGroupHandle;

/***************************************************************************
Summary:
Destination FIFO Group Handle
***************************************************************************/
typedef struct BAPE_DfifoGroup *BAPE_DfifoGroupHandle;

/***************************************************************************
Summary:
Sample Rate Converter Group Handle
***************************************************************************/
typedef struct BAPE_SrcGroup *BAPE_SrcGroupHandle;

/***************************************************************************
Summary:
Sample Rate Converter Group Handle
***************************************************************************/
typedef struct BAPE_SrcMemory *BAPE_SrcMemoryHandle;

/***************************************************************************
Summary:
Mixer Group Handle
***************************************************************************/
typedef struct BAPE_MixerGroup *BAPE_MixerGroupHandle;

/***************************************************************************
Summary:
Loopback Group Handle
***************************************************************************/
typedef struct BAPE_LoopbackGroup *BAPE_LoopbackGroupHandle;

/***************************************************************************
Summary:
Dummysink Group Handle
***************************************************************************/
typedef struct BAPE_DummysinkGroup *BAPE_DummysinkGroupHandle;

/***************************************************************************
Summary:
FCI Splitter Group Handle
***************************************************************************/
typedef struct BAPE_FciSplitterGroup *BAPE_FciSplitterGroupHandle;

/***************************************************************************
Summary:
FCI Splitter Group Handle
***************************************************************************/
typedef struct BAPE_FciSplitterOutputGroup *BAPE_FciSplitterOutputGroupHandle;

/***************************************************************************
Summary:
Decoder State
***************************************************************************/
typedef enum BAPE_DecoderState
{
    BAPE_DecoderState_eStopped,             /* Not running */
    BAPE_DecoderState_eStarting,            /* Preparing to start */
    BAPE_DecoderState_eStarted,             /* Running in normal operation */
    BAPE_DecoderState_ePaused,              /* Running, paused */
    BAPE_DecoderState_eDisabled,            /* Prepared for flush from started state */
    BAPE_DecoderState_eDisabledPaused,      /* Prepared for flush from paused state */
    BAPE_DecoderState_eFrozen,              /* Decoder STC/output frozen */
    BAPE_DecoderState_eMax
} BAPE_DecoderState;

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
    bool allocated;
    uint8_t poolIndex;
} BAPE_BufferNode;

#include "bape_fmt_priv.h"

/***************************************************************************
Summary:
Device Handle
***************************************************************************/
#define BAPE_MAX_BUFFER_POOLS (4)
typedef struct BAPE_Device
{
    BDBG_OBJECT(BAPE_Device)

    /* Open Parameters */
    BCHP_Handle chpHandle;
    BREG_Handle regHandle;
    BMEM_Handle memHandle;
    BINT_Handle intHandle;
    BTMR_Handle tmrHandle;
    BDSP_Handle dspHandle;
    BDSP_Handle armHandle;
    BAPE_Settings settings;

    /* Ramp Step Sizes */
    uint32_t outputVolumeRampStep;
    uint32_t sampleRateConverterRampStep;

    /* Buffer Lists */
    struct
    {
        BLST_S_HEAD(FreeBufferList, BAPE_BufferNode) freeList;
        BLST_S_HEAD(AllocatedBufferList, BAPE_BufferNode) allocatedList;
        BAPE_FMT_Capabilities capabilities;
        unsigned bufferSize, numFreeBuffers;
#if BDBG_DEBUG_BUILD
        unsigned numBuffers, maxUsed;
#endif
    } buffers[BAPE_MAX_BUFFER_POOLS];

    /* Software resource allocation */
#if BAPE_CHIP_MAX_DECODERS > 0
    BDSP_ContextHandle       dspContext;
    BDSP_ContextHandle       armContext;
    unsigned                 numDsps;
    unsigned                 numArms;
    BAPE_DecoderHandle       decoders[BAPE_CHIP_MAX_DECODERS];
    struct
    {
        BAPE_DecoderState state;
        BAPE_DecoderStartSettings startSettings;
    } decoderWatchdogInfo[BAPE_CHIP_MAX_DECODERS];
#endif

#if BAPE_CHIP_MAX_INPUT_CAPTURES > 0
    BAPE_InputCaptureHandle  inputCaptures[BAPE_CHIP_MAX_INPUT_CAPTURES];
    struct
    {
        bool restartRequired;
        BAPE_InputCaptureStartSettings startSettings;
    } inputCaptureWatchdogInfo[BAPE_CHIP_MAX_INPUT_CAPTURES];
#endif
#if BAPE_CHIP_MAX_PLAYBACKS > 0
    BAPE_PlaybackHandle      playbacks[BAPE_CHIP_MAX_PLAYBACKS];
    struct
    {
        bool restartRequired;
        BAPE_PlaybackStartSettings startSettings;
    } playbackWatchdogInfo[BAPE_CHIP_MAX_PLAYBACKS];
#endif
#if BAPE_CHIP_MAX_DACS > 0
    BAPE_DacHandle           dacs[BAPE_CHIP_MAX_DACS];
#endif
#if BAPE_CHIP_MAX_I2S_OUTPUTS > 0
    BAPE_I2sOutputHandle     i2sOutputs[BAPE_CHIP_MAX_I2S_OUTPUTS];
#endif
#if BAPE_CHIP_MAX_I2S_MULTI_OUTPUTS > 0
    BAPE_I2sMultiOutputHandle     i2sMultiOutputs[BAPE_CHIP_MAX_I2S_MULTI_OUTPUTS];
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
#if BAPE_CHIP_MAX_DUMMYSINKS > 0
    BAPE_DummyOutputHandle     dummyOutputs[BAPE_CHIP_MAX_DUMMYSINKS];
#endif
#if BAPE_CHIP_MAX_I2S_INPUTS > 0
    BAPE_I2sInputHandle      i2sInputs[BAPE_CHIP_MAX_I2S_INPUTS];
#endif
#if BAPE_CHIP_MAX_SPDIF_INPUTS > 0
    BAPE_SpdifInputHandle    spdifInputs[BAPE_CHIP_MAX_SPDIF_INPUTS];
#endif
#if BAPE_CHIP_MAX_MAI_INPUTS > 0
    BAPE_MaiInputHandle      maiInputs[BAPE_CHIP_MAX_MAI_INPUTS];
#endif
#if BAPE_CHIP_MAX_AUDIO_RETURN_CHANNELS > 0
    BAPE_AudioReturnChannelHandle      audioReturnChannels[BAPE_CHIP_MAX_AUDIO_RETURN_CHANNELS];
#endif
#if BAPE_CHIP_MAX_RFMODS > 0
    BAPE_RfModHandle         rfmods[BAPE_CHIP_MAX_RFMODS];
#endif
#if BAPE_CHIP_MAX_CRCS > 0
    BAPE_CrcHandle         crcs[BAPE_CHIP_MAX_CRCS];
#endif



    BLST_S_HEAD(MixerList, BAPE_Mixer) mixerList;
    BLST_S_HEAD(MuxOutputList, BAPE_MuxOutput) muxOutputList;

#if BAPE_CHIP_MAX_PLLS > 0
    /* PLL Status */
    BAPE_AudioPll   audioPlls[BAPE_CHIP_MAX_PLLS];
#endif
#if BAPE_CHIP_MAX_NCOS > 0
    /* PLL Status */
    BAPE_AudioNco   audioNcos[BAPE_CHIP_MAX_NCOS];
#endif

    /* Groups */
    BAPE_SfifoGroupHandle     sfifoGroups[BAPE_CHIP_MAX_SFIFO_GROUPS];
    BAPE_DfifoGroupHandle     dfifoGroups[BAPE_CHIP_MAX_DFIFO_GROUPS];
    BAPE_SrcGroupHandle       srcGroups[BAPE_CHIP_MAX_SRC_GROUPS];
    BAPE_MixerGroupHandle     mixerGroups[BAPE_CHIP_MAX_MIXER_GROUPS];
#if BAPE_CHIP_MAX_LOOPBACK_GROUPS > 0
    BAPE_LoopbackGroupHandle  loopbackGroups[BAPE_CHIP_MAX_LOOPBACK_GROUPS];
#endif
#if BAPE_CHIP_MAX_DUMMYSINK_GROUPS > 0
    BAPE_DummysinkGroupHandle dummysinkGroups[BAPE_CHIP_MAX_DUMMYSINK_GROUPS];
#endif
#if BAPE_CHIP_MAX_FCISPLITTER_GROUPS > 0
    BAPE_FciSplitterGroupHandle     fciSplitterGroups[BAPE_CHIP_MAX_FCISPLITTER_GROUPS];
#endif
#if BAPE_CHIP_MAX_FCISPLITTER_OUTPUT_GROUPS > 0
    BAPE_FciSplitterOutputGroupHandle     fciSplitterOutputGroups[BAPE_CHIP_MAX_FCISPLITTER_OUTPUT_GROUPS];
#endif

    /* SRC Memory Heaps (DTV-Only) */
    BMEM_Heap_Handle srcCoefHeap;
    BMEM_Heap_Handle srcScratchHeap;

    /* Interrupts */
    BINT_CallbackHandle isrBfEsr1;
    BINT_CallbackHandle isrBfEsr2;
    BINT_CallbackHandle isrBfEsr3;
    BINT_CallbackHandle isrBfEsr4;
    BAPE_L3Interrupt sourceRbufFreemark[BAPE_CHIP_MAX_SFIFOS];
    BAPE_L3Interrupt destRbufFullmark[BAPE_CHIP_MAX_DFIFOS];
    BAPE_L3Interrupt destRbufOverflow[BAPE_CHIP_MAX_DFIFOS];

    /* Hardware resource allocation map */
    bool sfifoAllocated[BAPE_CHIP_MAX_SFIFOS];
    bool dfifoAllocated[BAPE_CHIP_MAX_DFIFOS];
    bool srcAllocated[BAPE_CHIP_MAX_SRCS];
#ifdef BAPE_CHIP_MAX_SRC_COEFF_CHUNKS
    bool srcCoeffAllocated[BAPE_CHIP_MAX_SRC_COEFF_CHUNKS];
#endif
    bool mixerAllocated[BAPE_CHIP_MAX_MIXERS];
    bool playbackAllocated[BAPE_CHIP_MAX_MIXER_PLAYBACKS];
    bool dummysinkAllocated[BAPE_CHIP_MAX_DUMMYSINKS];
#if BAPE_CHIP_MAX_LOOPBACKS > 0
    bool loopbackAllocated[BAPE_CHIP_MAX_LOOPBACKS];
#endif
#if BAPE_CHIP_MAX_FS > 0
    bool fsAllocated[BAPE_CHIP_MAX_FS];
#endif
    bool adaptRateAllocated[BAPE_CHIP_MAX_ADAPTRATE_CONTROLLERS];
#if BAPE_CHIP_MAX_CRCS > 0
    bool crcAllocated[BAPE_CHIP_MAX_CRCS];
#endif
#if BAPE_CHIP_MAX_FCI_SPLITTERS > 0
    bool fciSplitterAllocated[BAPE_CHIP_MAX_FCI_SPLITTERS];
#endif
#if BAPE_CHIP_MAX_FCI_SPLITTER_OUTPUTS > 0
    bool fciSplitterOutputAllocated[BAPE_CHIP_MAX_FCI_SPLITTER_OUTPUTS];
#endif

    /* Usage Tracking for DP Playback */
    BAPE_FciId playbackFci[BAPE_CHIP_MAX_MIXER_PLAYBACKS];
    uint8_t playbackReferenceCount[BAPE_CHIP_MAX_MIXER_PLAYBACKS];

    /* Interrupts */
    BAPE_InterruptHandlers interrupts;

    bool bStandby;

#if BAPE_CHIP_MAX_PLLS > 0
    BTMR_TimerHandle pllTimer[BAPE_CHIP_MAX_PLLS];
#endif

} BAPE_Device;

/***************************************************************************
Summary:
Generic Mixer Input Structure
***************************************************************************/
typedef struct BAPE_PathConnector
{
    BDBG_OBJECT(BAPE_PathConnector)
    struct BAPE_PathNode *pParent;

    /* Data Format Descriptor */
    BAPE_FMT_Descriptor format;
    bool useBufferPool;                             /* True if buffers should be pulled from the buffer pool */

#if BAPE_DSP_SUPPORT
    /* FW Source Information */
    BDSP_StageHandle hStage;                        /* Data source stage */
#endif

    /* Links to downstream connections */
    BLST_SQ_HEAD(ConnectionList, BAPE_PathConnection) connectionList;
    /* Buffer resources if useBufferBool is true */
    BAPE_BufferNode *pBuffers[BAPE_ChannelPair_eMax];
    /* Name */
    const char *pName;
} BAPE_PathConnector;

/***************************************************************************
Summary:
Init a connector
***************************************************************************/
#define BAPE_P_InitPathConnector(input, connectorParent) \
   do {\
         BKNI_Memset((input),0,sizeof(BAPE_PathConnector));\
         BDBG_OBJECT_SET((input),BAPE_PathConnector);\
         (input)->pParent=(connectorParent);\
         BLST_SQ_INIT(&(input)->connectionList); \
         BAPE_FMT_P_InitDescriptor(&(input)->format); \
         (input)->pName = ""; \
   } while(0)

/***************************************************************************
Summary:
Path Node Type
***************************************************************************/
typedef enum BAPE_PathNodeType
{
    BAPE_PathNodeType_eDecoder,                     /* subtype=BAVC_AudioCompressionStd */
    BAPE_PathNodeType_ePlayback,
    BAPE_PathNodeType_eInputCapture,
    BAPE_PathNodeType_ePostProcessor,               /* subtype=BAPE_PostProcessorType */
    BAPE_PathNodeType_eEncoder,                     /* subtype=BAVC_AudioCompressionStd */
    BAPE_PathNodeType_eRfEncoder,                   /* subtype=BAPE_RfAudioEncoding */
    BAPE_PathNodeType_eMixer,                       /* subtype=BAPE_MixerType */
    BAPE_PathNodeType_eEqualizer,
    BAPE_PathNodeType_eMuxOutput,
    BAPE_PathNodeType_eEchoCanceller,               /* subtype=BAPE_EchoCancellerAlgorithm */
    BAPE_PathNodeType_eMax
} BAPE_PathNodeType;

/***************************************************************************
Summary:
Path Node State
***************************************************************************/
typedef enum BAPE_PathNodeState
{
    BAPE_PathNodeState_eIdle,
    BAPE_PathNodeState_eAllocatingPath,
    BAPE_PathNodeState_eAllocatedPath,
    BAPE_PathNodeState_eConfiguredPath,
    BAPE_PathNodeState_eStarted,
    BAPE_PathNodeState_eMax
} BAPE_PathNodeState;

/***************************************************************************
Summary:
Maximum output connectors per node
***************************************************************************/
#define BAPE_MAX_CONNECTORS_PER_NODE (BAPE_ConnectorFormat_eMax)

/***************************************************************************
Summary:
Path Node
***************************************************************************/
typedef struct BAPE_PathNode
{
    BDBG_OBJECT(BAPE_PathNode)

    /* Node Specifics */
    BAPE_PathNodeType type;
    unsigned subtype;
    unsigned dspIndex;          /* Source Task DSP Index */
    void *pHandle;              /* Pointer to actual object handle */
    BAPE_Handle deviceHandle;
    const char *pName;

    /* Node Status */
    BAPE_PathNodeState state;
    unsigned numStarts;         /* Number of times start has been called.  Stop must be called the same number of times.  */
    bool orphan;                /* Set to true if there is no eventual data consumption from this node.  It will be ignored if true. */

    /* Output Connectors from this node */
    unsigned numConnectors;
    BAPE_PathConnector connectors[BAPE_MAX_CONNECTORS_PER_NODE];

    /* List of input connections to this node */
    BLST_S_HEAD(UpstreamList, BAPE_PathConnection) upstreamList;
    BAPE_FMT_Capabilities inputCapabilities;    /* Supported input formats */

    /* For InputPort consumerList */
    BLST_S_ENTRY(BAPE_PathNode) consumerNode;

    /* Inter-Node Callbacks */
    BERR_Code (*allocatePathFromInput)(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection);
    void      (*freePathFromInput)(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection);
    BERR_Code (*allocatePathToOutput)(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection);
    void      (*freePathToOutput)(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection);
    BERR_Code (*configPathFromInput)(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection);
    BERR_Code (*configPathToOutput)(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection);
    BERR_Code (*startPathFromInput)(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection);
    void      (*stopPathFromInput)(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection);
    BERR_Code (*startPathToOutput)(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection);
    void      (*stopPathToOutput)(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection);
    void      (*inputSampleRateChange_isr)(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection, unsigned newSampleRate);
    BERR_Code (*inputFormatChange)(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection, const BAPE_FMT_Descriptor *pNewFormat);
    void      (*inputMute)(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection, bool enabled);
    BERR_Code (*outputConnectionAdded)(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection);
    void      (*outputConnectionRemoved)(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection);

    /* InputPort->Node Callbacks */
    BERR_Code (*inputPortFormatChange_isr)(struct BAPE_PathNode *pNode, BAPE_InputPort port);

    /* Cleanup Callback */
    void      (*removeInput)(struct BAPE_PathNode *pNode, BAPE_PathConnector *pConnector);
} BAPE_PathNode;

/***************************************************************************
Summary:
Init a connector
***************************************************************************/
#define BAPE_P_InitPathNode(node, nodetype, nodesubtype, nConnectors, devHandle, handle) \
   do {\
         unsigned i;\
         BKNI_Memset((node),0,sizeof(BAPE_PathNode));\
         BDBG_OBJECT_SET((node),BAPE_PathNode);\
         (node)->pHandle=(handle);\
         (node)->type=(nodetype);\
         (node)->subtype=(nodesubtype);\
         (node)->numConnectors=(nConnectors);\
         (node)->deviceHandle=(devHandle);\
         (node)->dspIndex = BAPE_DSP_ID_INVALID;\
         BDBG_OBJECT_ASSERT((devHandle), BAPE_Device);\
         for ( i = 0; i < (nConnectors); i++ ) \
         { \
             BAPE_P_InitPathConnector(&((node)->connectors[i]), (node)); \
         } \
         BLST_S_INIT(&(node)->upstreamList); \
         BAPE_FMT_P_InitCapabilities(&(node)->inputCapabilities, NULL, NULL); \
   } while(0)

/***************************************************************************
Summary:
Output Path Status
***************************************************************************/
typedef struct BAPE_PathNodeOutputStatus
{
    unsigned numConnectors;
    struct
    {
        unsigned directConnections;      /* Number of connections at the first level */
        unsigned totalConnections;       /* Total number of downstream connections */
        unsigned totalBranches;          /* Total number of downstream branches */
        unsigned numValidOutputs;        /* Total number of non-orphan output nodes */
        unsigned numOutputs[BAPE_DataType_eMax];  /* Total number of outputs of each data type from this node. */
    } connectorStatus[BAPE_MAX_CONNECTORS_PER_NODE];

    unsigned activeConnectors;       /* Number of active connectors */
    unsigned directConnections;      /* Number of connections at the first level of all connectors */
    unsigned totalConnections;       /* Total number of downstream connections including all connectors */
    unsigned totalBranches;          /* Total number of downstream branches including all connectors */
    unsigned numValidOutputs;        /* Total number of non-orphan output nodes */
    unsigned numOutputs[BAPE_DataType_eMax];  /* Total number of outputs of each data type from this node. */
} BAPE_PathNodeOutputStatus;

/***************************************************************************
Summary:
Invalid Stream ID
***************************************************************************/
#define BAPE_STREAM_ID_INVALID (0xff)

/***************************************************************************
Summary:
Connection from an upstream connector to a downstream node
***************************************************************************/
typedef struct BAPE_PathConnection
{
    BDBG_OBJECT(BAPE_PathConnection)
    BAPE_PathConnector *pSource;
    BAPE_PathNode *pSink;
    BLST_S_ENTRY(BAPE_PathConnection) upstreamNode;
    BLST_SQ_ENTRY(BAPE_PathConnection) downstreamNode;

    /* Are the resources allocated? */
    bool resourcesAllocated;

    /* Data format descriptor */
    BAPE_FMT_Descriptor format;

    /* FMM Input Details */
    BAPE_FciIdGroup inputFciGroup;

    /* FMM Resources */
    BAPE_SfifoGroupHandle       sfifoGroup;
    BAPE_SrcGroupHandle         srcGroup;

#if BAPE_DSP_SUPPORT
    /* Buffers for DSP mixer/Echo Canceller connections */
    BDSP_InterTaskBufferHandle hInterTaskBuffer;

    /* Input Index for DSP mixer */
    unsigned dspInputIndex;


    /* DSP output ID */
    unsigned dspOutputIndex;
#endif
} BAPE_PathConnection;

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
    BAPE_MclkSource_eNco0,
    BAPE_MclkSource_eNco1,
    BAPE_MclkSource_eNco2,
    BAPE_MclkSource_eNco3,
    BAPE_MclkSource_eNco4,
    BAPE_MclkSource_eNco5,
    BAPE_MclkSource_eNco6,
    BAPE_MclkSource_eMax
} BAPE_MclkSource;

#define BAPE_MCLKSOURCE_IS_PLL(mclkSource)  (mclkSource >= BAPE_MclkSource_ePll0     && mclkSource <= BAPE_MclkSource_ePll2)
#define BAPE_MCLKSOURCE_IS_DAC(mclkSource)  (mclkSource >= BAPE_MclkSource_eHifidac0 && mclkSource <= BAPE_MclkSource_eHifidac2)
#define BAPE_MCLKSOURCE_IS_NCO(mclkSource)  (mclkSource >= BAPE_MclkSource_eNco0     && mclkSource <= BAPE_MclkSource_eNco6)

/***************************************************************************
Summary:
Mixer Interface Structure to store callbacks for mixer functions.
***************************************************************************/
typedef struct BAPE_MixerInterface {
    void      (*destroy)            (BAPE_MixerHandle handle);
    BERR_Code (*start)              (BAPE_MixerHandle handle);
    void      (*stop)               (BAPE_MixerHandle handle);
    BERR_Code (*addInput)           (BAPE_MixerHandle handle, BAPE_Connector input, const BAPE_MixerAddInputSettings *pSettings);
    BERR_Code (*removeInput)        (BAPE_MixerHandle handle, BAPE_Connector input );
    BERR_Code (*removeAllInputs)    (BAPE_MixerHandle handle);
    BERR_Code (*addOutput)          (BAPE_MixerHandle handle, BAPE_OutputPort output );
    BERR_Code (*removeOutput)       (BAPE_MixerHandle handle, BAPE_OutputPort output );
    BERR_Code (*removeAllOutputs)   (BAPE_MixerHandle handle);
    BERR_Code (*getInputVolume)     (BAPE_MixerHandle handle, BAPE_Connector input, BAPE_MixerInputVolume *pVolume);
    BERR_Code (*setInputVolume)     (BAPE_MixerHandle handle, BAPE_Connector input, const BAPE_MixerInputVolume *pVolume);
    BERR_Code (*applyOutputVolume)  (BAPE_MixerHandle handle, BAPE_OutputPort output);
    BERR_Code (*setSettings)        (BAPE_MixerHandle hMixer, const BAPE_MixerSettings *pSettings);
} BAPE_MixerInterface ;

/***************************************************************************
Summary:
Mixer Structure
***************************************************************************/
typedef struct BAPE_Mixer
{
    BDBG_OBJECT(BAPE_Mixer)
    const BAPE_MixerInterface *interface;
    unsigned running;
    bool startedExplicitly;  /* true => mixer was started with BAPE_Mixer_Start()*/
    bool resourcesAllocated;
    bool restartPending;
    BAPE_MixerFormat explicitFormat;
    unsigned index;
    unsigned numMixerGroups;
    unsigned fs;
    unsigned numOutputs;
    unsigned numOutputConnections;  /* Number of output connections.  Used when connecting a standard mixer to another FMM node such as eq or mixer. */
    BAPE_PathNode pathNode;
    BAPE_MclkSource mclkSource;
    BAPE_MixerGroupHandle mixerGroups[BAPE_CHIP_MAX_MIXERS];
    BAPE_Handle deviceHandle;
    BAPE_MixerSettings settings;
    BAPE_Connector master;
    BAPE_Connector inputs[BAPE_CHIP_MAX_MIXER_INPUTS];
    BAPE_MixerInputCaptureHandle inputCaptures[BAPE_CHIP_MAX_MIXER_INPUTS];
    bool inputMuted[BAPE_CHIP_MAX_MIXER_INPUTS];
    bool inputRunning[BAPE_CHIP_MAX_MIXER_INPUTS];
    BAPE_MixerInputVolume inputVolume[BAPE_CHIP_MAX_MIXER_INPUTS];
    BAPE_CrcHandle crcs[BAPE_CHIP_MAX_MIXER_INPUTS];
#if BAPE_CHIP_MAX_DSP_MIXERS > 0
    bool taskStarted;
    BDSP_InterTaskBufferHandle hInterTaskBuffers[BAPE_CHIP_MAX_MIXER_INPUTS];
    BDSP_TaskHandle hTask;
    BDSP_StageHandle hMixerStage, hSrcStage;
    BDSP_AF_P_sOpSamplingFreq sampleRateMap;
    BAPE_MuxOutputHandle hMuxOutput;
    BAPE_BufferNode            *pLoopbackBuffers[BAPE_ChannelPair_eMax];
    BAPE_MixerGroupHandle       loopbackMixerGroup;
    BAPE_DfifoGroupHandle       loopbackDfifoGroup;
    BAPE_LoopbackGroupHandle    loopbackGroup;
    unsigned                    loopbackRunning;
    unsigned                    loopbackDspInput;
#endif
    BLST_S_HEAD(OutputList, BAPE_OutputPortObject) outputList;
    BLST_S_ENTRY(BAPE_Mixer) pllNode;
    BLST_S_ENTRY(BAPE_Mixer) ncoNode;
    BLST_S_ENTRY(BAPE_Mixer) node;
} BAPE_Mixer;

/***************************************************************************
Summary:
BAPE_MixerInputCapture
***************************************************************************/
typedef struct BAPE_MixerInputCapture
{
    BDBG_OBJECT(BAPE_MixerInputCapture)

#if BAPE_DSP_SUPPORT
    BDSP_AudioCaptureHandle hCapture;
#endif

    BAPE_MixerInputCaptureInterruptHandlers interrupts;

    /* Capture pointer info for all the output capture ports */
} BAPE_MixerInputCapture;

/***************************************************************************
Summary:
Output Port Types
***************************************************************************/
typedef enum BAPE_OutputPortType
{
    BAPE_OutputPortType_eI2sOutput,
    BAPE_OutputPortType_eI2sMultiOutput,
    BAPE_OutputPortType_eMaiOutput,
    BAPE_OutputPortType_eSpdifOutput,
    BAPE_OutputPortType_eDac,
    BAPE_OutputPortType_eDummyOutput,
    BAPE_OutputPortType_eOutputCapture,
    BAPE_OutputPortType_eMax
}BAPE_OutputPortType;

/***************************************************************************
Summary:
Output Enable/Disable params
***************************************************************************/
typedef struct BAPE_OutputPort_P_EnableParams
{
    struct
    {
        uint32_t address;
        uint32_t mask;
        uint32_t value;
    } chPairs[BAPE_ChannelPair_eMax];
} BAPE_OutputPort_P_EnableParams;

/***************************************************************************
Summary:
Generic Mixer Output Structure
***************************************************************************/
typedef struct BAPE_OutputPortObject
{
    BDBG_OBJECT(BAPE_OutputPort)
    BAPE_OutputPortType type;       /* type of output device this port belongs to */
    unsigned index;                 /* index with respect to type */
    void *pHandle;                  /* pointer to the specific output device */
    BAPE_MclkSource mclkOutput;     /* If this output generates an MCLK (e.g. DAC), which one is it? */
    bool muteInMixer;
    bool fsTiming;                  /* If this output slaves to an Fs source, this is true. */
    bool pllRequired;               /* If this output should always use a PLL this is true. */
    bool builtinNco;                /* If this is true, an output has its own NCO timing source (e.g. DAC). */

    BAPE_FMT_Capabilities capabilities; /* Input formats this output will accept */

    /* Primary APE_Mixer <-> Output Mapping */
    BAPE_MixerHandle mixer;                         /* NULL if not connected (set by mixer) */
    BLST_S_ENTRY(BAPE_OutputPortObject) node;       /* This is the node used in a mixer's linked list to manage a list of outputs (set by mixer) */

    BAPE_MixerGroupHandle sourceMixerGroup; /* (Set by mixer) */
    unsigned sourceMixerOutputIndex;        /* (Set by mixer) */
    BAPE_FciIdGroup sourceMixerFci;         /* (Set by mixer) */

    BAPE_OutputVolume volume;               /* (Set by mixer) */

    int32_t additionalGain;                 /* Additional attenuation or gain to be applied due to loudness equivalance mode
                                               RF - 0dB, +3dB, +4dB
                                               HDMI - 0dB, -7dB, -8dB
                                               SPDIF - 0dB, -7dB, -8dB
                                               OTHERS - 0dB */

    BAPE_CrcHandle crc;                     /* CRC, if non NULL will be synchronized to capture CRC data for this output */

    void (*setTimingParams_isr)(BAPE_OutputPort output, unsigned sampleRate, BAVC_Timebase timebase);
    void (*setMclk_isr)(BAPE_OutputPort output, BAPE_MclkSource mclkSource, unsigned pllChannel, unsigned mclkFreqToFsRatio);
    void (*setFs)(BAPE_OutputPort output, unsigned fsNum);
    void (*setMute)(BAPE_OutputPort output, bool muted, bool sync);
    BERR_Code (*enable)(BAPE_OutputPort output);
    void      (*disable)(BAPE_OutputPort output);
    void      (*getEnableParams)(BAPE_OutputPort output, bool enable, BAPE_OutputPort_P_EnableParams* params);
    const char *pName;
} BAPE_OutputPortObject;

/***************************************************************************
Summary:
Init an output port
***************************************************************************/
#define BAPE_P_InitOutputPort(output, typ, indx, handle) \
   do {\
         unsigned _idx;\
         BKNI_Memset((output),0,sizeof(BAPE_OutputPortObject));\
         BDBG_OBJECT_SET((output),BAPE_OutputPort);\
         (output)->type = typ; \
         (output)->index = indx; \
         (output)->pHandle=handle;\
         for ( _idx = 0; _idx < BAPE_Channel_eMax; _idx++ ) { (output)->volume.volume[_idx] = 0x800000; }\
            BAPE_FciIdGroup_Init(&(output)->sourceMixerFci); \
   } while(0)

/***************************************************************************
Summary:
Input Port Types
***************************************************************************/
typedef enum BAPE_InputPortType
{
    BAPE_InputPortType_eI2s,
    BAPE_InputPortType_eMai,
    BAPE_InputPortType_eSpdif,
    BAPE_InputPortType_eRf,
    BAPE_InputPortType_eAdc,
    BAPE_InputPortType_eMax
}BAPE_InputPortType;

/***************************************************************************
Summary:
Generic InputPort Structure
***************************************************************************/
typedef struct BAPE_InputPortObject
{
    BDBG_OBJECT(BAPE_InputPort)
    BAPE_InputPortType type;    /* type of output device this port belongs to */
    unsigned index;             /* index with respect to type */
    void *pHandle;              /* pointer to the specific output device */
    BLST_S_HEAD(InputConsumerList, BAPE_PathNode) consumerList;    /* List of Consumers of this Input Port */
    BAPE_FciSplitterGroupHandle fciSpGroup;           /* FCI splitter group handle - Available on certain 28nm and newer chips */
    bool consumerAttaching;                           /* True if a consumer is in process of attaching */
    uint16_t streamId[BAPE_ChannelPair_eMax];         /* Input stream ID */
    BAPE_FMT_Descriptor format;                       /* Output data format */
    void (*enable)(BAPE_InputPort  inputPort);        /* Called when consumer starts */
    void (*disable)(BAPE_InputPort inputPort);        /* Called when consumer stops */
    BERR_Code (*consumerAttached_isr)(BAPE_InputPort inputPort, BAPE_PathNode *pConsumer, BAPE_FMT_Descriptor *pFormat);       /* Called when a new consumer attaches */
    void      (*consumerDetached_isr)(BAPE_InputPort inputPort, BAPE_PathNode *pConsumer);       /* Called when a consumer detaches */
    bool halted;                /* true => Input halted due to unhandled format change. */
    const char *pName;
} BAPE_InputPortObject;

/***************************************************************************
Summary:
Init an input port
***************************************************************************/
#define BAPE_P_InitInputPort(input, typ, indx, handle) \
   do {\
         unsigned i;\
         BKNI_Memset((input),0,sizeof(BAPE_InputPortObject));\
         BDBG_OBJECT_SET((input),BAPE_InputPort);\
         (input)->type = typ; \
         (input)->index = indx; \
         (input)->pHandle=handle;\
         BAPE_FMT_P_InitDescriptor(&(input)->format); \
         for ( i = 0; i < BAPE_ChannelPair_eMax; i++ ) { (input)->streamId[i] = BAPE_STREAM_ID_INVALID; }\
   } while(0)

/***************************************************************************
Summary:
Get the input port format descriptor
***************************************************************************/
void BAPE_InputPort_P_GetFormat_isr(
    BAPE_InputPort inputPort,
    BAPE_FMT_Descriptor *pFormat    /* [out] */
    );

/***************************************************************************
Summary:
Update the input port format descriptor
***************************************************************************/
BERR_Code BAPE_InputPort_P_SetFormat_isr(
    BAPE_InputPort inputPort,
    const BAPE_FMT_Descriptor *pNewFormat
    );

/***************************************************************************
Summary:
Tell the input port to enter the "halted" state.
***************************************************************************/
BERR_Code BAPE_InputPort_P_Halt_isr(
    BAPE_InputPort inputPort
    );

/***************************************************************************
Summary:
Attach a consumer to an input port
***************************************************************************/
BERR_Code BAPE_InputPort_P_AttachConsumer(
    BAPE_InputPort inputPort,
    BAPE_PathNode *pConsumer,
    BAPE_FMT_Descriptor *pInputFormat   /* [out] Current format */
    );

/***************************************************************************
Summary:
Detach a consumer from an input port
***************************************************************************/
BERR_Code BAPE_InputPort_P_DetachConsumer(
    BAPE_InputPort inputPort,
    BAPE_PathNode *pConsumer
    );

/***************************************************************************
Summary:
Get FCI Ids for Input Ports
***************************************************************************/
void BAPE_InputPort_P_GetFciIds(
    BAPE_InputPort inputPort,
    BAPE_FciIdGroup *pFciGroup      /* [out] */
    );

/***************************************************************************
Summary:
Convert IEC-61937 Burst Preamble C to a codec type.
***************************************************************************/
void BAPE_InputPort_P_BurstPreambleToCodec_isr(
    uint32_t burstPreamble,
    BAVC_AudioCompressionStd *codec
    );

/***************************************************************************
Summary:
***************************************************************************/
bool BAPE_InputPort_P_HasConsumersAttached(BAPE_InputPort inputPort);

/***************************************************************************
Summary:
***************************************************************************/
bool BAPE_InputPort_P_ConsumerIsAttached(BAPE_InputPort inputPort, BAPE_PathNode * pConsumer);

#ifdef BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR
/***************************************************************************
Summary:
Rinbguffer Stride
***************************************************************************/
#define BAPE_P_RINGBUFFER_STRIDE (BCHP_AUD_FMM_BF_CTRL_RINGBUF_1_RDADDR-BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR)

/***************************************************************************
Summary:
Helpers to get ringbuffer register addresses from source channel id
***************************************************************************/
#define BAPE_P_SFIFO_TO_RINBGUFFER(id) (2*(id))
#define BAPE_P_SFIFO_TO_RDADDR_REG(id) (BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR + (2*BAPE_P_RINGBUFFER_STRIDE*((uint32_t)(id))))
#define BAPE_P_SFIFO_TO_WRADDR_REG(id) (BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR + (2*BAPE_P_RINGBUFFER_STRIDE*((uint32_t)(id))))
#define BAPE_P_SFIFO_TO_BASEADDR_REG(id) (BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_BASEADDR + (2*BAPE_P_RINGBUFFER_STRIDE*((uint32_t)(id))))
#define BAPE_P_SFIFO_TO_ENDADDR_REG(id) (BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_ENDADDR + (2*BAPE_P_RINGBUFFER_STRIDE*((uint32_t)(id))))
#define BAPE_P_SFIFO_TO_FREEFULL_REG(id) (BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_FREEFULL_MARK + (2*BAPE_P_RINGBUFFER_STRIDE*((uint32_t)(id))))
#define BAPE_P_SFIFO_TO_WRPOINT_REG(id) (BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_START_WRPOINT + (2*BAPE_P_RINGBUFFER_STRIDE*((uint32_t)(id))))

/***************************************************************************
Summary:
Helpers to get ringbuffer register addresses from destination channel id
***************************************************************************/
#define BAPE_P_DFIFO_TO_RINBGUFFER(id) ((2*(id)) + BAPE_P_SFIFO_TO_RINBGUFFER(BAPE_CHIP_MAX_SFIFOS))
#define BAPE_P_DFIFO_TO_RDADDR_REG(id) (BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR + (2*BAPE_P_RINGBUFFER_STRIDE*((uint32_t)(id)+BAPE_CHIP_MAX_SFIFOS)))
#define BAPE_P_DFIFO_TO_WRADDR_REG(id) (BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR + (2*BAPE_P_RINGBUFFER_STRIDE*((uint32_t)(id)+BAPE_CHIP_MAX_SFIFOS)))
#define BAPE_P_DFIFO_TO_BASEADDR_REG(id) (BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_BASEADDR + (2*BAPE_P_RINGBUFFER_STRIDE*((uint32_t)(id)+BAPE_CHIP_MAX_SFIFOS)))
#define BAPE_P_DFIFO_TO_ENDADDR_REG(id) (BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_ENDADDR + (2*BAPE_P_RINGBUFFER_STRIDE*((uint32_t)(id)+BAPE_CHIP_MAX_SFIFOS)))
#define BAPE_P_DFIFO_TO_FREEFULL_REG(id) (BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_FREEFULL_MARK + (2*BAPE_P_RINGBUFFER_STRIDE*((uint32_t)(id)+BAPE_CHIP_MAX_SFIFOS)))
#define BAPE_P_DFIFO_TO_WRPOINT_REG(id) (BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_START_WRPOINT + (2*BAPE_P_RINGBUFFER_STRIDE*((uint32_t)(id)+BAPE_CHIP_MAX_SFIFOS)))

#else
/* Newer 7429-style RDB */
/***************************************************************************
Summary:
Rinbguffer Stride
***************************************************************************/
#define BAPE_P_RINGBUFFER_STRIDE (BCHP_AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_1_RDADDR-BCHP_AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_RDADDR)

/***************************************************************************
Summary:
Helpers to get ringbuffer register addresses from source channel id
***************************************************************************/
#define BAPE_P_SFIFO_TO_RINBGUFFER(id) (2*(id))
#define BAPE_P_SFIFO_TO_RDADDR_REG(id) (BCHP_AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_RDADDR + (2*BAPE_P_RINGBUFFER_STRIDE*((uint32_t)(id))))
#define BAPE_P_SFIFO_TO_WRADDR_REG(id) (BCHP_AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_WRADDR + (2*BAPE_P_RINGBUFFER_STRIDE*((uint32_t)(id))))
#define BAPE_P_SFIFO_TO_BASEADDR_REG(id) (BCHP_AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_BASEADDR + (2*BAPE_P_RINGBUFFER_STRIDE*((uint32_t)(id))))
#define BAPE_P_SFIFO_TO_ENDADDR_REG(id) (BCHP_AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_ENDADDR + (2*BAPE_P_RINGBUFFER_STRIDE*((uint32_t)(id))))
#define BAPE_P_SFIFO_TO_FREEFULL_REG(id) (BCHP_AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_FREEFULL_MARK + (2*BAPE_P_RINGBUFFER_STRIDE*((uint32_t)(id))))
#define BAPE_P_SFIFO_TO_WRPOINT_REG(id) (BCHP_AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_START_WRPOINT + (2*BAPE_P_RINGBUFFER_STRIDE*((uint32_t)(id))))

/***************************************************************************
Summary:
Helpers to get ringbuffer register addresses from destination channel id
***************************************************************************/
#define BAPE_P_DFIFO_TO_RINBGUFFER(id) ((2*(id)) + BAPE_P_SFIFO_TO_RINBGUFFER(BAPE_CHIP_MAX_SFIFOS))
#define BAPE_P_DFIFO_TO_RDADDR_REG(id) (BCHP_AUD_FMM_BF_CTRL_DESTCH_RINGBUF_0_RDADDR + (2*BAPE_P_RINGBUFFER_STRIDE*((uint32_t)(id))))
#define BAPE_P_DFIFO_TO_WRADDR_REG(id) (BCHP_AUD_FMM_BF_CTRL_DESTCH_RINGBUF_0_WRADDR + (2*BAPE_P_RINGBUFFER_STRIDE*((uint32_t)(id))))
#define BAPE_P_DFIFO_TO_BASEADDR_REG(id) (BCHP_AUD_FMM_BF_CTRL_DESTCH_RINGBUF_0_BASEADDR + (2*BAPE_P_RINGBUFFER_STRIDE*((uint32_t)(id))))
#define BAPE_P_DFIFO_TO_ENDADDR_REG(id) (BCHP_AUD_FMM_BF_CTRL_DESTCH_RINGBUF_0_ENDADDR + (2*BAPE_P_RINGBUFFER_STRIDE*((uint32_t)(id))))
#define BAPE_P_DFIFO_TO_FREEFULL_REG(id) (BCHP_AUD_FMM_BF_CTRL_DESTCH_RINGBUF_0_FREEFULL_MARK + (2*BAPE_P_RINGBUFFER_STRIDE*((uint32_t)(id))))
#define BAPE_P_DFIFO_TO_WRPOINT_REG(id) (BCHP_AUD_FMM_BF_CTRL_DESTCH_RINGBUF_0_START_WRPOINT + (2*BAPE_P_RINGBUFFER_STRIDE*((uint32_t)(id))))
#endif

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
BERR_Code BAPE_P_SetDfifoFullmarkInterrupt(
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
BERR_Code BAPE_P_SetDfifoOverflowInterrupt(
    BAPE_Handle handle,
    unsigned destChannelId,
    BINT_CallbackFunc callback_isr,
    void *pParam1,
    int param2
    );

/***************************************************************************
Summary:
Helper to get the SPDIF channel status code for a sample rate (IEC60958-3)
***************************************************************************/
unsigned BAPE_P_GetSampleRateCstatCode_isr(unsigned sampleRate);

/***************************************************************************
Summary:
Create a "Standard" mixer
***************************************************************************/
BERR_Code BAPE_StandardMixer_P_Create(
    BAPE_Handle deviceHandle,
    const BAPE_MixerSettings *pSettings,
    BAPE_MixerHandle *pHandle               /* [out] */
    );

/***************************************************************************
Summary:
Create a DSP mixer
***************************************************************************/
BERR_Code BAPE_DspMixer_P_Create(
    BAPE_Handle deviceHandle,
    const BAPE_MixerSettings *pSettings,
    BAPE_MixerHandle *pHandle               /* [out] */
    );

/***************************************************************************
Summary:
Find the index of a mixer input connector
***************************************************************************/
#define BAPE_MIXER_INPUT_INDEX_INVALID ((unsigned)-1)
unsigned BAPE_Mixer_P_FindInputIndex_isrsafe(BAPE_MixerHandle mixer, BAPE_Connector input);

/***************************************************************************
Summary:
Return the default mixer input volume
***************************************************************************/
void BAPE_Mixer_P_GetDefaultInputVolume(BAPE_MixerInputVolume *pVolume);

/***************************************************************************
Summary:
Apply the volume setting to a mixer's output
***************************************************************************/
BERR_Code BAPE_Mixer_P_ApplyOutputVolume(BAPE_MixerHandle mixer, BAPE_OutputPort output);

/***************************************************************************
Summary:
Print information (using BDBG_LOG) about a BAPE_PathNode.
***************************************************************************/
BERR_Code BAPE_Mixer_P_PrintNodeInfo( BAPE_PathNode *pPathNode, int level, int index);

/***************************************************************************
Summary:
Print a mixer-centric representation of the audio filter graph (for all
mixers).
***************************************************************************/
BERR_Code BAPE_Mixer_P_PrintMixers(BAPE_Handle deviceHandle);

/***************************************************************************
Summary:
Print a representation of the audio filter graph (for all downstream
PathNodes).
***************************************************************************/
BERR_Code BAPE_Mixer_P_PrintDownstreamNodes(BAPE_PathNode *pPathNode);

/***************************************************************************
Summary:
Print a representation of the audio filter graph (for all downstream
PathNodes).
***************************************************************************/
const char *BAPE_Mixer_P_MclkSourceToText_isrsafe(BAPE_MclkSource mclkSource);

/***************************************************************************
Summary:
Get a mixer's output data type
***************************************************************************/
#define BAPE_Mixer_P_GetOutputDataType_isr BAPE_Mixer_P_GetOutputDataType
#define BAPE_Mixer_P_GetOutputDataType(hMix) ((hMix)->pathNode.connectors[0].format.type)

/***************************************************************************
Summary:
Get a mixer's output data type
***************************************************************************/
#define BAPE_Mixer_P_GetOutputFormat_isr BAPE_Mixer_P_GetOutputFormat
#define BAPE_Mixer_P_GetOutputFormat(hMix) ((const BAPE_FMT_Descriptor *)&(hMix)->pathNode.connectors[0].format)

/***************************************************************************
Summary:
Get a mixer's output sample rate
***************************************************************************/
#define BAPE_Mixer_P_GetOutputSampleRate_isr BAPE_Mixer_P_GetOutputSampleRate
#define BAPE_Mixer_P_GetOutputSampleRate(hMix) ((hMix)->pathNode.connectors[0].format.sampleRate)

/***************************************************************************
Summary:
Get a mixer's output data type based on the widest of all inputs
***************************************************************************/
BERR_Code BAPE_Mixer_P_DetermineOutputDataType(BAPE_MixerHandle handle, BAPE_DataType *pDataType /* [out] */);

/***************************************************************************
Summary:
Inform a mixer that the consumer SFIFO has started
***************************************************************************/
void BAPE_StandardMixer_P_SfifoStarted(BAPE_MixerHandle handle, BAPE_PathConnection *pConnection);

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
Verify that after updates shared Plls aren't running a different rates
***************************************************************************/
void BAPE_P_VerifyPllCallback_isr(void *pParam1, int param2);

/***************************************************************************
Summary:
Attach a mixer to a NCO
***************************************************************************/
void BAPE_P_AttachMixerToNco(BAPE_MixerHandle mixer, BAPE_Nco nco);

/***************************************************************************
Summary:
Detach a mixer from a NCO
***************************************************************************/
void BAPE_P_DetachMixerFromNco(BAPE_MixerHandle mixer, BAPE_Nco nco);

/***************************************************************************
Summary:
Update NCO sample rate
***************************************************************************/
BERR_Code BAPE_P_UpdateNco_isr(BAPE_Handle handle, BAPE_Nco nco);

/***************************************************************************
Summary:
Allocate buffers from the resource pool
***************************************************************************/
BERR_Code BAPE_P_AllocateBuffers(
    BAPE_Handle deviceHandle,
    const BAPE_FMT_Descriptor *pDesc,
    BAPE_BufferNode *pBuffers[BAPE_ChannelPair_eMax]
    );

/***************************************************************************
Summary:
Release buffers to the resource pool
***************************************************************************/
void BAPE_P_FreeBuffers(
    BAPE_Handle deviceHandle,
    BAPE_BufferNode *pBuffers[BAPE_ChannelPair_eMax]
    );

/***************************************************************************
Summary:
Allocate FMM Resource
***************************************************************************/
typedef enum BAPE_FmmResourceType
{
    BAPE_FmmResourceType_eSfifo,
    BAPE_FmmResourceType_eDfifo,
    BAPE_FmmResourceType_eSrc,
    BAPE_FmmResourceType_eMixer,
    BAPE_FmmResourceType_ePlayback,
    BAPE_FmmResourceType_eDummysink,
    BAPE_FmmResourceType_eLoopback,
    BAPE_FmmResourceType_eFs,
    BAPE_FmmResourceType_eAdaptiveRate,
    BAPE_FmmResourceType_eFciSplitter,
    BAPE_FmmResourceType_eFciSplitterOutput,
    BAPE_FmmResourceType_eMax
} BAPE_FmmResourceType;

/***************************************************************************
Summary:
Allocate FMM Resource
***************************************************************************/
#if 1
#define BAPE_P_AllocateFmmResource(h,r,n,f) BAPE_P_AllocateFmmResource_tagged((h),(r),(n),(f),__FILE__,__LINE__);
BERR_Code BAPE_P_AllocateFmmResource_tagged(BAPE_Handle handle, BAPE_FmmResourceType resourceType, unsigned numChannelPairs, unsigned *pFirstResource, const char *pFile, unsigned line);
#else
BERR_Code BAPE_P_AllocateFmmResource(
    BAPE_Handle handle,
    BAPE_FmmResourceType resourceType,
    unsigned numChannelPairs,
    unsigned *pFirstResource
    );
#endif

/***************************************************************************
Summary:
Free FMM Resource
***************************************************************************/
void BAPE_P_FreeFmmResource(
    BAPE_Handle handle,
    BAPE_FmmResourceType resourceType,
    unsigned numChannelPairs,
    unsigned firstResource
    );

/***************************************************************************
Summary:
Release all unused path resources.
***************************************************************************/
void BAPE_P_ReleaseUnusedPathResources(BAPE_Handle handle);

/***************************************************************************
Summary:
Allocate Input Buffers from the resource pool
***************************************************************************/
BERR_Code BAPE_P_AllocateInputBuffers(BAPE_Handle handle, BAPE_Connector input);

/***************************************************************************
Summary:
Free Input Buffers to the resource pool
***************************************************************************/
void BAPE_P_FreeInputBuffers(BAPE_Handle handle, BAPE_Connector input);

/***************************************************************************
Summary:
Determine if a MuxOutput object has been started
***************************************************************************/
bool BAPE_MuxOutput_P_IsRunning(BAPE_MuxOutputHandle handle);

#if defined BCHP_AUD_FMM_MS_CTRL_REG_START
/***************************************************************************
Summary:
Microsequencer FW version
***************************************************************************/
extern const uint32_t g_BAPE_MS_FirmwareVersion[4];

/***************************************************************************
Summary:
Microsequencer FW
***************************************************************************/
#include "bchp_aud_fmm_ms_ctrl.h"
extern const uint32_t g_BAPE_MS_Firmware[BCHP_AUD_FMM_MS_CTRL_USEQ_INSTi_ARRAY_END + 1];
#endif


#define BAPE_P_FIFO_HEAD(name, type) struct name { type *bf_base; type *bf_last; type *bf_read; type *bf_write; int bf_wrap; }

#define BAPE_P_FIFO_INIT(fifo, base, size) do {(fifo)->bf_wrap=0; \
    (fifo)->bf_base=(fifo)->bf_read=(fifo)->bf_write=(base);\
    (fifo)->bf_last=(fifo)->bf_base+(size);}while(0)

#define BAPE_P_FIFO_WRITE(fifo) (fifo)->bf_write
#define BAPE_P_FIFO_READ(fifo) (fifo)->bf_read

#define BAPE_P_FIFO_WRITE_PEEK(fifo) \
    /* |====W---R===| */ ((unsigned)(((fifo)->bf_write < (fifo)->bf_read) ? (fifo)->bf_read - (fifo)->bf_write : ( \
    /* |---R===W---| */ ((fifo)->bf_write > (fifo)->bf_read) ? (fifo)->bf_last - (fifo)->bf_write  : ( \
    /* |---RW---| */ (fifo)->bf_wrap ? 0 : (fifo)->bf_last - (fifo)->bf_write))))

#define BAPE_P_FIFO_READ_PEEK(fifo) \
    /* |====W---R===| */ ((unsigned)(((fifo)->bf_write < (fifo)->bf_read) ? (fifo)->bf_last - (fifo)->bf_read : ( \
    /* |---R===W---| */ ((fifo)->bf_write > (fifo)->bf_read) ? (fifo)->bf_write - (fifo)->bf_read : ( \
    /* |---RW---| */ (fifo)->bf_wrap ? (fifo)->bf_last - (fifo)->bf_read:0))))

#define BAPE_P_FIFO_WRITE_COMMIT(fifo, size) do { \
    BDBG_ASSERT((size)>0 && BAPE_P_FIFO_WRITE_PEEK(fifo) >= (size));  \
    (fifo)->bf_write += (size); \
    if ((fifo)->bf_write >= (fifo)->bf_last) {(fifo)->bf_write = (fifo)->bf_base;(fifo)->bf_wrap++;}  \
    } while(0)

#define BAPE_P_FIFO_READ_COMMIT(fifo, size) do { \
    BDBG_ASSERT(BAPE_P_FIFO_READ_PEEK(fifo) >= (unsigned)(size)); \
    (fifo)->bf_read += (size); \
    if ((fifo)->bf_read >= (fifo)->bf_last) {(fifo)->bf_read = (fifo)->bf_base;(fifo)->bf_wrap--;}  \
    } while(0)

#define BAPE_P_FIFO_WRITE_LEFT(fifo)  \
    /* |====W---R===| */ (((fifo)->bf_write < (fifo)->bf_read) ? (fifo)->bf_read - (fifo)->bf_write : ( \
    /* |---R===W---| */ ((fifo)->bf_write > (fifo)->bf_read) ? ((fifo)->bf_read - (fifo)->bf_base) + ((fifo)->bf_last - (fifo)->bf_write)  : ( \
    /* |---RW---| */ (fifo)->bf_wrap ? 0 : (fifo)->bf_last - (fifo)->bf_base)))

#define BAPE_P_FIFO_READ_LEFT(fifo) \
    /* |====W---R===| */ (((fifo)->bf_write < (fifo)->bf_read) ? ((fifo)->bf_write - (fifo)->bf_base) + (fifo)->bf_last - (fifo)->bf_read : ( \
    /* |---R===W---| */ ((fifo)->bf_write > (fifo)->bf_read) ? (fifo)->bf_write - (fifo)->bf_read : ( \
    /* |---RW---| */ (fifo)->bf_wrap ? (fifo)->bf_last - (fifo)->bf_base:0)))

#define BAPE_P_FIFO_VALIDATE(fifo) do { \
        BDBG_ASSERT((fifo)->bf_wrap==0 || (fifo)->bf_wrap==1); \
        BDBG_ASSERT((fifo)->bf_read>=(fifo)->bf_base && (fifo)->bf_read<(fifo)->bf_last); \
        BDBG_ASSERT((fifo)->bf_write>=(fifo)->bf_base && (fifo)->bf_write<(fifo)->bf_last); \
    } while(0)

#define BAPE_P_FIFO_STATUS(DBG, header,fifo)  DBG((header ":(%#lx:%#lx) write (%u/%u/%#lx), read (%u/%u/%#lx)", (unsigned long)(fifo)->bf_base, (unsigned long)(fifo)->bf_last, BAPE_P_FIFO_WRITE_PEEK(fifo), BAPE_P_FIFO_WRITE_LEFT(fifo), (unsigned long)BAPE_P_FIFO_WRITE(fifo), BAPE_P_FIFO_READ_PEEK(fifo), BAPE_P_FIFO_READ_LEFT(fifo), (unsigned long)BAPE_P_FIFO_READ(fifo)))

#if BAPE_DSP_SUPPORT
/***************************************************************************
Summary:
Decode to memory buffer node
***************************************************************************/
typedef struct BAPE_DecodeToMemoryNode
{
    BLST_Q_ENTRY(BAPE_DecodeToMemoryNode) node;
    BAPE_DecoderBufferDescriptor descriptor;
    void *pMetadataAddr;    /* un-cached address */
    BDSP_OnDemand_MetaDataInfo *pMetadata; /* cached address */
    uint32_t metadataOffset;
} BAPE_DecodeToMemoryNode;
#endif

/***************************************************************************
Summary:
Decoder Handle
***************************************************************************/
typedef struct BAPE_Decoder
{
    BDBG_OBJECT(BAPE_Decoder)
#if BAPE_DSP_SUPPORT
    BAPE_Handle deviceHandle;
    unsigned index;
    BAPE_PathNode node;
    char name[10]; /* Decoder %u */

    /* Basic State Information */
    bool independentDelay;
    bool passthrough;
    bool simul;
    bool stereoOnMultichannel;
    bool stereoOnCompressed;

    BAPE_DecoderType type;
    BAPE_PathNodeOutputStatus outputStatus;
    BAPE_DecoderState state;
    BAPE_DecoderSettings settings;
    BAPE_DecoderStartSettings startSettings;
    BAVC_XptContextMap contextMap;
    BAPE_DecoderTsmSettings tsmSettings;
    struct {
        BAPE_DecoderDecodeToMemorySettings settings;
        BAPE_DecoderDecodeToMemoryStatus status;
        BDSP_QueueHandle hARQ, hADQ;
        void *pArqMem, *pAdqMem;
        BAPE_DecodeToMemoryNode *pNodes;
        void *pMetadata;
        BLST_Q_HEAD(FreeList, BAPE_DecodeToMemoryNode) freeList;
        BLST_Q_HEAD(PendingList, BAPE_DecodeToMemoryNode) pendingList;
        BLST_Q_HEAD(CompletedList, BAPE_DecodeToMemoryNode) completedList;
    } decodeToMem;

    uint32_t lastValidPts;

    union
    {
#if defined BDSP_MS10_SUPPORT || defined BDSP_DOLBY_DCV_SUPPORT
        BDSP_Raaga_Audio_DDPMultiStreamConfigParams ddp;
#else
        BDSP_Raaga_Audio_UdcdecConfigParams ddp;
#endif
#if BDSP_MS12_SUPPORT
        BDSP_Raaga_Audio_DolbyAacheUserConfig aac;
#elif BDSP_MS10_SUPPORT
        BDSP_Raaga_Audio_DolbyPulseUserConfig aac;
#else
        BDSP_Raaga_Audio_AacheConfigParams aac;
#endif
        BDSP_Raaga_Audio_AC4DecConfigParams ac4;
        BDSP_Raaga_Audio_MpegConfigParams mpeg;
        BDSP_Raaga_Audio_WmaConfigParams wma;
        BDSP_Raaga_Audio_WmaProConfigParams wmaPro;
        BDSP_Raaga_Audio_DtsHdConfigParams dts;
        BDSP_Raaga_Audio_DtslbrConfigParams dtsExpress;
        BDSP_Raaga_Audio_AmrConfigParams amr;
        BDSP_Raaga_Audio_AmrwbdecConfigParams amrWb;
        BDSP_Raaga_Audio_DraConfigParams dra;
        BDSP_Raaga_Audio_RalbrConfigParams cook;
        BDSP_Raaga_Audio_PcmWavConfigParams pcmWav;
        BDSP_Raaga_G711G726Settings g711g726;
        BDSP_Raaga_G723_1Settings g723;
        BDSP_Raaga_G729Settings g729;
        BDSP_Raaga_Audio_AdpcmConfigParams adpcm;
        BDSP_Raaga_Audio_LpcmUserConfig lpcm;
        BDSP_Raaga_Audio_FlacDecConfigParams flac;
        BDSP_Raaga_Audio_iLBCdecConfigParams ilbc;
        BDSP_Raaga_Audio_iSACdecConfigParams isac;
        BDSP_Raaga_Audio_OpusDecConfigParams opus;
        BDSP_Raaga_Audio_ALSDecConfigParams als;
    } userConfig;
    BDSP_AudioTaskDatasyncSettings datasyncSettings;

    union
    {
#if defined BDSP_MS10_SUPPORT || defined BDSP_DOLBY_DCV_SUPPORT
        BDSP_Raaga_Audio_MultiStreamDDPStreamInfo ddp;
#else
        BDSP_Raaga_Audio_UdcStreamInfo ddp;
#endif
        BDSP_Raaga_Audio_AC4StreamInfo ac4;
        BDSP_Raaga_Audio_MpegStreamInfo mpeg;
        BDSP_Raaga_Audio_AacheStreamInfo aac;
        BDSP_Raaga_Audio_WmaStreamInfo wma;
        BDSP_Raaga_Audio_WmaProStreamInfo wmaPro;
        BDSP_Raaga_Audio_DtsHdStreamInfo dts;
        BDSP_Raaga_Audio_DtslbrStreamInfo dtsExpress;
        BDSP_Raaga_Audio_PcmWavStreamInfo pcmwav;
        BDSP_Raaga_Audio_AmrStreamInfo amr;
        BDSP_Raaga_Audio_AmrWbStreamInfo amrWb;
        BDSP_Raaga_Audio_DraStreamInfo dra;
        BDSP_Raaga_Audio_RalbrStreamInfo cook;
        BDSP_Raaga_Audio_LpcmStreamInfo lpcm;
        BDSP_Raaga_Audio_MlpStreamInfo mlp;
        BDSP_Raaga_Audio_AdpcmStreamInfo adpcm;
        BDSP_Raaga_Audio_G726StreamInfo g711g726;
        BDSP_Raaga_Audio_G729DecStreamInfo g729;
        BDSP_Raaga_Audio_G723_1_StreamInfo g723;
        BDSP_Raaga_VorbisStatus vorbis;
        BDSP_Raaga_MacStatus ape;
        BDSP_Raaga_FlacStatus flac;
        BDSP_Raaga_iLBCStatus ilbc;
        BDSP_Raaga_iSACStatus isac;
        BDSP_Raaga_OpusDecStatus opus;
        BDSP_Raaga_Audio_ALSDecStreamInfo als;
        BDSP_Raaga_GenericPassThruStatus generic;
    } streamInfo;

    /* Codec-specific settings */
    /* TODO: Codec settings should be stored in the individual structures and not the full union to save space. */
    BAPE_DecoderCodecSettings ac3Settings, ac3PlusSettings;
    BAPE_DecoderCodecSettings ac4Settings;
    BAPE_DecoderCodecSettings aacSettings, aacPlusSettings;
    BAPE_DecoderCodecSettings wmaProSettings;
    BAPE_DecoderCodecSettings dtsSettings, dtsExpressSettings;
    BAPE_DecoderCodecSettings adpcmSettings;
    BAPE_DecoderCodecSettings ilbcSettings;
    BAPE_DecoderCodecSettings isacSettings;
    BAPE_DecoderCodecSettings alsSettings;
    BAPE_DecoderInterruptHandlers interrupts;

    /* DSP Task Information */
    BDSP_TaskHandle hTask;
    BDSP_StageHandle hPrimaryStage;
    BDSP_StageHandle hSrcStageStereo;
    BDSP_StageHandle hSrcStageMultichannel;
    BDSP_StageHandle hDsolaStageStereo;
    BDSP_StageHandle hDsolaStageMultichannel;
    BDSP_StageHandle hKaraokeStage;
    BDSP_StageHandle hPassthroughStage;
    BDSP_StageHandle hOutputFormatter;
    unsigned dspIndex;
    unsigned streamSampleRate;
    unsigned pcmOutputSampleRate;
    BDSP_AF_P_sOpSamplingFreq sampleRateMap;
    unsigned mode;
    BDSP_AudioBitRateChangeInfo bitRateInfo;
    BDSP_AudioTaskDelayMode pathDelayMode;

    /* DFIFO Input Information */
    BAPE_FMT_Descriptor inputPortFormat;
    BAPE_DfifoGroupHandle inputDfifoGroup;
    BAPE_BufferNode *pInputBuffers[BAPE_ChannelPair_eMax];
    BAPE_FciSplitterOutputGroupHandle fciSpOutput;

    /* Required for Multi-stream Decoding  */
    bool fwMixerMaster;
    BAPE_DolbyDigitalReencodeHandle ddre;
    BAPE_MixerHandle fwMixer;

    /* Ancillary Data Handling */
    BDSP_QueueHandle hAncDataQueue;
    BDSP_AF_P_sSINGLE_CIRC_BUFFER *pAncDataBufferDescriptor;
    void *pAncDataDspBuffer;
    size_t ancDataBufferSize;
    void *pAncDataHostBuffer, *pAncDataHostCached;
    BAPE_P_FIFO_HEAD(AncDataFifo, uint8_t) ancDataFifo;
    bool ancDataInit;

    /* Encoder overflow handling */
    /*BAPE_MuxOutputHandle hMuxOutput;*/
    BLST_S_HEAD(DecoderMuxOutputList, BAPE_MuxOutput) muxOutputList;

    /* Timer for Underflow checks */
    BTMR_TimerHandle underFlowTimer;
    unsigned underFlowCount;
    bool halted;
#else
    /* Stubs required in mixer for debug */
    BAPE_DecoderStartSettings startSettings;
    BAVC_XptContextMap contextMap;
#endif
} BAPE_Decoder;

/***************************************************************************
Summary:
Initialize the ancillary data buffer descriptor prior to start
***************************************************************************/
void BAPE_Decoder_P_InitAncillaryDataBuffer(BAPE_DecoderHandle hDecoder);

/***************************************************************************
Summary:
Set a decoder's sample rate
***************************************************************************/
void BAPE_Decoder_P_SetSampleRate_isr(BAPE_DecoderHandle handle, unsigned sampleRate);

/***************************************************************************
Summary:
Get default codec specifics for the DSP codecs
***************************************************************************/
void BAPE_Decoder_P_GetDefaultCodecSettings(BAPE_DecoderHandle handle);

/***************************************************************************
Summary:
Apply codec settings to DSP
***************************************************************************/
BERR_Code BAPE_Decoder_P_ApplyCodecSettings(BAPE_DecoderHandle handle);

/***************************************************************************
Summary:
Get codec status from DSP
***************************************************************************/
BERR_Code BAPE_Decoder_P_GetCodecStatus(BAPE_DecoderHandle handle, BAPE_DecoderStatus *pStatus);

/***************************************************************************
Summary:
Get AC-4 Presentation Info
***************************************************************************/
BERR_Code BAPE_Decoder_P_GetAc4PresentationInfo(BAPE_DecoderHandle handle, unsigned presentationIndex, BAPE_DecoderPresentationInfo *pInfo);

/***************************************************************************
Summary:
Get data sync status from DSP
***************************************************************************/
BERR_Code BAPE_Decoder_P_GetDataSyncStatus_isr(BAPE_DecoderHandle handle, unsigned *underflowCount);

/***************************************************************************
Summary:
Propagate settings between decoder and DDRE
***************************************************************************/
BERR_Code BAPE_DolbyDigitalReencode_P_SettingsChanged(
    BAPE_DolbyDigitalReencodeHandle handle,
    BAPE_DecoderHandle decoder
    );

/***************************************************************************
Summary:
Get the DDRE Multichannel Format
***************************************************************************/
BAPE_MultichannelFormat BAPE_DolbyDigitalReencode_P_GetMultichannelFormat(
    BAPE_DolbyDigitalReencodeHandle handle
    );

/***************************************************************************
Summary:
Get the Task Handle for a dependent encoder task (dual dsp or host audio)
***************************************************************************/
void * BAPE_DolbyDigitalReencode_P_GetEncoderTaskHandle(
    BAPE_DolbyDigitalReencodeHandle handle
    );

/***************************************************************************
Summary:
Query whether there are compressed consumers
***************************************************************************/
bool BAPE_DolbyDigitalReencode_P_HasCompressedConsumers(
    BAPE_DolbyDigitalReencodeHandle handle
    );

/***************************************************************************
Summary:
Get the Device Index (used for debug logging)
***************************************************************************/
unsigned BAPE_DolbyDigitalReencode_P_GetDeviceIndex(
    BAPE_DolbyDigitalReencodeHandle handle
    );

/***************************************************************************
Summary:
Playback Handle Structure
***************************************************************************/
typedef struct BAPE_Playback
{
    BDBG_OBJECT(BAPE_Playback)
    BAPE_Handle hApe;
    BMEM_Heap_Handle hHeap;
    unsigned index;
    BAPE_PathNode node;
    BAPE_PathConnection *pMaster;
    BAPE_PlaybackStartSettings startSettings;
    void *pBuffer[BAPE_Channel_eMax];
    BAPE_PlaybackSettings settings;
    unsigned bufferSize;
    unsigned numBuffers;
    unsigned threshold;
    unsigned bufferDepth;   /* Required for pre-start buffer fills (used in looparound mode) */
    BAPE_PlaybackInterruptHandlers interrupts;
    bool running;
    char name[12]; /* Playback %d */
} BAPE_Playback;

/***************************************************************************
Summary:
Input Capture Handle Structure
***************************************************************************/
typedef struct BAPE_InputCapture
{
    BDBG_OBJECT(BAPE_InputCapture)
    BAPE_Handle deviceHandle;
    BMEM_Heap_Handle hHeap;
    unsigned bufferSize;
    bool useLargeRingBuffers;
    unsigned index;
    BAPE_PathNode node;
    BAPE_PathConnection *pMaster;
    BAPE_InputCaptureStartSettings startSettings;
    BAPE_InputCaptureSettings settings;
    BAPE_InputCaptureInterruptHandlers interrupts;
    unsigned numBuffers;
    void *pBuffers[BAPE_Channel_eMax];
    uint32_t bufferOffset[BAPE_Channel_eMax];
    BAPE_DfifoGroupHandle dfifoGroup;
    BAPE_PathConnection *pMasterConnection;         /* Master for SFIFO master/slave designation */
    BAPE_FciSplitterOutputGroupHandle fciSpOutput;
    BAPE_FMT_Descriptor inputPortFormat;
    bool sfifoForFmmOutputs;
    bool sfifoRequired;
    bool interleave;
    bool running;
    bool halted;
    char name[16];  /* InputCapture %d */
} BAPE_InputCapture;

/***************************************************************************
Summary:
Echo Canceller Handle Structure
***************************************************************************/
typedef struct BAPE_EchoCanceller
{
    BDBG_OBJECT(BAPE_EchoCanceller)
#if BAPE_DSP_SUPPORT
    BAPE_Handle deviceHandle;
    BAPE_PathNode localNode, remoteNode;    /* Need two nodes to distinguish the local vs. remote input paths.  They don't behave the same way. */
    BAPE_EchoCancellerSettings settings;
    BAPE_SpeexEchoCancellerSettings speexSettings;
    BAPE_Connector localInput, remoteInput;
    bool localStarted, remoteStarted;
    BDSP_StageHandle hStage;
    unsigned dspStageInput;

    /* Inter-task communication details */
    BDSP_InterTaskBufferHandle hInterTaskBuffer;
#endif
} BAPE_EchoCanceller;

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
    unsigned fsIndex,
    BAPE_MclkSource mclkSource,
    unsigned pllChannel,
    unsigned mclkFreqToFsRatio
    );

#if BAPE_DSP_SUPPORT
/***************************************************************************
Summary:
Map User settings to DSP for TruVolume.  Required in both StudioSound
and TruVolume.
***************************************************************************/
BERR_Code BAPE_TruVolume_P_ConvertSettingsToDsp(
    const BAPE_TruVolumeSettings *pSettings,
    BDSP_Raaga_Audio_TruVolumeUserConfig *pUserConfig
    );

/***************************************************************************
Summary:
Map User settings to DSP for TruSurroundHd.  Required in both StudioSound
and TruSurroundHd.
***************************************************************************/
BERR_Code BAPE_TruSurroundHd_P_ConvertSettingsToDsp(
    const BAPE_TruSurroundHdSettings *pSettings,
    unsigned numChannelPairs,
    BDSP_Raaga_Audio_TruSurrndHDConfigParams *pUserConfig
    );
#endif

#if BAPE_CHIP_MAX_SPDIF_OUTPUTS > 0 || BAPE_CHIP_MAX_MAI_OUTPUTS > 0
/***************************************************************************
Summary:
SPDIF Channel Status Bits
***************************************************************************/
typedef struct BAPE_Spdif_P_ChannelStatusBits
{
    uint32_t bits[3];
}BAPE_Spdif_P_ChannelStatusBits;

/***************************************************************************
Summary:
Setup Channel Status Bits for SPDIF or HDMI outputs
***************************************************************************/
void BAPE_P_MapSpdifChannelStatusToBits_isr(
    BAPE_OutputPort output,
    const BAPE_SpdifChannelStatus *pChannelStatus,
    BAPE_Spdif_P_ChannelStatusBits *pBits           /* [out] */
    );
#endif

typedef struct BAPE_ItbEntry
{
    uint32_t words[4];
} BAPE_ItbEntry;

typedef struct BAPE_FrameItbEntries
{
    BAPE_ItbEntry baseAddress;
    BAPE_ItbEntry ptsDts;
    BAPE_ItbEntry bitRate;
    BAPE_ItbEntry escrMetadata;
} BAPE_FrameItbEntries;

#define BAPE_MUXOUTPUT_MAX_ITBDESCRIPTORS 128
#define BAPE_MUXOUTPUT_MAX_METADATADESCRIPTORS 1
#define BAPE_MUXOUTPUT_MAX_ADTS_SEGMENTS 5          /* ADTS can have up to 4 data segments with a max of one CDB wrap around */
#define BAPE_MUXOUTPUT_MAX_ASC_ENTRIES 2

typedef struct BAPE_OutputDescriptorInfo
{
    uint32_t uiITBBufferShadowReadOffset; /* Points to the ITB entry that needs to be parsed next */
    uint32_t uiCDBBufferShadowReadOffset; /* Points to the CDB location that needs to be muxed next */

    struct {
        BAVC_AudioBufferDescriptor *cached;
        BMMA_Block_Handle block;
        unsigned blockOffset;
    } descriptors;
    struct {
        BAVC_AudioMetadataDescriptor *cached;
        BMMA_Block_Handle block;
        unsigned blockOffset;
    } metadata;
    uint32_t uiDescriptorWriteOffset;
    uint32_t uiDescriptorReadOffset;
    unsigned numOutstandingDescriptors;

    /* ITB Parsing Info */
    struct
    {
        BAPE_FrameItbEntries current, next;
        BAPE_ItbEntry metadata;
    } itb;

    /* ADTS Parsing Info */
    unsigned adtsSegment, adtsNumSegments;
    unsigned adtsSegmentOffset[BAPE_MUXOUTPUT_MAX_ADTS_SEGMENTS];
    unsigned adtsSegmentLength[BAPE_MUXOUTPUT_MAX_ADTS_SEGMENTS];
    unsigned adtsSegmentRawOffset[BAPE_MUXOUTPUT_MAX_ADTS_SEGMENTS];
    unsigned adtsSegmentRawLength[BAPE_MUXOUTPUT_MAX_ADTS_SEGMENTS];
} BAPE_OutputDescriptorInfo;

typedef enum BAPE_MuxOutputState
{
    BAPE_MuxOutputState_Init,
    BAPE_MuxOutputState_Started,
    BAPE_MuxOutputState_Stopped,
    BAPE_MuxOutputState_Max
} BAPE_MuxOutputState;

typedef struct BAPE_BufferInterface
{
    uint32_t base;
    uint32_t end;
    uint32_t read;
    uint32_t valid;
    bool     inclusive; /* If true the end address is included */
} BAPE_BufferInterface;

typedef struct BAPE_MuxOutput
{
    BDBG_OBJECT(BAPE_MuxOutput)
    BAPE_PathNode node;
    BAPE_MuxOutputState state;
    bool sendEos;
    bool sendMetadata;
    BAPE_MuxOutputStartSettings startSettings;
    BAVC_XptContextMap contextMap;
    BAPE_MuxOutputCreateSettings createSettings;
    BAPE_Connector input;
    struct {
        void *cached;
        BMMA_DeviceOffset offset;

        /* RDB */
        BDSP_QueueHandle queue;
        BDSP_AF_P_sDRAM_CIRCULAR_BUFFER buffer;
        BAPE_BufferInterface bufferInterface;
    } cdb, itb;
    BAPE_OutputDescriptorInfo descriptorInfo;
    BLST_S_ENTRY(BAPE_MuxOutput) deviceListNode;
    BLST_S_ENTRY(BAPE_MuxOutput) decoderListNode;
    #if BAPE_DSP_SUPPORT
    BDSP_StageHandle hStage;
    #endif
    BAPE_MuxOutputInterruptHandlers interrupts;
    BAVC_Xpt_StcSoftIncRegisters nonRealTimeIncrement;
} BAPE_MuxOutput;

/***************************************************************************
Summary:
Handle an overflow interrupt for MuxOutput (GenCdbItb)
***************************************************************************/
void BAPE_MuxOutput_P_Overflow_isr(BAPE_MuxOutputHandle hMuxOutput);

/***************************************************************************
Summary:
Callback to check status of decoder underflow
***************************************************************************/
void BAPE_P_CheckUnderflow_isr (
    void *pParam1,
    int param2
    );

/***************************************************************************
Summary:
CRC Start
***************************************************************************/
BERR_Code BAPE_Crc_P_Start(
    BAPE_CrcHandle handle
    );

/***************************************************************************
Summary:
CRC Stop
***************************************************************************/
void BAPE_Crc_P_Stop(
    BAPE_CrcHandle handle
    );

#if BAPE_DSP_SUPPORT
void BAPE_P_PopulateSupportedBDSPAlgos(
    BDSP_AlgorithmType type, /* [in] */
    const BAVC_AudioCompressionStd * pSupportedCodecs, /* [in] */
    unsigned numSupportedCodecs, /* [in] */
    const bool * inAlgorithmSupported, /* [in] */
    bool * outAlgorithmSupported /* [out] */
    );

#if 0
void BAPE_P_PopulateSupportedBAVCAlgos(
    BAPE_Handle deviceHandle, /* [in] */
    BDSP_AlgorithmType algoType, /* [in] */
    bool * outSupportedCodecs /* [out] */
    );
#endif
#endif

/* These must be after definition of path types above */
#if BAPE_DSP_SUPPORT
#include "bape_dsp_utils_priv.h"
#else
#define BAPE_P_GetCodecName(x) "Unknown"
#endif
#include "bape_path_priv.h"
#include "bape_fmm_priv.h"

#endif /* #ifndef BAPE_PRIV_H_ */
