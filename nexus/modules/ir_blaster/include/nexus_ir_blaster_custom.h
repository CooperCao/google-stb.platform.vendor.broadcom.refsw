/***************************************************************************
*  Broadcom Proprietary and Confidential. (c)2004-2016 Broadcom. All rights reserved.
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
*
* API Description:
*   API name: IrBlaster
*    Specific APIs related to IR Blaster Control.
*
***************************************************************************/
#ifndef NEXUS_IR_BLASTER_CUSTOM_H__
#define NEXUS_IR_BLASTER_CUSTOM_H__

#include "nexus_ir_blaster.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
Custom IR Blaster Settings
***************************************************************************/
typedef struct NEXUS_IrBlasterIndexPair
{
    uint16_t on;        /* index of on duration */
    uint16_t off;       /* index of off duration */
} NEXUS_IrBlasterIndexPair;

/***************************************************************************
Summary:
IR Blaster Custom Settings -- Please refer to the Hardware Data Module
                              document for descriptions of these values
 ***************************************************************************/
typedef struct NEXUS_IrBlasterCustomSettings
{
    unsigned                reserved;                       /* do not set */
    uint8_t                 masterClockDivisor;             /* pre-scaler for the primary IRB clock */
    uint8_t                 carrierClockDivisor;            /* pre-scaler for carrier counter clock */
    uint8_t                 indexClockDivisor;              /* pre-scaler for modulation counter clock */
    uint8_t                 noCarrier;                      /* 0=output w/ carrier, 1=output w/out carrier */
    uint8_t                 carrierHighCount;               /* high time for the carrier frequency */
    uint8_t                 carrierLowCount;                /* low time for the carrier frequency */
    uint8_t                 numberSequences;                /* number of modulation on/off sequences */
    NEXUS_IrBlasterIndexPair logic0IndexPair;               /* pointer to mod_reg_file with logic0 */
    NEXUS_IrBlasterIndexPair logic1IndexPair;               /* pointer to mod_reg_file with logic1 */
    NEXUS_IrBlasterIndexPair sequenceIndex[161];            /* blast_seq_regfile */
    uint32_t                duration[14];                   /* blast_mod_regfile */
    uint32_t                framePeriod;                    /* IR packet frame period A */
    uint32_t                framePeriodB;                   /* IR packet frame period B */
    uint16_t                lastSequenceOffIndex;           /* index to the last sequence off duration */
    uint8_t                 repeatCount;                    /* IR blaster repeat number */
    uint8_t                 repeatStartIndex;               /* IR blaster repeat index */
} NEXUS_IrBlasterCustomSettings;

void NEXUS_IrBlaster_GetDefaultCustomSettings(
    NEXUS_IrBlasterCustomSettings *pSettings
    );

NEXUS_Error NEXUS_IrBlaster_SetCustomSettings(
    NEXUS_IrBlasterHandle irBlaster,
    const NEXUS_IrBlasterCustomSettings *pSettings
    );
    
#ifdef __cplusplus
}
#endif

#endif
