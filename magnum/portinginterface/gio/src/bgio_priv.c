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
 *
 * Revision History:
 * $brcm_Log: $
 *
 ***************************************************************************/

#include "bgio_priv.h"
#include "bchp_gio.h"
#include "bkni.h"

BDBG_MODULE(BGIO);


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
	uint32_t *            pulBitOffset )
{
	const BGIO_P_PinSet  *pGioPinSet = BGIO_P_GetPinMapping();

	/* assert para from our private code */
	BDBG_ASSERT( NULL != pGioPinSet );
	BDBG_ASSERT( BGIO_PinId_eInvalid > ePinId );
	BDBG_ASSERT( (BGIO_P_REG_BASE <= ulRegLow) &&
				 (ulRegLow <= BGIO_P_REG_LOW_TOP ) );
	BDBG_ASSERT( NULL != pulRegOffset );
	BDBG_ASSERT( NULL != pulBitOffset );

	/* calc register and bit offset */
	if(pGioPinSet->eSetLoStart != BGIO_PinId_eInvalid &&
	   pGioPinSet->eSetLoEnd   != BGIO_PinId_eInvalid &&
	   ePinId <= pGioPinSet->eSetLoEnd && ePinId < BGIO_PinId_eSgpio00)
	{
		/* _LO */
		*pulRegOffset = (BCHP_GIO_ODEN_LO - BCHP_GIO_REG_START) + (ulRegLow - BCHP_GIO_REG_START);
		*pulBitOffset = ePinId - pGioPinSet->eSetLoStart;
		if(pGioPinSet->ulSetSgio == 0)
			*pulBitOffset += pGioPinSet->ulSetSgioShift;
	}
	else if(pGioPinSet->eSetHiStart != BGIO_PinId_eInvalid &&
	        pGioPinSet->eSetHiEnd   != BGIO_PinId_eInvalid &&
	        ePinId <= pGioPinSet->eSetHiEnd && ePinId < BGIO_PinId_eSgpio00)
	{
		/* _HI */
		*pulRegOffset = (BCHP_GIO_ODEN_HI - BCHP_GIO_REG_START) + (ulRegLow - BCHP_GIO_REG_START);
		*pulBitOffset = ePinId - pGioPinSet->eSetHiStart;
		if(pGioPinSet->ulSetSgio == 1)
			*pulBitOffset += pGioPinSet->ulSetSgioShift;
	}
	else if(pGioPinSet->eSetExtStart != BGIO_PinId_eInvalid &&
	        pGioPinSet->eSetExtEnd   != BGIO_PinId_eInvalid &&
	        ePinId <= pGioPinSet->eSetExtEnd && ePinId < BGIO_PinId_eSgpio00)
	{
		/* _EXT */
#ifdef BCHP_GIO_ODEN_EXT
		*pulRegOffset = (BCHP_GIO_ODEN_EXT - BCHP_GIO_REG_START) + (ulRegLow - BCHP_GIO_REG_START);
#else
		BDBG_ASSERT(0);
#endif
		*pulBitOffset = (ePinId - pGioPinSet->eSetExtStart);
		if(pGioPinSet->ulSetSgio == 2)
			*pulBitOffset += pGioPinSet->ulSetSgioShift;
	}
	else if(pGioPinSet->eSetExtHiStart != BGIO_PinId_eInvalid &&
	        pGioPinSet->eSetExtHiEnd   != BGIO_PinId_eInvalid &&
	        ePinId <= pGioPinSet->eSetExtHiEnd && ePinId < BGIO_PinId_eSgpio00)
	{
		/* _EXT_HI */
#ifdef BCHP_GIO_ODEN_EXT_HI
		*pulRegOffset = (BCHP_GIO_ODEN_EXT_HI - BCHP_GIO_REG_START) + (ulRegLow - BCHP_GIO_REG_START);
#else
		BDBG_ASSERT(0);
#endif
		*pulBitOffset = ePinId - pGioPinSet->eSetExtHiStart;
		if(pGioPinSet->ulSetSgio == 3)
			*pulBitOffset += pGioPinSet->ulSetSgioShift;
	}
	else if(pGioPinSet->eSetExt2Start != BGIO_PinId_eInvalid &&
	        pGioPinSet->eSetExt2End   != BGIO_PinId_eInvalid &&
	        ePinId <= pGioPinSet->eSetExt2End && ePinId < BGIO_PinId_eSgpio00)
	{
		/* _EXT2 */
#ifdef BCHP_GIO_ODEN_EXT2
		*pulRegOffset = (BCHP_GIO_ODEN_EXT2 - BCHP_GIO_REG_START) + (ulRegLow - BCHP_GIO_REG_START);
#else
		BDBG_ASSERT(0);
#endif
		*pulBitOffset = ePinId - pGioPinSet->eSetExt2Start;
		if(pGioPinSet->ulSetSgio == 4)
			*pulBitOffset += pGioPinSet->ulSetSgioShift;
	}
	else if(pGioPinSet->eSetExt3Start != BGIO_PinId_eInvalid &&
	        pGioPinSet->eSetExt3End   != BGIO_PinId_eInvalid &&
	        ePinId <= pGioPinSet->eSetExt3End && ePinId < BGIO_PinId_eSgpio00)
	{
		/* _EXT3 */
#ifdef BCHP_GIO_ODEN_EXT3
		*pulRegOffset = (BCHP_GIO_ODEN_EXT3 - BCHP_GIO_REG_START) + (ulRegLow - BCHP_GIO_REG_START);
#else
		BDBG_ASSERT(0);
#endif
		*pulBitOffset = ePinId - pGioPinSet->eSetExt3Start;
		if(pGioPinSet->ulSetSgio == 5)
			*pulBitOffset += pGioPinSet->ulSetSgioShift;
	}
	else if (ePinId < BGIO_PinId_eSgpio00)
	{
		BDBG_ERR(("ePinId = %d doesn't match with pintable", ePinId));
		return BERR_INVALID_PARAMETER;
	}
	else if(ePinId < BGIO_PinId_eAgpio00)
	{
		/* special gpio pins */
		*pulRegOffset = (BGIO_P_NUM_LOW_REGS * 4 * pGioPinSet->ulSetSgio + ulRegLow) - BGIO_P_REG_BASE;
		*pulBitOffset = ePinId - BGIO_PinId_eSgpio00;
	}
	else if(ePinId < BGIO_PinId_eAsgpio00)
	{
		/* Aon GPIO */
		*pulRegOffset = ulRegLow - BGIO_P_REG_BASE;
		*pulBitOffset = ePinId - BGIO_PinId_eAgpio00;
	}
	else
	{
		/* Aon SGPIO */
		*pulRegOffset = (BGIO_P_NUM_LOW_REGS * 4 * 1 + ulRegLow) - BGIO_P_REG_BASE;
		*pulBitOffset = ePinId - BGIO_PinId_eAsgpio00;
	}

	return BERR_SUCCESS;
}

