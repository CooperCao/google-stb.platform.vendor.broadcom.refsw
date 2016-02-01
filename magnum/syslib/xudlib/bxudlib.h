/***************************************************************************
 *     Copyright (c) 2003-2012, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
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
typedef BERR_Code (*BXUDlib_UserDataSink_Add)(void *pPrivateSinkContext, const BUDP_Encoder_FieldInfo *astUserData, size_t uiCount, size_t *puiQueuedCount);
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
BERR_Code
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
