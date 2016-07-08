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

#include "bgio_pin_priv.h"
#include "bgio_priv.h"
#include "berr.h"
#include "bkni.h"
#include "bchp_gio.h"
#include "bchp_sun_top_ctrl.h"
#include "bchp_common.h"
#ifdef BCHP_AON_PIN_CTRL_REG_START
#include "bchp_aon_pin_ctrl.h"
#endif
#include "bkni.h"

BDBG_MODULE(BGIO);
BDBG_OBJECT_ID(BGIO_PIN);

/***************************************************************************
 *
 * Utility functions
 *
 ***************************************************************************/
static BERR_Code BGIO_P_Pin_ClearOpenDrainSet(
    BGIO_Handle           hGpio,
    BGIO_PinId            ePinId )
{
    BERR_Code eResult = BERR_SUCCESS;
    uint32_t  ulRegOffset, ulBitOffset;
    uint32_t  ulRegIndex = 0;

    BDBG_OBJECT_ASSERT(hGpio, BGIO);
    BDBG_ASSERT( BGIO_PinId_eInvalid > ePinId );

    /* read the HW register and modify it for this setting */
    eResult = BGIO_P_CalcPinRegAndBit( ePinId, BCHP_GIO_DATA_LO,
                                       &ulRegOffset, &ulBitOffset );
    BDBG_ASSERT( BERR_SUCCESS == eResult );

    if (BERR_SUCCESS == eResult )
    {
        ulRegIndex = ulRegOffset / 4;
        hGpio->aulOpenDrainSet[ulRegIndex] &= (~ BGIO_P_BIT_MASK(ulBitOffset));
    }

    return eResult;
}


/***************************************************************************
 *
 * API support functions
 *
 ***************************************************************************/


#define BGIO_P_PIN_MUX_SEL_GPIO     0
/***************************************************************************
 *
 */
BERR_Code BGIO_P_Pin_Create(
    BGIO_Handle           hGpio,
    BGIO_PinId            ePinId,
    BGIO_Pin_Handle *     phPin )
{
    BERR_Code  eResult = BERR_SUCCESS;
    BGIO_P_Pin_Context *  pPin = NULL;
    const BGIO_P_PinMux *  pPinMux;
    BREG_Handle  hRegister;
    uint32_t ulRegValue;

    if ( NULL != phPin )
        *phPin = NULL;

    BDBG_OBJECT_ASSERT(hGpio, BGIO);
    pPinMux = BGIO_P_GetPinMux(ePinId);
    if ((NULL == phPin) ||
        (BGIO_PinId_eInvalid <= ePinId) ||
        (pPinMux->ulReg == BGIO_P_NULL_REG) )
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* GPIO pin can not share among two apps */
    if ( NULL != BGIO_P_GetPinHandle(hGpio, ePinId) )
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* allocate pin gio sub-module's context */
    pPin = (BGIO_P_Pin_Context *)BKNI_Malloc( sizeof(BGIO_P_Pin_Context) );
    if ( NULL == pPin )
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
    BKNI_Memset((void*)pPin, 0x0, sizeof(BGIO_P_Pin_Context));
    BDBG_OBJECT_SET(pPin, BGIO_PIN);

    if(pPinMux->ulReg != BGIO_P_GIO_REG)
    {
        /* set pin mux to make the pin work as GPIO pin */
        hRegister = BGIO_P_GetRegisterHandle( hGpio );
        BKNI_EnterCriticalSection();
        ulRegValue = BREG_Read32(hRegister, pPinMux->ulReg) &
            ~ pPinMux->ulBitMask;
        ulRegValue |= pPinMux->ulValue;
        BREG_Write32( hRegister, pPinMux->ulReg, ulRegValue );
        BKNI_LeaveCriticalSection();
    }

    /* init pin sub-module's main context */
    BGIO_P_PIN_SET_BLACK_MAGIC( pPin );
    pPin->hGpio = hGpio;
    pPin->ePinId = ePinId;

    eResult = BGIO_P_Pin_SetType( pPin, BGIO_PinType_eInput, false );
    BDBG_ASSERT( BERR_SUCCESS == eResult );
    eResult = BGIO_P_Pin_SetIntrMode( pPin, BGIO_IntrMode_eDisabled, false );
    BDBG_ASSERT( BERR_SUCCESS == eResult );
    eResult = BGIO_P_Pin_ClearIntrStatus( pPin, false );
    BDBG_ASSERT( BERR_SUCCESS == eResult );

    /* connect pin gio sub-module to gio module's main context */
    eResult = BGIO_P_AddPinToList( hGpio, pPin );

    *phPin = pPin;
    return eResult;
}

/***************************************************************************
 *
 */
