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

#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include "bstd.h"
#include "bkni.h"

#include "nexus_platform.h"
#include "nexus_platform_init.h"
#include "sage_manufacturing_module_ids.h"

#if USE_NXCLIENT
#include "nxclient.h"
#endif

#include "sage_manufacturing_api.h"


BDBG_MODULE(sage_manufacturing_tool);


#define OTP_KEY_FLAG                 "--otp_key:"

static void print_help(void);

/*one global structure if possible */
typedef struct
{
    char InputFile[256];
    char OutputFile[256];
    int  option;
} combinedAppStruct;

combinedAppStruct gAppStruct;

static const char *INPUT_FOLDER = "./Input";
static const char *OUTPUT_FOLDER = "./Output";

int validationCommand = 0;

static int SAGE_Provisioning_ProcessBinFile(const char *path, const struct stat *sptr, int type);

/* main
 * The manufacutring platform can run in a standalone application or
 * be a part of a larger one.
 */

int main(int argc, char* argv[])
{
    int rc = 0;
    NEXUS_Error rc_nexus;
    char * pArgv = NULL;
    int arg_counter = 1;

    SAGE_Manufacturing_OTP_Index otp_index = SAGE_Manufacturing_OTP_Index_UNDEFINED;

    BDBG_LOG(("\n\n\n"));
    BDBG_LOG(("**************************************************************************"));
    BDBG_LOG(("**************************************************************************"));
    BDBG_LOG(("***************     Broadcom Manufacturing Utility     *******************"));
    BDBG_LOG(("**************************************************************************"));
    BDBG_LOG(("**************************************************************************\n"));

    BKNI_Memset((uint8_t*)&gAppStruct, 0x00, sizeof(combinedAppStruct));

    /*--- Verify Input/Output Directory ---*/
    rc =  access(OUTPUT_FOLDER, W_OK);
    if(rc != 0)
    {
        BDBG_ERR(("'%s' directory does not exist or folder is not writeable", OUTPUT_FOLDER));
        print_help();
        goto handle_error;
    }

    rc =  access(INPUT_FOLDER, R_OK);
    if(rc != 0)
    {
        BDBG_ERR(("'%s' directory does not exist or folder is not readable", INPUT_FOLDER));
        print_help();
        goto handle_error;
    }

    if(argc > 4)
    {
        BDBG_ERR(("\t Invalid number of command line arguments (%u)\n", argc));
        print_help();
        rc = -1;
        goto handle_error;
    }

    /* start by assumning no input file (i.e. option 2) */
    gAppStruct.option = 2;

    /*--- Verify OTP Index Usable ---*/

    /* point to first argument following the executable name */
    pArgv = argv[arg_counter];
    while(pArgv != NULL)
    {
        BDBG_MSG(("current argument '%s'", pArgv));
        if(strstr(pArgv, OTP_KEY_FLAG) != NULL)
        {
            /* fetch index */
            pArgv += strlen(OTP_KEY_FLAG);
            BDBG_MSG(("after incrementing '%s'", pArgv));

            /* index should only be one character long so fail if otherwise */
            if(strlen(pArgv)!= 1)
            {
                BDBG_ERR(("Invalid OTP key index choice '%s', valid range is A-H (a-h)", pArgv));
                print_help();
                rc = -1;
                goto handle_error;
            }

            otp_index = toupper((int)pArgv[0]) - 'A';
            if(otp_index > SAGE_Manufacturing_OTP_Index_H)
            {
                BDBG_ERR(("Invalid OTP key index choice '%c', valid range is A-H (a-h)", otp_index + 'A'));
                print_help();
                rc = -1;
                goto handle_error;
            }
            BDBG_LOG(("OTP index = '%c' OTP key index specified '%s' (otp_index=%d)", otp_index + 'A', pArgv, otp_index));
        }
        else
        {
            /* If the InputFile member has not been filled, fill it, otherwise invalid state */
            if(strlen(gAppStruct.InputFile) == 0)
            {
                BKNI_Memcpy(gAppStruct.InputFile, pArgv, strlen(pArgv));
                gAppStruct.option = 1;
            }
            else
            {
                BDBG_ERR(("Invalid arguments. Input file detected '%s'.  Current argument is '%s'. ERROR, verify command line",
                           gAppStruct.InputFile, pArgv));
                print_help();
                rc = -1;
                goto handle_error;
            }
        }

        arg_counter++;
        pArgv = argv[arg_counter];
    }

#if USE_NXCLIENT
    NxClient_JoinSettings joinSettings;

    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "openssl_rsa_sage");
    rc_nexus = NxClient_Join(&joinSettings);
    if (rc_nexus != NEXUS_SUCCESS) {
        BDBG_ERR(("Error joining NxClient: please check if nxserver is running already!!\n"));
        goto handle_error;
    }

