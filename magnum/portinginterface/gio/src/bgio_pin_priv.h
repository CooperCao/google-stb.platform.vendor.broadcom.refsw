/***************************************************************************
 *     Copyright (c) 2003-2012, Broadcom Corporation
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
