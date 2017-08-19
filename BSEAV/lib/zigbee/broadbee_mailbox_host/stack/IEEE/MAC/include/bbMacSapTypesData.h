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
 *      MCPS-DATA service data types definition.
 *
*******************************************************************************/

#ifndef _BB_MAC_SAP_TYPES_DATA_H
#define _BB_MAC_SAP_TYPES_DATA_H


/************************* INCLUDES *****************************************************/
#include "bbMacSapDefs.h"
#include "bbMacSapAddress.h"
#include "bbMacSapSecurity.h"
#include "bbMacSapService.h"


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Data type for the MSDU-Handle parameter of the MCPS-DATA.request/confirm.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.1.1.1.1, table 41.
 */
typedef uint8_t  MAC_MsduHandle_t;


/**//**
 * \brief   Enumeration of values of the Acknowledged/Unacknowledged Transmission option
 *  specified with the TX-Options parameter of the MCPS-DATA.request.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.1.1.1.1, table 41.
 */
typedef enum _MAC_TxOptionAck_t
{
    MAC_TX_OPTION_UNACK = 0,        /*!< Perform Unacknowledged transmission. */

    MAC_TX_OPTION_ACK   = 1,        /*!< Perform Acknowledged transmission. */

} MAC_TxOptionAck_t;


/**//**
 * \brief   Enumeration of values of the GTS/CAP Transmission option specified with the
 *  TX-Options parameter of the MCPS-DATA.request.
 * \note    This enumeration is not actually used by the MAC because GTS (guaranteed
 *  time-slot) is not implemented as a specific feature of beacon-enabled PANs only. All
 *  transmissions are processed during the CAP (contention access period).
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.1.1.1.1, table 41.
 */
typedef enum _MAC_TxOptionGts_t
{
    MAC_TX_OPTION_CAP = 0,          /*!< Transmit during Contention Access Period (CAP). */

    MAC_TX_OPTION_GTS = 1,          /*!< Transmit during Guaranteed Time Slot (GTS). */

} MAC_TxOptionGts_t;


/**//**
 * \brief   Enumeration of values of the Indirect/Direct Transmission option specified
 *  with the TX-Options parameter of the MCPS-DATA.request.
 * \note    This enumeration is used only by the ZigBee PRO context. The RF4CE context
 *  does not implement indirect transmission.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.1.1.1.1, table 41.
 */
typedef enum _MAC_TxOptionIndirect_t
{
    MAC_TX_OPTION_DIRECT   = 0,     /*!< Perform Direct (i.e., immediate) Transmission. */

    MAC_TX_OPTION_INDIRECT = 1,     /*!< Push MPDU into the transactions queue for indirect transmission. */

} MAC_TxOptionIndirect_t;


/**//**
 * \brief   Structure for the TX-Options parameters set of the MCPS-DATA.request.
 * \note    The Acknowledged Transmission option takes effect only in the case of unicast
 *  transmission (according to the Destination Address parameter). For the case of
 *  broadcast transmission this option is ignored and considered to be set to
 *  Unacknowledged; the Warning will be logged if this option is set to Acknowledged with
 *  the broadcast destination address.
 * \note    The GTS Transmission option is not implemented and has no effect (ignored). It
 *  is constantly considered to be set to CAP Transmission; the Warning will be logged if
 *  this option is set to GTS Transmission. But if _MAC_SAP_PROCESS_REDUNDANT_PARAMS_
 *  conditional build key is defined by the project make configuration file, in the case
 *  when MCPS-DATA.request is called with GTS Transmission option, the INVALID_GTS status
 *  will be confirmed.
 * \note    The Indirect Transmission option is implemented only for the ZigBee PRO
 *  context. For the case of RF4CE context this option is ignored and constantly
 *  considered to be set to Direct Transmission; but if _MAC_SAP_PROCESS_REDUNDANT_PARAMS_
 *  conditional build key is defined by the project make configuration file, in the case
 *  when MCPS-DATA.request is called with Indirect Transmission option for the RF4CE
 *  context, the TRANSACTION_OVERFLOW status will be confirmed. For ZigBee PRO context in
 *  the case of transmission onto the PAN Coordinator with omitted Destination Address
 *  this option is also ignored (indirect transmissions to the PAN Coordinator are
 *  forbidden) and direct (immediate) transmission is performed; the Warning will be
 *  logged if this option is set to Indirect transmission with omitted Destination
 *  Address.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.1.1.1.1, table 41.
 */
