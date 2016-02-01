/***************************************************************************
 *     Copyright (c) 2003-2011, Broadcom Corporation
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
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
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
#if (BCHP_CHIP==7400)
#define BGIO_P_NUM_CTRL_SETS           4
#else
#define BGIO_P_NUM_CTRL_SETS           3
#endif

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
