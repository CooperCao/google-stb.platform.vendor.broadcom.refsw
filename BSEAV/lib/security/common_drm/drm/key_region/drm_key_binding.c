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
 **************************************************************************/
#include "bstd.h"
#include "bdbg.h"
#include "bkni.h"

#if (NEXUS_SECURITY_API_VERSION==1)
#include "nexus_otpmsp.h"
#else
#include "nexus_otp_msp.h"
#include "nexus_otp_key.h"
#include "nexus_otp_msp_indexes.h"
#endif

#include "drm_key_binding.h"

#include "drm_common_swcrypto.h"

BDBG_MODULE(drm_key_binding);

/******************************************************************************
 ** FUNCTION:
 **   DRM_KeyBinding_Init
 **
 ** DESCRIPTION:
 **   Currently unused since there are no initialization requirements for drm_key_binding
 **
 ** RETURNS:
 **   N/A
 **
 ******************************************************************************/
void DRM_KeyBinding_Init(void)
{
    return;
}

/******************************************************************************
 ** FUNCTION:
 **   DRM_KeyBinding_UnInit
 **
 ** DESCRIPTION:
 **   Currently unused since there are no closure requirements for drm_key_binding
 **
 ** RETURNS:
 **   N/A
 **
 ******************************************************************************/
void DRM_KeyBinding_UnInit(void)
{
    return;
}

/******************************************************************************
 ** FUNCTION:
 **   DRM_KeyBinding_FetchDeviceIds
 **
 ** DESCRIPTION:
 **   Retrieve the OTP IDs
 **
 ** PARAMETERS:
 **   pStruct [out] - Pointer to struct to contain the OTP IDs
 **
 ** RETURNS:
 **   Success -- Drm_Success
 **   Failure -- Other
 **
 ******************************************************************************/
DrmRC DRM_KeyBinding_FetchDeviceIds(drm_key_binding_t *pStruct)
{
    unsigned count = 0;
    DrmRC rc = Drm_Success;
#if (NEXUS_SECURITY_API_VERSION==1)
    NEXUS_ReadOtpIO readOtpIO;
    NEXUS_OtpCmdReadRegister readOtpEnum;
    NEXUS_OtpKeyType keyType;

    BKNI_Memset(&readOtpIO, 0x00, sizeof(NEXUS_ReadOtpIO));

    for(count = 0; count < 2; count++)
    {
        readOtpEnum = NEXUS_OtpCmdReadRegister_eKeyID;

        if(count == 0)
        {
            keyType = NEXUS_OtpKeyType_eA;
        }
        else
        {
            keyType = NEXUS_OtpKeyType_eB;
        }

        if(NEXUS_Security_ReadOTP(readOtpEnum, keyType, &readOtpIO) != NEXUS_SUCCESS)
        {
            BDBG_ERR(("%s - Call to 'NEXUS_Security_ReadOTP' FAILED.", BSTD_FUNCTION));
            rc = Drm_BindingErr;
            goto ErrorExit;
        }
        else
        {
            BDBG_MSG(("%s - Call to 'NEXUS_Security_ReadOTP' succeeded. otpKeyIdSize = '0x%08x'", BSTD_FUNCTION, readOtpIO.otpKeyIdSize));
            if(count == 0){
                BKNI_Memcpy(pStruct->devIdA, readOtpIO.otpKeyIdBuf, readOtpIO.otpKeyIdSize);
            }
            else{
                BKNI_Memcpy(pStruct->devIdB, readOtpIO.otpKeyIdBuf, readOtpIO.otpKeyIdSize);
            }
        }
    }//end of for-loop
#else
    NEXUS_OtpKeyInfo otpKeyInfo;
    BKNI_Memset(&otpKeyInfo, 0x00, sizeof(NEXUS_OtpKeyInfo));

    for(count = 0; count < 2; count++)
    {

        if(NEXUS_OtpKey_GetInfo( count, &otpKeyInfo ) != NEXUS_SUCCESS)
        {
            BDBG_ERR(("%s - Call to 'NEXUS_OtpKey_GetInfo' FAILED.", BSTD_FUNCTION));
            rc = Drm_BindingErr;
            goto ErrorExit;
        }
        else
        {
            BDBG_MSG(("%s - Call to 'NEXUS_OtpKey_GetInfo' succeeded. otpKeyIdSize = '0x%08x'", BSTD_FUNCTION, NEXUS_OTP_KEY_ID_LENGTH));
            if(count == 0){
                BKNI_Memcpy(pStruct->devIdA, otpKeyInfo.id, NEXUS_OTP_KEY_ID_LENGTH);
            }
            else{
                BKNI_Memcpy(pStruct->devIdB, otpKeyInfo.id, NEXUS_OTP_KEY_ID_LENGTH);
            }
        }
    }//end of for-loop
#endif

 ErrorExit:
    BDBG_MSG(("%s - Exiting function", BSTD_FUNCTION));
    return rc;
}

