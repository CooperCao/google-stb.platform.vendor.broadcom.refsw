/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *****************************************************************************/

/*******************************************************************************
 *
 * DESCRIPTION:
 *      MAC-PIB API interface.
 *
*******************************************************************************/

#ifndef _BB_MAC_PIB_API_H
#define _BB_MAC_PIB_API_H


/************************* INCLUDES *****************************************************/
#include "private/bbMacFeTaskTimeDisp.h"    /* MAC-FE Task-Time Dispatcher interface. */
#include "private/bbMacLeTrxModeDisp.h"     /* MAC-LE Transceiver Mode Dispatcher interface. */
#include "private/bbMacMemory.h"            /* MAC Memory interface. */
#include "private/bbPhyPibApi.h"            /* PHY-PIB API interface. */


/************************* INLINES ******************************************************/
#if defined(_HAL_USE_PRNG_)
/*************************************************************************************//**
 * \brief   Returns value of the MAC-PIB attribute macPrngSeed.
 * \param[in]   __givenContextId    Identifier of the specified MAC Context.
 * \return  Value of the requested MAC-PIB attribute.
*****************************************************************************************/
INLINE MAC_ContextEnabled_t macPibApiGetPrngSeed(MAC_WITHIN_GIVEN_CONTEXT)
{
    return MAC_MEMORY_FE_PRNG_DESCR()->seed;
}


/*************************************************************************************//**
 * \brief   Assigns a new value to the MAC-PIB attribute macPrngSeed.
 * \param[in]   __givenContextId    Identifier of the specified MAC Context.
 * \param[in]   newValue            New value of the specified MAC-PIB attribute.
*****************************************************************************************/
INLINE void macPibApiSetPrngSeed(MAC_WITH_GIVEN_CONTEXT(const MAC_PrngSeed_t newValue))
{
    HAL_RandomInit(MAC_MEMORY_FE_PRNG_DESCR(), newValue);
}


/*************************************************************************************//**
 * \brief   Returns value of the MAC-PIB attribute macPrngCounter.
 * \param[in]   __givenContextId    Identifier of the specified MAC Context.
 * \return  Value of the requested MAC-PIB attribute.
*****************************************************************************************/
INLINE MAC_ContextEnabled_t macPibApiGetPrngCounter(MAC_WITHIN_GIVEN_CONTEXT)
{
    return HAL_RandomCounter(MAC_MEMORY_FE_PRNG_DESCR());
}


/*************************************************************************************//**
 * \brief   Assigns a new value to the MAC-PIB attribute macPrngCounter.
 * \param[in]   __givenContextId    Identifier of the specified MAC Context.
 * \param[in]   newValue            New value of the specified MAC-PIB attribute.
*****************************************************************************************/
INLINE void macPibApiSetPrngCounter(MAC_WITH_GIVEN_CONTEXT(const MAC_PrngCounter_t newValue))
{
    HAL_RandomMove(MAC_MEMORY_FE_PRNG_DESCR(), newValue);
}
#endif /* _HAL_USE_PRNG_ */


/*************************************************************************************//**
 * \brief   Returns value of the MAC-PIB attribute macContextEnabled.
 * \param[in]   __givenContextId    Identifier of the specified MAC Context.
 * \return  Value of the requested MAC-PIB attribute.
*****************************************************************************************/
INLINE MAC_ContextEnabled_t macPibApiGetContextEnabled(MAC_WITHIN_GIVEN_CONTEXT)
{
    return MAC_MEMORY_PIB_PERMANENT().macContextEnabled;
}


/*************************************************************************************//**
 * \brief   Assigns a new value to the MAC-PIB attribute macContextEnabled.
 * \param[in]   __givenContextId    Identifier of the specified MAC Context.
 * \param[in]   newValue            New value of the specified MAC-PIB attribute.
*****************************************************************************************/
INLINE void macPibApiSetContextEnabled(MAC_WITH_GIVEN_CONTEXT(const MAC_ContextEnabled_t newValue))
{
    SYS_DbgAssertComplex(MAC_ATTR_ONLYALLOWED_VALUE_CONTEXT_ENABLED == newValue,
            LOG_macPibApiSetContextEnabled_InvalidNewValue);

    ATOMIC_SECTION_ENTER(SYS_ATOMIC_DEFAULT_UID)
        MAC_MEMORY_PIB_PERMANENT().macContextEnabled = newValue;
    ATOMIC_SECTION_LEAVE(SYS_ATOMIC_DEFAULT_UID)
}


/*************************************************************************************//**
 * \brief   Returns value of the MAC-PIB attribute macPanCoordinator.
 * \param[in]   __givenContextId    Identifier of the specified MAC Context.
 * \return  Value of the requested MAC-PIB attribute.
*****************************************************************************************/
INLINE MAC_PanCoordinator_t macPibApiGetPanCoordinator(MAC_WITHIN_GIVEN_CONTEXT)
{
    if (MAC_IS_ZBPRO_CONTEXT(MAC_GIVEN_CONTEXT_ID))
        MAC_CODE_FOR_ZBPRO_CONTEXT( return MAC_MEMORY_PIB_ZBPRO().macPanCoordinator );
    else
        return MAC_ATTR_DEFAULT_VALUE_PAN_COORDINATOR;
}


#if defined(_MAC_CONTEXT_ZBPRO_)
/*************************************************************************************//**
 * \brief   Assigns a new value to the ZigBee PRO MAC-PIB attribute macPanCoordinator.
 * \param[in]   newValue    New value of the specified MAC-PIB attribute.
*****************************************************************************************/
INLINE void macPibApiSetPanCoordinatorZBPRO(const MAC_PanCoordinator_t newValue)
{
    ATOMIC_SECTION_ENTER(SYS_ATOMIC_DEFAULT_UID)
        MAC_MEMORY_PIB_ZBPRO().macPanCoordinator = newValue;
# if defined(_MAC_DUAL_CONTEXT_)
        HAL_RadioFrameFilterSetPanCoord(MAC_ZBPRO_CONTEXT_ID, newValue);
# else
        HAL_RadioFrameFilterSetPanCoord(newValue);
# endif
    ATOMIC_SECTION_LEAVE(SYS_ATOMIC_DEFAULT_UID)
}
#endif /* _MAC_CONTEXT_ZBPRO_ */


/*************************************************************************************//**
 * \brief   Returns value of the MAC-PIB attribute macExtendedAddress.
 * \param[in]   __givenContextId    Identifier of the specified MAC Context.
 * \return  Value of the requested MAC-PIB attribute.
*****************************************************************************************/
INLINE MAC_ExtendedAddress_t macPibApiGetExtendedAddress(MAC_WITHIN_GIVEN_CONTEXT)
{
    return MAC_MEMORY_PIB_PERMANENT().macExtendedAddress;
}


/*************************************************************************************//**
 * \brief   Assigns a new value to the MAC-PIB attribute macExtendedAddress.
 * \param[in]   __givenContextId    Identifier of the specified MAC Context.
 * \param[in]   newValue            New value of the specified MAC-PIB attribute.
*****************************************************************************************/
INLINE void macPibApiSetExtendedAddress(MAC_WITH_GIVEN_CONTEXT(const MAC_ExtendedAddress_t newValue))
{
    ATOMIC_SECTION_ENTER(SYS_ATOMIC_DEFAULT_UID)
        MAC_MEMORY_PIB_PERMANENT().macExtendedAddress = newValue;
#if defined(_MAC_DUAL_CONTEXT_)
        HAL_RadioFrameFilterSetExtAddr(MAC_GIVEN_CONTEXT_ID, newValue);
#else
        HAL_RadioFrameFilterSetExtAddr(newValue);
#endif
    ATOMIC_SECTION_LEAVE(SYS_ATOMIC_DEFAULT_UID)

#if defined(_HAL_USE_PRNG_)
    /* Initialize PRNG if it was not initialized directly yet. */
    if (0 == MAC_MEMORY_FE_PRNG_DESCR()->seed)
    {
        HAL_PrngSeed_t  seed =      /* Use MAC Extended Address as a source for the PRNG Seed. */
                (((uint32_t)(newValue >> 32)) ^ (uint32_t)newValue);
        if (0 == seed)
            seed = 1;               /* Avoid using zero as a seed. */
        HAL_RandomInit(MAC_MEMORY_FE_PRNG_DESCR(), seed);
    }
#endif /* _HAL_USE_PRNG_ */

#if !defined(_MAC_ZERO_SEQNUM_ON_RESET_)
    /* Initialize macDSN and macBSN with random values. */
    MAC_MEMORY_PIB_ESSENTIAL().macDSN = HAL_Random(MAC_MEMORY_FE_PRNG_DESCR());
# if !defined(_MAC_CONTEXT_RF4CE_CONTROLLER_)
    MAC_MEMORY_PIB_BEACONING().macBSN = HAL_Random(MAC_MEMORY_FE_PRNG_DESCR());
# endif
#endif /* ! _MAC_ZERO_SEQNUM_ON_RESET_ */
}


