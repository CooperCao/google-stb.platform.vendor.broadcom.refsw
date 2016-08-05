/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************
*
* FILENAME: $Workfile: trunk/stack/IEEE/PHY/include/private/bbPhyPibApi.h $
*
* DESCRIPTION:
*   PHY-PIB API interface.
*
* $Revision: 10263 $
* $Date: 2016-02-29 18:03:06Z $
*
*****************************************************************************************/


#ifndef _BB_PHY_PIB_API_H
#define _BB_PHY_PIB_API_H


/************************* INCLUDES *****************************************************/
#include "bbPhySapPib.h"
#include "bbPhySapDefs.h"
#include "private/bbPhyMemory.h"


/************************* PROTOTYPES ***************************************************/
/*************************************************************************************//**
 * \brief   Obtains the set of channels implemented on particular channel page.
 * \param[in]   pageId      Identifier of a channel page.
 * \return  Bitmask of corresponding channels set. Empty channels set (zero value) is
 *  returned if the channel page with identifier \p pageId is not implemented.
*****************************************************************************************/
PHY_PRIVATE PHY_ChannelMask_t phyPibApiGetChannelsSet(const PHY_Page_t pageId);


/*************************************************************************************//**
 * \brief   Discovers if a channel is implemented on a particular channel page.
 * \param[in]   channelOnPage   Plain value of a Channel-on-Page object.
 * \return  TRUE if the specified channel is implemented on the specified channel page;
 *  FALSE otherwise.
*****************************************************************************************/
PHY_PRIVATE bool phyPibApiFindChannelOnPage(const PHY_PageChannel_t channelOnPage);


/************************* INLINES ******************************************************/
/*************************************************************************************//**
 * \brief   Returns value of the PHY-PIB attribute phyCurrentChannel.
 * \return  Value of the requested PHY-PIB attribute.
*****************************************************************************************/
INLINE PHY_Channel_t phyPibApiGetCurrentChannel(void)
{
    return PHY__Take_Channel(PHY_MEMORY_PIB().phyCurrentChannelOnPage);
}


/*************************************************************************************//**
 * \brief   Returns value of the PHY-PIB attribute phyChannelsSupported.
 * \param[out]  payload     Pointer to the unused payload descriptor to save the requested
 *  supported channels sets array.
 * \details
 *  The caller shall allocate an unused payload descriptor and specify it to this function
 *  by pointer with \p payload to receive the requested supported channels sets array.
 *  Such a descriptor may not be emptied by the caller prior to calling this function.
 * \details
 *  The static payload descriptor is returned. The static payload capacity and the actual
 *  payload length both are set to the number of implemented channel pages multiplied by
 *  four (the size of bitmask of implemented channels set for a single channels page).
*****************************************************************************************/
INLINE void phyPibApiGetChannelsSupported(PHY_ChannelsSupported_t *const payload)
{
    SYS_LinkStaticPayload(payload, (void*)(PHY_MEMORY_PIB_CHANNELS_SUPPORTED()),
            HAL_RADIO__PAGES_NUM * sizeof(PHY_ChannelsSupportedOnPage_t));
    SYS_MemAlloc(payload, HAL_RADIO__PAGES_NUM * sizeof(PHY_ChannelsSupportedOnPage_t));
}


/*************************************************************************************//**
 * \brief   Returns value of the PHY-PIB attribute phyTransmitPower.
 * \return  Value of the requested PHY-PIB attribute.
*****************************************************************************************/
INLINE PHY_TransmitPower_t phyPibApiGetTransmitPower(void)
{
    return PHY_MEMORY_PIB().phyTransmitPower;
}


