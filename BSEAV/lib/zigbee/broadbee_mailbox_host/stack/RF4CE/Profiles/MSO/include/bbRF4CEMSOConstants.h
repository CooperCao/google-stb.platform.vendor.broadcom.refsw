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
 * FILENAME: $Workfile: trunk/stack/RF4CE/Profiles/MSO/include/bbRF4CEMSOConstants.h $
 *
 * DESCRIPTION:
 *   This is the header file for the RF4CE MSO Profile Constants.
 *
 * $Revision: 2579 $
 * $Date: 2014-06-02 07:16:01Z $
 *
 ****************************************************************************************/
#ifndef _RF4CE_MSO_CONSTANTS_H
#define _RF4CE_MSO_CONSTANTS_H

/************************* DEFINITIONS *************************************************/
/**//**
 * \brief The MSO profile ID.
 */
#define RF4CE_MSO_PROFILE_ID            0xc0

/**//**
 * \brief Supported MSO command codes.
 */
#define MSO_CONTROL_PRESSED             0x01
#define MSO_CONTROL_REPEATED            0x02
#define MSO_CONTROL_RELEASED            0x03
#define MSO_CHECK_VALIDATION_REQUEST    0x20
#define MSO_CHECK_VALIDATION_RESPONSE   0x21
#define MSO_SET_ATTRIBUTE_REQUEST       0x22
#define MSO_SET_ATTRIBUTE_RESPONSE      0x23
#define MSO_GET_ATTRIBUTE_REQUEST       0x24
#define MSO_GET_ATTRIBUTE_RESPONSE      0x25

/**//**
 * \brief RF4CE MSO Profile Constants.
 */
#define MSO_APLC_MAX_KEY_REPEAT_INTERVAL            120 /*!< The maximum time in milliseconds between consecutive user
                                                             control repeated command frame transmission. */
#define MSO_APLC_MAX_RIB_ATTRIBUTE_SIZE             92  /*!< The maximum size in octets of the elements of the
                                                             attributes in the RIB. At the same time, the maximum size
                                                             in octets of the Value field in the set attribute
                                                             request and get attribute response command frames. */
#define MSO_APLC_BLACKOUT_TIME                      100 /*!< The time in milliseconds at the start of the validation
                                                             procedure during which packets SHALL NOT be transmitted. */
#define MSO_APLC_MIN_KEY_EXCHANGE_TRANSFER_COUNT    3   /*!< The minimum value of the KeyExTransferCount parameter
                                                             passed to the pair request primitive during the push button
                                                             pairing procedure. */
#define MSO_APLC_MAX_USER_STRING_LENGTH             9   /*!< The maximum allowed user string length. The rest is used
                                                             as a special data extension. */
#define MSO_APLC_RESPONSE_IDLE_TIME                 50  /*!< The timeout before transmission of the Check Validation Response. */

#define MSO_APLC_VENDOR_ID                          0x109D

/**//**
 * \brief RF4CE MSO maximum number of data requests supported.
 */
#define RF4CE_MSO_MAX_DATA_REQUESTS                 5

/**//**
 * \brief RF4CE MSO discovery duration.
 */
#define RF4CE_MSO_DISCOVERY_DURATION                0x186A

#define RF4CE_MSO_CLIENT_USER_STRING \
{ \
    'T', 'H', 'E', ' ', 'M', 'S', 'O', 0, 0, 0, \
    0, 0, 0, 0, \
    0x01 \
}

#define RF4CE_MSO_TARGET_USER_STRING \
{ \
    'T', 'H', 'E', ' ', 'M', 'S', 'O', 0, 0, 0, \
    0, 0, 0xBF, \
    0, \
    0 \
}

#endif /* _RF4CE_MSO_CONSTANTS_H */