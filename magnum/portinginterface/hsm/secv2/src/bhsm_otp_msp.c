/******************************************************************************
* Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include "bhsm.h"
#include "bhsm_priv.h"
#include "bhsm_bsp_msg.h"
#include "bhsm_otp_msp.h"
#include "bhsm_otp_priv.h"
#include "bhsm_p_otpmsp.h"
#include "bhsm_p_otpmisc.h"

BDBG_MODULE( BHSM );

BERR_Code BHSM_OtpMsp_Write( BHSM_Handle hHsm, BHSM_OtpMspWrite *pParam )
{
    BHSM_P_OtpMspProg bspParam;
    BHSM_P_OtpMiscProgPatternSet bspPattern;
    unsigned i;
    BERR_Code rc;

    BKNI_Memset( &bspPattern, 0, sizeof(bspPattern) );
    bspPattern.in.patternArray[0] = 0xBC32F4AC;
    bspPattern.in.patternArray[1] = 0xD18616B6;
    bspPattern.in.patternArray[2] = 0x9FEB4D54;
    bspPattern.in.patternArray[3] = 0x4A27BF4A;
    bspPattern.in.patternArray[4] = 0xCF1C3178;
    bspPattern.in.patternArray[5] = 0xE2DB98A0;
    bspPattern.in.patternArray[6] = 0x24F64BBA;
    bspPattern.in.patternArray[7] = 0x7698E712;
    bspPattern.in.patternArray[8] = 0x0000F48D;
    rc = BHSM_P_OtpMisc_ProgPatternSet( hHsm, &bspPattern );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    BKNI_Memset( &bspParam, 0, sizeof(bspParam) );

    bspParam.in.cmdMspEnum = pParam->index;
    bspParam.in.data = pParam->data;
    bspParam.in.mask = pParam->mask;
    for( i = 0; i < 32; i++ ) {
        if( pParam->mask & (1<<i) ){ bspParam.in.numBits++; }
    }

    rc = BHSM_P_OtpMsp_Prog( hHsm, &bspParam );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    return BERR_SUCCESS;
}

BERR_Code BHSM_OtpMsp_Read( BHSM_Handle hHsm, BHSM_OtpMspRead *pParam )
{
    BHSM_P_OtpMspRead bspParam;
    BERR_Code rc;

    BKNI_Memset( &bspParam, 0, sizeof(bspParam) );

    bspParam.in.cmdMspEnum = pParam->index;

    rc = BHSM_P_OtpMsp_Read( hHsm, &bspParam );
    if( rc != BERR_SUCCESS ) {
        BDBG_ERR(("%s Can't read MSP-OTP[%d]", BSTD_FUNCTION, pParam->index ));
        return BERR_TRACE( rc );
    }

    pParam->data = bspParam.out.mspData;
    pParam->valid = bspParam.out.mspLock;

    return BERR_SUCCESS;
}



BERR_Code BHSM_OtpMsp_ReadRange( BHSM_Handle hHsm, BHSM_OtpMspReadRange *pParam )
{
    BSTD_UNUSED( hHsm );
    BSTD_UNUSED( pParam );

   /*  BERR_Code BHSM_P_OtpMsp_MultiRead( BHSM_Handle hHsm, BHSM_P_OtpMspMultiRead *pParam )*/
    return BERR_TRACE( BERR_NOT_SUPPORTED );
}
