/******************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/

#ifndef BDSP_SOFT_FMM_STRUCT_H_
#define BDSP_SOFT_FMM_STRUCT_H_

#define MAX_FMM_INPUT_PORT (2)

/***************************************************************************
Summary:
Interrupt Handlers for Soft FMM Output
***************************************************************************/
typedef struct BDSP_SoftFMMOutputInterruptHandlers
{
    /* Interrupt fires when Output is written */
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2, uint32_t ui32NumBytesWritten);
        void *pParam1;
        int param2;
    } dataReady;
}BDSP_SoftFMMOutputInterruptHandlers;

/***************************************************************************
Summary:
Interrupt Handlers for Soft FMM Input
***************************************************************************/
typedef struct BDSP_SoftFMMInputInterruptHandlers
{
    /* Interrupt fires when Input data is consumed */
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2, uint32_t ui32NumBytesConsumed);
        void *pParam1;
        int param2;
    } freeAvailable;
}BDSP_SoftFMMInputInterruptHandlers;

typedef struct BDSP_SoftFmm_SPDIFPacketizationConfigParams {
    /*0 -> Disable, will work as passthrough
    1 -> Enable*/
    /*default Disable(0)*/
    uint32_t        ui32Enable;

    /*Type of burst to be insrted in case of discontinuity*/
    BDSP_AF_P_BurstFillType eBurstFillType;

    /*Controls whether first pause burst after underflow and last pause before switching to null have the stop bit set or cleared.
    One common scenario when finished sending a compressed stream is to insert a stop pause burst after the last good
    frame and then continue to insert null bursts while idle. This can be done by setting STOP=1, TYPE=Null and REP_PERIOD=PER_4096 then letting the SDRAM buffer underflow.
    1 = Enable
    0 = Disable
    Default Disable.*/
    uint32_t ui32StopBit;

    /*Burst period in sample*/
    uint32_t        ui32BurstPeriod;

    /*Burst padding*/
    uint32_t ui32BurstPadding;

    /*whther to add the length in byte or bits*/
    uint32_t        uiLengthCode;

    /*Number of bits per in sample for PCM mode
    default: 24*/
    uint32_t        ui32BitsPerSample;


    /*Whether the Supplied channel status bits are valid, If valid channel status bits will be used*/
    /*0 invalid, 1 valid*/
    /*default invalid (0)*/
    uint32_t        ui32ChannelStatusValid;

    /*Specifies the channel status bits 0 to 31 for this stream.
    channel status bits [23:20] can be different for right and left.
    This register controls left[23:20]. bits 31:28 of ui32ChannelStatus_2 specifies the values for right[23:20]
    default: 0x0.*/
    /*if channel status bits are valid, these will be inserted*/
    uint32_t        ui32ChannelStatus_0;
    /*Specifies the channel status bits 32 to 63 for this stream.
    If channel status bits are valid, these will be inserted*
    default: 0x0.*/

    uint32_t        ui32ChannelStatus_1;
    /*Specifies channel status bits 64 to 91 for this stream.
    bits 31:28 specifies channel status bits [23:20] for the right channel of this stream.
    if channel status bits are valid, these will be inserted
    default: 0x0.*/
    uint32_t        ui32ChannelStatus_2;
} BDSP_SoftFmm_SPDIFPacketizationConfigParams;


typedef enum BDSP_FMM_OutputType
{
    BDSP_SoftFMM_Output_eMAI,
    BDSP_SoftFMM_Output_eSPDIF,
    BDSP_SoftFMM_Output_eDAC,
    BDSP_SoftFMM_Output_eI2S,
    BDSP_SoftFMM_Output_eDSP,
    BDSP_SoftFMM_Output_eCapture,
    BDSP_SoftFMM_Output_eMax,
    BDSP_SoftFMM_Output_eInvalid = 0x7FFFFFFF
}BDSP_SoftFMM_OutputType;


