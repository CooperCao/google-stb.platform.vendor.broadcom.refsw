/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ******************************************************************************/

#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "bstd.h"
#include "bkni.h"
#include "bsagelib_types.h"
#include "bsagelib_drm_types.h"
#include "bsagelib_tools.h"

#include "sage_srai.h"
#include "sage_manufacturing_module_ids.h"

#include "nexus_security_datatypes.h"
#if (NEXUS_SECURITY_API_VERSION == 1)
#include "nexus_otpmsp.h"
#include "nexus_read_otp_id.h"
#else
#include "nexus_otp_msp.h"
#include "nexus_otp_key.h"
#endif

#include "sage_manufacturing_api.h"

BDBG_MODULE(sage_manufacturing);

#define OTP_MSP0_VALUE_ZS (0x02)
#define OTP_MSP1_VALUE_ZS (0x02)
#define OTP_MSP0_VALUE_ZB (0x3E)
#define OTP_MSP1_VALUE_ZB (0x3F)

#define MFG_TA_BIN_FILENAME "sage_ta_manufacturing.bin"
#define MFG_TA_DEV_BIN_FILENAME "sage_ta_manufacturing_dev.bin"

/* defines */
#define SIZEOF_DRM_BINFILE_HEADER    (192)
#define SIZEOF_DYNAMIC_OFFSET_HEADER (16)
#define OFFSET_TO_RKS_FIELD          (19)
#define OFFSET_TO_TYPE_FIELD         (20)

#define GET_UINT32_FROM_BUF(pBuf) \
    (((uint32_t)(((uint8_t*)(pBuf))[0]) << 24) | \
     ((uint32_t)(((uint8_t*)(pBuf))[1]) << 16) | \
     ((uint32_t)(((uint8_t*)(pBuf))[2]) << 8)  | \
     ((uint8_t *)(pBuf))[3])

/* Chipset type */
typedef enum
{
    ChipType_eZS = 0,
    ChipType_eZB,
    ChipType_eCustomer,
    ChipType_eCustomer1
} ChipType_e;

/* create the table */
typedef struct SAGE_Manufacturing_P_Settings {
    BSAGElib_InOutContainer *SAGELib_Container;
    SAGE_Manufacturing_OTP_Index OTP_Index;
    SRAI_ModuleHandle provToolModuleHandle;
    SRAI_PlatformHandle platformHandle;
    SRAI_ModuleHandle validationToolModuleHandle;
} SAGE_Manufacturing_P_Settings;

/* static function definitions */
static const char * _MapDrmEnumToString(uint32_t drm_type);
static bool verifyOtpIndex(SAGE_Manufacturing_OTP_Index OTP_Index);
static ChipType_e _P_GetChipType(void);
static BERR_Code _P_TA_Install(void);
static BERR_Code _P_GetFileSize(char * filename, uint32_t *filesize);

static SAGE_Manufacturing_P_Settings SAGE_Private_Settings;

