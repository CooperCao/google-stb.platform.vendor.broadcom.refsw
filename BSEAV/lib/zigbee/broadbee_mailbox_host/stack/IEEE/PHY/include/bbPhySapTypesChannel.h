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
 *      PLME-SET-CHANNEL service data types definition.
 *
*******************************************************************************/

#ifndef _BB_PHY_SAP_TYPES_CHANNEL_H
#define _BB_PHY_SAP_TYPES_CHANNEL_H

/************************* INCLUDES ***********************************************************************************/
#include "bbPhyBasics.h"

/************************* DEFINITIONS ********************************************************************************/
/**//**
 * \name    Data types used for PHY channel page and current channel representation.
 * \details A total of 32 channel pages are available with channel pages 3 to 31 being reserved for future use. A total
 *  of 27 channels numbered 0 to 26 are available per channel page.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclauses 6.1.2.1, 6.1.2.2, table 2.
 */
/**@{*/
/**//**
 * \brief   Data type used for packed representation of the couple of channel page and current channel identifiers.
 * \details This data type represents particular channel on particular channel page in the following format:
 *  - bits 4..0 - Channel[4..0] - indicate the channel in the range from 0 to 26
 *  - bits 7..5 - Page[2..0] - indicate the channel page in the range from 0 to 7
 */
typedef HAL_Radio__PgCh_t  PHY_PageChannel_t;
SYS_DbgAssertStatic(sizeof(PHY_PageChannel_t) == 1);

/**//**
 * \brief   Data type for the PHY channel page identifier.
 */
typedef HAL_Radio__Page_t  PHY_Page_t;
SYS_DbgAssertStatic(sizeof(PHY_Page_t) == 1);

/**//**
 * \brief   Data type for the PHY current channel identifier.
 */
typedef HAL_Radio__Channel_t  PHY_Channel_t;
SYS_DbgAssertStatic(sizeof(PHY_Channel_t) == 1);

/**//**
 * \brief   Data type for PHY channel mask (set, bitmap).
 * \details This data type represents the bitmap of channels available on particular channel page in the following
 *  format:
 *  - bits 26..0 - ChannelMask[26..0] - indicate the status: 1 - channel is available, 0 - channel is unavailable - for
 *      each of up to 27 valid channels (bit #k indicates the status of channel k) supported by a channel page
 *  - bits 31..27 - Page[4..0] - indicate the channel page in the range from 0 to 2
 *
 * \note    The channel mask may be given also without specifying the page in bits 31..27. In this case bitmask just
 *  specifies a set of channels (for example, for repeated task performed on such a set of channels).
 * \note    In the case of a single bit mask, the channel mask may be used for representing the positional code of a
 *  channel. The bit #k (that is the only bit set to one in the bitmap) will indicate the channel k.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclauses 6.1.2.1, 6.1.2.2, table 2, 23.
 */
typedef uint32_t  PHY_ChannelMask_t;
SYS_DbgAssertStatic(sizeof(PHY_ChannelMask_t) == 4);
/**@}*/

/*--------------------------------------------------------------------------------------------------------------------*/
/**//**
 * \name    Macro-constants used for PHY channel page and current channel enumeration and validation.
 * \details A total of 32 channel pages are available with channel pages 3 to 31 being reserved for future use. A total
 *  of 27 channels numbered 0 to 26 are available per channel page.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclauses 6.1.2.1, 6.1.2.2, table 2.
 */
/**@{*/
/**//**
 * \brief   The maximum actually possible identifier of a channel page.
 * \details A total of 32 channel pages are available with channel pages 3 to 31 being reserved for future use.
 * \note    Not all channel pages may be actually supported by particular Radio hardware.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 6.1.2.2, table 2.
 */
#define PHY_PAGE_MAX                (2)

/**//**
 * \brief   The total number of actually defined channel pages.
 * \details A total of 32 channel pages are available with channel pages 3 to 31 being reserved for future use.
 * \note    Not all channel pages may be actually supported by particular Radio hardware.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 6.1.2.2, table 2.
 */
#define PHY_PAGES_NUM               (PHY_PAGE_MAX + 1)

/**//**
 * \brief   The maximum possible identifier of a current channel.
 * \details A total of 27 channels numbered 0 to 26 are available per channel page.
 * \note    Not all channels may be defined on each of channel pages defined in the standard. And not all of the defined
 *  channels may be actually supported by particular Radio hardware.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 6.1.2.1, table 2.
 */
#define PHY_CHANNEL_MAX             (26)

/**//**
 * \brief   The maximum possible number of channels on a single channel page.
 * \details A total of 27 channels numbered 0 to 26 are available per channel page.
 * \note    Not all channels may be defined on each of channel pages defined in the standard. And not all of the defined
 *  channels may be actually supported by particular Radio hardware.
 * \note    This constant may also be used as the invalid channel identifier throughout the stack procedures.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 6.1.2.1, table 2.
 */
#define PHY_CHANNELS_NUM            (PHY_CHANNEL_MAX + 1)

