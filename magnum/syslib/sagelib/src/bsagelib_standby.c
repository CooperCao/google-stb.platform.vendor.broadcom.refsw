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


#include "bstd.h"
#include "bkni.h"
#include "bsagelib_types.h"
#include "bsagelib.h"
#include "bsagelib_client.h"
#include "bsagelib_rai.h"
#include "priv/bsagelib_shared_types.h"
#include "bsagelib_priv.h"

#include "bhsm_keyladder.h"

BDBG_MODULE(BSAGElib);

#define SAGE_SUSPENDVAL_RUN              0x4F4E4D4C
#define SAGE_SUSPENDVAL_SLEEP            0x4F567856
#define SAGE_SUSPENDVAL_RESUME           0x4E345678
#define SAGE_SUSPENDVAL_RESUMING         0x77347856
#define SAGE_SUSPENDVAL_S3READY          0x534E8C53
#define SAGE_SUSPENDVAL_S2READY          0x524E8C52
#define SAGE_SUSPENDVAL_STANDBYFAIL_RUN  0x544E8C54
#define SAGE_SUSPENDVAL_STANDBYFAIL_DEAD 0x554E8C55

#define SAGE_SUSPENDADDR BSAGElib_GlobalSram_GetRegister(BSAGElib_GlobalSram_eSuspend)

/* How to wait for S2 or S3 command */
#define S2_WAIT_DELAY 5
#define S2_WAIT_COUNT 200
#define S3_WAIT_DELAY 10
#define S3_WAIT_COUNT 3000

/* Local functions */
#if SAGE_VERSION < SAGE_VERSION_CALC(3,0)
static BERR_Code BSAGElib_P_Standby_PMCommand(BSAGElib_ClientHandle hSAGElibClient, BSAGElib_InOutContainer *container);
#endif
static BERR_Code BSAGElib_P_Standby_S2(BSAGElib_ClientHandle hSAGElibClient, bool enter);
static BERR_Code BSAGElib_P_Standby_S3(BSAGElib_ClientHandle hSAGElibClient, bool enter);

/* Semi-private: reset variables on SAGE reset condition */
void
BSAGElib_P_Standby_Reset_isrsafe(
    BSAGElib_Handle hSAGElib)
{
    hSAGElib->currentMode = BSAGElib_eStandbyModeOn;
}

#if SAGE_VERSION < SAGE_VERSION_CALC(3,0)
/* This private function is in charge of sending the PM command to SAGE
 * - add a remote
 * - prepare command
 * - send command
 * - cleanup remote */
static BERR_Code
BSAGElib_P_Standby_PMCommand(
    BSAGElib_ClientHandle hSAGElibClient,
    BSAGElib_InOutContainer *container)
{
    BERR_Code rc = BERR_SUCCESS;
    BSAGElib_RpcCommand SAGElib_command;
    BSAGElib_Handle hSAGElib = hSAGElibClient->hSAGElib;
    BSAGElib_RpcRemoteHandle hRemote = hSAGElibClient->hSAGElib->hStandbyRemote;

    BKNI_Memset(&SAGElib_command, 0, sizeof(SAGElib_command));
    SAGElib_command.systemCommandId = BSAGElib_SystemCommandId_ePowerManagement;

    if (container) {
        SAGElib_command.containerOffset =
            BSAGElib_Tools_ContainerAddressToOffset(container,
                                                    &hSAGElib->i_memory_sync_isrsafe,
                                                    &hSAGElib->i_memory_map);
        if (!SAGElib_command.containerOffset) {
            BDBG_ERR(("%s: Cannot convert container address %p to offset",
                      __FUNCTION__, (void *)container));
            rc = BERR_INVALID_PARAMETER;
            goto end;
        }
    }

    rc = BSAGElib_Rpc_SendCommand(hRemote, &SAGElib_command, NULL);
    if (rc != BERR_SUCCESS) {
        BDBG_ERR(("%s: BSAGElib_Rpc_SendCommand() failure %u", __FUNCTION__, rc));
        rc = BERR_INVALID_PARAMETER;
        BSAGElib_Rpc_RemoveRemote(hRemote);
        goto end;
    }

end:
    return rc;
}