BERR_Code BGIO_P_Pin_Destroy(
    BGIO_Pin_Handle       hPin )
{
    BERR_Code  eResult = BERR_SUCCESS;
    BDBG_OBJECT_ASSERT(hPin, BGIO_PIN);

    /* block the pin's driving and interrupt,
     * important for other pins' Pin_SetValue later */
    eResult = BGIO_P_Pin_SetType( hPin, BGIO_PinType_eInput, false );
    BDBG_ASSERT( BERR_SUCCESS == eResult );
    eResult = BGIO_P_Pin_SetIntrMode( hPin, BGIO_IntrMode_eDisabled, false );
    BDBG_ASSERT( BERR_SUCCESS == eResult );
    eResult = BGIO_P_Pin_ClearOpenDrainSet(hPin->hGpio, hPin->ePinId);
    BDBG_ASSERT( BERR_SUCCESS == eResult );

    /* remove pin handle from the pin list in gio module's main context */
    eResult = BGIO_P_RemovePinFromList( hPin->hGpio, hPin );
    BDBG_ASSERT( BERR_SUCCESS == eResult );

    BDBG_OBJECT_DESTROY(hPin, BGIO_PIN);
    BKNI_Free((void*)hPin);
    return eResult;
}

/***************************************************************************
 *
 */
#define  BGIO_P_PUSH_PULL           BGIO_PinValue_e0
#define  BGIO_P_OPEN_DRAIN          BGIO_PinValue_e1
#define  BGIO_P_OUTPUT              BGIO_PinValue_e0
#define  BGIO_P_INPUT_ONLY          BGIO_PinValue_e1

BERR_Code BGIO_P_Pin_SetType(
    BGIO_Pin_Handle       hPin,
    BGIO_PinType          ePinType,
    bool                  bInIsr )
{
    BERR_Code  eResult = BERR_SUCCESS;
    BGIO_PinValue  eValIoDir, eValOdEn;

    BDBG_OBJECT_ASSERT(hPin, BGIO_PIN);
    if (BGIO_PinType_eInvalid <= ePinType)
    {
        eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
        return eResult;
    }

    /* calc reg values */
    switch ( ePinType )
    {
    case BGIO_PinType_eInput:
        eValIoDir = BGIO_P_INPUT_ONLY;
        eValOdEn = BGIO_P_PUSH_PULL;
        break;
    case BGIO_PinType_ePushPull:
        /* special GPIO pins can not work as push-pull type */
        if((BGIO_PinId_eSgpio00 <= hPin->ePinId && hPin->ePinId < BGIO_PinId_eAgpio00) ||
           (BGIO_PinId_eAsgpio00 <= hPin->ePinId))
        {
            eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
            return eResult;
        }
        eValIoDir = BGIO_P_OUTPUT;
        eValOdEn = BGIO_P_PUSH_PULL;
        break;
    case BGIO_PinType_eOpenDrain:
        eValIoDir = BGIO_P_OUTPUT;
        eValOdEn = BGIO_P_OPEN_DRAIN;
        break;
    default:
        eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
        return eResult;
    }
    hPin->ePinType = ePinType;

    /* modify hGpio's register value records for this pin and write HW reg */
    eResult = BGIO_P_WritePinRegBit( hPin->hGpio, hPin->ePinId, hPin->ePinType,
                                     BCHP_GIO_IODIR_LO, eValIoDir, bInIsr );
    BDBG_ASSERT( BERR_SUCCESS == eResult );
    eResult = BGIO_P_WritePinRegBit( hPin->hGpio, hPin->ePinId, hPin->ePinType,
                                     BCHP_GIO_ODEN_LO, eValOdEn, bInIsr );
    BDBG_ASSERT( BERR_SUCCESS == eResult );

    /* TODO: init value for push-pull XXX ??? */
    if ( BGIO_PinType_eOpenDrain == ePinType )
        eResult = BGIO_P_Pin_SetValue( hPin, BGIO_PinValue_e1, bInIsr ); /* release */
    else
        eResult = BGIO_P_Pin_ClearOpenDrainSet(hPin->hGpio, hPin->ePinId);
    BDBG_ASSERT( BERR_SUCCESS == eResult );
    return eResult;
}

/***************************************************************************
 *
 */
BERR_Code BGIO_P_Pin_GetValue(
    BGIO_Pin_Handle       hPin,
    BGIO_PinValue *       pePinValue )
{
    BERR_Code  eResult = BERR_SUCCESS;
    BGIO_PinValue         ePinValue = BGIO_PinValue_eInvalid;

    BDBG_OBJECT_ASSERT(hPin, BGIO_PIN);
    if (NULL == pePinValue)
    {
        eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
        return eResult;
    }

    /* read the pin value from HW reg */
    eResult = BGIO_P_ReadPinRegBit( hPin->hGpio, hPin->ePinId, BCHP_GIO_DATA_LO,
                                    &ePinValue );
    BDBG_ASSERT( BERR_SUCCESS == eResult );
    *pePinValue = ePinValue;
    return eResult;
}

