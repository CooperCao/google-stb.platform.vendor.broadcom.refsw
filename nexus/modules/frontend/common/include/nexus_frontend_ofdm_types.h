/***************************************************************************
*     (c)2004-2013 Broadcom Corporation
*
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* API Description:
*   API name: Frontend OFDM
*    Generic APIs for OFDM (Orthogonal Frequency-Division Multiplexing) tuning.
*    This is used in DVB-H and DVB-T environments.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#ifndef NEXUS_FRONTEND_OFDM_TYPES_H__
#define NEXUS_FRONTEND_OFDM_TYPES_H__

#include "nexus_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
The NEXUS_FrontendOfdmBandwidth enum has been deprecated. It is an unsigned integer in units of Hz now.
The following #defines are for backward compatibility only.
***************************************************************************/
#define NEXUS_FrontendOfdmBandwidth_e5Mhz 5000000
#define NEXUS_FrontendOfdmBandwidth_e6Mhz 6000000
#define NEXUS_FrontendOfdmBandwidth_e7Mhz 7000000
#define NEXUS_FrontendOfdmBandwidth_e8Mhz 8000000
#define NEXUS_FrontendOfdmBandwidth_eMax  4

/***************************************************************************
Summary:
    OFDM guard interval
****************************************************************************/
typedef enum NEXUS_FrontendOfdmGuardInterval
{
    NEXUS_FrontendOfdmGuardInterval_e1_32,  /* 1/32 Guard Interval */
    NEXUS_FrontendOfdmGuardInterval_e1_16,  /* 1/16 Guard Interval */
    NEXUS_FrontendOfdmGuardInterval_e1_8,   /* 1/8 Guard Interval */
    NEXUS_FrontendOfdmGuardInterval_e1_4,   /* 1/4 Guard Interval */
    NEXUS_FrontendOfdmGuardInterval_e1_128, /* 1/128 Guard Interval DVB-T2 */
    NEXUS_FrontendOfdmGuardInterval_e19_128,/* 19/128 Guard Interval DVB-T2 */
    NEXUS_FrontendOfdmGuardInterval_e19_256,/* 19/256 Guard Interval DVB-T2 */
    NEXUS_FrontendOfdmGuardInterval_eMax
}NEXUS_FrontendOfdmGuardInterval;

/***************************************************************************
Summary:
    OFDM Code Rate
****************************************************************************/
typedef enum NEXUS_FrontendOfdmCodeRate
{
    NEXUS_FrontendOfdmCodeRate_e1_2,      /* Rate 1/2 */
    NEXUS_FrontendOfdmCodeRate_e2_3,      /* Rate 2/3 */
    NEXUS_FrontendOfdmCodeRate_e3_4,      /* Rate 3/4 */
    NEXUS_FrontendOfdmCodeRate_e3_5,      /* Rate 3/5 DVB-T2*/
    NEXUS_FrontendOfdmCodeRate_e4_5,      /* Rate 4/5 DVB-T2*/
    NEXUS_FrontendOfdmCodeRate_e5_6,      /* Rate 5/6 */
    NEXUS_FrontendOfdmCodeRate_e7_8,      /* Rate 7/8 */
    NEXUS_FrontendOfdmCodeRate_eMax
}NEXUS_FrontendOfdmCodeRate;

/***************************************************************************
Summary:
    OFDM Hierarchy
****************************************************************************/
typedef enum NEXUS_FrontendOfdmHierarchy
{
    NEXUS_FrontendOfdmHierarchy_e0,        /* Select Hierarchy 0 */
    NEXUS_FrontendOfdmHierarchy_e1,        /* Select Hierarchy 1 */
    NEXUS_FrontendOfdmHierarchy_e2,        /* Select Hierarchy 2 */
    NEXUS_FrontendOfdmHierarchy_e4,        /* Select Hierarchy 4 */
    NEXUS_FrontendOfdmHierarchy_eMax
}NEXUS_FrontendOfdmHierarchy;