/*************************************************************************************//**
 * \brief   Returns value of the MAC-PIB attribute macAckWaitDuration.
 * \return  Value of the requested MAC-PIB attribute.
*****************************************************************************************/
INLINE MAC_AckWaitDuration_t macPibApiGetAckWaitDuration(void)
{
    MAC_AckWaitDuration_t     macAckWaitDuration;           /* The value to be returned. */
    PHY_ShrDuration_t         phyShrDuration;               /* Value of the PHY-PIB attribute phyShrDuration. */
    PHY_SymbolsPerOctetX10_t  phySymbolsPerOctetX10;        /* Value of the PHY-PIB attribute phySymbolsPerOctet
                                                                multiplied by 10. */
    ATOMIC_SECTION_ENTER(SYS_ATOMIC_DEFAULT_UID)
        phyShrDuration = phyPibApiGetShrDuration();
        phySymbolsPerOctetX10 = phyPibApiGetSymbolsPerOctetX10();
    ATOMIC_SECTION_LEAVE(SYS_ATOMIC_DEFAULT_UID)

    /* The following formula is implemented:
     *  macAckWaitDuration = aUnitBackoffPeriod + aTurnaroundTime +
     *                       phySHRDuration + RoundUp(6 * phySymbolPerOctet).
     *
     * It gives the following values for different bands and modulations:
     *   868 BPSK       120 symbols = 20 + 12 + 40 + RoundUp(6 * 8)         = 6000 us
     *   915 BPSK       120 symbols = 20 + 12 + 40 + RoundUp(6 * 8)         = 3000 us
     *   868 ASK         38 symbols = 20 + 12 +  3 + RoundUp(6 * 0.4)       = 3040 us
     *   915 ASK         49 symbols = 20 + 12 +  7 + RoundUp(6 * 1.6)       =  980 us
     *   868 O-QPSK      54 symbols = 20 + 12 + 10 + RoundUp(6 * 2)         = 2160 us
     *   915 O-QPSK      54 symbols = 20 + 12 + 10 + RoundUp(6 * 2)         =  864 us
     *  2450 O-QPSK      54 symbols = 20 + 12 + 10 + RoundUp(6 * 2)         =  864 us
     */
    macAckWaitDuration = MAC_aUnitBackoffPeriod + PHY_aTurnaroundTime +
            phyShrDuration + CEIL(6 * phySymbolsPerOctetX10, 10);

    return macAckWaitDuration;
}


/*************************************************************************************//**
 * \brief   Returns value of the MAC-PIB attribute macAssociationPermit.
 * \param[in]   __givenContextId    Identifier of the specified MAC Context.
 * \return  Value of the requested MAC-PIB attribute.
*****************************************************************************************/
INLINE MAC_AssociationPermit_t macPibApiGetAssociationPermit(MAC_WITHIN_GIVEN_CONTEXT)
{
    if (MAC_IS_ZBPRO_CONTEXT(MAC_GIVEN_CONTEXT_ID))
        MAC_CODE_FOR_ZBPRO_CONTEXT( return MAC_MEMORY_PIB_ZBPRO().macAssociationPermit );
    else
        return MAC_ATTR_DEFAULT_VALUE_ASSOCIATION_PERMIT;
}


#if defined(_MAC_CONTEXT_ZBPRO_)
/*************************************************************************************//**
 * \brief   Assigns a new value to the ZigBee PRO MAC-PIB attribute macAssociationPermit.
 * \param[in]   newValue    New value of the specified MAC-PIB attribute.
*****************************************************************************************/
INLINE void macPibApiSetAssociationPermitZBPRO(const MAC_AssociationPermit_t newValue)
{
    MAC_MEMORY_PIB_ZBPRO().macAssociationPermit = newValue;
}
#endif /* _MAC_CONTEXT_ZBPRO_ */


/*************************************************************************************//**
 * \brief   Returns value of the MAC-PIB attribute macAutoRequest.
 * \param[in]   __givenContextId    Identifier of the specified MAC Context.
 * \return  Value of the requested MAC-PIB attribute.
*****************************************************************************************/
INLINE MAC_AutoRequest_t macPibApiGetAutoRequest(MAC_WITHIN_GIVEN_CONTEXT)
{
#if !defined(_MAC_CONTEXT_RF4CE_CONTROLLER_)
    return MAC_MEMORY_PIB_BEACONING().macAutoRequest;
#else
    return MAC_ATTR_DEFAULT_VALUE_AUTO_REQUEST;
#endif
}


#if !defined(_MAC_CONTEXT_RF4CE_CONTROLLER_)
/*************************************************************************************//**
 * \brief   Assigns a new value to the MAC-PIB attribute macAutoRequest.
 * \param[in]   __givenContextId    Identifier of the specified MAC Context.
 * \param[in]   newValue    New value of the specified MAC-PIB attribute.
*****************************************************************************************/
INLINE void macPibApiSetAutoRequest(MAC_WITH_GIVEN_CONTEXT(const MAC_AutoRequest_t newValue))
{
    MAC_MEMORY_PIB_BEACONING().macAutoRequest = newValue;
}
#endif /* ! _MAC_CONTEXT_RF4CE_CONTROLLER_ */


/*************************************************************************************//**
 * \brief   Returns value of the MAC-PIB attribute macBattLifeExt.
 * \return  Value of the requested MAC-PIB attribute.
*****************************************************************************************/
INLINE MAC_BattLifeExt_t macPibApiGetBattLifeExt(void)
{
    return MAC_ATTR_DEFAULT_VALUE_BATT_LIFE_EXT;
}


/*************************************************************************************//**
 * \brief   Returns value of the MAC-PIB attribute macBattLifeExtPeriods.
 * \return  Value of the requested MAC-PIB attribute.
*****************************************************************************************/
INLINE MAC_BattLifeExtPeriods_t macPibApiGetBattLifeExtPeriods(void)
{
    MAC_BattLifeExtPeriods_t  macBattLifeExtPeriods;        /* The value to be returned. */
    PHY_ShrDuration_t         phyShrDuration;               /* Value of the PHY-PIB attribute phyShrDuration. */

    /* This value is dependent on the supported PHY and is the sum of three terms:
     * Term 1: The value (2^x)-1, where x is the maximum value of macMinBE in BLE mode; it equals to two.
     *         This term thus equals to 3, in back-off periods.
     * Term 2: The duration of the initial contention window length.
     *         This term thus equals to 2, in back-off periods.
     * Term 3: The Preamble field length and the SFD field length of the supported PHY, summed together
     *         (i.e., phySHRDuration, in symbols) and rounded up (if necessary) to an integer number of back-off
     *         periods. The number of symbols forming one back-off period is equal to aUnitBackoffPeriod.
     */
    phyShrDuration = phyPibApiGetShrDuration();

    /* The following formula is implemented:
     *  macBattLifeExtPeriods = 3 + 2 + RoundUp(phySHRDuration / aUnitBackoffPeriod).
     */
    macBattLifeExtPeriods = 3 + 2 + CEIL(phyShrDuration, MAC_aUnitBackoffPeriod);

    return macBattLifeExtPeriods;
}


/*************************************************************************************//**
 * \brief   Returns value of the MAC-PIB attribute macBeaconPayloadLength.
 * \param[in]   __givenContextId    Identifier of the specified MAC Context.
 * \return  Value of the requested MAC-PIB attribute.
*****************************************************************************************/
INLINE MAC_BeaconPayloadLength_t macPibApiGetBeaconPayloadLength(MAC_WITHIN_GIVEN_CONTEXT)
{
#if !defined(_MAC_CONTEXT_RF4CE_CONTROLLER_)
    return MAC_MEMORY_PIB_BEACONING().macBeaconPayloadLength;
#else
    return MAC_ATTR_DEFAULT_VALUE_BEACON_PAYLOAD_LENGTH;
#endif
}