/*--------------------------------------------------------------------------
 * To be called to write the GPIO pin's bit into one register
 */
BERR_Code BGIO_P_WritePinRegBit(
	BGIO_Handle           hGpio,
	BGIO_PinId            ePinId,
	BGIO_PinType          ePinType,
	uint32_t              ulRegLow,
	BGIO_PinValue         ePinValue,
	bool                  bInIsr )
{
	BERR_Code eResult = BERR_SUCCESS;
	uint32_t  ulRegOffset, ulBitOffset;
	uint32_t  ulRegValue;
	uint32_t  ulRegIndex = 0;
	uint32_t  ulOpenDrainSet = 0;
	uint32_t  ulRegBase = (ePinId < BGIO_PinId_eAgpio00) ? BGIO_P_REG_BASE : BGIO_P_AON_BASE;

	/* check input para */
	BDBG_OBJECT_ASSERT(hGpio, BGIO);
	BDBG_ASSERT( BGIO_PinId_eInvalid > ePinId );
	BDBG_ASSERT( (BGIO_P_REG_BASE <= ulRegLow) &&
				 (ulRegLow <= BGIO_P_REG_LOW_TOP ) );
	BDBG_ASSERT( BGIO_PinValue_eInvalid > ePinValue );

	/* read the HW register and modify it for this setting */
	eResult = BGIO_P_CalcPinRegAndBit( ePinId, ulRegLow,
									   &ulRegOffset, &ulBitOffset );
	BDBG_ASSERT( BERR_SUCCESS == eResult );
	if(!bInIsr)
	{
		BKNI_EnterCriticalSection();
	}
	ulRegValue = BREG_Read32( hGpio->hRegister, ulRegBase + ulRegOffset );

	/* for the data of other pins of open drain type, we can not write 0 only if HW
	 * reading returns 0, since it might be due to that some other device is pulling
	 * down the bus */
	if (BCHP_GIO_DATA_LO == ulRegLow)
	{
		ulRegIndex = ulRegOffset / 4;
		ulOpenDrainSet = hGpio->aulOpenDrainSet[ulRegIndex];
		ulRegValue = ulRegValue | ulOpenDrainSet;
	}

	/* set new value to the bit */
	ulRegValue = ulRegValue & (~ BGIO_P_BIT_MASK(ulBitOffset));
	ulRegValue = (ePinValue << ulBitOffset) | ulRegValue;

	/* write to HW */
	BREG_Write32( hGpio->hRegister, ulRegBase + ulRegOffset, ulRegValue );
	if(!bInIsr)
	{
		BKNI_LeaveCriticalSection();
	}

	BDBG_MSG(("Write: RegAddr=0x%08x, RegValue=0x%08x", ulRegBase + ulRegOffset, ulRegValue));

	/* record open drain pin data set for future modification to this register by some other pin */
	if ((BCHP_GIO_DATA_LO == ulRegLow) && (BGIO_PinType_eOpenDrain == ePinType))
	{
		hGpio->aulOpenDrainSet[ulRegIndex] =
			(ulOpenDrainSet & (~ BGIO_P_BIT_MASK(ulBitOffset))) | (ePinValue << ulBitOffset);
	}

	return BERR_TRACE(eResult);
}

