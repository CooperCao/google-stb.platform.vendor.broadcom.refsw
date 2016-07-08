/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
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
 ******************************************************************************
*
* FILENAME: $Workfile$
*
* DESCRIPTION:
*   MAC Common Part Sublayer Transmission Flowchart interface.
*
* $Revision$
* $Date$
*
*****************************************************************************************/

#ifndef _BB_MAC_CPS_TX_FLOWCHART_H
#define _BB_MAC_CPS_TX_FLOWCHART_H

/************************* INCLUDES ***********************************************************************************/
#include "private/bbMacMpdu.h"

/************************* DEFINITIONS ********************************************************************************/
/**//**
 * \brief   Structure for composed frame descriptor.
 * \details Combined Header includes:
 *  - conventional MAC header (MHR),
 *  - auxiliary security header (ASH) - only for secured frame,
 *  - nonpayload fields - command frame identifier, or beacon frame parameters.
 *
 * \details Payload Fields include:
 *  - for the case of Data frame - the complete unsecured MSDU,
 *  - for the case of Beacon frame - the beacon payload (it does not include beacon frame parameters),
 *  - for the case of Command frame - the command payload, or command parameters (it does not include command
 *      identifier).
 *
 * \details Following parts are not included in either of sets:
 *  - message integrity code (MIC) - for the case of secured frame,
 *  - conventional MAC footer (MFR) containing the FCS field.
 */
struct MCPS_TXFC__MPDU_descr_t {
    SYS_DataPointer_t   combinedHeader;     /*!< Descriptor of payload containing the Combined Header. */
    SYS_DataPointer_t   payloadFields;      /*!< Descriptor of payload containing the Payload Fields. */
};

/************************* PROTOTYPES *********************************************************************************/
#if !defined(_MAC_CONTEXT_RF4CE_CONTROLLER_)
/**//**
 * \brief   MPDU Constructor callback function assigned for currently processed frame.
 * \details This variable shall be assigned with the constructor callback function, provided by the corresponding MAC-FE
 *  request processor, prior to call MCPS-TX-DATA.request. The linked function is called by the MCPS Transmission
 *  Flowchart when it needs the structured part of the Outgoing Frame Descriptor to be assigned with transmitted frame
 *  parameters.
 * \details The RF4CE Controller build has only one MPDU Constructor - for Data frame.
 */
extern MacMpduConstructor_t  MCPS_TXFC__MPDU_Constructor;
#endif

/*--------------------------------------------------------------------------------------------------------------------*/
/**//**
 * \brief   Outgoing Frame Descriptor.
 */
extern struct MCPS_TXFC__MPDU_descr_t  MCPS_TXFC__MPDU_descr;

/*--------------------------------------------------------------------------------------------------------------------*/
/**//**
 * \brief   Initializes MAC CPS Transmission Flowchart.
 * \details This function must be called once on the application startup.
 */
void MCPS_TXFC__Init(void);

/*--------------------------------------------------------------------------------------------------------------------*/
/**//**
 * \brief   Initiates outgoing frame processing.
 */
void MCPS_TXFC__DATA_req(void);

#endif /* _BB_MAC_CPS_TX_FLOWCHART_H */
