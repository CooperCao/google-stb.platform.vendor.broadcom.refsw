/******************************************************************************
* (c) 2014 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its
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
******************************************************************************/
/*****************************************************************************
*
* FILENAME: $Workfile: trunk/stack/IEEE/PHY/include/bbPhySapDefs.h $
*
* DESCRIPTION:
*   PHY-SAP common definitions.
*
* $Revision: 3495 $
* $Date: 2014-09-09 00:00:15Z $
*
*****************************************************************************************/


#ifndef _BB_PHY_SAP_DEFS_H
#define _BB_PHY_SAP_DEFS_H


/************************* INCLUDES *****************************************************/
#include "bbPhyBasics.h"            /* Basic PHY set. */


/************************* DEFINITIONS **************************************************/
/*
 * PHY constants.
 */
#define PHY_MAX_PACKET_SIZE     127     /*!< The maximum PSDU size (in octets) the PHY shall be able to receive. */

#define PHY_TURNAROUND_TIME     12      /*!< RX-to-TX or TX-to-RX maximum turnaround time (in symbol periods). */


/**//**
 * \brief   Consolidated enumerations for the PHY-SAP.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 6.2.3, table 18.
 */
typedef enum _PHY_Enumerations_t
{
    PHY_UNDEFINED             = 0x00,       /*!< Nonstandard status returned if the PHY is in the undefined state. */

    PHY_BUSY                  = 0x00,       /*!< The CCA attempt has detected a busy channel. */

    PHY_BUSY_RX               = 0x01,       /*!< The transceiver is asked to change its state while receiving. */

    PHY_BUSY_TX               = 0x02,       /*!< The transceiver is asked to change its state while transmitting. */

    PHY_FORCE_TRX_OFF         = 0x03,       /*!< The transceiver is to be switched off immediately. */

    PHY_IDLE                  = 0x04,       /*!< The CCA attempt has detected an idle channel. */

    PHY_INVALID_PARAMETER     = 0x05,       /*!< A SET/GET request was issued with a parameter in the primitive that is
                                                out of the valid range. */

    PHY_RX_ON                 = 0x06,       /*!< The transceiver is in or is to be configured into the receiver enabled
                                                state. */

    PHY_SUCCESS               = 0x07,       /*!< A SET/GET, an ED operation, or a transceiver state change was
                                                successful. */

    PHY_TRX_OFF               = 0x08,       /*!< The transceiver is in or is to be configured into the transceiver
                                                disabled state. */

    PHY_TX_ON                 = 0x09,       /*!< The transceiver is in or is to be configured into the transmitter
                                                enabled state. */

    PHY_UNSUPPORTED_ATTRIBUTE = 0x0A,       /*!< A SET/GET request was issued with the identifier of an attribute that
                                                is not supported. */

    PHY_READ_ONLY             = 0x0B,       /*!< A SET/GET request was issued with the identifier of an attribute that
                                                is read-only. */
} PHY_Enumerations_t;


/**//**
 * \brief   Data type for the \c status parameter returned by the PHY-SAP confirmation
 *  primitives.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclauses 6.2.1.2.1, 6.2.2.2.1, 6.2.2.4.1, 6.2.2.6.1,
 *  6.2.2.8.1, 6.2.2.10.1, tables 7, 10, 11, 13, 15, 17.
 */
typedef PHY_Enumerations_t  PHY_Status_t;


/**//**
 * \brief   Data type for the PHY radio hardware logical channel identifier.
 * \details The valid range is from 0 to 26.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 6.1.2.1.
 */
typedef uint8_t  PHY_LogicalChannelId_t;


/**//**
 * \brief   The maximum number of channel in single channel page.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 6.1.2.1.
 */
#define PHY_MAX_NUMBER_OF_LOGICAL_CHANNELS_ON_PAGE  27


/**//**
 * \brief   Data type for the set of PHY channels on a single channel page.
 * \details The set is restricted on bits from #0 to #26.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclauses 6.1.2, tables 2.
 */
typedef BitField32_t  PHY_ChannelsSet_t;


/**//**
 * \brief   The mask of bits allowed for channels representing in a single page
 *  descriptor.
 * \details The set is restricted on bits from #0 to #26.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclauses 6.1.2, table 2.
 */
#define PHY_MASK_OF_LOGICAL_CHANNELS_ON_PAGE    BIT_MASK(PHY_MAX_NUMBER_OF_LOGICAL_CHANNELS_ON_PAGE)


/**//**
 * \brief   Enumeration of codes for PHY radio hardware channel pages.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 6.1.2.2, table 2.
 */
typedef enum _PHY_ChannelPageId_t
{
    PHY_CHANNEL_PAGE_0   = 0,       /*!< Channel page #0: 2.4 GHz O-QPSK, 915/868 MHz BPSK. */

    PHY_CHANNEL_PAGE_1   = 1,       /*!< Channel page #1: 915/868 MHz band using ASK. */

    PHY_CHANNEL_PAGE_2   = 2,       /*!< Channel page #2: 915/868 MHz band using O-QPSK. */

    PHY_CHANNEL_PAGE_MAX = 31,      /*!< The maximum value for Channel Page identifier. */

} PHY_ChannelPageId_t;


/**//**
 * \brief   Data type for the plain value of the Channel-on-Page structured object.
 * \details Value of this type has 16-bit width. The MSB contains the channel page
 *  identifier; and the LSB contains the logical channel identifier.
 */
/* TODO: Rename to PHY_ChannelOnPage_t. */
typedef uint16_t  PHY_ChannelOnPagePlain_t;


/**//**
 * \brief   Macro to assemble Channel-on-Page object from the set of subfields.
 * \param[in]   page        Identifier of the channel page.
 * \param[in]   channel     Identifier of the channel.
 * \return  Channel-on-Page object plain value.
 */
#define PHY_MAKE_CHANNEL_ON_PAGE(page, channel)     ((((PHY_ChannelOnPagePlain_t)(page)) << 8) | (channel))


/**//**
 * \brief   Macro to extract the logical channel identifier from a Channel-on-Page object.
 * \param[in]   channelOnPage   Plain value of a Channel-on-Page object.
 * \return  Logical channel identifier.
 */
#define PHY_EXTRACT_CHANNEL_ID(channelOnPage)       ((PHY_LogicalChannelId_t)(channelOnPage))


/**//**
 * \brief   Macro to extract the channel page identifier from a Channel-on-Page object.
 * \param[in]   channelOnPage   Plain value of a Channel-on-Page object.
 * \return  Channel page identifier.
 */
#define PHY_EXTRACT_PAGE_ID(channelOnPage)          ((PHY_ChannelPageId_t)((channelOnPage) >> 8))


/**//**
 * \brief   Structure for the pair of Channel Page and Logical Channel identifiers.
 */
/* TODO: Replace with PHY_ChannelOnPagePlain_t. */
typedef union _PHY_ChannelOnPage_t
{
    PHY_ChannelOnPagePlain_t    plain;              /*!< Plain data. */

    struct
    {
        PHY_LogicalChannelId_t  logicalChannel;     /*!< (LSB) Logical Channel identifier. */

        PHY_ChannelPageId_t     channelPage;        /*!< (MSB) Channel Page identifier. */
    };
} PHY_ChannelOnPage_t;


#endif /* _BB_PHY_SAP_DEFS_H */