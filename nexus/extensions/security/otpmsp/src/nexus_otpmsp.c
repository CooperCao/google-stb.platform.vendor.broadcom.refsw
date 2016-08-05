/******************************************************************************
* Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
*
* This program is the proprietary software of Broadcom and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant
* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied), right
* to use, or waiver of any kind with respect to the Software, and Broadcom
* expressly reserves all rights in and to the Software and all intellectual
* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. , WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/

#include "nexus_security_module.h"
#include "nexus_security_datatypes.h"
#include "nexus_security.h"
#include "priv/nexus_security_priv.h"
#include "priv/nexus_core.h"

#include "bhsm.h"
#include "bsp_s_commands.h"
#include "bsp_s_misc.h"
#include "bhsm_misc.h"
#include "bsp_s_hw.h"

#include "bhsm_otpmsp.h"

BDBG_MODULE( nexus_security );

#if (BHSM_ZEUS_VERSION <  BHSM_ZEUS_VERSION_CALC(1,0))
NEXUS_Error NEXUS_Security_ProgramOTP(
    const NEXUS_ProgramOtpIO    *pProgOtpIO
    )
{
    BSTD_UNUSED(pProgOtpIO);
    return BERR_TRACE( NEXUS_NOT_SUPPORTED );
}
#endif

NEXUS_Error NEXUS_Security_ReadOTP(
    NEXUS_OtpCmdReadRegister    readOtpEnum,
   #if (BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(1,0))
    NEXUS_OtpKeyType            keyType,
   #endif
    NEXUS_ReadOtpIO        *pReadOtpIO
    )
{
    BHSM_ReadOtpIO_t readOtpParm;
    BERR_Code        rc;
    BHSM_Handle      hHsm;

    NEXUS_Security_GetHsm_priv(&hHsm);
    if( !hHsm )
    {
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }

    /* formulate the request structure */
    readOtpParm.readOtpEnum = (BPI_Otp_CmdReadRegister_e)readOtpEnum;
    #if BHSM_ZEUS_VERSION >=  BHSM_ZEUS_VERSION_CALC(1,0)
    readOtpParm.keyType = (BPI_Otp_KeyType_e)keyType;
    #endif

    rc = BHSM_ReadOTP( hHsm, &readOtpParm );
    if( rc != BERR_SUCCESS )
    {
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }

   #if BHSM_ZEUS_VERSION >=  BHSM_ZEUS_VERSION_CALC(1,0)

    BKNI_Memcpy( (void*)(&pReadOtpIO->otpKeyIdBuf[4]), (void*)readOtpParm.regValueLo, BHSM_OTP_DATA_LEN );
    pReadOtpIO->otpKeyIdSize = BHSM_OTP_DATA_LEN;

    if( readOtpEnum == NEXUS_OtpCmdReadRegister_eKeyID || readOtpEnum == NEXUS_OtpCmdReadRegister_eKeyHash )
    {
        BKNI_Memcpy( (void*)pReadOtpIO->otpKeyIdBuf, (void*)readOtpParm.regValueHi, BHSM_OTP_DATA_LEN );
        pReadOtpIO->otpKeyIdSize += BHSM_OTP_DATA_LEN;
    }

   #else

    BKNI_Memcpy((void *)pReadOtpIO->otpKeyIdBuf, (void *)readOtpParm.aucKeyId, BHSM_OTP_KEY_ID_LEN);
    pReadOtpIO->otpKeyIdSize = BHSM_OTP_KEY_ID_LEN;

   #endif

    return NEXUS_SUCCESS;
}