BERR_Code SAGE_Manufacturing_Init(SAGE_Manufacturing_OTP_Index otp_index)
{
    BSAGElib_State state;
    BERR_Code rc = 0;
    BSAGElib_InOutContainer *container = NULL;
#if USE_NXCLIENT /* NxClient customization */
    SRAI_Settings appSettings;
#endif

    /*--- Verify OTP Index Usability First ---*/

    if (otp_index == SAGE_Manufacturing_OTP_Index_UNDEFINED)
    {
        BDBG_LOG(("No OTP key specified as command line argument, setting to --otp_key:A"));
        SAGE_Private_Settings.OTP_Index = SAGE_Manufacturing_OTP_Index_A;
    }
    else
    {
        SAGE_Private_Settings.OTP_Index = otp_index;
    }

    BDBG_LOG(("%s() SAGE_Private_Settings.OTP_Index assigned to %d", BSTD_FUNCTION, SAGE_Private_Settings.OTP_Index));

    /* perform sanity check on choice once NEXUS is up AND only if Type 1 is detected, otherwise doesn't make sense */
    if(verifyOtpIndex(SAGE_Private_Settings.OTP_Index) != true)
    {
        BDBG_ERR(("OTP key index choice '%c' is not valid.  Verify OTP map!!\n", otp_index + 'A'));
        rc = -1;
        goto handle_error;
    }

    /*--- Init SRAI Module ---*/

#if USE_NXCLIENT /* NxClient customization */
    // Get Current Settings
    SRAI_GetSettings(&appSettings);

    // customize appSettings, for example if designed to use NxClient API:
    appSettings.generalHeapIndex = NXCLIENT_FULL_HEAP;
    appSettings.videoSecureHeapIndex = NXCLIENT_VIDEO_SECURE_HEAP;

    // Save/Apply new settings
    SRAI_SetSettings(&appSettings);
#endif

    rc = _P_TA_Install();
    if(rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s: error opening MFG platform", BSTD_FUNCTION));
        goto handle_error;
    }

    container = SRAI_Container_Allocate();
    if (container == NULL)
    {
        rc = 1;
        goto handle_error;
    }

    /* Open the Manufacturing platform */
    rc = SRAI_Platform_Open(BSAGE_PLATFORM_ID_MANUFACTURING,
                                 &state,
                                 &SAGE_Private_Settings.platformHandle);
    if (rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s: error opening MFG platform", BSTD_FUNCTION));
        goto handle_error;
    }

    /* Check init state */
    if (state != BSAGElib_State_eInit)
    {
        /* Not yet initialized: send init command */
        rc = SRAI_Platform_Init(SAGE_Private_Settings.platformHandle, NULL);
        if (rc != BERR_SUCCESS)
        {
            BDBG_ERR(("%s: error initializing MFG platform", BSTD_FUNCTION));
            goto handle_error;
        }
    }

    /* Initialize Provisioning module */
    rc = SRAI_Module_Init(SAGE_Private_Settings.platformHandle,
                               PROVISIONING_MODULE,
                               container, &SAGE_Private_Settings.provToolModuleHandle);
    if ((rc != BERR_SUCCESS) || (container->basicOut[0] != 0))
    {
        goto handle_error;
    }

    /* Initialize Validation module */
    rc = SRAI_Module_Init(SAGE_Private_Settings.platformHandle,
                               VALIDATION_MODULE,
                               container, &SAGE_Private_Settings.validationToolModuleHandle);
    if ((rc != BERR_SUCCESS) || (container->basicOut[0] != 0))
    {
        goto handle_error;
    }

    BDBG_LOG(("%s: Sage Manufacturing platform and module initialized", BSTD_FUNCTION));

handle_error:
    if (rc != BERR_SUCCESS)
    {
        /* error: cleanup */
        SAGE_Manufacturing_Uninit();
    }

    if(container != NULL)
    {
        BDBG_LOG(("\tFreeing SRAI container\n"));
        SRAI_Container_Free(container);
        container = NULL;
    }

    return rc;
}

void SAGE_Manufacturing_Uninit(void)
{
    if (SAGE_Private_Settings.validationToolModuleHandle)
    {
        BDBG_LOG(("Uninitializing Validation module '%p'", (void*)SAGE_Private_Settings.validationToolModuleHandle));
        SRAI_Module_Uninit(SAGE_Private_Settings.validationToolModuleHandle);
        SAGE_Private_Settings.validationToolModuleHandle = NULL;
    }

    if (SAGE_Private_Settings.provToolModuleHandle)
    {
        BDBG_LOG(("Uninitializing Provisioning module '%p'", (void*)SAGE_Private_Settings.provToolModuleHandle));
        SRAI_Module_Uninit(SAGE_Private_Settings.provToolModuleHandle);
        SAGE_Private_Settings.provToolModuleHandle = NULL;
    }

    if (SAGE_Private_Settings.platformHandle)
    {
        BDBG_LOG(("Closing Manufacturing platform '%p'", (void*)SAGE_Private_Settings.platformHandle));
        SRAI_Platform_Close(SAGE_Private_Settings.platformHandle);
        SAGE_Private_Settings.platformHandle = NULL;
        BDBG_LOG(("Uninstalling Manufacturing platform..."));
        SRAI_Platform_UnInstall(BSAGE_PLATFORM_ID_MANUFACTURING);
    }

    return;
}

int SAGE_Manufacturing_AllocBinBuffer(size_t file_size, uint8_t **ppBinDataBuffer)
{
    int rc = 0;
    BSAGElib_InOutContainer *container = NULL;

    container = SRAI_Container_Allocate();
    if (container == NULL)
    {
        BDBG_ERR(("Error allocating SRAI container"));
        rc = 1;
        goto handle_error;
    }

    /****************************************************
     * Allocate SRAI buffer
     *****************************************************/
    container->blocks[0].data.ptr = SRAI_Memory_Allocate(file_size, SRAI_MemoryType_Shared);
    if(container->blocks[0].data.ptr == NULL)
    {
        BDBG_ERR(("\tCannot allocate '%zu' bytes of memory ^^^^^^^^^^^^^^^^^^^", file_size));
        rc = 1;
        goto handle_error;
    }
    container->blocks[0].len = file_size;

    *ppBinDataBuffer = container->blocks[0].data.ptr;
    BDBG_LOG(("%s(): Allocating SRAI buffer (%zu bytes) success.", BSTD_FUNCTION, file_size));

    /* Save container to settings */
    SAGE_Private_Settings.SAGELib_Container = container;

handle_error:

    return rc;
}

