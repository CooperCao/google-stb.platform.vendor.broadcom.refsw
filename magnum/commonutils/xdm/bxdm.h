/***************************************************************************
 *     Copyright (c) 2003-2010, Broadcom Corporation
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
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef BXDM_H_
#define BXDM_H_

#ifdef __cplusplus
extern "C" {
#endif

#if 0
}
#endif

typedef struct BXDM_QualifiedValue
{
        uint32_t uiValue;
        bool bValid;
} BXDM_QualifiedValue;

typedef struct BXDM_DisplayInterruptInfo
{
      BAVC_Polarity eInterruptPolarity;
      BXDM_QualifiedValue stInterruptCount;
      BXDM_QualifiedValue *astSTC;
      uint32_t uiSTCCount;
} BXDM_DisplayInterruptInfo;

typedef BERR_Code (*BXDM_DisplayInterruptHandler_isr)(
         void *pPrivateContext,
         BXDM_DisplayInterruptInfo *pstXvdDisplayInterruptInfo
         );

typedef BERR_Code (*BXDM_DisplayInterruptHandler_PictureProvider_isr)(
         void* pPrivateContext,
         const BXDM_DisplayInterruptInfo *pstDisplayInterruptInfo,
         BAVC_MVD_Picture **pDispMgrToVDC
         );

#ifdef __cplusplus
}
#endif

#endif /* BXDM_H_ */
