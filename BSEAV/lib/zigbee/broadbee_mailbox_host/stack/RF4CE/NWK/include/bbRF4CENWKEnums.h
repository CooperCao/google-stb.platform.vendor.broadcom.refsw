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
 *      This is the header file for the RF4CE Network Layer component enumerations declaration.
 *
*******************************************************************************/

#ifndef _RF4CE_NWK_ENUMS_H
#define _RF4CE_NWK_ENUMS_H

/************************* INCLUDES ****************************************************/
#include "bbSysTypes.h"

/************************* ENUMERATIONS ************************************************/
/**//**
 * \brief NLDE-DATA Status Enumeration used.
 */
typedef enum _RF4CE_NLDE_DATA_Status_t
{
    RF4CE_SUCCESS                   = 0x00, /*!< The requested operation was completed successfully. */
    RF4CE_NO_ORG_CAPACITY           = 0xb0, /*!< A pairing link cannot be established since the originator node has
                                                 reached its maximum number of entries in its pairing table. */
    RF4CE_NO_REC_CAPACITY           = 0xb1, /*!< A pairing link cannot be established since the recipient node has
                                                 reached its maximum number of entries in its pairing table. */
    RF4CE_NO_PAIRING                = 0xb2, /*!< A pairing table entry could not be found that corresponds to the
                                                 supplied pairing reference. */
    RF4CE_NO_RESPONSE               = 0xb3, /*!< A response frame was not received within nwkResponseWaitTime or an
                                                 acknowledgement for a data frame was not received within
                                                 nwkcMaxDutyCycle. */
    RF4CE_NOT_PERMITTED             = 0xb4, /*!< A pairing request was denied by the recipient node or an attempt to
                                                 update a security link key was not possible due to one or more nodes
                                                 not supporting security. */
    RF4CE_DUPLICATE_PAIRING         = 0xb5, /*!< A duplicate pairing table entry was detected following the receipt of
                                                 a pairing request command frame. */
    RF4CE_FRAME_COUNTER_EXPIRED     = 0xb6, /*!< The frame counter has reached its maximum value. */
    RF4CE_DISCOVERY_ERROR           = 0xb7, /*!< Too many unique matched discovery request or valid response command
                                                 frames were received than requested. */
    RF4CE_DISCOVERY_TIMEOUT         = 0xb8, /*!< No discovery request or response command frames were received during
                                                 discovery. */
    RF4CE_SECURITY_TIMEOUT          = 0xb9, /*!< The security link key exchange or recovery procedure did not complete
                                                 within the required time. */
    RF4CE_SECURITY_FAILURE          = 0xba, /*!< A security link key was not successfully established between both ends
                                                 of a pairing link. */
    RF4CE_INVALID_PARAMETER         = 0xe8, /*!< A parameter in the primitive is either not supported or is out of the
                                                 valid range. */
    RF4CE_NO_ACK                    = 0xe9, /*!< No ACK received. */
    RF4CE_UNSUPPORTED_ATTRIBUTE     = 0xf4, /*!< A SET/GET request was issued with the identifier of a NIB attribute
                                                 that is not supported. */
    RF4CE_INVALID_INDEX             = 0xf9, /*!< An attempt to write to a NIB attribute that is in a table failed
                                                 because the specified table index was out of range. */
    RF4CE_AUTO_DISCOVERY_CANCELLED  = 0xfa
} RF4CE_NLDE_DATA_Status_t;

/**//**
 * \brief RF4CE NWK Machine State Enumeration used.
 */
typedef enum _RF4CE_NWKState_t
{
    RF4CE_NWK_STATE_DORMANT = 0x00,                 /*!< NWK cold startup state. Needs call to NMLE.RESET to start
                                                         network before a call to NMLE.START. */
    RF4CE_NWK_STATE_RESET,                          /*!< NWK state after reset. Only from that state NWK can start! */
    RF4CE_NWK_STATE_STARTING,                       /*!< NWK starting state. */
    RF4CE_NWK_STATE_RUNNING,                        /*!< NWK started and robust. */
    RF4CE_NWK_STATE_DISCOVERY,                      /*!< NWK is in discovery request state. */
    RF4CE_NWK_STATE_PAIRING,                        /*!< NWK is in pairing request state. */
    RF4CE_NWK_STATE_PAIR,                           /*!< NWK is in pairing request state. */
    RF4CE_NWK_STATE_UNPAIR,                         /*!< NWK is in unpairing request state. */
    RF4CE_NWK_STATE_DATA,                           /*!< NWK is in data request state. */
    RF4CE_NWK_STATE_FA                              /*!< NWK is in frequency agility state. */
} RF4CE_NWKState_t;

#endif /* _RF4CE_NWK_ENUMS_H */

/* eof bbRF4CENWKEnums.h */