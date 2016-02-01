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
 * FILENAME: $Workfile: trunk/stack/ZbPro/APS/include/bbZbProAps.h $
 *
 * DESCRIPTION:
 *   This is the general header file for the ZigBee PRO APS component.
 *
 * $Revision: 2784 $
 * $Date: 2014-07-02 16:12:58Z $
 *
 ****************************************************************************************/
#ifndef _ZBPRO_APS_H
#define _ZBPRO_APS_H

/************************* INCLUDES ****************************************************/
#include "bbZbProApsKeywords.h"
#include "bbZbProApsCommon.h"

/************************* DEFINITIONS *************************************************/
/**//**
 * \brief APS start/stop confirmation data type.
 */
typedef struct _ZBPRO_APS_StartStopConfParams_t
{
    ZBPRO_APS_Status_t status;
} ZBPRO_APS_StartStopConfParams_t;

/**//**
 * \brief APS start/stop request descriptor prototype.
 */
typedef struct _ZBPRO_APS_StartStopReqDescr_t ZBPRO_APS_StartStopReqDescr_t;

/**//**
 * \brief Callback function type.
 */
typedef void ZBPRO_APS_StartStopConfCallback_t(ZBPRO_APS_StartStopReqDescr_t *const reqDescr,
        ZBPRO_APS_StartStopConfParams_t *const confParams);

/**//**
 * \brief APS start/stop request descriptor data type.
 */
struct _ZBPRO_APS_StartStopReqDescr_t
{
    ZBPRO_APS_StartStopConfCallback_t *callback;
};

/************************* FUNCTIONS PROTOTYPES ****************************************/
/**//**
 * \brief ZB PRO APS layer initialization routine.
 */
APS_PUBLIC void ZBPRO_APS_Initialization(bool setToDefault);

/**//**
 * \brief ZB PRO APS layer raises this indication when initialization is complete.
 */
APS_PUBLIC void ZBPRO_APS_InitializationDoneInd(bool setToDefault);

/**//**
 * \brief ZB PRO APS layer start routine.
 */
APS_PUBLIC void ZBPRO_APS_StartReq(ZBPRO_APS_StartStopReqDescr_t *const req);

/**//**
 * \brief ZB PRO APS layer stop routine.
 */
APS_PUBLIC void ZBPRO_APS_StopReq(ZBPRO_APS_StartStopReqDescr_t *const req);

#endif /* _ZBPRO_APS_H */