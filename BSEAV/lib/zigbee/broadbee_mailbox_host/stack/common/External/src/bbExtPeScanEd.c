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
* FILENAME: $Workfile: branches/apokidko/ED/stack/common/External\src\bbExtPeScanEd.c $
*
* DESCRIPTION:
*   PE scan energy measurement function.
*
* $Revision: 11238 $
* $Date: 2016-04-08 18:21:51Z $
*
*****************************************************************************************/

#ifdef _PE_ENERGY_MEASUREMENT_

/************************* INCLUDES *****************************************************/
#include "bbExtPeScanEd.h"
#include "bbSysTimeoutTask.h"
#include "bbSocCalControl.h"
#include "bbSocRxDriver.h"
#include "bbMacSapTypesSet.h"

typedef struct _ScanDataStruct_t
{
    SYS_TimeoutSignal_t          timeoutSignal;
    MAC_SetReqDescr_t            macSetReqDescr;
    EXT_PeScanEdStartReqDescr_t  *startReq;

    uint8_t        isOverload;
    uint16_t       length;
    uint16_t       indicationThreshold;
    uint32_t       countStart, countStop;
    SYS_Time_t     timeStamp[EXT_PeScanEdTABLE_COUNT_MAX];
    uint32_t       energy[EXT_PeScanEdTABLE_COUNT_MAX];
} ScanDataStruct_t;

typedef enum
{
    PE_SCAN_ED_STATUS_SUCCESS,
    PE_SCAN_ED_STATUS_FAILURE,
    PE_SCAN_ED_STATUS_OVERLOAD,
    PE_SCAN_ED_STATUS_HAVE_DATA,
} PE_SCAN_ED_Status_t;


#define RECORD_SIZE                   (sizeof(SYS_Time_t)+sizeof(uint32_t))
#define MAX_COUNT                     (SYS_DYNAMIC_DATA_LENGTH_MAX / RECORD_SIZE)

static ScanDataStruct_t history;

extern struct RxState_t RxState;

/************************* IMPLEMENTATION ***********************************************/
static void extPeEdTimecb(SYS_TimeoutTaskServiceField_t *const timeoutService);
static void extPeScanEdFillHistoryConf(EXT_PeScanEdGetHistoryConfParams_t *const conf);
static void macSetChannelCb(MAC_SetReqDescr_t *const reqDescr, MAC_SetConfParams_t *const confParams);
static void macSetRxOnWhenIdleCb(MAC_SetReqDescr_t *const reqDescr, MAC_SetConfParams_t *const confParams);

/* Start energy meter measurements. */
void EXT_PeScanEdStartReq(EXT_PeScanEdStartReqDescr_t *const reqDescr)
{
    history.startReq = reqDescr;
    history.indicationThreshold = reqDescr->params.indicationThreshold;

    history.timeoutSignal.timeout = reqDescr->params.timeout;
    history.timeoutSignal.callback = extPeEdTimecb;

#if defined(_MAC_CONTEXT_ZBPRO_)
    if (reqDescr->params.needSetup)
    {
        history.macSetReqDescr.params.attribute = MAC_CURRENT_CHANNEL;
        history.macSetReqDescr.params.attributeValue.macCurrentChannel = reqDescr->params.channel;
        history.macSetReqDescr.params.payload = SYS_EMPTY_PAYLOAD;
        history.macSetReqDescr.callback = macSetChannelCb;
        ZBPRO_MAC_SetReq(&history.macSetReqDescr);
        return;
    }
#endif

    SOC_RxStartEnergyDetect();

    SYS_TimeoutSignalStart(&history.timeoutSignal, TIMEOUT_TASK_REPEAT_MODE);

    {
        EXT_PeScanEdStartConfParams_t conf;
        conf.timeStart = HAL_GetSystemTime();
        conf.status = PE_SCAN_ED_STATUS_SUCCESS;
        reqDescr->callback(reqDescr, &conf);
    }
}