/**//**
 * \brief   The complete mask of channels on a single channel page.
 * \details A total of 27 channels numbered 0 to 26 are available per channel page.
 * \note    Not all channels may be defined on each of channel pages defined in the standard. And not all of the defined
 *  channels may be actually supported by particular Radio hardware.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 6.1.2.1, table 2.
 */
#define PHY_LEGAL_CHANNELS          BIT_MASK(PHY_CHANNELS_NUM)
/**@}*/

/*--------------------------------------------------------------------------------------------------------------------*/
/**//**
 * \name    Macro-functions used for PHY channel page and current channel compact representation.
 * \details A total of 32 channel pages are available with channel pages 3 to 31 being reserved for future use. A total
 *  of 27 channels numbered 0 to 26 are available per channel page.
 * \details The packed page-channel data type represents particular channel on particular channel page in the following
 *  format:
 *  - bits 4..0 - Channel[4..0] - indicate the channel in the range from 0 to 26
 *  - bits 7..5 - Page[2..0] - indicate the channel page in the range from 0 to 7
 *
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclauses 6.1.2.1, 6.1.2.2, table 2.
 */
/**@{*/
/**//**
 * \brief   Composes packed PHY page-channel identifier from distinct channel page and current channel identifiers.
 * \param[in]   page        The page identifier, in the range from 0 to 7.
 * \param[in]   channel     The channel identifier, in the range from 0 to 26.
 * \return  The packed page-channel identifier.
 */
#define PHY__Make_PageChannel(page, channel)    (HAL_Radio__Make_PgCh(page, channel))
SYS_DbgAssertStatic(PHY__Make_PageChannel(/*page*/ 1, /*channel*/ 23) == 0x37);

/**//**
 * \brief   Extracts the PHY channel page identifier from the packed page-channel identifier.
 * \param[in]   pgch    The packed page-channel identifier.
 * \return  The extracted channel page identifier.
 */
#define PHY__Take_Page(pgch)        (HAL_Radio__Take_Page(pgch))
SYS_DbgAssertStatic(PHY__Take_Page(0x37) == 1);

/**//**
 * \brief   Extracts the PHY current channel identifier from the packed page-channel identifier.
 * \param[in]   pgch    The packed page-channel identifier.
 * \return  The extracted current channel identifier.
 */
#define PHY__Take_Channel(pgch)     (HAL_Radio__Take_Channel(pgch))
SYS_DbgAssertStatic(PHY__Take_Channel(0x37) == 23);

/**//**
 * \brief   Substitutes the page identifier with the new one in the given packed page-channel identifier and returns.
 * \param[in]   pgch    The packed page-channel identifier to be updated.
 * \param[in]   page    The new page identifier, in the range from 0 to 7.
 * \return  Updated packed page-channel identifier.
 * \details The current channel identifier is preserved, only the channel page identifier is substituted.
 */
#define PHY__Substitute_Page(pgch, page)\
        ((((PHY_PageChannel_t)(pgch)) & 0x1F) | (((PHY_PageChannel_t)(page)) << 5))
SYS_DbgAssertStatic(PHY__Substitute_Page(0x37, /*new page*/ 2) == 0x57);

/**//**
 * \brief   Substitutes the channel identifier with the new one in the given packed page-channel identifier and returns.
 * \param[in]   pgch        The packed page-channel identifier to be updated.
 * \param[in]   channel     The new channel identifier, in the range from 0 to 26.
 * \return  Updated packed page-channel identifier.
 * \details The channel page identifier is preserved, only the current channel identifier is substituted.
 */
#define PHY__Substitute_Channel(pgch, channel)\
        ((((PHY_PageChannel_t)(pgch)) & 0xE0) | (((PHY_PageChannel_t)(channel)) << 0))
SYS_DbgAssertStatic(PHY__Substitute_Channel(0x37, /*new channel*/ 10) == 0x2A);

/**//**
 * \brief   Updates the page identifier with the new one in the given packed page-channel identifier.
 * \param[in/out]   pgch    Reference to the packed page-channel identifier to be updated.
 * \param[in]       page    The new page identifier, in the range from 0 to 7.
 * \details The updated packed page-channel identifier is saved in the \p pgch.
 * \details The current channel identifier is preserved, only the channel page identifier is updated.
 */
#define PHY__Update_Page(pgch, page)\
        do { (pgch) = PHY__Substitute_Page(pgch, page); } while(0)

/**//**
 * \brief   Updates the channel identifier with the new one in the given packed page-channel identifier.
 * \param[in/out]   pgch        Reference to the packed page-channel identifier to be updated.
 * \param[in]       channel     The new channel identifier, in the range from 0 to 26.
 * \details The updated packed page-channel identifier is saved in the \p pgch.
 * \details The channel page identifier is preserved, only the current channel identifier is updated.
 */
#define PHY__Update_Channel(pgch, channel)\
        do { (pgch) = PHY__Substitute_Channel(pgch, channel); } while(0)
/**@}*/

#endif /* _BB_PHY_SAP_TYPES_CHANNEL_H */

/* eof bbPhySapTypesChannel.h */