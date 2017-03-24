/***************************************************************************
*  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
*
* API Description:
*   API name: I2c
*    Specific APIs related to I2c Control.
*
***************************************************************************/
#ifndef NEXUS_I2C_PRIV_H__
#define NEXUS_I2C_PRIV_H__

#include "nexus_i2c.h"
#include "breg_i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
    This function returns a BREG_I2C_Handle from a NEXUS_I2cHandle.

Description:
    This function returns a BREG_I2C_Handle that will handle internal
    synchronization properly based upon two factors.  The first is the module
    that created the I2C Handle (not all are local I2C Masters), and the
    second is the module that intends to use it.  Supported models are as
    follows:  1) Local I2C Master, any nexus module may use it.  2) Module-
    specific I2C Master (e.g. Frontend), only that module may use it.  If
    platform requires a BREG_I2C_Handle for some reason, it may pass NULL
    for the module handle here.  That is only supported for local i2c masters.

See Also:
    NEXUS_I2c_Close(), NEXUS_I2c_GetDefaultSettings()

****************************************************************************/
BREG_I2C_Handle NEXUS_I2c_GetRegHandle(
    NEXUS_I2cHandle handle,
    NEXUS_ModuleHandle moduleHandle     /* The module requiring a BREG_I2C Handle, determines synchronization. model */
    );

/***************************************************************************
 * I2C Callback Description - This allows for modules outside of I2C to
 *                            create their own I2C controllers.  Any callback
 *                            not provided will return BERR_NOT_IMPLEMENTED.
 ***************************************************************************/
typedef struct NEXUS_I2cCallbacks
{
    BERR_Code (*write)(void *pContext, uint16_t chipAddr, uint32_t subAddr, const uint8_t *pData, size_t length);
    BERR_Code (*writeSw)(void *pContext, uint16_t chipAddr, uint32_t subAddr, const uint8_t *pData, size_t length);
    BERR_Code (*writeNoAck)(void *pContext, uint16_t chipAddr, uint32_t subAddr, const uint8_t *pData, size_t length);
    BERR_Code (*writeA16)(void *pContext, uint16_t chipAddr, uint32_t subAddr, const uint8_t *pData, size_t length);
    BERR_Code (*writeA24)(void *pContext, uint16_t chipAddr, uint32_t subAddr, const uint8_t *pData, size_t length);
    BERR_Code (*writeNoAddr)(void *pContext, uint16_t chipAddr, const uint8_t *pData, size_t length);
    BERR_Code (*writeNoAddrNoAck)(void *pContext, uint16_t chipAddr, const uint8_t *pData, size_t length);
    BERR_Code (*writeNvram)(void *pContext, uint16_t chipAddr, uint32_t subAddr, const uint8_t *pData, size_t length);
    BERR_Code (*read)(void *pContext, uint16_t chipAddr, uint32_t subAddr, uint8_t *pData, size_t length);
    BERR_Code (*readSw)(void *pContext, uint16_t chipAddr, uint32_t subAddr, uint8_t *pData, size_t length);
    BERR_Code (*readNoAck)(void *pContext, uint16_t chipAddr, uint32_t subAddr, uint8_t *pData, size_t length);
    BERR_Code (*readA16)(void *pContext, uint16_t chipAddr, uint32_t subAddr, uint8_t *pData, size_t length);
    BERR_Code (*readA24)(void *pContext, uint16_t chipAddr, uint32_t subAddr, uint8_t *pData, size_t length);
    BERR_Code (*readNoAddr)(void *pContext, uint16_t chipAddr, uint8_t *pData, size_t length);
    BERR_Code (*readSwNoAddr)(void *pContext, uint16_t chipAddr, uint8_t *pData, size_t length);
    BERR_Code (*readEDDC)(void *pContext, uint8_t chipAddr, uint8_t segment, uint32_t subAddr, uint8_t *pData, size_t length);
    BERR_Code (*readSwEDDC)(void *pContext, uint8_t chipAddr, uint8_t segment, uint32_t subAddr, uint8_t *pData, size_t length);
    BERR_Code (*writeEDDC)(void *pContext, uint8_t chipAddr, uint8_t segment, uint32_t subAddr, const uint8_t *pData, size_t length);    
} NEXUS_I2cCallbacks;

/***************************************************************************
Summary:
    This function initializes an I2C Callback structure to defaults
See Also:
    NEXUS_I2c_CreateHandle
****************************************************************************/
void NEXUS_I2c_InitCallbacks(
    NEXUS_I2cCallbacks *pCallbacks  /* [out] */
    );

/***************************************************************************
Summary:
    Create a NEXUS_I2cHandle for a module from a set of callbacks.

Description:
    This function allows another nexus module to create a new I2C handle
    that can be used with the standard NEXUS_I2c_Xxx routines by the application.
    This is required for several frontend devices.

See Also:
    NEXUS_I2c_DestroyHandle(), NEXUS_I2c_InitCallbacks()
****************************************************************************/
NEXUS_I2cHandle NEXUS_I2c_CreateHandle(
    NEXUS_ModuleHandle moduleHandle,            /* The module requiring a BREG_I2C Handle, determines synchronization. model */
    void *pContext,                             /* This is the device context that will be passed to each callback */
    const NEXUS_I2cCallbacks *pI2cCallbacks     /* A list of callbacks supported by this device */
    );

/***************************************************************************
Summary:
    Destroy a handle created with NEXUS_I2c_CreateHandle.

See Also:
    NEXUS_I2c_CreateHandle()
****************************************************************************/
void NEXUS_I2c_DestroyHandle(
    NEXUS_I2cHandle handle
    );

NEXUS_OBJECT_CLASS_DECLARE(NEXUS_I2c);

void NEXUS_I2c_GetDefaultSettings_priv(
    NEXUS_I2cSettings *pSettings    /* [out] */
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_I2C_PRIV_H__ */

