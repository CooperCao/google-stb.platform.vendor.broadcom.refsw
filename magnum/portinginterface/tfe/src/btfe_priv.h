/***************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ***************************************************************************/

#ifndef BTFE_PRIV_H__
#define BTFE_PRIV_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "btfe.h"
#include "bhab.h"
#include "bchp_dfe_miscdec.h"

#define BTFE_MAJOR_VERSION 1
#define BTFE_MINOR_VERSION 5
 
#define BTFE_MAX_CHANNELS 1

#define BCHP_INT_ID_DFE_LOCK_STATUS_CHANGE  BCHP_INT_ID_CREATE(BCHP_DFE_MISCDEC_MISC_DFE_INT_STATUS, 2)
#define BCHP_INT_ID_DFE_AUDIO_CLIPPED       BCHP_INT_ID_CREATE(BCHP_DFE_MISCDEC_MISC_DFE_INT_STATUS, 0)
#define BCHP_INT_ID_DFE_SCAN                BCHP_INT_ID_CREATE(BCHP_DFE_MISCDEC_MISC_DFE_INT_STATUS, 1)

#define BTFE_IRQ_AUDIO_CLIPPED 0x01
#define BTFE_IRQ_SCAN          0x02
#define BTFE_IRQ_LOCK_CHANGE   0x04
#define BTFE_IRQ_ALL           0x07


#define QAM64_SYMBOL_RATE               (5056941)
#define QAM256_SYMBOL_RATE             (5360537)


 /******************************************************************************
Summary:
   This is the structure for the BTFE_Handle. 
******************************************************************************/
BDBG_OBJECT_ID_DECLARE(BTFE);
typedef struct BTFE_P_Handle
{
   BDBG_OBJECT(BTFE)
   BTFE_Settings        settings;   /* user settings */
   uint8_t              totalChannels;
   struct BTFE_P_ChannelHandle **pChannels;
   BHAB_DevId devId;
   BINT_Handle          hInterrupt;
   BREG_Handle          hRegister;
   BCHP_Handle          hChip;     /* chip handle passed into BTFE_Open() */
   BTMR_TimerHandle     hTimer;
   BTMR_TimerHandle		hStatusTimer;
   BKNI_EventHandle     hBBSIntEvent;
   BHAB_Handle hHab;
} BTFE_P_Handle;


/******************************************************************************
Summary:
   This is the structure for the BTFE_ChannelHandle. 
******************************************************************************/
BDBG_OBJECT_ID_DECLARE(BTFE_Channel);
typedef struct BTFE_P_ChannelHandle
{
   BDBG_OBJECT(BTFE_Channel)
   BTFE_ChannelSettings settings;
   BTFE_P_Handle        *pDevice;
   uint8_t              channel;
   BHAB_DevId devId;
   BKNI_EventHandle     hLockStateChangeEvent;
   BKNI_EventHandle     hAudioMaxThresholdEvent;
   BKNI_EventHandle     hScanEvent;
   BINT_CallbackHandle  hLockStatusChangeCb;   /* callback handle for lock change interrupt */   
   BINT_CallbackHandle  hAudioClippedCb;       /* callback handle for audio clipped interrupt */     
   BINT_CallbackHandle  hScanCb;               /* callback handle for scan interrupt */
   BTFE_CallbackFunc pCallback[BTFE_Callback_eLast];
   void *pCallbackParam[BTFE_Callback_eLast];
   void                 *pFatHandle;
   BTFE_AcquireParams previousAcquireParams;
   bool bPowerdown;
   BHAB_Handle hHab;
#ifdef BTFE_SUPPORTS_SHARED_MEMORY
   BTFE_BBS_SharedMemory *sharedMemory;
   BTFE_BBS_AcquireParams *acqParams;  
   BTFE_StatusFAT *fatStatus;
   BTFE_StatusConstellationData *constellationData;
#endif
} BTFE_P_ChannelHandle;

void BTFE_MultU32U32(uint32_t A, uint32_t B, uint32_t *P_hi, uint32_t *P_lo);
void BTFE_DivU64U32(uint32_t A_hi, uint32_t A_lo, uint32_t B, uint32_t *Q_hi, uint32_t *Q_lo);
void BTFE_P_LockStatusChange_isr(void *p, int param);
void BTFE_P_AudioClipped_isr(void *p, int param);
void BTFE_P_Scan_isr(void *p, int param);
void BTFE_P_EnableIrq(BTFE_ChannelHandle hTFEChan, uint32_t irq);
void BTFE_P_DisableIrq(BTFE_ChannelHandle hTFEChan, uint32_t irq);

#ifdef __cplusplus
}
#endif

#endif /* BTFE_PRIV_H__ */
