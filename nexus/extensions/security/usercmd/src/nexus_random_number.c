/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

#include "nexus_security_module.h"
#include "nexus_security_datatypes.h"
#include "nexus_security.h"
#include "priv/nexus_security_priv.h"
#include "priv/nexus_core.h"

#include "bhsm_random_number.h"

BDBG_MODULE(nexus_random_number);



void NEXUS_RandomNumber_GetDefaultGenerateSettings( NEXUS_RandomNumberGenerateSettings *pSettings )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}


NEXUS_Error NEXUS_RandomNumber_Generate( const NEXUS_RandomNumberGenerateSettings *pSettings, NEXUS_RandomNumberOutput *pOutput )
{
    BERR_Code                   rc;
    BHSM_Handle                 hHsm;
    BHSM_UserRandomNumberIO_t   userCmdRandNumIO;

    NEXUS_Security_GetHsm_priv(&hHsm);
    if( hHsm == NULL)
    {
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }

     /* formulate the request structure */
    switch (pSettings->type)
    {
        case NEXUS_RandomNumberType_eRngRaw:
        {
            userCmdRandNumIO.RandomNumType = BHSM_RandomNumber_Type_eRNGRAW;
            break;
        }
        case NEXUS_RandomNumberType_eRngSha1:
        {
            userCmdRandNumIO.RandomNumType = BHSM_RandomNumber_Type_eRNGSHA;
            break;
        }
        default:
        {
            return BERR_TRACE( NEXUS_INVALID_PARAMETER );
        }
    }

    userCmdRandNumIO.keyDest = pSettings->destination;
    userCmdRandNumIO.unDataLen = pSettings->randomNumberSize;

    rc = BHSM_UserRandomNumber( hHsm, &userCmdRandNumIO );
    if (rc != 0)
    {
        BDBG_ERR(("Error calling BHSM_UserRandomNumber()"));
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }

    /* collect output */
    if( userCmdRandNumIO.unStatus )
    {
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }

    BKNI_Memcpy( (void *)pOutput->buffer,
                 (void *)userCmdRandNumIO.aucData,
                 userCmdRandNumIO.unDataLen );
    pOutput->size = userCmdRandNumIO.unDataLen;

    return NEXUS_SUCCESS;
}
