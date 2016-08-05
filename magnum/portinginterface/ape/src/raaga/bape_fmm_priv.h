/***************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
 * Module Description: FMM Interfaces
 *
 ***************************************************************************/

#ifndef BAPE_FMM_PRIV_H_
#define BAPE_FMM_PRIV_H_

#include "bstd.h"
#include "bkni.h"
#include "bape.h"
#include "blst_slist.h"
#include "bape_chip_priv.h"


/***************************************************************************
Summary:
Prepare the MAI (HDMI) inputs to go into standby
***************************************************************************/
BERR_Code BAPE_MaiInput_P_PrepareForStandby(
    BAPE_Handle bapeHandle
    );

/***************************************************************************
Summary:
Prepare the MAI (HDMI) outputs to go into standby
***************************************************************************/
BERR_Code BAPE_MaiOutput_P_PrepareForStandby(
    BAPE_Handle bapeHandle
    );

/***************************************************************************
Summary:
Prepare the Spdif outputs to go into standby
***************************************************************************/
BERR_Code BAPE_SpdifOutput_P_PrepareForStandby(
    BAPE_Handle bapeHandle
    );

/***************************************************************************
Summary:
Prepare the DAC outputs to go into standby
***************************************************************************/
BERR_Code BAPE_Dac_P_PrepareForStandby(
    BAPE_Handle bapeHandle
    );

/***************************************************************************
Summary:
Prepare the I2s outputs to go into standby
***************************************************************************/
BERR_Code BAPE_I2sOutput_P_PrepareForStandby(
    BAPE_Handle bapeHandle
    );

/***************************************************************************
Summary:
Prepare the SPDIF inputs to go into standby
***************************************************************************/
BERR_Code BAPE_SpdifInput_P_PrepareForStandby(
    BAPE_Handle bapeHandle
    );


/***************************************************************************
Summary:
Restore the NCOs' state when resuming from standby
***************************************************************************/
BERR_Code BAPE_Nco_P_ResumeFromStandby(
    BAPE_Handle bapeHandle
    );

/***************************************************************************
Summary:
Restore the PLLs' state when resuming from standby
***************************************************************************/
BERR_Code BAPE_Pll_P_ResumeFromStandby(
    BAPE_Handle bapeHandle
    );

/***************************************************************************
Summary:
Restore the LoopbackGroups' state when resuming from standby
***************************************************************************/
BERR_Code BAPE_LoopbackGroup_P_ResumeFromStandby(
    BAPE_Handle bapeHandle
    );

/***************************************************************************
Summary:
Restore the Dummysinks' state when resuming from standby
***************************************************************************/
BERR_Code BAPE_DummysinkGroup_P_ResumeFromStandby(
    BAPE_Handle bapeHandle
    );

/***************************************************************************
Summary:
Restore the AudioReturnChannels' state when resuming from standby
***************************************************************************/
BERR_Code BAPE_AudioReturnChannel_P_ResumeFromStandby(
    BAPE_Handle bapeHandle
    );

/***************************************************************************
Summary:
Restore the DACs' state when resuming from standby
***************************************************************************/
BERR_Code BAPE_Dac_P_ResumeFromStandby(
    BAPE_Handle bapeHandle
    );

/***************************************************************************
Summary:
Restore the DummyOutputs' state when resuming from standby
***************************************************************************/
BERR_Code BAPE_DummyOutput_P_ResumeFromStandby(
    BAPE_Handle bapeHandle
    );

/***************************************************************************
Summary:
Restore the I2sMulti outputs' state when resuming from standby
***************************************************************************/
BERR_Code BAPE_I2sMultiOutput_P_ResumeFromStandby(
    BAPE_Handle bapeHandle
    );

/***************************************************************************
Summary:
Restore the I2S outputs' state when resuming from standby
***************************************************************************/
BERR_Code BAPE_I2sOutput_P_ResumeFromStandby(
    BAPE_Handle bapeHandle
    );

/***************************************************************************
Summary:
Restore the MAI (HDMI) outputs' state when resuming from standby
***************************************************************************/
BERR_Code BAPE_MaiOutput_P_ResumeFromStandby(
    BAPE_Handle bapeHandle
    );

/***************************************************************************
Summary:
Restore the RF modulators' state when resuming from standby
***************************************************************************/
BERR_Code BAPE_RfMod_P_ResumeFromStandby(
    BAPE_Handle bapeHandle
    );

/***************************************************************************
Summary:
Restore the SPDIF outputs' state when resuming from standby
***************************************************************************/
BERR_Code BAPE_SpdifOutput_P_ResumeFromStandby(
    BAPE_Handle bapeHandle
    );

/***************************************************************************
Summary:
Restore the MAI (HDMI) inputs' state when resuming from standby
***************************************************************************/
BERR_Code BAPE_MaiInput_P_ResumeFromStandby(
    BAPE_Handle bapeHandle
    );

/***************************************************************************
Summary:
Restore the SPDIF inputs' state when resuming from standby
***************************************************************************/
BERR_Code BAPE_SpdifInput_P_ResumeFromStandby(
    BAPE_Handle bapeHandle
    );

/***************************************************************************
Summary:
Initialize the BF block
***************************************************************************/
BERR_Code BAPE_P_InitDacHw(
    BAPE_Handle handle
    );

/***************************************************************************
Summary:
Initialize the BF block data structures
***************************************************************************/
BERR_Code BAPE_P_InitBfSw(
    BAPE_Handle handle
    );

/***************************************************************************
Summary:
Initialize the BF block hardware
***************************************************************************/
BERR_Code BAPE_P_InitBfHw(
    BAPE_Handle handle
    );

/***************************************************************************
Summary:
Un-Initialize the BF block
***************************************************************************/
void BAPE_P_UninitBfSw(
    BAPE_Handle handle
    );

/***************************************************************************
Summary:
Initialize the SRC block data structures
***************************************************************************/
BERR_Code BAPE_P_InitSrcSw(
    BAPE_Handle handle
    );

/***************************************************************************
Summary:
Initialize the SRC block hardware
***************************************************************************/
BERR_Code BAPE_P_InitSrcHw(
    BAPE_Handle handle
    );