/* Handles S2 transitions (in and out) */
static BERR_Code
BSAGElib_P_Standby_S2(
    BSAGElib_ClientHandle hSAGElibClient,
    bool enter)
{
    BERR_Code rc = BERR_SUCCESS;
    BSAGElib_Handle hSAGElib = hSAGElibClient->hSAGElib;
    BSAGElib_RpcRemoteHandle hRemote = NULL;

    if (enter) {
        BDBG_MSG(("%s/enter: S2 'passive sleep'", __FUNCTION__));
        hRemote = BSAGElib_Rpc_AddRemote(hSAGElibClient, BSAGElib_eStandbyModePassive, 0, NULL);
        if (!hRemote) {
            BDBG_ERR(("%s: cannot add remote", __FUNCTION__));
            rc = BERR_INVALID_PARAMETER;
            goto end;
        }

        /* save the remote; will be cleared when leaving S2 */
        hSAGElib->hStandbyRemote = hRemote;
        hRemote = NULL;

        BREG_Write32(hSAGElib->core_handles.hReg, SAGE_SUSPENDADDR, SAGE_SUSPENDVAL_SLEEP);
        rc = BSAGElib_P_Standby_PMCommand(hSAGElibClient, NULL);
        if (rc != BERR_SUCCESS) {
            BDBG_ERR(("%s/enter: failed to send Standby PM Command", __FUNCTION__));
            BSAGElib_Rpc_RemoveRemote(hSAGElib->hStandbyRemote);
            hSAGElib->hStandbyRemote = NULL;
            goto end;
        }
    }
    else {
        BDBG_MSG(("%s/leave: S2 'passive sleep'", __FUNCTION__));
        BREG_Write32(hSAGElib->core_handles.hReg, SAGE_SUSPENDADDR, SAGE_SUSPENDVAL_RESUME);
    }

end:
    return rc;
}

