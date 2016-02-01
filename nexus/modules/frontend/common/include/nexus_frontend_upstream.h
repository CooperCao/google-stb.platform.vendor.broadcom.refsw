/***************************************************************************
*     (c)2004-2009 Broadcom Corporation
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
*   API name: Frontend UpStream
*    Generic APIs for upstream tuner device control
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#ifndef NEXUS_FRONTEND_UPSTREAM_H__
#define NEXUS_FRONTEND_UPSTREAM_H__

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
Upstream Tuner Mode
***************************************************************************/
typedef enum NEXUS_FrontendUpstreamMode
{
    NEXUS_FrontendUpstreamMode_eAnnexA,
    NEXUS_FrontendUpstreamMode_eDvs167 = NEXUS_FrontendUpstreamMode_eAnnexA,
    NEXUS_FrontendUpstreamMode_eDavic = NEXUS_FrontendUpstreamMode_eAnnexA,
    NEXUS_FrontendUpstreamMode_eDvs178,
    NEXUS_FrontendUpstreamMode_eStarvue = NEXUS_FrontendUpstreamMode_eDvs178,
    NEXUS_FrontendUpstreamMode_eDocsis,
    NEXUS_FrontendUpstreamMode_ePodAnnexA,
    NEXUS_FrontendUpstreamMode_ePodDvs167 = NEXUS_FrontendUpstreamMode_ePodAnnexA,
    NEXUS_FrontendUpstreamMode_ePodDavic = NEXUS_FrontendUpstreamMode_ePodAnnexA,
    NEXUS_FrontendUpstreamMode_ePodDvs178,
    NEXUS_FrontendUpstreamMode_ePodStarvue = NEXUS_FrontendUpstreamMode_ePodDvs178,
    NEXUS_FrontendUpstreamMode_eSingleCw,
    NEXUS_FrontendUpstreamMode_eDoubleCw,
    NEXUS_FrontendUpstreamMode_ePn23, 
    NEXUS_FrontendUpstreamMode_eMax
} NEXUS_FrontendUpstreamMode;

/***************************************************************************
Summary:
Upstream Tuning Parameters
***************************************************************************/
typedef struct NEXUS_FrontendUpstreamTestModeSettings
{
    bool bypassFilters;                 /* 0 = Bypass filter. 1 = Don't bypass filter. Applies only to single tone and double tone test modes. */
    bool nyquistAlphaValue;             /* 0 = 25% Nyquist Alpha. 1 = 50% Nyquist Alpha. Applies to all test modes. */
    bool spectrumInverted;              /* 0 = Spectrum not inverted. 1 = Spectrum inverted. Applies only to pn23 test mode. */
    unsigned int qamMode;               /* 0 = QAM2; 1 = QAM4; 2 = QAM16; 3 = QAM64; 4= QAM256. Applies only to pn23 test mode.*/
    unsigned long rfFreq2;              /* Sets the second Rf frequency for two tone testing. */
} NEXUS_FrontendUpstreamTestModeSettings;

/***************************************************************************
Summary:
Upstream Tuning Parameters
***************************************************************************/
typedef struct NEXUS_FrontendUpstreamSettings
{
    NEXUS_FrontendUpstreamMode mode;
    unsigned frequency;                 /* In Hz */
    unsigned symbolRate;                /* In Baud */
    unsigned powerLevel;                /* In dBmV */
    NEXUS_FrontendUpstreamTestModeSettings testModeSettings;
} NEXUS_FrontendUpstreamSettings;

/***************************************************************************
Summary:
    Initialize an upstream settings structure to defaults
***************************************************************************/
void NEXUS_Frontend_GetDefaultUpstreamSettings(
    NEXUS_FrontendUpstreamSettings *pSettings
    );

/***************************************************************************
Summary:
    Tune an upstream tuner
***************************************************************************/
NEXUS_Error NEXUS_Frontend_TuneUpstream(
    NEXUS_FrontendHandle handle,
    const NEXUS_FrontendUpstreamSettings *pSettings
    );

/***************************************************************************
Summary:
Upstream Tuning Status
***************************************************************************/
typedef struct NEXUS_FrontendUpstreamStatus
{
    NEXUS_FrontendUpstreamMode mode;
    unsigned frequency;                 /* In Hz */
    unsigned symbolRate;                /* In Baud */
    unsigned powerLevel;                /* In dBmV */
    unsigned sysXtalFreq;               /* in Hz, Sys. Xtal freq */    
} NEXUS_FrontendUpstreamStatus;

/***************************************************************************
Summary:
    Get the current status of an upstream tuner
***************************************************************************/
NEXUS_Error NEXUS_Frontend_GetUpstreamStatus(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendUpstreamStatus *pStatus
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_FRONTEND_UPSTREAM_H__ */