/***************************************************************************
Summary:
Un-Initialize the SRC block
***************************************************************************/
void BAPE_P_UninitSrcSw(
    BAPE_Handle handle
    );

/***************************************************************************
Summary:
Initialize the DP block data structures
***************************************************************************/
BERR_Code BAPE_P_InitDpSw(
    BAPE_Handle handle
    );

/***************************************************************************
Summary:
Initialize the DP block hardware
***************************************************************************/
BERR_Code BAPE_P_InitDpHw(
    BAPE_Handle handle
    );

/***************************************************************************
Summary:
Un-Initialize the DP block
***************************************************************************/
void BAPE_P_UninitDpSw(
    BAPE_Handle handle
    );

/***************************************************************************
Summary:
Initialize the IOP block data structures
***************************************************************************/
BERR_Code BAPE_P_InitIopSw(
    BAPE_Handle handle
    );

/***************************************************************************
Summary:
Initialize the IOP block hardware
***************************************************************************/
BERR_Code BAPE_P_InitIopHw(
    BAPE_Handle handle
    );

/***************************************************************************
Summary:
Un-Initialize the IOP block
***************************************************************************/
void BAPE_P_UninitIopSw(
    BAPE_Handle handle
    );

/***************************************************************************
Summary:
Source FIFO Group Create Settings
***************************************************************************/
typedef struct BAPE_SfifoGroupCreateSettings
{
    unsigned numChannelPairs;   /* Number of channel pairs */
    bool ppmCorrection;         /* If true, adaptive rate control resources will be allocated */
} BAPE_SfifoGroupCreateSettings;

/***************************************************************************
Summary:
Get default SFIFO Group Create Settings
***************************************************************************/
void BAPE_SfifoGroup_P_GetDefaultCreateSettings(
    BAPE_SfifoGroupCreateSettings *pSettings    /* [out] */
    );

/***************************************************************************
Summary:
Create SFIFO Group
***************************************************************************/
BERR_Code BAPE_SfifoGroup_P_Create(
    BAPE_Handle deviceHandle,
    const BAPE_SfifoGroupCreateSettings *pSettings,
    BAPE_SfifoGroupHandle *pHandle  /* [out] */
    );

/***************************************************************************
Summary:
Destroy SFIFO Group
***************************************************************************/
void BAPE_SfifoGroup_P_Destroy(
    BAPE_SfifoGroupHandle handle
    );

/***************************************************************************
Summary:
Source FIFO Settings
***************************************************************************/
typedef struct BAPE_SfifoGroupSettings
{
    bool bypassMemory;          /* If true, data will be fed directly from a linked DFIFO */
    bool highPriority;          /* If sample rate is >= 96000 set this to true. */
    bool interleaveData;        /* If true, data will be interleaved into a single buffer per channel pair */
    bool stereoData;            /* If true, the data is stereo.  If false, the data is mono. */
    bool signedData;            /* If true, data is signed. */
    bool loopAround;            /* If true, the SFIFO will not pause when empty */
    bool wrpointEnabled;        /* If true, the start WRPOINT register will enable data flow once the write pointer reaches that value */
    bool sampleRepeatEnabled;   /* If true, samples will be repeated on underflow (set to false for compressed data) */
    bool reverseEndian;         /* If true, reverse sample endian-ness */
    BAPE_SfifoGroupHandle master;   /* If set, this SFIFO group will be setup as a slave to the master provided. */
    unsigned dataWidth;         /* Data width in bits. */
    unsigned defaultSampleRate; /* Expected sample rate - can be overridden later by BAPE_SfifoGroup_P_SetSampleRate_isr */
    struct
    {
        uint32_t base;          /* Base offset of the buffer */
        uint32_t length;        /* Length of the buffer in bytes */
        uint32_t writeOffset;   /* Initial write pointer value will be base+writeOffset */
        uint32_t watermark;     /* Watermark interrupt threshold in bytes */
        uint32_t wrpoint;       /* Start write point (ignored unless wrpointEnabled is true) */
    } bufferInfo[BAPE_Channel_eMax];
} BAPE_SfifoGroupSettings;

/***************************************************************************
Summary:
Get Source FIFO Settings
***************************************************************************/
void BAPE_SfifoGroup_P_GetSettings(
    BAPE_SfifoGroupHandle handle,
    BAPE_SfifoGroupSettings *pSettings  /* [out] */
    );

/***************************************************************************
Summary:
Set Source FIFO Settings
***************************************************************************/
BERR_Code BAPE_SfifoGroup_P_SetSettings(
    BAPE_SfifoGroupHandle handle,
    const BAPE_SfifoGroupSettings *pSettings
    );

/***************************************************************************
Summary:
Start SFIFO group
***************************************************************************/
BERR_Code BAPE_SfifoGroup_P_Start(
    BAPE_SfifoGroupHandle handle,
    bool enableOnly                 /* If true, a separate call to BAPE_SfifoGroup_P_Run_isr is required to
                                       start data flow.  If false, data flow will start immediately. */
    );

/***************************************************************************
Summary:
Stop SFIFO group
***************************************************************************/
void BAPE_SfifoGroup_P_Stop(
    BAPE_SfifoGroupHandle handle
    );

/***************************************************************************
Summary:
Start SFIFO group data flow

Description:
This call is only required if enableOnly was set to true in BAPE_SfifoGroup_P_Start
or if BAPE_SfifoGroup_P_Halt_isr was previously called.
***************************************************************************/
void BAPE_SfifoGroup_P_Run_isr(
    BAPE_SfifoGroupHandle handle
    );

/***************************************************************************
Summary:
Stop SFIFO group data flow

Description:
This call allows data flow to be stopped from an interrupt if required.
***************************************************************************/
void BAPE_SfifoGroup_P_Halt_isr(
    BAPE_SfifoGroupHandle handle
    );

/***************************************************************************
Summary:
Update SFIFO Group sample rate

Description:
This will reprogram the adaptive rate controllers for a new sample rate
if required.
***************************************************************************/
void BAPE_SfifoGroup_P_SetSampleRate_isr(
    BAPE_SfifoGroupHandle handle,
    unsigned sampleRate
    );