int SAGE_Manufacturing_DeallocBinBuffer()
{
    int rc = 0;
    BSAGElib_InOutContainer *container = SAGE_Private_Settings.SAGELib_Container;

    if (container != NULL)
    {
        BDBG_MSG(("\tFreeing shared memory\n"));
        if(container->blocks[0].data.ptr != NULL)
        {
            SRAI_Memory_Free(container->blocks[0].data.ptr);
            container->blocks[0].data.ptr = NULL;
        }
        BDBG_MSG(("\tFreeing SRAI container\n"));
        SRAI_Container_Free(container);
        container = NULL;
    }

    return rc;
}



/* todo: add buff size param + use it to check out of bound access */
int SAGE_Manufacturing_BinFile_ParseAndDisplay(uint8_t* pBinData, uint32_t binFileLength, int *validationCommand)
{
    uint32_t number_of_drms = 0;
    uint32_t current_drm_type = 0;
    uint32_t i;
    const char * pString = NULL;

    if(binFileLength < (SIZEOF_DRM_BINFILE_HEADER + SIZEOF_DYNAMIC_OFFSET_HEADER))
    {
        BDBG_ERR(("%s: DRM bin file length '%u' is less than the minimum size of a valid bin file (%u)", BSTD_FUNCTION,
                  binFileLength, (SIZEOF_DRM_BINFILE_HEADER + SIZEOF_DYNAMIC_OFFSET_HEADER)));
        return -1;
    }

    /* jump to the end of the header and get the number of DRMs */
    number_of_drms = GET_UINT32_FROM_BUF(&pBinData[SIZEOF_DRM_BINFILE_HEADER]);

    /* sanity check */
    if(number_of_drms > MAX_NUMBER_BINFILE_DRM_TYPES)
    {
        BDBG_ERR(("\tNumber of DRMs detected in header (%u) exceeds number of supported types (%u)",
                  number_of_drms, MAX_NUMBER_BINFILE_DRM_TYPES));
        return -1;
    }

    BDBG_LOG(("\tThe current DRM bin file contains the following '%u' DRMs:", number_of_drms));
    for(i = 0; i < number_of_drms; i++)
    {
        /* todo : check out of bound access */
        current_drm_type = GET_UINT32_FROM_BUF(&pBinData[SIZEOF_DRM_BINFILE_HEADER + SIZEOF_DYNAMIC_OFFSET_HEADER + 16*i]);
        pString = _MapDrmEnumToString(current_drm_type);
        if(pString == NULL)
        {
            BDBG_ERR(("\tInvalid DRM type detected (0x%08x), exiting application", current_drm_type));
            return -1;
        }

        /* Set the flag if a DRM bin file uses HDCP-2.2 */
        if((!strcmp(pString, "HDCP 2.2 RX")) || (!strcmp(pString, "HDCP 2.2 TX")))
        {
            *validationCommand |= VALIDATION_COMMAND_ValidateHdcp22;
        }
        else if(!strcmp(pString, "EDRM"))
        {
            *validationCommand |= VALIDATION_COMMAND_ValidateEdrm;
        }
        else if(!strcmp(pString, "ECC"))
        {
            *validationCommand |= VALIDATION_COMMAND_ValidateEcc;
        }
        else if(!strcmp(pString, "DTCP-IP"))
        {
            *validationCommand |= VALIDATION_COMMAND_ValidateDtcpIp;
        }
        else if(!strcmp(pString, "MEDIAROOM"))
        {
            *validationCommand |= VALIDATION_COMMAND_ValidateMediaroom;
        }
        BDBG_LOG(("\t\t>>>> %s (0x%08x)\n", pString, current_drm_type));
    }

    return 0;
}

