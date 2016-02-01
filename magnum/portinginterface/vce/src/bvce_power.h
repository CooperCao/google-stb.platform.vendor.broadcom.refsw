/***************************************************************************
 *     Copyright (c) 2003-2012, Broadcom Corporation
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

#ifndef BVCE_POWER_H_
#define BVCE_POWER_H_

#ifdef __cplusplus
extern "C" {
#endif

#define BVCE_Power_P_AcquireResource_isr( hVce, eType ) { BDBG_ASSERT( ( 0 != hVce->stPlatformConfig.stPower.astResource[eType].uiRefCount ) ); }
#define BVCE_Power_P_ReleaseResource_isr( handle, type )

void
BVCE_Power_P_AcquireResource(
      BVCE_Handle hVce,
      BVCE_Power_Type eType
      );

void
BVCE_Power_P_ReleaseResource(
      BVCE_Handle hVce,
      BVCE_Power_Type eType
      );

unsigned
BVCE_Power_P_QueryResource(
      BVCE_Handle hVce,
      BVCE_Power_Type eType
      );

void
BVCE_Power_P_ReleaseAllResources(
      BVCE_Handle hVce
      );

#ifdef __cplusplus
}
#endif

#endif /* BVCE_POWER_H_ */
/* End of File */