#if !defined(_MAC_CONTEXT_RF4CE_CONTROLLER_)
/*************************************************************************************//**
 * \brief   Assigns a new value to the MAC-PIB attribute macBeaconPayloadLength.
 * \param[in]   __givenContextId    Identifier of the specified MAC Context.
 * \param[in]   newValue            New value of the specified MAC-PIB attribute.
*****************************************************************************************/
INLINE void macPibApiSetBeaconPayloadLength(MAC_WITH_GIVEN_CONTEXT(const MAC_BeaconPayloadLength_t newValue))
{
    SYS_DbgAssertComplex(newValue <= MAC_aMaxBeaconPayloadLength, LOG_macPibApiSetBeaconPayloadLength_InvalidNewValue);
    SYS_DbgAssertComplex(IMP(MAC_IS_ZBPRO_CONTEXT(MAC_GIVEN_CONTEXT_ID),
                             newValue <= MAC_ATTR_MAXALLOWED_VALUE_BEACON_PAYLOAD_LENGTH_ZBPRO),
            LOG_macPibApiSetBeaconPayloadLength_InvalidNewValueZBPRO);
    SYS_DbgAssertComplex(IMP(MAC_IS_RF4CE_CONTEXT(MAC_GIVEN_CONTEXT_ID),
                             newValue <= MAC_ATTR_MAXALLOWED_VALUE_BEACON_PAYLOAD_LENGTH_RF4CE),
            LOG_macPibApiSetBeaconPayloadLength_InvalidNewValueRF4CE);

    MAC_MEMORY_PIB_BEACONING().macBeaconPayloadLength = newValue;
}
#endif /* ! _MAC_CONTEXT_RF4CE_CONTROLLER_ */


/*************************************************************************************//**
 * \brief   Returns value of the MAC-PIB attribute macBeaconPayload.
 * \param[in]   __givenContextId    Identifier of the specified MAC Context.
 * \param[out]  payload             Pointer to the unused payload descriptor to save the
 *  requested beacon payload.
 * \details
 *  The caller shall allocate an unused payload descriptor and specify it to this function
 *  by pointer with \p payload to receive the requested beacon payload. Such a descriptor
 *  may not be emptied by the caller prior to calling this function.
 * \details
 *  The static payload descriptor is returned if beacons are supported (i.e., for cases of
 *  ZigBee PRO and/or RF4CE-Target); the static payload capacity is set to the maximum
 *  allowed beacon payload capacity for the specified MAC Context, and the actual payload
 *  length is set according to the current value of \c macBeaconPayloadLength attribute.
 * \details
 *  The empty payload descriptor is returned if beacons are not supported (i.e., for the
 *  case of RF4CE-Controller).
*****************************************************************************************/
INLINE void macPibApiGetBeaconPayload(MAC_WITH_GIVEN_CONTEXT(MAC_BeaconPayload_t *const payload))
{
#if !defined(_MAC_CONTEXT_RF4CE_CONTROLLER_)

    if (MAC_IS_ZBPRO_CONTEXT(MAC_GIVEN_CONTEXT_ID))
        MAC_CODE_FOR_ZBPRO_CONTEXT( SYS_LinkStaticPayload(payload,
                MAC_MEMORY_PIB_BEACON_PAYLOAD_ZBPRO(),
                MAC_ATTR_MAXALLOWED_VALUE_BEACON_PAYLOAD_LENGTH_ZBPRO) );
    else /* MAC_IS_RF4CE_CONTEXT(MAC_GIVEN_CONTEXT_ID) */
        MAC_CODE_FOR_RF4CE_CONTEXT( SYS_LinkStaticPayload(payload,
                MAC_MEMORY_PIB_BEACON_PAYLOAD_RF4CE(),
                MAC_ATTR_MAXALLOWED_VALUE_BEACON_PAYLOAD_LENGTH_RF4CE) );

    SYS_MemAlloc(payload, macPibApiGetBeaconPayloadLength(MAC_GIVEN_CONTEXT_ID));

#else /* _MAC_CONTEXT_RF4CE_CONTROLLER_ */

    SYS_SetEmptyPayload(payload);

#endif
}


#if !defined(_MAC_CONTEXT_RF4CE_CONTROLLER_)
/*************************************************************************************//**
 * \brief   Assigns a new value to the MAC-PIB attribute macBeaconPayload.
 * \param[in]   __givenContextId    Identifier of the specified MAC Context.
 * \param[in]   newPayload          Pointer to the descriptor of the new beacon payload.
 * \details
 *  The caller specifies the new beacon payload with the descriptor pointed by
 *  \p newPayload argument. The caller shall allocate a dynamic payload or link the
 *  descriptor to a static payload array or structure before calling this function; this
 *  function uses such a payload as the source and copy it into the MAC private memory.
 *  The caller is responsible for freeing the memory allocated for the payload.
 * \details
 *  The new value is assigned also to the \c macBeaconPayloadLength attribute according to
 *  the new payload length.
 * \details
 *  To assign the empty beacon payload the caller shall specify the empty payload.
*****************************************************************************************/
INLINE void macPibApiSetBeaconPayload(MAC_WITH_GIVEN_CONTEXT(const MAC_BeaconPayload_t *const newPayload))
{
    SYS_DataLength_t  newPayloadSize;       /* The new value for macBeaconPayloadLength. */

    SYS_DbgAssertComplex(NULL != newPayload, LOG_macPibApiSetBeaconPayload_NullNewPayload);
    newPayloadSize = SYS_GetPayloadSize(newPayload);
    SYS_DbgAssertComplex(newPayloadSize <= UINT8_MAX, LOG_macPibApiSetBeaconPayload_NewPayloadTooLong);
    macPibApiSetBeaconPayloadLength(MAC_FOR_GIVEN_CONTEXT((MAC_BeaconPayloadLength_t)newPayloadSize));

    if (newPayloadSize > 0)
    {
        if (MAC_IS_ZBPRO_CONTEXT(MAC_GIVEN_CONTEXT_ID))
            MAC_CODE_FOR_ZBPRO_CONTEXT( SYS_CopyFromPayload(MAC_MEMORY_PIB_BEACON_PAYLOAD_ZBPRO(),
                    newPayload, /*offset*/ 0, newPayloadSize) );
        else /* MAC_IS_RF4CE_CONTEXT(MAC_GIVEN_CONTEXT_ID) */
            MAC_CODE_FOR_RF4CE_CONTEXT( SYS_CopyFromPayload(MAC_MEMORY_PIB_BEACON_PAYLOAD_RF4CE(),
                    newPayload, /*offset*/ 0, newPayloadSize) );
    }
}
#endif /* ! _MAC_CONTEXT_RF4CE_CONTROLLER_ */


/*************************************************************************************//**
 * \brief   Returns value of the MAC-PIB attribute macBeaconOrder.
 * \return  Value of the requested MAC-PIB attribute.
*****************************************************************************************/
INLINE MAC_BeaconOrder_t macPibApiGetBeaconOrder(void)
{
    return MAC_ATTR_DEFAULT_VALUE_BEACON_ORDER;
}


/*************************************************************************************//**
 * \brief   Returns value of the MAC-PIB attribute macBeaconTxTime.
 * \return  Value of the requested MAC-PIB attribute.
*****************************************************************************************/
INLINE MAC_BeaconTxTime_t macPibApiGetBeaconTxTime(void)
{
    return MAC_ATTR_DEFAULT_VALUE_BEACON_TX_TIME;
}


/*************************************************************************************//**
 * \brief   Returns value of the MAC-PIB attribute macBSN.
 * \param[in]   __givenContextId    Identifier of the specified MAC Context.
 * \return  Value of the requested MAC-PIB attribute.
*****************************************************************************************/
INLINE MAC_Bsn_t macPibApiGetBsn(MAC_WITHIN_GIVEN_CONTEXT)
{
#if !defined(_MAC_CONTEXT_RF4CE_CONTROLLER_)
    return MAC_MEMORY_PIB_BEACONING().macBSN;
#else
    return MAC_ATTR_DEFAULT_VALUE_BSN;
#endif
}


