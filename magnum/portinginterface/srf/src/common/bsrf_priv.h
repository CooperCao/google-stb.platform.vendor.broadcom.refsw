/***************************************************************************
 *     Copyright (c) 2005-2014, Broadcom Corporation
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
#ifndef BSRF_PRIV_H__
#define BSRF_PRIV_H__

#include "bsrf.h"


#define BSRF_CHK_RETCODE(x) \
   { if ((retCode = (x)) != BERR_SUCCESS) goto done; }

#define BSRF_ABS(x) (((x) < 0) ? -(x):(x))


struct BSRF_P_ChannelHandle;

/******************************************************************************
Summary:
   This is the structure for the BSRF_Handle.
******************************************************************************/
typedef struct BSRF_P_Handle
{
   BSRF_Settings                 settings;      /* user settings */
   uint8_t                       totalChannels; /* total channels */
   struct BSRF_P_ChannelHandle   **pChannels;   /* pointer to array of channel handles */
   void                          *pImpl;        /* pointer to specific implementation */
} BSRF_P_Handle;


/******************************************************************************
Summary:
   This is the structure for the BSRF_ChannelHandle.
******************************************************************************/
typedef struct BSRF_P_ChannelHandle
{
   BSRF_ChannelSettings settings;      /* channel settings */
   BSRF_P_Handle        *pDevice;      /* pointer to device handle */
   uint8_t              channel;       /* channel number */
   bool                 bEnabled;      /* channel enabled */
   void                 *pImpl;        /* pointer to specific implementation */
} BSRF_P_ChannelHandle;


/* private chip functions */
BERR_Code BSRF_P_ReadRegister(BSRF_ChannelHandle h, uint32_t reg, uint32_t *val);
BERR_Code BSRF_P_WriteRegister(BSRF_ChannelHandle h, uint32_t reg, uint32_t val);

/* bsrf_priv.c */
void BSRF_P_ReadModifyWriteRegister(BSRF_ChannelHandle h, uint32_t reg, uint32_t and_mask, uint32_t or_mask);
void BSRF_P_OrRegister(BSRF_ChannelHandle h, uint32_t reg, uint32_t or_mask);
void BSRF_P_AndRegister(BSRF_ChannelHandle h, uint32_t reg, uint32_t and_mask);
void BSRF_P_ToggleBit(BSRF_ChannelHandle h, uint32_t reg, uint32_t mask);
uint32_t BSRF_P_GCF(uint32_t m, uint32_t n);


#endif /* BSRF_PRIV_H__ */