/***************************************************************************
Summary:
    OFDM ISDB-T Time Interleaving Mode
****************************************************************************/
typedef enum NEXUS_FrontendOfdmTimeInterleaving
{
    NEXUS_FrontendOfdmTimeInterleaving_e0x,
    NEXUS_FrontendOfdmTimeInterleaving_e1x,
    NEXUS_FrontendOfdmTimeInterleaving_e2x,
    NEXUS_FrontendOfdmTimeInterleaving_e3x,
    NEXUS_FrontendOfdmTimeInterleaving_eMax
} NEXUS_FrontendOfdmTimeInterleaving;

/***************************************************************************
Summary:
    OFDM modulation mode
****************************************************************************/
typedef enum NEXUS_FrontendOfdmModulation
{
    NEXUS_FrontendOfdmModulation_eQpsk,
    NEXUS_FrontendOfdmModulation_eQam16,
    NEXUS_FrontendOfdmModulation_eQam64,
    NEXUS_FrontendOfdmModulation_eQam256, /* DVB-T2 */
    NEXUS_FrontendOfdmModulation_eDqpsk, /* ISDB-T */
    NEXUS_FrontendOfdmModulation_eMax
}NEXUS_FrontendOfdmModulation;


/***************************************************************************
Summary:
    OFDM transmission mode
****************************************************************************/
typedef enum NEXUS_FrontendOfdmTransmissionMode
{
    NEXUS_FrontendOfdmTransmissionMode_e1k,     /* DVB-T2 */
    NEXUS_FrontendOfdmTransmissionMode_e2k,
    NEXUS_FrontendOfdmTransmissionMode_e4k,
    NEXUS_FrontendOfdmTransmissionMode_e8k,
    NEXUS_FrontendOfdmTransmissionMode_e16k,    /* DVB-T2 */
    NEXUS_FrontendOfdmTransmissionMode_e32k,    /* DVB-T2 */
    NEXUS_FrontendOfdmTransmissionMode_eMax
}NEXUS_FrontendOfdmTransmissionMode;

/***************************************************************************
Summary:
Represnts total sampled bits and error bits.

Only rate is guaranteed to be populated. count and total may be zero.
***************************************************************************/
typedef struct NEXUS_FrontendErrorRate
{
    uint64_t count; /* Count of error bits. */
    uint64_t total; /* Total number of sampled bits. If total wraps, total and count are cleared together. It could wrap for internal reasons, before reaching full 64 bits. */
    uint32_t rate;  /* Count/total, represented in 1/(2^31) th units. Convert to floating point by dividing by (float)2^31.  */
} NEXUS_FrontendErrorRate;

/***************************************************************************
Summary:
Represents FEC(Viterbi + Reed Solomon) block counts.
Total blocks is the sum of the three counts.
***************************************************************************/
typedef struct NEXUS_FrontendBlockCounts
{
    unsigned    corrected;   /* number of blocks with errors that have been corrected */
    unsigned    uncorrected; /* number of blocks with errors that have not been corrected */
    unsigned    clean;       /* number of blocks without errors */
} NEXUS_FrontendBlockCounts;

/***************************************************************************
Summary:
    Enumeration for receiver mode
****************************************************************************/
typedef enum NEXUS_FrontendOfdmMode
{
    NEXUS_FrontendOfdmMode_eDvbt,       /* DVB-T */
    NEXUS_FrontendOfdmMode_eDvbt2,      /* DVB-T2 */
    NEXUS_FrontendOfdmMode_eDvbc2,      /* DVB-C2 */
    NEXUS_FrontendOfdmMode_eIsdbt,      /* ISDB-T */
    NEXUS_FrontendOfdmMode_eMax
} NEXUS_FrontendOfdmMode;

/***************************************************************************
Summary:
    Enumeration for receiver CCI (CoChannel Interference Filter) Mode
****************************************************************************/
typedef enum NEXUS_FrontendOfdmCciMode
{
    NEXUS_FrontendOfdmCciMode_eNone,    /* No CCI selected */
    NEXUS_FrontendOfdmCciMode_eAuto,    /* Auto Selection */
    NEXUS_FrontendOfdmCciMode_eMax
} NEXUS_FrontendOfdmCciMode;

