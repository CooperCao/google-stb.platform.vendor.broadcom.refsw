/******************************************************************************
 *    (c)2007-2015 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 *
 *****************************************************************************/

#include "nexus_hmac_sha_cmd.h"
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

#include "bhsm_user_cmds.h"

BDBG_MODULE(nexus_hmac_sha_cmd);

#if (BCHP_CHIP == 3548) || (BCHP_CHIP == 3556) || (BCHP_CHIP == 3563) || (BCHP_CHIP == 7325) || (BCHP_CHIP == 7335) || \
    (BCHP_CHIP == 7336) || (BCHP_CHIP == 7400) || (BCHP_CHIP == 7401) || (BCHP_CHIP == 7403) || (BCHP_CHIP == 7405) || \
    (BCHP_CHIP == 35130) 

/* No HMAC/SHA User Command support for these platforms yet */

void NEXUS_HMACSHA_GetDefaultOpSettings(
    NEXUS_HMACSHA_OpSettings *pOpSettings /* [out] */
    )

{
	BSTD_UNUSED(pOpSettings);
}


NEXUS_Error NEXUS_HMACSHA_PerformOp(
    const NEXUS_HMACSHA_OpSettings *pOpSettings,    /* structure holding input parameters */
    NEXUS_HMACSHA_DigestOutput     *pOutput         /* [out] structure holding digest buffer and size */
    )
{

	BSTD_UNUSED(pOpSettings);
	BSTD_UNUSED(pOutput);
	return NEXUS_NOT_SUPPORTED;

}


#else


void NEXUS_HMACSHA_GetDefaultOpSettings(
    NEXUS_HMACSHA_OpSettings *pOpSettings /* [out] */
    )

{
    BKNI_Memset(pOpSettings, 0, sizeof(*pOpSettings));
	pOpSettings->dataSrc   = NEXUS_HMACSHA_DataSource_eDRAM;
	pOpSettings->keySource = NEXUS_HMACSHA_KeySource_eKeyVal;

}


NEXUS_Error NEXUS_HMACSHA_PerformOp(
    const NEXUS_HMACSHA_OpSettings *pOpSettings,    /* structure holding input parameters */
    NEXUS_HMACSHA_DigestOutput     *pOutput         /* [out] structure holding digest buffer and size */
    )
{
    BHSM_Handle             hHsm;
    BHSM_UserHmacShaIO_t    userHmacShaIO;
    void                   *pDataBuf = NULL;
    uint32_t                pPhysAddr;
    NEXUS_Error             rc = NEXUS_SUCCESS;
    uint32_t datasize;

    NEXUS_Security_GetHsm_priv (&hHsm);
    if ( !hHsm )
        return NEXUS_NOT_INITIALIZED;

	BKNI_Memset(&userHmacShaIO, 0, sizeof(BHSM_UserHmacShaIO_t));

	if(pOpSettings->dataSize==0)
        pDataBuf = BMEM_AllocAligned(g_pCoreHandles->heap[0].mem, 64, 2, 0);
	else if((pOpSettings->dataSize%4))
        pDataBuf = BMEM_AllocAligned(g_pCoreHandles->heap[0].mem, ((pOpSettings->dataSize)/4)*4 + 4, 2, 0);
	else
        pDataBuf = BMEM_AllocAligned(g_pCoreHandles->heap[0].mem, pOpSettings->dataSize,2, 0);
    
    if (pDataBuf == NULL)
	{
	
       return NEXUS_NOT_SUPPORTED;
	}

	/*
	 * Assemble the command block
	 */
	userHmacShaIO.oprMode   = pOpSettings->opMode;
	userHmacShaIO.unIncludeKey = pOpSettings->includeKey;  /* new */
	userHmacShaIO.shaType   = pOpSettings->shaType;
	userHmacShaIO.keySource = pOpSettings->keySource;

	userHmacShaIO.unKeyLength = pOpSettings->keyLength;
	
	if (pOpSettings->keySource == NEXUS_HMACSHA_KeySource_eKeyLadder)
	{
		userHmacShaIO.VirtualKeyLadderID = pOpSettings->VKL;
		userHmacShaIO.keyLayer           = pOpSettings->keyLayer;
	}
	else
	{
		BKNI_Memcpy(&(userHmacShaIO.keyData), &(pOpSettings->keyData), NEXUS_HMACSHA_KEY_LEN);
	}

	if (pOpSettings->context == NEXUS_HMACSHA_BSPContext_eContext0)
	{
		userHmacShaIO.contextSwitch = BPI_HmacSha1_Context_eHmacSha1Ctx0;
	}
	else
	{
		userHmacShaIO.contextSwitch = BPI_HmacSha1_Context_eHmacSha1Ctx1;
	}
	userHmacShaIO.dataSource    = pOpSettings->dataSrc;
	if (pOpSettings->dataSrc == NEXUS_HMACSHA_DataSource_eCmdBuf)
	{
		/* This mode is currently not supported */
		rc = NEXUS_NOT_SUPPORTED;
		goto error;
	}
	else /* Data is supplied in a buffer pointed to by dataAddress */
	{
		if((pOpSettings->dataSize%4))
			{
				datasize = ((pOpSettings->dataSize)/4)*4 + 4;
				BKNI_Memcpy(pDataBuf, pOpSettings->dataAddress, datasize);
			}
		else
		BKNI_Memcpy(pDataBuf, pOpSettings->dataAddress, pOpSettings->dataSize);
		if (BMEM_ConvertAddressToOffset(g_pCoreHandles->heap[0].mem, pDataBuf, &pPhysAddr) != BERR_SUCCESS)
		{
			
		   rc = NEXUS_NOT_SUPPORTED;
		   goto error;
		}
		userHmacShaIO.pucInputData   = (unsigned char *)pPhysAddr;
		userHmacShaIO.unInputDataLen = pOpSettings->dataSize;
	}
	userHmacShaIO.contMode = pOpSettings->dataMode;

	/* Submit the command now */
	rc = BHSM_UserHmacSha(hHsm, &userHmacShaIO);
    if (rc != 0)
    {
   	
        BDBG_ERR(("Error calling BHSM_UserHmacSha()\n"));
		rc = NEXUS_INVALID_PARAMETER;
		goto error;
    }

	/* If there are one or more data buffer left to submit, just fall through */
	if (pOpSettings->dataMode == NEXUS_HMACSHA_DataMode_eMore)
	{
		goto error;
	}
	else /* else collect the digest value */
	{
		if (userHmacShaIO.unStatus)
		{
		
			rc = NEXUS_INVALID_PARAMETER;
			goto error;
		}
		BKNI_Memcpy((void *)pOutput->digestData,
					(void *)userHmacShaIO.aucOutputDigest,
					userHmacShaIO.digestSize);
		pOutput->digestSize = userHmacShaIO.digestSize;

	}

error:

    if (pDataBuf != NULL)
        BMEM_Free(g_pCoreHandles->heap[0].mem, pDataBuf);
    return rc;


}

#endif
