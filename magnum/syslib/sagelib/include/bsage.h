/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
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
 ******************************************************************************/

#ifndef BSAGE_H_
#define BSAGE_H_

#include "breg_mem.h"
#include "bchp.h"
#include "bint.h"
#include "btmr.h"
#include "bhsm.h"

#include "bsagelib_interfaces.h"
#include "bsagelib_client.h"
#include "bsagelib_boot.h"
#include "priv/bsagelib_rpc_shared.h"

/* Host to Sage communication buffers size */
#ifndef SAGE_HOST_BUF_SIZE
#define SAGE_HOST_BUF_SIZE (32)
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum{
    BSAGE_Event_msgIndication,      /* triggered by msg from Sage BSAGElib_RpcMessage_eResponse */
                                    /* callback function: BSAGElib_Rpc_IndicationRecvCallback indicationRecv_isr  */

    BSAGE_Event_msgResponse,        /* triggered by msg from Sage BSAGElib_RpcMessage_eIndication */
                                    /* callback function: BSAGE_Rpc_DispatchResponse_isr response_isr */

    BSAGE_Event_msgCallbackRequest, /* triggered by msg from Sage BSAGElib_RpcMessage_eCallbackRequest */
                                    /* callback function: BSAGElib_Rpc_CallbackRequestISRCallback callbackRequest_isr */

    BSAGE_Event_msgTATermination,   /* triggered by msg from Sage BSAGElib_RpcMessage_eTATerminate */
                                    /* callback function: BSAGElib_Rpc_TATerminateCallback taTerminate_isr */

    BSAGE_Event_watchdog            /* triggered by Sage Watchdog event */
                                    /* callback function: */

} SAGE_Event;

/* This handle is used to store the context of a SAGE instance */
typedef struct BSAGE_P_Instance          *BSAGE_Handle;
typedef struct BSAGE_P_RpcRemote         *BSAGE_RpcRemoteHandle;

typedef void (*BSAGE_Callback)(void *context);
typedef void (*BSAGE_Rpc_DispatchResponse_isr)(BSAGElib_RpcRemoteHandle remote,BERR_Code response_rc);

typedef struct {
    /* Called under ISR */
    BSAGElib_Rpc_IndicationRecvCallback indicationRecv_isr;
    void *indicationContext;

    BSAGE_Rpc_DispatchResponse_isr response_isr;
    void *responseContext;

    BSAGElib_Rpc_CallbackRequestISRCallback callbackRequest_isr;/* should not be used - semi-private API for two ways communication */
    void *callbackRequestContext;

    BSAGElib_Rpc_TATerminateCallback taTerminate_isr; /* received if TA is terminated uppon error on SAGE-side */
    void *taTerminationContext;

} BSAGE_RpcInterface;

typedef struct {
    BREG_Handle hReg;
    BINT_Handle hInt;
    BTMR_Handle hTmr;

    /* default interfaces */
    BSAGElib_MemoryMapInterface i_memory_map;
    BSAGElib_MemorySyncInterface i_memory_sync;
    BSAGElib_SyncInterface i_sync_sage; /* Sync can be overriden by instance */
    BSAGElib_SyncInterface i_sync_hsm; /* Sync can be overriden by instance */

    uint8_t enablePinmux;

} BSAGE_Settings;

typedef struct
{
    /* The image buffers shall be allocated in a global region (general heap or other global heaps)    */
    uint32_t bootloaderOffset; /* SAGE bootloader image loaded into memory.                    */
    uint32_t bootloaderSize;

    uint32_t frameworkOffset;  /* SAGE Framework image loaded into memory.                        */
    uint32_t frameworkSize;

    uint32_t bootloaderDevOffset; /* SAGE bootloader image loaded into memory.                    */
    uint32_t bootloaderDevSize;

    uint32_t frameworkDevOffset;  /* SAGE Framework image loaded into memory.                        */
    uint32_t frameworkDevSize;

    /* Buffer holding the parameters of SAGE log buffer*/
    uint32_t logBufferOffset;
    uint32_t logBufferSize;

    uint32_t HSIBufferOffset;

    /* Regions map; memory block that mus be accessible by SAGE-side
       (see bsagelib_shared_globalsram.h for more details) */
    uint32_t regionMapOffset;
    uint32_t regionMapNum;

    BCMD_VKLID_e vkl1;
    BCMD_VKLID_e vkl2;
    BCMD_VKLID_e vklBoot;

} BSAGE_BootSettings;