/***************************************************************************
Summary:
    Enumeration for stream priority

Description:
    DVB-T can hierarchically encode streams.  A low-priority stream can
    be embedded within a high priority stream.  Receivers with good reception
    conditions may be able to receive both streams, but receivers with poor
    reception conditions may only be able to receive the high priority stream.

****************************************************************************/
typedef enum NEXUS_FrontendOfdmPriority
{
    NEXUS_FrontendOfdmPriority_eHigh,
    NEXUS_FrontendOfdmPriority_eLow,
    NEXUS_FrontendOfdmPriority_eMax
} NEXUS_FrontendOfdmPriority;

/***************************************************************************
Summary:
    OFDM modulation mode
****************************************************************************/
typedef enum NEXUS_FrontendOfdmAcquisitionMode
{
    NEXUS_FrontendOfdmAcquisitionMode_eAuto,        /* Default.  Automatic Re-acquisition. */
    NEXUS_FrontendOfdmAcquisitionMode_eManual,      /* Manual Re-acquisition, app must re-acquire if signal is lost. */
    NEXUS_FrontendOfdmAcquisitionMode_eScan,        /* Auto-scan acquisition.  Will re-acquire if signal strength is considered sufficient. */
    NEXUS_FrontendOfdmAcquisitionMode_eMax
}NEXUS_FrontendOfdmAcquisitionMode;

/***************************************************************************
Summary:
    OFDM Pull-In Range
****************************************************************************/
typedef enum NEXUS_FrontendOfdmPullInRange
{
    NEXUS_FrontendOfdmPullInRange_eNarrow,  /* Carrier acquisition range is narrow +/-200kHz */
    NEXUS_FrontendOfdmPullInRange_eWide,    /* carrier acquisition range is wide +/-600kHz */
    NEXUS_FrontendOfdmPullInRange_eMax
}NEXUS_FrontendOfdmPullInRange;

/***************************************************************************
Summary:
    DEPRECATED.
    Refer to NEXUS_FrontendOfdmSpectrumMode and NEXUS_FrontendOfdmSpectralInversion.
 ***************************************************************************/
typedef enum NEXUS_FrontendOfdmSpectrum
{
    NEXUS_FrontendOfdmSpectrum_eAuto,
    NEXUS_FrontendOfdmSpectrum_eNonInverted,
    NEXUS_FrontendOfdmSpectrum_eInverted,
    NEXUS_FrontendOfdmSpectrum_eMax
} NEXUS_FrontendOfdmSpectrum;

/***************************************************************************
Summary:
Determines if Ofdm spectrum mode.
***************************************************************************/
typedef enum NEXUS_FrontendOfdmSpectrumMode
{
    NEXUS_FrontendOfdmSpectrumMode_eAuto,
    NEXUS_FrontendOfdmSpectrumMode_eManual,
    NEXUS_FrontendOfdmSpectrumMode_eMax
} NEXUS_FrontendOfdmSpectrumMode;

/***************************************************************************
Summary:
Determines if Ofdm spectral inversion is enabled or not
***************************************************************************/
typedef enum NEXUS_FrontendOfdmSpectralInversion
{
    NEXUS_FrontendOfdmSpectralInversion_eNormal,
    NEXUS_FrontendOfdmSpectralInversion_eInverted,
    NEXUS_FrontendOfdmSpectralInversion_eMax
} NEXUS_FrontendOfdmSpectralInversion;

/***************************************************************************
Summary:
    OFDM spectral parameters
 ***************************************************************************/
typedef enum NEXUS_FrontendOfdmModeGuard
{
    NEXUS_FrontendOfdmModeGuard_eManual,
    NEXUS_FrontendOfdmModeGuard_eAutoDvbt,
    NEXUS_FrontendOfdmModeGuard_eAutoIsdbtJapan,
    NEXUS_FrontendOfdmModeGuard_eAutoIsdbtBrazil,
    NEXUS_FrontendOfdmModeGuard_eMax
} NEXUS_FrontendOfdmModeGuard;

/***************************************************************************
Summary:
    OFDM BERT header leangth used for bit error rate testing.
 ***************************************************************************/
