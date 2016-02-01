/******************************************************************************
*    (c)2011-2013 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* Module Description:
*
* Revision History:
*
* $brcm_Log: $
* 
*****************************************************************************/
#ifndef _BDSQ_G1_H_
#define _BDSQ_G1_H_


/* define device configuration parameters */
/* TBD... */

/* define channel configuration parameters */
enum {
   BDSQ_g1_CONFIG_RRTO_US,             /* diseqc receive reply timeout in usecs */
   BDSQ_g1_CONFIG_BIT_THRESHOLD_US,    /* threshold for diseqc rx bit detection in usecs */
   BDSQ_g1_CONFIG_TONE_THRESHOLD,      /* threshold for diseqc energy tone detect in units of 0.16 counts per mV */
   BDSQ_g1_CONFIG_PRETX_DELAY_MS,      /* delay time in msecs before transmitting diseqc command */
   BDSQ_g1_CONFIG_VSENSE_THRESHOLD_HI, /* voltage detect overvoltage threshold, binary offset format */
   BDSQ_g1_CONFIG_VSENSE_THRESHOLD_LO, /* voltage detect undervoltage threshold, binary offset format */
   BDSQ_g1_CONFIG_PGA_GAIN,            /* receiver pga gain */
   BDSQ_g1_CONFIG_ENABLE_LNBPU,        /* 0=LNBPU not used , 1=LNBPU on TXEN pin */
   BDSQ_g1_CONFIG_RX_TONE_MODE,        /* TONMDSEL selects tone mode for receiver */
   BDSQ_g1_CONFIG_IDLE_TIMEOUT_US,     /* diseqc idle timeout is usecs */
   BDSQ_g1_CONFIG_RX_BIT_TIMING,       /* rx bit timing */
   BDSQ_g1_CONFIG_CIC_LEN,             /* fast cic filter length */
   BDSQ_g1_CONFIG_CIC_MIN_THRESH,      /* fast cic min threshold */
   BDSQ_g1_CONFIG_CIC_DELTA_THRESH,    /* fast cic delta threshold */
   BDSQ_g1_CONFIG_MAX
};


/* interrupts */
typedef enum BDSQ_g1_IntID{
   BDSQ_g1_IntID_eTimer1 = 0,
   BDSQ_g1_IntID_eTimer2,
   BDSQ_g1_IntID_eDiseqcDone,
   BDSQ_g1_IntID_eTxAlmostEmpty,
   BDSQ_g1_IntID_eRxAlmostEmpty,
   BDSQ_g1_IntID_eAcwDone,
   BDSQ_g1_IntID_eUnderVoltage,
   BDSQ_g1_IntID_eOverVoltage,
   BDSQ_g1_IntID_eToneRise,
   BDSQ_g1_IntID_eToneFall,
   BDSQ_g1_IntID_eIdleTimeout,
   BDSQ_g1_MaxIntID
} BDSQ_g1_IntID;

/* timers */
typedef enum BDSQ_g1_TimerSelect
{
   BDSQ_g1_TimerSelect_e1 = 0,
   BDSQ_g1_TimerSelect_e2
} BDSQ_g1_TimerSelect;


/* BDSQ G1 public functions */
BERR_Code BDSQ_g1_GetDefaultSettings(BDSQ_Settings*);
BERR_Code BDSQ_g1_GetChannelDefaultSettings(BDSQ_Handle, uint32_t, BDSQ_ChannelSettings *);


#ifdef BDSQ_EXCLUDE_API_TABLE
#include "bdsq_g1_priv.h"
#define BDSQ_Open                          BDSQ_g1_P_Open
#define BDSQ_Close                         BDSQ_g1_P_Close
#define BDSQ_GetTotalChannels              BDSQ_g1_P_GetTotalChannels
#define BDSQ_GetChannelDefaultSettings     BDSQ_g1_GetChannelDefaultSettings
#define BDSQ_OpenChannel                   BDSQ_g1_P_OpenChannel
#define BDSQ_CloseChannel                  BDSQ_g1_P_CloseChannel
#define BDSQ_GetDevice                     BDSQ_g1_P_GetDevice
#define BDSQ_GetVersionInfo                BDSQ_g1_P_GetVersionInfo
#define BDSQ_Reset                         BDSQ_g1_P_Reset
#define BDSQ_ResetChannel                  BDSQ_g1_P_ResetChannel
#define BDSQ_PowerDownChannel              BDSQ_g1_P_PowerDownChannel
#define BDSQ_PowerUpChannel                BDSQ_g1_P_PowerUpChannel
#define BDSQ_IsChannelOn                   BDSQ_g1_P_IsChannelOn
#define BDSQ_PowerDownVsense               BDSQ_g1_P_PowerDownVsense
#define BDSQ_PowerUpVsense                 BDSQ_g1_P_PowerUpVsense
#define BDSQ_IsVsenseOn                    BDSQ_g1_P_IsVsenseOn
#define BDSQ_SetTone                       BDSQ_g1_P_SetTone
#define BDSQ_GetTone                       BDSQ_g1_P_GetTone
#define BDSQ_SetVoltage                    BDSQ_g1_P_SetVoltage
#define BDSQ_GetVoltage                    BDSQ_g1_P_GetVoltage
#define BDSQ_EnableVsenseInterrupt         BDSQ_g1_P_EnableVsenseInterrupt
#define BDSQ_EnableActivityInterrupt       BDSQ_g1_P_EnableActivityInterrupt
#define BDSQ_EnableIdleInterrupt           BDSQ_g1_P_EnableIdleInterrupt
#define BDSQ_Write                         BDSQ_g1_P_Write
#define BDSQ_EnableRx                      BDSQ_g1_P_EnableRx
#define BDSQ_EnableLnb                     BDSQ_g1_P_EnableLnb
#define BDSQ_Read                          BDSQ_g1_P_Read
#define BDSQ_SendAcw                       BDSQ_g1_P_SendAcw
#define BDSQ_GetStatus                     BDSQ_g1_P_GetStatus
#define BDSQ_GetActivityStatus             BDSQ_g1_P_GetActivityStatus
#define BDSQ_ResetActivityStatus           BDSQ_g1_P_ResetActivityStatus
#define BDSQ_SetChannelSettings            BDSQ_g1_P_SetChannelSettings
#define BDSQ_GetChannelSettings            BDSQ_g1_P_GetChannelSettings
#define BDSQ_SetConfig                     BDSQ_g1_P_SetConfig
#define BDSQ_GetConfig                     BDSQ_g1_P_GetConfig
#define BDSQ_SetChannelConfig              BDSQ_g1_P_SetChannelConfig
#define BDSQ_GetChannelConfig              BDSQ_g1_P_GetChannelConfig
#define BDSQ_GetTxEventHandle              BDSQ_g1_P_GetTxEventHandle
#define BDSQ_GetRxEventHandle              BDSQ_g1_P_GetRxEventHandle
#define BDSQ_GetVsenseEventHandle          BDSQ_g1_P_GetVsenseEventHandle
#define BDSQ_GetActivityEventHandle        BDSQ_g1_P_GetActivityEventHandle
#define BDSQ_GetIdleEventHandle            BDSQ_g1_P_GetIdleEventHandle
#endif

 
#endif /* _BDSQ_G1_H_ */