static void macSetChannelCb(MAC_SetReqDescr_t *const reqDescr, MAC_SetConfParams_t *const confParams)
{
#if defined(_MAC_CONTEXT_ZBPRO_)
    history.macSetReqDescr.params.attribute = MAC_RX_ON_WHEN_IDLE;
    history.macSetReqDescr.params.attributeValue.macRxOnWhenIdle = 1;
    history.macSetReqDescr.params.payload = SYS_EMPTY_PAYLOAD;
    history.macSetReqDescr.callback = macSetRxOnWhenIdleCb;
    ZBPRO_MAC_SetReq(&history.macSetReqDescr);
#endif
}

static void macSetRxOnWhenIdleCb(MAC_SetReqDescr_t *const reqDescr, MAC_SetConfParams_t *const confParams)
{
    SOC_RxStartEnergyDetect();

    SYS_TimeoutSignalStart(&history.timeoutSignal, TIMEOUT_TASK_REPEAT_MODE);

    {
        EXT_PeScanEdStartConfParams_t conf;
        conf.timeStart = HAL_GetSystemTime();
        conf.status = PE_SCAN_ED_STATUS_SUCCESS;
        history.startReq->callback(history.startReq, &conf);
    }
}

/* brief   Stop energy meter measurements. */
void EXT_PeScanEdStopReq(EXT_PeScanEdStopReqDescr_t *const reqDescr)
{
    SYS_TimeoutSignalStop(&history.timeoutSignal);

    {
        EXT_PeScanEdStopConfParams_t conf;
        conf.status = PE_SCAN_ED_STATUS_SUCCESS;
        reqDescr->callback(reqDescr, &conf);
    }
}

/* brief   Get energy meter history. */
void EXT_PeScanEdGetHistoryReq(EXT_PeScanEdGetHistoryReqDescr_t *const reqDescr)
{
    EXT_PeScanEdGetHistoryConfParams_t conf;
    extPeScanEdFillHistoryConf(&conf);
    reqDescr->callback(reqDescr, &conf);
}

static void extPeScanEdFillHistoryConf(EXT_PeScanEdGetHistoryConfParams_t *const conf)
{
    conf->status = PE_SCAN_ED_STATUS_SUCCESS;
    conf->count = 0;
    conf->payload = SYS_EMPTY_PAYLOAD;

    uint8_t buf[RECORD_SIZE * MAX_COUNT];
    for(uint16_t offset = 0; conf->count < MAX_COUNT;)
    {
        if (history.countStart == history.countStop && !history.isOverload)
            break;

        memcpy(&buf[offset],&history.timeStamp[history.countStart], 4);
        offset += 4;
        memcpy(&buf[offset],&history.energy[history.countStart], 4);
        offset += 4;

        conf->count++;
        history.countStart++;
        history.length--;
        if (history.countStart >= EXT_PeScanEdTABLE_COUNT_MAX)
            history.countStart = 0;
    }

    if (!SYS_MemAlloc(&conf->payload, conf->count * RECORD_SIZE))
    {
        conf->status = PE_SCAN_ED_STATUS_FAILURE;
    }
    else
    {
        SYS_CopyToPayload(&conf->payload, 0, buf, conf->count * RECORD_SIZE);

        if (history.countStart != history.countStop)
            conf->status = PE_SCAN_ED_STATUS_HAVE_DATA;

        if (history.isOverload)
        {
            conf->status = PE_SCAN_ED_STATUS_OVERLOAD;
            history.isOverload = false;
        }
    }
}

static void extPeEdTimecb(SYS_TimeoutTaskServiceField_t *const timeoutService)
{
    if (!history.isOverload)
    {
        int8_t rssi = SOC_GetLastRssi();
        history.energy[history.countStop] = (uint8_t)(128 + rssi);
        // history.energy[history.countStop] = RxState.Ed.EdOutput;
        history.timeStamp[history.countStop] = HAL_GetSystemTime();

        history.countStop++;
        history.length++;
        if (history.countStop >= EXT_PeScanEdTABLE_COUNT_MAX)
            history.countStop = 0;

        if (history.countStart == history.countStop)
        {
            history.isOverload = true;
        }
    }

    if (history.length > history.indicationThreshold)
    {
        EXT_PeScanEdGetHistoryConfParams_t conf;
        extPeScanEdFillHistoryConf(&conf);
        EXT_PeScanEdGetHistoryInd(&conf);
    }
}

#endif // _PE_ENERGY_MEASUREMENT_