#if !defined(_MAC_CONTEXT_RF4CE_CONTROLLER_)
/*************************************************************************************//**
 * \brief   Returns value of the MAC-PIB attribute macBSN with post-increment by one.
 * \param[in]   __givenContextId    Identifier of the specified MAC Context.
 * \return  Value of the requested MAC-PIB attribute.
*****************************************************************************************/
INLINE MAC_Bsn_t macPibApiGetBsnWithPostInc(MAC_WITHIN_GIVEN_CONTEXT)
{
    return (MAC_MEMORY_PIB_BEACONING().macBSN)++;
}


/*************************************************************************************//**
 * \brief   Assigns a new value to the MAC-PIB attribute macBSN.
 * \param[in]   __givenContextId    Identifier of the specified MAC Context.
 * \param[in]   newValue            New value of the specified MAC-PIB attribute.
*****************************************************************************************/
INLINE void macPibApiSetBsn(MAC_WITH_GIVEN_CONTEXT(const MAC_Bsn_t newValue))
{
    MAC_MEMORY_PIB_BEACONING().macBSN = newValue;
}
#endif /* ! _MAC_CONTEXT_RF4CE_CONTROLLER_ */


/*************************************************************************************//**
 * \brief   Returns value of the MAC-PIB attribute macCoordExtendedAddress.
 * \param[in]   __givenContextId    Identifier of the specified MAC Context.
 * \return  Value of the requested MAC-PIB attribute.
*****************************************************************************************/
INLINE MAC_CoordExtendedAddress_t macPibApiGetCoordExtendedAddress(MAC_WITHIN_GIVEN_CONTEXT)
{
    if (MAC_IS_ZBPRO_CONTEXT(MAC_GIVEN_CONTEXT_ID))
        MAC_CODE_FOR_ZBPRO_CONTEXT( return MAC_MEMORY_PIB_ZBPRO().macCoordExtendedAddress );
    else
        return MAC_ATTR_DEFAULT_VALUE_COORD_EXTENDED_ADDRESS;
}


#if defined(_MAC_CONTEXT_ZBPRO_)
/*************************************************************************************//**
 * \brief   Assigns a new value to the ZigBee PRO MAC-PIB attribute
 *  macCoordExtendedAddress.
 * \param[in]   newValue    New value of the specified MAC-PIB attribute.
*****************************************************************************************/
INLINE void macPibApiSetCoordExtendedAddressZBPRO(const MAC_CoordExtendedAddress_t newValue)
{
    MAC_MEMORY_PIB_ZBPRO().macCoordExtendedAddress = newValue;
}
#endif /* _MAC_CONTEXT_ZBPRO_ */


/*************************************************************************************//**
 * \brief   Returns value of the MAC-PIB attribute macCoordShortAddress.
 * \param[in]   __givenContextId    Identifier of the specified MAC Context.
 * \return  Value of the requested MAC-PIB attribute.
*****************************************************************************************/
INLINE MAC_CoordShortAddress_t macPibApiGetCoordShortAddress(MAC_WITHIN_GIVEN_CONTEXT)
{
    if (MAC_IS_ZBPRO_CONTEXT(MAC_GIVEN_CONTEXT_ID))
        MAC_CODE_FOR_ZBPRO_CONTEXT( return MAC_MEMORY_PIB_ZBPRO().macCoordShortAddress );
    else
        return MAC_ATTR_DEFAULT_VALUE_COORD_SHORT_ADDRESS;
}


#if defined(_MAC_CONTEXT_ZBPRO_)
/*************************************************************************************//**
 * \brief   Assigns a new value to the ZigBee PRO MAC-PIB attribute macCoordShortAddress.
 * \param[in]   newValue    New value of the specified MAC-PIB attribute.
*****************************************************************************************/
INLINE void macPibApiSetCoordShortAddressZBPRO(const MAC_CoordShortAddress_t newValue)
{
    MAC_MEMORY_PIB_ZBPRO().macCoordShortAddress = newValue;
}
#endif /* _MAC_CONTEXT_ZBPRO_ */


/*************************************************************************************//**
 * \brief   Returns value of the MAC-PIB attribute macDSN.
 * \param[in]   __givenContextId    Identifier of the specified MAC Context.
 * \return  Value of the requested MAC-PIB attribute.
*****************************************************************************************/
INLINE MAC_Dsn_t macPibApiGetDsn(MAC_WITHIN_GIVEN_CONTEXT)
{
    return MAC_MEMORY_PIB_ESSENTIAL().macDSN;
}


/*************************************************************************************//**
 * \brief   Returns value of the MAC-PIB attribute macDSN with post-increment by one.
 * \param[in]   __givenContextId    Identifier of the specified MAC Context.
 * \return  Value of the requested MAC-PIB attribute.
*****************************************************************************************/
INLINE MAC_Dsn_t macPibApiGetDsnWithPostInc(MAC_WITHIN_GIVEN_CONTEXT)
{
    return (MAC_MEMORY_PIB_ESSENTIAL().macDSN)++;
}


/*************************************************************************************//**
 * \brief   Assigns a new value to the MAC-PIB attribute macDSN.
 * \param[in]   __givenContextId    Identifier of the specified MAC Context.
 * \param[in]   newValue            New value of the specified MAC-PIB attribute.
*****************************************************************************************/
INLINE void macPibApiSetDsn(MAC_WITH_GIVEN_CONTEXT(const MAC_Dsn_t newValue))
{
    MAC_MEMORY_PIB_ESSENTIAL().macDSN = newValue;
}


/*************************************************************************************//**
 * \brief   Returns value of the MAC-PIB attribute macGTSPermit.
 * \return  Value of the requested MAC-PIB attribute.
*****************************************************************************************/
INLINE MAC_GTSPermit_t macPibApiGetGTSPermit(void)
{
    return MAC_ATTR_DEFAULT_VALUE_GTS_PERMIT;
}


/*************************************************************************************//**
 * \brief   Returns value of the MAC-PIB attribute macMaxCSMABackoffs.
 * \param[in]   __givenContextId    Identifier of the specified MAC Context.
 * \return  Value of the requested MAC-PIB attribute.
*****************************************************************************************/
INLINE MAC_MaxCSMABackoffs_t macPibApiGetMaxCSMABackoffs(MAC_WITHIN_GIVEN_CONTEXT)
{
    return MAC_MEMORY_PIB_ESSENTIAL().macMaxCSMABackoffs;
}


/*************************************************************************************//**
 * \brief   Assigns a new value to the ZigBee PRO MAC-PIB attribute macMaxCSMABackoffs.
 * \param[in]   __givenContextId    Identifier of the specified MAC Context.
 * \param[in]   newValue            New value of the specified MAC-PIB attribute.
*****************************************************************************************/
INLINE void macPibApiSetMaxCSMABackoffs(MAC_WITH_GIVEN_CONTEXT(const MAC_MaxCSMABackoffs_t newValue))
{
    SYS_DbgAssertComplex(newValue <= MAC_ATTR_MAXALLOWED_VALUE_MAX_CSMA_BACKOFFS,
            LOG_macPibApiSetMaxCSMABackoffs_InvalidNewValue);

    ATOMIC_SECTION_ENTER(SYS_ATOMIC_DEFAULT_UID)
        MAC_MEMORY_PIB_ESSENTIAL().macMaxCSMABackoffs = newValue;
    ATOMIC_SECTION_LEAVE(SYS_ATOMIC_DEFAULT_UID)
}


/*************************************************************************************//**
 * \brief   Returns value of the MAC-PIB attribute macMinBE.
 * \param[in]   __givenContextId    Identifier of the specified MAC Context.
 * \return  Value of the requested MAC-PIB attribute.
*****************************************************************************************/
INLINE MAC_MinBE_t macPibApiGetMinBE(MAC_WITHIN_GIVEN_CONTEXT)
{
    return MAC_MEMORY_PIB_ESSENTIAL().macMinBE;
}


/*************************************************************************************//**
 * \brief   Assigns a new value to the ZigBee PRO MAC-PIB attribute macMinBE.
 * \param[in]   __givenContextId    Identifier of the specified MAC Context.
 * \param[in]   newValue    New value of the specified MAC-PIB attribute.
*****************************************************************************************/
INLINE void macPibApiSetMinBE(MAC_WITH_GIVEN_CONTEXT(const MAC_MinBE_t newValue))
{
    SYS_DbgAssertComplex(newValue <= MAC_MEMORY_PIB_ESSENTIAL().macMaxBE, LOG_macPibApiSetMinBE_InvalidNewValue);

    ATOMIC_SECTION_ENTER(SYS_ATOMIC_DEFAULT_UID)
        MAC_MEMORY_PIB_ESSENTIAL().macMinBE = newValue;
    ATOMIC_SECTION_LEAVE(SYS_ATOMIC_DEFAULT_UID)
}


