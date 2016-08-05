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

#include "bhsm.h"
#include "bsp_s_otp.h"
#include "bhsm_otp_id.h"
#include "bhsm_private.h"
#include "bchp_jtag_otp.h"


BDBG_MODULE(BHSM);

BDBG_OBJECT_ID_DECLARE( BHSM_P_Handle );

static BERR_Code BHSM_P_WaitJtagCtrl( BHSM_Handle hHsm, uint32_t waitbit );
#if BHSM_ZEUS_VERSION < BHSM_ZEUS_VERSION_CALC(4,0)
static BERR_Code BCSD_P_WriteJtagCtrl3( BHSM_Handle hHsm, uint32_t ctrl_0, uint32_t ctrl_3, uint32_t waitbit );
#endif
static BERR_Code BHSM_ReadOtpData( BHSM_Handle hHsm, uint32_t row, uint32_t *pData );


/*
 * Return the masked off values for the chipset extension.
 *
 */
BERR_Code BHSM_DEBUG_GetChipsetOtpType( BHSM_Handle hHsm, char *pLetter1, char *pLetter2 )
{
    uint32_t extension = 0;
    static char  cache[2] = {0};
    static bool cached = false;

    BDBG_ENTER( BHSM_DEBUG_GetChipsetOtpType );

    if( pLetter1 == NULL || pLetter2 == NULL )
    {
        return BERR_TRACE( BERR_INVALID_PARAMETER );
    }

    if( cached == false )
    {
        if( BERR_SUCCESS != BHSM_ReadOtpData( hHsm, 13, &extension ) )
        {
            return BERR_TRACE( BHSM_STATUS_FAILED );
        }

        cached = true;
        cache[0] = (char)(((extension>>20)&0x3f) | 0x40 );
        cache[1] = (char)(((extension>>14)&0x3f) | 0x40 );

        if( ((cache[0] < 'A') || (cache[0] > 'Z')) && ((cache[0] < 'a') || (cache[0] > 'z')) ) { cache[0] = '-'; }
        if( ((cache[1] < 'A') || (cache[1] > 'Z')) && ((cache[1] < 'a') || (cache[1] > 'z')) ) { cache[1] = '-'; }
    }

    *pLetter1 = cache[0];
    *pLetter2 = cache[1];

    BDBG_LEAVE( BHSM_DEBUG_GetChipsetOtpType );
    return BERR_SUCCESS;
}

/*
   '****************************************************************************************
   '    read a word from production OTP
   '****************************************************************************************
 */
static BERR_Code BHSM_ReadOtpData( BHSM_Handle hHsm, uint32_t row, uint32_t *pData )
{
    uint32_t regVal;

    regVal = BREG_Read32( hHsm->regHandle, BCHP_JTAG_OTP_GENERAL_CTRL_1 );
    BREG_Write32( hHsm->regHandle, BCHP_JTAG_OTP_GENERAL_CTRL_1, (regVal | 0x00000001) );

  #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0)
    BREG_Write32( hHsm->regHandle, BCHP_JTAG_OTP_GENERAL_CTRL_0, 0 );
    BREG_Write32( hHsm->regHandle, BCHP_JTAG_OTP_GENERAL_CTRL_3, row*32 );

    BREG_Write32( hHsm->regHandle, BCHP_JTAG_OTP_GENERAL_CTRL_0, 0x00000000 );
    BREG_Write32( hHsm->regHandle, BCHP_JTAG_OTP_GENERAL_CTRL_0, 0x00000001 );

    if( BERR_SUCCESS != BHSM_P_WaitJtagCtrl( hHsm, 0x2 ) ) { return BERR_TRACE( BHSM_STATUS_FAILED ); }
  #else
    if( BERR_SUCCESS != BCSD_P_WriteJtagCtrl3( hHsm, 0x00200003, 0x0000000f, 0x1 ) ) { return BERR_TRACE( BHSM_STATUS_FAILED ); }
    if( BERR_SUCCESS != BCSD_P_WriteJtagCtrl3( hHsm, 0x00200003, 0x00000004, 0x1 ) ) { return BERR_TRACE( BHSM_STATUS_FAILED ); }
    if( BERR_SUCCESS != BCSD_P_WriteJtagCtrl3( hHsm, 0x00200003, 0x00000008, 0x1 ) ) { return BERR_TRACE( BHSM_STATUS_FAILED ); }
    if( BERR_SUCCESS != BCSD_P_WriteJtagCtrl3( hHsm, 0x00200003, 0x0000000d, 0x1 ) ) { return BERR_TRACE( BHSM_STATUS_FAILED ); }

    BREG_Write32( hHsm->regHandle, BCHP_JTAG_OTP_GENERAL_CTRL_3, row*32     );
    BREG_Write32( hHsm->regHandle, BCHP_JTAG_OTP_GENERAL_CTRL_0, 0x00000000 );
    BREG_Write32( hHsm->regHandle, BCHP_JTAG_OTP_GENERAL_CTRL_0, 0x00A00001 );

    if( BERR_SUCCESS != BHSM_P_WaitJtagCtrl( hHsm, 0x1 ) ) { return BERR_TRACE( BHSM_STATUS_FAILED ); }
  #endif

    regVal = BREG_Read32( hHsm->regHandle, BCHP_JTAG_OTP_GENERAL_STATUS_0 );
    *pData = regVal;
    BREG_Write32( hHsm->regHandle, BCHP_JTAG_OTP_GENERAL_CTRL_0, 0 );

    return BERR_SUCCESS;
}


/* Support functions for BHSM_ReadOtpData */
/* Wait for a bit in general status 1, or timeout */
static BERR_Code BHSM_P_WaitJtagCtrl( BHSM_Handle hHsm, uint32_t waitbit )
{
    uint32_t regval = 0;
    uint32_t count  = 0;


    while( (regval & waitbit) == 0 )
    {
        if( ++count > 10 )
        {
            return BERR_TRACE( BHSM_STATUS_FAILED );
        }
        BKNI_Sleep(10);
        regval = BREG_Read32( hHsm->regHandle, BCHP_JTAG_OTP_GENERAL_STATUS_1 );
    }

    return BERR_SUCCESS;
}

#if BHSM_ZEUS_VERSION < BHSM_ZEUS_VERSION_CALC(4,0)
/* Write to JTAG CTRL 3 */
static BERR_Code BCSD_P_WriteJtagCtrl3( BHSM_Handle hHsm, uint32_t ctrl_0, uint32_t ctrl_3, uint32_t waitbit )
{
    BREG_Write32( hHsm->regHandle, BCHP_JTAG_OTP_GENERAL_CTRL_3, (unsigned) ctrl_3 );
    BREG_Write32( hHsm->regHandle, BCHP_JTAG_OTP_GENERAL_CTRL_0, (unsigned) ctrl_0 );

    if( BERR_SUCCESS != BHSM_P_WaitJtagCtrl( hHsm, waitbit ) ) { return BERR_TRACE( BHSM_STATUS_FAILED ); }

    BREG_Write32( hHsm->regHandle, BCHP_JTAG_OTP_GENERAL_CTRL_0, 0x00000000 );

    return BERR_SUCCESS;
}
#endif