/***************************************************************************
 *
 */
BERR_Code BGIO_P_Pin_SetValue(
    BGIO_Pin_Handle       hPin,
    BGIO_PinValue         ePinValue,
    bool                  bInIsr )
{
    BERR_Code  eResult = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hPin, BGIO_PIN);
    if (BGIO_PinValue_eInvalid <= ePinValue)
    {
        eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
        return eResult;
    }

    /* modify hGpio's register value records for this pin and write HW reg */
    eResult = BGIO_P_WritePinRegBit( hPin->hGpio, hPin->ePinId, hPin->ePinType,
                                     BCHP_GIO_DATA_LO, ePinValue, bInIsr );
    BDBG_ASSERT( BERR_SUCCESS == eResult );
    return eResult;
}

/***************************************************************************
 *
 */
#define  BGIO_P_INTR_RESET          BGIO_PinValue_e0  /* reset to work normal */
#define  BGIO_P_INTR_CLEAR          BGIO_PinValue_e1  /* clear and block */
#define  BGIO_P_INTR_DISABLE        BGIO_PinValue_e0
#define  BGIO_P_INTR_ENABLE         BGIO_PinValue_e1
#define  BGIO_P_INTR_FALL_EDGE      BGIO_PinValue_e0
#define  BGIO_P_INTR_RISE_EDGE      BGIO_PinValue_e1
#define  BGIO_P_INTR_ONE_EDGE       BGIO_PinValue_e0
#define  BGIO_P_INTR_BOTH_EDGE      BGIO_PinValue_e1
#if (BCHP_CHIP != 7038) && (BCHP_CHIP != 7438)
#define  BGIO_P_INTR_EDGE           BGIO_PinValue_e0
#define  BGIO_P_INTR_LEVEL          BGIO_PinValue_e1
#define  BGIO_P_INTR_0              BGIO_PinValue_e0
#define  BGIO_P_INTR_1              BGIO_PinValue_e1
#endif

BERR_Code BGIO_P_Pin_SetIntrMode(
    BGIO_Pin_Handle       hPin,
    BGIO_IntrMode         eIntrMode,
    bool                  bInIsr )
{
    BERR_Code  eResult = BERR_SUCCESS;
    BGIO_PinValue  eIntrEna, eEdgeSel, eEdgeInsen;
#if (BCHP_CHIP != 7038) && (BCHP_CHIP != 7438)
    BGIO_PinValue  eLvlEdge;
#endif

    BDBG_OBJECT_ASSERT(hPin, BGIO_PIN);
    if (BGIO_IntrMode_eInvalid <= eIntrMode)
    {
        eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
        return eResult;
    }

    /* calc reg values */
    eEdgeInsen = BGIO_P_INTR_ONE_EDGE;
#if (BCHP_CHIP != 7038) && (BCHP_CHIP != 7438)
    eLvlEdge = BGIO_P_INTR_EDGE;
#endif
    switch ( eIntrMode )
    {
    case BGIO_IntrMode_eDisabled:
        eIntrEna = BGIO_P_INTR_DISABLE;
        eEdgeSel = BGIO_P_INTR_FALL_EDGE;
        break;
    case BGIO_IntrMode_e0To1:
        eIntrEna = BGIO_P_INTR_ENABLE;
        eEdgeSel = BGIO_P_INTR_RISE_EDGE;
        break;
    case BGIO_IntrMode_e1To0:
        eIntrEna = BGIO_P_INTR_ENABLE;
        eEdgeSel = BGIO_P_INTR_FALL_EDGE;
        break;
    case BGIO_IntrMode_e0To1_Or_1To0:
        eIntrEna = BGIO_P_INTR_ENABLE;
        eEdgeSel = BGIO_P_INTR_FALL_EDGE;
        eEdgeInsen = BGIO_P_INTR_BOTH_EDGE;
        break;
#if (BCHP_CHIP != 7038) && (BCHP_CHIP != 7438)
    case BGIO_IntrMode_e0:
        eIntrEna = BGIO_P_INTR_ENABLE;
        eLvlEdge = BGIO_P_INTR_LEVEL;
        eEdgeSel = BGIO_P_INTR_0;
        break;
    case BGIO_IntrMode_e1:
        eIntrEna = BGIO_P_INTR_ENABLE;
        eLvlEdge = BGIO_P_INTR_LEVEL;
        eEdgeSel = BGIO_P_INTR_1;
        break;
#endif
    default:
        eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
        return eResult;
    }
    hPin->eIntrMode = eIntrMode;

    /* modify hGpio's register value records for this pin and write HW reg */
#if (BCHP_CHIP == 7038) || (BCHP_CHIP==7438)
    /* Both MASK and RESET in RST are needed to really enable interrupt */
    eResult = BGIO_P_WritePinRegBit( hPin->hGpio, hPin->ePinId, hPin->ePinType,
                                     BCHP_GIO_RST_LO, BGIO_P_INTR_CLEAR, bInIsr );
    BDBG_ASSERT( BERR_SUCCESS == eResult ); /* clear and block for 7038 */
    if ( BGIO_IntrMode_eDisabled != eIntrMode )
    {
        eResult = BGIO_P_WritePinRegBit( hPin->hGpio, hPin->ePinId, hPin->ePinType,
                                         BCHP_GIO_RST_LO, BGIO_P_INTR_RESET, bInIsr );
        BDBG_ASSERT( BERR_SUCCESS == eResult ); /* reset to work normal */
    }
#else
    eResult = BGIO_P_WritePinRegBit( hPin->hGpio, hPin->ePinId, hPin->ePinType,
                                     BCHP_GIO_STAT_LO, BGIO_P_INTR_CLEAR, bInIsr );
    BDBG_ASSERT( BERR_SUCCESS == eResult ); /* clear and work normal */
    eResult = BGIO_P_WritePinRegBit( hPin->hGpio, hPin->ePinId, hPin->ePinType,
                                     BCHP_GIO_LEVEL_LO, eLvlEdge, bInIsr );
    BDBG_ASSERT( BERR_SUCCESS == eResult );
#endif
    eResult = BGIO_P_WritePinRegBit( hPin->hGpio, hPin->ePinId, hPin->ePinType,
                                     BCHP_GIO_EC_LO, eEdgeSel, bInIsr );
    BDBG_ASSERT( BERR_SUCCESS == eResult );
    eResult = BGIO_P_WritePinRegBit( hPin->hGpio, hPin->ePinId, hPin->ePinType,
                                     BCHP_GIO_EI_LO, eEdgeInsen, bInIsr );
    BDBG_ASSERT( BERR_SUCCESS == eResult );
    eResult = BGIO_P_WritePinRegBit( hPin->hGpio, hPin->ePinId, hPin->ePinType,
                                     BCHP_GIO_MASK_LO, eIntrEna, bInIsr );
    BDBG_ASSERT( BERR_SUCCESS == eResult );

    return eResult;
}