/*************************************************************************************//**
 * \brief   Returns value of the MAC-PIB attribute macPANId.
 * \param[in]   __givenContextId    Identifier of the specified MAC Context.
 * \return  Value of the requested MAC-PIB attribute.
*****************************************************************************************/
INLINE MAC_PanId_t macPibApiGetPanId(MAC_WITHIN_GIVEN_CONTEXT)
{
    return MAC_MEMORY_PIB_ESSENTIAL().macPANId;
}


/*************************************************************************************//**
 * \brief   Checks if the MAC-PIB attribute macPANId is assigned with valid PAN Id value.
 * \param[in]   __givenContextId    Identifier of the specified MAC Context.
 * \return  TRUE if macPANId is assigned with valid PAN Id - i.e., less than 0xFFFF; FALSE
 *  otherwise.
*****************************************************************************************/
INLINE bool macPibApiGetPanIdAssigned(MAC_WITHIN_GIVEN_CONTEXT)
{
    return (MAC_MEMORY_PIB_ESSENTIAL().macPANId < MAC_UNASSIGNED_PAN_ID);
}


/*************************************************************************************//**
 * \brief   Assigns a new value to the MAC-PIB attribute macPANId.
 * \param[in]   __givenContextId    Identifier of the specified MAC Context.
 * \param[in]   newValue            New value of the specified MAC-PIB attribute.
*****************************************************************************************/
INLINE void macPibApiSetPanId(MAC_WITH_GIVEN_CONTEXT(const MAC_PanId_t newValue))
{
    ATOMIC_SECTION_ENTER(SYS_ATOMIC_DEFAULT_UID)
        MAC_MEMORY_PIB_ESSENTIAL().macPANId = newValue;
#if defined(_MAC_DUAL_CONTEXT_)
        HAL_RadioFrameFilterSetPanId(MAC_GIVEN_CONTEXT_ID, newValue);
#else
        HAL_RadioFrameFilterSetPanId(newValue);
#endif
    ATOMIC_SECTION_LEAVE(SYS_ATOMIC_DEFAULT_UID)
}


/*************************************************************************************//**
 * \brief   Returns value of the MAC-PIB attribute macPromiscuousMode.
 * \return  Value of the requested MAC-PIB attribute.
*****************************************************************************************/
INLINE MAC_PromiscuousMode_t macPibApiGetPromiscuousMode(void)
{
#if defined(_MAC_TESTER_)
    return macMemoryPromiscuousMode;
#else
    return MAC_ATTR_DEFAULT_VALUE_PROMISCUOUS_MODE;
#endif
}


#if defined(_MAC_TESTER_)
/*************************************************************************************//**
 * \brief   Assigns a new value to the MAC-PIB attribute macPromiscuousMode.
 * \param[in]   newValue    New value of the specified MAC-PIB attribute.
 * \note    Attribute macPromiscuousMode is shared by both contexts for the case of dual
 *  context MAC.
 * \note    Attribute macPromiscuousMode has custom format and regulates different aspects
 *  of MAC behavior.
*****************************************************************************************/
INLINE void macPibApiSetPromiscuousMode(const MAC_PromiscuousMode_t newValue)
{
    ATOMIC_SECTION_ENTER(ATM_macPibApiSetPromiscuousMode)
        macMemoryPromiscuousMode = newValue;
        SET_REG_FIELD(RF4CE_MAC_RX_FF_CONFIG, FRAME_FILTERING_DISABLE,
                ((newValue & (MAC_PROMISCUOUS_MODE_DISABLE_HW_FILTER |
                        MAC_PROMISCUOUS_MODE_DISABLE_DST_FILTER)) != 0x0));
        SET_REG_FIELD(RF4CE_MAC_RX_FF_CONFIG, BYPASS_CRC,
                ((newValue & MAC_PROMISCUOUS_MODE_DISABLE_HW_FILTER) != 0x0));
    ATOMIC_SECTION_LEAVE(ATM_macPibApiSetPromiscuousMode)
}
#endif

/*************************************************************************************//**
 * \brief   Returns value of the MAC-PIB attribute macRxOnWhenIdle.
 * \param[in]   __givenContextId    Identifier of the specified MAC Context.
 * \return  Value of the requested MAC-PIB attribute.
*****************************************************************************************/
INLINE MAC_RxOnWhenIdle_t macPibApiGetRxOnWhenIdle(MAC_WITHIN_GIVEN_CONTEXT)
{
    return MAC_MEMORY_PIB_ESSENTIAL().macRxOnWhenIdle;
}


/*************************************************************************************//**
 * \brief   Assigns a new value to the MAC-PIB attribute macRxOnWhenIdle.
 * \param[in]   __givenContextId    Identifier of the specified MAC Context.
 * \param[in]   newValue            New value of the specified MAC-PIB attribute.
*****************************************************************************************/
INLINE void macPibApiSetRxOnWhenIdle(MAC_WITH_GIVEN_CONTEXT(const MAC_RxOnWhenIdle_t newValue))
{
    ATOMIC_SECTION_ENTER(ATM_macPibApiSetRxOnWhenIdle)
        MAC_MEMORY_PIB_ESSENTIAL().macRxOnWhenIdle = newValue;
        SYS_DbgAssertStatic(MAC_TRX_MODE_CMD_PERSIST_OFF == FALSE);
        SYS_DbgAssertStatic(MAC_TRX_MODE_CMD_PERSIST_ON  == TRUE);
        macLeTrxModeDispAcceptCmd(MAC_FOR_GIVEN_CONTEXT((MacLeTrxModeCmd_t)(FALSE != newValue), /*timestamp*/ 0));
    ATOMIC_SECTION_LEAVE(ATM_macPibApiSetRxOnWhenIdle)
}


/*************************************************************************************//**
 * \brief   Returns value of the MAC-PIB attribute macShortAddress.
 * \param[in]   __givenContextId    Identifier of the specified MAC Context.
 * \return  Value of the requested MAC-PIB attribute.
*****************************************************************************************/
INLINE MAC_ShortAddress_t macPibApiGetShortAddress(MAC_WITHIN_GIVEN_CONTEXT)
{
    return MAC_MEMORY_PIB_ESSENTIAL().macShortAddress;
}


/*************************************************************************************//**
 * \brief   Checks if the MAC-PIB attribute macShortAddress is assigned with valid short
 *  address value.
 * \param[in]   __givenContextId    Identifier of the specified MAC Context.
 * \return  TRUE if macShortAddress is assigned with valid short address - i.e., less than
 *  0xFFFE; FALSE otherwise.
*****************************************************************************************/
INLINE bool macPibApiGetShortAddressAssigned(MAC_WITHIN_GIVEN_CONTEXT)
{
    return (MAC_MEMORY_PIB_ESSENTIAL().macShortAddress < MAC_DONT_USE_SHORT_ADDRESS);
}


/*************************************************************************************//**
 * \brief   Assigns a new value to the MAC-PIB attribute macShortAddress.
 * \param[in]   __givenContextId    Identifier of the specified MAC Context.
 * \param[in]   newValue            New value of the specified MAC-PIB attribute.
*****************************************************************************************/
INLINE void macPibApiSetShortAddress(MAC_WITH_GIVEN_CONTEXT(const MAC_ShortAddress_t newValue))
{
    ATOMIC_SECTION_ENTER(SYS_ATOMIC_DEFAULT_UID)
        MAC_MEMORY_PIB_ESSENTIAL().macShortAddress = newValue;
#if defined(_MAC_DUAL_CONTEXT_)
        HAL_RadioFrameFilterSetShortAddr(MAC_GIVEN_CONTEXT_ID, newValue);
#else
        HAL_RadioFrameFilterSetShortAddr(newValue);
#endif
    ATOMIC_SECTION_LEAVE(SYS_ATOMIC_DEFAULT_UID)
}