/***************************************************************************
Summary:
Get FCI Connection IDs for this group
***************************************************************************/
void BAPE_SfifoGroup_P_GetOutputFciIds(
    BAPE_SfifoGroupHandle handle,
    BAPE_FciIdGroup *pFciGroup      /* [out] */
    );

/***************************************************************************
Summary:
Get Source FIFO Buffers
***************************************************************************/
BERR_Code BAPE_SfifoGroup_P_GetBuffer(
    BAPE_SfifoGroupHandle handle,
    BAPE_BufferDescriptor *pBuffers      /* [out] */
    );

/***************************************************************************
Summary:
Commit data to Source FIFO Buffers
***************************************************************************/
BERR_Code BAPE_SfifoGroup_P_CommitData(
    BAPE_SfifoGroupHandle handle,
    unsigned numBytes                   /* Number of bytes written into the buffer */
    );

/***************************************************************************
Summary:
Flush Source FIFO Buffers
***************************************************************************/
BERR_Code BAPE_SfifoGroup_P_Flush(
    BAPE_SfifoGroupHandle handle
    );

/***************************************************************************
Summary:
Get num queued bytes
***************************************************************************/
BERR_Code BAPE_SfifoGroup_P_GetQueuedBytes(
    BAPE_SfifoGroupHandle handle,
    unsigned *pQueuedBytes
    );

/***************************************************************************
Summary:
Get sfifo write ptr
***************************************************************************/
void BAPE_SfifoGroup_P_GetReadAddress(
    BAPE_SfifoGroupHandle handle,
    unsigned chPair,       /*0,1,2,3*/
    unsigned bufferNum,     /*0,1*/
    uint32_t *pReadPtr
    );

/***************************************************************************
Summary:
Enable/Disable a source channel freemark interrupt
***************************************************************************/
BERR_Code BAPE_SfifoGroup_P_SetFreemarkInterrupt(
    BAPE_SfifoGroupHandle handle,
    BINT_CallbackFunc callback_isr,
    void *pParam1,
    int param2
    );

/***************************************************************************
Summary:
Re-Arm the SFIFO Group Freemark Interrupt
***************************************************************************/
void BAPE_SfifoGroup_P_RearmFreemarkInterrupt(
    BAPE_SfifoGroupHandle handle
    );

/***************************************************************************
Summary:
Get Hardware Index for a particular SFIFO

Description:
This function should be avoided unless absolutely necessary
(e.g. setting up DSP <-> FMM Linkage)
***************************************************************************/
uint32_t BAPE_SfifoGroup_P_GetHwIndex(
    BAPE_SfifoGroupHandle handle,
    BAPE_ChannelPair channelPair
    );

/***************************************************************************
Summary:
Get Hardware Index for a particular SFIFO's Adaptive Rate Controller

Description:
This function should be avoided unless absolutely necessary
(e.g. setting up DSP <-> FMM Linkage)
***************************************************************************/
uint32_t BAPE_SfifoGroup_P_GetAdaptRateWrcntAddress(
    BAPE_SfifoGroupHandle handle,
    BAPE_ChannelPair channelPair
    );

/***************************************************************************
Summary:
Destination FIFO Group Create Settings
***************************************************************************/
typedef struct BAPE_DfifoGroupCreateSettings
{
    unsigned numChannelPairs;   /* Number of channel pairs */
} BAPE_DfifoGroupCreateSettings;

/***************************************************************************
Summary:
Get default DFIFO Group Create Settings
***************************************************************************/
void BAPE_DfifoGroup_P_GetDefaultCreateSettings(
    BAPE_DfifoGroupCreateSettings *pSettings    /* [out] */
    );

/***************************************************************************
Summary:
Create DFIFO Group
***************************************************************************/
BERR_Code BAPE_DfifoGroup_P_Create(
    BAPE_Handle deviceHandle,
    const BAPE_DfifoGroupCreateSettings *pSettings,
    BAPE_DfifoGroupHandle *pHandle  /* [out] */
    );

/***************************************************************************
Summary:
Destroy DFIFO Group
***************************************************************************/
void BAPE_DfifoGroup_P_Destroy(
    BAPE_DfifoGroupHandle handle
    );

/***************************************************************************
Summary:
DFIFO Settings
***************************************************************************/
typedef struct BAPE_DfifoGroupSettings
{
    BAPE_FciIdGroup input;              /* Input FCI ID */
    BAPE_SfifoGroupHandle linkedSfifo;  /* If not NULL, linked source FIFO */
    bool bypassMemory;                  /* If true and a linked SFIFO is provided, bypass memory and directly connect the FIFOs */
    bool highPriority;                  /* If sample rate is >= 96000 set this to true. */
    bool interleaveData;                /* If true, data will be interleaved into a single buffer per channel pair */
    bool reverseEndian;                 /* If true, data will be endian swapped during capture */
    unsigned dataWidth;                 /* Data width in bits.  32 is supported on all platforms, 16 if BAPE_CHIP_DFIFO_SUPPORTS_16BIT_CAPTURE is defined to 1 */
    struct
    {
        uint32_t base;                  /* Base offset of the buffer */
        uint32_t length;                /* Length of the buffer in bytes */
        uint32_t watermark;             /* Watermark interrupt threshold in bytes */
    } bufferInfo[BAPE_Channel_eMax];
} BAPE_DfifoGroupSettings;

/***************************************************************************
Summary:
Get Destination FIFO Settings
***************************************************************************/
void BAPE_DfifoGroup_P_GetSettings(
    BAPE_DfifoGroupHandle handle,
    BAPE_DfifoGroupSettings *pSettings  /* [out] */
    );

/***************************************************************************
Summary:
Set Destination FIFO Settings
***************************************************************************/
BERR_Code BAPE_DfifoGroup_P_SetSettings(
    BAPE_DfifoGroupHandle handle,
    const BAPE_DfifoGroupSettings *pSettings
    );

/***************************************************************************
Summary:
Start DFIFO group
***************************************************************************/
BERR_Code BAPE_DfifoGroup_P_Start(
    BAPE_DfifoGroupHandle handle,
    bool enableOnly                 /* If true, a separate call to BAPE_DfifoGroup_P_Run_isr is required to
                                       start data flow.  If false, data flow will start immediately. */
    );

