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
 *      MLME-SCAN service data types definition.
 *
*******************************************************************************/

#ifndef _BB_MAC_SAP_TYPES_SCAN_H
#define _BB_MAC_SAP_TYPES_SCAN_H


/************************* INCLUDES *****************************************************/
#include "bbMacSapTypesBeacon.h"    /* MLME-BEACON service data types. */


/************************* VALIDATIONS **************************************************/
#if defined(_MAC_CONTEXT_RF4CE_CONTROLLER_) && !defined(MAILBOX_UNIT_TEST)
# error This header is not for the RF4CE-Controller build.
#endif


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Enumeration of MAC scanning types.
 * \note    Passive and Orphan scan types are not implemented.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.1.11.1, table 67.
 */
typedef enum
{
    MAC_SCAN_TYPE_ED      = 0x0,    /*!< Energy Detection scan. */
    MAC_SCAN_TYPE_ACTIVE  = 0x1,    /*!< Active scan. */
    MAC_SCAN_TYPE_PASSIVE = 0x2,    /*!< Passive scan. Not implemented. */
    MAC_SCAN_TYPE_ORPHAN  = 0x3,    /*!< Orphan scan. Not implemented. */
    MAC_SCAN_TYPE_MAX     = 0x3,    /*!< Maximum allowed value of Scan Type. */
} MAC_ScanType_t;


/**//**
 * \brief   Special value for the Scan Duration parameter to perform extremely fast single
 *  channel Energy Detection Scanning for purposes of the RF4CE Frequency Agility.
 */
#define MAC_SCAN_DURATION_FOR_RF4CE_FREQUENCY_AGILITY   0xFA


/**//**
 * \brief   Actual duration of the Energy Detection Scanning for the case of request for
 *  Frequency Agility, in whole symbols.
 * \details The value of 160 symbols is chosen according to the RF4CE NWK Certification
 *  Test Specification. The Noise Generator device will be active for 8 ms, then will keep
 *  silence for 2 ms, and then again active for 8 ms and so on. Consequently the ED
 *  scanning period of a single channel for needs of Frequency Agility Certification Test
 *  must not be shorter than 2 ms (indeed it is recommended to be at least 2.5 ms or
 *  slightly more). Note that ED scanning returns the MAXimum of all the ED values each
 *  measured over 8 symbols. If converted to 2.45 GHz O-QPSK symbol, the period of 2.5 ms
 *  equals to 156 symbols. So, the value 160 symbols is chosen.
 * \par     Documentation
 *  See ZigBee Document 094969r03ZB, subclause 10.3.
 */
#define MAC_SCAN_PERIOD_FOR_RF4CE_FREQUENCY_AGILITY     160


/**//**
 * \brief   Structure for parameters of MLME-SCAN.request primitive.
 * \ingroup ScanReq
 * \note    MAC Security is not implemented. The SecurityLevel parameter is left
 *  unassigned; or assigned with 0x00 'None' if _MAC_SAP_PROCESS_REDUNDANT_PARAMS_
 *  conditional build key is defined by the project make configuration file. All received
 *  secured frames are acknowledged but then dropped; in this case
 *  MLME-COMM-STATUS.indication is issued with the status UNSUPPORTED_LEGACY (except the
 *  RF4CE Controller for which a received secured frame is acknowledged and then dropped
 *  without any indication).
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.1.11.1, table 67.
 */
typedef struct _MAC_ScanReqParams_t
{
    /* 32-bit data. */
    PHY_ChannelMask_t    scanChannels;      /*!< Indicates which channels are to be scanned. */

    /* 8-bit data. */
    MAC_ScanType_t       scanType;          /*!< Indicates the type of scan performed. */

    MAC_BeaconOrder_t    scanDuration;      /*!< A value used to calculate the length of time to spend scanning each
                                                channel: from 0 to 14. Use 0xFA special value to perform extremely fast
                                                single channel Energy Detection Scanning for purposes of the RF4CE
                                                Frequency Agility. */

    PHY_Page_t           channelPage;       /*!< The channel page on which to perform the scan. */

    MAC_SecurityLevel_t  securityLevel;     /*!< The security level to be used. */

} MAC_ScanReqParams_t;


/**//**
 * \brief   The maximum value of ScanDuration parameter of MLME-SCAN.request.
 */
#define MAC_SCAN_DURATION_MAX   14


/**//**
 * \brief   Structure for parameters of MLME-SCAN.confirm primitive.
 * \ingroup ScanConf
 * \note    Fields \c EnergyDetectList and \c PANDescriptorList are implemented via
 *  \c payload. The value of field \c ResultListSize may be restored from the field
 *  \c payload with special macro-function \c MAC_SCAN_RESULT_LIST_SIZE().
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.1.11.2, table 68.
 */