/* Handles S2 transitions (in and out) */
static BERR_Code
BSAGElib_P_Standby_S3(
    BSAGElib_ClientHandle hSAGElibClient,
    bool enter)
{
    BERR_Code rc = BERR_SUCCESS;
    BSAGElib_Handle hSAGElib = hSAGElibClient->hSAGElib;
    BSAGElib_InOutContainer *container = NULL;
    BHSM_M2MKeySlotIO_t M2MKeySlotIO;
    bool allocatedKeyslot = false;
    uint8_t *pMemoryBlock = NULL; /* need to go through a temporary variable for proper cleanup */
    BSAGElib_RpcRemoteHandle hRemote = NULL;

    if (enter) {

        hRemote = BSAGElib_Rpc_AddRemote(hSAGElibClient, BSAGElib_eStandbyModeDeepSleep, 0, NULL);
        if (!hRemote) {
            BDBG_ERR(("%s: cannot add remote", __FUNCTION__));
            rc = BERR_INVALID_PARAMETER;
            goto end;
        }

        hSAGElib->hStandbyRemote = hRemote;
        hRemote = NULL;

        BDBG_MSG(("%s/enter: S3 'deep sleep'", __FUNCTION__));

        container = BSAGElib_Rai_Container_Allocate(hSAGElibClient);
        if(container == NULL) {
            rc = BERR_OUT_OF_DEVICE_MEMORY;
            BDBG_ERR(("%s/enter: BSAGElib_Rai_Container_Allocate() failure", __FUNCTION__));
            goto end;
        }

        /* client field is needed starting with Zeus 3.0 */
        M2MKeySlotIO.client = BHSM_ClientType_eSAGE;
        /* keySlotType is needed starting with Zeus 4.0 */
#if HSM_IS_ASKM_40NM
        M2MKeySlotIO.keySlotType = BCMD_XptSecKeySlot_eType3;
#else
        M2MKeySlotIO.keySlotType = BCMD_XptSecKeySlot_eType1;
#endif
        BSAGElib_iLockHsm();
        rc = BHSM_AllocateM2MKeySlot(hSAGElib->core_handles.hHsm, &M2MKeySlotIO);
        BSAGElib_iUnlockHsm();
        if (rc != BERR_SUCCESS) {
            BDBG_ERR(("%s/enter: Cannot Allocated M2M keyslot", __FUNCTION__));
            goto end;
        }
        allocatedKeyslot = true;
        BDBG_MSG(("%s/enter: Allocated M2M keyslot %d", __FUNCTION__, M2MKeySlotIO.keySlotNum));

        BREG_Write32(hSAGElib->core_handles.hReg, SAGE_SUSPENDADDR, SAGE_SUSPENDVAL_SLEEP);

        /* Send the PM command to SAGE, associated the keyslot num and a memory block from CRR */
        container->basicIn[0] = M2MKeySlotIO.keySlotNum;
        container->blocks[0].len = 1024;
        pMemoryBlock = BSAGElib_Rai_Memory_Allocate(hSAGElibClient,
                                                    container->blocks[0].len,
                                                    BSAGElib_MemoryType_Restricted);
        if (!pMemoryBlock) {
            BDBG_ERR(("%s/enter: cannot allocate memory block for S3", __FUNCTION__));
            rc = BERR_OUT_OF_DEVICE_MEMORY;
            goto end;
        }
        container->blocks[0].data.ptr = pMemoryBlock;

        rc = BSAGElib_P_Standby_PMCommand(hSAGElibClient, container);
        if (rc != BERR_SUCCESS) {
            BDBG_ERR(("%s/enter: failed to send Standby PM Command", __FUNCTION__));
            goto end;
        }

        BDBG_MSG(("%s/enter: Command sent, waiting for SAGE to be ready for S3", __FUNCTION__));

        while (BREG_Read32(hSAGElib->core_handles.hReg, SAGE_SUSPENDADDR) != SAGE_SUSPENDVAL_S3READY) {
            BKNI_Sleep(1);
        }

        /* Nexus/SecurityModule is going down right after. Nexus/SageModule cleans security related resources */
        BSAGElib_P_SageVklsUninit(hSAGElib);

        BDBG_MSG(("%s/enter: SAGE is now ready for S3", __FUNCTION__));
    }
    else {
        BDBG_MSG(("%s/leave: S3 'deep sleep'", __FUNCTION__));

        BSAGElib_P_SageVklsInit(hSAGElib);

        BREG_Write32(hSAGElib->core_handles.hReg, SAGE_SUSPENDADDR, SAGE_SUSPENDVAL_RUN);
        hSAGElib->resetPending = 1;
        if (hSAGElib->enablePinmux) {
            BSAGElib_P_Init_Serial(hSAGElib);
        }
    }

end:
    if (allocatedKeyslot) {
        /* Can only occur when entering S3 ; here for error handling */
        BSAGElib_iLockHsm();
        rc = BHSM_FreeM2MKeySlot(hSAGElib->core_handles.hHsm, &M2MKeySlotIO);
        BSAGElib_iUnlockHsm();
        if (rc != BERR_SUCCESS) {
            rc = BERR_SUCCESS; /* from SAGE standpoint the Standby S3 task is completed */
            BDBG_WRN(("%s/enter: Cannot Free M2M keyslot, proceed", __FUNCTION__));
        }
    }
    if (pMemoryBlock) {
        BSAGElib_Rai_Memory_Free(hSAGElibClient, pMemoryBlock);
    }
    if (container) {
        container->blocks[0].data.ptr = NULL;
        BSAGElib_Rai_Container_Free(hSAGElibClient, container);
    }
    return rc;
}
#else
/* Handles S2 transitions (in and out) */
static BERR_Code
BSAGElib_P_Standby_S2(
    BSAGElib_ClientHandle hSAGElibClient,
    bool enter)
{
    BERR_Code rc = BERR_SUCCESS;
    BSAGElib_Handle hSAGElib = hSAGElibClient->hSAGElib;
    BSAGElib_InOutContainer *container = NULL;
    BSAGElib_RpcCommand command;
    BSAGElib_RpcRemoteHandle hRemote = NULL;
    uint32_t suspendRegValue;
    uint32_t count;

    if (enter) {
        BDBG_MSG(("%s/enter: S2 'passive sleep'", __FUNCTION__));

        hRemote = BSAGElib_Rpc_AddRemote(hSAGElibClient,
                                         BSAGE_PLATFORM_ID_SYSTEM_CRIT,
                                         SystemCrit_ModuleId_eFramework, NULL);
        if (!hRemote) {
            BDBG_ERR(("%s: cannot add remote", __FUNCTION__));
            rc = BERR_INVALID_PARAMETER;
            goto end;
        }

        container = BSAGElib_Rai_Container_Allocate(hSAGElibClient);
        if(container == NULL) {
            rc = BERR_OUT_OF_DEVICE_MEMORY;
            BDBG_ERR(("%s/enter: BSAGElib_Rai_Container_Allocate() failure", __FUNCTION__));
            goto end;
        }

        BDBG_MSG(("%s/enter: S2 'passive sleep' before write %x", __FUNCTION__, BREG_Read32(hSAGElib->core_handles.hReg, SAGE_SUSPENDADDR)));
        BREG_Write32(hSAGElib->core_handles.hReg, SAGE_SUSPENDADDR, SAGE_SUSPENDVAL_SLEEP);
        BDBG_MSG(("%s/enter: S2 'passive sleep' after write %x", __FUNCTION__, BREG_Read32(hSAGElib->core_handles.hReg, SAGE_SUSPENDADDR)));

        container->basicIn[1] = FrameworkModule_CommandId_eStandbyPassive;

        command.containerOffset = BSAGElib_Tools_ContainerAddressToOffset(container,
                                                                      &hSAGElib->i_memory_sync_isrsafe,
                                                                      &hSAGElib->i_memory_map);
        command.containerVAddr = container;
        command.moduleCommandId = 0;
        command.systemCommandId = BSAGElib_SystemCommandId_eModuleInit;

        rc = BSAGElib_Rpc_SendCommand(hRemote, &command, NULL);
        if (rc != BERR_SUCCESS) {
            BDBG_ERR(("%s: BSAGElib_Rpc_SendCommand (%u)", __FUNCTION__, rc));
            goto end;
        }
        /* eat-up remote and remove it when leaving S2 inside the close client */
        hRemote = NULL;

        /* Will be releasing BCHP_PWR resources... wait for OK from sage before continuing */
        for (count = 0; count < S2_WAIT_COUNT; count++) {
            suspendRegValue = BREG_Read32(hSAGElib->core_handles.hReg, SAGE_SUSPENDADDR);
            if (suspendRegValue == SAGE_SUSPENDVAL_S2READY) {
                /* Everything is ok */
                rc = BERR_SUCCESS;
                goto end;
            }

            if (suspendRegValue == SAGE_SUSPENDVAL_STANDBYFAIL_RUN) {
                BDBG_ERR(("SAGE ERROR entering S2"));
                BREG_Write32(hSAGElib->core_handles.hReg, SAGE_SUSPENDADDR, SAGE_SUSPENDVAL_RUN);
                rc = BERR_UNKNOWN;
                goto end;
            }

            if (suspendRegValue == SAGE_SUSPENDVAL_STANDBYFAIL_DEAD) {
                /* Note that on a production system, the chip should have reset by now */
                BDBG_ERR(("SAGE ERROR entering S2. SAGE NO LONGER RUNNING"));
                rc = BERR_UNKNOWN;
                goto end;
            }

            BKNI_Sleep(S2_WAIT_DELAY);
        }

        /* If here, there was a timeout */
        BDBG_ERR(("Timeout waiting for SAGE to enter S2 state"));
        rc = BERR_TIMEOUT;
    }
    else {
        suspendRegValue = BREG_Read32(hSAGElib->core_handles.hReg, SAGE_SUSPENDADDR);
        BDBG_MSG(("%s/leave: S2 'passive sleep' %x", __FUNCTION__, suspendRegValue));

        if(suspendRegValue == SAGE_SUSPENDVAL_SLEEP){
            BDBG_MSG(("%s/leave: Command sent, waiting for SAGE to be ready for S2", __FUNCTION__));
            while (BREG_Read32(hSAGElib->core_handles.hReg, SAGE_SUSPENDADDR) != SAGE_SUSPENDVAL_S2READY) {
                BKNI_Sleep(1);
            }
        }
        BREG_Write32(hSAGElib->core_handles.hReg, SAGE_SUSPENDADDR, SAGE_SUSPENDVAL_RESUME);
    }

end:

    if(container){
        BSAGElib_Rai_Container_Free(hSAGElibClient,container);
    }

    if(hRemote){
        BSAGElib_Rpc_RemoveRemote(hRemote);
    }
    return rc;
}