typedef struct BDSP_SoftFMM_OutputSettings
{
    uint32_t                               ui32Enable;
    BDSP_SoftFMM_OutputType                eSoftFMMOutputType;
    uint32_t                               ui32SampleRate;
    BDSP_SoftFmm_SPDIFPacketizationConfigParams sSPDIFPacketizationConfig;
}BDSP_SoftFMM_OutputSettings;


typedef struct BDSP_SoftFmm_ZeroInsertionConfigParams {
    /*0 -> Disable, 1 -> Enable*/
    /*default Disable(0)*/
    uint32_t ui32Enable;

    /*0 -> Disable ramp down in zero insertion
    1 -> Enable ramp down in zero insertion*/
    /*default Disable(0)*/
    uint32_t ui32RampDownEnable;

    /*Ramping step for Ramp down in 9.23 format*/
    /*Specified for each channel*/
    int32_t ui32RampDownStep[BDSP_AF_P_MAX_CHANNELS];

    /*0 -> Disable ramp up after zero insertion
    1 -> Enable ramp up after zero insertion*/
    /*default Disable(0)*/
    uint32_t ui32RampUpEnable;

    /*Ramping step for Ramp up in 9.23 format*/
    /*Specified for each channel*/
    int32_t ui32RampUpStep[BDSP_AF_P_MAX_CHANNELS];

} BDSP_SoftFmm_ZeroInsertionConfigParams;

typedef struct BDSP_SoftFmm_SrcConfigParams {
    /*0 -> Disable, 1 -> Enable*/
    /*default Disable(0)*/
    uint32_t ui32Enable;

} BDSP_SoftFmm_SrcConfigParams;


typedef struct BDSP_SoftFmm_RateControlConfigParams {
    /*0 -> Disable, 1 -> Enable*/
    /*default Disable(0)*/
    uint32_t ui32Enable;

    /* 0- No Correction (default), 1-DSOLA, 2-SOFT-PPM*
    No Correction: Input will be bypassed. This mode is just bypass tight lipsync correction.
    This mode may get used for certification.

    Soft PPM: In this mode, frame will get compress/expand slowly based on PTS and STC diff.
    If the STC and PTS diff is well within the bound, input will be bypassed to output.*/

    uint32_t ui32TsmCorrectionMode;

    /*Window size for Tri-windowing for repeat and drop*/
    /*can take value between 16 and 1024 (both inclusive) which should be power of two*/
    /*default 256*/

    uint32_t ui32WindowSize;

    /*Threshhold (time in milli seconds ), for triggerring rate control*/
    /*default 2 ms*/
    uint32_t ui32RateControlThresholdInMs;


    /*Location where STC phase information is passed*/
    uint64_t ui64RateControlCfgAddress;
} BDSP_SoftFmm_RateControlConfigParams;

typedef struct BDSP_SoftFmm_MixerConfigParams {
    /*0 -> Disable, 1 -> Enable, when disable zero will be put to output*/
    /*default Disable(0)*/
    uint32_t ui32Enable;

    /*Down mixing coefficients , specified in 9.23 format*/
    /*DownmixCoef[i][j] specifies the contribution of i-th input channel to j-th output channel*/
    uint32_t ui32MixingCoef[BDSP_AF_P_MAX_CHANNELS][BDSP_AF_P_MAX_CHANNELS];
} BDSP_SoftFmm_MixerConfigParams;

typedef struct BDSP_SoftFMM_InputSettings
{

    uint32_t ui32SampleRate;
    BDSP_SoftFMMBufferDescriptor            sSoftFMMInputBufferDescriptor;
    BDSP_SoftFmm_ZeroInsertionConfigParams  sZeroInsertionConfigParams;
    BDSP_SoftFmm_SrcConfigParams            sSrcConfigParams;
    BDSP_SoftFmm_RateControlConfigParams    sRateControlConfigParams;
    BDSP_SoftFmm_MixerConfigParams          sMixerConfigParams;

}BDSP_SoftFMM_InputSettings;

