/***************************************************************************
*     (c)2012-2013 Broadcom Corporation
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
* API Description:
* API name: DOCSIS
* DOCSIS module private APIs
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#ifndef NEXUS_DOCSIS_PRIV_H__
#define NEXUS_DOCSIS_PRIV_H__

#include "nexus_docsis_priv_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 +           DOCSIS device specific private APIs                          +
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/***************************************************************************
Summary: 
Host creates a notification thread. This thread is responsible for 
processing the notification events from DOCSIS through polling mechanism. 
This thread has no association with NEXUS_Frontend or NEXUS_FrontDevice. 
***************************************************************************/
void NEXUS_Docsis_P_NotificationThread(void *arg);

/***************************************************************************
Summary: 
Host creates a heart beat thread. This thread is responsbile for detecting 
whether DOCSIS device is alive or dead and for resetting the DOCSIS channels 
in case of transitions like live->dead->live. 
This thread has no association with NEXUS_Frontend or NEXUS_FrontDevice. 
***************************************************************************/
void NEXUS_Docsis_P_HeartBeatThread(void * arg);


/***************************************************************************
Summary:
    close/Open all the host controlled docsis channels after a DOCSIS device reset.
    This API has no association with NEXUS_Frontend or NEXUS_FrontDevice.
***************************************************************************/
NEXUS_Error NEXUS_Docsis_P_Reset(NEXUS_DocsisDeviceHandle hDevice);

/***************************************************************************
Summary:
    Mapped to NEXUS_FrontendDevice->close. Invoked when app calls
    NEXUS_FrontendDevice_Close.
***************************************************************************/
void NEXUS_Docsis_P_CloseDevice(
    void *handle
    );

/***************************************************************************
Summary:
    Mapped to NEXUS_FrontendDevice->getStatus. Invoked when app calls
    NEXUS_FrontendDevice_GetStatus.
***************************************************************************/

NEXUS_Error NEXUS_Docsis_P_GetStatus(
    void *handle,
     NEXUS_FrontendDeviceStatus *pStatus);

/***************************************************************************
Summary:
    Mapped to NEXUS_FrontendDevice->getDocsisLnaDeviceAgcValue.
    This API will be invoked by non-DOCSIS QAM devices's API
    whose LNA device is controlled by DOCSIS. non-DOCSIS QAM device
    API shall extract the agc value from DOCSIS and program
    it into non-DOCSIS QAM device at tune time and also when
    lock status is changed to unlocked from locked state.
***************************************************************************/
NEXUS_Error NEXUS_Docsis_P_GetDocsisLnaDeviceAgcValue(
    void *handle,
     uint32_t *agcVal);

/***************************************************************************
Summary:
    Mapped to NEXUS_FrontendDevice->setHostChannelLockStatus.
    This API will be invoked by non-DOCSIS QAM devices's API
    whose LNA device is controlled by DOCSIS. non-DOCSIS QAM device
    API shall call into this API to let the DOCSIS device know
    of the host channel lock status. If any one of the
    host channels is locked and NEXUS_Docsis_ConfigureDeviceLna API
    is called then DOCSIS would ignore the request to configure
    the LNA as it would disrupt the data flow to host channels.
***************************************************************************/
NEXUS_Error NEXUS_Docsis_P_SetHostChannelLockStatus(
    void *handle,
    unsigned channelNum,
    bool locked);


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 +           DOCSIS channel specific private APIs                          +
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/***************************************************************************
Summary:
    Mapped to NEXUS_Frontend->close. Invoked when
    app calls NEXUS_Frontend_Close API.
***************************************************************************/
void NEXUS_Docsis_P_CloseChannel(
   NEXUS_FrontendHandle handle
   );

/***************************************************************************
Summary:
    Mapped to NEXUS_Frontend->getFastStatus. Invoked when
    app calls NEXUS_Frontend_GetFastStatus API. This is applicable only
    for QAM and OOB channels.
***************************************************************************/
NEXUS_Error NEXUS_Docsis_P_GetFastStatus(
   void *handle,
   NEXUS_FrontendFastStatus *pStatus
   );

/***************************************************************************
Summary:
    Mapped to NEXUS_Frontend->untune. Invoked when
    app calls NEXUS_Frontend_Untune API.
***************************************************************************/
void NEXUS_Docsis_P_Untune(
    void *handle
    );

/***************************************************************************
Summary:
       Mapped to NEXUS_Frontend->tune. Invoked when
       app calls NEXUS_Frontend_TuneQam API.
***************************************************************************/
NEXUS_Error NEXUS_Docsis_P_TuneQam(
    void *handle,
    const NEXUS_FrontendQamSettings *pSettings
    );

/***************************************************************************
Summary:
    Mapped to NEXUS_Frontend->getQamStatus. Invoked when
   app calls NEXUS_Frontend_GetQamStatus API.
***************************************************************************/
NEXUS_Error NEXUS_Docsis_P_GetQamStatus(
    void *handle,
    NEXUS_FrontendQamStatus *pStatus /* [out] */
    );

/***************************************************************************
Summary:
     Mapped to NEXUS_Frontend->requestQamAsyncStatus. Invoked when
     app calls NEXUS_Frontend_RequestQamAsyncStatus API.
***************************************************************************/
NEXUS_Error NEXUS_Docsis_P_RequestQamAsyncStatus(
    void *handle
    );

/***************************************************************************
Summary:
    Mapped to NEXUS_Frontend->getQamAsyncStatus. Invoked when
    app calls NEXUS_Frontend_GetQamAsyncStatus API.
 ***************************************************************************/
NEXUS_Error NEXUS_Docsis_P_GetQamAsyncStatus(
    void *handle,
    NEXUS_FrontendQamStatus *pStatus
    );