int SAGE_Manufacturing_VerifyDrmBinFileType(uint8_t* pBinData, int validationCommand)
{
    /* Check to see if already bound with OTP Key A-H */
    if((pBinData[OFFSET_TO_RKS_FIELD] >= 0x01 && pBinData[OFFSET_TO_RKS_FIELD] <= 0x08) && pBinData[OFFSET_TO_TYPE_FIELD] == 0x03)
    {
        BDBG_LOG(("\t*** DRM bin file is already bound to chip (Type 3) therefore will not be provisioned ***"));
        if((validationCommand & VALIDATION_COMMAND_ValidateHdcp22) & 0xF)
        {
            BDBG_LOG(("\t*** HDCP 2.2 key(s) detected, validation will proceed. Ignoring otp_key flag (if specified)...***"));
        }
        if((validationCommand & VALIDATION_COMMAND_ValidateEdrm) & 0xF)
        {
            BDBG_LOG(("\t*** EDRM keys detected, validation will proceed. Ignoring otp_key flag (if specified)...***"));
        }
        if((validationCommand & VALIDATION_COMMAND_ValidateEcc) & 0xF)
        {
            BDBG_LOG(("\t*** ECC keys detected, validation will proceed. Ignoring otp_key flag (if specified)...***"));
        }
        if((validationCommand & VALIDATION_COMMAND_ValidateDtcpIp) & 0xF)
        {
            BDBG_LOG(("\t*** DTCP-IP keys detected, validation will proceed. Ignoring otp_key flag (if specified)...***"));
        }
        if((validationCommand & VALIDATION_COMMAND_ValidateMediaroom) & 0x1F)
        {
            BDBG_LOG(("\t*** Mediaroom keys detected, validation will proceed. Ignoring otp_key flag (if specified)...***"));
        }
        BDBG_LOG(("\n\n"));
        return DRM_BIN_FILE_TYPE_3;
    }

    if(pBinData[OFFSET_TO_RKS_FIELD] == 0 && pBinData[OFFSET_TO_TYPE_FIELD] == 0x03)
    {
        BDBG_LOG(("\t*** DRM bin file is already bound to chip (Type 3 prime) therefore will not be provisioned ***"));
        if((validationCommand & VALIDATION_COMMAND_ValidateHdcp22) & 0xF)
        {
            BDBG_LOG(("\t*** HDCP 2.2 key(s) detected, validation will proceed. Ignoring otp_key flag (if specified)...***"));
        }
        if((validationCommand & VALIDATION_COMMAND_ValidateEdrm) & 0xF)
        {
            BDBG_LOG(("\t*** EDRM keys detected, validation will proceed. Ignoring otp_key flag (if specified)...***"));
        }
        if((validationCommand & VALIDATION_COMMAND_ValidateEcc) & 0xF)
        {
            BDBG_LOG(("\t*** ECC keys detected, validation will proceed. Ignoring otp_key flag (if specified)...***"));
        }
        if((validationCommand & VALIDATION_COMMAND_ValidateDtcpIp) & 0xF)
        {
            BDBG_LOG(("\t*** DTCP-IP keys detected, validation will proceed. Ignoring otp_key flag (if specified)...***"));
        }
        if((validationCommand & VALIDATION_COMMAND_ValidateMediaroom) & 0x1F)
        {
            BDBG_LOG(("\t*** Mediaroom keys detected, validation will proceed. Ignoring otp_key flag (if specified)...***"));
        }
        BDBG_LOG(("\n\n"));
        return DRM_BIN_FILE_TYPE_3_PRIME;
    }

    /* Otherwise, already bound with CusKey */
    if((pBinData[OFFSET_TO_RKS_FIELD] == 0x00 || pBinData[OFFSET_TO_RKS_FIELD] == 0x0C) && pBinData[OFFSET_TO_TYPE_FIELD] == 0x01)
    {
        return DRM_BIN_FILE_TYPE_1;
    }

    /* Type 2 */
    if((pBinData[OFFSET_TO_RKS_FIELD] == 0x00 || pBinData[OFFSET_TO_RKS_FIELD] == 0x0C) && pBinData[OFFSET_TO_TYPE_FIELD] == 0x02)
    {
        return DRM_BIN_FILE_TYPE_2;
    }

    /* Type 0 */
    if(pBinData[OFFSET_TO_RKS_FIELD] == 0x00 && pBinData[OFFSET_TO_TYPE_FIELD] == 0x00)
    {
        return DRM_BIN_FILE_TYPE_0;
    }
    BDBG_LOG(("\n"));
    return 0;
}

BERR_Code SAGE_Manufacturing_Provision_BinData(int *pStatus)
{
    BERR_Code rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *container = SAGE_Private_Settings.SAGELib_Container;

    BDBG_LOG(("\tSENDING Provision command to Sage with OTP index '0x%02x'\n", SAGE_Private_Settings.OTP_Index));

    /* set the OTP key index specified by the user and make it map to SAGE_Crypto enum for RootKeySrc hence the +1 */
    container->basicIn[0] = SAGE_Private_Settings.OTP_Index + 1;

    rc = SRAI_Module_ProcessCommand(SAGE_Private_Settings.provToolModuleHandle, PROVISIONING_COMMAND_ProcessBinFile, container);
    if ((rc != BERR_SUCCESS) || (container->basicOut[0] != 0))
    {
        BDBG_ERR(("\tError provisioning DRM bin file (rc = 0x%08x), container->basicOut[0] = 0x%08x", rc, container->basicOut[0]));
        goto handle_error;
    }
    BDBG_LOG(("\tProvisioning command successfully returned from SAGE, analyzing result....\n"));

    if (container->basicOut[1] == OPERATION_SUCCESSFULLY_OCCURRED){
        *pStatus = 0;
    }
    else{
        *pStatus = -1;
    }

handle_error:
    return rc;
}