typedef struct BDSP_SoftFmm_InputPortConfig {
    /*Speifies the associated input Port*/
    uint32_t ui32PortIndex;

    uint32_t ui32SampleRate;
    BDSP_SoftFmm_ZeroInsertionConfigParams  sZeroInsertionConfigParams;
    BDSP_SoftFmm_SrcConfigParams            sSrcConfigParams;
    BDSP_SoftFmm_RateControlConfigParams    sRateControlConfigParams;
    BDSP_SoftFmm_MixerConfigParams          sMixerConfigParams;
} BDSP_SoftFmm_InputPortConfig;

typedef struct BDSP_SoftFmm_VolumeControlConfigParams {
    /*0 -> Disable, 1 -> Enable*/
    /*default Disable(0)*/
    uint32_t ui32Enable;

    /*0 -> disable, 1 -> enable. when enabled, output will be muted*/
    /*default disable*/
    uint32_t ui32MuteEnable;

    /*whether Ramping should be enabled for output, 0 -> disable, 1 -> enable, default -> diable*/
    /*Specified for each channel*/
    uint32_t ui32RampEnable[BDSP_AF_P_MAX_CHANNELS];

    /*Ramping step for achieving desired volume, specified in 9.23 format*/
    /*Specified for each channel*/
    int32_t ui32RampStep[BDSP_AF_P_MAX_CHANNELS];

    /*Desired volume level, specified in 9.23 format*/
    /*Specified for each channel*/
    uint32_t ui32DesiredVolume[BDSP_AF_P_MAX_CHANNELS];
} BDSP_SoftFmm_VolumeControlConfigParams;

/* Burst Type defines*/
typedef enum {
    BDSP_eBurstTypeNull     = 0, /*Insert NULL burst*/
    BDSP_eBurstTypePause	= 1, /*Insert Pause Burst*/
    BDSP_eBurstTypeZero	= 2, /*Insert Zero burst, here all fields of payload of SPDIF frame will be zero. It will still carry channel status bits. It's equivalent to DATA_ENABLE of AUD_FMM_IOP_OUT_SPDIF_0_SPDIF_FORMAT_CFG register */

    BDSP_eBurstType_INVALID =    0x7FFFFFFF
} BDSP_eBurstType;

/* SPDIF length codedefines */
typedef enum {
	eSpdifLengthCode_BITS	 = 0,
	eSpdifLengthCode_BYTES	 = 1,

	eSpdifLengthCode_INVALID =	0x7FFFFFFF
} eSpdifLengthCode;

typedef struct BDSP_SoftFmm_HDMIPacketizationConfigParams {
    /*0 -> Disable, will work as passthrough
    1 -> Enable*/
    /*default Disable(0)*/
    uint32_t        ui32Enable;

    /*Type of burst to be insrted in case of discontinuity*/
    BDSP_eBurstType eBurstType;

    /*Controls whether first pause burst after underflow and last pause before switching to null have the stop bit set or cleared.
    One common scenario when finished sending a compressed stream is to insert a stop pause burst after the last good
    frame and then continue to insert null bursts while idle. This can be done by setting STOP=1, TYPE=Null and REP_PERIOD=PER_4096 then letting the SDRAM buffer underflow.
    1 = Enable
    0 = Disable
    Default Disable.*/
    uint32_t ui32StopBit;

    /*Burst period in sample*/
    uint32_t        ui32BurstPeriod;

    /*whther to add the length in byte or bits*/
    /*0 -> in bits*/
    /*1 -> in byte*/
    /*Default 0*/
    uint32_t        uiLengthCode;

    /*Number of bits per in sample for PCM mode
    default: 24*/
    uint32_t        ui32BitsPerSample;


    /*Whether the Supplied channel status bits are valid, If valid channel status bits will be used*/
    /*0 invalid, 1 valid*/
    /*default invalid (0)*/
    uint32_t		ui32ChannelStatusValid;

    /*Specifies the channel status bits 0 to 31 for this stream.
    channel status bits [23:20] can be different for right and left.
    This register controls left[23:20]. bits 31:28 of ui32ChannelStatus_2 specifies the values for right[23:20]
    default: 0x0.*/
    /*if channel status bits are valid, these will be inserted*/
    uint32_t		ui32ChannelStatus_0;
    /*Specifies the channel status bits 32 to 63 for this stream.
    If channel status bits are valid, these will be inserted*
    default: 0x0.*/

    uint32_t		ui32ChannelStatus_1;
    /*Specifies channel status bits 64 to 91 for this stream.
    bits 31:28 specifies channel status bits [23:20] for the right channel of this stream.
    if channel status bits are valid, these will be inserted
    default: 0x0.*/
    uint32_t		ui32ChannelStatus_2;

    /*Stuffing bits to be used after null/pause burst*/
    uint32_t		ui32BurstPadding;
} BDSP_SoftFmm_HDMIPacketizationConfigParams;

