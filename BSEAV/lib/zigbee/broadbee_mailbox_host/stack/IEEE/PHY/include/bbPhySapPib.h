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
* FILENAME: $Workfile: trunk/stack/IEEE/PHY/include/bbPhySapPib.h $
*
* DESCRIPTION:
*   PHY-PIB for PHY-SAP definitions.
*
* $Revision: 10530 $
* $Date: 2016-03-18 00:29:29Z $
*
*****************************************************************************************/


#ifndef _BB_PHY_SAP_PIB_H
#define _BB_PHY_SAP_PIB_H


/************************* INCLUDES *****************************************************/
#include "bbPhyBasics.h"


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Enumeration of identifiers of private PHY-PIB attributes.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 6.4.2, and table 23.
 */
typedef enum _PhyPibAttributeId_t
{
    PHY_ATTRIBUTES_ID_BEGIN = 0x00,     /*!< First ID of PHY-PIB attributes. */

    PHY_CURRENT_CHANNEL     = PHY_ATTRIBUTES_ID_BEGIN,
                                        /*!< The RF channel to use for all following transmissions and receptions. */

    PHY_CHANNELS_SUPPORTED  = 0x01,     /*!< The array of bit-fields of supported pages and channels. */

    PHY_TRANSMIT_POWER      = 0x02,     /*!< The transmission power and tolerance. */

    PHY_CCA_MODE            = 0x03,     /*!< The CCA mode. */

    PHY_CURRENT_PAGE        = 0x04,     /*!< The current PHY channel page. */

    PHY_MAX_FRAME_DURATION  = 0x05,     /*!< The maximum number of symbols in a frame. */

    PHY_SHR_DURATION        = 0x06,     /*!< The duration of the synchronization header (SHR)
                                            in symbols for the current PHY. */

    PHY_SYMBOLS_PER_OCTET   = 0x07,     /*!< The number of symbols per octet for the current PHY. The value of this
                                            attribute is stored and returned been multiplied by 10. */

#if defined(RF4CE_TARGET)
    PHY_RSSI                = 0x08,     /*!< The current RSSI value measured each 2 us. */

    PHY_ATTRIBUTES_ID_END   = PHY_RSSI,                 /*!< Last ID of PHY-PIB private attributes. */
#else
    PHY_ATTRIBUTES_ID_END   = PHY_SYMBOLS_PER_OCTET,    /*!< Last ID of PHY-PIB private attributes. */
#endif

} PhyPibAttributeId_t;


/**//**
 * \brief   Enumeration of identifiers of public PHY-PIB attributes.
 * \details PHY-PIB public attributes set includes only PHY-PIB private attributes subset;
 *  there are no other attributes subsets below the PHY-PIB.
 * \note    Enumeration \c PHY_PibAttributeId_t has the same data size (8-bit) as
 *  \c PhyPibAttributeId_t.
 */
typedef PhyPibAttributeId_t  PHY_PibAttributeId_t;
SYS_DbgAssertStatic(sizeof(PHY_PibAttributeId_t) == sizeof(uint8_t));


/*
 * Data types for private PHY-PIB attributes.
 */
typedef SYS_DataPointer_t       PHY_ChannelsSupported_t;    /*!< Data type for phyChannelsSupported. */
/* Defined here below:          PHY_TransmitPower_t;           < Data type for phyTransmitPower. */
typedef uint16_t                PHY_MaxFrameDuration_t;     /*!< Data type for phyMaxFrameDuration. */
typedef uint8_t                 PHY_ShrDuration_t;          /*!< Data type for phyShrDuration. */
typedef uint8_t                 PHY_SymbolsPerOctetX10_t;   /*!< Data type for phySymbolsPerOctet times 10. */

/**//**
 * \brief   Data type for a single row of phyChannelsSupported.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclauses 6.1.2, 6.4.2, tables 2, 23.
 */
typedef struct _PHY_ChannelsSupportedOnPage_t
{
    PHY_ChannelMask_t       validChannels : PHY_CHANNELS_NUM;       /*!< Status for each of up to 27 channels
                                                                        supported by this channel page. */

    PHY_Page_t              channelPage   : 5;                      /*!< Identifier of the channel page. */

} PHY_ChannelsSupportedOnPage_t;


/**//**
 * \brief   Evaluates the number of channel pages described in the phyChannelsSupported
 *  attribute.
 * \param[in]   channelsSupported   Pointer to the phyChannelsSupported attribute data
 *  structure.
 * \return  The number of channel pages returned in the \c phyChannelsSupported attribute.
 */
#define PHY_CHANNELS_SUPPORTED_LIST_SIZE(channelsSupported)\
        (SYS_GetPayloadSize(channelsSupported) / sizeof(PHY_ChannelsSupportedOnPage_t))


