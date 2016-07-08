/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
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
 ******************************************************************************/

#ifndef BHAB_3158_H
#define BHAB_3158_H

#ifdef __cplusplus
extern "C" {
#endif


#include "bhab.h"

typedef struct BHAB_3158_Settings
{
   bool bUseInternalMemory; /* true if LEAP is loaded in 3158 internal memory */
   uint32_t physAddr;       /* physical address of 1MB LEAP memory (applies only when bUseInternalMemory=false) */
   void *pRam;              /* pointer to 1MB LEAP memory (applies only when bUseInternalMemory=false) */
   void *heap;              /* heap handle where pRam is allocated */
} BHAB_3158_Settings;

typedef enum BHAB_3158_StandbyMode
{
    BHAB_3158_StandbyMode_eOn,        /* Normal mode of operation.
                         Also known as S0 mode */
    BHAB_3158_StandbyMode_eActive,    /* S1 mode */
    BHAB_3158_StandbyMode_ePassive,   /* S2 mode */
    BHAB_3158_StandbyMode_eDeepSleep, /* S3 mode */
    BHAB_3158_StandbyMode_eMax
} BHAB_3158_StandbyMode;

/***************************************************************************
Summary:
Standby Settings for LEAP
***************************************************************************/
typedef struct BHAB_3158_StandbySettings
{
    BHAB_3158_StandbyMode mode;
} BHAB_3158_StandbySettings;

/***************************************************************************
Summary:
	This function returns the default settings for 3158 module.

Description:
	This function is responsible for returns the default setting for
	3158 module. The returning default setting should be used when
	opening the device.

Returns:
	TODO:

See Also:
	BSPI_Open()

****************************************************************************/
BERR_Code BHAB_3158_GetDefaultSettings(
	BHAB_Settings * pDefSetting     /* [in] Default settings */
);

/***************************************************************************
Summary:
	This function returns the default recalibrate settings for a 3158 module.

Description:
	This function is responsible for returning the default recalibrate settings
    for a 3158 module.

Returns:
	TODO:

See Also:

****************************************************************************/
BERR_Code BHAB_3158_GetDefaultRecalibrateSettings(
    BHAB_RecalibrateSettings *pRecalibrateSettings /* [out] default recalibrate settings */
);

/***************************************************************************
Summary:
    Get the current LEAP standby settings.
***************************************************************************/
BERR_Code BHAB_3158_P_GetStandbySettings(
    BHAB_Handle handle,                         /* [in] Device handle */
    BHAB_3158_StandbySettings *pSettings        /* [out] Standby Settings */
    );

/***************************************************************************
Summary:
Use this function to put LEAP into and out of standby.

Description:

***************************************************************************/
BERR_Code BHAB_3158_P_SetStandbySettings(
    BHAB_Handle handle,                         /* [in] Device handle */
    const BHAB_3158_StandbySettings *pSettings  /* [in] Standby Settings */
    );

#ifdef __cplusplus
}
#endif

#endif
