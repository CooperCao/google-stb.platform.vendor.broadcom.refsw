/***************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
 * Module Description:
 *
 ***************************************************************************/
#ifndef BGIO_PIN_PRIV_H__
#define BGIO_PIN_PRIV_H__

#include "blst_list.h"
#include "bgio.h"
#include "bgio_macro.h"

#ifdef __cplusplus
extern "C" {
#endif

BDBG_OBJECT_ID_DECLARE(BGIO_PIN);

#define BGIO_P_PIN_SET_BLACK_MAGIC(handle) \
    BGIO_GENERIC_SET_BLACK_MAGIC((handle), BGIO_P_Pin_Context)

#define BGIO_P_NULL_REG          0
#define BGIO_P_GIO_REG           1

/***************************************************************************
 * Pin Context
 */
typedef struct BGIO_P_Pin_Context
{
    BDBG_OBJECT(BGIO_PIN)

    uint32_t   ulBlackMagic;   /* Black magic for handle validation */

    /* created from this handle */
    BGIO_Handle  hGpio;

    /* pin info */
    BGIO_PinId     ePinId;
    BGIO_PinType   ePinType;
    BGIO_IntrMode  eIntrMode;

    /* struct {pNext, pPre} for linking */
    BLST_D_ENTRY(BGIO_P_Pin_Context) Link;

} BGIO_P_Pin_Context;

/***************************************************************************
 * Pin mux control for one pin
 */
typedef struct BGIO_P_PinMux
{
    BGIO_PinId ePinId;        /* pin number */
    uint32_t   ulReg;         /* register addr offset */
    uint32_t   ulBitMask;     /* bit mask for this pin in the reg */
    uint32_t   ulValue;       /* mux value set for this pin in the reg */

} BGIO_P_PinMux;

/***************************************************************************
 * Pin mux control setting in GIO register
 * Note: If adding new set here, make sure to increase number of control
 * set defined in bgio_priv.h
 */
typedef struct BGIO_P_PinSet
{
    BGIO_PinId   eSetLoStart;         /* First pin of Set Lo */
    BGIO_PinId   eSetLoEnd;           /* Last pin of Set Lo */
    BGIO_PinId   eSetHiStart;         /* First pin of Set Hi */
    BGIO_PinId   eSetHiEnd;           /* Last pin of Set Hi */
    BGIO_PinId   eSetExtStart;        /* First pin of Set Ext */
    BGIO_PinId   eSetExtEnd;          /* Last pin of Set Ext */
    BGIO_PinId   eSetExtHiStart;      /* First pin of Set Ext Hi */
    BGIO_PinId   eSetExtHiEnd;        /* Last pin of Set Ext Hi */
    BGIO_PinId   eSetExt2Start;       /* First pin of Set Ext2 */
    BGIO_PinId   eSetExt2End;         /* Last pin of Set Ext2 */
    BGIO_PinId   eSetExt3Start;       /* First pin of Set Ext3 */
    BGIO_PinId   eSetExt3End;         /* Last pin of Set Ext3 */
    uint32_t     ulSetSgio;           /* The set number where sgio pins are located */
    uint32_t     ulSetSgioShift;      /* Number of bit shift in sgio set */
} BGIO_P_PinSet;

/***************************************************************************
 *
 * API support functions
 *
 ***************************************************************************/

/***************************************************************************
 *
 */
BERR_Code BGIO_P_Pin_Create(
    BGIO_Handle           hGpio,
    BGIO_PinId            ePinId,
    BGIO_Pin_Handle *     phPin );

/***************************************************************************
 *
 */
BERR_Code BGIO_P_Pin_Destroy(
    BGIO_Pin_Handle       hPin );

/***************************************************************************
 *
 */
BERR_Code BGIO_P_Pin_SetType(
    BGIO_Pin_Handle       hPin,
    BGIO_PinType          ePinType,
    bool                  bInIsr );

/***************************************************************************
 *
 */
BERR_Code BGIO_P_Pin_GetValue(
    BGIO_Pin_Handle       hPin,
    BGIO_PinValue *       pePinValue );

/***************************************************************************
 *
 */
BERR_Code BGIO_P_Pin_SetValue(
    BGIO_Pin_Handle       hPin,
    BGIO_PinValue         ePinValue,
    bool                  bInIsr );

/***************************************************************************
 *
 */
BERR_Code BGIO_P_Pin_SetIntrMode(
    BGIO_Pin_Handle       hPin,
    BGIO_IntrMode         eIntrMode,
    bool                  bInIsr );

/***************************************************************************
 *
 */
BERR_Code BGIO_P_Pin_ClearIntrStatus(
    BGIO_Pin_Handle       hPin,
    bool                  bInIsr );

/***************************************************************************
 *
 */
BERR_Code BGIO_P_Pin_GetIntrStatus(
    BGIO_Pin_Handle       hPin,
    bool *                pbFire );

/***************************************************************************
 *
 */
const BGIO_P_PinMux * BGIO_P_GetPinMux(
    BGIO_PinId            ePinId );

const BGIO_P_PinSet * BGIO_P_GetPinMapping( void );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BGIO_PIN_PRIV_H__ */

/* end of file */