typedef struct _MAC_ScanConfParams_t
{
    /* 32-bit data. */
    PHY_ChannelMask_t   unscannedChannels;      /*!< Indicates which channels given in the request were not scanned. */

    SYS_DataPointer_t   payload;                /*!< The list of energy measurements or the list of PAN descriptors. */

    /* 8-bit data. */
    MAC_Status_t        status;                 /*!< The status of the scan request. */

    MAC_ScanType_t      scanType;               /*!< Indicates the type of scan performed. */

    PHY_Page_t          channelPage;            /*!< The channel page on which the scan was performed. */

} MAC_ScanConfParams_t;


/**//**
 * \brief   Evaluates the field \c ResultListSize of parameters structure for
 *  MLME-SCAN.confirm primitive.
 * \param[in]   confParams  Pointer to the MLME-SCAN.confirm parameters object.
 * \return  The number of elements returned in the \c EnergyDetectList result list.
 * \note    This function is allowed for Energy Detection and Active scan types.
 */
#define MAC_SCAN_RESULT_LIST_SIZE(confParams)\
        (SYS_GetPayloadSize(&(confParams)->payload) / (\
         MAC_SCAN_TYPE_ED == (confParams)->scanType ?\
                 sizeof(PHY_ED_t) : sizeof(MAC_PanDescriptor_t)))


/**//**
 * \brief   The maximum number of \c ResultListSize for Energy Detection scan.
 */
#define MAC_SCAN_RESULT_LIST_SIZE_MAX_ED        (PHY_CHANNELS_NUM)


#if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_MAC_)
# if !defined(MAC_SCAN_RESULT_LIST_SIZE_MAX_ACTIVE)
/**//**
 * \brief   The maximum number of \c ResultListSize for Active scan.
 * \note    For purposes of the MAC Certification Test TP/154/MAC/SCANNING-06 the maximum
 *  number of PAN Descriptors returned in the MLME-SCAN.confirm may be reduced by defining
 *  this constant in the project make configuration file. If the project make
 *  configuration file does not define this constant, it will be assigned with the default
 *  value according to capabilities of the Stack.
 */
#  define MAC_SCAN_RESULT_LIST_SIZE_MAX_ACTIVE  10
# endif

# if defined(__arc__)
/* Enabled only for the ARC platform (not for the i386 platform) because of different rules of padding. */
SYS_DbgAssertStatic(MAC_SCAN_RESULT_LIST_SIZE_MAX_ACTIVE * sizeof(MAC_PanDescriptor_t) <=
                    /*MailBox maximum payload length*/ 255);
# endif
#endif /* WRAPPERS_ALL == _MAILBOX_WRAPPERS_MAC_ */


/**//**
 * \brief   Structure for descriptor of MLME-SCAN.request.
 * \ingroup ScanReq
 */
typedef struct _MAC_ScanReqDescr_t  MAC_ScanReqDescr_t;


/**//**
 * \brief   Template for callback handler-function of MLME-SCAN.confirm primitive.
 * \ingroup ScanConf
 * \param[in]   reqDescr    Pointer to the confirmed request descriptor.
 * \param[in]   confParams  Pointer to the confirmation parameters object.
 * \details Call functions of this type provided by higher layers of corresponding MAC
 *  contexts, ZigBee PRO and RF4CE, from the MAC to issue the MLME-SCAN.confirm to the
 *  ZigBee PRO higher layer or RF4CE higher layer.
 * \note    Treat the parameters structure pointed by the \p confParams and passed into
 *  the confirmation callback handler-function as it has been allocated in the program
 *  stack by the MAC before calling this callback handler and will be destroyed just after
 *  this callback returns. The payload pointed by \p confParams->payload is allocated by
 *  MAC in the dynamic memory and must be dismissed by the higher layer.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.1.11.2.
 */
typedef void MAC_ScanConfCallback_t(MAC_ScanReqDescr_t   *const reqDescr,
                                    MAC_ScanConfParams_t *const confParams);


/**//**
 * \brief   Structure for descriptor of MLME-SCAN.request.
 * \ingroup ScanReq
 */
struct _MAC_ScanReqDescr_t
{
    /* 32-bit data. */
    MAC_ScanConfCallback_t *callback;       /*!< Confirmation callback handler-function entry point. */

#ifndef _HOST_
    /* Structured data. */
    MacServiceField_t       service;        /*!< MAC-FE requests/responses service field. */
#endif

    MAC_ScanReqParams_t     params;         /*!< MLME-SCAN.request parameters set. */
};


#endif /* _BB_MAC_SAP_TYPES_SCAN_H */

/* eof bbMacSapTypesScan.h */