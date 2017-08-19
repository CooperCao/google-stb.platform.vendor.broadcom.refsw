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
 *      PHY-SAP common definitions.
 *
*******************************************************************************/

#ifndef _BB_PHY_SAP_DEFS_H
#define _BB_PHY_SAP_DEFS_H

/************************* INCLUDES ***********************************************************************************/
#include "bbPhyBasics.h"

/************************* DEFINITIONS ********************************************************************************/
/**//**
 * \name    PHY constants.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclauses 6.3, 6.4.1, 6.9.1, 6.9.2, 7.2.1, 7.2.2.3, tables 21, 22.
 */
/**@{*/
#define PHY_aMaxPHYPacketSize   (127)       /*!< The maximum PSDU size (in octets) the PHY shall be able to receive. */
#define PHY_aTurnaroundTime     (12)        /*!< RX-to-TX or TX-to-RX maximum turnaround time (in symbol periods). */
#define PHY_aAckMPDUOverhead    (5)         /*!< The number of octets added by the MAC to the PSDU for the ACK frame. */
#define PHY_aFCSSize            (2)         /*!< The number of octets added by the MAC to the PSDU for the FCS field. */
/**@}*/
SYS_DbgAssertStatic(PHY_aMaxPHYPacketSize == HAL_aMaxPHYPacketSize);
SYS_DbgAssertStatic(PHY_aTurnaroundTime == HAL_aTurnaroundTime);
SYS_DbgAssertStatic(PHY_aAckMPDUOverhead == HAL_aAckMPDUOverhead);
SYS_DbgAssertStatic(PHY_aFCSSize == HAL_aFCSSize);

/*--------------------------------------------------------------------------------------------------------------------*/
/**//**
 * \brief   Consolidated enumeration for the PHY-SAP.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 6.2.3, table 18.
 */
enum PHY_Enum_t {
    PHY__BUSY                   = 0x00,     /*!< The CCA attempt has detected a busy channel. */
    PHY__BUSY_RX                = 0x01,     /*!< The transceiver is asked to change its state while receiving. */
    PHY__BUSY_TX                = 0x02,     /*!< The transceiver is asked to change its state while transmitting. */
    PHY__FORCE_TRX_OFF          = 0x03,     /*!< The transceiver is to be switched off immediately. */
    PHY__IDLE                   = 0x04,     /*!< The CCA attempt has detected an idle channel. */
    PHY__INVALID_PARAMETER      = 0x05,     /*!< A SET/GET request was issued with a parameter in the primitive that is
                                                out of the valid range. */
    PHY__RX_ON                  = 0x06,     /*!< The transceiver is in or is to be configured into the receiver enabled
                                                state. */
    PHY__SUCCESS                = 0x07,     /*!< A SET/GET, an ED operation, or a transceiver state change was
                                                successful. */
    PHY__TRX_OFF                = 0x08,     /*!< The transceiver is in or is to be configured into the transceiver
                                                disabled state. */
    PHY__TX_ON                  = 0x09,     /*!< The transceiver is in or is to be configured into the transmitter
                                                enabled state. */
    PHY__UNSUPPORTED_ATTRIBUTE  = 0x0A,     /*!< A SET/GET request was issued with the identifier of an attribute that
                                                is not supported. */
    PHY__READ_ONLY              = 0x0B,     /*!< A SET/GET request was issued with the identifier of an attribute that
                                                is read-only. */
};
SYS_DbgAssertStatic(sizeof(enum PHY_Enum_t) == 1);

/*--------------------------------------------------------------------------------------------------------------------*/
/**//**
 * \brief   Data type for the \c status parameter returned by the PHY-SAP confirmation primitives.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclauses 6.2.1.2.1, 6.2.2.2.1, 6.2.2.4.1, 6.2.2.6.1, 6.2.2.8.1, 6.2.2.10.1, tables 7, 10,
 *  11, 13, 15, 17.
 */
typedef enum PHY_Enum_t  PHY_Status_t;

#endif /* _BB_PHY_SAP_DEFS_H */

/* eof bbPhySapDefs.h */