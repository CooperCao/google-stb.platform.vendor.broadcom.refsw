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

#include <stdio.h> /* printf */

#include "bsagelib_types.h"
#include "sage_srai.h"

#include "bp3_platform_ids.h"
#include "bp3_module_ids.h"
#include "bp3_module_host.h"
#include "bp3_platform_host.h"


BDBG_MODULE(bp3_module_host);

SRAI_ModuleHandle hBP3Module = NULL;
int SAGE_BP3Module_Init(SRAI_PlatformHandle platform)
{
    int                      rc      = -1;
    BERR_Code                sage_rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *pSageInOutContainer;

    pSageInOutContainer = SRAI_Container_Allocate();
    if (!pSageInOutContainer)
    {
        BDBG_ERR(("Cannot allocate container for process BP3 bin file command"));
        rc = 1;
        goto end;
    }

    sage_rc = SRAI_Module_Init(platform,
                               BP3_ModuleId_eBP3,
                               pSageInOutContainer,
                               &hBP3Module);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s: Cannot Init Module",BSTD_FUNCTION));
        rc = 1;
        goto end;
    }

    rc = 0;
end:

    return 0;
}

void SAGE_BP3Module_Uninit(void)
{
    if (hBP3Module)
    {
        SRAI_Module_Uninit(hBP3Module);
        hBP3Module = NULL;
    }
}

//  Generate Session Token
BERR_Code SAGE_BP3Module_GetSessionToken(uint8_t *pSessionToken, uint32_t tokenSize)
{
    BERR_Code                rc = BERR_UNKNOWN;
    BSAGElib_InOutContainer *pSageInOutContainer = NULL;

    pSageInOutContainer = SRAI_Container_Allocate();
    if (pSageInOutContainer == NULL)
    {
        BDBG_ERR(("%s: Unable to allocate container.",BSTD_FUNCTION));
        goto end;
    }
    if (tokenSize == 0)
    {
        BDBG_ERR(("%s: Invalid session token size.",BSTD_FUNCTION));
        goto end;
    }
    pSageInOutContainer->blocks[0].data.ptr  = pSessionToken;
    pSageInOutContainer->blocks[0].len       = tokenSize;
    BDBG_MSG(("%s: SessionToken Ptr 0x%x  Size %lu",BSTD_FUNCTION,pSessionToken,tokenSize));
    rc = SRAI_Module_ProcessCommand(hBP3Module,
                                    BP3_CommandId_eGenerateSessionToken,
                                    pSageInOutContainer);
    if (rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s: Failed to send generate session token command. %d",BSTD_FUNCTION,rc));
    }
    rc = pSageInOutContainer->basicOut[0];
    if (rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s: Failed to generate Session Token %d",BSTD_FUNCTION,rc));
    }
end:
    if (pSageInOutContainer)
    {
        SRAI_Container_Free(pSageInOutContainer);
        pSageInOutContainer = NULL;
    }
    return rc;
}


//  Provision BP3 part
//  Encrypted BP3_CCF file format:
//      RSA encrypted Session Key
//      RSA Encrypted Session IV
//      Size of encrypted BP3_CCF without padding
//      PKCS-7 padded BP3_CCF
//      HMAC-SHA256 over encrypted BP3_CCF

BERR_Code SAGE_BP3Module_Provision(
    uint8_t  *pEncryptedBp3CcfFile,     // input: points to encrypted BP3_CCF file
    uint32_t  encryptedBp3CcfFileSize,  // input: encrypted BP3_CCF file size
    uint8_t  *pBp3BinBuff,              // input/output: points to buffer to write maximum size bp3.bin file to
    uint32_t *pBp3BinBuffSize,          // input: max bp3.bin file buffer size / actual bp3.bin file size
    uint32_t  existingBp3BinSize,        // input: the size of existing bp3.bin file
    uint8_t  *pLogBuff,                 // input: points to buffer to write bp3 log to
    uint32_t *pLogBuffSize,             // input/output: bp3 log file max buffer size / actual log file size
    uint32_t *pCcfStatus,               // input/ output: points to status
    uint32_t  ccfStatusSize)            // input/output: size of ccf block status for max blocks
{
    BERR_Code                rc   = BERR_UNKNOWN;
    BSAGElib_InOutContainer *pSageInOutContainer = SRAI_Container_Allocate();
    uint8_t                  index;

    if (!pSageInOutContainer)
    {
        BDBG_ERR(("Cannot allocate container for BP3 provisioning."));
        rc = BERR_UNKNOWN;
        goto end;
    }

    pSageInOutContainer->blocks[0].data.ptr = pEncryptedBp3CcfFile;
    pSageInOutContainer->blocks[0].len      = encryptedBp3CcfFileSize;
    pSageInOutContainer->blocks[1].data.ptr = pBp3BinBuff;
    pSageInOutContainer->blocks[1].len      = *pBp3BinBuffSize;
    pSageInOutContainer->blocks[2].data.ptr = pLogBuff;
    pSageInOutContainer->blocks[2].len      = *pLogBuffSize;
    pSageInOutContainer->blocks[3].data.ptr = (uint8_t *)pCcfStatus;
    pSageInOutContainer->blocks[3].len      = ccfStatusSize;
    pSageInOutContainer->basicIn[0]         = existingBp3BinSize;
    rc = SRAI_Module_ProcessCommand(hBP3Module,
                                         BP3_CommandId_eProvisionBp3Ccf,
                                         pSageInOutContainer);
    if (rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s: Cannot Send Command\n",BSTD_FUNCTION));
        goto end;
    }
    rc               = pSageInOutContainer->basicOut[0];
    *pBp3BinBuffSize = pSageInOutContainer->basicOut[1];
    *pLogBuffSize    = pSageInOutContainer->basicOut[2];

    BDBG_LOG(("bp3.bin size=%d, log size=%d, rc=%d",*pBp3BinBuffSize, *pLogBuffSize, rc));
    // print status
    for (index=0; index<5; index++)
    {
        if (index == 0)
        {
            BDBG_LOG(("CCF block %d or header status %d",index,pCcfStatus[index]));
        }
        else
        {
            BDBG_LOG(("CCF block %d status %d",index,pCcfStatus[index]));
        }
    }
end:
    if (pSageInOutContainer)
    {
        SRAI_Container_Free(pSageInOutContainer);
    }
    return rc;
}
