/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 ***************************************************************/
#ifndef BTEE_INSTANCE_H__
#define BTEE_INSTANCE_H__

/*================== Module Overview =====================================

This defines the interface to create an instance of the BTEE module.  This
allows upper-layer code to link a trusted execution environment library or
3rd-party module into magnum indirectly and avoid any external dependencies.

========================================================================*/

#include "btee.h"

/***************************************************************************
Summary:
    Instance Creation Settings
***************************************************************************/
typedef struct BTEE_InstanceCreateSettings
{
    void *pInstanceData;     /* Private data passed to client create routine */
    BERR_Code (*ClientCreate)(void *pInstanceData, const char *pName ,const BTEE_ClientCreateSettings *pSettings, void **pClientPrivate);
    void (*ClientDestroy)(void *pClientPrivate);
    BERR_Code (*ClientReceiveMessage)(void *pClientPrivate, void **pConnection, void *pMessage, size_t maxLength, size_t *pLength, int timeoutMsec);
    BERR_Code (*ClientAlloc)(void *pClientPrivate, size_t size, void **pMemory);
    void (*ClientFree)(void *pClientPrivate, void *pMemory);
    BERR_Code (*ClientSecureAlloc)(void *pClientPrivate, size_t size, uint32_t *pAddress);
    void (*ClientSecureFree)(void *pClientPrivate, uint32_t address);
    BERR_Code (*ClientAddrToOffset)(void *pClientPrivate, void *pMemory, uint32_t *pOffset);
    BERR_Code (*ClientOffsetToAddr)(void *pClientPrivate, uint32_t offset, void **pMemory);
    BERR_Code (*ClientContextSwitch)(void *pClientPrivate);
    BERR_Code (*FileOpen)(void *pClientPrivate, const char *pPath, int flags, void **pFilePrivate);
    BERR_Code (*FileRead)(void *pFilePrivate, uint32_t address, size_t bytesToRead, size_t *pBytesRead);
    BERR_Code (*FileWrite)(void *pFilePrivate, uint32_t address, size_t bytesToWrite, size_t *pBytesWritten);
    void (*FileClose)(void *pFilePrivate);
    BERR_Code (*AppOpen)(void *pClientPrivate, const char *pName, const char *pPath, void **pAppPrivate);
    void (*AppClose)(void *pAppPrivate);
    BERR_Code (*ConnectionOpen)(void *pAppPrivate, const char *pServiceName, const BTEE_ConnectionSettings *pSettings, void **pConnectionPrivate);
    BERR_Code (*ConnectionSendMessage)(void *pConnectionPrivate, const void *pMessage, size_t messageLength);
    void (*ConnectionClose)(void *pConnectionPrivate);
    BERR_Code (*GetStatus)(BTEE_InstanceStatus *pStatus);
} BTEE_InstanceCreateSettings;

/***************************************************************************
Summary:
    Get default TEE Instance settings
***************************************************************************/
void BTEE_Instance_GetDefaultCreateSettings(
    BTEE_InstanceCreateSettings *pSettings /* [out] default settings */
    );

/***************************************************************************
Summary:
    Create a TEE instance
***************************************************************************/
BERR_Code BTEE_Instance_Create(
    const BTEE_InstanceCreateSettings *pSettings,
    BTEE_InstanceHandle *pInstance /* [out] handle to instance */
    );

/***************************************************************************
Summary:
    Destroy a TEE instance
***************************************************************************/
void BTEE_Instance_Destroy(
    BTEE_InstanceHandle hInstance
    );

#endif /* BTEE_INSTANCE_H__ */