/***************************************************************************
Summary:
Stop DFIFO group
***************************************************************************/
void BAPE_DfifoGroup_P_Stop(
    BAPE_DfifoGroupHandle handle
    );

/***************************************************************************
Summary:
Start DFIFO group data flow

Description:
This call is only required if enableOnly was set to true in BAPE_DfifoGroup_P_Start
or if BAPE_DfifoGroup_P_Halt_isr was previously called.
***************************************************************************/
void BAPE_DfifoGroup_P_Run_isr(
    BAPE_DfifoGroupHandle handle
    );

/***************************************************************************
Summary:
Stop DFIFO group data flow

Description:
This call allows data flow to be stopped from an interrupt if required.
***************************************************************************/
void BAPE_DfifoGroup_P_Halt_isr(
    BAPE_DfifoGroupHandle handle
    );

/***************************************************************************
Summary:
Get DFIFO Buffers
***************************************************************************/
BERR_Code BAPE_DfifoGroup_P_GetBuffer(
    BAPE_DfifoGroupHandle handle,
    BAPE_BufferDescriptor *pBuffers      /* [out] */
    );

/***************************************************************************
Summary:
Commit data to DFIFO Buffers
***************************************************************************/
BERR_Code BAPE_DfifoGroup_P_CommitData_isr(
    BAPE_DfifoGroupHandle handle,
    unsigned numBytes                   /* Number of bytes written into the buffer */
    );

/***************************************************************************
Summary:
Flush DFIFO Buffers
***************************************************************************/
BERR_Code BAPE_DfifoGroup_P_Flush_isr(
    BAPE_DfifoGroupHandle handle
    );

/***************************************************************************
Summary:
Enable/Disable a DFIFO Fullmark Interrupt
***************************************************************************/
BERR_Code BAPE_DfifoGroup_P_SetFullmarkInterrupt(
    BAPE_DfifoGroupHandle handle,
    BINT_CallbackFunc callback_isr,
    void *pParam1,
    int param2
    );

/***************************************************************************
Summary:
Re-Arm the Dfifo Group Fullmark Interrupt
***************************************************************************/
void BAPE_DfifoGroup_P_RearmFullmarkInterrupt_isr(
    BAPE_DfifoGroupHandle handle
    );

/***************************************************************************
Summary:
Enable/Disable a DFIFO Overflow Interrupt
***************************************************************************/
BERR_Code BAPE_DfifoGroup_P_SetOverflowInterrupt(
    BAPE_DfifoGroupHandle handle,
    BINT_CallbackFunc callback_isr,
    void *pParam1,
    int param2
    );

/***************************************************************************
Summary:
Re-Arm the Dfifo Group Overflow Interrupt
***************************************************************************/
void BAPE_DfifoGroup_P_RearmOverflowInterrupt_isr(
    BAPE_DfifoGroupHandle handle
    );

/***************************************************************************
Summary:
Get Hardware Index for a particular DFIFO

Description:
This function should be avoided unless absolutely necessary
(e.g. setting up DSP <-> FMM Linkage)
***************************************************************************/
uint32_t BAPE_DfifoGroup_P_GetHwIndex(
    BAPE_DfifoGroupHandle handle,
    BAPE_ChannelPair channelPair
    );

/***************************************************************************
Summary:
SRC Operation Mode
***************************************************************************/
typedef enum BAPE_SrcMode
{
    BAPE_SrcMode_eBypass,
    BAPE_SrcMode_eLinear,
    BAPE_SrcMode_eIir,
    BAPE_SrcMode_eUp2,
    BAPE_SrcMode_eDown2,
    BAPE_SrcMode_eUp4,
    BAPE_SrcMode_eDown4,
    BAPE_SrcMode_eMax
} BAPE_SrcMode;

/***************************************************************************
Summary:
IIR Coefficients
***************************************************************************/

typedef struct BAPE_SRC_P_IIRCoeff
{
#ifdef BAPE_CHIP_P_MAX_IIR_FILTERS_PER_SRC
    int32_t b0[BAPE_CHIP_P_MAX_IIR_FILTERS_PER_SRC];
    int32_t b1[BAPE_CHIP_P_MAX_IIR_FILTERS_PER_SRC];
    int32_t b2[BAPE_CHIP_P_MAX_IIR_FILTERS_PER_SRC];
    int32_t a1[BAPE_CHIP_P_MAX_IIR_FILTERS_PER_SRC];
    int32_t a2[BAPE_CHIP_P_MAX_IIR_FILTERS_PER_SRC];
#else
    int32_t dummy;
#endif
}BAPE_SRC_P_IIRCoeff;

/***************************************************************************
Summary:
SRC Coefficient Memory Descriptor
***************************************************************************/
typedef struct BAPE_SrcCoefficients
{
    uint32_t baseAddress;   /* Register Address Base */
    unsigned numCoefs;      /* Number of 4-byte Coefficient Entries */
} BAPE_SrcCoefficients;

/***************************************************************************
Summary:
SRC Coefficient Memory Parameters
***************************************************************************/
typedef struct BAPE_SrcCoefficientsSettings
{
    BAPE_SrcMode mode;      /* Mode Requiring the Coefficients - Typically this should only be Equalizer as
                               other modes are internally allocated when required. */
    union
    {
        struct
        {
            unsigned numIirBands; /* Used for Equalizer mode, ranges from 1..8 - default is 8 */
        } equalizer;
    }modeSettings;
} BAPE_SrcCoefficientsSettings;

/***************************************************************************
Summary:
Get Default SRC Coefficient Memory Parameters
***************************************************************************/
void BAPE_SrcCoefficients_P_GetDefaultSettings(
    BAPE_Handle deviceHandle,
    const BAPE_SrcCoefficientsSettings *pSettings
    );

/***************************************************************************
Summary:
Allocate SRC Coefficient Memory
***************************************************************************/
BERR_Code BAPE_SrcCoefficients_P_Allocate(
    BAPE_Handle deviceHandle,
    const BAPE_SrcCoefficientsSettings *pSettings,
    BAPE_SrcCoefficients *pCoefficients             /* [out] Coefficient status */
    );

