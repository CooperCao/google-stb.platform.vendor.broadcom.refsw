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
* FILENAME: $Workfile: branches/ext_xhuajun/MailboxIntegration/stack/common/HAL/arc601-soc/src/bbSocPowerFilterKey.c $
*
* DESCRIPTION:
*
*
* $Revision: 3888 $
* $Date: 2014-10-04 00:03:46Z $
*
*****************************************************************************************/


/************************* INCLUDES *****************************************************/
#include "bbSysBasics.h"
#include "bbMailPowerFilterKey.h"
#include "bbMailAPI.h"
//#include "bbSocMailbox.h"
//#include "bbSocIrqCtrl.h"           /* SoC Interrupt Controller Hardware interface. */

/************************************ The global data ***********************************/
/******************************************************************************************
* The filter key mapping table's format is as below,
* 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15......255  bits
* [    byte0    ][       byte1       ] .....[byte31]
*******************************************************************************************/
static uint8_t rf4cewakeUpActionCodeFilter[RF4CE_WAKE_UP_ACTION_CODE_FILTER_LENGTH];


/*********************************** The helper function *********************************/
INLINE uint8_t *RF4CEWakeUpActionCodeFilter(void)  { return rf4cewakeUpActionCodeFilter; }


/*********************************** The API function *********************************/
/******************************************************************************************
  \brief    Set the key map table to wake up host cpu. If the key[keycode] is set, always
            wake up the host CPU. Otherwise, ignore the message going to host.
  \param[in]  req - The request descriptor.
  \return     None.
*******************************************************************************************/
void RF4CE_ZRC_SetWakeUpActionCodeReq(RF4CE_ZRC_SetWakeUpActionCodeReqDescr_t *req)
{
    memcpy(RF4CEWakeUpActionCodeFilter(), req->params.wakeUpActionCodeFilter, RF4CE_WAKE_UP_ACTION_CODE_FILTER_LENGTH);
    RF4CE_ZRC_SetWakeUpActionCodeConfParams_t conf;
    memset(&conf, 0, sizeof(RF4CE_ZRC_SetWakeUpActionCodeConfParams_t));
    memcpy(conf.wakeUpActionCodeFilter, RF4CEWakeUpActionCodeFilter(), RF4CE_WAKE_UP_ACTION_CODE_FILTER_LENGTH);
    conf.status = 0;
    req->callback(req, &conf);
}

/******************************************************************************************
  \brief    Get the key map table to wake up host cpu.
  \param[in]  req - The request descriptor.
  \return     None.
*******************************************************************************************/
void RF4CE_ZRC_GetWakeUpActionCodeReq(RF4CE_ZRC_GetWakeUpActionCodeReqDescr_t *req)
{
    RF4CE_ZRC_GetWakeUpActionCodeConfParams_t conf;
    memset(&conf, 0, sizeof(RF4CE_ZRC_GetWakeUpActionCodeConfParams_t));
    memcpy(conf.wakeUpActionCodeFilter, RF4CEWakeUpActionCodeFilter(), RF4CE_WAKE_UP_ACTION_CODE_FILTER_LENGTH);
    conf.status = 0;
    req->callback(req, &conf);
}

/******************************************************************************************
  \brief  Check if this is the power up key.
  \param[in]  keyCode - the key code to be checked.
  \return     > 0 If this key is the power up key.
              <=0 Otherwise
*******************************************************************************************/
uint8_t RF4CE_ShouldWakeUpHostCpu(uint8_t keyCode)
{
    return (RF4CEWakeUpActionCodeFilter()[keyCode / 8]) & (1 << (keyCode % 8)) ? 1 : 0;
}

#ifdef RF4CE_ZRC_WAKEUP_ACTION_CODE_SUPPORT

static bool hostWakingFlag = false;

//static SYS_TimeoutSignal_t hostWakeupTimer;

void RF4CE_PMSetHostWakingUp(bool wakeup)
{
    hostWakingFlag = wakeup;
}

static bool isHostWakingUp()
{
    return hostWakingFlag;
}

// TODO
static void socWakeUpHostCpu()
{
    SET_REG_FIELD(RF4CE_MAC_PWR_CTL, WAKE_CPU, 1);
}
// TODO
static uint8_t socCpuIsSleep()
{
    return GET_REG_FIELD(RF4CE_MAC_PWR_STAT, PM_S3_MODE) || GET_REG_FIELD(RF4CE_MAC_PWR_STAT, PM_S2_MODE);
}

void RF4CE_ZRC2_ControlCommandInd(RF4CE_ZRC2_ControlCommandIndParams_t *const indParams)
{
    if(0 == socCpuIsSleep()){
        Mail_Serialize(mailDescriptorPtr, RF4CE_ZRC2_CONTROL_COMMAND_IND_FID, indParams);
        return;
    }
    // Check if this key could wake up the CPU.
    RF4CE_ZRC2_ActionVendor_t actionData;
    for(int iter = 0; iter < SYS_GetPayloadSize(&indParams->payload) / sizeof(RF4CE_ZRC2_ActionVendor_t); iter ++){
        SYS_CopyFromPayload(&actionData, &indParams->payload, iter * sizeof(RF4CE_ZRC2_ActionVendor_t), sizeof(RF4CE_ZRC2_ActionVendor_t));
        if(RF4CE_ShouldWakeUpHostCpu(actionData.code)){
            socWakeUpHostCpu();
            Mail_Serialize(mailDescriptorPtr, RF4CE_ZRC2_CONTROL_COMMAND_IND_FID, indParams);
            break;
        }
    }
}

void hostWakeupTimerFired()
{
    RF4CE_PMSetHostWakingUp(false);
}

void RF4CE_ZRC1_ControlCommandInd(RF4CE_ZRC1_ControlCommandIndParams_t *const indParams)
{
    if(0 == socCpuIsSleep() && !isHostWakingUp()){
        Mail_Serialize(mailDescriptorPtr, RF4CE_ZRC1_IND_CONTROLCOMMAND_FID, indParams);
        return;
    }
    // Check if this key could wake up the CPU.
    if(RF4CE_ShouldWakeUpHostCpu(indParams->commandCode) && !isHostWakingUp()){
        socWakeUpHostCpu();
        RF4CE_PMSetHostWakingUp(true);
        Mail_Serialize(mailDescriptorPtr, RF4CE_ZRC1_IND_CONTROLCOMMAND_FID, indParams);
    }
}
#endif

/* eof bbSocPowerFilterKey.c */