typedef struct BDSP_SoftFMM_MixerSettings
{
    uint32_t                                    ui32SampleRate;
	/*BDSP_DataType		                        eDataType;*/
    BDSP_SoftFmm_VolumeControlConfigParams      sVolumeControlConfigParams;
}BDSP_SoftFMM_MixerSettings;


typedef struct BDSP_SoftFMM_Output_HWConfig
{
    /* Indicates whether the Output HW Configuration is valid */
    /* 1 - valid. 0 - invalid */
    uint32_t ui32valid;

    /* Address of the STC Priming Count Register */
    /* The Initial zero fill size is written into this register by the SoftFMM firmware*/
    uint64_t stcPrimingCountRegAddr;

    /* Address of the STC Snapshot Count Register*/
    /* This register is read by the SoftFMM firmware to get the output buffer depth*/
    uint64_t stcSnapshotCntrRegAddr;

    /* Address of the STC Frame Size Register*/
    /*The frame size in samples is written into this register by the SoftFMM firmware whenever a frame is written to output*/
    uint64_t stcFrameSizeRegAddr;

    /* Address of the STC Snapshot Control Register*/
    /* This is the register which controls the loading of Initial Zero fill from STC Priming Count Register to the STC Snapshot Count Register*/
    uint64_t stcSnapshotCtrlRegAddr;

    /* Address of the SC Control Register*/
    /* This register controls the start/stop of the STC Snapshot Count Register.*/
    /* SoftFMM firmware will use this register to start decrementing the STC Snapshot Count once it writes the first frame into the output*/
    uint64_t scCtrlRegAddr;
}BDSP_SoftFMM_Output_HWConfig;


typedef struct BDSP_SoftFmmConfigParams {
    /*0 -> Disable, 1 -> Enable*/
    /*default Disable(0)*/
    uint32_t ui3EnableOutput;
    uint32_t ui32OutputSampleRate;

    BDSP_SoftFMM_Output_HWConfig                sOutputHWConfig;
    BDSP_SoftFmm_InputPortConfig                sInputPortConfig[2];
    BDSP_SoftFmm_VolumeControlConfigParams      sVolumeControlConfigParams;
    BDSP_SoftFmm_HDMIPacketizationConfigParams  sHDMIPacketizationConfigParams;
} BDSP_SoftFmmConfigParams;


typedef struct BDSP_SoftFMM_RateControlStatusInfo {
    /* PTS Value as it is from the stream */
    uint32_t                        ui32PTS;

    /* 0 - Invalid and 1 - Valid. Any Other Value - Invalid */
    uint32_t                        ui32PTSValid;

    /* This Field Represents PTS types as 0-Coded, 1-INTERPOLATED, 2-INTERPOLATED from INVALID PTS */
    uint32_t                        ui32PTSType;

    /* This Field Represents the time (in msec with 45 KHz running clock) ajusted.
    * Negative Value represents that the frame is shorten which is possible when PTS is behind STC.*/
    int32_t                         i32TimeInMsecAdjusted;

    /* This Field Represents the sample ajusted.
    * Negative Value represents that the frame is shorten which is possible when PTS is behind STC.*/
    int32_t                         i32SamplesAdjusted;

    /* This Field Represents the time (in msec with 45 KHz running clock) by which we still need to adjust TSM.*/
    /* Negative Value represents that the frame is shorten which is possible when PTS is behind STC.*/
    int32_t                         i32TimeInMsecYetToAdjust;


    /* This is time snapshot of unadulterated 45KHz timer */
    uint32_t                        ui32TimeSnapshot45KHz;

    /* This is diff between STC and PTS represented in msec at 45 KHz clock */
    uint32_t                        ui32StcPtsPhaseDiffInT45;

    /* This represents the STC and PTS position. Possible values: 0 or 1, 0 - (STC > PTS),  1 - (STC < PTS) */
    uint32_t                        ui32StcBehindPtsStatus;

    /* ui32StatusValid=1 indicates status as "valid"
    ui32StatusValid = other than 1 indicates status as "in-valid" */
    uint32_t                        ui32StatusValid;
} BDSP_SoftFMM_RateControlStatusInfo;

