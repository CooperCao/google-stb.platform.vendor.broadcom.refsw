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
 *      MLME-POLL service data types definition.
 *
*******************************************************************************/

#ifndef _BB_MAC_SAP_TYPES_POLL_H
#define _BB_MAC_SAP_TYPES_POLL_H


/************************* INCLUDES *****************************************************/
#include "bbMacSapDefs.h"           /* MAC-SAP common definitions. */
#include "bbMacSapAddress.h"        /* MAC-SAP addressing definitions. */
#include "bbMacSapService.h"        /* MAC-SAP service data types. */
#include "bbMacSapSecurity.h"       /* MAC Security data types. */


/************************* VALIDATIONS **************************************************/
#if !defined(_MAC_CONTEXT_ZBPRO_)
# error This file requires the MAC Context for ZigBee PRO to be included into the build.
#endif


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief Structure for parameters of MLME-POLL.request primitive.
 * \ingroup PollReq
 * \par Documentation
 *  See IEEE Std 802.15.4-2006, subclause 7.1.16.1 'MLME-POLL.request'.
 */
typedef struct _MAC_PollReqParams_t
{
    /* 64-bit data. */
    MAC_Address_t            coordAddress;   /*!< The address of the coordinator to which the
                                                  poll is intended.*/
    /* 16-bit data. */
    MAC_PanId_t              coordPanId;     /*!< The PAN identifier of the coordinator to which
                                                  the poll is intended. */
    /* 8-bit data. */
    MAC_AddrMode_t           coordAddrMode;  /*!< The addressing mode of the coordinator to
                                                  which the poll is intended. */

    MAC_SecurityLevel_t      securityLevel;  /*!< The security level to be used. */

    MAC_SecurityParams_t     securityParams; /*!< Security parameters. They are ignored if the SecurityLevel is
                                                 set to zero. */
} MAC_PollReqParams_t;


/**//**
 * \brief   Structure for parameters of the MLME-POLL.confirm.
 * \ingroup PollConf
 * \details Possible values for the \c status parameter are the following:
 *  - SUCCESS                   The requested transmission was completed successfully.
 *  - CHANNEL_ACCESS_FAILURE    A transmission could not take place due to activity on the
 *                              channel, i.e., the CSMA-CA mechanism has failed.
 *  - FRAME_TOO_LONG            A frame resulting from processing has a length that is
 *                              greater than aMaxPHYPacketSize.
 *  - INVALID_PARAMETER         A parameter in the primitive is either not supported or is
 *                              out of the valid range.
 *  - NO_ACK                    No acknowledgment was received after macMaxFrameRetries.
 *  - UNSUPPORTED_SECURITY      The requested security level is not supported.
 *  - NO_DATA                   Data frame was not received from the Coordinator, or the Data frame.
 *                              which was received from the Coordinator, has a zero length payload.
 *  - RESET                     An MLME-RESET.request was issued prior to execution of the
 *                              MLME-POLL.request being confirmed.
 *
 * \note    The following values for the \c status parameter are not used due to listed
 *  reasons:
 *  - COUNTER_ERROR, UNAVAILABLE_KEY    This implementation of the MAC does not support
 *      the MAC Security.
 *
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclaus 7.1.16.2 MLME-POLL.confirm
 */
typedef struct _MAC_PollConfParams_t
{
    /* 8-bit data. */
    MAC_Status_t           status;          /*!< The status of the last MSDU transmission. */

} MAC_PollConfParams_t;


/**//**
 * \brief   Structure for descriptor of the MLME-POLL.request.
 * \ingroup PollReq
 */
typedef struct _MAC_PollReqDescr_t  MAC_PollReqDescr_t;


/**//**
 * \brief   Template for the callback handler-function of the MLME-POLL.confirm.
 * \ingroup PollConf
 * \param[in]   reqDescr    Pointer to the confirmed request descriptor.
 * \param[in]   confParams  Pointer to the confirmation parameters object.
 * \note Treat the parameters structure pointed by the \p indParams and passed into the
 *  indication callback handler-function as it has been allocated in the program stack by
 *  the MAC before calling this callback handler and will be destroyed just after this
 *  callback returns.
  * \par     Documentation
 *  See IEEE Std 802.15.4-2006, subclause 7.1.16.2 MLME-POLL.confirm.
 */
typedef void MAC_PollConfCallback_t(MAC_PollReqDescr_t   *const reqDescr,
                                    MAC_PollConfParams_t *const confParams);


/**//**
 * \brief   Structure for descriptor of the MLME-POLL.request.
 * \ingroup PollReq
 */
struct _MAC_PollReqDescr_t
{
    /* 32-bit data. */
    MAC_PollConfCallback_t   *callback;      /*!< Entry point of the confirmation callback function. */

#ifndef _HOST_
    /* Structured data. */
    MacServiceField_t         service;       /*!< MAC requests service field. */
#endif

    MAC_PollReqParams_t       params;        /*!< Request parameters structured object. */
};


/**//**
 * \brief Structure for parameters of MLME-POLL.indication primitive.
 * \ingroup PollInd
 * \note This primitive is not a standard one, so the MCPS-DATA.indication primitive
 *  parameters are taken as the model except DSN, Dst. Address, and MSDU Payload.
 * \note Security parameters are excluded because MAC security is not implemented.
 * \par Documentation
 *  See IEEE Std 802.15.4-2006, subclause 7.1.1.3 'MCPS-DATA.indication'.
 */
/* TODO: Decide to use the set of fields like MLME-COMM-STATUS.indication. */
typedef struct _MAC_PollIndParams_t
{
    /* 64-bit data. */
    MAC_Address_t          srcAddr;             /*!< The individual device address of the entity
                                                    from which the MPDU was received. */
    /* 32-bit data. */
    HAL_Symbol__Tstamp_t   timestamp;           /*!< The time, in symbols, at which the
                                                    Data Request MAC Command were received. */
    /* 16-bit data. */
    MAC_PanId_t            srcPanId;            /*!< The 16-bit PAN identifier of the entity
                                                    from which the MPDU was received. */
    /* 8-bit data. */
    MAC_AddrMode_t         srcAddrMode;         /*!< The source addressing mode for this primitive
                                                    corresponding to the received MPDU. */

    PHY_LQI_t              mpduLinkQuality;     /*!< LQI value measured during reception of the MPDU. */

    MAC_Dsn_t              dsn;                 /*!< The DSN of the received data request frame. */

} MAC_PollIndParams_t;


/**//**
 * \brief Template for callback handler-function of MLME-POLL.indication primitive.
 * \ingroup PollInd
 * \details Call the function of this type provided by the higher layer from the MAC to
 *  issue the MLME-POLL.indication to the ZigBee PRO higher layer.
 * \param indParams Pointer to the indication parameters structure.
 * \note Treat the parameters structure pointed by the \p indParams and passed into the
 *  indication callback handler-function as it has been allocated in the program stack by
 *  the MAC before calling this callback handler and will be destroyed just after this
 *  callback returns.
 */
typedef void MAC_PollIndCallback_t(MAC_PollIndParams_t *const indParams);


#endif /* _BB_MAC_SAP_TYPES_POLL_H */

/* eof bbMacSapTypesPoll.h */