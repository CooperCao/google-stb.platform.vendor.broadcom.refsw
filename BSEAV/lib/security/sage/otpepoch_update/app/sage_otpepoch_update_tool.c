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
 *
 *******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "bsagelib_types.h"
#include "bkni.h"
#include "sage_srai.h"

#ifdef NXCLIENT_SUPPORT
#include "nxclient.h"
#endif

#include "nexus_security_types.h"
#include "nexus_security.h"
#include "nexus_base_os.h"

#include "sage_app_utils.h"

BDBG_MODULE(sage_otpepoch_update_tool);

static struct
{
    SRAI_PlatformHandle hPlatform;
    SRAI_ModuleHandle hModule;
    uint8_t *pCertificateBuff;
    uint32_t certificateSize;
} _ctx;

typedef enum OTPEPOCH_Update_Error_e {
    OTPEPOCH_Update_Error_eNone = 0,
    OTPEPOCH_Update_Error_eUnknown,
    OTPEPOCH_Update_Error_eAllocateMemory,
    OTPEPOCH_Update_Error_eInvalidPayload,
    OTPEPOCH_Update_Error_eInvalidPayloadSize,
    OTPEPOCH_Update_Error_eGetPayloadSize,
    OTPEPOCH_Update_Error_eGetPayload,
    OTPEPOCH_Update_Error_eGetAllLoadedTaInfo,
    OTPEPOCH_Update_Error_eSdlId0Mismatch,
    OTPEPOCH_Update_Error_eSdlId1Mismatch,
    OTPEPOCH_Update_Error_eUpdateHvdFwEpoch,
    OTPEPOCH_Update_Error_eUpdateRaggaFwEpoch,
    OTPEPOCH_Update_Error_eUpdateViceFwEpoch,
    OTPEPOCH_Update_Error_eUpdateRaveFwEpoch,
    OTPEPOCH_Update_Error_eCheckMarketId,
    OTPEPOCH_Update_Error_eCheckSdlId,
    OTPEPOCH_Update_Error_eUpdateBasedOnSdlId,
    OTPEPOCH_Update_Error_eUpdateBasedOnTaList,
    OTPEPOCH_Update_Error_eInvalidTaList,
    OTPEPOCH_Update_Error_eInvalidTaNumber,
    OTPEPOCH_Update_Error_eTaNotLoaded
} OTPEPOCH_Update_Error_e;

typedef struct ErrorCode_NameValue
{
    OTPEPOCH_Update_Error_e value;
    const char *name;
} ErrorCode_NameValue;

static const char *notFound = "NotFound";

static const ErrorCode_NameValue g_errorCode[] = {
    {OTPEPOCH_Update_Error_eNone, "None"},
    {OTPEPOCH_Update_Error_eUnknown, "Unknown"},
    {OTPEPOCH_Update_Error_eAllocateMemory, "AllocateMemory"},
    {OTPEPOCH_Update_Error_eInvalidPayload, "InvalidPayload"},
    {OTPEPOCH_Update_Error_eInvalidPayloadSize, "InvalidPayloadSize"},
    {OTPEPOCH_Update_Error_eGetPayloadSize, "GetPayLoadSize"},
    {OTPEPOCH_Update_Error_eGetPayload, "GetPayload"},
    {OTPEPOCH_Update_Error_eGetAllLoadedTaInfo, "GetAllLoadedTaInfo"},
    {OTPEPOCH_Update_Error_eSdlId0Mismatch, "SdlId0Mismatch"},
    {OTPEPOCH_Update_Error_eSdlId1Mismatch, "SdlId1Mismatch"},
    {OTPEPOCH_Update_Error_eUpdateHvdFwEpoch, "UpdateHvdFwEpoch"},
    {OTPEPOCH_Update_Error_eUpdateRaggaFwEpoch, "UpdateRaggaFwEpoch"},
    {OTPEPOCH_Update_Error_eUpdateViceFwEpoch, "UpdateViceFwEpoch"},
    {OTPEPOCH_Update_Error_eUpdateRaveFwEpoch, "UpdateRaveFwEpoch"},
    {OTPEPOCH_Update_Error_eCheckMarketId, "CheckMarketId"},
    {OTPEPOCH_Update_Error_eCheckSdlId, "CheckSdlId"},
    {OTPEPOCH_Update_Error_eUpdateBasedOnSdlId, "UpdateBasedOnSdlId"},
    {OTPEPOCH_Update_Error_eUpdateBasedOnTaList, "UpdateBasedOnTaList"},
    {OTPEPOCH_Update_Error_eInvalidTaList, "InvalidTaList"},
    {OTPEPOCH_Update_Error_eInvalidTaNumber, "InvalidTaNumber"},
    {OTPEPOCH_Update_Error_eTaNotLoaded, "TaNotLoaded"},
    {0, NULL}
};


/* local function */
static const char *_P_GetNameFromValue(const ErrorCode_NameValue *table, OTPEPOCH_Update_Error_e value);
static int _P_Initialize(void);
static void _P_Unintialize(void);
static int _P_ModuleInit(void);
static void _P_ModuleUninit(void);
static int _P_Read_File(const char *filePath, uint8_t** binBuff, uint32_t* binSize);
static void _usage(const char *binName);
static int _P_ProcessCertificate(void);