typedef enum  NEXUS_FrontendOfdmBertHeader
{
     NEXUS_FrontendOfdmBertHeader_e4Byte, /* strip out 4-byte header */
     NEXUS_FrontendOfdmBertHeader_e1Byte, /* strip out 1-byte header (sync byte only) */
     NEXUS_FrontendOfdmBertHeader_eMax
}  NEXUS_FrontendOfdmBertHeader;

/***************************************************************************
Summary:
    OFDM BERT polynomial used for bit error rate testing.
 ***************************************************************************/
typedef enum  NEXUS_FrontendOfdmBertPolynomial
{
     NEXUS_FrontendOfdmBertPolynomial_e23, /* 23-bit polynomial */
     NEXUS_FrontendOfdmBertPolynomial_e15, /* 15-bit polynomial */
     NEXUS_FrontendOfdmBertPolynomial_eMax
}  NEXUS_FrontendOfdmBertPolynomial;

/***************************************************************************
Summary:
    OFDM spectral parameters
 ***************************************************************************/
typedef enum NEXUS_FrontendDvbt2Profile
{
    NEXUS_FrontendDvbt2Profile_eBase, /* DVB-T2 Base Profile */
    NEXUS_FrontendDvbt2Profile_eLite, /* DVB-T2 Lite Profile */
    NEXUS_FrontendDvbt2Profile_eMax
} NEXUS_FrontendDvbt2Profile;

/***************************************************************************
Summary:
    OFDM tuning parameters
 ***************************************************************************/
