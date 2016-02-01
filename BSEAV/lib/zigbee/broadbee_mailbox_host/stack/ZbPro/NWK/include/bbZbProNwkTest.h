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
 * FILENAME: $Workfile: trunk/stack/ZbPro/NWK/include/bbZbProNwkTest.h $
 *
 * DESCRIPTION:
 *   This is the header file created for testing purposes.
 *
 * $Revision: 1751 $
 * $Date: 2014-03-11 13:14:58Z $
 *
 ****************************************************************************************/
#ifndef _ZBPRO_NWK_TEST_H
#define _ZBPRO_NWK_TEST_H
#if defined(_CONSOLE_TEST_) && defined(_ZBPRO_NWK_TEST_)

/************************* INCLUDES ****************************************************/
#include "bbZbProNwkCommon.h"

/************************* FUNCTIONS PROTOTYPES ****************************************/
/************************************************************************************//**
  \brief Performs several tests of main NWK layer services.
  \param[in] isMaster - should be true if the device is initiator of data exchange.
  \param[in] address - address of this device.
  \param[in] panId - working PAN ID.
  \param[in] channel - working channel.
  \param[in] destAddr - destination address.
 ****************************************************************************************/
void ZBPRO_NWK_StartTest(bool isMaster,
                         ZBPRO_NWK_NwkAddr_t address,
                         ZBPRO_NWK_PanId_t panId,
                         ZBPRO_NWK_Channel_t channel,
                         ZBPRO_NWK_NwkAddr_t destAddr,
                         ZBPRO_NWK_ExtAddr_t destExtAddr);

#endif /* defined(_CONSOLE_TEST_) && define(_ZBPRO_NWK_TEST_) */

#endif /* _ZBPRO_NWK_TEST_H */