/***************************************************************************
 *
 */
BERR_Code BGIO_P_Pin_ClearIntrStatus(
    BGIO_Pin_Handle       hPin,
    bool                  bInIsr )
{
    BERR_Code  eResult = BERR_SUCCESS;
    BDBG_OBJECT_ASSERT(hPin, BGIO_PIN);

#if (BCHP_CHIP == 7038) || (BCHP_CHIP==7438)
    /* modify hGpio's register value records for this pin and write HW reg
     * needs to write a "1" and a "0" to the clear bit to clear the
     * interrupt status and to set ready for new interrupt for the pin */
    eResult = BGIO_P_WritePinRegBit( hPin->hGpio, hPin->ePinId, hPin->ePinType,
                                     BCHP_GIO_RST_LO, BGIO_PinValue_e1, bInIsr );
    BDBG_ASSERT( BERR_SUCCESS == eResult );
    eResult = BGIO_P_WritePinRegBit( hPin->hGpio, hPin->ePinId, hPin->ePinType,
                                     BCHP_GIO_RST_LO, BGIO_PinValue_e0, bInIsr );
    BDBG_ASSERT( BERR_SUCCESS == eResult );
#else
    if ( (BGIO_IntrMode_e0 != hPin->eIntrMode) &&
         (BGIO_IntrMode_e1 != hPin->eIntrMode) )
    {
        eResult = BGIO_P_WritePinRegBit( hPin->hGpio, hPin->ePinId, hPin->ePinType,
                                         BCHP_GIO_STAT_LO, BGIO_PinValue_e1, bInIsr );
    }
    else
    {
        BDBG_ERR(("Level trigered intr must be cleared by changing pin value"));
        eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
    }
#endif
    return eResult;
}

/***************************************************************************
 *
 */
BERR_Code BGIO_P_Pin_GetIntrStatus(
    BGIO_Pin_Handle       hPin,
    bool *                pbFire )
{
    BERR_Code  eResult = BERR_SUCCESS;
    BGIO_PinValue  ePinValue = BGIO_PinValue_eInvalid;

    BDBG_OBJECT_ASSERT(hPin, BGIO_PIN);
    if (NULL == pbFire)
    {
        eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
        return eResult;
    }

    /* read the pin value from HW reg */
    eResult = BGIO_P_ReadPinRegBit( hPin->hGpio, hPin->ePinId,
                                    BCHP_GIO_STAT_LO, &ePinValue );
    BDBG_ASSERT( BERR_SUCCESS == eResult );

    *pbFire = (BGIO_PinValue_e1 == ePinValue);
    return eResult;
}


/* End of File */