typedef struct NEXUS_FrontendOfdmSettings
{
    NEXUS_FrontendOfdmMode mode;
    unsigned bandwidth; /* in Hz */
    unsigned frequency;                     /* In Hz */
    bool terrestrial;                                  /* Receiver mode: true=terrestrial, false=cable */
    NEXUS_FrontendOfdmAcquisitionMode acquisitionMode; /* Acquisition Mode.  Default is auto, not supported on all platforms. */
    NEXUS_FrontendOfdmSpectrum spectrum;                   /* DEPRECATED.  */
    NEXUS_FrontendOfdmSpectrumMode spectrumMode;       /* Defines manual or auto spectrum mode. */
    NEXUS_FrontendOfdmSpectralInversion spectralInversion; /* Defines normal or inverted spectrum selection for acquisition. */
    NEXUS_CallbackDesc lockCallback;                   /* Callback will be called when lock status changes */
    NEXUS_CallbackDesc asyncStatusReadyCallback;       /* Callback will be called when the async ofdm status is ready. */
    bool spurDetectAndCancel;                          /* If true, enables the detection and cancellation of the strongest spur if it crosses a interally fixed threshold.
	                                                      By default, this is disabled as it increases the acquisition time. */

    struct
    {
       bool plpMode;    /* This bit controls whether the output  PLP is  manually or automatically selected.  When plpMode is false(manual mode), the user must supply the desired plpId.  */
       uint8_t plpId;   /* In single-PLP applications, this unsigned number specifies the desired PLP for output by the T2 receiver.
                                              In multiple-PLP applications, this unsigned number specifies the ID for the data  (Type1 or Type2) PLP
                                              and the common (Type0) PLP is automatically determined. */
       NEXUS_FrontendDvbt2Profile   profile;    /* DVB-T2 profile to tune/acquire. */
    } dvbt2Settings;

    struct
    {
       bool plpMode;    /* This bit controls whether the output  PLP is  manually or automatically selected.  When plpMode is false(manual mode), the user must supply the desired plpId.  */
       uint8_t plpId;   /* In single-PLP applications, this unsigned number specifies the desired PLP for output by the C2 receiver.
                            In multiple-PLP applications, this unsigned number specifies the ID for the data    (Type1 or Type2) PLP
                            and the common (Type0) PLP is automatically determined. */
    } dvbc2Settings;

    struct
    {
      bool                              enabled;     /* Enable or disable BERT to save power and to avoid returning meaningless numbers given none prbs data. */

      NEXUS_FrontendOfdmBertHeader      header;     /* Selects the number of header bytes to be stripped at the BERR module input. */
      NEXUS_FrontendOfdmBertPolynomial  polynomial; /* PRBS(Pseudo Random Binary Sequence) used by the BERT module. */
    } bert;

    /* The parameters below are ISDB-T and DVB-T settings. Left as is for legacy reasons. */

    unsigned ifFrequency;                   /* In Hz */
    NEXUS_CallbackDesc ewsCallback;                    /* Callback will be called when the ISDB-T EWS (Emergency Warning System) status changes */
    NEXUS_FrontendOfdmCciMode cciMode;
    NEXUS_FrontendOfdmPriority priority;
    NEXUS_FrontendOfdmPullInRange pullInRange;

    bool manualTpsSettings;     /* Set to true if you want to manually specify TPS (Transmission Parameters Signalling) settings for DVB-T */
    struct
    {
        NEXUS_FrontendOfdmCodeRate highPriorityCodeRate;    /* High priority coderate to be used in acquisition.  Ignored if manualTpsSettings = false */
        NEXUS_FrontendOfdmCodeRate lowPriorityCodeRate;     /* Low priority coderate to be used in acquisition.  Ignored if manualTpsSettings = false */
        NEXUS_FrontendOfdmHierarchy hierarchy;              /* Hierarchy to be used in acquisition.  Ignored if manualTpsSettings = false */
        NEXUS_FrontendOfdmModulation modulation;            /* Modulation to be used in acquisition.  Ignored if manualTpsSettings = false */
    } tpsSettings;

    bool manualModeSettings;    /* Set to true, if you want to choose the auto mode particular to a region and standard, or to explicitly set transmission mode and guard interval. */
    struct
    {
        NEXUS_FrontendOfdmModeGuard modeGuard;         /* Automatically selects a particular subset of transmission modes and guard intervals according to the standard and region. */
                                                       /* If you choose to explicitly set the transmission mode and guard interval, then set this to NEXUS_FrontendOfdmModeGuard_eManual. */
        NEXUS_FrontendOfdmTransmissionMode mode;            /* Transmission mode to be used in acquisition.  Ignored if manualModeSettings = false */
        NEXUS_FrontendOfdmGuardInterval guardInterval;      /* Guard interval to be used in acquisition.  Ignored if manualModeSettings = false */
    } modeSettings;

    bool manualTmccSettings;     /* Set to true if you want to manually specify TMCC (Transmission and Multiplexing Configuration Control) settings */
    struct
    {
        bool                               partialReception;        /* Manually set ISDB-T partial reception. */
        NEXUS_FrontendOfdmModulation       modulationA;             /* Manually set ISDB-T Layer A modulation type. */
        NEXUS_FrontendOfdmCodeRate         codeRateA;               /* Manually set ISDB-T Layer A code rate. */
        NEXUS_FrontendOfdmTimeInterleaving timeInterleavingA;       /* Manually set ISDB-T Layer A time interleaving.*/
        unsigned                           numSegmentsA;            /* Manually set ISDB-T Layer A number of segments. */
        NEXUS_FrontendOfdmModulation       modulationB;             /* Manually set ISDB-T Layer B modulation type. */
        NEXUS_FrontendOfdmCodeRate         codeRateB;               /* Manually set ISDB-T Layer B code rate. */
        NEXUS_FrontendOfdmTimeInterleaving timeInterleavingB;       /* Manually set ISDB-T Layer B time interleaving.*/
        unsigned                           numSegmentsB;            /* Manually set ISDB-T Layer B number of segments. */
        NEXUS_FrontendOfdmModulation       modulationC;             /* Manually set ISDB-T Layer C modulation type. */
        NEXUS_FrontendOfdmCodeRate         codeRateC;               /* Manually set ISDB-T Layer C code rate. */
        NEXUS_FrontendOfdmTimeInterleaving timeInterleavingC;       /* Manually set ISDB-T Layer C time interleaving.*/
        unsigned                           numSegmentsC;            /* Manually set ISDB-T Layer C number of segments. */
    } tmccSettings;
} NEXUS_FrontendOfdmSettings;

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_FRONTEND_OFDM_TYPES_H__ */

