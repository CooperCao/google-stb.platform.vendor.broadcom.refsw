/***************************************************************************
 *     Copyright (c) 2003-2011, Broadcom Corporation
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

#ifndef BVCE_TELEM_H_
#define BVCE_TELEM_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct BVCE_Telem_P_Context *BVCE_Telem_Handle;

typedef struct BVCE_Telem_Settings
{
   unsigned uiUnused;
} BVCE_Telem_Settings;

void
BVCE_Telem_GetDefaultCreateSettings(
         BVCE_Telem_Settings *pstTelemSettings
         );

BERR_Code
BVCE_Telem_Create(
         BVCE_Telem_Handle *phVCETelem,
         const BVCE_Telem_Settings *pstTelemSettings
         );

BERR_Code
BVCE_Telem_Destroy(
         BVCE_Telem_Handle hVCETelem
         );

BERR_Code
BVCE_Telem_Parse(
   BVCE_Telem_Handle hVCETelem,
   const uint8_t *pInputBuffer0, /* [in] */
   size_t uiInputLength0, /* [in] */
   const uint8_t *pInputBuffer1, /* [in] */
   size_t uiInputLength1, /* [in] */
   size_t *puiInputLengthRead, /* out */
   char *szOutputBuffer,   /* [out] pointer to buffer where log is copied to */
   size_t uiOutputLength,  /* [in] maximum number of bytes to copy to buffer */
   size_t *puiOutputLengthWritten  /* [out] number of bytes copied from debug log */
   );

#ifdef __cplusplus
}
#endif

#endif /* BVCE_TELEM_H_ */
/* End of File */