/***************************************************************************
Summary: 
 DOCSIS notification thread receives a lock change event after
 a tune command for QAM channel is sent. BDCM_ADS module
 module processes the event and invokes this API.
 This API in turn invokes the app specified callback passed through
 NEXUS_Frontend_TuneQam API settings.
***************************************************************************/
void NEXUS_Docsis_P_QamLockStatus(
	void *context
	);

/***************************************************************************
Summary: 
Upon DOCSIS reset, all the DOCSIS channels are closed and opened and 
tuned again to a frequency that was sent by the application before reset. 
This API is invoked after DOCSIS reset periodically to check on 
the lock status and send the notification to the application through 
app specified callbacks. 
***************************************************************************/
void NEXUS_Docsis_P_CheckQamTuneStatus(
    void *handle
    );

/***************************************************************************
Summary:
    Mapped to NEXUS_Frontend->tuneOutOfBand. Invoked when
    app calls NEXUS_Frontend_TuneOutOfBand API. 
***************************************************************************/
NEXUS_Error NEXUS_Docsis_P_TuneOutOfBand(
    void *handle,
    const NEXUS_FrontendOutOfBandSettings *pSettings
    );

/***************************************************************************
Summary: 
 DOCSIS notification thread receives a lock change event after
 a tune command for an OOB channel is sent. BDCM_Aob module
 module processes the event and invokes this API.
 This API in turn invokes the app specified callback passed through
 NEXUS_Frontend_TuneOutOfBand API settings.
***************************************************************************/
void NEXUS_Docsis_P_OobLockStatus(
	void *context
	);

/***************************************************************************
Summary: 
    Mapped to NEXUS_Frontend->getOutOfBandStatus. Invoked when
    app calls NEXUS_Frontend_GetOutofBandStatus API. 
***************************************************************************/
NEXUS_Error NEXUS_Docsis_P_GetOutOfBandStatus(
    void *handle,
    NEXUS_FrontendOutOfBandStatus *pStatus 
    );

/***************************************************************************
Summary:
    Mapped to NEXUS_Frontend->requestOutOfBandAsyncStatus. Invoked when
    app calls NEXUS_Frontend_RequestOutOfBandAsyncStatus API. 
***************************************************************************/
NEXUS_Error NEXUS_Docsis_P_RequestOutOfBandAsyncStatus(
    void *handle
    );

/***************************************************************************
Summary:
    Mapped to NEXUS_Frontend->getOutOfBandAsyncStatus. Invoked when
    app calls NEXUS_Frontend_GetOutOfBandAsyncStatus API. 
***************************************************************************/
NEXUS_Error NEXUS_Docsis_P_GetOutOfBandAsyncStatus(
    void *handle,
    NEXUS_FrontendOutOfBandStatus *pStatus /* [out] */
    );

/***************************************************************************
Summary:
    Mapped to NEXUS_Frontend->tuneUpStream. Invoked when
    app calls NEXUS_Frontend_tuneUpstream API. 
***************************************************************************/
NEXUS_Error NEXUS_Docsis_P_TuneUpstream(
    void *handle,
    const NEXUS_FrontendUpstreamSettings *pSettings
    );

/***************************************************************************
Summary: 
    Mapped to NEXUS_Frontend->getUpstreamStatus. Invoked when
    app calls NEXUS_Frontend_GetUpstreamStatus API. 
***************************************************************************/
NEXUS_Error NEXUS_Docsis_P_GetUpstreamStatus(
    void *handle,
    NEXUS_FrontendUpstreamStatus *pStatus
    );

/***************************************************************************
Summary:
    Mapped to NEXUS_Frontend->transmitDebugPacket. Invoked when
    app calls NEXUS_Frontend_TransmitDebugPacket API. 
************************************************************************/
NEXUS_Error NEXUS_Docsis_P_TransmitDebugPacket(
   void *handle,
   NEXUS_FrontendDebugPacketType type,
   const uint8_t *pBuffer,
   size_t size
   );

/***************************************************************************
Summary:
    Mapped to NEXUS_Frontend->getSoftDecisions. Invoked when
    app calls NEXUS_Frontend_GetSoftDecisions API. 
************************************************************************/
NEXUS_Error NEXUS_Docsis_P_GetSoftDecisions(
    void *handle,
    NEXUS_FrontendSoftDecision *pDecisions,
    size_t length
    );

/***************************************************************************
Summary:
    Mapped to NEXUS_Frontend->reapplyTransportSettings. Invoked when
    app calls NEXUS_Frontend_ReapplyTransportSettings API. 
************************************************************************/
NEXUS_Error NEXUS_Docsis_P_ReapplyChannelTransportSettings(
	void *handle
	);

/***************************************************************************
Summary:
    Mapped to NEXUS_Frontend->standby. Invoked when
    NEXUS internal NEXUS_Frontend_Standby_priv API is invoked.
************************************************************************/
NEXUS_Error NEXUS_Docsis_P_ChannelStandby(
    void *handle,
    bool enabled,
    const NEXUS_StandbySettings *pSettings);

/***************************************************************************
Summary:
    Mapped to NEXUS_Frontend->getType. Invoked when
    NEXUS_Frontend_GetType API is invoked.
************************************************************************/
void NEXUS_Docsis_P_GetType(void *handle,
                            NEXUS_FrontendType *type);


/***************************************************************************
Summary:
    Mapped to NEXUS_Frontend->getQamScanStatus. Invoked when
    NEXUS_Frontend_GetQamScamStatus API is invoked.
************************************************************************/
NEXUS_Error NEXUS_Docsis_P_GetQamScanStatus(
    void *handle,
    NEXUS_FrontendQamScanStatus *pStatus
    );


#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_DOCSIS_PRIV_H__ */

