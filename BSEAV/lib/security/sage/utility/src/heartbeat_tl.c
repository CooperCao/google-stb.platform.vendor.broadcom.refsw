/******************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ******************************************************************************/

#include "utility_platform.h"
#include "utility_ids.h"
#include "heartbeat_tl.h"

static SRAI_ModuleHandle moduleHandle = NULL;

BDBG_MODULE(heartbeat_tl);

void HeartbeatTl_GetDefaultSettings(HeartbeatTlSettings *pHeartbeatModuleSettings)
{
    BDBG_ENTER(HeartbeatTl_GetDefaultSettings);

    if(pHeartbeatModuleSettings != NULL){
        BKNI_Memset((uint8_t *)pHeartbeatModuleSettings, 0x00, sizeof(HeartbeatTlSettings));
    }

    BDBG_ENTER(HeartbeatTl_GetDefaultSettings);
    return;
}

BERR_Code HeartbeatTl_Init(HeartbeatTlSettings *pHeartbeatModuleSettings)
{
    BERR_Code rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *container = NULL;

    BDBG_ENTER(HeartbeatTl_Init);

    if(pHeartbeatModuleSettings == NULL)
    {
        BDBG_ERR(("%s - Parameter settings are NULL", BSTD_FUNCTION));
        rc = BERR_INVALID_PARAMETER;
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", BSTD_FUNCTION));
        rc = BSAGE_ERR_CONTAINER_REQUIRED;
        goto ErrorExit;
    }


    /* Initialize SAGE Heartbeat module */
    rc = Utility_ModuleInit(Utility_ModuleId_eHeartbeat,
                            NULL,
                           container,
                           &moduleHandle);
    if(rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error initializing Heartbeat TL module (0x%08x)", BSTD_FUNCTION, container->basicOut[0]));
        goto ErrorExit;
    }

ErrorExit:
    BDBG_LEAVE(HeartbeatTl_Init);

    if(container != NULL){
        SRAI_Container_Free(container);
        container = NULL;
    }

    return rc;
}

BERR_Code HeartbeatTl_Uninit(void)
{
    BDBG_ENTER(HeartbeatTl_Uninit);


    /* After sending UnInit command to SAGE, close the module handle (i.e. send DRM_Adobe_UnInit to SAGE) */
    if(moduleHandle != NULL){
        Utility_ModuleUninit(moduleHandle);
        moduleHandle = NULL;
    }

    BDBG_LEAVE(HeartbeatTl_Uninit);
    return BERR_SUCCESS;
}


BERR_Code HeartbeatTl_ProcessCommand(uint32_t command_id)
{
    BERR_Code sage_rc;
    BERR_Code rc = BERR_SUCCESS;

    BSAGElib_InOutContainer *container = NULL;
    container = SRAI_Container_Allocate();
    if (container == NULL)
    {
        rc = BSAGE_ERR_CONTAINER_REQUIRED;
        goto handle_error;
    }

    if(command_id != Heartbeat_CommandId_eTakePulse)
    {
        BDBG_ERR(("%s - Invalid command ID (%u)", BSTD_FUNCTION, command_id));
        rc = BERR_INVALID_PARAMETER;
        goto handle_error;
    }


    sage_rc = SRAI_Module_ProcessCommand(moduleHandle, command_id, container);
    if ((sage_rc != BERR_SUCCESS) || (container->basicOut[0] != 0xFFFF))
    {
        BDBG_ERR(("%s - error taking SAGE's heartbeat (sage rc = %u, actual rc = %u)", BSTD_FUNCTION, sage_rc, container->basicOut[0]));
        rc = sage_rc;
        goto handle_error;
    }

    BDBG_LOG(("\t*** Heartbeat command successfully returned from SAGE....IT's ALIVE!\n"));
handle_error:

    if(container != NULL)
    {
        /* overwrite bin file */
        if (container->basicOut[1] != BERR_SUCCESS)
        {
            /* Display error code received from SAGE side */
            BDBG_LOG(("The following SAGE error occurred with the SAGE heartbeat module (0x%08x):", container->basicOut[1]));
            BDBG_LOG(("\t%s", BSAGElib_Tools_ReturnCodeToString(container->basicOut[1])));
        }

        SRAI_Container_Free(container);
        container = NULL;
    }
    return rc;
}