int SAGE_Manufacturing_ValidateDRM(int *pStatus, int validationCommand)
{
    int rc = 0;
    int sage_rc = 0;
    int validationErrorFlag = 0;
    BSAGElib_InOutContainer *container = SAGE_Private_Settings.SAGELib_Container;

    sage_rc = SRAI_Module_ProcessCommand(SAGE_Private_Settings.validationToolModuleHandle, validationCommand, container);
    if ((sage_rc != BERR_SUCCESS) || (container->basicOut[0] != 0))
    {
        BDBG_ERR(("\tError processing bin file (validation) w/ sage_rc = 0x%x", sage_rc));
        rc = 1;
        *pStatus = -1;
        goto handle_error;
    }
    BDBG_LOG(("\tValidation command successfully returned from SAGE, analyzing result (%u, %u).",
              container->basicOut[0], container->basicOut[1]));

    if ((validationCommand & VALIDATION_COMMAND_ValidateHdcp22) & 0xF) {
        if ((container->basicOut[2] & HDCP22_MASK) == OPERATION_SUCCESSFULLY_OCCURRED)
            *pStatus = 0;
        else {
            BDBG_ERR(("\tError validating HDCP-2.2 (return value: %u)!!!", (container->basicOut[2] & HDCP22_MASK)));
            *pStatus = -1;
            validationErrorFlag = -1;
        }
    }
    if ((validationCommand & VALIDATION_COMMAND_ValidateEdrm) & 0xF) {
        if (((container->basicOut[2] & EDRM_MASK) >> 1 == OPERATION_SUCCESSFULLY_OCCURRED) && validationErrorFlag != -1)
            *pStatus = 0;
        else {
            BDBG_ERR(("\tError validating EDRM (return value: %u)!!!", (container->basicOut[2] & EDRM_MASK)));
            *pStatus = -1;
            validationErrorFlag = -1;
        }
    }
    if ((validationCommand & VALIDATION_COMMAND_ValidateEcc) & 0xF) {
        if (((container->basicOut[2] & ECC_MASK) >> 2 == OPERATION_SUCCESSFULLY_OCCURRED) && validationErrorFlag != -1)
            *pStatus = 0;
        else {
            BDBG_ERR(("\tError validating ECC (return value: %u)!!!", (container->basicOut[2] & ECC_MASK)));
            *pStatus = -1;
        }
    }
    if ((validationCommand & VALIDATION_COMMAND_ValidateDtcpIp) & 0xF) {
        if (((container->basicOut[2] & DTCP_IP_MASK) >> 3 == OPERATION_SUCCESSFULLY_OCCURRED) && validationErrorFlag != -1)
            *pStatus = 0;
        else {
            BDBG_ERR(("\tError validating DTCP-IP (return value: %u)!!!", (container->basicOut[2] & DTCP_IP_MASK)));
            *pStatus = -1;
        }
    }
    if ((validationCommand & VALIDATION_COMMAND_ValidateMediaroom) & 0xF) {
        if (((container->basicOut[2] & MEDIAROOM_MASK) >> 3 == OPERATION_SUCCESSFULLY_OCCURRED) && validationErrorFlag != -1)
            *pStatus = 0;
        else {
            BDBG_ERR(("\tError validating Mediaroom (return value: %u)!!!", (container->basicOut[2] & MEDIAROOM_MASK)));
            *pStatus = -1;
        }
    }
handle_error:
    return rc;
}

int SAGE_Manufacturing_GetErrorInfo(int *error_code, char *p_error_string)
{
    BSAGElib_InOutContainer *container = SAGE_Private_Settings.SAGELib_Container;

    if (container != NULL)
    {
        *error_code = container->basicOut[1];
        strcpy(p_error_string, BSAGElib_Tools_ReturnCodeToString(container->basicOut[1]));
        return 0;
    }
    else
        return -1;
}


static const char * _MapDrmEnumToString(uint32_t drm_type)
{
    switch(drm_type)
    {
    case BSAGElib_BinFileDrmType_eNetflix:
        return "NETFLIX";
    case BSAGElib_BinFileDrmType_eWidevine:
           return "WIDEVINE";
    case BSAGElib_BinFileDrmType_eDtcpIp:
           return "DTCP-IP";
    case BSAGElib_BinFileDrmType_ePlayready:
           return "PLAYREADY 2.5";
    case BSAGElib_BinFileDrmType_eSecureRsa:
           return "SECURE_RSA";
    case BSAGElib_BinFileDrmType_eCustomPrivate:
           return "CUSTOM_PRIVATE";
    case BSAGElib_BinFileDrmType_eAdobeAxcess:
           return "ADOBE";
    case BSAGElib_BinFileDrmType_eHdcp22Rx:
           return "HDCP 2.2 RX";
    case BSAGElib_BinFileDrmType_eHdcp22Tx:
           return "HDCP 2.2 TX";
    case BSAGElib_BinFileDrmType_eSslCerts:
           return "SSL Certificates";
    case BSAGElib_BinFileDrmType_eGeneric:
           return "Generic";
    case BSAGElib_BinFileDrmType_eEdrm:
           return "EDRM";
    case BSAGElib_BinFileDrmType_eEcc:
           return "ECC";
    case BSAGElib_BinFileDrmType_ePlayready30:
           return "PLAYREADY 3.0";
    case BSAGElib_BinFileDrmType_eMediaroom:
           return "MEDIAROOM";
    case BSAGElib_BinFileDrmType_eBp3:
           return "BP3";
    case BSAGElib_BinFileDrmType_eSecuremedia:
           return "SECUREMEDIA";
    case BSAGElib_BinFileDrmType_eHdcp14Rx:
           return "HDCP 1.4 RX";
    case BSAGElib_BinFileDrmType_eHdcp14Tx:
           return "HDCP 1.4 TX";
    case BSAGElib_BinFileDrmType_eMarlin:
           return "MARLIN";
    case BSAGElib_BinFileDrmType_eKeyMaster:
           return "KEY_MASTER";
    case BSAGElib_BinFileDrmType_eMax:
           return NULL;
    default:
        return NULL;
    }
    return NULL;
}

