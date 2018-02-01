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

 /* basp.h */

#ifndef _BASP_H_
#define _BASP_H_

/* basemodules includes */
#include "bstd.h"
#include "bkni.h"
#include "bmma.h"
#include "bint.h"
#include "btmr.h"

#include "bafl.h"
#include "basp_fw_api.h"

#ifdef __cplusplus
extern "C" {
#if 0
}
#endif
#endif

BDBG_FILE_MODULE(BASP_MEMORY);

#define BASP_MAX_NUMBER_OF_CHANNEL 32 /* TODO: See if this can be derived from RDB.*/

#define BASP_DEFAULT_STREAM_IN_DESCRIPTOR_TYPE 2

#define BASP_EDPKT_HEADER_BUFFER_SIZE 5*1024*1024 /* TODO: Later Anand will provide us a proper value.*/
#define BASP_EDPKT_HEADER_BUFFER_ALLIGNMENT 32

#define BASP_DEBUG_BUFFER_SIZE 5*1024 /* TODO: Later we can update it once fw team let us know a different size.*/
#define BASP_DEBUG_BUFFER_ALLIGNMENT 32

#define BASP_CVT_TO_ASP_REG_ADDR(addr)  ((addr) | 0xf0000000)
#define BASP_CVT_TO_HOST_REG_ADDR(addr)  ((addr) % 0x0fffffff)

/***************************************************************************
Summary:
ASP Device Handle
***************************************************************************/
typedef struct BASP_P_Device   *BASP_Handle;

/***************************************************************************
Summary:
ASP Device Context Handle
***************************************************************************/
typedef struct BASP_P_Context  *BASP_ContextHandle;

/***************************************************************************
Summary:
ASP Channel Handle
***************************************************************************/
typedef struct BASP_P_Channel  *BASP_ChannelHandle;

/***************************************************************************
Summary:
ASP Open Settings
***************************************************************************/
typedef struct BASP_OpenSettings
{
   uint32_t ui32Signature;/* [DO NOT MODIFY] Populated by BASP_GetDefaultOpenSettings() */

   BMMA_Heap_Handle hFirmwareHeap;  /* If NULL, system memory handle (hMem) used.
                                     * If non-NULL, heap should be created with "system bookkeeping"
                                     * to ensure firmware is loaded at the expected location. */

   /*********************/
   /* Firmware and Boot */
   /*********************/

   /* Interface to access firmware image. This interface must be
    * implemented and the function pointers must be stored here.
    */
   const BIMG_Interface *pImgInterface;

   /* Context for the Image Interface. This is also provided by
    * the implementation of the Image interface
    */
   const void* const *pImgContext;

   /* Add Authenticated image support */

   /* Pointer to the Asp boot callback function. If non-NULL,
    * PI will call this function after the firmware has been
    * loaded into memory instead of booting the Asp. If this
    * function is NULL, then the PI will boot the Asp
    * normally. This function should return BERR_SUCCESS if
    * successful.
    */
   BERR_Code (*pBootCallback)(void* pPrivateData,
                                 const BAFL_BootInfo *pstBootInfo);

   /* Pointer to Asp boot callback private data that is passed
    * back into the callback.  Can be used to store any
    * information necessary for the application to boot the
    * core.
    */
   void *pBootCallbackData;

   /* If pAspBootCallback is non-NULL, then by default, the
       * asp PI does NOT explicitly boot the core. However, if
       * bAspBootEnable is true, then the Asp PI will boot the
       * core AFTER the pAspBootCallback has been executed.
       * Note: If pAspBootCallback is NULL, Asp PI always boots
       * the core.
       */
   bool bBootEnabled;

   uint64_t ui64StatusBufferDeviceOffset;
   uint32_t ui32StatusBufferSize;
} BASP_OpenSettings;

/***************************************************************************
Summary:
Get Default Open Settings for an ASP Device
***************************************************************************/
void BASP_GetDefaultOpenSettings(
    BASP_OpenSettings  *pstOpenSettings /* [out] Default ASP settings */
    );

/***************************************************************************
Summary:
Open an ASP Device
***************************************************************************/
BERR_Code BASP_Open(
    BASP_Handle *phAsp, /* [out] ASP Device handle returned */
    BCHP_Handle hChp,   /* [in] Chip handle */
    BREG_Handle hReg,   /* [in] Register handle */
    BMMA_Heap_Handle hMem,   /* [in] System Memory handle */
    BINT_Handle hInt,   /* [in] Interrupt handle */
    BTMR_Handle hTmr,   /* [in] Timer handle */
    const BASP_OpenSettings *pOpenSettings /* [in] ASP Device Open settings */
    );

/***************************************************************************
Summary:
Close an ASP Device
***************************************************************************/
BERR_Code BASP_Close(
    BASP_Handle hAsp
    );

