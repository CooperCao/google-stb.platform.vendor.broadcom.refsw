/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

/*******************************************************************************
 *
 * DESCRIPTION:
 *
 *
*******************************************************************************/

#ifndef _BB_MAIL_POWER_FILTER_KEY_H
#define _BB_MAIL_POWER_FILTER_KEY_H


#define RF4CE_WAKE_UP_ACTION_CODE_FILTER_LENGTH  (256/8)

typedef struct _RF4CE_ZRC_SetWakeUpActionCodeReqDescr_t  RF4CE_ZRC_SetWakeUpActionCodeReqDescr_t;
typedef struct _RF4CE_ZRC_SetWakeUpActionCodeConfParams_t RF4CE_ZRC_SetWakeUpActionCodeConfParams_t;
typedef struct _RF4CE_ZRC_SetWakeUpActionCodeConfParams_t RF4CE_ZRC_GetWakeUpActionCodeConfParams_t;
typedef struct _RF4CE_ZRC_GetWakeUpActionCodeReqDescr_t  RF4CE_ZRC_GetWakeUpActionCodeReqDescr_t;

/****************************************************************************************
  \brief
  \wakeUpActionCodeFilter - the action code filter to wake up Host CPU.  Each bit location corresponds to the HDMI-CEC code.
  \status - 0 if success, otherwise fail.
*****************************************************************************************/
typedef struct _RF4CE_ZRC_SetWakeUpActionCodeConfParams_t
{
    uint8_t wakeUpActionCodeFilter[RF4CE_WAKE_UP_ACTION_CODE_FILTER_LENGTH];
    uint8_t status;
} RF4CE_ZRC_SetWakeUpActionCodeConfParams_t;

/**//**
 * \brief Set wake-up action code parameters type
 */
typedef struct _RF4CE_ZRC_SetWakeUpActionCodeReqParams_t
{
    uint8_t wakeUpActionCodeFilter[RF4CE_WAKE_UP_ACTION_CODE_FILTER_LENGTH];
} RF4CE_ZRC_SetWakeUpActionCodeReqParams_t;


/**//**
 * \brief Set wake-up action code request type
 */
typedef struct _RF4CE_ZRC_SetWakeUpActionCodeReqDescr_t
{
    RF4CE_ZRC_SetWakeUpActionCodeReqParams_t params;
    void (*callback)(RF4CE_ZRC_SetWakeUpActionCodeReqDescr_t *, RF4CE_ZRC_SetWakeUpActionCodeConfParams_t *);
} RF4CE_ZRC_SetWakeUpActionCodeReqDescr_t;

/**//**
 * \brief Get wake-up action code request type
 */
typedef struct _RF4CE_ZRC_GetWakeUpActionCodeReqDescr_t
{
    void (*callback)(RF4CE_ZRC_GetWakeUpActionCodeReqDescr_t *, RF4CE_ZRC_GetWakeUpActionCodeConfParams_t *);
} RF4CE_ZRC_GetWakeUpActionCodeReqDescr_t;

void RF4CE_ZRC_SetWakeUpActionCodeReq(RF4CE_ZRC_SetWakeUpActionCodeReqDescr_t *req);
void RF4CE_ZRC_GetWakeUpActionCodeReq(RF4CE_ZRC_GetWakeUpActionCodeReqDescr_t *req);
uint8_t RF4CE_ShouldWakeUpHostCpu(uint8_t keyCode);
void RF4CE_PMSetHostWakingUp(bool wakeup);

#endif

/* eof bbExtPowerFilterKey.h */