#else
    NEXUS_PlatformSettings platformSettings;

    BDBG_LOG(("\t up all Nexus modules for platform using default settings\n\n"));

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;

    rc_nexus = NEXUS_Platform_Init(&platformSettings);
    if (rc_nexus != NEXUS_SUCCESS) {
        BDBG_ERR(("Error calling Nexus Platform init\n"));
        goto handle_error;
    }

#endif


    /*--- Init SAGE provisioning & Verify OTP Index usability ---*/

    /* Initialize manufacturing platform and provisioning and validtion modules */
    rc = SAGE_Manufacturing_Init(otp_index);
    if (rc != 0) {
        goto handle_error;
    }

    /*--- Read Input DRM bin file ---*/
    if(gAppStruct.option == 1)
    {
        rc = SAGE_Provisioning_ProcessBinFile(gAppStruct.InputFile, NULL, FTW_F);
    }
    else if(gAppStruct.option == 2)
    {
        ftw(INPUT_FOLDER, SAGE_Provisioning_ProcessBinFile, 1);
    }
    else {
        /* TBD: error-handling if gAppStruct.option != 1 or 2 */
        goto handle_error;
    }


handle_error:
    BDBG_LOG(("Shutting down manufacturing platform and modules"));
    SAGE_Manufacturing_Uninit();

#if USE_NXCLIENT
    /* Leave Nexus: Finalize platform ... */
    BDBG_LOG(("Closing Nexus driver..."));
    NxClient_Uninit();
#else
    NEXUS_Platform_Uninit();
#endif

    if (rc) {
        BDBG_ERR(("Failure running SAGE manufacturing tool\n"));
    }

    return rc;
}


