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
*   MLME-COMM-STATUS service data types definition.
*
* $Revision$
* $Date$
*
*****************************************************************************************/


#ifndef _BB_MAC_SAP_TYPES_COMM_STATUS_H
#define _BB_MAC_SAP_TYPES_COMM_STATUS_H


/************************* INCLUDES *****************************************************/
#include "bbMacSapDefs.h"           /* MAC-SAP common definitions. */
#include "bbMacSapAddress.h"        /* MAC-SAP addressing definitions. */


/************************* VALIDATIONS **************************************************/
#if defined(_MAC_CONTEXT_RF4CE_CONTROLLER_) && !defined(MAILBOX_UNIT_TEST)
# error This header is not for the RF4CE-Controller build.
#endif


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Structure for parameters of the MLME-COMM-STATUS.indication.
 * \details The MLME-COMM-STATUS.indication is issued by the MAC to the higher layer in
 *  the following cases:
 *  - on receipt of a frame that generates an error in its security processing (namely, in
 *      the case of secured MAC frame reception, because the MAC security is not
 *      implemented),
 *  - following either the MLME-ASSOCIATE.response primitive or the MLME-ORPHAN.response
 *      primitive.
 *
 * \details In the case when the MLME-COMM-STATUS.indication is generated on receipt of a
 *  secured frame possible values for the \c status parameter are the following:
 *  - UNSUPPORTED_LEGACY        A secured MAC frame received.
 *
 * \details In the case when the MLME-COMM-STATUS.indication is generated following either
 *  the MLME-ASSOCIATE.response primitive or the MLME-ORPHAN.response primitive possible
 *  values for the \c status parameter are the following:
 *  - SUCCESS                   The requested transmission was completed successfully.
 *  - TRANSACTION_EXPIRED       The transaction has expired and its information was
 *      discarded (only for the case of MLME-ASSOCIATE.response).
 *  - CHANNEL_ACCESS_FAILURE    A transmission could not take place due to activity on the
 *      channel, i.e., the CSMA-CA mechanism has failed (only for the case of
 *      MLME-ORPHAN.response).
 *  - NO_ACK                    No acknowledgment was received after macMaxFrameRetries
 *      (only for the case of MLME-ORPHAN.response).
 *  - INVALID_PARAMETER         A parameter in the primitive is either not supported or is
 *      out of the valid range.
 *  - RESET                     An MLME-RESET.request was issued prior to execution of the
 *      MLME-ASSOCIATE.response or the MLME-ORPHAN.response or prior to the pending
 *      Association Response MAC Command frame is successfully transmitted.
 *
 * \note    The following values for the \c status parameter are not used due to listed
 *  reasons:
 *  - TRANSACTION_OVERFLOW      This implementation of the MAC allows arbitrary number of
 *      pending requests in the transactions queue.
 *  - COUNTER_ERROR, FRAME_TOO_LONG, IMPROPER_KEY_TYPE, IMPROPER_SECURITY_LEVEL,
 *      SECURITY_ERROR, UNAVAILABLE_KEY, UNSUPPORTED_SECURITY       This implementation of
 *      the MAC does not support the MAC Security.
 *
 * \note    Security parameters are excluded because the MAC Security is not implemented.
 * \par     Documentation
 *  See IEEE Std 802.15.4-2006, subclauses 7.1.12.1.1, 7.5.8.2.3, table 69.
 */
typedef struct _MAC_CommStatusIndParams_t
{
    /* 64-bit data. */
    MAC_Address_t   dstAddr;            /*!< The individual device address of the device for which the frame was
                                            intended. */

    MAC_Address_t   srcAddr;            /*!< The individual device address of the entity from which the frame causing
                                            the error originated; or this device address in the case of indication on
                                            just performed MLME-ASSOCIATE.response or MLME-ORPHAN.response. */
    /* 16-bit data. */
    MAC_PanId_t     panId;              /*!< The 16-bit PAN identifier of the device from which the frame was received
                                            or to which the frame was being sent. */
    /* 8-bit data. */
    MAC_AddrMode_t  dstAddrMode;        /*!< The destination addressing mode for this primitive. */

    MAC_AddrMode_t  srcAddrMode;        /*!< The source addressing mode for this primitive. */

    MAC_Status_t         status;            /*!< The communications status. */

    /* TODO: This field is redundant. Wrap it with a conditional build key. */
    MAC_SecurityLevel_t  securityLevel;     /*!< The security level to be used; or the security level purportedly used
                                                by the received frame. */
} MAC_CommStatusIndParams_t;


/**//**
 * \brief   Template for the callback handler-function of the MLME-COMM-STATUS.indication.
 * \param[in]   indParams   Pointer to the indication parameters object.
 * \details Call functions of this type provided by higher layers of corresponding MAC
 *  contexts, ZigBee PRO and RF4CE, from the MAC to issue the MLME-COMM-STATUS.indication
 *  to the destination ZigBee PRO and/or RF4CE higher layers in the case of secured MAC
 *  frame reception. The indication callback handler-functions must be statically linked
 *  in the project.
 * \note    For the case when the MLME-COMM-STATUS.indication is issued following the
 *  MLME-ASSOCIATE.response or the MLME-ORPHAN.response the callback function specified in
 *  the response descriptor shall be called (just as a kind of confirmation on response)
 *  instead of this common MLME-COMM-STATUS.indication callback function.
 * \details Treat the parameters structure pointed by the \p indParams and passed into the
 *  indication callback handler-function as it has been allocated in the program stack by
 *  the MAC before calling this callback function and will be destroyed just after this
 *  callback function returns.
 * \details For secured frames addressed to both contexts, ZigBee PRO and RF4CE, the
 *  indication handler from the ZigBee PRO stack is called first and then the indication
 *  handler from the Zigbee RF4CE is called. Each handler function is called with its own
 *  copy of indication parameters. Both stacks higher layers are free to alter their own
 *  copies of indication parameters in any way; and it will not affect the second stack.
 * \par     Documentation
 *  See IEEE Std 802.15.4-2006, subclause 7.1.12.1.
 */
typedef void MAC_CommStatusIndCallback_t(MAC_CommStatusIndParams_t *const indParams);


#endif /* _BB_MAC_SAP_TYPES_COMM_STATUS_H */