#if (NEXUS_SECURITY_API_VERSION == 1)
static bool verifyOtpIndex(SAGE_Manufacturing_OTP_Index OTP_Index)
{
    bool brc = true;
    NEXUS_ReadMspParms readMspParms;
    NEXUS_ReadMspIO readMspIO;
    NEXUS_ReadOtpIO readOtpIO;
    NEXUS_OtpIdOutput otpRead;
    NEXUS_OtpIdType OtpIdArray[] =
              { NEXUS_OtpIdType_eA,
                NEXUS_OtpIdType_eB,
                NEXUS_OtpIdType_eC,
                NEXUS_OtpIdType_eD,
                NEXUS_OtpIdType_eE,
                NEXUS_OtpIdType_eF/*,
                NEXUS_OtpIdType_eG,
                NEXUS_OtpIdType_eH*/};

    /* int otp_index = OTP_Index - SAGE_Manufacturing_OTP_Index_A; */

    BDBG_LOG(("%s - Selecting element '%u'", BSTD_FUNCTION, OTP_Index));

    if(NEXUS_Security_ReadOtpId( OtpIdArray[OTP_Index], &otpRead ) != NEXUS_SUCCESS )
    {
        BDBG_ERR(("%s - NEXUS_Security_ReadOtpId() failed.", BSTD_FUNCTION ));
        brc = false;
        goto ErrorExit;
    }

    BKNI_Memset( &readMspParms, 0, sizeof(readMspParms) );

    readMspParms.readMspEnum = NEXUS_OtpCmdReadRegister_eKeyMc0_CustomerMode;

    if(NEXUS_Security_ReadMSP( &readMspParms, &readMspIO ) != NEXUS_SUCCESS )
    {
        BDBG_ERR(("%s - NEXUS_Security_ReadMSP() FAILED for customer mode.  Field may not be programmed...", BSTD_FUNCTION ));
        brc = false;
        goto ErrorExit;
    }

    BKNI_Memset( &readOtpIO, 0, sizeof(NEXUS_ReadOtpIO) );

    if( NEXUS_Security_ReadOTP(    /*NEXUS_OtpCmdReadRegister*/25,
                                   OtpIdArray[OTP_Index],
                                   &readOtpIO)   != NEXUS_SUCCESS )
    {
        BDBG_ERR(("%s - NEXUS_Security_ReadMSP() FAILED for SageKeyladderDisallowed mode", BSTD_FUNCTION));
        brc = false;
        goto ErrorExit;
    }

    BDBG_MSG(("data = %02x %02x %02x %02x ...", readOtpIO.otpKeyIdBuf[0], readOtpIO.otpKeyIdBuf[1], readOtpIO.otpKeyIdBuf[2],readOtpIO.otpKeyIdBuf[3]));
    BDBG_MSG(("       %02x %02x %02x %02x ", readOtpIO.otpKeyIdBuf[4], readOtpIO.otpKeyIdBuf[5], readOtpIO.otpKeyIdBuf[6],readOtpIO.otpKeyIdBuf[7]));
    if(readOtpIO.otpKeyIdBuf[7] == 1)
    {
        BDBG_ERR(("%s - Specified OTP key index is not allowed to use SAGE keyladder", BSTD_FUNCTION));
        brc = false;
        goto ErrorExit;
    }

ErrorExit:
    return brc;
}

