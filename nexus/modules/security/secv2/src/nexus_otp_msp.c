/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "nexus_security_module.h"
#include "nexus_base.h"
#include "nexus_types.h"
#include "nexus_security_common.h"
#include "nexus_otp_msp.h"
#include "priv/nexus_security_priv.h"
#include "bhsm_otp_msp.h"

BDBG_MODULE(nexus_otp_msp);


NEXUS_Error NEXUS_OtpMsp_Read( unsigned index, NEXUS_OtpMspRead *pResult )
{
    BERR_Code rc = NEXUS_SUCCESS;
    BHSM_OtpMspRead otpParam;
    BHSM_Handle hHsm;

    BDBG_ENTER( NEXUS_OtpMsp_Read );

    if ( !pResult ) { return BERR_TRACE( NEXUS_INVALID_PARAMETER ); }

    NEXUS_Security_GetHsm_priv( &hHsm );
    if( !hHsm ) { return BERR_TRACE( NEXUS_NOT_INITIALIZED ); }

    BKNI_Memset( &otpParam, 0, sizeof(otpParam) );
    otpParam.index = index;

    rc = BHSM_OtpMsp_Read( hHsm, &otpParam );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( NEXUS_INVALID_PARAMETER ); }

    pResult->data = otpParam.data;
    pResult->valid = otpParam.valid;

    BDBG_LEAVE( NEXUS_OtpMsp_Read );
    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_OtpMsp_Write( unsigned index, const NEXUS_OtpMspWrite *pConf )
{
    BERR_Code rc = NEXUS_SUCCESS;
    BHSM_OtpMspWrite otpParam;
    BHSM_Handle hHsm;

    BDBG_ENTER( NEXUS_OtpMsp_Program );

    if( !pConf ) { return BERR_TRACE( NEXUS_INVALID_PARAMETER ); }

    BKNI_Memset( &otpParam, 0, sizeof(otpParam) );
    otpParam.index = index;
    otpParam.data = pConf->data;
    otpParam.mask = pConf->mask;

    NEXUS_Security_GetHsm_priv( &hHsm );
    if( !hHsm ) { return BERR_TRACE( NEXUS_NOT_INITIALIZED ); }

    rc = BHSM_OtpMsp_Write( hHsm, &otpParam );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( NEXUS_INVALID_PARAMETER ); }

    BDBG_LEAVE( NEXUS_OtpMsp_Program );
    return NEXUS_SUCCESS;
}
