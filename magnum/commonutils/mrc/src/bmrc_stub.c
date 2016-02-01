/***************************************************************************
 *     Copyright (c) 2014, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
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

BERR_Code BMRC_Close
    ( BMRC_Handle hMrc )
{
    BSTD_UNUSED(hMrc);
    return BERR_SUCCESS;
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

BERR_Code BMRC_Checker_Destroy
    ( BMRC_Checker_Handle hChecker  /* [in] Checker handle to be destroyed */
    )
{
    BSTD_UNUSED(hChecker);
    return BERR_SUCCESS;
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

BERR_Code BMRC_GetMaxCheckers ( BMRC_Handle hMrc, uint32_t *pulMaxChecker )
{
    BSTD_UNUSED(hMrc);
    *pulMaxChecker = 8;
    return BERR_SUCCESS;
}

BERR_Code BMRC_GetDefaultSettings ( BMRC_Settings *pDefSettings )
{
    BSTD_UNUSED(pDefSettings);
    return BERR_SUCCESS;
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
