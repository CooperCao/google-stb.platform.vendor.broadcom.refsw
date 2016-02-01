/***************************************************************************
 *     Copyright (c) 2005-2011, Broadcom Corporation
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

#ifndef BRFM_SCRIPTS_H__
#define BRFM_SCRIPTS_H__

#ifdef __cplusplus
extern "C" {
#endif

const BRFM_P_ModulationInfo *BRFM_P_GetModInfoPtr(
    BRFM_ModulationType modType /* [in] Requested modulation type */
    );

#ifdef __cplusplus
}
#endif
 
#endif