static const char *_P_GetNameFromValue(const ErrorCode_NameValue *table, OTPEPOCH_Update_Error_e value)
{
    unsigned i;
    for (i=0; table[i].name; i++) {
        if (table[i].value == value) {
            return table[i].name;
        }
    }
    fprintf(stderr, "unable to find value %d\n", value);
    return notFound;
}

static int _P_Initialize(void)
{
    BERR_Code sage_rc;
    BSAGElib_State state;
    int rc = 0;

#ifdef NXCLIENT_SUPPORT
    SRAI_Settings appSettings;

    /* Get Current Settings */
    SRAI_GetSettings(&appSettings);

    /* Customize appSettings, for example if designed to use NxClient API */
    appSettings.generalHeapIndex     = NXCLIENT_FULL_HEAP;
    appSettings.videoSecureHeapIndex = NXCLIENT_VIDEO_SECURE_HEAP;
    appSettings.exportHeapIndex      = NXCLIENT_EXPORT_HEAP;
    appSettings.arrHeapIndex         = NXCLIENT_ARR_HEAP;

    /* Save/Apply new settings */
    SRAI_SetSettings(&appSettings);
#endif

    /* Open the system platform */
    sage_rc = SRAI_Platform_Open(BSAGE_PLATFORM_ID_SYSTEM,
                                 &state,
                                 &_ctx.hPlatform);
    if (sage_rc != BERR_SUCCESS) {
        rc = 1;
        BDBG_ERR(("%s: Cannot Open Platform", BSTD_FUNCTION));
        goto end;
    }

    /* Check init state */
    if (state != BSAGElib_State_eInit) {
        BDBG_MSG(("Initializing system platform"));
        /* Not yet initialized: send init command, should never happen*/
        sage_rc = SRAI_Platform_Init(_ctx.hPlatform, NULL);
        if (sage_rc != BERR_SUCCESS) {
            rc = 2;
            BDBG_ERR(("%s: Failed to initialize SYSTEM platform", BSTD_FUNCTION));
            goto end;
        }
    }

    /* Initialize module */
    BDBG_MSG(("Initializing OTP EPOCH update Module"));
    if (_P_ModuleInit()) {
        rc = 3;
        goto end;
    }

end:
    if (rc) {
        /* error: cleanup platform */
        _P_Unintialize();
    }
    return rc;
}

static void _P_Unintialize(void)
{
    if (_ctx.pCertificateBuff) {
        SRAI_Memory_Free(_ctx.pCertificateBuff);
        _ctx.pCertificateBuff = NULL;
    }

    _P_ModuleUninit();

    if (_ctx.hPlatform) {
        SRAI_Platform_Close(_ctx.hPlatform);
        _ctx.hPlatform = NULL;
    }
}

static int _P_ModuleInit(void)
{
    int rc      = -1;
    BERR_Code sage_rc = BERR_SUCCESS;

    if (_ctx.hPlatform == NULL) {
        BDBG_ERR(("%s: Invalid platform handle", BSTD_FUNCTION));
        rc = 1;
        goto end;
    }

    sage_rc = SRAI_Module_Init(_ctx.hPlatform,
                               System_ModuleId_eOTPEpochUpdate,
                               NULL,
                               &_ctx.hModule);
    if (sage_rc != BERR_SUCCESS) {
        BDBG_ERR(("%s: Cannot Init Module", BSTD_FUNCTION));
        rc = 2;
        goto end;
    }

    rc = 0;

end:
    return rc;
}

static void _P_ModuleUninit(void)
{
    if (_ctx.hModule) {
        SRAI_Module_Uninit(_ctx.hModule);
        _ctx.hModule = NULL;
    }
}

static int _P_Read_File(const char *filePath, uint8_t** binBuff, uint32_t* binSize)
{
    int rc = -1;
    int fd = -1;
    uint32_t size = 0;
    uint8_t *buff = NULL;
    ssize_t readSize;

    fd = open(filePath, O_RDONLY);
    if (fd == -1) {
        rc = -2;
        goto end;
    }

    /* poll size, allocate destination buffer and return pos to start */
    {
        off_t pos = lseek(fd, 0, SEEK_END);
        if (pos == -1) {
            rc = -3;
            goto end;
        }
        size = (uint32_t)pos;
        buff = SRAI_Memory_Allocate(size, SRAI_MemoryType_Shared);
        if (buff == NULL) {
            rc = -4;
            goto end;
        }

        pos = lseek(fd, 0, SEEK_SET);
        if (pos != 0) {
            rc = -5;
            goto end;
        }
    }

    /* read file in memory */
    {
        readSize = read(fd, (void *)buff, (size_t)size);
        if (readSize != (ssize_t)size) {
            rc = -6;
            goto end;
        }
    }
    rc = 0;

end:
    if (fd != -1) {
        close(fd);
        fd = -1;
    }

    if (rc) {
        if (buff) {
            SRAI_Memory_Free(buff);
            buff = NULL;
        }
        fprintf(stderr, "%s ERROR #%d\n", BSTD_FUNCTION, rc);
    }

    *binBuff = buff;
    *binSize = size;

    return rc;
}

