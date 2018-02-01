/***************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
 * Module Description:
 *
 ***************************************************************************/
#include "bstd.h"
#include "bmrc.h"

BERR_Code BMRC_Open
    ( BMRC_Handle                     *phMrc,        /* [out] MRC handle to be returned */
      BREG_Handle                      hRegister,    /* [in] Register access handle */
      BINT_Handle                      hInterrupt,   /* [in] Interrupt handle */
      const BMRC_Settings             *pDefSettings  /* [in] Default settings */
    )
{
    *phMrc = (void *)phMrc;
    BSTD_UNUSED(hRegister);
    BSTD_UNUSED(hInterrupt);
    BSTD_UNUSED(pDefSettings);
    return BERR_SUCCESS;
}

void BMRC_Close
    ( BMRC_Handle hMrc )
{
    BSTD_UNUSED(hMrc);
}

BERR_Code BMRC_Checker_Create
    ( BMRC_Handle hMrc,               /* [in] MRC Module Handle */
      BMRC_Checker_Handle *phChecker  /* [out] Checker handle to be returned */
    )
{
    BSTD_UNUSED(hMrc);
    BSTD_UNUSED(phChecker);
    return BERR_SUCCESS;
}

void BMRC_Checker_Destroy
    ( BMRC_Checker_Handle hChecker  /* [in] Checker handle to be destroyed */
    )
{
    BSTD_UNUSED(hChecker);
}

BERR_Code BMRC_Checker_SetRange
    ( BMRC_Checker_Handle hChecker,  /* [in] Checker handle */
      BSTD_DeviceOffset ulStart,              /* [in] Memory range start address */
      uint64_t ulSize                /* [in] Memory range size */
    )
{
    BSTD_UNUSED(hChecker);
    BSTD_UNUSED(ulStart);
    BSTD_UNUSED(ulSize);
    return BERR_SUCCESS;
}

BERR_Code BMRC_Checker_SetAccessCheck
    ( BMRC_Checker_Handle hChecker,  /* [in] Checker handle */
      BMRC_AccessType eAccessType    /* [in] Access type to check */
    )
{
    BSTD_UNUSED(hChecker);
    BSTD_UNUSED(eAccessType);
    return BERR_SUCCESS;
}

BERR_Code BMRC_Checker_SetBlock
    ( BMRC_Checker_Handle hChecker,  /* [in] Checker handle */
      BMRC_AccessType eBlockType     /* [in] Access type to block on violations*/
    )
{
    BSTD_UNUSED(hChecker);
    BSTD_UNUSED(eBlockType);
    return BERR_SUCCESS;
}

BERR_Code BMRC_Checker_SetExclusive
    ( BMRC_Checker_Handle hChecker,  /* [in] Checker handle */
      bool bExclusive                /* [in] Enable/disable exclusive mode */
    )
{
    BSTD_UNUSED(hChecker);
    BSTD_UNUSED(bExclusive);
    return BERR_SUCCESS;
}

BERR_Code BMRC_Checker_SetClient
    ( BMRC_Checker_Handle hChecker,  /* [in] Checker handle */
      BMRC_Client eClient,           /* [in] The client to configure */
      BMRC_AccessType eAccessType    /* [in] The client's access rights */
    )
{
    BSTD_UNUSED(hChecker);
    BSTD_UNUSED(eClient);
    BSTD_UNUSED(eAccessType);
    return BERR_SUCCESS;
}

BERR_Code BMRC_Checker_Enable
    ( BMRC_Checker_Handle hChecker  /* [in] Checker handle */
    )
{
    BSTD_UNUSED(hChecker);
    return BERR_SUCCESS;
}


BERR_Code BMRC_Checker_Disable
    ( BMRC_Checker_Handle hChecker  /* [in] Checker handle */
    )
{
    BSTD_UNUSED(hChecker);
    return BERR_SUCCESS;
}

BERR_Code BMRC_Checker_EnableCallback
    ( BMRC_Checker_Handle hChecker  /* [in] Checker handle */
    )
{
    BSTD_UNUSED(hChecker);
    return BERR_SUCCESS;
}

BERR_Code BMRC_Checker_DisableCallback
    ( BMRC_Checker_Handle hChecker  /* [in] Checker handle */
    )
{
    BSTD_UNUSED(hChecker);
    return BERR_SUCCESS;
}

BERR_Code BMRC_Checker_EnableCallback_isr
    ( BMRC_Checker_Handle hChecker  /* [in] Checker handle */
    )
{
    BSTD_UNUSED(hChecker);
    return BERR_SUCCESS;
}

BERR_Code BMRC_Checker_DisableCallback_isr
    ( BMRC_Checker_Handle hChecker  /* [in] Checker handle */
    )
{
    BSTD_UNUSED(hChecker);
    return BERR_SUCCESS;
}

BERR_Code BMRC_Checker_SetCallback
    ( BMRC_Checker_Handle hChecker,          /* [in] Checker handle */
      const BMRC_CallbackFunc_isr pfCbFunc,  /* [in] Pointer to the callback function */
      void *pvCbData1,                       /* [in] User defined callback data structure. */
      int iCbData2)                          /* [in] User defined callback value */
{
    BSTD_UNUSED(hChecker);
    BSTD_UNUSED(pfCbFunc);
    BSTD_UNUSED(pvCbData1);
    BSTD_UNUSED(iCbData2);
    return BERR_SUCCESS;
}

void BMRC_Standby( BMRC_Handle hMrc )
{
    BSTD_UNUSED(hMrc);
    return;
}

void BMRC_Resume( BMRC_Handle hMrc )
{
    BSTD_UNUSED(hMrc);
    return;
}

void BMRC_GetSettings ( BMRC_Handle hMrc, BMRC_Settings *pSettings )
{
    BSTD_UNUSED(hMrc);
    BSTD_UNUSED(pSettings);
    return;
}

void BMRC_GetMaxCheckers ( BMRC_Handle hMrc, uint32_t *pulMaxChecker )
{
    BSTD_UNUSED(hMrc);
    *pulMaxChecker = 8;
}

void BMRC_GetDefaultSettings ( BMRC_Settings *pDefSettings )
{
    BSTD_UNUSED(pDefSettings);
}

BERR_Code BMRC_Checker_GetClientInfo(BMRC_Handle hMrc, BMRC_Client eClient, BMRC_ClientInfo *pClientInfo)
{
    BSTD_UNUSED(hMrc);
    BSTD_UNUSED(eClient);
    BSTD_UNUSED(pClientInfo);
    return BERR_SUCCESS;
}

void BMRC_PrintBlockingArcs(BREG_Handle reg)
{
    BSTD_UNUSED(reg);
    return;
}