typedef struct _MAC_TxOptions_t
{
    MAC_TxOptionAck_t       ack : 1;        /*!< Acknowledged / Unacknowledged transmission. */

    MAC_TxOptionGts_t       gts : 1;        /*!< GTS / CAP transmission. */

    MAC_TxOptionIndirect_t  ind : 1;        /*!< Indirect / Direct transmission. */

} MAC_TxOptions_t;


/**//**
 * \brief   Structure for parameters of the MCPS-DATA.request.
 * \ingroup DataReq
 * \details The higher layer is responsible for dismissing the dynamic memory allocated
 *  for the MSDU payload. The payload may be dismissed by the higher layer when the
 *  corresponding \c callback handler-function for MCPS-DATA.confirm is called by the MAC.
 * \note    See also notes to the TxOptions parameter.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.1.1.1.1, table 41.
 */
typedef struct _MAC_DataReqParams_t
{
    /* 64-bit data. */
    MAC_Address_t        dstAddr;           /*!< The individual device address of the entity to which the MSDU is being
                                                transferred. */
    /* Structured / 32-bit data. */
    SYS_DataPointer_t    payload;           /*!< The set of octets forming the MSDU to be transmitted by the MAC
                                                sublayer entity. */
    /* 16-bit data. */
    MAC_PanId_t          dstPanId;          /*!< The 16-bit PAN identifier of the entity to which the MSDU is being
                                                transferred. */
    /* 8-bit data. */
    MAC_AddrMode_t       dstAddrMode;       /*!< The destination addressing mode for this primitive and subsequent
                                                MPDU. */

    MAC_AddrMode_t       srcAddrMode;       /*!< The source addressing mode for this primitive and subsequent MPDU. */

    MAC_MsduHandle_t     msduHandle;        /*!< The handle associated with the MSDU to be transmitted by the MAC
                                                sublayer entity. */
    /* Structured / 8-bit data. */
    MAC_TxOptions_t      txOptions;         /*!< The 3 bits (b0, b1, b2) indicate the transmission options for this
                                                MSDU. */

    /* 8-bit data. */
    MAC_SecurityLevel_t  securityLevel;     /*!< The security level to be used. */

    MAC_SecurityParams_t securityParams;    /*!< Security parameters. They are ignored if the SecurityLevel is
                                                 set to zero. */
} MAC_DataReqParams_t;


/**//**
 * \brief   Width of the Timestamp parameter of the MCPS-DATA.confirm and the
 *  MCPS-DATA.indication, in bits.
 * \details The Timestamp parameter of the MCPS-DATA.confirm and the MCPS-DATA.indication
 *  shall belong to the range from 0x00000000 to 0x00FFFFFF, i.e. only lowest 24 bits of
 *  the original timestamp 32-bit value are used.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclauses 7.1.1.2.1, 7.1.1.3.1, tables 42, 43.
 */
#define MAC_DATA_TIMESTAMP_WIDTH        24


/**//**
 * \brief   Bitmask for the Timestamp parameter of the MCPS-DATA.confirm and the
 *  MCPS-DATA.indication.
 * \details The Timestamp parameter of the MCPS-DATA.confirm and the MCPS-DATA.indication
 *  shall belong to the range from 0x00000000 to 0x00FFFFFF, i.e. only lowest 24 bits of
 *  the original timestamp 32-bit value are used.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclauses 7.1.1.2.1, 7.1.1.3.1, tables 42, 43.
 */
#define MAC_DATA_TIMESTAMP_BITMASK      (BIT_MASK(MAC_DATA_TIMESTAMP_WIDTH))


