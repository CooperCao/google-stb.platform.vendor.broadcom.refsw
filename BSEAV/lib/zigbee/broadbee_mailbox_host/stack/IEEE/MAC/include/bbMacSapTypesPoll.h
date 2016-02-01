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
* FILENAME: $Workfile: trunk/stack/IEEE/MAC/include/bbMacSapTypesPoll.h $
*
* DESCRIPTION:
*   MLME-POLL service data types definition.
*
* $Revision: 3159 $
* $Date: 2014-08-05 19:11:02Z $
*
*****************************************************************************************/


#ifndef _BB_MAC_SAP_TYPES_POLL_H
#define _BB_MAC_SAP_TYPES_POLL_H


/************************* INCLUDES *****************************************************/
#include "bbMacSapDefs.h"           /* MAC-SAP common definitions. */
#include "bbMacSapAddress.h"        /* MAC-SAP addressing definitions. */
#include "bbMacSapService.h"        /* MAC-SAP service data types. */


/************************* VALIDATIONS **************************************************/
#if !defined(_MAC_CONTEXT_ZBPRO_)
# error This file requires the MAC Context for ZigBee PRO to be included into the build.
#endif


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief Structure for parameters of MLME-POLL.indication primitive.
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
    HAL_SymbolTimestamp_t  timestamp;           /*!< The time, in symbols, at which the
                                                    Data Request MAC Command were received. */
    /* 16-bit data. */
    MAC_PanId_t            srcPanId;            /*!< The 16-bit PAN identifier of the entity
                                                    from which the MPDU was received. */
    /* 8-bit data. */
    MAC_AddrMode_t         srcAddrMode;         /*!< The source addressing mode for this primitive
                                                    corresponding to the received MPDU. */

    PHY_Lqi_t              mpduLinkQuality;     /*!< LQI value measured during reception of the MPDU. */

    MAC_Dsn_t              dsn;                 /* TODO: Delete this parameter. */

} MAC_PollIndParams_t;


/**//**
 * \brief Template for callback handler-function of MLME-POLL.indication primitive.
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