typedef BERR_Code       (*Boot_Launch_func)(BHSM_Handle hHsm,BSAGE_BootSettings *pBootSettings);

typedef BERR_Code       (*Boot_GetInfo_func)(BSAGElib_ChipInfo *pChipInfo,BSAGElib_ImageInfo *pBootloaderInfo,BSAGElib_ImageInfo *pFrameworkInfo);

typedef BERR_Code       (*GetStatus_func)(BSAGElib_Status *pStatus);

typedef struct BSAGE_P_RpcRemote *(*Rpc_AddRemote_func)(uint32_t platformId,uint32_t moduleId,void *async_argument,uint64_t messageOffset);

typedef void            (*Rpc_RemoveRemote_func)(BSAGE_RpcRemoteHandle remote);

typedef BERR_Code       (*Rpc_SendCommand_func)(BSAGE_RpcRemoteHandle remote,BSAGElib_RpcCommand *command,uint32_t *pAsync_id);

typedef BERR_Code       (*Rpc_SendCallbackResponse_func)(BSAGE_RpcRemoteHandle remote,uint32_t sequenceId,BERR_Code retCode);

typedef BERR_Code       (*RegisterCallback_func)(SAGE_Event event,void *callback,void *context,BSAGE_RpcRemoteHandle remote);

typedef BERR_Code       (*UnRegisterCallback_func)(SAGE_Event event,BSAGE_RpcRemoteHandle remote);

typedef void            (*Management_Reset_func)(void);

typedef struct BSAGE_Interface{
    Boot_Launch_func              Boot_Launch;
    Boot_GetInfo_func             Boot_GetInfo;
    GetStatus_func                GetStatus;
    Rpc_AddRemote_func            Rpc_AddRemote;
    Rpc_RemoveRemote_func         Rpc_RemoveRemote;
    Rpc_SendCommand_func          Rpc_SendCommand;
    Rpc_SendCallbackResponse_func Rpc_SendCallbackResponse;
    RegisterCallback_func         RegisterCallback;
    UnRegisterCallback_func       UnRegisterCallback;
    Management_Reset_func         Management_Reset;
} BSAGE_Interface;

/* BSAGE_Open() with NULL setting will check if BSAGE is already openned,
   if yes, it will return valid handle, if no, NULL handle will be returned.
   if NULL SAGE handle used in APIs later, APIs will check if hSAGE_Global is available,
   if yes, hSAGE_Global will be used.*/
BERR_Code
BSAGE_Open(
    BSAGE_Handle *pSAGEHandle, /* [out] SAGElib handle */
    BSAGE_Settings *settings
    );

/* Uninitialize the SAGE library
 * None of other SAGE lib API can be used until BSAGElib_Open is called */
void
BSAGE_Close(
    BSAGE_Handle hSAGE);

BERR_Code
BSAGE_Boot_Launch(
    BHSM_Handle hHsm,
    BSAGE_BootSettings *pBootSettings);

BERR_Code
BSAGE_Boot_GetInfo(
    BSAGElib_ChipInfo *pChipInfo,
    BSAGElib_ImageInfo *pBootloaderInfo,
    BSAGElib_ImageInfo *pFrameworkInfo);

BERR_Code
BSAGE_GetStatus(
    BSAGElib_Status *pStatus);

BERR_Code
BSAGE_Rpc_Init(uint32_t HSIBufferOffset); /* the buffer is SAGE_HOST_BUF_SIZE*4 bytes long */

BSAGE_RpcRemoteHandle
BSAGE_Rpc_AddRemote(
    uint32_t platformId,
    uint32_t moduleId,
    void *async_argument,
    uint64_t messageOffset);

void
BSAGE_Rpc_RemoveRemote(
    BSAGE_RpcRemoteHandle remote);

BERR_Code
BSAGE_Rpc_SendCommand(
    BSAGE_RpcRemoteHandle remote,
    BSAGElib_RpcCommand *command,
    uint32_t *pAsync_id);

BERR_Code
BSAGE_Rpc_SendCallbackResponse(
    BSAGE_RpcRemoteHandle remote,
    uint32_t sequenceId,
    BERR_Code retCode);

BERR_Code
BSAGE_RegisterCallback(
    SAGE_Event event,
    BSAGE_Callback callback,
    void *context,
    BSAGE_RpcRemoteHandle remote);

BERR_Code
BSAGE_UnRegisterCallback(
    SAGE_Event event,
    BSAGE_RpcRemoteHandle remote);

#ifdef __cplusplus
}
#endif


#endif /* BSAGE_H_ */