typedef struct BDSP_SoftFMM_VolumeControlStatusInfo {
    uint32_t ui32CurrentVolume;
} BDSP_SoftFMM_VolumeControlStatusInfo;

typedef struct BDSP_SoftFMM_ZeroInsertionStatusInfo {
    uint32_t ui32RampInProgress;
    uint32_t ui32RampDownInProgress;
    uint32_t ui32RampUpInProgress;
} BDSP_SoftFMM_ZeroInsertionStatusInfo;

typedef struct BDSP_SoftFMM_MixerStatusInfo {
    uint32_t ui32MixerEnable;
} BDSP_SoftFMM_MixerStatusInfo;

typedef struct BDSP_SoftFMM_SrcStatusInfo {
    uint32_t ui32MixerEnable;
} BDSP_SoftFMM_SrcStatusInfo;

typedef struct BDSP_SoftFMM_OutputStatus
{
    uint32_t ui32Enable;
}BDSP_SoftFMM_OutputStatus;


typedef struct BDSP_SoftFMM_InputStatus
{
    BDSP_SoftFMM_RateControlStatusInfo      sRateControlStatusInfo;
    BDSP_SoftFMM_ZeroInsertionStatusInfo    sZeroInsertionStatusInfo;
}BDSP_SoftFMM_InputStatus;

typedef struct BDSP_SoftFMM_MixerStatus
{
    uint32_t                                ui32NumSampleConsumed;
    uint32_t                                ui32NumSampleGenerated;
    BDSP_SoftFMM_VolumeControlStatusInfo    sVolumeControlStatusInfo;
}BDSP_SoftFMM_MixerStatus;

typedef struct BDSP_SoftFmm_InputPortStatusInfo {
    /*Speifies the associated input Port*/
    uint32_t ui32PortIndex;

    BDSP_SoftFMM_ZeroInsertionStatusInfo	sZeroInsertionStatusInfo;
    BDSP_SoftFMM_SrcStatusInfo            	sSrcStatusInfo;
    BDSP_SoftFMM_RateControlStatusInfo	    sRateControlStatusInfo;
} BDSP_SoftFmm_InputPortStatusInfo;

typedef struct BDSP_SoftFmm_HDMIPacketizationStatusInfo {
    uint32_t                                ui32OutputEnable;
} BDSP_SoftFmm_HDMIPacketizationStatusInfo;

typedef struct BDSP_SoftFMMStatusInfo {
    uint32_t                                ui32OutputEnable;

    uint32_t                                ui32NumSampleConsumed;
    uint32_t                                ui32NumSampleGenerated;

    BDSP_SoftFmm_InputPortStatusInfo		 sInputPortStatusInfo[MAX_FMM_INPUT_PORT];
    BDSP_SoftFMM_MixerStatusInfo             sMixerStatusInfo;
    BDSP_SoftFMM_VolumeControlStatusInfo     sVolumeControlStatusInfo;
    BDSP_SoftFmm_HDMIPacketizationStatusInfo sHDMIPacketizationStatusInfo;
} BDSP_SoftFMMStatusInfo;

#endif /* BDSP_SOFT_FMM_STRUCT_H_ */