/*--------------------------------------------------------------------------
 * To be called to write the GPIO pin's bit into one register
 */
BERR_Code BGIO_P_ReadPinRegBit(
	BGIO_Handle           hGpio,
	BGIO_PinId            ePinId,
	uint32_t              ulRegLow,
	BGIO_PinValue *       pePinValue )
{
	BERR_Code eResult = BERR_SUCCESS;
	uint32_t  ulRegOffset, ulBitOffset;
	uint32_t  ulRegValue;
	BGIO_PinValue  ePinValue;
	uint32_t  ulRegBase = (ePinId < BGIO_PinId_eAgpio00) ? BGIO_P_REG_BASE : BGIO_P_AON_BASE;

	/* check input para */
	BDBG_OBJECT_ASSERT(hGpio, BGIO);
	BDBG_ASSERT( BGIO_PinId_eInvalid > ePinId );
	BDBG_ASSERT( (BGIO_P_REG_BASE <= ulRegLow) &&
				 (ulRegLow <= BGIO_P_REG_LOW_TOP ) );
	BDBG_ASSERT( NULL != pePinValue );

	/* read the HW reg
	 * note: should not modify pGpio's records for this register based on the
	 * reading from HW, since it might be diff from user's last setting, such
	 * in as open dran case, and pGpio's records for this register will be
	 * used in BGIO_P_WritePinRegBit */
	eResult = BGIO_P_CalcPinRegAndBit( ePinId, ulRegLow,
									   &ulRegOffset, &ulBitOffset );
	BDBG_ASSERT( BERR_SUCCESS == eResult );
	ulRegValue = BREG_Read32( hGpio->hRegister, ulRegBase + ulRegOffset );
	ePinValue = (ulRegValue & BGIO_P_BIT_MASK(ulBitOffset)) >> ulBitOffset;

	BDBG_MSG(("Read: RegAddr=0x%08x, RegValue=0x%08x", ulRegBase + ulRegOffset, ulRegValue));

	*pePinValue = ePinValue;
	return BERR_TRACE(eResult);
}

/***************************************************************************
 * To be called to add a pin handle into the pin list in BGIO's main
 * context
 */
BERR_Code BGIO_P_AddPinToList(
	BGIO_Handle           hGpio,
	BGIO_Pin_Handle       hPin )
{

	BDBG_OBJECT_ASSERT(hGpio, BGIO);
	BDBG_OBJECT_ASSERT(hPin, BGIO_PIN);

	/* add to the head */
	BLST_D_INSERT_HEAD(&hGpio->PinHead , hPin, Link);
	return BERR_SUCCESS;
}

/***************************************************************************
 * To be called to remove a pin handle from the pin list in BGIO's main
 * context
 */
BERR_Code BGIO_P_RemovePinFromList(
	BGIO_Handle           hGpio,
	BGIO_Pin_Handle       hPin )
{
	BDBG_OBJECT_ASSERT(hGpio, BGIO);
	BDBG_OBJECT_ASSERT(hPin, BGIO_PIN);
	BDBG_ASSERT( BGIO_P_GetPinHandle(hGpio, hPin->ePinId) );

	BLST_D_REMOVE(&hGpio->PinHead, hPin, Link);
	return BERR_SUCCESS;
}

/*--------------------------------------------------------------------------
 * To be called to get the pin handle for a PinId from the pin list in
 * BGIO's main context. NULL returned if it does not exist.
 */
BGIO_Pin_Handle BGIO_P_GetPinHandle(
	BGIO_Handle           hGpio,
	BGIO_PinId            ePinId )
{
	BGIO_P_Pin_Context *  pPin;

	BDBG_OBJECT_ASSERT(hGpio, BGIO);
	BDBG_ASSERT( BGIO_PinId_eInvalid > ePinId );

	/* check whether the pin is already being in use */
	pPin = BLST_D_FIRST(&hGpio->PinHead);
	while ( NULL != pPin )
	{
		if ( pPin->ePinId == ePinId )
		{
			return pPin;
		}
		pPin = BLST_D_NEXT(pPin, Link);
	}

	/* not found */
	return NULL;
}

/***************************************************************************
 * To be called to get the register handle
 */
BREG_Handle BGIO_P_GetRegisterHandle(
	BGIO_Handle           hGpio )
{

	BDBG_OBJECT_ASSERT(hGpio, BGIO);
	return hGpio->hRegister;
}

/* End of File */