/*************************************************************************************//**
 * \brief   Assigns a new value to the PHY-PIB attribute phyTransmitPower.
 * \param[in]   newValue    New value of the specified PHY-PIB attribute.
 * \details
 *  Only six least significant bits are assigned with the new transmit power value. Two
 *  most significant bits contain transmit power tolerance and kept unchanged.
*****************************************************************************************/
INLINE void phyPibApiSetTransmitPower(const PHY_TransmitPower_t newValue)
{
    const PHY_TXPower_t  newTxPower = newValue.txPower;     /* New value of transmit power to be assigned. */

    SYS_DbgAssertLog(0 == newValue.tolerance, LOG_phyPibApiSetTransmitPower_NonemptyTransmitPowerTolerance);
    SYS_DbgAssertComplex(newTxPower >= PHY_ATTR_MINALLOWED_VALUE_TRANSMIT_POWER_VALUE,
            LOG_phyPibApiSetTransmitPower_NewValueTooLow);
    SYS_DbgAssertComplex(newTxPower <= PHY_ATTR_MAXALLOWED_VALUE_TRANSMIT_POWER_VALUE,
            LOG_phyPibApiSetTransmitPower_NewValueTooHigh);

    ATOMIC_SECTION_ENTER(ATM_phyPibApiSetTransmitPower)
        PHY_MEMORY_PIB().phyTransmitPower.txPower = newTxPower;
        HAL_Radio__TX_POWER_set(newTxPower);
    ATOMIC_SECTION_LEAVE(ATM_phyPibApiSetTransmitPower)
}


/*************************************************************************************//**
 * \brief   Returns value of the PHY-PIB attribute phyCcaMode.
 * \return  Value of the requested PHY-PIB attribute.
*****************************************************************************************/
INLINE enum PHY_CCAMode_t phyPibApiGetCcaMode(void)
{
    return PHY_MEMORY_PIB().phyCcaMode;
}


/*************************************************************************************//**
 * \brief   Assigns a new value to the PHY-PIB attribute phyCcaMode.
 * \param[in]   newValue    New value of the specified PHY-PIB attribute.
*****************************************************************************************/
INLINE void phyPibApiSetCcaMode(const enum PHY_CCAMode_t newValue)
{
    const enum PHY_CCAMode_t  newCcaMode = newValue;    /* New value of CCA mode to be assigned. */

#if (PHY_ATTR_MINALLOWED_VALUE_CCA_MODE > 0)
    SYS_DbgAssertComplex(newCcaMode >= PHY_ATTR_MINALLOWED_VALUE_CCA_MODE, LOG_phyPibApiSetCcaMode_InvalidTooLow);
#endif
    SYS_DbgAssertComplex(newCcaMode <= PHY_ATTR_MAXALLOWED_VALUE_CCA_MODE, LOG_phyPibApiSetCcaMode_InvalidTooHigh);

    ATOMIC_SECTION_ENTER(ATM_phyPibApiSetCcaMode)
        PHY_MEMORY_PIB().phyCcaMode = newCcaMode;
        HAL_Radio__CCA_MODE_set(newCcaMode);
    ATOMIC_SECTION_LEAVE(ATM_phyPibApiSetCcaMode)
}


/*************************************************************************************//**
 * \brief   Returns value of the PHY-PIB attribute phyCurrentPage.
 * \return  Value of the requested PHY-PIB attribute.
*****************************************************************************************/
INLINE PHY_Page_t phyPibApiGetCurrentPage(void)
{
    return PHY__Take_Page(PHY_MEMORY_PIB().phyCurrentChannelOnPage);
}


/*************************************************************************************//**
 * \brief   Assigns new values to PHY-PIB attributes phyCurrentChannel and phyCurrentPage.
 * \param[in]   newChannelOnPage    Plain value of Channel-on-Page object.
*****************************************************************************************/
INLINE void phyPibApiSetCurrentChannelOnPage(const PHY_PageChannel_t newChannelOnPage)
{
    SYS_DbgAssertComplex(FALSE != phyPibApiFindChannelOnPage(newChannelOnPage),
            LOG_phyPibApiSetCurrentChannelOnPage_ChannelOnPageNotImplemented);

    ATOMIC_SECTION_ENTER(ATM_phyPibApiSetCurrentChannelOnPage)
        PHY_MEMORY_PIB().phyCurrentChannelOnPage = newChannelOnPage;
        HAL_Radio__CHANNEL_req(newChannelOnPage);
    ATOMIC_SECTION_LEAVE(ATM_phyPibApiSetCurrentChannelOnPage)
}