/**//**
 * \brief   Structure for parameters of the MCPS-DATA.confirm.
 * \ingroup DataConf
 * \details Possible values for the \c status parameter are the following:
 *  - SUCCESS                   The requested transmission was completed successfully.
 *  - CHANNEL_ACCESS_FAILURE    A transmission could not take place due to activity on the
 *      channel, i.e., the CSMA-CA mechanism has failed.
 *  - FRAME_TOO_LONG            A frame resulting from processing has a length that is
 *      greater than aMaxPHYPacketSize.
 *  - INVALID_ADDRESS           A request to send data was unsuccessful because neither
 *      the source address parameters nor the destination address parameters were present.
 *      This status is used only for the ZigBee PRO context; for the RF4CE context if the
 *      source and/or the destination addressing mode is set to no-address the
 *      INVALID_PARAMETER status is confirmed.
 *  - INVALID_GTS               The requested GTS transmission failed because this
 *      implementation of the MAC does not support GTS (guaranteed time slots).
 *  - INVALID_PARAMETER         A parameter in the primitive is either not supported or is
 *      out of the valid range.
 *  - NO_ACK                    No acknowledgment was received after macMaxFrameRetries.
 *  - TRANSACTION_EXPIRED       The transaction has expired and its information was
 *      discarded. This status is used only for the ZigBee PRO context.
 *  - TRANSACTION_OVERFLOW      This status is used only for the RF4CE context: it is
 *      returned in the case when MCPS-DATA.request is issued with Indirect TX Option.
 *      This status is not used for the ZigBee PRO context, because this implementation of
 *      the MAC allows arbitrary number of pending requests in the transactions queue.
 *  - UNSUPPORTED_SECURITY      The requested security level is not supported.
 *  - PURGED                    An MCPS-PURGE.request was issued prior to the pending
 *      transaction is successfully transmitted, for the case of indirect data
 *      transmission.
 *  - RESET                     An MLME-RESET.request was issued prior to execution of the
 *      MCPS-DATA.request being confirmed, or prior to the pending transaction is
 *      successfully transmitted, for the case of indirect data transmission.
 *
 * \note    The following values for the \c status parameter are not used due to listed
 *  reasons:
 *  - COUNTER_ERROR, UNAVAILABLE_KEY    This implementation of the MAC does not support
 *      the MAC Security.
 *
 * \details The \c timestamp at which the data were transmitted is bound to the leading
 *  edge of the first chip (or the first symbol) of the PPDU.PHR field of the transmitted
 *  frame. For the case of unsuccessful confirmation status this parameter is set to zero.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclauses 7.1.1.2.1, 7.1.17, tables 42, 78.
 */
typedef struct _MAC_DataConfParams_t
{
    /* 32-bit data. */
    HAL_Symbol__Tstamp_t   timestamp;       /*!< The time, in symbols, at which the data were transmitted. */

    /* 8-bit data. */
    MAC_MsduHandle_t       msduHandle;      /*!< The handle associated with the MSDU being confirmed. */

    MAC_Status_t           status;          /*!< The status of the last MSDU transmission. */

} MAC_DataConfParams_t;


/**//**
 * \brief   Structure for descriptor of the MCPS-DATA.request.
 * \ingroup DataReq
 */
typedef struct _MAC_DataReqDescr_t  MAC_DataReqDescr_t;


/**//**
 * \brief   Template for the callback handler-function of the MCPS-DATA.confirm.
 * \ingroup DataConf
 * \param[in]   reqDescr    Pointer to the confirmed request descriptor.
 * \param[in]   confParams  Pointer to the confirmation parameters object.
 * \details Call functions of this type provided by higher layers of corresponding MAC
 *  contexts, ZigBee PRO and RF4CE, from the MAC to issue the MCPS-DATA.confirm to the
 *  higher layer that originally issued the request primitive to the MAC.
 * \details To issue the confirmation primitive the MAC calls the confirmation callback
 *  handler-function that was specified with the \c callback parameter of the original
 *  request primitive descriptor that is pointed here by the \p reqDescr argument.
 * \details The request descriptor object that was originally used to issue request to the
 *  MAC and is pointed here with the \p reqDescr is released by the MAC for random use by
 *  the higher layer (the owner of the request descriptor object) when this confirmation
 *  callback handler-function is called by the MAC. If there was a payload specified with
 *  the request parameters, it may be dismissed by the higher layer when this callback
 *  function is called by the MAC (the MAC will not ever dismiss payloads received from
 *  the higher layers itself).
 * \details Treat the parameters structure pointed by the \p confParams and passed into
 *  the confirmation callback handler-function as it has been allocated in the program
 *  stack by the MAC before calling this callback function and will be destroyed just
 *  after this callback function returns.
 * \note    The MCPS-DATA.confirm is issued by the MAC for each single MCPS-DATA.request
 *  even if the indirect data transmission was interrupted by the MCPS-PURGE.request or
 *  the MLME-RESET.request (for this case the \c status parameter of confirmation is set
 *  to RESET).
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.1.1.2.
 */
typedef void MAC_DataConfCallback_t(MAC_DataReqDescr_t *const reqDescr, MAC_DataConfParams_t *const confParams);


/**//**
 * \brief   Structure for descriptor of the MCPS-DATA.request.
 * \ingroup DataReq
 */
