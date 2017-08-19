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
 ******************************************************************************/

#include "bhsm_priv.h"
#include "bhsm_bsp_msg.h"
#include "bhsm_otp_key.h"
#include "bhsm_otp_priv.h"
#include "bhsm_keyladder.h"
#include "bsp_types.h"
#include "bhsm_p_otpmode0.h"

BDBG_MODULE( BHSM );



BERR_Code BHSM_OtpKey_GetInfo( BHSM_Handle hHsm, BHSM_OtpKeyInfo *pKeyInfo )
{
    BERR_Code rc;
    BHSM_P_OtpMode0KeyMc0Read keyRead;
    BDBG_ENTER( BHSM_OtpKey_GetInfo );

    /* KeyHash */
    BKNI_Memset( &keyRead, 0, sizeof(keyRead) );
    keyRead.in.sectionIndex = pKeyInfo->index;
    keyRead.in.field = Bsp_Otp_KeyField_eKeyHash;
    rc = BHSM_P_OtpMode0_KeyMc0Read( hHsm, &keyRead );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE(rc); }
    BHSM_MemcpySwap( &pKeyInfo->hash[4], &keyRead.out.data[0], 4 );
    BHSM_MemcpySwap( &pKeyInfo->hash[0], &keyRead.out.data[1], 4 );

    /* KeyId */
    BKNI_Memset( &keyRead, 0, sizeof(keyRead) );
    keyRead.in.sectionIndex = pKeyInfo->index;
    keyRead.in.field = Bsp_Otp_KeyField_eKeyID;
    rc = BHSM_P_OtpMode0_KeyMc0Read( hHsm, &keyRead );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE(rc); }
    BHSM_MemcpySwap( &pKeyInfo->id[4], keyRead.out.data, 4 );
    BHSM_MemcpySwap( &pKeyInfo->id[0], keyRead.out.data, 4 );

    /* ca Keyladder allowed*/
    BKNI_Memset( &keyRead, 0, sizeof(keyRead) );
    keyRead.in.sectionIndex = pKeyInfo->index;
    keyRead.in.field = Bsp_Otp_KeyField_eMc0CaKeyLadderDisallow;
    rc = BHSM_P_OtpMode0_KeyMc0Read( hHsm, &keyRead );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE(rc); }
    if( !keyRead.out.data[0] ) pKeyInfo->caKeyLadderAllow = true;

    /* cp Keyladder allowed*/
    BKNI_Memset( &keyRead, 0, sizeof(keyRead) );
    keyRead.in.sectionIndex = pKeyInfo->index;
    keyRead.in.field = Bsp_Otp_KeyField_eMc0CpKeyLadderDisallow;
    rc = BHSM_P_OtpMode0_KeyMc0Read( hHsm, &keyRead );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE(rc); }
    if( !keyRead.out.data[0] ) pKeyInfo->cpKeyLadderAllow = true;

    /* gp1 Keyladder allowed*/
    BKNI_Memset( &keyRead, 0, sizeof(keyRead) );
    keyRead.in.sectionIndex = pKeyInfo->index;
    keyRead.in.field = Bsp_Otp_KeyField_eMc0Gp1KeyLadderDisallow;
    rc = BHSM_P_OtpMode0_KeyMc0Read( hHsm, &keyRead );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE(rc); }
    if( !keyRead.out.data[0] ) pKeyInfo->gp1KeyLadderAllow = true;

    /* gp2 Keyladder allowed*/
    BKNI_Memset( &keyRead, 0, sizeof(keyRead) );
    keyRead.in.sectionIndex = pKeyInfo->index;
    keyRead.in.field = Bsp_Otp_KeyField_eMc0Gp2KeyLadderDisallow;
    rc = BHSM_P_OtpMode0_KeyMc0Read( hHsm, &keyRead );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE(rc); }
    if( !keyRead.out.data[0] ) pKeyInfo->gp2KeyLadderAllow = true;

    /* sage Keyladder allowed*/
    BKNI_Memset( &keyRead, 0, sizeof(keyRead) );
    keyRead.in.sectionIndex = pKeyInfo->index;
    keyRead.in.field = Bsp_Otp_KeyField_eMc0SageKeyLadderDisallow ;
    rc = BHSM_P_OtpMode0_KeyMc0Read( hHsm, &keyRead );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE(rc); }
    if( !keyRead.out.data[0] ) pKeyInfo->sageKeyLadderAllow = true;

    /* deobfuscationEnabled */
    BKNI_Memset( &keyRead, 0, sizeof(keyRead) );
    keyRead.in.sectionIndex = pKeyInfo->index;
    keyRead.in.field = Bsp_Otp_KeyField_eMc0DeobfuscationEnable;
    rc = BHSM_P_OtpMode0_KeyMc0Read( hHsm, &keyRead );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE(rc); }
    if( keyRead.out.data[0] ) pKeyInfo->deobfuscationEnabled = true;

    /* customer Mode*/
    BKNI_Memset( &keyRead, 0, sizeof(keyRead) );
    keyRead.in.sectionIndex = pKeyInfo->index;
    keyRead.in.field = Bsp_Otp_KeyField_eMc0CustomerMode;
    rc = BHSM_P_OtpMode0_KeyMc0Read( hHsm, &keyRead );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE(rc); }
    pKeyInfo->customerMode = keyRead.out.data[0];

    BDBG_LEAVE( BHSM_OtpKey_GetInfo );
    return BERR_SUCCESS;
}



BERR_Code BHSM_OtpKey_Program( BHSM_Handle hHsm, const BHSM_OtpKeyProgram *pKeyConfig )
{
    BSTD_UNUSED( hHsm );
    BSTD_UNUSED( pKeyConfig );

    return 0;
}