/**//**
 * \brief   Extracts structured object describing the page identifier and the set of
 *  implemented channels on that page from the phyChannelsSupported array-type PHY-PIB
 *  attribute according to the page descriptor index.
 * \param[out]  channelsSupportedOnPage     Pointer to the structured object describing
 *  the page identifier and the set of implemented channels on that page.
 * \param[in]   phyChannelsSupported        Pointer to the array-type PHY-PIB attribute
 *  phyChannelsSupported.
 * \param[in]   pageDescrIdx                Index of the requested page descriptor in the
 *  phyChannelsSupported array.
 * \note
 *  The \p pageDescrIdx denotes not the channel page identifier, but the index of a page
 *  descriptor in the \p phyChannelsSupported array.
 */
#define PHY_CHANNELS_SUPPORTED_ON_PAGE(channelsSupportedOnPage,\
                                       phyChannelsSupported,\
                                       pageDescrIdx)\
        SYS_CopyFromPayload((channelsSupportedOnPage),\
                            (phyChannelsSupported),\
                            sizeof(PHY_ChannelsSupportedOnPage_t) * (pageDescrIdx),\
                            sizeof(PHY_ChannelsSupportedOnPage_t))


/**//**
 * \brief   Macro used to obtain the IEEE-Code-of-Tolerance from the numeric value of
 *  Transmit Power Tolerance, measured in dBm.
 * \param[in]   tolDbm      Value of tolerance, in dBm.
 * \return  IEEE-Code of the tolerance value.
 */
#define PHY_TX_POWER_TOLERANCE_DBM_TO_CODE(tolDbm)  ((tolDbm) / 3)


/**//**
 * \brief   Enumeration of values for the PHY nominal transmit power tolerance.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 6.4.2, table 23.
 */
typedef enum _PHY_TxPowerTolerance_t
{
    PHY_TX_POWER_TOLERANCE_1DBM = PHY_TX_POWER_TOLERANCE_DBM_TO_CODE(1),        /*!< The tolerance is ±1 dBm. */

    PHY_TX_POWER_TOLERANCE_3DBM = PHY_TX_POWER_TOLERANCE_DBM_TO_CODE(3),        /*!< The tolerance is ±3 dBm. */

    PHY_TX_POWER_TOLERANCE_6DBM = PHY_TX_POWER_TOLERANCE_DBM_TO_CODE(6),        /*!< The tolerance is ±6 dBm. */

} PHY_TxPowerTolerance_t;


/**//**
 * \brief   Data type for phyTransmitPower.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 6.4.2, table 23.
 */
typedef struct _PHY_TransmitPower_t
{
    PHY_TXPower_t           txPower   : 6;          /*!< The nominal transmit power of the device
                                                        in decibels relative to 1 mW. */

    PHY_TxPowerTolerance_t  tolerance : 2;          /*!< Tolerance on the transmit power. */

} PHY_TransmitPower_t;


/**//**
 * \brief   Union of all private PHY-PIB attributes.
 * \note    Attribute phyChannelsSupported is not included into the union because it is
 *  transferred as a payload but not by its value.
 */
#define PHY_PIB_PRIVATE_VARIANT\
    union\
    {\
        PHY_Channel_t             phyCurrentChannel;\
        PHY_TransmitPower_t       phyTransmitPower;\
        enum PHY_CCAMode_t        phyCcaMode;\
        PHY_Page_t                phyCurrentPage;\
        PHY_MaxFrameDuration_t    phyMaxFrameDuration;\
        PHY_ShrDuration_t         phyShrDuration;\
        PHY_SymbolsPerOctetX10_t  phySymbolsPerOctetX10;\
        PHY_RSSI_t                phyRssi;\
    }


/**//**
 * \brief   Variant data type for private PHY-PIB attributes.
 */
typedef PHY_PIB_PRIVATE_VARIANT  PhyPibAttributeValue_t;


/**//**
 * \brief   Union of all public PHY-PIB attributes.
 * \details PHY-PIB public attributes set includes only PHY-PIB private attributes subset;
 *  there are no other attributes subsets below the PHY-PIB.
 * \details Use this macro to define one-step-higher-layer public variant data type.
 * \par     Example of usage
 * \code
 *  #define MAC_PIB_PUBLIC_VARIANT\
 *      union\
 *      {\
 *          PHY_PIB_PUBLIC_VARIANT;\
 *          MAC_PIB_PRIVATE_VARIANT;\
 *      }
 * \endcode
 */
#define PHY_PIB_PUBLIC_VARIANT\
    union\
    {\
        PHY_PIB_PRIVATE_VARIANT;\
    }


/**//**
 * \brief   Variant data type for public PHY-PIB attributes.
 * \details PHY-PIB public attributes set includes only PHY-PIB private attributes subset;
 *  there are no other attributes subsets below the PHY-PIB.
 */
typedef PHY_PIB_PUBLIC_VARIANT  PHY_PibAttributeValue_t;


#endif /* _BB_PHY_SAP_PIB_H */