/* Handles S3 transitions (in and out) */
static BERR_Code
BSAGElib_P_Standby_S3(
    BSAGElib_ClientHandle hSAGElibClient,
    bool enter)
{
    BERR_Code rc = BERR_SUCCESS;
    BSAGElib_Handle hSAGElib = hSAGElibClient->hSAGElib;
    BSAGElib_InOutContainer *container = NULL;
    BSAGElib_RpcCommand command;
    BSAGElib_RpcRemoteHandle hRemote = NULL;
    uint32_t count;
    uint32_t suspendRegValue;

    if (enter) {
        BDBG_MSG(("%s/enter: S3 'deep sleep'", __FUNCTION__));
        hRemote = BSAGElib_Rpc_AddRemote(hSAGElibClient,
                                         BSAGE_PLATFORM_ID_SYSTEM_CRIT,
                                         SystemCrit_ModuleId_eFramework, NULL);
        if (!hRemote) {
            BDBG_ERR(("%s: cannot add remote", __FUNCTION__));
            rc = BERR_INVALID_PARAMETER;
            goto end;
        }

        container = BSAGElib_Rai_Container_Allocate(hSAGElibClient);
        if(container == NULL) {
            rc = BERR_OUT_OF_DEVICE_MEMORY;
            BDBG_ERR(("%s/enter: BSAGElib_Rai_Container_Allocate() failure", __FUNCTION__));
            goto end;
        }
        BKNI_Memset(container, 0, sizeof(*container));

        BREG_Write32(hSAGElib->core_handles.hReg, SAGE_SUSPENDADDR, SAGE_SUSPENDVAL_SLEEP);

        /* Send the PM command to SAGE, associated the keyslot num and a memory block from CRR */
        container->basicIn[1] = FrameworkModule_CommandId_eStandbyDeepSleep;

        command.containerOffset = BSAGElib_Tools_ContainerAddressToOffset(container,
                                                                      &hSAGElib->i_memory_sync_isrsafe,
                                                                      &hSAGElib->i_memory_map);
        command.containerVAddr = container;
        command.moduleCommandId = 0;
        command.systemCommandId = BSAGElib_SystemCommandId_eModuleInit;

        rc = BSAGElib_Rpc_SendCommand(hRemote, &command, NULL);
        if (rc != BERR_SUCCESS) {
            BDBG_ERR(("%s: BSAGElib_Rpc_SendCommand (%u)", __FUNCTION__, rc));
            goto end;
        }

        BDBG_MSG(("%s/enter: Command sent, waiting for SAGE to be ready for S3", __FUNCTION__));

        for (count = 0; count < S3_WAIT_COUNT; count++) {
            suspendRegValue = BREG_Read32(hSAGElib->core_handles.hReg, SAGE_SUSPENDADDR);
            if (suspendRegValue == SAGE_SUSPENDVAL_S3READY) {
                /* Everything is ok */
                break;
            }

            if (suspendRegValue == SAGE_SUSPENDVAL_STANDBYFAIL_RUN) {
                BDBG_ERR(("SAGE ERROR entering S3"));
                BREG_Write32(hSAGElib->core_handles.hReg, SAGE_SUSPENDADDR, SAGE_SUSPENDVAL_RUN);
                rc = BERR_UNKNOWN;
                goto end;
            }

            if (suspendRegValue == SAGE_SUSPENDVAL_STANDBYFAIL_DEAD) {
                /* Note that on a production system, the chip should have reset by now */
                BDBG_ERR(("SAGE ERROR entering S3. SAGE NO LONGER RUNNING"));
                rc = BERR_UNKNOWN;
                goto end;
            }

            BKNI_Sleep(S3_WAIT_DELAY);
        }
        if (count >= S3_WAIT_COUNT) {
            BDBG_ERR(("Timeout waiting for SAGE to enter S3"));
            rc = BERR_TIMEOUT;
            goto end;
        }

        /* Nexus/SecurityModule is going down right after. Nexus/SageModule cleans security related resources */
        BSAGElib_P_SageVklsUninit(hSAGElib);

        BDBG_MSG(("%s/enter: SAGE is now ready for S3", __FUNCTION__));
    }
    else {
        BDBG_MSG(("%s/leave: S3 'deep sleep'", __FUNCTION__));

        BSAGElib_P_SageVklsInit(hSAGElib);

        BREG_Write32(hSAGElib->core_handles.hReg, SAGE_SUSPENDADDR, SAGE_SUSPENDVAL_RUN);
        hSAGElib->resetPending = 1;
        if (hSAGElib->enablePinmux) {
            BSAGElib_P_Init_Serial(hSAGElib);
        }
    }

end:
    if (container) {
        container->blocks[0].data.ptr = NULL;
        BSAGElib_Rai_Container_Free(hSAGElibClient, container);
    }
    if(hRemote){
        BSAGElib_Rpc_RemoveRemote(hRemote);
    }

    return rc;
}
#endif

