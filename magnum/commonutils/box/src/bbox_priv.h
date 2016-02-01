/***************************************************************************
 *     Copyright (c) 2003-2014, Broadcom Corporation
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
#ifndef BBOX_PRIV_H__
#define BBOX_PRIV_H__


#include "bbox.h"
#include "bbox_vdc.h"

/***************************************************************************
 * BBOX_P_Handle
 ***************************************************************************/
typedef struct BBOX_P_Context
{
	BDBG_OBJECT(BBOX_BOX)

	BBOX_Config              stBoxConfig;
	bool                     bRtsLoaded;
} BBOX_P_Context;

BERR_Code BBOX_P_ValidateId
	(uint32_t                ulId);

BERR_Code BBOX_P_Vdc_SetBoxMode
	( uint32_t               ulBoxId,
	  BBOX_Vdc_Capabilities *pBoxVdc );

BERR_Code BBOX_P_Vce_SetBoxMode
   ( uint32_t               ulBoxId,
     BBOX_Vce_Capabilities *pBoxVce );

BERR_Code BBOX_P_Audio_SetBoxMode
   ( uint32_t               ulBoxId,
     BBOX_Audio_Capabilities *pBoxAudio );

BERR_Code BBOX_P_LoadRts
	( const BREG_Handle      hReg,
	  const uint32_t         ulBoxId );

BERR_Code BBOX_P_GetMemConfig
	( uint32_t                       ulBoxId,
	  BBOX_MemConfig                *pBoxMemConfig );

/* Add module specific box mode functions here */

#endif /* BBOX_PRIV_H__ */
/* end of file */