/*************************************************************************************//**
 * \brief   Returns value of the MAC-PIB attribute macSuperframeOrder.
 * \return  Value of the requested MAC-PIB attribute.
*****************************************************************************************/
INLINE MAC_SuperframeOrder_t macPibApiGetSuperframeOrder(void)
{
    return MAC_ATTR_DEFAULT_VALUE_SUPERFRAME_ORDER;
}


/*************************************************************************************//**
 * \brief   Returns value of the MAC-PIB attribute macTransactionPersistenceTime.
 * \param[in]   __givenContextId    Identifier of the specified MAC Context.
 * \return  Value of the requested MAC-PIB attribute.
*****************************************************************************************/
INLINE MAC_TransactionPersistenceTime_t macPibApiGetTransactionPersistenceTime(MAC_WITHIN_GIVEN_CONTEXT)
{
    if (MAC_IS_ZBPRO_CONTEXT(MAC_GIVEN_CONTEXT_ID))
        MAC_CODE_FOR_ZBPRO_CONTEXT( return MAC_MEMORY_PIB_ZBPRO().macTransactionPersistenceTime );
    else
        return MAC_ATTR_DEFAULT_VALUE_TRANSACTION_PERSISTENCE_TIME;
}


#if defined(_MAC_CONTEXT_ZBPRO_)
/*************************************************************************************//**
 * \brief   Assigns a new value to the ZigBee PRO MAC-PIB attribute
 *  macTransactionPersistenceTime.
 * \param[in]   newValue    New value of the specified MAC-PIB attribute.
*****************************************************************************************/
INLINE void macPibApiSetTransactionPersistenceTimeZBPRO(const MAC_TransactionPersistenceTime_t newValue)
{
    MAC_MEMORY_PIB_ZBPRO().macTransactionPersistenceTime = newValue;
}
#endif /* _MAC_CONTEXT_ZBPRO_ */


/*************************************************************************************//**
 * \brief   Returns value of the MAC-PIB attribute macAssociatedPANCoord.
 * \return  Value of the requested MAC-PIB attribute.
*****************************************************************************************/
INLINE MAC_AssociatedPANCoord_t macPibApiGetAssociatedPANCoord(void)
{
    return MAC_ATTR_DEFAULT_VALUE_ASSOCIATED_PAN_COORD;
}


/*************************************************************************************//**
 * \brief   Returns value of the MAC-PIB attribute macMaxBE.
 * \param[in]   __givenContextId    Identifier of the specified MAC Context.
 * \return  Value of the requested MAC-PIB attribute.
*****************************************************************************************/
INLINE MAC_MaxBE_t macPibApiGetMaxBE(MAC_WITHIN_GIVEN_CONTEXT)
{
    return MAC_MEMORY_PIB_ESSENTIAL().macMaxBE;
}


/*************************************************************************************//**
 * \brief   Assigns a new value to the ZigBee PRO MAC-PIB attribute macMaxBE.
 * \param[in]   __givenContextId    Identifier of the specified MAC Context.
 * \param[in]   newValue    New value of the specified MAC-PIB attribute.
*****************************************************************************************/
INLINE void macPibApiSetMaxBE(MAC_WITH_GIVEN_CONTEXT(const MAC_MaxBE_t newValue))
{
    SYS_DbgAssertComplex(newValue >= MAC_ATTR_MINALLOWED_VALUE_MAX_BE, LOG_macPibApiSetMaxBE_NewValueTooLow);
    SYS_DbgAssertComplex(newValue <= MAC_ATTR_MAXALLOWED_VALUE_MAX_BE, LOG_macPibApiSetMaxBE_NewValueTooHigh);

    ATOMIC_SECTION_ENTER(SYS_ATOMIC_DEFAULT_UID)
        MAC_MEMORY_PIB_ESSENTIAL().macMaxBE = newValue;
    ATOMIC_SECTION_LEAVE(SYS_ATOMIC_DEFAULT_UID)
}


/*************************************************************************************//**
 * \brief   Returns the default value of the MAC-PIB attribute macMaxFrameTotalWaitTime.
 * \param[in]   __givenContextId    Identifier of the specified MAC Context.
 * \return  Default value of the requested MAC-PIB attribute.
*****************************************************************************************/
INLINE MAC_MaxFrameTotalWaitTime_t macPibApiGetDefaultMaxFrameTotalWaitTime(MAC_WITHIN_GIVEN_CONTEXT)
{
    MAC_MaxFrameTotalWaitTime_t  macMaxFrameTotalWaitTime;      /* The value to be returned. */

    MAC_MaxBE_t            macMaxBE;                /* Value of the MAC-PIB attribute macMaxBE. */
    MAC_MinBE_t            macMinBE;                /* Value of the MAC-PIB attribute macMinBE. */
    MAC_MaxCSMABackoffs_t  macMaxCSMABackoffs;      /* Value of the MAC-PIB attribute macMaxCSMABackoffs. */

    MAC_MaxBE_t                  m;
    MAC_MaxFrameTotalWaitTime_t  term1a;
    MAC_MaxFrameTotalWaitTime_t  term1b;
    MAC_MaxFrameTotalWaitTime_t  term2a;
    MAC_MaxFrameTotalWaitTime_t  term2b;

    PHY_MaxFrameDuration_t  phyMaxFrameDuration;    /* Value of the attribute phyMaxFrameDuration. */

    /* This value is dependent on the supported PHY and is the sum of three terms:
     * Term 1: The value (2^macMinBE) multiplied by (2^m)-1, where m is the minimum value of (macMaxBE-macMinBE)
     *         and macMaxCSMABackoffs, converted from back-off periods into symbols.
     * Term 2: The value (2^macMaxBE)-1 multiplied by (macMaxCSMABackoffs-m), converted from
     *         back-off periods into symbols.
     * Term 3: The value of phyMaxFrameDuration, in symbols.
     *
     * The following formula is implemented:
     *  macMaxFrameTotalWaitTime = (2^macMinBE) * (2^m - 1) * aUnitBackoffPeriod +
     *          (2^macMaxBE - 1) * (macMaxCSMABackoffs - m) * aUnitBackoffPeriod +
     *          phyMaxFrameDuration,
     * where
     *  m = MIN(macMaxBE-macMinBE, macMaxCSMABackoffs),
     *  (2^macMinBE) * (2^m - 1) is the sum of geometric progression: Sum[ 2^(macMinBE+k) ] for k from 0 to (m-1).
     *
     * Comments on the formula:
     * - The first two additives are the maximum CSMA-CA wait time excluding 8-symbol periods of CCA;
     * - The total number of backoffs in CSMA-CA equals to (macMaxCSMABackoffs + 1);
     * - The maximum durations of consecutive backoff periods are progressive in general as (2^k - 1),
     *    where k is from macMinBE to macMaxBE;
     *
     * - For the case when (macMaxBE - macMinBE) = macMaxCSMABackoffs we have the following progression:
     *    (2^macMinBE - 1) -CCA- (2^(macMinBE+1) - 1) -CCA- ... -CCA- (2^macMaxBE - 1) -CCA-
     *    where the total number of backoffs is (macMaxCSMABackoffs + 1);
     *
     * - For the case when (macMaxBE - macMinBE) < macMaxCSMABackoffs we have the following progression:
     *    (2^macMinBE - 1) -CCA- (2^(macMinBE+1) - 1) -CCA- ... -CCA- (2^macMaxBE - 1) -CCA-
     *    where the total number of backoffs is ((macMaxBE - macMinBE) + 1)
     *    and then we have sequence of even backoffs:
     *    (2^macMaxBE - 1) -CCA- (2^macMaxBE - 1) -CCA- ... -CCA- (2^macMaxBE - 1) -CCA-
     *    where the total number of backoffs is (macMaxCSMABackoffs - (macMaxBE - macMinBE));
     *
     * - For the case when (macMaxBE - macMinBE) > macMaxCSMABackoffs we have the following progression:
     *    (2^macMinBE - 1) -CCA- (2^(macMinBE+1) - 1) -CCA- ... -CCA- (2^(macMinBE+macMaxCSMABackoffs) - 1) -CCA-
     *    where the total number of backoffs is (macMaxCSMABackoffs + 1);
     *
     * - For the general case lets m = MIN(macMaxBE - macMinBE, macMaxCSMABackoffs);
     * - Then for each of three cases above we have the following first progression:
     *    (2^macMinBE - 1) -CCA- (2^(macMinBE+1) - 1) -CCA- ... -CCA- (2^(macMinBE+m) - 1) -CCA-
     *    where the total number of backoffs is (m + 1)
     *    and then we have sequence of even backoffs:
     *    (2^macMaxBE - 1) -CCA- (2^macMaxBE - 1) -CCA- ... -CCA- (2^macMaxBE - 1) -CCA-
     *    where the total number of backoffs is (macMaxCSMABackoffs - m)
     *    this sequence actually is empty if (macMaxBE - macMinBE) >= macMaxCSMABackoffs;
     *
     * - Duration of the first sequence excluding 8-symbol periods of CCA equals to the sum of members of geometric
     *   progression with the first term a0 = 2^(macMinBE+0), ratio r = 2, and the last term am = 2^(macMinBE+m), and
     *   minus the value of 1 * (m + 1) = m + 1.
     *   The sum of the geometric progression (a0 + ... + am) for this case equals = a0 * (r^(m+1) - 1) / (r - 1).
     *   Taking into account values of a0 and r we have = (2^macMinBE) * (2^(m+1) - 1) / (2 - 1).
     *   And finally we have:
     *   (2^macMinBE - 1) + (2^(macMinBE+1) - 1) + ... + (2^(macMinBE+m) - 1) = (2^macMinBE) * (2^(m+1) - 1) - (m + 1)
     *
     * - Unfortunately this expression does not correspond to the expression (14) given in the IEEE 802.15.4-2006.
     */
    macMaxBE = macPibApiGetMaxBE(MAC_GIVEN_CONTEXT_ID);
    macMinBE = macPibApiGetMinBE(MAC_GIVEN_CONTEXT_ID);
    macMaxCSMABackoffs = macPibApiGetMaxCSMABackoffs(MAC_GIVEN_CONTEXT_ID);

    m = MIN(macMaxBE - macMinBE, macMaxCSMABackoffs);

    term1a = (1 << macMinBE);
    term1b = (1 << m) - 1;
    term2a = (1 << macMaxBE);
    term2b = (macMaxCSMABackoffs - m);

    phyMaxFrameDuration = phyPibApiGetMaxFrameDuration();

    macMaxFrameTotalWaitTime = (term1a * term1b + term2a * term2b) * MAC_aUnitBackoffPeriod + phyMaxFrameDuration;

    return macMaxFrameTotalWaitTime;
}