static ChipType_e _P_GetChipType(void)
{
    NEXUS_ReadMspParms     readMspParms;
    NEXUS_ReadMspIO        readMsp0;
    NEXUS_ReadMspIO        readMsp1;
    NEXUS_Error rc =  NEXUS_SUCCESS;

    readMspParms.readMspEnum = NEXUS_OtpCmdMsp_eReserved233;
    rc = NEXUS_Security_ReadMSP(&readMspParms,&readMsp0);

    readMspParms.readMspEnum = NEXUS_OtpCmdMsp_eReserved234;
    rc = NEXUS_Security_ReadMSP(&readMspParms,&readMsp1);

    BDBG_MSG(("OTP MSP0 %d %d %d %d OTP MSP0 %d %d %d %d",readMsp0.mspDataBuf[0], readMsp0.mspDataBuf[1], readMsp0.mspDataBuf[2], readMsp0.mspDataBuf[3],
                                                          readMsp1.mspDataBuf[0], readMsp1.mspDataBuf[1], readMsp1.mspDataBuf[2], readMsp1.mspDataBuf[3]));

    if((readMsp0.mspDataBuf[3] == OTP_MSP0_VALUE_ZS) && (readMsp1.mspDataBuf[3] == OTP_MSP1_VALUE_ZS)){
        return ChipType_eZS;
    }
    return ChipType_eZB;
}
#else
typedef enum otpIdType
{
    otpIdType_eA,
    otpIdType_eB,
    otpIdType_eC,
    otpIdType_eD,
    otpIdType_eE,
    otpIdType_eF,
    otpIdType_eG,
    otpIdType_eH,

    otpIdType_eMax
}   otpIdType;

static bool verifyOtpIndex(SAGE_Manufacturing_OTP_Index OTP_Index)
{
    bool brc = true;
    NEXUS_OtpMspRead readMspIO;
    NEXUS_OtpKeyInfo otpRead;
    otpIdType OtpIdArray[] =
              { otpIdType_eA,
                otpIdType_eB,
                otpIdType_eC,
                otpIdType_eD,
                otpIdType_eE,
                otpIdType_eF/*,
                otpIdType_eG,
                otpIdType_eH*/};

    /* int otp_index = OTP_Index - SAGE_Manufacturing_OTP_Index_A; */

    BDBG_LOG(("%s - Selecting element '%u'", BSTD_FUNCTION, OTP_Index));

    if(NEXUS_OtpKey_GetInfo( OtpIdArray[OTP_Index], &otpRead ) != NEXUS_SUCCESS )
    {
        BDBG_ERR(("%s - NEXUS_Security_ReadOtpId() failed.", BSTD_FUNCTION ));
        brc = false;
        goto ErrorExit;
    }

    if(NEXUS_OtpMsp_Read( 6, &readMspIO ) != NEXUS_SUCCESS )
    {
        BDBG_ERR(("%s - NEXUS_Security_ReadMSP() FAILED for customer mode.  Field may not be programmed...", BSTD_FUNCTION ));
        brc = false;
        goto ErrorExit;
    }

    BDBG_MSG(("data = %02x %02x %02x %02x ...", otpRead.id[0], otpRead.id[1], otpRead.id[2],otpRead.id[3]));
    BDBG_MSG(("       %02x %02x %02x %02x ", otpRead.id[4], otpRead.id[5], otpRead.id[6],otpRead.id[7]));
    if(otpRead.id[7] == 1)
    {
        BDBG_ERR(("%s - Specified OTP key index is not allowed to use SAGE keyladder", BSTD_FUNCTION));
        brc = false;
        goto ErrorExit;
    }

ErrorExit:
    return brc;
}


static ChipType_e _P_GetChipType(void)
{
    NEXUS_OtpMspRead        readMsp0;
    NEXUS_OtpMspRead        readMsp1;
    uint32_t Msp0Data;
    uint32_t Msp1Data;
    NEXUS_Error rc =  NEXUS_SUCCESS;
#if NEXUS_ZEUS_VERSION < NEXUS_ZEUS_VERSION_CALC(5,0)
    rc = NEXUS_OtpMsp_Read(233, &readMsp0);
    rc = NEXUS_OtpMsp_Read(234, &readMsp1);
#else
    rc = NEXUS_OtpMsp_Read(224, &readMsp0);
    rc = NEXUS_OtpMsp_Read(225, &readMsp1);
#endif
    Msp0Data = readMsp0.data & readMsp0.valid;
    Msp1Data = readMsp1.data & readMsp1.valid;

    BDBG_MSG(("OTP MSP0 %u OTP MSP1 %u", Msp0Data, Msp1Data));

    if((Msp0Data == OTP_MSP0_VALUE_ZS) && (Msp1Data == OTP_MSP1_VALUE_ZS)){
        return ChipType_eZS;
    }

    return ChipType_eZB;
}
#endif