static void _usage(const char *binName)
{
    printf("usage:\n");
    printf("%s <sage_fc_otp_epoch_update.bin file> \n", binName);
}

static int _parse_cmdline(int argc, char *argv[])
{
    int rc = -1;
    int curr_arg = 1;

    if (argc < 2) {
        _usage("sage_otpepoch_update_tool");
        rc = -1;
        goto end;
    }

    rc = _P_Read_File(argv[curr_arg],&_ctx.pCertificateBuff, &_ctx.certificateSize);
    if (rc) { goto end; }

    rc = 0;

end:
    return rc;
}

static int _P_ProcessCertificate(void)
{
    int rc = 0;
    BERR_Code sage_rc;
    BSAGElib_InOutContainer *container = NULL;

    if ((!_ctx.pCertificateBuff) || (!_ctx.certificateSize)) {
        BDBG_ERR(("%s: Invalid certificate binary", BSTD_FUNCTION));
        rc = -1;
        goto end;
    }

    container = SRAI_Container_Allocate();
    if (!container) {
        BDBG_ERR(("%s: Cannot allocate container for process OTP EPOCH update command", BSTD_FUNCTION));
        rc = -2;
        goto end;
    }

    /* Prepare to send the certifacate binary with the command */
    container->blocks[0].data.ptr = _ctx.pCertificateBuff;
    container->blocks[0].len = _ctx.certificateSize;

    sage_rc = SRAI_Module_ProcessCommand(_ctx.hModule, OTPEpochUpdateModule_CommandId_eUpdateEpoch, container);
    if (sage_rc != BERR_SUCCESS) {
        BDBG_ERR(("%s: Error during processing command (0x%08x)", BSTD_FUNCTION, sage_rc));
        rc = sage_rc;
        goto end;
    }

    rc = container->basicOut[0];
    if (rc) {
        BDBG_ERR(("Update Epoch operation failed - \"%s\"", _P_GetNameFromValue(g_errorCode, rc)));

        if (rc == OTPEPOCH_Update_Error_eUpdateBasedOnTaList) {
            BDBG_LOG(("Update Epoch based on TA list failed. Please check the TA check list. '1' means the TA is updated; '0' means the TA is not updated."));
            BDBG_LOG(("TA check list (The bit position reflets the index in the TA check list):"));
            BDBG_LOG(("TA check list 0: 0x%08x", container->basicOut[1]));
            BDBG_LOG(("TA check list 1(contiguous): 0x%08x", container->basicOut[2]));
        }
        else if (rc == OTPEPOCH_Update_Error_eTaNotLoaded) {
            BDBG_LOG(("Some TAs are not updated since the TAs are not loaded. Please check the TA check list. '1' means the TA is updated; '0' means the TA is not updated."));
            BDBG_LOG(("TA check list (The bit position reflets the index in the TA check list):"));
            BDBG_LOG(("TA check list 0: 0x%08x", container->basicOut[1]));
            BDBG_LOG(("TA check list 1(contiguous): 0x%08x", container->basicOut[2]));
        }
   }
    else {
        BDBG_LOG(("**************************************************************************"));
        BDBG_LOG(("***************     OTP EPOCH UPDATE SUCCEED         *********************"));
        BDBG_LOG(("**************************************************************************"));
    }

    BDBG_LOG(("Please check the Epoch select check list. '1' means the operation is implemented; '0' means means the operation is not implemented"));
    BDBG_LOG(("bit[0] - Update based on sdl id"));
    BDBG_LOG(("bit[1] - Update based on TA list"));
    BDBG_LOG(("bit[2] - Update HVD FW epoch"));
    BDBG_LOG(("bit[3] - Update RAGGA FW epoch"));
    BDBG_LOG(("bit[4] - Update VICE FW epoch"));
    BDBG_LOG(("bit[5] - Update RAVE FW epoch"));
    BDBG_LOG(("Epoch select check list: 0x%08x", container->basicOut[3]));

end:
    if (container) {
        SRAI_Container_Free(container);
        container = NULL;
    }

    return rc;
}

int main(int argc, char *argv[])
{
    int rc = 0;

    BDBG_LOG(("**************************************************************************"));
    BDBG_LOG(("***************     SAGE OTP EPOCH Update tool         *******************"));
    BDBG_LOG(("**************************************************************************"));

    /* Join Nexus: Initialize platform ... */
    rc = SAGE_app_join_nexus();
    if (rc)
        goto leave_nexus;
    rc = _P_Initialize();
    if (rc)
        goto leave;

    if (_parse_cmdline(argc, argv))
    {
        rc = -1;
        goto leave;
    }

    rc = _P_ProcessCertificate();
    if (rc)
        goto leave;

leave:
    _P_Unintialize();

    /* Leave Nexus: Finalize platform ... */
    SAGE_app_leave_nexus();

leave_nexus:
    if (rc) {
        fprintf(stderr, "%s ERROR #%d\n", BSTD_FUNCTION, rc);
    }
    return rc;
}
