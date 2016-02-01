/***************************************************************************
*     (c)2011 Broadcom Corporation
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
*   API name: Frontend channel scan
*    Not all platforms support channel (frequency) scanning, and detectable
*    signal types vary platform by platform.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

/* base was /nexus/modules/frontend/35230/include/nexus_frontend_scan.h@@/main/1 */

#ifndef NEXUS_FRONTEND_SCAN_H__
#define NEXUS_FRONTEND_SCAN_H__

#include "nexus_frontend.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
    Signal types that may be scanned
***************************************************************************/
typedef enum NEXUS_FrontendSignalType
{
    NEXUS_FrontendSignalType_eVsb,
    NEXUS_FrontendSignalType_eQam, /* J.83B */
    NEXUS_FrontendSignalType_eAnalog,
    NEXUS_FrontendSignalType_eNtsc = NEXUS_FrontendSignalType_eAnalog,
    NEXUS_FrontendSignalType_eDvbt,
    NEXUS_FrontendSignalType_eIsdbt,
    NEXUS_FrontendSignalType_eDvbc, /* J.83A */
    NEXUS_FrontendSignalType_ePal,
    NEXUS_FrontendSignalType_eSecam,
    NEXUS_FrontendSignalType_eMax
} NEXUS_FrontendSignalType;

/***************************************************************************
Summary:
    Options for NEXUS_Frontend_ScanFrequency
***************************************************************************/
typedef struct NEXUS_FrontendScanSettings
{
    bool signalTypes[NEXUS_FrontendSignalType_eMax]; /* signal types to scan */

    unsigned frequency;             /* In Hz */
    unsigned ifFrequency;           /* In Hz, ignored if 0 (zero) */
    unsigned bandwidths[NEXUS_FrontendBandwidth_eMax]; /* supported bandwidths, in Hz.
        Ignored for fixed bandwidth signals, for example 6MHz for VSB.
        Applied bandwidths are from index 0 upward until repeating the same bandwidth or NEXUS_FrontendBandwidth_eMax.
        Supported bandwidths are platform and attached-tuner dependent. */
} NEXUS_FrontendScanSettings;

/***************************************************************************
Summary:
    Get default options for NEXUS_Frontend_ScanFrequency
***************************************************************************/
void NEXUS_Frontend_GetDefaultScanSettings(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendScanSettings *pSettings   /* [out] */
    );

/***************************************************************************
Summary:
    Frequency Scan Results
***************************************************************************/
typedef struct NEXUS_FrontendScanResults
{
    NEXUS_FrontendSignalType signalType;    /* Detected signal type */

    /* Based on the value in signalType, precise modulation scheme is
       listed below */
    union
    {
        NEXUS_FrontendQamMode qam;
        NEXUS_FrontendVsbMode vsb;
    } mode;

    unsigned bandwidth;     /* Detected bandwidth in Hz */
    bool spectrumInverted;  /* is the spectrum inverted? */
    int frequencyOffset;    /* in Hz */
    unsigned symbolRate;    /* in baud, not all detected signals report */
} NEXUS_FrontendScanResults;

/***************************************************************************
Summary:
Scan a frequency for specified signal types

Description:
This is a blocking function.  It will return NEXUS_SUCCESS if a signal
is detected matching the specified settings.  If no signal is
detected, NEXUS_TIMEOUT will be returned.

See NEXUS_Frontend_SatellitePeakscan for satellite frequency scanning.
***************************************************************************/
NEXUS_Error NEXUS_Frontend_ScanFrequency(
    NEXUS_FrontendHandle handle,
    const NEXUS_FrontendScanSettings *pSettings,
    NEXUS_FrontendScanResults *pResults             /* [out] */
    );

/***************************************************************************
Summary:
    Thresholds for NEXUS_Frontend_ScanFrequency operation
***************************************************************************/
typedef struct NEXUS_FrontendScanThresholds
{
    unsigned qamThreshold;          /* Value in ???      */
    unsigned agfThreshold;          /* Value in 1/256 dB */
    unsigned agcThreshold;          /* Value in 1/256 dB */
} NEXUS_FrontendScanThresholds;

/***************************************************************************
Summary:
    Get thresholds for NEXUS_Frontend_ScanFrequency operation
***************************************************************************/
void NEXUS_Frontend_GetScanThresholds(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendScanThresholds *pThresholds   /* [out] */
    );

/***************************************************************************
Summary:
    Set thresholds for NEXUS_Frontend_ScanFrequency operation
***************************************************************************/
NEXUS_Error NEXUS_Frontend_SetScanThresholds(
    NEXUS_FrontendHandle handle,
    const NEXUS_FrontendScanThresholds *pThresholds
    );

#ifdef __cplusplus
}
#endif

#endif /* NEXUS_FRONTEND_SCAN_H__ */