struct _MAC_DataReqDescr_t
{
    /* 32-bit data. */
    MAC_DataConfCallback_t *callback;       /*!< Entry point of the confirmation callback function. */

    /* Structured data. */
    MacServiceField_t       service;        /*!< MAC requests service field. */

    MAC_DataReqParams_t     params;         /*!< Request parameters structured object. */
};


/**//**
 * \brief   Structure for parameters of the MCPS-DATA.indication.
 * \ingroup DataInd
 * \details The \c timestamp at which the data were received is bound to the leading edge
 *  of the first chip (or the first symbol) of the PPDU.PHR field of the received frame.
 * \details The MSDU received within the Data frame is indicated to the higher layer with
 *  \c payload parameter. The MSDU length may be obtained by the higher layer from the
 *  dynamic payload object described by the \c payload. The higher layer is responsible
 *  for dismissing the dynamic memory allocated by the MAC for the MSDU payload.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.1.1.3.1, table 43.
 */
typedef struct _MAC_DataIndParams_t
{
    /* 64-bit data. */
    MAC_Address_t          dstAddr;             /*!< The individual device address of the entity to which the MSDU is
                                                    being transferred. */

    MAC_Address_t          srcAddr;             /*!< The individual device address of the entity from which the MSDU was
                                                    received. */
    /* 32-bit data. */
    HAL_Symbol__Tstamp_t   timestamp;           /*!< The time, in symbols, at which the data were received. */

    /* Structured / 32-bit data. */
    SYS_DataPointer_t      payload;             /*!< The set of octets forming the MSDU being indicated by the MAC
                                                    sublayer entity and its length. */
    /* 16-bit data. */
    MAC_PanId_t            dstPanId;            /*!< The 16-bit PAN identifier of the entity to which the MSDU is being
                                                    transferred. */

    MAC_PanId_t            srcPanId;            /*!< The 16-bit PAN identifier of the entity from which the MSDU was
                                                    received. */
    /* 8-bit data. */
    MAC_AddrMode_t         dstAddrMode;         /*!< The destination addressing mode for this primitive corresponding to
                                                    the received MPDU. */

    MAC_AddrMode_t         srcAddrMode;         /*!< The source addressing mode for this primitive corresponding to the
                                                    received MPDU. */

    PHY_LQI_t              mpduLinkQuality;     /*!< LQI value measured during reception of the MPDU. */

    MAC_Dsn_t              dsn;                 /*!< The DSN of the received data frame. */

    MAC_SecurityLevel_t    securityLevel;       /*!< The security level purportedly used by the received data frame. */

    MAC_SecurityParams_t   securityParams;      /*!< Security parameters. They are ignored if the SecurityLevel is
                                                     set to zero. */
} MAC_DataIndParams_t;


/**//**
 * \brief   Template for the callback handler-function of the MCPS-DATA.indication.
 * \ingroup DataInd
 * \param[in]   indParams   Pointer to the indication parameters object.
 * \details Call functions of this type provided by higher layers of corresponding MAC
 *  contexts, ZigBee PRO and RF4CE, from the MAC to issue the MCPS-DATA.indication to the
 *  destination ZigBee PRO and/or RF4CE higher layers.
 * \details To issue the indication primitive the MAC calls the indication callback
 *  handler-function that is statically linked in the Project.
 * \details Treat the parameters structure pointed by the \p indParams and passed into the
 *  indication callback handler-function as it has been allocated in the program stack by
 *  the MAC before calling this callback function and will be destroyed just after this
 *  callback function returns.
 * \details The payload described by \c payload is allocated by the MAC in the dynamic
 *  memory and must be dismissed by the higher layer (the MAC will not ever dismiss
 *  payloads transferred to the higher layers itself); for indications routed to both
 *  stacks, each stack higher layer shall dismiss its own copy of the payload.
 * \details For Data frames addressed to both contexts, ZigBee PRO and RF4CE, the
 *  indication handler from the ZigBee PRO stack is called first and then the indication
 *  handler from the RF4CE is called. Each handler function is called with its own copy of
 *  indication parameters and of the MSDU payload. Both stacks higher layers are free to
 *  alter or dismiss their own copies of indication parameters and the MSDU payload in any
 *  way; and it will not affect the second stack.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.1.1.3.
 */
typedef void MAC_DataIndCallback_t(MAC_DataIndParams_t *const indParams);


#endif /* _BB_MAC_SAP_TYPES_DATA_H */

/* eof bbMacSapTypesData.h */