/***************************************************************************
Summary:
Free SRC Coefficient Memory
***************************************************************************/
void BAPE_SrcCoefficients_P_Free(
    BAPE_Handle deviceHandle,
    BAPE_SrcCoefficients *pCoefficients             /* [modified] Coefficients will be released */
    );

typedef struct BAPE_SrcGroupCoefficients
{
    BAPE_SrcCoefficients    srcCoefficients[BAPE_ChannelPair_eMax];
}BAPE_SrcGroupCoefficients;
/***************************************************************************
Summary:
SRC Group Create Settings
***************************************************************************/
typedef struct BAPE_SrcGroupCreateSettings
{
    unsigned blockId;
    unsigned numChannelPairs;   /* Number of channel pairs */
    BAPE_SrcMode mode;          /* Default is Linear */
    BAPE_SrcGroupCoefficients *pCoefMemory[2];   /* Coefficient Memory. Two are required only for IIR if doubleBuffered is true, otherwise one.
                                                    A valid value means sharing of an already allocated memory */
    /* Equalizer Settings */
    struct
    {
        unsigned numIirBands;   /* Used for Equalizer mode, ranges from 1..8 - default is 8 */
        bool     rampEnable;    /* If true (default), the coefficients will be double-buffered for on-the-fly settings changes.
                                   If false, the coefficients are not double-buffered to save coefficient memory space. */
    } equalizerSettings;
} BAPE_SrcGroupCreateSettings;

/***************************************************************************
Summary:
Get Default SRC Group Create Settings
***************************************************************************/
void BAPE_SrcGroup_P_GetDefaultCreateSettings(
    BAPE_SrcGroupCreateSettings *pSettings   /* [out] */
    );

/***************************************************************************
Summary:
Create SRC Group
***************************************************************************/
BERR_Code BAPE_SrcGroup_P_Create(
    BAPE_Handle deviceHandle,
    const BAPE_SrcGroupCreateSettings *pSettings,
    BAPE_SrcGroupHandle *pHandle        /* [out] */
    );

/***************************************************************************
Summary:
Destroy SRC Group
***************************************************************************/
void BAPE_SrcGroup_P_Destroy(
    BAPE_SrcGroupHandle handle
    );

/***************************************************************************
Summary:
SRC Settings
***************************************************************************/
typedef struct BAPE_SrcGroupSettings
{
    bool bypass;                /* If true, bypass this SRC */
    bool highPriority;          /* Set to true if sampleRate >= 96000 */
    bool rampEnabled;           /* If true, ramp is enabled (set to false for compressed input) */
    bool startupRampEnabled;    /* If true, ramp initial samples after startup (only valid if rampEnabled is true) */
    BAPE_FciIdGroup input;      /* Input FCI ID */
} BAPE_SrcGroupSettings;

/***************************************************************************
Summary:
Get SRC Settings
***************************************************************************/
void BAPE_SrcGroup_P_GetSettings(
    BAPE_SrcGroupHandle handle,
    BAPE_SrcGroupSettings *pSettings    /* [out] */
    );

/***************************************************************************
Summary:
Set SRC Settings
***************************************************************************/
BERR_Code BAPE_SrcGroup_P_SetSettings(
    BAPE_SrcGroupHandle handle,
    const BAPE_SrcGroupSettings *pSettings
    );

/***************************************************************************
Summary:
Set SRC Sample Rate (used for Linear mode only)
***************************************************************************/
void BAPE_SrcGroup_P_SetSampleRate_isr(
    BAPE_SrcGroupHandle handle,
    unsigned inputRate,
    unsigned outputRate
    );

/***************************************************************************
Summary:
Set SRC Mute
***************************************************************************/
void BAPE_SrcGroup_P_SetMute_isr(
    BAPE_SrcGroupHandle handle,
    bool muted
    );

/***************************************************************************
Summary:
Start SRC
***************************************************************************/
BERR_Code BAPE_SrcGroup_P_Start(
    BAPE_SrcGroupHandle handle
    );

/***************************************************************************
Summary:
Stop SRC
***************************************************************************/
void BAPE_SrcGroup_P_Stop(
    BAPE_SrcGroupHandle handle
    );

/***************************************************************************
Summary:
Set Coefficient Index (used for Equalizer mode only)
***************************************************************************/
void BAPE_SrcGroup_P_SetCoefficientIndex_isr(
    BAPE_SrcGroupHandle handle,
    unsigned index
    );

/***************************************************************************
Summary:
Get FCI Connection IDs for this group
***************************************************************************/
void BAPE_SrcGroup_P_GetOutputFciIds(
    BAPE_SrcGroupHandle handle,
    BAPE_FciIdGroup *pFciGroup      /* [out] */
    );

/***************************************************************************
Summary:
Update Coefficients in the SRC Coefficients Memory
***************************************************************************/
void BAPE_SrcGroup_P_UpdateCoefficients_isr(
    BAPE_SrcGroupHandle src,
    BAPE_SRC_P_IIRCoeff *pCoeff,
    unsigned *pStepSize          /* NULL indicates No Ramping */
    );

/***************************************************************************
Summary:
Mixer Group Create Settings
***************************************************************************/
typedef struct BAPE_MixerGroupCreateSettings
{
    unsigned blockId;
    unsigned numChannelPairs;   /* Number of channel pairs */
} BAPE_MixerGroupCreateSettings;

/***************************************************************************
Summary:
Get Default Mixer Group Create Settings
***************************************************************************/
void BAPE_MixerGroup_P_GetDefaultCreateSettings(
    BAPE_MixerGroupCreateSettings *pSettings    /* [out] */
    );

/***************************************************************************
Summary:
Create a Mixer Group
***************************************************************************/
BERR_Code BAPE_MixerGroup_P_Create(
    BAPE_Handle deviceHandle,
    const BAPE_MixerGroupCreateSettings *pSettings,
    BAPE_MixerGroupHandle *pHandle  /* [out] */
    );

/***************************************************************************
Summary:
Destroy a Mixer Group
***************************************************************************/
void BAPE_MixerGroup_P_Destroy(
    BAPE_MixerGroupHandle handle
    );

/***************************************************************************
Summary:
Mixer Group Settings
***************************************************************************/
typedef struct BAPE_MixerGroupSettings
{
    bool highPriority;          /* Set to true if sampleRate >= 96000 */
    bool volumeControlEnabled;  /* Set to true to enable volume control, false otherwise.  */
    bool softLimitEnabled;      /* Set to true to enable soft limiting */
} BAPE_MixerGroupSettings;