/*************************************************************************************//**
 * \brief   Returns value of the MAC-PIB attribute macMaxFrameTotalWaitTime.
 * \param[in]   __givenContextId    Identifier of the specified MAC Context.
 * \return  Value of the requested MAC-PIB attribute.
*****************************************************************************************/
INLINE MAC_MaxFrameTotalWaitTime_t macPibApiGetMaxFrameTotalWaitTime(MAC_WITHIN_GIVEN_CONTEXT)
{
    if (MAC_IS_ZBPRO_CONTEXT(MAC_GIVEN_CONTEXT_ID))
        MAC_CODE_FOR_ZBPRO_CONTEXT( return MAC_MEMORY_PIB_ZBPRO().macMaxFrameTotalWaitTime );
    else
        return macPibApiGetDefaultMaxFrameTotalWaitTime(MAC_GIVEN_CONTEXT_ID);
}


#if defined(_MAC_CONTEXT_ZBPRO_)
/*************************************************************************************//**
 * \brief   Assigns a new value to the ZigBee PRO MAC-PIB attribute
 *  macMaxFrameTotalWaitTime.
 * \param[in]   newValue    New value of the specified MAC-PIB attribute.
*****************************************************************************************/
INLINE void macPibApiSetMaxFrameTotalWaitTimeZBPRO(const MAC_MaxFrameTotalWaitTime_t newValue)
{
    ATOMIC_SECTION_ENTER(SYS_ATOMIC_DEFAULT_UID)
        MAC_MEMORY_PIB_ZBPRO().macMaxFrameTotalWaitTime = newValue;
    ATOMIC_SECTION_LEAVE(SYS_ATOMIC_DEFAULT_UID)
}
#endif /* _MAC_CONTEXT_ZBPRO_ */


/*************************************************************************************//**
 * \brief   Returns value of the MAC-PIB attribute macMaxFrameRetries.
 * \param[in]   __givenContextId    Identifier of the specified MAC Context.
 * \return  Value of the requested MAC-PIB attribute.
*****************************************************************************************/
INLINE MAC_MaxFrameRetries_t macPibApiGetMaxFrameRetries(MAC_WITHIN_GIVEN_CONTEXT)
{
    return MAC_MEMORY_PIB_ESSENTIAL().macMaxFrameRetries;
}


/*************************************************************************************//**
 * \brief   Assigns a new value to the ZigBee PRO MAC-PIB attribute macMaxFrameRetries.
 * \param[in]   __givenContextId    Identifier of the specified MAC Context.
 * \param[in]   newValue            New value of the specified MAC-PIB attribute.
*****************************************************************************************/
INLINE void macPibApiSetMaxFrameRetries(MAC_WITH_GIVEN_CONTEXT(const MAC_MaxFrameRetries_t newValue))
{
    SYS_DbgAssertComplex(newValue <= MAC_ATTR_MAXALLOWED_VALUE_MAX_FRAME_RETRIES,
            LOG_macPibApiSetMaxFrameRetriesZBPRO_InvalidNewValue);

    ATOMIC_SECTION_ENTER(SYS_ATOMIC_DEFAULT_UID)
        MAC_MEMORY_PIB_ESSENTIAL().macMaxFrameRetries = newValue;
    ATOMIC_SECTION_LEAVE(SYS_ATOMIC_DEFAULT_UID)
}


/*************************************************************************************//**
 * \brief   Returns value of the MAC-PIB attribute macResponseWaitTime.
 * \param[in]   __givenContextId    Identifier of the specified MAC Context.
 * \return  Value of the requested MAC-PIB attribute.
*****************************************************************************************/
INLINE MAC_ResponseWaitTime_t macPibApiGetResponseWaitTime(MAC_WITHIN_GIVEN_CONTEXT)
{
    if (MAC_IS_ZBPRO_CONTEXT(MAC_GIVEN_CONTEXT_ID))
        MAC_CODE_FOR_ZBPRO_CONTEXT( return MAC_MEMORY_PIB_ZBPRO().macResponseWaitTime );
    else
        return MAC_ATTR_DEFAULT_VALUE_RESPONSE_WAIT_TIME;
}


#if defined(_MAC_CONTEXT_ZBPRO_)
/*************************************************************************************//**
 * \brief   Assigns a new value to the ZigBee PRO MAC-PIB attribute macResponseWaitTime.
 * \param[in]   newValue    New value of the specified MAC-PIB attribute.
*****************************************************************************************/
INLINE void macPibApiSetResponseWaitTimeZBPRO(const MAC_ResponseWaitTime_t newValue)
{
    SYS_DbgAssertComplex(newValue >= MAC_ATTR_MINALLOWED_VALUE_RESPONSE_WAIT_TIME,
            LOG_macPibApiSetResponseWaitTimeZBPRO_NewValueTooLow);
    SYS_DbgAssertComplex(newValue <= MAC_ATTR_MAXALLOWED_VALUE_RESPONSE_WAIT_TIME,
            LOG_macPibApiSetResponseWaitTimeZBPRO_NewValueTooHigh);

    MAC_MEMORY_PIB_ZBPRO().macResponseWaitTime = newValue;
}
#endif /* _MAC_CONTEXT_ZBPRO_ */


/*************************************************************************************//**
 * \brief   Returns value of the MAC-PIB attribute macSyncSymbolOffset.
 * \return  Value of the requested MAC-PIB attribute.
*****************************************************************************************/
INLINE MAC_SyncSymbolOffset_t macPibApiGetSyncSymbolOffset(void)
{
    return MAC_ATTR_DEFAULT_VALUE_SYNC_SYMBOL_OFFSET;
}


/*************************************************************************************//**
 * \brief   Returns value of the MAC-PIB attribute macTimestampSupported.
 * \return  Value of the requested MAC-PIB attribute.
*****************************************************************************************/
INLINE MAC_TimestampSupported_t macPibApiGetTimestampSupported(void)
{
    return MAC_ATTR_DEFAULT_VALUE_TIMESTAMP_SUPPORTED;
}


