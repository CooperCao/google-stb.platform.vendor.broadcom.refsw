/***************************************************************************
 *     (c)2007-2011 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 * Data Injection service shall provide APIs for injecting data in to an
 * ES substream using hardware packet substitution.
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ****************************************************************************/
#ifndef _B_DVR_DATAINJECTIONSERVICE_H
#define _B_DVR_DATAINJECTIONSERVICE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "b_dvr_const.h"
#include "b_dvr_datatypes.h"

BDBG_OBJECT_ID_DECLARE(B_DVR_DataInjectionService);     
/*****************************************************************************
Summary:
B_DVR_DataInjectionService_Open shall open the resources for a data injection
service instance.
******************************************************************************/
B_DVR_DataInjectionServiceHandle B_DVR_DataInjectionService_Open(
    B_DVR_DataInjectionServiceOpenSettings *pOpenSettings);

/******************************************************************************
Summary:
B_DVR_DataInjectionService_Close shall close the resources for a data injection
service instance.
Param[in] 
dataInjectionService - Handle for a data injection service instance.
Param[out]
B_DVR_ERROR  - Error code returned
******************************************************************************/
B_DVR_ERROR B_DVR_DataInjectionService_Close(
    B_DVR_DataInjectionServiceHandle dataInjectionService);

/********************************************************************************
Summary:
B_DVR_DataInjectionService_GetSettings shall get the settings for a dataInjection
service instance
Param[in]
B_DVR_DataInjectionServiceHandle - Handle for a data injection service instance.
Param[out]
B_DVR_DataInjectionServiceSetting - Settings returned
Param[out]
B_DVR_ERROR - Error code returned.
********************************************************************************/
B_DVR_ERROR B_DVR_DataInjectionService_GetSettings(
    B_DVR_DataInjectionServiceHandle dataInjection,
    B_DVR_DataInjectionServiceSettings *pSettings);

/********************************************************************************
Summary:
B_DVR_DataInjectionService_SetSettings shall set the settings for dataInjection
service instance
Param[in]
B_DVR_DataInjectionServiceHandle - Handle for a data injection service instance.
Param[in]
B_DVR_DataInjectionServiceSetting - Settings passed in.
Param[out]
B_DVR_ERROR - Error code returned.
********************************************************************************/
B_DVR_ERROR B_DVR_DataInjectionService_SetSettings(
    B_DVR_DataInjectionServiceHandle dataInjection,
    B_DVR_DataInjectionServiceSettings *pSettings);


/********************************************************************************
Summary:
B_DVR_DataInjectionService_Start shall start a dataInjection service instance.
Param[in]
B_DVR_DataInjectionServiceHandle - Handle for a data injection service instance
Param[out]
B_DVR_ERROR - Error code returned.
********************************************************************************/

B_DVR_ERROR B_DVR_DataInjectionService_Start(
    B_DVR_DataInjectionServiceHandle dateInjection,
    NEXUS_PidChannelHandle pidChannel,
    uint8_t *buf,
    unsigned size);

/********************************************************************************
Summary:
B_DVR_DataInjectionService_Stop shall stop the dataInjection service instance.
Param[in]
B_DVR_DataInjectionServiceHandle - Handle for a data injection service instance
Param[out]
B_DVR_ERROR - Error code returned.
********************************************************************************/
B_DVR_ERROR B_DVR_DataInjectionService_Stop(
    B_DVR_DataInjectionServiceHandle dataInjection);

/***************************************************************************
Summary:
B_DVR_DataInjectionService_InstallCallback shall add an application provided
callback to a dataInjection service instance.
Param[in]
dataInjectionService - Handle for a dataInjection service instance.
Param[in]
registeredCallback - Application provided callback
Param[in]
appContext - Application context
Param[out]
B_DVR_ERROR - Error code returned
*****************************************************************************/
B_DVR_ERROR B_DVR_DataInjectionService_InstallCallback(
    B_DVR_DataInjectionServiceHandle dataInjectionService,
    B_DVR_ServiceCallback registeredCallback,
    void * appContext
    );

/***************************************************************************
Summary:
B_DVR_DataInjectionService_RemoveCallback shall remove the application provided
callback from a dataInjection instance.
Param[in]
dataInjectionService - Handle for a dataInjection service instance.
Param[out]
B_DVR_ERROR - Error code returned
*****************************************************************************/
B_DVR_ERROR B_DVR_DataInjectionService_RemoveCallback(
    B_DVR_DataInjectionServiceHandle dataInjectionService);

/****************************************************************************
Summary:
B_DVR_DataInjectionService_GetChannelIndex shall HW PSUB channel associated
with a datainjection service.
*****************************************************************************/
unsigned B_DVR_DataInjectionService_GetChannelIndex(
    B_DVR_DataInjectionServiceHandle dataInjectionService);

#ifdef __cplusplus
}
#endif

#endif /* B_DVR_DATAINJECTIONSERVICE */