static int SAGE_Provisioning_ProcessBinFile(const char *path, const struct stat *sptr, int type)
{
    int rc = 0;
    size_t file_size = 0;
    size_t bytes_read = 0;
    uint8_t *pDRMBinDataBuffer = NULL;
    int drm_binfile_type = 0;
    int prov_status = -1;
    int validation_status = -1;
    int error_code = 0;
    char error_string[256];
    FILE *fptr = NULL;
    char *filename = NULL;

    BSTD_UNUSED(sptr);

    BDBG_LOG(("\tRead '%s' and send to SAGE for provisioning/binding...", path));

    if (type != FTW_F)
    {
        if(strncmp(path, INPUT_FOLDER, strlen(INPUT_FOLDER)) == 0)
        {
            BDBG_LOG(("Input directory case, continue"));
            rc = 0;
        }
        else
        {
            BDBG_LOG(("'%s' is not a file, error\n", path));
            memset(gAppStruct.InputFile, 0x00, sizeof(gAppStruct.InputFile));
            rc = -2;
        }
        goto handle_error;
    }
    memcpy(gAppStruct.InputFile, path, strlen(path));

    rc =  access(gAppStruct.InputFile, R_OK);
    if(rc != 0)
    {
        BDBG_ERR(("Error: '%s' doesn't exist or is not readable\n", gAppStruct.InputFile));
        print_help();
        goto handle_error;
    }

    BDBG_LOG(("\tFile to provision and/or validate = '%s'\n", gAppStruct.InputFile));

    /****************************************************
     * Determine file size
     *****************************************************/
    fptr = fopen (gAppStruct.InputFile, "rb");
    if (fptr == NULL)
    {
        BDBG_ERR(("\tError opening file '%s' in order to determine size ^^^^^^", gAppStruct.InputFile));
        rc = 1;
        goto handle_error;
    }

    if (fseek (fptr, 0, SEEK_END) != 0)
    {
        BDBG_ERR(("Error seeking to the end of the file (%s)", strerror(errno)));
        rc = 1;
        goto handle_error;
    }

    file_size = ftell (fptr);
    if (fseek (fptr, 0, SEEK_SET) != 0)
    {
        BDBG_ERR(("Error rewinding file pointer to beginning of file (%s)", strerror(errno)));
        rc = 1;
        goto handle_error;
    }

    BDBG_LOG(("\tSize of '%s' is '%u' bytes.\n", gAppStruct.InputFile, file_size));


    /*--- Allocate Bin Processing Buffer ---*/

    rc =  SAGE_Manufacturing_AllocBinBuffer(file_size, &pDRMBinDataBuffer);
    if(rc != 0)
    {
        BDBG_ERR(("Error Allocating Bin Processing Buffer with size %d\n", file_size));
        print_help();
        goto handle_error;
    }

    /*--- Read Input DRM Bin To Buffer ---*/

    bytes_read = fread(pDRMBinDataBuffer, 1, file_size, fptr);
    if (bytes_read != file_size)
    {
        BDBG_ERR(("\tError amounts of bytes_read '%u' != file_size '%u'  ^^^^^^^^^^^^^^^^^^^", bytes_read, file_size));
        rc = 1;
        goto handle_error;
    }

    /*--- Parse Input DRM bin data ---*/

    rc = SAGE_Manufacturing_BinFile_ParseAndDisplay(pDRMBinDataBuffer, bytes_read, &validationCommand);
    if(rc != 0)
    {
        BDBG_ERR(("Error Parsing Input DRM bin file (rc = %d)\n", rc));
        goto handle_error;
    }

    /*--- Verify Input DRM bin type ---*/

    /* if Buffer[19] == 0x01 && buffer[20] == 0x03 -> Already type 3, will not be provisioned */
    drm_binfile_type = SAGE_Manufacturing_VerifyDrmBinFileType(pDRMBinDataBuffer, validationCommand);

    /* Run Provisioning tool */
    if(drm_binfile_type <= DRM_BIN_FILE_TYPE_2)
    {
        rc = SAGE_Manufacturing_Provision_BinData(&prov_status);
        if (rc != 0)
        {
            BDBG_ERR(("Error Provisioning Input DRM bin file (rc = %d)\n", rc));
            goto handle_error;
        }
    }

    /* Run Validation tool if validation command is set appropriately */
    if(validationCommand)
    {
        rc = SAGE_Manufacturing_ValidateDRM(&validation_status, validationCommand);
        if (rc != 0)
        {
            BDBG_ERR(("Error validating Input DRM bin file (rc = %d)\n", rc));
            goto handle_error;
        }
    }

handle_error:
    if(fptr != NULL)
    {
        fclose(fptr);
        fptr = NULL;
    }

    if (!prov_status && (drm_binfile_type != DRM_BIN_FILE_TYPE_3))
    {
        BDBG_LOG(("\t*** SAGE-side provisioning successfully completed!!! ****"));
        if(((validationCommand & VALIDATION_COMMAND_ValidateHdcp22) & 0xF))
        {
            if(validation_status == 0){
                BDBG_LOG(("\t********* SAGE-side HDCP-2.2 validation successfully completed!!! *********"));
            }
            else{
                BDBG_ERR(("\tHDCP-2.2 validation FAILED!!!! No file will be provisioned/written to rootfs"));
                goto end;
            }
        }
        if(((validationCommand & VALIDATION_COMMAND_ValidateEdrm) & 0xF) && !validation_status)
        {
            if(validation_status == 0){
                BDBG_LOG(("\t********* SAGE-side EDRM validation successfully completed!!! *********"));
            }
            else{
                BDBG_ERR(("\tEDRM validation FAILED!!!! No file will be provisioned/written to rootfs"));
                goto end;
            }
        }
        if(((validationCommand & VALIDATION_COMMAND_ValidateEcc) & 0xF) && !validation_status)
        {
            if(validation_status == 0){
                BDBG_LOG(("\t********* SAGE-side ECC validation successfully completed!!! *********"));
            }
            else{
                BDBG_ERR(("\tEDRM validation FAILED!!!! No file will be provisioned/written to rootfs"));
                goto end;
            }
        }

        filename = strrchr(gAppStruct.InputFile, '/');
        if(filename == NULL)
        {
            BDBG_ERR(("\tCannot locate filename only from '%s' string.", gAppStruct.InputFile));
        }
        else
        {
            sprintf(gAppStruct.OutputFile, "%s%s", OUTPUT_FOLDER, filename);
            if(strlen(gAppStruct.OutputFile) == 0)
            {
                BDBG_ERR(("\tInvalid state of output file name string (%s).   Failed to create bounded DRM bin file", filename));
            }
            else
            {
                fptr = fopen (gAppStruct.OutputFile, "w+b");
                if (fptr != NULL)
                {
                    BDBG_LOG(("\tAttempting to write new file to '%s'\n", gAppStruct.OutputFile));
                    bytes_read = fwrite(pDRMBinDataBuffer, 1, file_size, fptr);
                    if (bytes_read != file_size)
                    {
                        BDBG_ERR(("\tError amounts of bytes written '%u' != file_size '%u'", bytes_read, file_size));
                    }
                    else
                    {
                        BDBG_LOG(("\tFile '%s' successfully created", gAppStruct.OutputFile));
                        fclose(fptr);
                    }
                }
                else
                {
                    BDBG_ERR(("\tError opening file '%s'after binding ^^^^^(%s)", gAppStruct.OutputFile, strerror(errno)));
                    return -1;
                }
            }
        }
    }
    else if(drm_binfile_type == DRM_BIN_FILE_TYPE_3)
    {
        if(((validationCommand & VALIDATION_COMMAND_ValidateHdcp22) & 0xF) && !validation_status)
        {
            if(validation_status == 0){
                BDBG_LOG(("\t*** SAGE-side HDCP-2.2 validation successfully completed for type 3 DRM bin file!!! ***"));
            }
            else{
                BDBG_ERR(("\tHDCP-2.2 validation FAILED for Type 3 DRM bin file!!!"));
                goto end;
            }
        }
        if(((validationCommand & VALIDATION_COMMAND_ValidateEdrm) & 0xF) && !validation_status)
        {
            if(validation_status == 0){
                BDBG_LOG(("\t*** SAGE-side EDRM validation successfully completed for type 3 DRM bin file!!! ***"));
            }
            else{
                BDBG_ERR(("\tEDRM validation FAILED for Type 3 DRM bin file!!!"));
                goto end;
            }
        }
        if(((validationCommand & VALIDATION_COMMAND_ValidateEcc) & 0xF) && !validation_status)
        {
            if(validation_status == 0){
                BDBG_LOG(("\t*** SAGE-side ECC validation successfully completed for type 3 DRM bin file!!! ***"));
            }
            else{
                BDBG_ERR(("\tECC validation FAILED for Type 3 DRM bin file!!!"));
                goto end;
            }
        }
    }
    else
    {
        /* Display error code received from SAGE side */
        rc = SAGE_Manufacturing_GetErrorInfo(&error_code, error_string);
        if (rc == 0 && error_code != 0)
        {
            BDBG_LOG(("The following SAGE error occurred during the DRM binfile binding process (0x%08x):", error_code));
            BDBG_LOG(("\t%s", error_string));
        }
    }

end:
    memset(gAppStruct.InputFile, 0x00, sizeof(gAppStruct.InputFile));
    memset(gAppStruct.OutputFile, 0x00, sizeof(gAppStruct.OutputFile));
    rc = SAGE_Manufacturing_DeallocBinBuffer();
    if (rc != 0)
    {
        BDBG_ERR(("Error Deallocate DRM bin buffer (rc = %d)\n", rc));
        goto handle_error;
    }
    return rc;
}




static void print_help(void)
{
    BDBG_LOG(("\n\n"));
    BDBG_LOG(("<<<<<<<<< HELP MENU >>>>>>>>"));
    BDBG_LOG(("\tOption 1)    ./nexus brcm_drm_manufacturing_tool <path to DRM bin file>\n"));
    BDBG_LOG(("\tOption 2)    ./nexus brcm_drm_manufacturing_tool\n"));
    BDBG_LOG(("\tIn the case of option 2) the DRM bin file(s) should be placed in the ./Input/ directory\n"));
    BDBG_LOG(("\tFlags:"));
    BDBG_LOG(("\t  [--otp_key:<index>] specify what OTP key index to use for the binding process"));
    BDBG_LOG(("\n\n"));
    return;
}
