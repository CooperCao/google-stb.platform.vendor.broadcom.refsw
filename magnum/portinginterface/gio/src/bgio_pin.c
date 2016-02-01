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
 *
 * Revision History:
 * $brcm_Log: $
 * 
 ***************************************************************************/
#include "bgio_priv.h"
#include "bgio_pin_priv.h"
#include "berr.h"

BDBG_MODULE(BGIO);

/***************************************************************************
 *
 * API support functions
 *
 ***************************************************************************/


/***************************************************************************
 *
 */
BERR_Code BGIO_Pin_Create(
	BGIO_Handle           hGpio,
	BGIO_PinId            ePinId,
	BGIO_Pin_Handle *     phPin )
{
	BERR_Code  eResult = BERR_SUCCESS;
	BDBG_OBJECT_ASSERT(hGpio, BGIO);

	/* input para validation done in BGIO_P_Pin_Create */
	eResult = BGIO_P_Pin_Create( hGpio, ePinId, phPin );
	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */
BERR_Code BGIO_Pin_Destroy(
	BGIO_Pin_Handle       hPin )
{
	BERR_Code  eResult = BERR_SUCCESS;
	BDBG_OBJECT_ASSERT(hPin, BGIO_PIN);

	/* input para validation done in BGIO_P_Pin_Destroy */
	eResult = BGIO_P_Pin_Destroy( hPin );
	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */
BERR_Code BGIO_Pin_GetId(
	BGIO_Pin_Handle       hPin,
	BGIO_PinId *          pePinId )
{
	BDBG_OBJECT_ASSERT(hPin, BGIO_PIN);
	if (NULL == pePinId)
	{
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	BDBG_ASSERT( BGIO_PinId_eInvalid > hPin->ePinId );
	*pePinId = hPin->ePinId;
	return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BGIO_Pin_GetType(
	BGIO_Pin_Handle       hPin,
	BGIO_PinType *        pePinType )
{
	BDBG_OBJECT_ASSERT(hPin, BGIO_PIN);
	if (NULL == pePinType)
	{
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	BDBG_ASSERT( BGIO_PinType_eInvalid > hPin->ePinType );
	*pePinType = hPin->ePinType;
	return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BGIO_Pin_SetType(
	BGIO_Pin_Handle       hPin,
	BGIO_PinType          ePinType )
{
	BERR_Code  eResult = BERR_SUCCESS;
	BDBG_OBJECT_ASSERT(hPin, BGIO_PIN);

	/* input para validation done in BGIO_P_Pin_SetType */
	eResult = BGIO_P_Pin_SetType( hPin, ePinType, false );
	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */
BERR_Code BGIO_Pin_SetType_isr(
	BGIO_Pin_Handle       hPin,
	BGIO_PinType          ePinType )
{
	BERR_Code  eResult = BERR_SUCCESS;
	BDBG_OBJECT_ASSERT(hPin, BGIO_PIN);

	/* input para validation done in BGIO_P_Pin_SetType */
	eResult = BGIO_P_Pin_SetType( hPin, ePinType, true );
	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */
BERR_Code BGIO_Pin_GetValue(
	BGIO_Pin_Handle       hPin,
	BGIO_PinValue *       pePinValue )
{
	BERR_Code  eResult = BERR_SUCCESS;
	BDBG_OBJECT_ASSERT(hPin, BGIO_PIN);

	/* input para validation done in BGIO_P_Pin_GetValue */
	eResult = BGIO_P_Pin_GetValue( hPin, pePinValue );
	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */
BERR_Code BGIO_Pin_PushPull_SetValue(
	BGIO_Pin_Handle       hPin,
	BGIO_PinValue         ePinValue )
{
	BERR_Code  eResult = BERR_SUCCESS;
	BDBG_OBJECT_ASSERT(hPin, BGIO_PIN);

	if (BGIO_PinType_ePushPull != hPin->ePinType)
	{
		eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
		return eResult;
	}

	/* more input para validation done in BGIO_P_Pin_SetType */
	eResult = BGIO_P_Pin_SetValue( hPin, ePinValue, false );
	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */
BERR_Code BGIO_Pin_PushPull_SetValue_isr(
	BGIO_Pin_Handle       hPin,
	BGIO_PinValue         ePinValue )
{
	BERR_Code  eResult = BERR_SUCCESS;
	BDBG_OBJECT_ASSERT(hPin, BGIO_PIN);

	if (BGIO_PinType_ePushPull != hPin->ePinType)
	{
		eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
		return eResult;
	}

	/* more input para validation done in BGIO_P_Pin_SetType */
	eResult = BGIO_P_Pin_SetValue( hPin, ePinValue, true );
	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */
BERR_Code BGIO_Pin_OpenDrain_PullDown(
	BGIO_Pin_Handle       hPin )
{
	BERR_Code  eResult = BERR_SUCCESS;
	BDBG_OBJECT_ASSERT(hPin, BGIO_PIN);

	if (BGIO_PinType_eOpenDrain != hPin->ePinType)
	{
		eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
		return eResult;
	}

	/* more input para validation done in BGIO_P_Pin_SetType */
	eResult = BGIO_P_Pin_SetValue( hPin, BGIO_PinValue_e0, false );
	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */
BERR_Code BGIO_Pin_OpenDrain_PullDown_isr(
	BGIO_Pin_Handle       hPin )
{
	BERR_Code  eResult = BERR_SUCCESS;
	BDBG_OBJECT_ASSERT(hPin, BGIO_PIN);

	if (BGIO_PinType_eOpenDrain != hPin->ePinType)
	{
		eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
		return eResult;
	}

	/* more input para validation done in BGIO_P_Pin_SetType */
	eResult = BGIO_P_Pin_SetValue( hPin, BGIO_PinValue_e0, true );
	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */
BERR_Code BGIO_Pin_OpenDrain_Release(
	BGIO_Pin_Handle       hPin )
{
	BERR_Code  eResult = BERR_SUCCESS;
	BDBG_OBJECT_ASSERT(hPin, BGIO_PIN);

	if (BGIO_PinType_eOpenDrain != hPin->ePinType)
	{
		eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
		return eResult;
	}

	/* more input para validation done in BGIO_P_Pin_SetType */
	eResult = BGIO_P_Pin_SetValue( hPin, BGIO_PinValue_e1, false );
	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */
BERR_Code BGIO_Pin_OpenDrain_Release_isr(
	BGIO_Pin_Handle       hPin )
{
	BERR_Code  eResult = BERR_SUCCESS;
	BDBG_OBJECT_ASSERT(hPin, BGIO_PIN);

	if (BGIO_PinType_eOpenDrain != hPin->ePinType)
	{
		eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
		return eResult;
	}

	/* more input para validation done in BGIO_P_Pin_SetType */
	eResult = BGIO_P_Pin_SetValue( hPin, BGIO_PinValue_e1, true );
	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */
BERR_Code BGIO_Pin_SetIntrMode(
	BGIO_Pin_Handle       hPin,
	BGIO_IntrMode         eIntrMode )
{
	BERR_Code  eResult = BERR_SUCCESS;
	BDBG_OBJECT_ASSERT(hPin, BGIO_PIN);

	/* input para validation done in BGIO_P_Pin_SetIntrMode */
	eResult = BGIO_P_Pin_SetIntrMode( hPin, eIntrMode, false );
	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */
BERR_Code BGIO_Pin_SetIntrMode_isr(
	BGIO_Pin_Handle       hPin,
	BGIO_IntrMode         eIntrMode )
{
	BERR_Code  eResult = BERR_SUCCESS;
	BDBG_OBJECT_ASSERT(hPin, BGIO_PIN);

	/* input para validation done in BGIO_P_Pin_SetIntrMode */
	eResult = BGIO_P_Pin_SetIntrMode( hPin, eIntrMode, true );
	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */
BERR_Code BGIO_Pin_ClearIntrStatus(
	BGIO_Pin_Handle       hPin )
{
	BERR_Code  eResult = BERR_SUCCESS;
	BDBG_OBJECT_ASSERT(hPin, BGIO_PIN);

	/* input para validation done in BGIO_P_Pin_ClearIntrStatus */
	eResult = BGIO_P_Pin_ClearIntrStatus( hPin, false );
	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */
BERR_Code BGIO_Pin_ClearIntrStatus_isr(
	BGIO_Pin_Handle       hPin )
{
	BERR_Code  eResult = BERR_SUCCESS;
	BDBG_OBJECT_ASSERT(hPin, BGIO_PIN);

	/* input para validation done in BGIO_P_Pin_ClearIntrStatus */
	eResult = BGIO_P_Pin_ClearIntrStatus( hPin, true );
	return BERR_TRACE(eResult);
}

/***************************************************************************
 *
 */
BERR_Code BGIO_Pin_GetIntrStatus(
	BGIO_Pin_Handle       hPin,
	bool *                pbFire )
{
	BERR_Code  eResult = BERR_SUCCESS;
	BDBG_OBJECT_ASSERT(hPin, BGIO_PIN);

	/* input para validation done in BGIO_P_Pin_GetIntrStatus */
	eResult = BGIO_P_Pin_GetIntrStatus( hPin, pbFire );
	return BERR_TRACE(eResult);
}

/* End of File */

