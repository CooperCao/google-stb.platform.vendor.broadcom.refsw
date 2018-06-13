/***************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * [File Description:]
 *
 ***************************************************************************/

#ifndef BXUDLIB_H_
#define BXUDLIB_H_

#include "berr.h"
#include "berr_ids.h"
#include "budp_vce.h"
#if B_REFSW_DSS_SUPPORT
#include "budp_dccparse_dss.h"
#endif
#include "bvdc.h"
#include "bavc_vce.h"

#ifdef __cplusplus
extern "C" {
#if 0
}
#endif
#endif

/*****************************************************************************
	Module specific standard BERR codes
 *****************************************************************************/
#define BERR_BXUD_NO_DATA         BERR_MAKE_CODE(BERR_XUDlib_ID, 0x0000)

typedef struct BXUDlib_P_Context *BXUDlib_Handle;


typedef struct BXUDlib_CreateSettings
{
    uint32_t queueSize;
} BXUDlib_CreateSettings;

/* The User data sink interface mimics the interface provided by the VCE */
typedef BERR_Code (*BXUDlib_UserDataSink_Add)(void *pPrivateSinkContext, const BUDP_Encoder_FieldInfo *astUserData, unsigned uiCount, unsigned *puiQueuedCount);
typedef BERR_Code (*BXUDlib_UserDataSink_Status)(void *pPrivateSinkContext, BAVC_VideoUserDataStatus *pstUserDataStatus);

typedef struct BXUDlib_UserDataSink_Interface
{
    void *pPrivateSinkContext;
    BXUDlib_UserDataSink_Add userDataAdd_isr; 
    BXUDlib_UserDataSink_Status userDataStatus_isr;
} BXUDlib_UserDataSink_Interface;

typedef struct BXUDlib_Settings
{
    BXUDlib_UserDataSink_Interface sinkInterface; 
} BXUDlib_Settings;


BERR_Code
BXUDlib_Create(BXUDlib_Handle *phXud, const BXUDlib_CreateSettings *pstXudCreateSettings); 

void
BXUDlib_Destroy(BXUDlib_Handle hXud);

void
BXUDlib_GetDefaultCreateSettings(BXUDlib_CreateSettings *pstXudCreateSettings);

/* BXUDlib_SetSettings is called to change the config settings. When the outputPacketType 
   is changed, the library will flush its queue of cc packets and start afresh.
   The library will discard all incoming user data if the sink interface is not set.
   If the sink interface is set, the library will perform the appropriate rate 
   conversion and feed the user data to the sink interface at the STG rate

   BXUDlib_GetSettings can be called prior to SetSettings to get the default config settings.
*/
void
BXUDlib_SetSettings(BXUDlib_Handle hXud, const BXUDlib_Settings *pstXudSettings); 

void
BXUDlib_GetSettings(BXUDlib_Handle hXud, BXUDlib_Settings *pstXudSettings); 

/* BXUDlib_UserDataHandler_isr is called to provide XUD with user data packets (only closed 
   captioning is used for now). All packets are associated with a "decode" picture id. XUD copies and queues the data 
   provided in this callback for later processing.   
*/
BERR_Code
BXUDlib_UserDataHandler_isr(BXUDlib_Handle hXud, const BAVC_USERDATA_info *pstUserData);

/* BXUDlib_DisplayInterruptHandler_isr is the heartbeat of the XUD library where all the work is done.  
   XUD gets the user data from the source for uiDecodePictureId via the BXUDlib_UserDataCallback_isr.
   On this call, XUD will appropriately redistribute the user data packets to the STG rate (rate conversion).
   XUD will add the rate converted user data to the sink per uiStgPictureId.
*/
BERR_Code
BXUDlib_DisplayInterruptHandler_isr(BXUDlib_Handle hXud, const BVDC_Display_CallbackData *pstDisplaytCallbackData);



#ifdef __cplusplus
}
#endif

#endif /* BXUDLIB_H_ */