/******************************************************************************
 ** FUNCTION:
 **   DRM_KeyBinding_GetProcsFromOtp
 **
 ** DESCRIPTION:
 **   Read Proc_in1 and Proc_in2 values from OTP region
 **
 ** PARAMETERS:
 **   pStruct [out] - Pointer to struct to contain the proc_in values
 **
 ** RETURNS:
 **   Success -- Drm_Success
 **   Failure -- Other
 **
 ******************************************************************************/
DrmRC DRM_KeyBinding_GetProcsFromOtp(drm_key_binding_t *pStruct)
{
    DrmRC rc = Drm_Success;
    unsigned count = 0;

#if (NEXUS_SECURITY_API_VERSION==1)
    NEXUS_ReadOtpIO readOtpIO;
    NEXUS_OtpCmdReadRegister readOtpEnum;
    NEXUS_OtpKeyType keyType;

    BKNI_Memset(&readOtpIO, 0x00, sizeof(NEXUS_ReadOtpIO));

    for(count = 0; count < 4; count++)
    {
        readOtpEnum = NEXUS_OtpCmdReadRegister_eKeyID;

        switch(count)
        {
        case 0:
            keyType = NEXUS_OtpKeyType_eC;
            break;

        case 1:
            keyType = NEXUS_OtpKeyType_eD;
            break;

        case 2:
            keyType = NEXUS_OtpKeyType_eE;
            break;

        case 3:
            keyType = NEXUS_OtpKeyType_eF;
            break;

        /* coverity[dead_error_begin] */
        default:
            break;
        }

        if(NEXUS_Security_ReadOTP(readOtpEnum, keyType, &readOtpIO) != NEXUS_SUCCESS)
        {
            BDBG_ERR(("%s - Call to 'BHSM_ReadOTP' FAILED.", BSTD_FUNCTION));
            rc = Drm_BindingErr;
            goto ErrorExit;
        }
        else
        {
            BDBG_MSG(("%s - Call to 'NEXUS_Security_ReadOTP' succeeded. otpKeyIdSize = '0x%08x'", BSTD_FUNCTION, readOtpIO.otpKeyIdSize));
            switch(count)
            {
            case 0:
                BKNI_Memcpy(pStruct->proc_in1, readOtpIO.otpKeyIdBuf, readOtpIO.otpKeyIdSize);
                break;

            case 1:
                BKNI_Memcpy(&(pStruct->proc_in1[8]), readOtpIO.otpKeyIdBuf, readOtpIO.otpKeyIdSize);
                break;

            case 2:
                BKNI_Memcpy(pStruct->proc_in2, readOtpIO.otpKeyIdBuf, readOtpIO.otpKeyIdSize);
                break;

            case 3:
                BKNI_Memcpy(&(pStruct->proc_in2[8]), readOtpIO.otpKeyIdBuf, readOtpIO.otpKeyIdSize);
                break;

            /* coverity[dead_error_begin] */
            default:
                break;
            }
        }
    }//end of for-loop
#else
        NEXUS_OtpKeyInfo otpKeyInfo;
        BKNI_Memset(&otpKeyInfo, 0x00, sizeof(NEXUS_OtpKeyInfo));
        for(count = 0; count < 4; count++)
    {
        if(NEXUS_OtpKey_GetInfo( count, &otpKeyInfo ) != NEXUS_SUCCESS)
        {
            BDBG_ERR(("%s - Call to 'BHSM_ReadOTP' FAILED.", BSTD_FUNCTION));
            rc = Drm_BindingErr;
            goto ErrorExit;
        }
        else
        {
            BDBG_MSG(("%s - Call to 'NEXUS_Security_ReadOTP' succeeded. otpKeyIdSize = '0x%08x'", BSTD_FUNCTION,  NEXUS_OTP_KEY_ID_LENGTH));
            switch(count)
            {
            case 0:
                BKNI_Memcpy(pStruct->proc_in1, otpKeyInfo.id,  NEXUS_OTP_KEY_ID_LENGTH);
                break;

            case 1:
                BKNI_Memcpy(&(pStruct->proc_in1[8]), otpKeyInfo.id,  NEXUS_OTP_KEY_ID_LENGTH);
                break;

            case 2:
                BKNI_Memcpy(pStruct->proc_in2, otpKeyInfo.id,  NEXUS_OTP_KEY_ID_LENGTH);
                break;

            case 3:
                BKNI_Memcpy(&(pStruct->proc_in2[8]), otpKeyInfo.id,  NEXUS_OTP_KEY_ID_LENGTH);
                break;

            default:
                break;
            }
        }
    }//end of for-loop
#endif
 ErrorExit:
    BDBG_MSG(("%s - Exiting function", BSTD_FUNCTION));
    return rc;
}

