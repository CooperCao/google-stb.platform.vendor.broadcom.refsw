/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
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

#ifndef BVCE_OUTPUT_H_
#define BVCE_OUTPUT_H_

#ifdef __cplusplus
extern "C" {
#endif

BERR_Code
BVCE_Output_Reset(
         BVCE_Output_Handle hVceOutput
         );

void
BVCE_Output_Flush(
   BVCE_Output_Handle hVceOutput
   );

#ifdef __cplusplus
}
#endif

#endif /* BVCE_OUTPUT_H_ */
/* End of File */
