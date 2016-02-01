/***************************************************************************
 *     Copyright (c) 2004-2011, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
 

#ifndef BTFE_PRIV_H__
#define BTFE_PRIV_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "btfe.h"
#include "bchp_dfe_miscdec.h"

#define BTFE_MAJOR_VERSION 1
#define BTFE_MINOR_VERSION 33
 
#define BTFE_35230_MAX_CHANNELS 1

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
typedef struct BTFE_P_Handle
{
   BTFE_Settings        settings;   /* user settings */
   uint8_t              totalChannels;
   struct BTFE_P_ChannelHandle **pChannels;
   BINT_Handle          hInterrupt;
   BREG_Handle          hRegister;
   BCHP_Handle          hChip;     /* chip handle passed into BTFE_Open() */
} BTFE_P_Handle;


/******************************************************************************
Summary:
   This is the structure for the BTFE_ChannelHandle. 
******************************************************************************/
typedef struct BTFE_P_ChannelHandle
{
   BTFE_ChannelSettings settings;
   BTFE_P_Handle        *pDevice;
   uint8_t              channel;
   BKNI_EventHandle     hLockStateChangeEvent;
   BKNI_EventHandle     hAudioMaxThresholdEvent;  
   BKNI_EventHandle     hScanEvent;
   BINT_CallbackHandle  hLockStatusChangeCb;   /* callback handle for lock change interrupt */   
   BINT_CallbackHandle  hAudioClippedCb;       /* callback handle for audio clipped interrupt */     
   BINT_CallbackHandle  hScanCb;               /* callback handle for scan interrupt */
   void                 *pFatHandle;   
} BTFE_P_ChannelHandle;

void BTFE_MultU32U32(uint32_t A, uint32_t B, uint32_t *P_hi, uint32_t *P_lo);
void BTFE_DivU64U32(uint32_t A_hi, uint32_t A_lo, uint32_t B, uint32_t *Q_hi, uint32_t *Q_lo);
void BTFE_P_LockStatusChange_isr(void *p, int param);
void BTFE_P_AudioClipped_isr(void *p, int param);
void BTFE_P_Scan_isr(void *p, int param);
void BTFE_P_EnableIrq(BTFE_ChannelHandle hTFEChan, uint32_t irq);
void BTFE_P_DisableIrq(BTFE_ChannelHandle hTFEChan, uint32_t irq);

BERR_Code BTFE_P_LogSearch(BTFE_ChannelHandle hTFEChan, BTFE_DvbcScanData *pScanData, int32_t defaultIFNomRate,int32_t *coarseOffset);

#ifdef __cplusplus
}
#endif

#endif /* BTFE_PRIV_H__ */
