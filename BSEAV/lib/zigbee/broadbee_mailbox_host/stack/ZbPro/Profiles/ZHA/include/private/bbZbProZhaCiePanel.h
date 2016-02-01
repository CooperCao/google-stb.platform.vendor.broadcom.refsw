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
* FILENAME: $Workfile: $
*
* DESCRIPTION:
*   ZHA CIE panel definitions.
*
* $Revision: 7361 $
* $Date: 2015-07-08 17:17:17Z $
*
*****************************************************************************************/

#ifndef _BB_ZBPRO_ZHA_CIE_PANEL_H_
#define _BB_ZBPRO_ZHA_CIE_PANEL_H_

/************************* INCLUDES *****************************************************/
#include "bbZbProZhaCommon.h"
#include "bbZbProZhaSapCieDevice.h"

/************************* DEFINITIONS **************************************************/
/**//**
 * \brief Type for the Seconds Remaining for the new Panel Status.
*/
typedef  uint8_t  ZbProZhaCiePanelSecondsRemaining_t;

/**//**
 * \brief Type for the Audible Notification state of the Panel.
*/
typedef  ZBPRO_ZCL_SapIasAceAudibleNotification_t ZbProZhaCiePaneAudibleNotification_t;

/**//**
 * \brief Type for the Alarm Status of the Panel.
*/
typedef  ZBPRO_ZCL_IasAceAlarmStatus_t  ZbProZhaCiePanelAlarmStatus_t;

/**//**
 * \brief Descriptor of the current state of the Panel.
*/
typedef struct _ZbProZhaCiePanelDescriptor_t
{
    ZbProZhaCiePanelStatus_t               status;               /*!< Next panel state, which will be after the Seconds
                                                                         Remainig. If Seconds Remaning == 0 it is a
                                                                         current panel state. */

    ZbProZhaCiePanelStatus_t               nextStatus;           /*!< Next status which will be set to the panel after
                                                                         secondsRemaining time. */

    ZbProZhaCiePanelSecondsRemaining_t     secondsRemaining;     /*!< Number of seconds remaining for the server to
                                                                         be in the state indicated in the Panel Status. */

    ZbProZhaCiePaneAudibleNotification_t   audibleNotification;  /*!< Current Audible Notification state. */

    ZbProZhaCiePanelAlarmStatus_t          alarmStatus;          /*!< Current Alarm Status. */

    SYS_TimeoutSignal_t                    setPanelStatusTimer;  /*!< Timer for procceed changing status. */

} ZbProZhaCiePanelDescriptor_t;

/**//**
 * \brief Timeout in milliseconds between two sending of the Panel Status Changed command.
 */
#define ZBPRO_ZHA_CIE_PANEL_CHANGE_STATUS_TIMEOUT  1000U

/**//**
 * \brief Initial seconds remaining value before going to ARMED state from DISARMED state. In seconds, of course.
*/
#define ZBPRO_ZHA_CIE_PANEL_DISARMED_TO_ARMED_DELAY 10u


/************************* PROTOTYPES ***************************************************/
/**//**
 * \brief Panel initialization. It drops the panel status.
*/
void zbProZhaCiePanelInit(void);

/**//**
 * \brief Returns a pointer to the Panel descriptor.
*/
ZbProZhaCiePanelDescriptor_t * const zbProZhaCiePanelGetDescr(void);

/**//**
 * \brief Setting the new status to the Panel.
 * \param [in] newStatus - the new status of the Panel,
 * \param [in] seconds - the seconds remaining to change a state.
 * \param [in] sendNotification - is it need to send Panes Status Changed command.
*/
void zbProZhaCiePanelSetStatus( ZbProZhaCiePanelStatus_t            newStatus,
                                ZbProZhaCiePanelSecondsRemaining_t  seconds,
                                bool sendNotification);

/**//**
 * \brief This function sets panel status for appropriate value corresponding
 * to armMode. Then it returns a corresponding Arm Notification value.
*/
ZBPRO_ZCL_IasAceArmNotification_t zbProZhaCiePanelArmProceed(ZBPRO_ZCL_IasAceArmMode_t armMode);


/**//**
 * \brief Function to proceed the Set Panel Status request.
*/
void zbProZhaCieDeviceSetPanelStatusReqHandler(ZBPRO_ZHA_CieSetPanelStatusReqDescr_t * const reqDescr);


/**//**
 * \brief Set Pane Status timer timeout proceeding.
*/
void zbProZhaCieDeviceSetPanelStatusReqProceed(SYS_TimeoutTaskServiceField_t *const timeoutService);


/**//**
 * \brief Just returns the current Panel status.
*/
ZbProZhaCiePanelStatus_t zbProZhaCiePanelGetStatus(void);




#endif