/******************************************************************************
 ** FUNCTION:
 **   DRM_KeyBinding_GenerateProcsFromOtp
 **
 ** DESCRIPTION:
 **   Generate Proc_in1 and Proc_in2 from OTP values
 **
 ** PARAMETERS:
 **   pStruct [in/out] - Pointer to struct containing the OTP ID, etc.
 **   pProcIn1FromBinHeader [in] - Pointer to proc_in1 input data
 **   pProcIn2FromBinHeader [in] - Pointer to proc_in2 input data
 **
 ** RETURNS:
 **   Success -- Drm_Success (Proc values generated)
 **   Failure -- Other
 **
 ******************************************************************************/
DrmRC DRM_KeyBinding_GenerateProcsFromOtp(
    drm_key_binding_t *pStruct,
    uint8_t *pProcIn1FromBinHeader,
    uint8_t *pProcIn2FromBinHeader)
{
    DrmRC rc = Drm_Success;
    uint8_t all_zero[16] = {0x00};
    uint8_t tmp_buf[24] = {0x00};
    uint8_t hash_value[20] = {0x00};

    if((BKNI_Memcmp(pStruct->devIdA, all_zero, 8) == 0) ||
       (BKNI_Memcmp(pStruct->devIdB, all_zero, 8) == 0))
    {
        BDBG_ERR(("%s - device ID A or B is all-zero", BSTD_FUNCTION));
        rc = Drm_BindingErr;
        goto ErrorExit;
    }

    if((BKNI_Memcmp(pProcIn1FromBinHeader, all_zero, 16) == 0) ||
       (BKNI_Memcmp(pProcIn2FromBinHeader, all_zero, 16) == 0))
    {
        BDBG_ERR(("%s - ProcIn1 or ProcIn2 from bin file header is all-zero", BSTD_FUNCTION));
        rc = Drm_BindingErr;
        goto ErrorExit;
    }

    /* Flag unusual case of devIdA == devIdB but keep going */
    if(BKNI_Memcmp(pStruct->devIdA, pStruct->devIdB, 8) == 0){
        BDBG_ERR(("%s - Not a production part, continuing......", BSTD_FUNCTION));
    }

    /* Generate ProcIn1 */
    BKNI_Memcpy(tmp_buf, pStruct->devIdA, 8);
    BKNI_Memcpy(&tmp_buf[8], pProcIn1FromBinHeader, 16);

    rc = DRM_Common_SwSha1(tmp_buf, hash_value,  24);
    if(rc != Drm_Success)
    {
        BDBG_ERR( ("%s - Error calculating SHA-1", BSTD_FUNCTION));
        goto ErrorExit;
    }

    BKNI_Memcpy(pStruct->proc_in1, &hash_value[4], 16);

    /* Generate ProcIn2 */
    BKNI_Memcpy(tmp_buf, pStruct->devIdB, 8);
    BKNI_Memcpy(&tmp_buf[8], pProcIn2FromBinHeader, 16);

    rc = DRM_Common_SwSha1(tmp_buf, hash_value,  24);
    if(rc != Drm_Success)
    {
        BDBG_ERR( ("%s - Error calculating SHA-1", BSTD_FUNCTION));
        goto ErrorExit;
    }

    BKNI_Memcpy(pStruct->proc_in2, hash_value, 16);

 ErrorExit:
    BDBG_MSG(("%s - Exiting function", BSTD_FUNCTION));
    return rc;
}