/***************************************************************************
Summary:
Get Mixer Group Settings
***************************************************************************/
void BAPE_MixerGroup_P_GetSettings(
    BAPE_MixerGroupHandle handle,
    BAPE_MixerGroupSettings *pSettings  /* [out] */
    );

/***************************************************************************
Summary:
Set Mixer Group Settings
***************************************************************************/
BERR_Code BAPE_MixerGroup_P_SetSettings(
    BAPE_MixerGroupHandle handle,
    const BAPE_MixerGroupSettings *pSettings
    );

/***************************************************************************
Summary:
Mixer Group Input Settings
***************************************************************************/
typedef struct BAPE_MixerGroupInputSettings
{
    BAPE_FciIdGroup input;                              /* Can only be changed while stopped */
    uint32_t rampStep;                                  /* Coefficient Ramp Step value (increments this amount per sample) */
    int32_t  coefficients[BAPE_ChannelPair_eMax][2][2]; /* Entries in this table reflect scaling from the input channel to the output channel.
                                                           The first index is the channel pair.  The second index is the input channel, 0
                                                           for the first (left) channel and 1 for the second (right) channel.  The third index
                                                           is the output channel.  Default is to have BAPE_VOLUME_NORMAL for each [0][0] and [1][1]
                                                           coefficient and BAPE_VOLUME_MIN for all others.  This maps input channels to the same
                                                           output channel with no scaling. */
} BAPE_MixerGroupInputSettings;

/***************************************************************************
Summary:
Mixer Group Output Settings
***************************************************************************/
typedef struct BAPE_MixerGroupOutputSettings
{
    uint32_t coefficients[BAPE_Channel_eMax];   /* Output volume scaling per output channel.  Default is BAPE_VOLUME_NORMAL for all channels.
                                                   Ignored for compressed data.  Values are specified in 5.23 integers, so 0x800000 corresponds
                                                   to unity (BAPE_VOLUME_NORMAL). */
    bool volumeRampDisabled;                    /* If true, disable the output volume ramping. */
    bool muted;                                 /* Mute all output channels if true. */

    unsigned numChannelPairs;                   /* By default, this is 0 to use the same number of channel pairs as there are mixers in the group.
                                                   If you want to force a different option, pass that here, but take care that one of the two
                                                   mixer outputs is consuming all channel pairs or the hardware will pause. */
} BAPE_MixerGroupOutputSettings;

/***************************************************************************
Summary:
Get Mixer Group Input Settings
***************************************************************************/
void BAPE_MixerGroup_P_GetInputSettings(
    BAPE_MixerGroupHandle handle,
    unsigned inputIndex,
    BAPE_MixerGroupInputSettings *pSettings  /* [out] */
    );

/***************************************************************************
Summary:
Set Mixer Group Input Settings
***************************************************************************/
BERR_Code BAPE_MixerGroup_P_SetInputSettings(
    BAPE_MixerGroupHandle handle,
    unsigned inputIndex,
    const BAPE_MixerGroupInputSettings *pSettings
    );

/***************************************************************************
Summary:
Get the actual Dp Mixer id used by this index
***************************************************************************/
unsigned BAPE_MixerGroup_P_GetDpMixerId(
    BAPE_MixerGroupHandle handle,
    unsigned idx
    );

/***************************************************************************
Summary:
Start a Mixer Group Input
***************************************************************************/
BERR_Code BAPE_MixerGroup_P_StartInput(
    BAPE_MixerGroupHandle handle,
    unsigned inputIndex
    );

/***************************************************************************
Summary:
Stop a Mixer Group Input
***************************************************************************/
void BAPE_MixerGroup_P_StopInput(
    BAPE_MixerGroupHandle handle,
    unsigned inputIndex
    );

/***************************************************************************
Summary:
Wait for Ramping to complete in a Mixer Group
***************************************************************************/
BERR_Code BAPE_MixerGroup_P_WaitForRamping(
    BAPE_MixerGroupHandle handle
    );

/***************************************************************************
Summary:
Set the Samplerate for the Mixer Group.
***************************************************************************/
void BAPE_MixerGroup_P_SetRampSamplerate_isr(
    BAPE_MixerGroupHandle handle,
    unsigned sampleRate
    );

/***************************************************************************
Summary:
Get Mixer Group Output Settings
***************************************************************************/
void BAPE_MixerGroup_P_GetOutputSettings(
    BAPE_MixerGroupHandle handle,
    unsigned outputIndex,
    BAPE_MixerGroupOutputSettings *pSettings  /* [out] */
    );

/***************************************************************************
Summary:
Set Mixer Group Output Settings
***************************************************************************/
BERR_Code BAPE_MixerGroup_P_SetOutputSettings(
    BAPE_MixerGroupHandle handle,
    unsigned outputIndex,
    const BAPE_MixerGroupOutputSettings *pSettings
    );

/***************************************************************************
Summary:
Mixer Group Output Status
***************************************************************************/
typedef struct BAPE_MixerGroupOutputStatus
{
    bool rampActive[BAPE_Channel_eMax];
} BAPE_MixerGroupOutputStatus;

/***************************************************************************
Summary:
Set Mixer Group Output Status
***************************************************************************/
void BAPE_MixerGroup_P_GetOutputStatus(
    BAPE_MixerGroupHandle handle,
    unsigned outputIndex,
    BAPE_MixerGroupOutputStatus *pStatus    /* [out] */
    );

/***************************************************************************
Summary:
Start a Mixer Group Output
***************************************************************************/
BERR_Code BAPE_MixerGroup_P_StartOutput(
    BAPE_MixerGroupHandle handle,
    unsigned outputIndex
    );

/***************************************************************************
Summary:
Stop a Mixer Group Output
***************************************************************************/
void BAPE_MixerGroup_P_StopOutput(
    BAPE_MixerGroupHandle handle,
    unsigned outputIndex
    );

