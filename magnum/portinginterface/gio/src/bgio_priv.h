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
#ifndef BGIO_PRIV_H__
#define BGIO_PRIV_H__

#include "bgio.h"
#include "bgio_macro.h"
#include "bgio_pin_priv.h"
#include "blst_list.h"
#include "bchp_common.h"
#ifdef BCHP_GIO_AON_REG_START
#include "bchp_gio_aon.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

BDBG_OBJECT_ID_DECLARE(BGIO);

#define BGIO_P_MAIN_SET_BLACK_MAGIC(handle) \
    BGIO_GENERIC_SET_BLACK_MAGIC((handle), BGIO_P_Context)

/* define PinContext head struct */
typedef struct BGIO_P_Pin_Context_Head BGIO_P_Pin_Context_Head;
BLST_D_HEAD(BGIO_P_Pin_Context_Head, BGIO_P_Pin_Context);

#define BGIO_P_BIT_MASK(bit)           (1 << (bit))
#define BGIO_P_REG_BASE                BCHP_GIO_ODEN_LO
#define BGIO_P_REG_LOW_TOP             BCHP_GIO_STAT_LO
#define BGIO_P_NUM_LOW_REGS            8
#ifdef BCHP_GIO_AON_ODEN_LO
#define BGIO_P_AON_BASE                BCHP_GIO_AON_ODEN_LO
#else
#define BGIO_P_AON_BASE                0
#endif
#define BGIO_P_NUM_CTRL_SETS           6

/***************************************************************************
 * BGIO main Context
 */
typedef struct BGIO_P_Context
{
    BDBG_OBJECT(BGIO)

    uint32_t   ulBlackMagic;   /* Black magic for handle validation */

    /* handed down from up level sw */
    BCHP_Handle    hChip;
    BREG_Handle    hRegister;

    /* records of values set to the data bits of open drain pins */
    uint32_t  aulOpenDrainSet[BGIO_P_NUM_LOW_REGS * BGIO_P_NUM_CTRL_SETS];

    /* link list for managed pin records */
    BGIO_P_Pin_Context_Head  PinHead;

} BGIO_P_Context;

/***************************************************************************
 *
 * API functions
 *
 ***************************************************************************/

/*--------------------------------------------------------------------------
 * To be called to init HW registers as BGIO openning
 */
BERR_Code BGIO_P_InitReg(
    BGIO_P_Context *      pGpio,
    BREG_Handle           hRegister );

/***************************************************************************
 *
 * Utility functions
 *
 ***************************************************************************/

/*--------------------------------------------------------------------------
 * To be called by BGIO_P_WritePinRegBit and BGIO_P_ReadPinRegBit to calculate the
 * register offset relative to BGIO_P_REG_BASE and the bit offset
 * relative to bit 0, based on pin ID.
 */
BERR_Code BGIO_P_CalcPinRegAndBit(
    BGIO_PinId            ePinId,
    uint32_t              ulRegLow,        /* corresponding reg_low */
    uint32_t *            pulRegOffset,
    uint32_t *            pulBitOffset );

/*--------------------------------------------------------------------------
 * To be called to write the GPIO pin's bit into one register
 */
BERR_Code BGIO_P_WritePinRegBit(
    BGIO_Handle           hGpio,
    BGIO_PinId            ePinId,
    BGIO_PinType          ePinType,
    uint32_t              ulRegLow,
    BGIO_PinValue         ePinValue,
    bool                  bInIsr );

/*--------------------------------------------------------------------------
 * To be called to write the GPIO pin's bit into one register
 */
BERR_Code BGIO_P_ReadPinRegBit(
    BGIO_Handle           hGpio,
    BGIO_PinId            ePinId,
    uint32_t              ulRegLow,
    BGIO_PinValue *       pePinValue );

/***************************************************************************
 * To be called to add a pin handle into the pin list in BGIO's main
 * context
 */
BERR_Code BGIO_P_AddPinToList(
    BGIO_Handle           hGpio,
    BGIO_Pin_Handle       hPin );

/***************************************************************************
 * To be called to remove a pin handle from the pin list in BGIO's main
 * context
 */
BERR_Code BGIO_P_RemovePinFromList(
    BGIO_Handle           hGpio,
    BGIO_Pin_Handle       hPin );

/*--------------------------------------------------------------------------
 * To be called to get the pin handle for a PinId from the pin list in
 * BGIO's main context. NULL returned if it does not exist.
 */
BGIO_Pin_Handle BGIO_P_GetPinHandle(
    BGIO_Handle           hGpio,
    BGIO_PinId            ePinId );

/***************************************************************************
 * To be called to get the register handle
 */
BREG_Handle BGIO_P_GetRegisterHandle(
    BGIO_Handle  hGpio );


#ifdef __cplusplus
}
#endif

#endif /* #ifndef BGIO_PRIV_H__ */

/* end of file */
