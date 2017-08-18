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
 *      PE scan energy measurement function interface.
 *
*******************************************************************************/

#ifndef BBEXTPESCANED_H_
#define BBEXTPESCANED_H_

/************************* INCLUDES *****************************************************/
#include "bbPhySapForMac.h"
#include "bbSysTime.h"
/******************************************************************************
                    Defines section
******************************************************************************/
#define EXT_PeScanEdTABLE_COUNT_MAX     300

/************************* DEFINITIONS **************************************************/

/*****************************************************************************************/
/* StartReq */
/*****************************************************************************************/
typedef struct _EXT_PeScanEdStartReqParams_t
{
    uint16_t      timeout;
    PHY_Channel_t channel;
    uint8_t       needSetup;
    uint16_t      indicationThreshold;
} EXT_PeScanEdStartReqParams_t;


typedef struct _EXT_PeScanEdStartConfParams_t
{
    uint8_t       status;
    SYS_Time_t    timeStart;
} EXT_PeScanEdStartConfParams_t;

typedef struct _EXT_PeScanEdStartReqDescr_t  EXT_PeScanEdStartReqDescr_t;

typedef void EXT_PeScanEdStartConfCallback_t(EXT_PeScanEdStartReqDescr_t *const reqDescr, EXT_PeScanEdStartConfParams_t *const confParams);

struct _EXT_PeScanEdStartReqDescr_t
{
    EXT_PeScanEdStartConfCallback_t *callback;
    EXT_PeScanEdStartReqParams_t     params;
};


/*****************************************************************************************/
/* StopReq */
/*****************************************************************************************/
typedef struct _EXT_PeScanEdStopConfParams_t
{
    uint8_t  status;
} EXT_PeScanEdStopConfParams_t;

typedef struct _EXT_PeScanEdStopReqDescr_t  EXT_PeScanEdStopReqDescr_t;

typedef void EXT_PeScanEdStopConfCallback_t(EXT_PeScanEdStopReqDescr_t *const reqDescr, EXT_PeScanEdStopConfParams_t *const confParams);

struct _EXT_PeScanEdStopReqDescr_t
{
    EXT_PeScanEdStopConfCallback_t *callback;
};


/*****************************************************************************************/
/* GetHistory */
/*****************************************************************************************/
typedef struct _EXT_PeScanEdGetHistoryConfParams_t
{
    uint8_t             status;
    uint32_t            count;
    SYS_DataPointer_t   payload;
} EXT_PeScanEdGetHistoryConfParams_t;

typedef struct _EXT_PeScanEdGetHistoryReqDescr_t  EXT_PeScanEdGetHistoryReqDescr_t;

typedef void EXT_PeScanEdGetHistoryConfCallback_t(EXT_PeScanEdGetHistoryReqDescr_t *const reqDescr, EXT_PeScanEdGetHistoryConfParams_t *const confParams);

struct _EXT_PeScanEdGetHistoryReqDescr_t
{
    EXT_PeScanEdGetHistoryConfCallback_t *callback;
};

/************************* PROTOTYPES ***************************************************/
/**//**
 * \brief   Start energy meter measurements.
 */
void EXT_PeScanEdStartReq(EXT_PeScanEdStartReqDescr_t *const reqDescr);


/**//**
 * \brief   Stop energy meter measurements.
 */
void EXT_PeScanEdStopReq(EXT_PeScanEdStopReqDescr_t *const reqDescr);


/**//**
 * \brief   Get energy meter history.
 */
void EXT_PeScanEdGetHistoryReq(EXT_PeScanEdGetHistoryReqDescr_t *const reqDescr);

void EXT_PeScanEdGetHistoryInd(EXT_PeScanEdGetHistoryConfParams_t *const ind);

#endif /* BBEXTPESCANED_H_ */

/* eof bbExtPeScanEd.h */