/*************************************************************************************//**
 * \brief   Returns value of the PHY-PIB attribute phyMaxFrameDuration, in symbols.
 * \return  Value of the requested PHY-PIB attribute.
*****************************************************************************************/
INLINE PHY_MaxFrameDuration_t phyPibApiGetMaxFrameDuration(void)
{
    /* Return value according to the selected band and modulation with the formula:
     *   phySHRDuration + CEIL(((aMaxPHYPacketSize + 1) * phySymbolsPerOctetX10) / 10).
     *
     * It gives the following values for different bands and modulations:
     *   868 BPSK       1064 symbols = 40 + RoundUp((127 + 1) * 8)
     *   915 BPSK       1064 symbols = 40 + RoundUp((127 + 1) * 8)
     *   868 ASK          55 symbols =  3 + RoundUp((127 + 1) * 0.4)
     *   915 ASK         212 symbols =  7 + RoundUp((127 + 1) * 1.6)
     *   868 O-QPSK      266 symbols = 10 + RoundUp((127 + 1) * 2)
     *   915 O-QPSK      266 symbols = 10 + RoundUp((127 + 1) * 2)
     *  2450 O-QPSK      266 symbols = 10 + RoundUp((127 + 1) * 2)
     */
    return 10 + CEIL((PHY_aMaxPHYPacketSize + 1) * 20, 10);         /* TODO: Implement table of constants. This one is just for 2.45 GHz. */
}


/*************************************************************************************//**
 * \brief   Returns value of the PHY-PIB attribute phyShrDuration, in symbols.
 * \return  Value of the requested PHY-PIB attribute.
*****************************************************************************************/
INLINE PHY_ShrDuration_t phyPibApiGetShrDuration(void)
{
    /* Return value according to the selected band and modulation with the formula:
     *   SHR-duration-in-octets * phySymbolsPerOctetX10 / 10.
     *
     * It gives the following values for different bands and modulations:
     *   868 BPSK       40 symbols = 5 * 80 / 10
     *   915 BPSK       40 symbols = 5 * 80 / 10
     *   868 ASK         3 symbols
     *   915 ASK         7 symbols
     *   868 O-QPSK     10 symbols = 5 * 20 / 10
     *   915 O-QPSK     10 symbols = 5 * 20 / 10
     *  2450 O-QPSK     10 symbols = 5 * 20 / 10
     */
    return (4 + 1) * 20 / 10;       /* TODO: Implement table of constants. This one is just for 2.45 GHz. */
}


/*************************************************************************************//**
 * \brief   Returns value of the PHY-PIB attribute phySymbolsPerOctet multiplied by 10.
 * \return  Value of the requested PHY-PIB attribute multiplied by 10.
 * \note
 *  The phySymbolsPerOctet attribute may have non-integer value of 0.4 and 1.6. In order
 *  to avoid using of float-point data types this argument is stored and returned
 *  multiplied by 10.
*****************************************************************************************/
INLINE PHY_SymbolsPerOctetX10_t phyPibApiGetSymbolsPerOctetX10(void)
{
    /* Return value according to the selected band and modulation.
     * Values for different bands and modulations:
     *   868 BPSK       80 = 8.0 symbols per 1 octet
     *   915 BPSK       80 = 8.0 symbols per 1 octet
     *   868 ASK         4 = 0.4 symbols per 1 octet
     *   915 ASK        16 = 1.6 symbols per 1 octet
     *   868 O-QPSK     20 = 2.0 symbols per 1 octet
     *   915 O-QPSK     20 = 2.0 symbols per 1 octet
     *  2450 O-QPSK     20 = 2.0 symbols per 1 octet
     */
    return 2 * 10;      /* TODO: Implement table of constants. This one is just for 2.45 GHz. */
}


#if defined(RF4CE_TARGET)
/*************************************************************************************//**
 * \brief   Returns value of the PHY-PIB attribute phyRssi.
 * \return  Value of the requested PHY-PIB attribute.
*****************************************************************************************/
INLINE PHY_RSSI_t phyPibApiGetRssi(void)
{
    return HAL_Radio__RSSI_get();
}
#endif /* RF4CE_TARGET */


#endif /* _BB_PHY_PIB_API_H */