/***************************************************************************
Summary:
Context Types
***************************************************************************/
typedef enum BASP_ContextType
{
    BASP_ContextType_eStreaming,
    BASP_ContextType_eMux,
    BASP_ContextType_eMax
} BASP_ContextType;

/***************************************************************************
Summary:
Context Create Settings
***************************************************************************/
typedef struct BASP_ContextCreateSettings
{
    BASP_ContextType type;
} BASP_ContextCreateSettings;

/***************************************************************************
Summary:
Get Default Context Create Settings
***************************************************************************/
void BASP_Context_GetDefaultCreateSettings(
    BASP_ContextCreateSettings *pSettings /* [out] */
    );

/***************************************************************************
Summary:
Create an ASP device context

Description:
An ASP device context is required for a caller to create and manage ASP
channels and interact with the ASP device.
***************************************************************************/
BERR_Code BASP_Context_Create(
  BASP_Handle hAsp,
  const BASP_ContextCreateSettings *pSettings,
  BASP_ContextHandle *pHandle /* [out] */
  );

/***************************************************************************
Summary:
Destroy an ASP context
***************************************************************************/
void BASP_Context_Destroy(
  BASP_ContextHandle hContext
  );

/***************************************************************************
Summary:
Callbacks for an ASP context
***************************************************************************/
typedef struct BASP_ContextCallbacks
{
    struct {
        void (*pCallback_isr)(void *pParam1, int param2);
        void *pParam1;
        int param2;
    } watchdog;
} BASP_ContextCallbacks;

/***************************************************************************
Summary:
Get currently registered callbakcks from an ASP context
***************************************************************************/
void BASP_Context_GetCallbacks(
  BASP_ContextHandle hContext,
  BASP_ContextCallbacks *pCallbacks /* [out] */
  );

/***************************************************************************
Summary:
Set callbacks for an ASP context
***************************************************************************/
BERR_Code BASP_Context_SetCallbacks(
  BASP_ContextHandle hContext,
  const BASP_ContextCallbacks *pCallbacks
  );

/***************************************************************************
Summary:
When the caller receives a watchdog callback from BASP, it must clean up
its state and call this routine to tell BASP to reset the device.  The device
will reset only after all contexts call this routine.
***************************************************************************/
BERR_Code BASP_Context_ProcessWatchdog(
  BASP_ContextHandle hContext
  );

/***************************************************************************
Summary:
Channel Settings
***************************************************************************/
typedef struct BASP_ChannelCreateSettings
{
    int channelNumber;
} BASP_ChannelCreateSettings;

/***************************************************************************
Summary:
Get default channel settings
***************************************************************************/
void BASP_Channel_GetDefaultCreateSettings(
  BASP_ChannelCreateSettings *pSettings /* [out] */
  );

/***************************************************************************
Summary:
Create an ASP channel
***************************************************************************/
BERR_Code BASP_Channel_Create(
  BASP_ContextHandle hContext,
  const BASP_ChannelCreateSettings *pSettings,
  BASP_ChannelHandle *pHandle
  );

/***************************************************************************
Summary:
Destroy an ASP channel
***************************************************************************/
void BASP_Channel_Destroy(
  BASP_ChannelHandle hChannel
  );

/***************************************************************************
Summary:
ASP Channel Callbacks
***************************************************************************/
typedef struct BASP_ChannelCallbacks
{
    struct {
        void (*pCallback_isr)(void *pParam1, int param2);
        void *pParam1;
        int param2;
    } messageReady;
} BASP_ChannelCallbacks;

/***************************************************************************
Summary:
Get currently registered channel callbacks
***************************************************************************/
void BASP_Channel_GetCallbacks(
  BASP_ChannelHandle hChannel,
  BASP_ChannelCallbacks *pCallbacks /* [out] */
  );

/***************************************************************************
Summary:
Set callbacks for an ASP channel
***************************************************************************/
BERR_Code BASP_Channel_SetCallbacks(
  BASP_ChannelHandle hChannel,
  const BASP_ChannelCallbacks *pCallbacks
  );

/***************************************************************************
Summary:
Send a message to an ASP channel
***************************************************************************/
BERR_Code BASP_Channel_SendMessage(
  BASP_ChannelHandle hChannel,
  BASP_MessageType messageType,
  BASP_ResponseType ResponseType,
  BASP_Pi2Fw_Message *pMessage
  );

/***************************************************************************
Summary:
Read a message sent from an ASP channel
***************************************************************************/
BERR_Code BASP_Channel_ReadMessage(
  BASP_ChannelHandle hChannel,
  BASP_MessageType *pType,
  BASP_Fw2Pi_Message *pMessage,
  unsigned *pMessageLength
  );
BERR_Code BASP_Channel_ReadMessage_isr(
  BASP_ChannelHandle hChannel,
  BASP_MessageType *pType,
  BASP_Fw2Pi_Message *pMessage,
  unsigned *pMessageLength
  );

#ifdef __cplusplus
}
#endif

#endif /* _BASP_H_ */