/***************************************************************************
Summary:
Get FCI IDs for a mixer output
***************************************************************************/
void BAPE_MixerGroup_P_GetOutputFciIds(
    BAPE_MixerGroupHandle handle,
    unsigned outputIndex,
    BAPE_FciIdGroup *pFciGroup      /* [out] */
    );

/***************************************************************************
Summary:
Apply mixer output volume (for mixers with downstream connections)
***************************************************************************/
BERR_Code BAPE_MixerGroup_P_ApplyOutputVolume(
    BAPE_MixerGroupHandle handle,
    const BAPE_OutputVolume * pVolume,
    const BAPE_FMT_Descriptor * pFormat,
    unsigned outputIndex
    );

/***************************************************************************
Summary:
Convert MixerFormat to number of channels
***************************************************************************/
unsigned BAPE_Mixer_P_MixerFormatToNumChannels(
    BAPE_MixerFormat format
    );

/***************************************************************************
Summary:
IOP Stream Settings
***************************************************************************/
typedef struct BAPE_IopStreamSettings
{
    unsigned resolution;    /* Resolution in bits.  Valid values are 16..24 */
    BAPE_FciId input;       /* Input FCI ID */
} BAPE_IopStreamSettings;

/***************************************************************************
Summary:
Get IOP Stream Settings
***************************************************************************/
void BAPE_Iop_P_GetStreamSettings(
    BAPE_Handle handle,
    unsigned streamId,
    BAPE_IopStreamSettings *pSettings   /* [out] */
    );

/***************************************************************************
Summary:
Set IOP Stream Settings
***************************************************************************/
BERR_Code BAPE_Iop_P_SetStreamSettings(
    BAPE_Handle handle,
    unsigned streamId,
    const BAPE_IopStreamSettings *pSettings
    );

/***************************************************************************
Summary:
Enable stream capture in IOP
***************************************************************************/
BERR_Code BAPE_Iop_P_EnableCapture(
    BAPE_Handle handle,
    unsigned baseCaptureId,
    unsigned numChannelPairs
    );

/***************************************************************************
Summary:
Disable stream capture in IOP
***************************************************************************/
void BAPE_Iop_P_DisableCapture(
    BAPE_Handle handle,
    unsigned baseCaptureId,
    unsigned numChannelPairs
    );

/***************************************************************************
Summary:
Loopback Create Settings
***************************************************************************/
typedef struct BAPE_LoopbackGroupCreateSettings
{
    unsigned numChannelPairs;
} BAPE_LoopbackGroupCreateSettings;

/***************************************************************************
Summary:
Get Default Loopback Create Settings
***************************************************************************/
void BAPE_LoopbackGroup_P_GetDefaultCreateSettings(
    BAPE_LoopbackGroupCreateSettings *pSettings         /* [out] */
    );

/***************************************************************************
Summary:
Create Loopback Group
***************************************************************************/
BERR_Code BAPE_LoopbackGroup_P_Create(
    BAPE_Handle deviceHandle,
    const BAPE_LoopbackGroupCreateSettings *pSettings,
    BAPE_LoopbackGroupHandle *pHandle   /* [out] */
    );

/***************************************************************************
Summary:
Destroy Loopback Group
***************************************************************************/
void BAPE_LoopbackGroup_P_Destroy(
    BAPE_LoopbackGroupHandle handle
    );

/***************************************************************************
Summary:
Loopback Settings
***************************************************************************/
typedef struct BAPE_LoopbackGroupSettings
{
    BAPE_FciIdGroup input;      /* Input FCI feeding the loopback from the FMM.
                                   Loopback will loop this data back and provides
                                   a capture FCI ID, that can be obtained
                                   via BAPE_LoopbackGroup_P_GetCaptureFciIds. */
    unsigned resolution;        /* Resolution in bits.  Valid values are 16..24 */

#if BAPE_CHIP_MAX_FS > 0
    unsigned fs;                    /* Which FS timing source will be used. */
#else
    BAPE_MclkSource mclkSource;     /* MCLK Source */
    unsigned pllChannel;            /* PLL Channel */
    unsigned mclkFreqToFsRatio;     /* Ratio of MCLK to sample clock */
#endif
} BAPE_LoopbackGroupSettings;

/***************************************************************************
Summary:
Get Loopback Settings
***************************************************************************/
void BAPE_LoopbackGroup_P_GetSettings_isr(
    BAPE_LoopbackGroupHandle handle,
    BAPE_LoopbackGroupSettings *pSettings    /* [out] */
    );

/***************************************************************************
Summary:
Set Loopback Settings
***************************************************************************/
BERR_Code BAPE_LoopbackGroup_P_SetSettings_isr(
    BAPE_LoopbackGroupHandle handle,
    const BAPE_LoopbackGroupSettings *pSettings
    );

/***************************************************************************
Summary:
Start Capture and Output, reads from input FCI ID in BAPE_LoopbackGroupSettings
***************************************************************************/
BERR_Code BAPE_LoopbackGroup_P_Start(
    BAPE_LoopbackGroupHandle handle
    );

/***************************************************************************
Summary:
Stop Output
***************************************************************************/
void BAPE_LoopbackGroup_P_Stop(
    BAPE_LoopbackGroupHandle handle
    );

/***************************************************************************
Summary:
Get Capture FCI For Loopback
***************************************************************************/
void BAPE_LoopbackGroup_P_GetCaptureFciIds(
    BAPE_LoopbackGroupHandle handle,
    BAPE_FciIdGroup *pGroup             /* [out] */
    );

/***************************************************************************
Summary:
Dummysink Create Settings
***************************************************************************/
typedef struct BAPE_DummysinkGroupCreateSettings
{
    unsigned numChannelPairs;
} BAPE_DummysinkGroupCreateSettings;

/***************************************************************************
Summary:
Get Default Dummysink Create Settings
***************************************************************************/
void BAPE_DummysinkGroup_P_GetDefaultCreateSettings(
    BAPE_DummysinkGroupCreateSettings *pSettings    /* [out] */
    );

/***************************************************************************
Summary:
Create Dummysink Group
***************************************************************************/
BERR_Code BAPE_DummysinkGroup_P_Create(
    BAPE_Handle deviceHandle,
    const BAPE_DummysinkGroupCreateSettings *pSettings,
    BAPE_DummysinkGroupHandle *pHandle              /* [out] */
    );