BERR_Code _P_TA_Install(void)
{
    BERR_Code rc = BERR_SUCCESS;
    char *ta_bin_filename;
    FILE * fptr = NULL;
    uint32_t file_size = 0;
    uint32_t read_size = 0;
    uint8_t *ta_bin_file_buff = NULL;
    BERR_Code sage_rc = BERR_SUCCESS;

    BDBG_ENTER(_P_TA_Install);

    if(_P_GetChipType() == ChipType_eZS)
        ta_bin_filename = MFG_TA_DEV_BIN_FILENAME;
    else
        ta_bin_filename = MFG_TA_BIN_FILENAME;

    BDBG_MSG(("%s - Loadable TA filename '%s'", BSTD_FUNCTION, ta_bin_filename));

    rc = _P_GetFileSize(ta_bin_filename, &file_size);
    if(rc != BERR_SUCCESS)
    {
        BDBG_LOG(("%s - Error determine file size of TA bin file", BSTD_FUNCTION));
        goto ErrorExit;
    }

    ta_bin_file_buff = SRAI_Memory_Allocate(file_size, SRAI_MemoryType_Shared);
    if(ta_bin_file_buff == NULL)
    {
        BDBG_ERR(("%s - Error allocating '%u' bytes for loading TA bin file", BSTD_FUNCTION, file_size));
        rc = BERR_INVALID_PARAMETER;
        goto ErrorExit;
    }

    fptr = fopen(ta_bin_filename, "rb");
    if(fptr == NULL)
    {
        BDBG_ERR(("%s - Error opening TA bin file (%s)", BSTD_FUNCTION, ta_bin_filename));
        rc = BERR_INVALID_PARAMETER;
        goto ErrorExit;
    }

    read_size = fread(ta_bin_file_buff, 1, file_size, fptr);
    if(read_size != file_size)
    {
        BDBG_ERR(("%s - Error reading TA bin file size (%u != %u)", BSTD_FUNCTION, read_size, file_size));
        rc = BERR_INVALID_PARAMETER;
        goto ErrorExit;
    }

    /* close file and set to NULL */
    if(fclose(fptr) != 0)
    {
        BDBG_ERR(("%s - Error closing TA bin file '%s'.  (%s)", BSTD_FUNCTION, ta_bin_filename, strerror(errno)));
        rc = BERR_INVALID_PARAMETER;
        goto ErrorExit;
    }
    fptr = NULL;

    BDBG_MSG(("%s - TA 0x%08x Install file '%s'", BSTD_FUNCTION, BSAGE_PLATFORM_ID_MANUFACTURING, ta_bin_filename));

    sage_rc = SRAI_Platform_Install(BSAGE_PLATFORM_ID_MANUFACTURING, ta_bin_file_buff, file_size);
    if(sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error calling SRAI_Platform_Install Error 0x%08x", BSTD_FUNCTION, sage_rc ));
        rc = BERR_INVALID_PARAMETER;
        goto ErrorExit;
    }

ErrorExit:

    if(ta_bin_file_buff != NULL){
        SRAI_Memory_Free(ta_bin_file_buff);
        ta_bin_file_buff = NULL;
    }

    if(fptr != NULL){
        fclose(fptr);
        fptr = NULL;
    }

    BDBG_LEAVE(_P_TA_Install);

    return rc;
}

BERR_Code _P_GetFileSize(char * filename, uint32_t *filesize)
{
    BERR_Code rc = BERR_SUCCESS;
    FILE *fptr = NULL;
    int pos = 0;

    BDBG_ENTER(_P_GetFileSize);

    fptr = fopen(filename, "rb");
    if(fptr == NULL)
    {
        BDBG_LOG(("%s - Error opening file '%s'.  (%s)", BSTD_FUNCTION, filename, strerror(errno)));
        rc = BERR_INVALID_PARAMETER;
        goto ErrorExit;
    }

    pos = fseek(fptr, 0, SEEK_END);
    if(pos == -1)
    {
        BDBG_ERR(("%s - Error seeking to end of file '%s'.  (%s)", BSTD_FUNCTION, filename, strerror(errno)));
        rc = BERR_INVALID_PARAMETER;
        goto ErrorExit;
    }

    pos = ftell(fptr);
    if(pos == -1)
    {
        BDBG_ERR(("%s - Error determining position of file pointer of file '%s'.  (%s)", BSTD_FUNCTION, filename, strerror(errno)));
        rc = BERR_INVALID_PARAMETER;
        goto ErrorExit;
    }

    /* check vs. arbitrary large file size */
    if(pos >= 2*1024*1024)
    {
        BDBG_ERR(("%s - Invalid file size detected for of file '%s'.  (%u)", BSTD_FUNCTION, filename, pos));
        rc = BERR_INVALID_PARAMETER;
        goto ErrorExit;
    }

    (*filesize) = pos;

ErrorExit:

    BDBG_MSG(("%s - Exiting function (%u bytes)", BSTD_FUNCTION, (*filesize)));

    if(fptr != NULL)
    {
        /* error closing?!  weird error case not sure how to handle */
        if(fclose(fptr) != 0){
            BDBG_ERR(("%s - Error closing Loadable TA file '%s'.  (%s)", BSTD_FUNCTION, filename, strerror(errno)));
            rc = 1;
        }
    }

    BDBG_LEAVE(_P_GetFileSize);

    return rc;
}
