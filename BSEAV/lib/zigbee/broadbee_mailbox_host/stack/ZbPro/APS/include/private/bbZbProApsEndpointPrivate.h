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

/******************************************************************************
 *
 * DESCRIPTION:
 *      Private interface of the APS Endpoint component
 *
*******************************************************************************/

#ifndef _ZBPRO_APS_ENDPOINT_PRIVATE_H
#define _ZBPRO_APS_ENDPOINT_PRIVATE_H

/************************* INCLUDES ****************************************************/
#include "bbZbProApsSapEndpoint.h"
#include "bbZbProApsEndpointPrivateTypes.h"
/************************* DEFINITIONS *************************************************/

#define ZBPRO_APS_ENDPOINT_TABLE_ROWS       ZBPRO_APS_NODE_ENDPOINTS
#define ZBPRO_APS_ENDPOINT_TABLE_BUSY_BIT   0

/**************************** TYPES ****************************************************/

/**//**
 * \brief Endpoint table entry
 */
typedef struct _ZbProApsEndpointTableEntry_t
{
    zbProApsEndpointRegisterReqParams_t   endpointDesc;
    bool                                    isBusy;
} ZbProApsEndpointTableEntry_t;
SYS_DbgAssertStatic(0 == offsetof(ZbProApsEndpointTableEntry_t, endpointDesc));

/**//**
 * \brief Endpoint component service descriptor.
 */
typedef struct _ZbProApsEndpointDescriptor_t
{
    ZbProApsEndpointTableEntry_t    table[ZBPRO_APS_ENDPOINT_TABLE_ROWS];
    SYS_QueueDescriptor_t           registerQueue;
    SYS_QueueDescriptor_t           unregisterQueue;
} ZbProApsEndpointDescriptor_t;


/**//**
 * \brief Types for callback-function, which will be called after save/load of the EndpointTable will be
 * finished.
*/
typedef void (* ZbProApsEndpointTableSaveLoadCallback_t)(void *cbParam);

/**//**
 * \brief Descriptor of the EndpointTable saving and loading process.
  * It contains a curernt ID in the NVM Parameters and currentRow in EndpointTable.
*/
typedef struct _ZbProApsEndpointTableSaveLoadDescriptor_t
{
    uint8_t currentID;
    uint8_t currentRow;
    ZbProApsEndpointTableSaveLoadCallback_t callback;
    void *cbParam;
} ZbProApsEndpointTableSaveLoadDescriptor_t;

/************************* FUNCTION PROTOTYPES ******************************************/

/**//**
 * \brief Initializes Endpoint table
 */
APS_PRIVATE void zbProApsEndpointReset(void);

/**//**
 * \brief Stops Endpoint table
 */
APS_PRIVATE void zbProApsEndpointStop(void);

/**//**
 * \brief Returns true if the module data is clean as after reset
 */
APS_PRIVATE bool zbProApsEndpointIsFresh(void);

/**//**
 * \brief Gets a registered endpoint list entry which is next to the specified one
 */
APS_PRIVATE bool ZbProApsEndpointGetNext(zbProApsEndpointRegisterReqParams_t **endpointDesc);

/**//**
 * \brief APS Endpoint_Register request primitive task handler function
 */
APS_PRIVATE void zbProApsEndpointHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);

/**//**
 * \brief APS Endpoint_Register request primitive task handler function
 * \param[in] endpoint  The endpoint identifier, from 0x00 to 0xFF.
 * \return  The apsMaxWindowSize value, from 0 to 8.
 * \details For the case when \p endpoint belongs to the range 0x01..0xFE this function returns the apsMaxWindowSize
 *  value that was specified originally with the endpoint registration request. It may be either 0, if the given
 *  endpoint disables fragmentation, or from 1 to 8 if fragmentation is enabled. If there is no registered (active)
 *  endpoint with the given identifier in the range 0x01..0xFE, this function returns 0 which means 'fragmentation is
 *  disabled' for nonexisting endpoint.
 * \details For the case when \p endpoint equals 0x00, the ZDO/ZDP profile endpoint, this function unconditionally
 *  returns 0, which means that fragmentation is disabled for ZDO/ZDP. See ZigBee Spec. r20, 2.5.3: <...> ZigBee Device
 *  Objects shall employ Endpoint 0. <...> ZDO frames shall not be fragmented.
 * \details For the case when \p endpoint equals 0xFF, the broadcast endpoint, this function looks through all the
 *  registered (active) endpoints, except the endpoint 0x00, and discovers whether all of them enable fragmentation and
 *  whether all of them have the same value of the apsMaxWindowSize. If at least one of registered endpoints disables
 *  fragmentation, or if at least one of them has different apsMaxWindowSize, this function returns 0, which means that
 *  in such situation fragmentation is disabled for frames delivered on the broadcast endpoint. If all endpoints (except
 *  the endpoint 0x00) allows fragmentation and if all of them have the same apsMaxWindowSize from 1 to 8, this function
 *  returns that apsMaxWindowSize value.
 */
APS_PRIVATE ZBPRO_APS_EndpointMaxWindowSize_t zbProApsEndpointWindowSizeGet(const ZBPRO_APS_EndpointId_t endpoint);

/**//**
 * \brief Pair of the starting of the EndpointTalbe saving/load process.
 * It function will save/load a base (constant-size) part of the EndpointTable and
 * starts saving/loading of the volatile-size part - clusters list for each
 * used endpoint.
*/
APS_PRIVATE void zbProApsEndpointSaveTableStart(ZbProApsEndpointTableSaveLoadCallback_t callback, void *cbParam);
APS_PRIVATE void zbProApsEndpointLoadTableStart(ZbProApsEndpointTableSaveLoadCallback_t callback, void *cbParam);


#endif /* _ZBPRO_APS_ENDPOINT_PRIVATE_H */

/* eof bbZbProApsEndpointPrivate.h */