/***************************************************************************
Summary:
Destroy Dummysink Group
***************************************************************************/
void BAPE_DummysinkGroup_P_Destroy(
    BAPE_DummysinkGroupHandle handle
    );

/***************************************************************************
Summary:
Dummysink Settings
***************************************************************************/
typedef struct BAPE_DummysinkGroupSettings
{
    BAPE_FciIdGroup input;          /* Input FCI feeding the dummysink from the FMM. */
    unsigned resolution;            /* Resolution in bits.  Valid values are 16..24 */
#if BAPE_CHIP_MAX_FS > 0
    unsigned fs;                    /* Which FS timing source will be used. */
#else
    BAPE_MclkSource mclkSource;     /* MCLK Source */
    unsigned pllChannel;            /* PLL Channel */
    unsigned mclkFreqToFsRatio;     /* Ratio of MCLK to sample clock */
#endif
} BAPE_DummysinkGroupSettings;

/***************************************************************************
Summary:
Get Dummysink Settings
***************************************************************************/
void BAPE_DummysinkGroup_P_GetSettings_isrsafe(
    BAPE_DummysinkGroupHandle handle,
    BAPE_DummysinkGroupSettings *pSettings  /* [out] */
    );

/***************************************************************************
Summary:
Set Dummysink Settings
***************************************************************************/
BERR_Code BAPE_DummysinkGroup_P_SetSettings_isr(
    BAPE_DummysinkGroupHandle handle,
    const BAPE_DummysinkGroupSettings *pSettings
    );

/***************************************************************************
Summary:
Start Capture and Output, reads from input FCI ID in BAPE_DummysinkGroupSettings
***************************************************************************/
BERR_Code BAPE_DummysinkGroup_P_Start(
    BAPE_DummysinkGroupHandle handle
    );

/***************************************************************************
Summary:
Stop Output
***************************************************************************/
void BAPE_DummysinkGroup_P_Stop(
    BAPE_DummysinkGroupHandle handle
    );

/***************************************************************************
Summary:
Get enable params for a Dummy Output
***************************************************************************/
void BAPE_DummysinkGroup_P_GetOutputEnableParams(
    BAPE_DummysinkGroupHandle handle,
    bool enable,
    BAPE_OutputPort_P_EnableParams *pEnableParams        /* [out] */
    );

/***************************************************************************
Summary:
Get Dummysink Group handle
***************************************************************************/
BAPE_DummysinkGroupHandle BAPE_DummyOutput_P_GetDummysinkGroup(
    BAPE_DummyOutputHandle handle
    );

/***************************************************************************
Summary:
FCI Splitter Group Create Settings
***************************************************************************/
typedef struct BAPE_FciSplitterGroupCreateSettings
{
    unsigned numChannelPairs;   /* Number of channel pairs */
} BAPE_FciSplitterGroupCreateSettings;

/***************************************************************************
Summary:
Get Default FCI Splitter Group Create Settings
***************************************************************************/
void BAPE_FciSplitterGroup_P_GetDefaultCreateSettings(
    BAPE_FciSplitterGroupCreateSettings *pSettings   /* [out] */
    );

/***************************************************************************
Summary:
Create FCI Splitter Group
***************************************************************************/
BERR_Code BAPE_FciSplitterGroup_P_Create(
    BAPE_Handle deviceHandle,
    const BAPE_FciSplitterGroupCreateSettings *pSettings,
    BAPE_FciSplitterGroupHandle *pHandle        /* [out] */
    );

/***************************************************************************
Summary:
Destroy FCI Splitter Group
***************************************************************************/
void BAPE_FciSplitterGroup_P_Destroy(
    BAPE_FciSplitterGroupHandle handle
    );

/***************************************************************************
Summary:
FCI Splitter Settings
***************************************************************************/
typedef struct BAPE_FciSplitterGroupSettings
{
    BAPE_FciIdGroup input;      /* Input FCI ID */
    unsigned numChannelPairs;   /* Number of channel pairs required */
} BAPE_FciSplitterGroupSettings;

/***************************************************************************
Summary:
Get FCI Splitter Settings
***************************************************************************/
void BAPE_FciSplitterGroup_P_GetSettings(
    BAPE_FciSplitterGroupHandle handle,
    BAPE_FciSplitterGroupSettings *pSettings    /* [out] */
    );

/***************************************************************************
Summary:
Set FCI Splitter Settings
***************************************************************************/
BERR_Code BAPE_FciSplitterGroup_P_SetSettings(
    BAPE_FciSplitterGroupHandle handle,
    const BAPE_FciSplitterGroupSettings *pSettings
    );

/***************************************************************************
Summary:
Start the FCI Splitter
***************************************************************************/
BERR_Code BAPE_FciSplitterGroup_P_Start(
    BAPE_FciSplitterGroupHandle handle
    );

/***************************************************************************
Summary:
Stop the FCI Splitter
***************************************************************************/
void BAPE_FciSplitterGroup_P_Stop(
    BAPE_FciSplitterGroupHandle handle
    );

/***************************************************************************
Summary:
Create FCI Splitter Output Group
***************************************************************************/
BERR_Code BAPE_FciSplitterOutputGroup_P_Create(
    BAPE_FciSplitterGroupHandle handle,
    BAPE_FciSplitterGroupCreateSettings *pSettings,
    BAPE_FciSplitterOutputGroupHandle *pHandle        /* [out] */
    );

/***************************************************************************
Summary:
Destroy FCI Splitter Output Group
***************************************************************************/
void BAPE_FciSplitterOutputGroup_P_Destroy(
    BAPE_FciSplitterOutputGroupHandle handle
    );

/***************************************************************************
Summary:
Get FCI Connection IDs for this FCI Splitter Output Group
***************************************************************************/
void BAPE_FciSplitterOutputGroup_P_GetOutputFciIds(
    BAPE_FciSplitterOutputGroupHandle handle,
    BAPE_FciIdGroup *pFciGroup      /* [out] */
    );

/***************************************************************************
Summary:
Initialize the FCI Splitter HW
***************************************************************************/
void BAPE_FciSplitter_P_InitHw(BAPE_Handle handle);

#endif /* #ifndef BAPE_FMM_PRIV_H_ */
