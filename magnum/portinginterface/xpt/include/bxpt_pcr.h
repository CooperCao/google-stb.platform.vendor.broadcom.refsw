/***************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 * Module Description:
 *
 ***************************************************************************/

#ifndef BXPT_PCR_H__
#define BXPT_PCR_H__

#include "bxpt.h"
#include "bchp_common.h"
#ifdef BCHP_XPT_DPCR0_REG_START
#include "bchp_xpt_dpcr0.h"
#endif

#ifdef __cplusplus
extern "C"{
#endif

/*=************************ Module Overview ********************************
<verbatim>
Overview:
</verbatim>
***************************************************************************/

/***************************************************************************
Summary:
    The opaque handle for a PCR module.
Description:
    This structure represents the PCR handle. It is created at BXPT_PCR_Open
***************************************************************************/
typedef struct BXPT_P_PcrHandle_Impl *BXPT_PCR_Handle;

/***************************************************************************
Summary:
    Time base frequency reference control configuration.  It is for non-transport
    timebase reference.
Description:
    The members in the structure controls the frequency of a timebase reference.
    It appies to analog references and the internal reference. This is not for
    timebase pulse frequency which is controlled in rate managers of different
    hardware blocks.
***************************************************************************/
typedef struct BXPT_PCR_TimebaseFreqRefConfig
{
    uint32_t Prescale;   /* frequency control parm*/
    uint32_t Inc;        /* frequency control parm*/
    uint8_t FiltA;          /* integrator leak. */
    uint8_t FiltB;          /* loop gain. */
    uint8_t FiltC;          /* direct path gain. */
}
BXPT_PCR_TimebaseFreqRefConfig;

/***************************************************************************
Summary:
Different format for the packet timestamps supported by the PCR block.
***************************************************************************/
typedef enum
{
    BXPT_PCR_JitterTimestampMode_e32 = 0,       /* 32 bit timestamp */
    BXPT_PCR_JitterTimestampMode_e28_4P = 1,    /* 28 bit timestamp with 4 bit parity.*/
    BXPT_PCR_JitterTimestampMode_e30_2U = 2,     /* 30 bit timstamp with 2 user bits. */
    BXPT_PCR_JitterTimestampMode_eAuto,         /* Do not change the current (global) setting. */
    BXPT_PCR_JitterTimestampMode_eMax
}
BXPT_PCR_JitterTimestampMode;

/***************************************************************************
Summary:
***************************************************************************/
typedef enum
{
    BXPT_PCR_JitterCorrection_eEnable = 0,
    BXPT_PCR_JitterCorrection_eDisable = 1,
    BXPT_PCR_JitterCorrection_eAuto,        /* Do not change the current (global) setting. */
    BXPT_PCR_JitterCorrection_eMax
}
BXPT_PCR_JitterCorrection;

/***************************************************************************
Summary:
Configuration for PCR module when transport stream is timebase reference.

Description:
This structure covers pcr pid, stream source, PcrSendEn, and sendmode
configurations when locking to a transport stream.

If jitter adjust is enabled, the jitter adjustment block is supposed to be
programmed to match with the Playback timestamp format (i.e. 32-bit, 28-bit
or 30-bit formats). This tells the jitter calculation hardware whether to use
all 32-bits of the timestamp, or to ignore the upper 4 bits of the timestamp
(4-bit parity + 28-bit, or 2-bit User + 30-bit). The 32-bit support in the
DPCR is for both binary and Mod-300. The hardware uses the Packet type (MPEG
or 130 byte) to decide whether timestamp mode is Mod-300 or binary.

NOTE!! that timestamp mode and PCR type must match for jitter calculation to
work correctly, i.e. if it is MPEG stream with PCRs then the timestamps must
also be Mod-300. This means we can't use jitter adjustment for MPEG stream
with PCRs but with binary/DLNA timestamps.
***************************************************************************/
typedef struct BXPT_PCR_XptStreamPcrCfg
{
#if BXPT_DPCR_GLOBAL_PACKET_PROC_CTRL
    uint32_t PidChannel;                /* PCR PID channel */

    /*
    ** Note: these params are unique in that the hw setting is *global* to all bxpt_pcr instances.
    ** Thus, changing one param here will affect other blocks. This isn't a problem in the usage
    ** cases the hw engineers know about, but it requires some care by sw. These params are
    ** tri-state, as follows. _eAuto should be selected, unless the user knows this block must
    ** have a unique setting. At some point in the future, these params may no longer be global.
    **
    ** The tri-state values are:
    **      _eAuto - Do not change the existing global setting.
    **      _eEnable or _eDisable for PbJitterDisable and LiveJitterDisable.
    **      _e32, etc for JitterTimestamp.
    */
    BXPT_PCR_JitterTimestampMode JitterTimestamp;
    BXPT_PCR_JitterCorrection PbJitterDisable;
    BXPT_PCR_JitterCorrection LiveJitterDisable;
#else
    uint32_t Pid;                     /* PCR Pid */
    BXPT_DataSource eStreamSelect;    /* Pcr stream source. NOTE: only input bands are supported. */
    unsigned int WhichSource;         /* Which instance of the source */
#endif
    uint32_t MaxPcrError;             /* Maximum PCR error thredhold*/

    BXPT_PCR_TimebaseFreqRefConfig TimebaseCfg;     /* Customized filter coefficients. */
}
BXPT_PCR_XptStreamPcrCfg;

/***************************************************************************
Summary:
    Configurations for modifying the rate of the STC extension counter.
Description:
    The members in the structure sets the rate control of STC extension.
***************************************************************************/
typedef struct BXPT_PCR_STCExtRateConfig
{
    uint32_t Prescale;   /* frequency control parm*/
    uint32_t Inc;        /* frequency control parm*/
}
BXPT_PCR_STCExtRateConfig;

/***************************************************************************
Summary:
    Timebase references for all the sources that a timebase can lock to.
Description:
    This enum represents all the time references that timebase
    may lock to.
***************************************************************************/
typedef enum BXPT_PCR_TimeRef
{
#if BXPT_HAS_REDUCED_TIMEREFS
    /* Not available on all chips. Check your hw docs. */

    BXPT_PCR_TimeRef_eI656_Hl = 3,          /* Lock to Hsync from ITU656 input 0  */
    BXPT_PCR_TimeRef_eI656_Vl = 4,          /* Lock to Vsync from ITU656 input 0  */
    BXPT_PCR_TimeRef_eI656_Fl = 5,          /* Lock to Fsync from ITU656 input 0  */
    BXPT_PCR_TimeRef_eI2S0 = 6,             /* Lock to I2S input #0  */
    BXPT_PCR_TimeRef_eHD_DVI_H0 = 11,       /* Hsync from HD DVI input 0 */
    BXPT_PCR_TimeRef_eHD_DVI_V0 = 12,       /* Vsync from HD DVI input 0 */
    BXPT_PCR_TimeRef_eInternal = 14,        /* Lock to Internal Reference which has a period of 16384 cycles at 27MHz */
    BXPT_PCR_TimeRef_eXpt = 15,             /* Lock to transport PCR values */

    BXPT_PCR_TimeRef_eHDMI_Passthru_HL = 16, /* Lock to Hsync from DVP_HR for HDMI passthru mode */
    BXPT_PCR_TimeRef_eHDMI_Passthru_VL = 17, /* Lock to Vsync from DVP_HR for HDMI passthru mode */
    BXPT_PCR_TimeRef_eSPDIF = 18,           /* SPDIF input 1 */
    BXPT_PCR_TimeRef_eMaiIn = 19,           /* Frame Sync from DVP_HR */

    BXPT_PCR_TimeRef_eMax = 100,            /* Max value, marks the end of the supported inputs. */

    /* These are placeholders, in case hw for HD DVI input 1 is added in the future. */
    BXPT_PCR_TimeRef_eHD_DVI_H1 = 101,      /* Hsync from HD DVI input 1 */
    BXPT_PCR_TimeRef_eHD_DVI_V1 = 102       /* Vsync from HD DVI input 1 */

#else
    BXPT_PCR_TimeRef_eVDECHl = 0,           /* Lock to Hsync from  VDEC #0  */
    BXPT_PCR_TimeRef_eVDECVl = 1,           /* Lock to Vsync from VDEC #0 */
    BXPT_PCR_TimeRef_eVDECCl = 2,           /* Chroma lock to VDEC #0  */

    BXPT_PCR_TimeRef_eI656_Hl = 3,          /* Lock to Hsync from ITU656 input 0  */
    BXPT_PCR_TimeRef_eI656_Vl = 4,          /* Lock to Vsync from ITU656 input 0  */
    BXPT_PCR_TimeRef_eI656_Fl = 5,          /* Lock to Fsync from ITU656 input 0  */

    BXPT_PCR_TimeRef_eI2S0 = 6,             /* Lock to I2S input #0  */

    /* These time references are not available on all chips. The PI will return
       BERR_INVALID_PARAMETER if the reference isn't supported. */
    BXPT_PCR_TimeRef_eHD_DVI_V0 = 18,       /* Vsync from HD DVI input 0 */
    BXPT_PCR_TimeRef_eHD_DVI_H0 = 19,       /* Hsync from HD DVI input 0 */
    BXPT_PCR_TimeRef_eHD_DVI_V1 = 20,       /* Vsync from HD DVI input 1 */
    BXPT_PCR_TimeRef_eHD_DVI_H1 = 21,       /* Hsync from HD DVI input 1 */
    BXPT_PCR_TimeRef_eSPDIF = 22,           /* SPDIF input 1 */
    BXPT_PCR_TimeRef_eVDEC_SEC_HL = 26,     /* VDEC Hsync */
    BXPT_PCR_TimeRef_eVDEC_SEC_VL = 27,     /* VDEC Vsync */
    BXPT_PCR_TimeRef_eVDEC_SEC_CL = 28,     /* VDEC chroma */
    BXPT_PCR_TimeRef_eMaiIn = 29,           /* Frame Sync from DVP_HR */

    /* The remaining references are supported on all devices. */
    BXPT_PCR_TimeRef_eI2S1 = 7,             /* Lock to I2S input #0  */
    BXPT_PCR_TimeRef_eI656_Hl_1 = 8,        /* Lock to Hsync from ITU656 input 1  */
    BXPT_PCR_TimeRef_eI656_Vl_1 = 9,        /* Lock to Vsync from ITU656 input 1  */
    BXPT_PCR_TimeRef_eI656_Fl_1 = 10,       /* Lock to Fsync from ITU656 input 1  */

    BXPT_PCR_TimeRef_eInternal = 14,        /* Lock to Internal Reference which has a period of 16384 cycles at 27MHz */
    BXPT_PCR_TimeRef_eXpt = 15              /* Lock to transport PCR values */
#endif
}
BXPT_PCR_TimeRef;

/***************************************************************************
Summary:
    Timebase references possible track Range values
Description:
***************************************************************************/
typedef enum BXPT_PCR_RefTrackRange
{
    BXPT_PCR_TrackRange_PPM_8 = 0,
    BXPT_PCR_TrackRange_PPM_15,
    BXPT_PCR_TrackRange_PPM_30,
    BXPT_PCR_TrackRange_PPM_61,
    BXPT_PCR_TrackRange_PPM_122,
    BXPT_PCR_TrackRange_PPM_244

}BXPT_PCR_RefTrackRange;

/***************************************************************************
Summary:
Valid error clamping ranges for the phase error input.
***************************************************************************/
typedef enum BXPT_PCR_PhaseErrorClampRange
{
#ifdef BXPT_P_HAS_0_238_PPM_RESOLUTION
    /* Newer chips have more options for the clamping range. */
    BXPT_PCR_PhaseErrorClampRange_e22_8mS = 10,     /* +/- 22.8 mS */
    BXPT_PCR_PhaseErrorClampRange_e11_4mS = 9,      /* +/- 11.4 mS */
    BXPT_PCR_PhaseErrorClampRange_e5_7mS = 8,       /* +/- 5.7 mS */
    BXPT_PCR_PhaseErrorClampRange_e2_8mS = 7,       /* +/- 2.8 mS */
    BXPT_PCR_PhaseErrorClampRange_e1_4mS = 6,       /* +/- 1.4 mS */
    BXPT_PCR_PhaseErrorClampRange_e0_7mS = 5,       /* +/- -0.7 mS */
    BXPT_PCR_PhaseErrorClampRange_e0_18mS = 4,      /* +/- -0.18 mS */
    BXPT_PCR_PhaseErrorClampRange_e0_044mS = 3,     /* +/- -0.044 mS */
    BXPT_PCR_PhaseErrorClampRange_e0_019mS = 2,     /* +/- -0.019 mS */
    BXPT_PCR_PhaseErrorClampRange_e0_0047mS = 1,    /* +/- -0.0047 mS */
    BXPT_PCR_PhaseErrorClampRange_e0_0012mS = 0     /* +/- -0.0012 mS */
#else
    BXPT_PCR_PhaseErrorClampRange_e22_8mS = 5,      /* +/- 22.8 mS */
    BXPT_PCR_PhaseErrorClampRange_e11_4mS = 4,      /* +/- 11.4 mS */
    BXPT_PCR_PhaseErrorClampRange_e5_7mS = 3,       /* +/- 5.7 mS */
    BXPT_PCR_PhaseErrorClampRange_e2_8mS = 2,       /* +/- 2.8 mS */
    BXPT_PCR_PhaseErrorClampRange_e1_4mS = 1,       /* +/- 1.4 mS */
    BXPT_PCR_PhaseErrorClampRange_e0_7mS = 0        /* +/- -0.7 mS */
#endif
}
BXPT_PCR_PhaseErrorClampRange;


/***************************************************************************
Summary:
    Defaultsettings PCR modules
Description:
    This structure represents the default settings for a pcr module.
***************************************************************************/
typedef struct BXPT_PCR_DefSettings
{
    uint32_t MaxPcrError;   /* Maximum PCR error thredhold*/
    bool PcrTwoErrReaquireEn; /* control of re-acquisition of PCR when TWO_PCR_ERROR occurs */
}
BXPT_PCR_DefSettings;

#if (!B_REFSW_MINIMAL)
/***************************************************************************
Summary:
    Return the number of Pcr channels.
Description:
    This function returns the number of pcr channels supported.
Returns:
    BERR_SUCCESS                - returns the number of Pcr channel.
    BERR_INVALID_PARAMETER      - Bad input parameter
****************************************************************************/
BERR_Code BXPT_PCR_GetTotalChannels(
    BXPT_Handle hXpt,           /* [in] The transport handle */
    unsigned int *TotalChannels     /* [out] The number of Pcr channels. */
    );
#endif

/***************************************************************************
Summary:
    Returns the Pcr channel default settings.

Description:
    This function gets the default settings for a given channel and places
    them in a structure. The default settings shall be retrieved and changed
    before channel openings. The values are set at channel openings.
Returns:
    BERR_SUCCESS                - Retrieved Pcr defaults.
    BERR_INVALID_PARAMETER      - Bad input parameter
****************************************************************************/
BERR_Code BXPT_PCR_GetChannelDefSettings(
    BXPT_Handle hXpt,                       /* [in] The transport handle - need chip info */
    unsigned int WhichPcr,                      /* [in] Which pcr module */
    BXPT_PCR_DefSettings *pcrSettings       /* [out] The default settings of a pcr module */
    );

/***************************************************************************
Summary:
    Open a Pcr channel for a given PCR module
Description:
    This functions obtains a PCR handle. Sets default settings,
    enable the PCR_TWO_ERROR interrupt.
Returns:
    BERR_SUCCESS                - if successful
    BERR_OUT_OF_DEVICE_MEMORY   - Memory failure (should not happen)
    BERR_INVALID_PARAMETER      - Bad input parameter
***************************************************************************/
BERR_Code BXPT_PCR_Open(
    BXPT_Handle hXpt,                  /* [in] The transport handle*/
    unsigned int WhichPcr,             /* [in] Which pcr module */
    BXPT_PCR_DefSettings *pcrSettings, /* [in] The default setting */
    BXPT_PCR_Handle *hPcr              /* [out] The pcr handle */
    );

/***************************************************************************
Summary:
    Close a Pcr channel.
Description:
    This function closes the pcr channel by disabling PCR and sets back to
    default settings.
Returns:
    BERR_SUCCESS                - if successful
    BERR_OUT_OF_DEVICE_MEMORY   - Memory failure (should not happen)
    BERR_INVALID_PARAMETER      - Bad input parameter
***************************************************************************/
BERR_Code BXPT_PCR_Close(
    BXPT_PCR_Handle hPcr     /* [in] The pcr handle */
);

/***************************************************************************
Summary:
    Gets PCR configuration when PCR is locking to a transport stream
Description:
    This functions gets the current setting of the PCR configuration. It includes
    PCR Pid, stream source, SEND_EN, and send mode for Video.
Returns:
    BERR_SUCCESS                - if successful
    BERR_OUT_OF_DEVICE_MEMORY   - Memory failure (should not happen)
    BERR_INVALID_PARAMETER      - Bad input parameter
***************************************************************************/
BERR_Code   BXPT_PCR_GetStreamPcrConfig(
    BXPT_PCR_Handle hPcr,                         /* [in] The pcr handle */
    BXPT_PCR_XptStreamPcrCfg *PcrCfg             /* [out] Transport source configuration */
    );

/***************************************************************************
Summary:
Sets PCR configuration when PCR is locking to a transport stream

Description:
This functions sets Pids, stream source, SEND_EN and timebase reference to
transport stream and puts the PCR module into acqusition mode.

For 7401B0 and later devices, the PCR block can accept data from
playback channels by setting PcrCfg->eStreamSelect = BXPT_DataSource_ePlayback

Returns:
    BERR_SUCCESS                - if successful
    BERR_OUT_OF_DEVICE_MEMORY   - Memory failure (should not happen)
    BERR_INVALID_PARAMETER      - Bad input parameter
***************************************************************************/
BERR_Code   BXPT_PCR_SetStreamPcrConfig(
    BXPT_PCR_Handle hPcr,                        /* [in] The pcr handle */
    const BXPT_PCR_XptStreamPcrCfg *PcrCfg       /* [in] Transport source configuration */
    );

/***************************************************************************
Summary:
Sets PCR configuration when PCR is locking to a transport stream.  This
function is  only be to used from within an ISR.

Description:
This functions sets Pids, stream source, SEND_EN and timebase reference to
transport stream and puts the PCR module into acqusition mode.

For 7401B0 and later devices, the PCR block can accept data from
playback channels by setting PcrCfg->eStreamSelect = BXPT_DataSource_ePlayback

Returns:
    BERR_SUCCESS                - if successful
    BERR_OUT_OF_DEVICE_MEMORY   - Memory failure (should not happen)
    BERR_INVALID_PARAMETER      - Bad input parameter
***************************************************************************/
BERR_Code   BXPT_PCR_SetStreamPcrConfig_isr(
    BXPT_PCR_Handle hPcr,                        /* [in] The pcr handle */
    const BXPT_PCR_XptStreamPcrCfg *PcrCfg       /* [in] Transport source configuration */
    );


/***************************************************************************
Summary:
XPT interface for retrieving phase error at isr time

Returns:
    BERR_SUCCESS                - if successful
***************************************************************************/
BERR_Code   BXPT_PCR_GetPhaseError_isr(
    BXPT_PCR_Handle hPcr,
    int32_t *p_error
    );

#if (!B_REFSW_MINIMAL)
/***************************************************************************
Summary:
    controls modifying the rate of the STC extension counter.
Description:
    This function configures the pre-scale and increment parameters of the
    PCR STC EXT control register. The PCR_INC_VAL (M) and PCR_PRESCALE (N)
    can be used to alter the rate of the STC counter to 27 MHz * M/(N+1).
    For example, to run the STC counter at 18 MHz, (i.e. 2/3 of the normal rate),
    program M=2 and N=2.
Returns:
    BERR_SUCCESS                - if successful
    BERR_INVALID_PARAMETER      - Bad input parameter
***************************************************************************/
BERR_Code   BXPT_PCR_SetStcExtRateControl(
    BXPT_PCR_Handle hPcr,                        /* [in] The pcr handle */
    const BXPT_PCR_STCExtRateConfig  *StcExtRateCfg /* [in] Stc extension rate control configuration */
    );

/***************************************************************************
Summary:
    enables PCR processing on PCR_PID and puts PCR into an acquisition mode.
Description:
    This function enables PCR processing on PCR_PID. Writing a "1" to Valid bit
    also puts the PCR module into an acquisition mode.
Returns:
    BERR_SUCCESS                - if successful
    BERR_INVALID_PARAMETER      - Bad input parameter
***************************************************************************/
void    BXPT_PCR_RefreshPcrPid(
    BXPT_PCR_Handle hPcr               /*[in] The pcr handle  */
    );

/***************************************************************************
Summary:
    Gets the last PCR captured in last PCR Hi/Lo registers.
Description:
    This function reads the values from PCR_LAST_PCR_HI/LO registers.
Returns:
    BERR_SUCCESS                - if successful
    BERR_INVALID_PARAMETER      - Bad input parameter
***************************************************************************/
BERR_Code   BXPT_PCR_GetLastPcr(
    BXPT_PCR_Handle hPcr,             /* [in] The pcr handle */
    uint32_t *p_pcrHi,            /* [out] Upper bits of PCR*/
    uint32_t *p_pcrLo             /* [out] Bit9-LSB of base bit[0-8]-extension*/
    );
#endif

/***************************************************************************
Summary:
    ISR version of BXPT_PCR_GetLastPcr.
Description:
    This function reads the values from PCR_LAST_PCR_HI/LO registers. It is
    callable from within an ISR context.
Returns:
    BERR_SUCCESS                - if successful
    BERR_INVALID_PARAMETER      - Bad input parameter
***************************************************************************/
BERR_Code   BXPT_PCR_GetLastPcr_isr(
    BXPT_PCR_Handle hPcr,
    uint32_t *p_pcrHi,
    uint32_t *p_pcrLo
    );

#if (!B_REFSW_MINIMAL)
/***************************************************************************
Summary:
    Gets the STC counter values
Description:
    This function read the PCR STC counters from STC_HI/LO registers
Returns:
    BERR_SUCCESS                - if successful
    BERR_INVALID_PARAMETER      - Bad input parameter
***************************************************************************/
BERR_Code   BXPT_PCR_GetStc(
    BXPT_PCR_Handle hPcr,            /* [in] The pcr handle */
    uint32_t *p_stcHi,           /* [out] Upper bits of STC*/
    uint32_t *p_stcLo            /* [out] bit9-LSB of base bit[0-8]-extension*/
    );
#endif

/***************************************************************************
Summary:
    Sets non-transport source for a timebase reference and configures frequency
    control for the reference.
Description:
    This function sets a timebase locking to a non transport source and sets
    the frequency control for the reference.  It is recommanded to use
    BXPT_PCR_GetfreqRefConfig before calling this function to preserve
    the default values of frequency control if no intention to change the
    frequency.
Returns:
    BERR_SUCCESS                - if successful
    BERR_INVALID_PARAMETER      - Bad input parameter
***************************************************************************/
BERR_Code   BXPT_PCR_ConfigNonStreamTimeBase(
    BXPT_PCR_Handle hPcr,                         /* [in] The pcr handle */
    BXPT_PCR_TimeRef  eNonStreamTimeBaseCfg,      /* [in] Non transport source configuration */
    BXPT_PCR_TimebaseFreqRefConfig *Timebasefreq  /* [in] Non transport source frequency config */
    );

/***************************************************************************
Summary:
    Gets timebase reference frequency control. It is for non transport time
    references.
Description:
    This function gets the pre-scale and increment parameters of the
    DPCR_REF_PCR control register.
Returns:
    BERR_SUCCESS                - if successful
    BERR_INVALID_PARAMETER      - Bad input parameter
***************************************************************************/
BERR_Code   BXPT_PCR_GetfreqRefConfig(
    BXPT_PCR_Handle hPcr,                               /* [in] The pcr handle */
    BXPT_PCR_TimebaseFreqRefConfig  *TimebaseFreqConfig /* [out] Non transport source configuration */
    );

#if (!B_REFSW_MINIMAL)
/***************************************************************************
Summary:
    Isr version of enabling PCR processing on PCR_PID and putting PCR into
    an acquisition mode.
Description:
    This function enables PCR processing on PCR_PID. Writing a "1" to Valid bit
    also puts the PCR module into an acquisition mode.
Returns:
    BERR_SUCCESS                - if successful
    BERR_INVALID_PARAMETER      - Bad input parameter
***************************************************************************/
void BXPT_PCR_RefreshPcrPid_isr(
    BXPT_PCR_Handle hPcr               /*[in] The pcr handle  */
    );
#endif

/***************************************************************************
Summary:
Freeze integration loop.

Description:
Stop loop tracking by freezing the integrator. Useful for cases like analog
decode. The integrator can be un-frozen by setting the Freeze argument to
false.

Returns:
    void
***************************************************************************/
void BXPT_PCR_FreezeIntegrator(
    BXPT_PCR_Handle hPcr,    /* [in] The pcr handle  */
    bool Freeze              /* [in] Freeze integrator if true, run if false. */
    );

/***************************************************************************
Summary:
Set the center frequency.

Description:
Set the absolute center frequency of the timebase relative to the crytal. A
value of 0x400000 will generate a timebase of 27MHz. Changing the center
frequency by one bit will change the timebase by about 3.8 ppm.

Returns:
    void
***************************************************************************/
void BXPT_PCR_SetCenterFrequency(
    BXPT_PCR_Handle hPcr,       /* [in] The pcr handle  */
    uint32_t CenterFreq         /* [in] Center frequency */
    );

/***************************************************************************
Summary:
Get default values for locking to the given timebase reference.

Description:
Return default values for the loop gain constants, pcr increment, and
prescale. These defaults are tailored to the given timebase reference. These
defaults may be used in a subsequent call to BXPT_PCR_ConfigNonStreamTimeBase.

Returns:
    BERR_SUCCESS                - if successful
    BERR_INVALID_PARAMETER      - Bad input parameter
***************************************************************************/
BERR_Code   BXPT_PCR_GetTimeBaseFreqRefDefaults(
    BXPT_PCR_Handle hPcr,
    BXPT_PCR_TimeRef TimeBase,
    BXPT_PCR_TimebaseFreqRefConfig *Def
    );

/***************************************************************************
Summary:
Set the time base tracking range in the PCR block.

Description:
The timebase has a limited tracking range. The tracking range is centered on
the crystal reference  + pcr center. Note that changes to the tracking range
may cause sudden changes to the timebase, and may affect the video output quality.

Returns:
    BERR_SUCCESS                - if successful
    BERR_INVALID_PARAMETER      - Bad input parameter
***************************************************************************/
void BXPT_PCR_SetTimeRefTrackRange(
    BXPT_PCR_Handle hPcr,
    BXPT_PCR_RefTrackRange TrackRange
    );

#if (!B_REFSW_MINIMAL)
/***************************************************************************
Summary:
Set the phase error clamp range for the phase error input to the loop filter

Description:

Returns:
    BERR_SUCCESS                - if successful
    BERR_INVALID_PARAMETER      - Bad input parameter
***************************************************************************/
void BXPT_PCR_SetPhaseErrorClampRange(
    BXPT_PCR_Handle hPcr,
    BXPT_PCR_PhaseErrorClampRange ClampRange
    );
#endif

/***************************************************************************
Summary:
Enumeration of all supported interrupts in a Rave context.
****************************************************************************/
typedef enum BXPT_PCR_IntName
{
    /* Set whenever a PCR is detected */
#ifdef BCHP_XPT_DPCR0_REG_START
    BXPT_PCR_IntName_ePhaseCompare = BCHP_XPT_DPCR0_INTR_STATUS_REG_PHASE_CMP_INT_SHIFT,

    /* Set whenever 2 consecutive PCR errors are seen in the stream. */
    BXPT_PCR_IntName_eTwoPcrErrors = BCHP_XPT_DPCR0_INTR_STATUS_REG_TWO_PCR_ERROR_SHIFT,

    BXPT_PCR_IntName_eOnePcrError = BCHP_XPT_DPCR0_INTR_STATUS_REG_ONE_PCR_ERROR_SHIFT,
    BXPT_PCR_IntName_ePhaseSaturation = BCHP_XPT_DPCR0_INTR_STATUS_REG_PHASE_SAT_INT_SHIFT,
#else
    BXPT_PCR_IntName_ePhaseCompare = 0,
    BXPT_PCR_IntName_eTwoPcrErrors,
    BXPT_PCR_IntName_eOnePcrError,
    BXPT_PCR_IntName_ePhaseSaturation,
#endif
    BXPT_PCR_IntName_eMax   /* Marks the end of the supported interrutps */
}
BXPT_PCR_IntName;

/***************************************************************************
Summary:
Generate the interrupt ID for the requested interrupt name.

Description:
Prevents the app from having to do it directly.

Returns:
    BERR_SUCCESS                - if successful
    BERR_INVALID_PARAMETER      - Bad input parameter
***************************************************************************/
BERR_Code BXPT_PCR_GetIntId(
    unsigned WhichPcr,          /* Which PCR block. Same value as used in BXPT_PCR_Open() */
    BXPT_PCR_IntName Name,      /* The interrupt in question. */
    BINT_Id *IntId              /* IntId suitable for the BINT module. */
   );

#if BXPT_HAS_DPCR_INTEGRATOR_WORKAROUND
BERR_Code BXPT_PCR_P_Integrator(
    BXPT_Handle hXpt
    );
#endif

#ifdef __cplusplus
}
#endif

#endif