NEXUS_Error NEXUS_Security_ProgramMSP(
    const NEXUS_ProgramMspIO    *pProgMspIO
    )
{
    BERR_Code           rc;
    BHSM_Handle         hHsm;
    BHSM_ProgramMspIO_t programMspIO;
    unsigned int        i;

    NEXUS_Security_GetHsm_priv(&hHsm);
    if( !hHsm )
    {
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }

    /* form the BHSM command structure */
    programMspIO.progMode     = (BCMD_Msp_CmdProgMode_e)pProgMspIO->progMode;
    programMspIO.progMspEnum  = (BCMD_Otp_CmdMsp_e)pProgMspIO->progMspEnum;
    programMspIO.ucDataBitLen = pProgMspIO->dataBitLen;
    for (i = 0; i < NEXUS_MSP_DATA_LEN; i++)
    {
        programMspIO.aucDataBitMask[i] = pProgMspIO->dataBitMask[i];
        programMspIO.aucMspData[i]     = pProgMspIO->mspData[i];
    }

    rc = BHSM_ProgramMSP(hHsm, &programMspIO);
    if( rc != BERR_SUCCESS )
    {
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }

    return NEXUS_SUCCESS;
}


NEXUS_Error      NEXUS_Security_ReadMSP(
    const NEXUS_ReadMspParms    *pReadMspParms,
    NEXUS_ReadMspIO              *pReadMspIO
    )
{
    BERR_Code        rc;
    BHSM_Handle      hHsm;
    BHSM_ReadMspIO_t readMspParm;

    NEXUS_Security_GetHsm_priv(&hHsm);
    if( !hHsm )
    {
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }

    /* formulate the request structure */
    BKNI_Memset(&readMspParm, 0, sizeof(readMspParm));
    readMspParm.readMspEnum = (BCMD_Otp_CmdMsp_e)pReadMspParms->readMspEnum;

    #if (BHSM_ZEUS_VERSION < BHSM_ZEUS_VERSION_CALC(1,0))
    readMspParm.bCheckSignature = pReadMspParms->checkSignature;
    #endif

    rc = BHSM_ReadMSP( hHsm, &readMspParm );
    if( rc != BERR_SUCCESS )
    {
        /* error, its likely that you don't have read permission on the OTP you are trying to read. */
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }

   #if (BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(1,0))
    BKNI_Memcpy((void *)pReadMspIO->mspDataBuf, (void *)readMspParm.aucMspData, BHSM_MSP_DATA_LEN);
    pReadMspIO->mspDataSize = BHSM_MSP_DATA_LEN;

    BKNI_Memcpy((void *)pReadMspIO->lockMspDataBuf, (void *)readMspParm.aucLockMspData, BHSM_MSP_DATA_LEN);
   #else
    BKNI_Memcpy((void *)pReadMspIO->mspDataBuf, (void *)readMspParm.aucMspData, BHSM_MSP_OUTPUT_DATA_LEN);
    pReadMspIO->mspDataSize = BHSM_MSP_OUTPUT_DATA_LEN;
   #endif

    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Security_ReadDataSect(
    NEXUS_OtpDataSection   readDsEnum,
    NEXUS_ReadDataSectIO  *pReadDataSectIO
    )
{
    BERR_Code             rc;
    BHSM_Handle           hHsm;
    BHSM_ReadDataSectIO_t dataSectParm;

    NEXUS_Security_GetHsm_priv( &hHsm );
    if( !hHsm )
    {
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }

    /* formulate the request structure */
    dataSectParm.readDsEnum = (BPI_Otp_DataSection_e)readDsEnum;

    rc = BHSM_ReadDataSect(hHsm, &dataSectParm);
    if( rc != BERR_SUCCESS )
    {
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }

    BKNI_Memcpy( (void *)pReadDataSectIO->dataSectBuf,
                 (void *)dataSectParm.aucDataSectData,
                 BHSM_READ_DATA_SECTION_DATA_LEN );

    pReadDataSectIO->dataSectSize = BHSM_READ_DATA_SECTION_DATA_LEN;

    return NEXUS_SUCCESS;
}


NEXUS_Error NEXUS_Security_ProgramDataSect (
    const NEXUS_ProgramDataSectIO  *pProgDataSectIO
    )
{
    BERR_Code    rc;
    BHSM_Handle  hHsm;

    NEXUS_Security_GetHsm_priv(&hHsm);
    if( !hHsm )
    {
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }

    rc = BHSM_ProgramDataSect(hHsm, (BHSM_ProgramDataSectIO_t *)pProgDataSectIO);
    if( rc != BERR_SUCCESS )
    {
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }

    return NEXUS_SUCCESS;
}
