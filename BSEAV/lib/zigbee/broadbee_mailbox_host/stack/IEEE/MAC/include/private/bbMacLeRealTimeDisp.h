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
* FILENAME: $Workfile: trunk/stack/IEEE/MAC/include/private/bbMacLeRealTimeDisp.h $
*
* DESCRIPTION:
*   MAC-LE Real-Time Dispatcher interface.
*
* $Revision: 10263 $
* $Date: 2016-02-29 18:03:06Z $
*
*****************************************************************************************/


#ifndef _BB_MAC_LE_REAL_TIME_DISP_H
#define _BB_MAC_LE_REAL_TIME_DISP_H


/************************* INCLUDES *****************************************************/
#include "private/bbMacMpdu.h"      /* MAC MPDU definitions. */


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Enumeration for the \c trxModeWhenIdle parameter of the
 *  MAC-LE-SET-TRX-MODE.request.
 */
typedef enum _MacTrxModeWhenIdle_t
{
    MAC_TRX_MODE_TRX_OFF_WHEN_IDLE = FALSE,     /*!< Code for the TRX_OFF_WHEN_IDLE mode of the MAC-LE. */

    MAC_TRX_MODE_TRX_ON_WHEN_IDLE  = TRUE,      /*!< Code for the TRX_ON_WHEN_IDLE mode of the MAC-LE. */

} MacTrxModeWhenIdle_t;


/**//**
 * \brief   Structure for parameter of the MAC-LE-COMPLETE.response.
 */
typedef struct _MacLeRealTimeDispCompleteRespParams_t
{
    /* 32-bit data. */
    HAL_Symbol__Tstamp_t   txStartTime;     /*!< Last PPDU transmission start timestamp. */

    /* 8-bit data. */
    MAC_Status_t           status;          /*!< Confirmation status. */

#if defined(_MAC_CONTEXT_ZBPRO_)
    MacMpduFramePending_t  ackWithFp;       /*!< Value of the received ACK frame FramePending subfield. */
#endif

#if !defined(_MAC_CONTEXT_RF4CE_CONTROLLER_)
    PHY_ED_t               edMaxValue;      /*!< Maximum value of the EnergyLevel measured. */
#endif

} MacLeRealTimeDispCompleteRespParams_t;


/**//**
 * \brief   Union for the structured parameters returned by the MAC-LE on the
 *  MAC-LE-COMPLETE.response, or the MPDU Surrogate structured object constructed by the
 *  MAC-LE during processing of the MAC-LE-RECEIVE.indication.
 */
typedef union _MacLeReturnedParams_t
{
    MacLeRealTimeDispCompleteRespParams_t  completeParams;      /*!< Parameters of the MAC-LE-COMPLETE.response. */

    MacMpduSurr_t                          rxIndMpduSurr;       /*!< MPDU Surrogate constructed within the
                                                                    MAC-LE-RECEIVE.indication. */
} MacLeReturnedParams_t;


#endif /* _BB_MAC_LE_REAL_TIME_DISP_H */
