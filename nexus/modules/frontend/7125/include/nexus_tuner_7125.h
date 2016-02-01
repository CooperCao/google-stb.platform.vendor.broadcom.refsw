/***************************************************************************
*     (c)2004-2010 Broadcom Corporation
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
*   API name: BCM7125 Tuner
*    APIs to open, close, and setup initial settings for a BCM7125 Tuner Device.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#ifndef NEXUS_TUNER_7125_H__
#define NEXUS_TUNER_7125_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "nexus_frontend.h"
#include "nexus_i2c.h"

/***************************************************************************
Summary:
    Settings to open a BCM7125 Tuner Device
 ***************************************************************************/
typedef struct NEXUS_7125TunerSettings
{
    NEXUS_I2cHandle qamLnaI2c;      /* I2C Bus connected to the QAM LNA (NULL if not used) */
    bool rfBypassEnabled;           /* If true, the RF bypass is enabled */
	unsigned unitNumber;       		/* Which tuner to open from this device. This is set only at open-time. */
	/* AGC thresholds set by VGA CAL.  Set them (Min_AGC_Threshold) to 0 if you want 
	   btnr_7125 to attempt its own VGA Calibration. Set them to good, calibrated
	   values if you have them.  Leave them as set by BTNR_7125_GetDefaultSettings
	   otherwise. */
	int32_t 			Min_AGC_Threshold;	/* Lower AGC Threshold */
	int32_t 			Max_AGC_Threshold;  /* Upper range of AGC */
	bool skipInit;			/* skip initialization that might disrupt ongoing reception */
} NEXUS_7125TunerSettings;

/***************************************************************************
Summary:
    Get the default settings for a BCM7125 Tuner tuner
 ***************************************************************************/
void NEXUS_Tuner_GetDefault7125Settings(
    NEXUS_7125TunerSettings *pSettings   /* [out] */
    );

/***************************************************************************
Summary:
    Open a handle to a BCM7125 device.
 ***************************************************************************/
NEXUS_TunerHandle NEXUS_Tuner_Open7125( /* attr{destructor=NEXUS_Tuner_Close} */
    const NEXUS_7125TunerSettings *pSettings
    );

/***************************************************************************
Summary:
    Settings for a BCM7125 Tuner Device
 ***************************************************************************/
typedef struct NEXUS_7125TuneSettings
{
    unsigned bandwidth;     /* Specified in MHz.  Supported values are 5..8 */
    unsigned frequency;     /* Specified in Hz. */
} NEXUS_7125TuneSettings;

/***************************************************************************
Summary:
    Get default tune settings for a BCM7125 tuner
 ***************************************************************************/
void NEXUS_Tuner_GetDefault7125TuneSettings(
    NEXUS_7125TuneSettings *pSettings  /* [out] */
    );

/***************************************************************************
Summary:
    Tune a BCM7125 tuner
 ***************************************************************************/
NEXUS_Error NEXUS_Tuner_Tune7125(
    NEXUS_TunerHandle handle,
    const NEXUS_7125TuneSettings *pSettings
    );

/***************************************************************************
Summary:
    Get current settings for the tuner
 ***************************************************************************/
void NEXUS_Tuner_Get7125Settings(
    NEXUS_TunerHandle handle,
    NEXUS_7125TunerSettings *pSettings  /* [out] */
    );

/***************************************************************************
Summary:
    Set current settings for the tuner
 ***************************************************************************/
NEXUS_Error NEXUS_Tuner_Set7125Settings(
    NEXUS_TunerHandle handle,
    const NEXUS_7125TunerSettings *pSettings
    );

/***************************************************************************
Summary:
    7125 Tuner Status
 ***************************************************************************/
typedef struct NEXUS_7125TunerStatus
{
    int rfAgcLevel;     /* RF AGC value in 1/10 percent */
    int ifAgcLevel;     /* IF AGC value in 1/10 percent */
    int lnaAgcLevel;    /* LNA AGC value in 1/10 percent */
    int dpmLvel;        /* dpm dvbm level 1/256 */
} NEXUS_7125TunerStatus;

/***************************************************************************
Summary:
    Get 7125 Tuner Status
 ***************************************************************************/
void NEXUS_Tuner_Get7125Status(
    NEXUS_TunerHandle handle,
    NEXUS_7125TunerStatus *pStatus  /* [out] */
    );

/***************************************************************************
Summary:
    Untune (power down) a BCM7125 tuner
 ***************************************************************************/
NEXUS_Error NEXUS_Tuner_Untune7125(
    NEXUS_TunerHandle handle
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_TUNER_7125_H__ */

