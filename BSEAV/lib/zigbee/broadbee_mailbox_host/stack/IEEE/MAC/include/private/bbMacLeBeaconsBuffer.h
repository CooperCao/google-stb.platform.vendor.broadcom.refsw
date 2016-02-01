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
* FILENAME: $Workfile$
*
* DESCRIPTION:
*   MAC-LE Beacons Buffer interface.
*
* $Revision$
* $Date$
*
*****************************************************************************************/


#ifndef _BB_MAC_LE_BEACONS_BUFFER_H
#define _BB_MAC_LE_BEACONS_BUFFER_H


/************************* INCLUDES *****************************************************/
#include "private/bbMacMpdu.h"      /* MAC MPDU definitions. */
#include "bbHalRadio.h"             /* Hardware Radio interface. */


/************************* VALIDATIONS **************************************************/
#if defined(_MAC_CONTEXT_RF4CE_CONTROLLER_)
# error This header is not for the RF4CE-Controller build.
#endif


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Maximum size of PPDU of a single beacon frame except the MFR.FCS field, in
 *  32-bit words.
 * \details Single simple beacon (i.e. beacon without payload and with simple 4-byte
 *  superframe descriptor) has PPDU of size of 14 or 20 octets including:
 *  - 1 octet           PHR field,
 *  - 2 octets          MHR.FCF field (Frame Control Field),
 *  - 1 octet           MHR.BSN field (Beacon Sequense Number),
 *  - 2 octets          MHR.SrcPANId field (Source PAN Id),
 *  - 2 or 8 octets     MHR.SrcAddress field (Source 16-bit or 64-bit address),
 *  - 2 octets          MSDU.SuperframeDescriptor field,
 *  - 1 octet           MSDU.GTSSpecification field,
 *  - 1 octet           MSDU.PandingAddressSpecification field,
 *  - 2 octets          MFR.FCS field (Frame Control Sequence).
 *
 * \details The received beacon frame PPDU is stored in the buffer except the trailing
 *  MFR.FCS field of size 2 octets.
 */
#define MAC_LE_BEACONS_BUFFER_SINGLE_PPDU_SIZE_WORDS\
        HAL_RADIO_CONVERT_BYTES_TO_WORDS(\
                MAC_PPDU_PHR_SIZE +\
                MAC_MPDU_MHR_FCF_SIZE +\
                MAC_MPDU_MHR_BSN_SIZE +\
                MAC_MPDU_MHR_PANID_SIZE +\
                MAC_MPDU_MHR_EXTADDR_SIZE +\
                MAC_MPDU_MSDU_SUPERFRAMESPEC_SIZE +\
                MAC_MPDU_MSDU_GTSSPEC_SIZE +\
                MAC_MPDU_MSDU_PENDINGADDRSPEC_SIZE)

/*
 * Validate the maximum size of PPDU of a single beacon frame except the MFR.FCS field, in
 * 32-bit words.
 */
SYS_DbgAssertStatic(5 == MAC_LE_BEACONS_BUFFER_SINGLE_PPDU_SIZE_WORDS);


/**//**
 * \brief   Structure for a single record in the MAC-LE Beacons Buffer.
 * \details The MAC-LE Buffer stores the following information for each received beacon:
 *  - PPDU of the received beacon except the trailing MFR.FCS field,
 *  - RX-Start timestamp when the beacon reception was started,
 *  - Channel-on-Page at which beacon was received,
 *  - LQI value of the received beacon packet.
 */
typedef struct _MacLeBeaconsBufferRecord_t
{
    uint32_t               ppdu[MAC_LE_BEACONS_BUFFER_SINGLE_PPDU_SIZE_WORDS];
                                                /*!< PPDU of the received beacon except the trailing MFR.FCS field. */

    HAL_SymbolTimestamp_t  timestamp;           /*!< Beacon reception start timestamp. */

    PHY_ChannelOnPage_t    channelOnPage;       /*!< Channel Page and Logical Channel on which beacon was received. */

    PHY_Lqi_t              lqi;                 /*!< LQI value of the received beacon packet. */

} MacLeBeaconsBufferRecord_t;


/**//**
 * \brief   Enumeration of results returned by the beacon extracting function on if the
 *  MAC-LE Beacons Buffer is finally (after extraction) empty or not.
 */
typedef enum _MacLeBeaconsBufferIsNotEmpty_t
{
    MAC_LE_BEACONS_BUFFER_EMPTY     = FALSE,    /*!< The MAC-LE Beacons Buffer is finally empty. */

    MAC_LE_BEACONS_BUFFER_NOT_EMPTY = TRUE,     /*!< The MAC-LE Beacons Buffer still contains at least one beacon. */

} MacLeBeaconsBufferIsNotEmpty_t;


/************************* PROTOTYPES ***************************************************/
/*************************************************************************************//**
 * \brief   Puts a new beacon into the MAC-LE Beacons Buffer.
 * \details
 *  This function is called by the MAC-LE Real-Time Dispatcher when a new beacon is
 *  received. This function shall be called immediately following the beacon frame
 *  filtering in the same real-time interrupt from the Radio hardware.
*****************************************************************************************/
MAC_PRIVATE void macLeBeaconsBufferPut(void);


/*************************************************************************************//**
 * \brief   Extracts the oldest (first received) beacon from the MAC-LE Beacons Buffer.
 * \param[out]  record      Pointer to the record object provided by the caller to be
 *  filled with the recorded information about the extracted beacon.
 * \return  TRUE if the MAC-LE Beacons Buffer contains at least one beacon after the
 *  oldest beacon extraction; FALSE otherwise.
 * \details
 *  This function is called by the MAC-FE Task-Time Dispatcher when processing the BUFFER
 *  task that was scheduled by the MAC-LE Real-Time Dispatcher on saving a new beacon to
 *  the MAC-LE Beacons Buffer.
*****************************************************************************************/
MAC_PRIVATE MacLeBeaconsBufferIsNotEmpty_t macLeBeaconsBufferGet(MacLeBeaconsBufferRecord_t *const record);


#endif _BB_MAC_LE_BEACONS_BUFFER_H