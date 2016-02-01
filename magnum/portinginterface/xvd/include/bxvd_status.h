/***************************************************************************
 *     Copyright (c) 2007, Broadcom Corporation
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

#ifndef BXVD_STATUS_H__
#define BXVD_STATUS_H__

#include "bxvd.h"

#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif

typedef struct BXVD_P_Status_Context *BXVD_StatusHandle;

/* Create the XVD Status Handle */
BERR_Code BXVD_Status_Open(
   BXVD_Handle hXvd,
   BXVD_StatusHandle *phXvdStatus
   );

/* Destroy the XVD Status Handle */
BERR_Code BXVD_Status_Close(
   BXVD_StatusHandle hXvdStatus
   );

/* NOP */
BERR_Code BXVD_Status_OpenChannel(
   BXVD_StatusHandle hXvdStatus,
   BXVD_ChannelHandle hXvdCh
   );

/* NOP */
BERR_Code BXVD_Status_CloseChannel(
   BXVD_StatusHandle hXvdStatus,
   BXVD_ChannelHandle hXvdCh
   );

/* NOP */
BERR_Code BXVD_Status_StartDecode(
   BXVD_StatusHandle hXvdStatus,
   BXVD_ChannelHandle hXvdCh
   );

/* Copy and Accumulate Channel Status */
BERR_Code BXVD_Status_StopDecode(
   BXVD_StatusHandle hXvdStatus,
   BXVD_ChannelHandle hXvdCh
   );

BERR_Code BXVD_Status_Print(
   BXVD_StatusHandle hXvdStatus
   );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BXVD_STATUS_H__ */
/* End of File */