/* Public API to trigger standby transitions
 *   state machine approach to allow and react to mode transitions
 *
 * Note:
 * The API is using SAGElib Client handle because it needs to get access to
 * sync interfaces to handle synchronisation for both security and Sage modules.
 * While security module sync is obvious and could be derived from default in SAGElib main handle,
 * the sage sync interface cannot as we don't know the calling context. */
BERR_Code
BSAGElib_Standby(
    BSAGElib_ClientHandle hSAGElibClient,
    BSAGElib_eStandbyMode mode)
{
    BERR_Code rc = BERR_INVALID_PARAMETER;
    BSAGElib_Handle hSAGElib = hSAGElibClient->hSAGElib;

    BDBG_ENTER(BSAGElib_Standby);

    BDBG_OBJECT_ASSERT(hSAGElib, BSAGElib_P_Instance);
    BDBG_OBJECT_ASSERT(hSAGElibClient, BSAGElib_P_Client);

    BDBG_MSG(("%s currentMode %d Mode %d",__FUNCTION__,hSAGElib->currentMode, mode));

    if((hSAGElib->currentMode != BSAGElib_eStandbyModeOn)&&(mode == BSAGElib_eStandbyModeOn))
    {
        /*power up*/
        /*acquire SAGE power resources */
#ifdef BCHP_PWR_RESOURCE_HDMI_TX_CLK
        BCHP_PWR_AcquireResource(hSAGElibClient->hSAGElib->core_handles.hChp, BCHP_PWR_RESOURCE_HDMI_TX_CLK);
#endif
#ifdef BCHP_PWR_RESOURCE_BVN
        BCHP_PWR_AcquireResource(hSAGElibClient->hSAGElib->core_handles.hChp, BCHP_PWR_RESOURCE_BVN);
#endif
#ifdef BCHP_PWR_RESOURCE_VDC_VEC
        BCHP_PWR_AcquireResource(hSAGElibClient->hSAGElib->core_handles.hChp, BCHP_PWR_RESOURCE_VDC_VEC);
#endif
    }

    switch (hSAGElib->currentMode) {

    case BSAGElib_eStandbyModeOn:
        /* currently in S0, not in standby
         * SAGE is up and running
         */
        switch (mode) {
        case BSAGElib_eStandbyModePassive:
            /* Enter S2 : SAGE will be gated and wait for resume */
            if (BSAGElib_P_Standby_S2(hSAGElibClient, true) != BERR_SUCCESS) {
                goto end;
            }
            break;
        case BSAGElib_eStandbyModeDeepSleep:
            /* Enter S3 : SAGE will be restarted */
            if (BSAGElib_P_Standby_S3(hSAGElibClient, true) != BERR_SUCCESS) {
                goto end;
            }
            break;
        case BSAGElib_eStandbyModeOn:
        default:
            goto end;
        }
        break;

    case BSAGElib_eStandbyModePassive:
        /* currently in S2 (passive sleep)
         * SAGE is waiting to resume
         */
        switch (mode) {
        case BSAGElib_eStandbyModeOn:
            /* Resume from S2 */
            if (BSAGElib_P_Standby_S2(hSAGElibClient, false) != BERR_SUCCESS) {
                goto end;
            }
            break;
        case BSAGElib_eStandbyModePassive:
        case BSAGElib_eStandbyModeDeepSleep:
        default:
            goto end;
        }
        break;

    case BSAGElib_eStandbyModeDeepSleep:
        /* currently in S3 (deep sleep)
         * SAGE needs to be restarted.
         */
        switch (mode) {
        case BSAGElib_eStandbyModeOn:
            /* Resume from S3 */
            if (BSAGElib_P_Standby_S3(hSAGElibClient, false) != BERR_SUCCESS) {
                goto end;
            }
            break;
        case BSAGElib_eStandbyModePassive:
        case BSAGElib_eStandbyModeDeepSleep:
        default:
            goto end;
        }
        break;

    default:
        goto end;
    }

    if((hSAGElib->currentMode == BSAGElib_eStandbyModeOn)&&(mode != BSAGElib_eStandbyModeOn))
    {
        /*power down*/
        /*release SAGE power resources */
#ifdef BCHP_PWR_RESOURCE_HDMI_TX_CLK
        BCHP_PWR_ReleaseResource(hSAGElibClient->hSAGElib->core_handles.hChp, BCHP_PWR_RESOURCE_HDMI_TX_CLK);
#endif
#ifdef BCHP_PWR_RESOURCE_BVN
        BCHP_PWR_ReleaseResource(hSAGElibClient->hSAGElib->core_handles.hChp, BCHP_PWR_RESOURCE_BVN);
#endif
#ifdef BCHP_PWR_RESOURCE_VDC_VEC
        BCHP_PWR_ReleaseResource(hSAGElibClient->hSAGElib->core_handles.hChp, BCHP_PWR_RESOURCE_VDC_VEC);
#endif
    }

    /* Only ends here on success scenarios */
    rc = BERR_SUCCESS;

end:
    if (rc != BERR_SUCCESS) {
        BDBG_ERR(("%s: cannot move from %u to %u", __FUNCTION__, hSAGElib->currentMode, mode));
    }
    else {
        BDBG_MSG(("%s: switched from %u to %u", __FUNCTION__, hSAGElib->currentMode, mode));
        hSAGElib->currentMode = mode;
    }

    BDBG_LEAVE(BSAGElib_Standby);
    return rc;
}
