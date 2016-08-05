/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/
#ifndef _BDSQ_PRIV_H_
#define _BDSQ_PRIV_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "bdsq.h"


/* phy config enum */
typedef enum BDSQ_AdcMode
{
   BDSQ_AdcMode_eRx = 0,
   BDSQ_AdcMode_eVsense
} BDSQ_AdcMode;


struct BDSQ_P_ChannelHandle;


/******************************************************************************
Summary:
   This is the structure for the BDSQ_Handle. 
******************************************************************************/
typedef struct BDSQ_P_Handle
{
   BDSQ_Settings               settings;   /* user settings */
   uint8_t                     totalChannels;
   struct BDSQ_P_ChannelHandle **pChannels;
   void                        *pImpl;     /* pointer to chip-specific structure */
} BDSQ_P_Handle;


/******************************************************************************
Summary:
   This is the structure for the BDSQ_ChannelHandle. 
******************************************************************************/
typedef struct BDSQ_P_ChannelHandle
{
   BDSQ_ChannelSettings settings;
   struct BDSQ_P_Handle *pDevice;
   uint8_t              channel;
   bool                 bEnabled;   /* diseqc enabled */
   void                 *pImpl;     /* pointer to chip-specific structure */
} BDSQ_P_ChannelHandle;


/* private functions */
BERR_Code BDSQ_P_ReadRegister_isrsafe(BDSQ_ChannelHandle h, uint32_t reg, uint32_t *val);
BERR_Code BDSQ_P_WriteRegister_isrsafe(BDSQ_ChannelHandle h, uint32_t reg, uint32_t val);
BERR_Code BDSQ_P_PowerDownDsecPhy(BDSQ_ChannelHandle h);
BERR_Code BDSQ_P_PowerUpDsecPhy(BDSQ_ChannelHandle h);
BERR_Code BDSQ_P_PowerDownVsensePhy(BDSQ_ChannelHandle h);
BERR_Code BDSQ_P_PowerUpVsensePhy(BDSQ_ChannelHandle h);
BERR_Code BDSQ_P_ReadVsenseAdc(BDSQ_ChannelHandle h, uint16_t *pVal);
BERR_Code BDSQ_P_GetTone(BDSQ_ChannelHandle h, bool *pbTone);

/* bdsq_priv.c */
void BDSQ_P_ReadModifyWriteRegister_isr(BDSQ_ChannelHandle h, uint32_t reg, uint32_t and_mask, uint32_t or_mask);
void BDSQ_P_OrRegister_isr(BDSQ_ChannelHandle h, uint32_t reg, uint32_t or_mask);
void BDSQ_P_AndRegister_isr(BDSQ_ChannelHandle h, uint32_t reg, uint32_t and_mask);
void BDSQ_P_ToggleBit_isr(BDSQ_ChannelHandle h, uint32_t reg, uint32_t mask);
void BDSQ_P_ReadModifyWriteRegister(BDSQ_ChannelHandle h, uint32_t reg, uint32_t and_mask, uint32_t or_mask);
void BDSQ_P_OrRegister(BDSQ_ChannelHandle h, uint32_t reg, uint32_t or_mask);
void BDSQ_P_AndRegister(BDSQ_ChannelHandle h, uint32_t reg, uint32_t and_mask);
void BDSQ_P_ToggleBit(BDSQ_ChannelHandle h, uint32_t reg, uint32_t mask);


#ifdef __cplusplus
}
#endif

#endif /* _BDSQ_PRIV_H_ */