/*************************************************************************************//**
 * \brief   Returns value of the MAC-PIB attribute macSecurityEnabled.
 * \param[in]   __givenContextId    Identifier of the specified MAC Context.
 * \return  Value of the requested MAC-PIB attribute.
*****************************************************************************************/
INLINE MAC_SecurityEnabled_t macPibApiGetSecurityEnabled(MAC_WITHIN_GIVEN_CONTEXT)
{
    if (MAC_IS_ZBPRO_CONTEXT(MAC_GIVEN_CONTEXT_ID))
        MAC_CODE_FOR_ZBPRO_CONTEXT( return MAC_MEMORY_PIB_ZBPRO().macSecurityEnabled );
    else
        return MAC_ATTR_DEFAULT_VALUE_SECURITY_ENABLED;
}


#if defined(_MAC_CONTEXT_ZBPRO_)
/*************************************************************************************//**
 * \brief   Assigns a new value to the ZigBee PRO MAC-PIB attribute macSecurityEnabled.
 * \param[in]   newValue    New value of the specified MAC-PIB attribute.
*****************************************************************************************/
INLINE void macPibApiSetSecurityEnabledZBPRO(const MAC_SecurityEnabled_t newValue)
{
    MAC_MEMORY_PIB_ZBPRO().macSecurityEnabled = newValue;
}
#endif /* _MAC_CONTEXT_ZBPRO_ */


/*************************************************************************************//**
 * \brief   Returns value of the MAC-PIB attribute macMinLIFSPeriod.
 * \return  Value of the requested MAC-PIB attribute.
*****************************************************************************************/
INLINE MAC_MinLIFSPeriod_t macPibApiGetMinLIFSPeriod(void)
{
    return MAC_ATTR_DEFAULT_VALUE_MIN_LIFS_PERIOD;
}


/*************************************************************************************//**
 * \brief   Returns value of the MAC-PIB attribute macMinSIFSPeriod.
 * \return  Value of the requested MAC-PIB attribute.
*****************************************************************************************/
INLINE MAC_MinSIFSPeriod_t macPibApiGetMinSIFSPeriod(void)
{
    return MAC_ATTR_DEFAULT_VALUE_MIN_SIFS_PERIOD;
}


/*************************************************************************************//**
 * \brief   Returns value of the MAC-PIB attribute macCurrentChannel.
 * \param[in]   __givenContextId    Identifier of the specified MAC Context.
 * \return  Value of the requested MAC-PIB attribute.
*****************************************************************************************/
INLINE PHY_Channel_t macPibApiGetCurrentChannel(MAC_WITHIN_GIVEN_CONTEXT)
{
    return PHY__Take_Channel(MAC_MEMORY_PIB_ESSENTIAL().macCurrentChannelOnPage);
}


/*************************************************************************************//**
 * \brief   Assigns a new value to the ZigBee PRO MAC-PIB attribute macCurrentChannel.
 * \param[in]   __givenContextId    Identifier of the specified MAC Context.
 * \param[in]   newValue            New value of the specified MAC-PIB attribute.
*****************************************************************************************/
INLINE void macPibApiSetCurrentChannel(MAC_WITH_GIVEN_CONTEXT(const PHY_Channel_t newValue))
{
    SYS_DbgAssertComplex(newValue <= PHY_ATTR_MAXALLOWED_VALUE_CURRENT_CHANNEL,
            LOG_macPibApiSetCurrentChannel_InvalidNewValue);

    PHY__Update_Channel(MAC_MEMORY_PIB_ESSENTIAL().macCurrentChannelOnPage, newValue);
}


/*************************************************************************************//**
 * \brief   Returns value of the MAC-PIB attribute macCurrentPage.
 * \param[in]   __givenContextId    Identifier of the specified MAC Context.
 * \return  Value of the requested MAC-PIB attribute.
*****************************************************************************************/
INLINE PHY_Page_t macPibApiGetCurrentPage(MAC_WITHIN_GIVEN_CONTEXT)
{
    return PHY__Take_Page(MAC_MEMORY_PIB_ESSENTIAL().macCurrentChannelOnPage);
}


/*************************************************************************************//**
 * \brief   Assigns a new value to the ZigBee PRO MAC-PIB attribute macCurrentPage.
 * \param[in]   __givenContextId    Identifier of the specified MAC Context.
 * \param[in]   newValue            New value of the specified MAC-PIB attribute.
*****************************************************************************************/
INLINE void macPibApiSetCurrentPage(MAC_WITH_GIVEN_CONTEXT(const PHY_Page_t newValue))
{
    SYS_DbgAssertComplex(newValue <= PHY_ATTR_MAXALLOWED_VALUE_CURRENT_PAGE,
            LOG_macPibApiSetCurrentPage_InvalidNewValue);

    PHY__Update_Page(MAC_MEMORY_PIB_ESSENTIAL().macCurrentChannelOnPage, newValue);
}


/*************************************************************************************//**
 * \brief   Assigns a new value to the ZigBee PRO MAC-PIB attributes macCurrentChannel and
 *  macCurrentPage.
 * \param[in]   __givenContextId    Identifier of the specified MAC Context.
 * \param[in]   newValue            New value of the specified MAC-PIB attributes.
*****************************************************************************************/
INLINE void macPibApiSetCurrentChannelOnPage(MAC_WITH_GIVEN_CONTEXT(const PHY_PageChannel_t newValue))
{
    SYS_DbgAssertComplex(PHY__Take_Channel(newValue) <= PHY_ATTR_MAXALLOWED_VALUE_CURRENT_CHANNEL,
            LOG_macPibApiSetCurrentChannelOnPage_InvalidChannelId);
    SYS_DbgAssertComplex(PHY__Take_Page(newValue) <= PHY_ATTR_MAXALLOWED_VALUE_CURRENT_PAGE,
            LOG_macPibApiSetCurrentChannelOnPage_InvalidPageId);

    MAC_MEMORY_PIB_ESSENTIAL().macCurrentChannelOnPage = newValue;
}


/*************************************************************************************//**
 * \brief   Resets MAC-PIB private attributes to their default values.
 * \param[in]   __givenContextId    Identifier of the specified MAC Context.
*****************************************************************************************/
INLINE void macPibApiReset(MAC_WITHIN_GIVEN_CONTEXT)
{
    macMemoryPibReset(MAC_GIVEN_CONTEXT_ID);

    /* Reinitialize the PRNG. */
    HAL_RandomMove(MAC_MEMORY_FE_PRNG_DESCR(), 0);

#if !defined(_MAC_ZERO_SEQNUM_ON_RESET_)
    /* Initialize macDSN and macBSN with random values. */
    MAC_MEMORY_PIB_ESSENTIAL().macDSN = HAL_Random(MAC_MEMORY_FE_PRNG_DESCR());
# if !defined(_MAC_CONTEXT_RF4CE_CONTROLLER_)
    MAC_MEMORY_PIB_BEACONING().macBSN = HAL_Random(MAC_MEMORY_FE_PRNG_DESCR());
# endif
#endif /* ! _MAC_ZERO_SEQNUM_ON_RESET_ */

    /* Synchronize Hardware Frame Filter registers with MAC-PIB attributes default values. */
#if defined(_MAC_DUAL_CONTEXT_)
    HAL_RadioFrameFilterSetPanCoord(MAC_GIVEN_CONTEXT_ID, macPibApiGetPanCoordinator(MAC_GIVEN_CONTEXT_ID));
    HAL_RadioFrameFilterSetPanId(MAC_GIVEN_CONTEXT_ID, macPibApiGetPanId(MAC_GIVEN_CONTEXT_ID));
    HAL_RadioFrameFilterSetShortAddr(MAC_GIVEN_CONTEXT_ID, macPibApiGetShortAddress(MAC_GIVEN_CONTEXT_ID));
    HAL_RadioFrameFilterSetExtAddr(MAC_GIVEN_CONTEXT_ID, macPibApiGetExtendedAddress(MAC_GIVEN_CONTEXT_ID));
#else
    HAL_RadioFrameFilterSetPanCoord(macPibApiGetPanCoordinator());
    HAL_RadioFrameFilterSetPanId(macPibApiGetPanId());
    HAL_RadioFrameFilterSetShortAddr(macPibApiGetShortAddress());
    HAL_RadioFrameFilterSetExtAddr(macPibApiGetExtendedAddress());
#endif
}


#endif /* _BB_MAC_PIB_API_H */

/* eof bbMacPibApi.h */