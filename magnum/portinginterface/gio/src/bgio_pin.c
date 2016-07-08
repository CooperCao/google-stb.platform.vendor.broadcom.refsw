/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
