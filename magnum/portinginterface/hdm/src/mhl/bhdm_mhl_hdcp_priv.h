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

#ifndef BHDM_MHL_HDCP_PRIV_H__
#define BHDM_MHL_HDCP_PRIV_H__

#include "bchp.h"       /* Chip Info */
#include "breg_mem.h"   /* Chip register access. */
#include "bkni.h"       /* Kernel Interface */
#include "bint.h"       /* Interrupt */
#include "btmr.h"   	/* Timer Handle  */

#include "berr_ids.h"   /* Error codes */
#include "bdbg.h"       /* Debug Support */

#include "bchp_hdmi.h"

#include "bhdm_hdcp.h"
#include "bhdm_mhl_priv.h"

#ifdef __cplusplus
extern "C" {
#endif

BERR_Code BHDM_MHL_P_Hdcp_GetVersion
	( BHDM_Handle         hHdm,
	  uint8_t            *pucHdcp2Version );

BERR_Code BHDM_MHL_P_Hdcp_GetRxBCaps
	( BHDM_Handle         hHdm,
	  uint8_t            *pucRxBcaps );

BERR_Code BHDM_MHL_P_Hdcp_GetRxStatus
	( BHDM_Handle         hHdm,
	  uint16_t           *puiRxStatus );

BERR_Code BHDM_MHL_P_Hdcp_GetRepeaterSha
	( BHDM_Handle         hHdm,
	  uint8_t             aucKsv[BHDM_P_MHL_HDCP_REPEATER_KSV_SIZE],
	  uint8_t             ucHOffset );

BERR_Code BHDM_MHL_P_Hdcp_GetRepeaterKsvFifo
	( BHDM_Handle         hHdm,
	  uint8_t            *pucRxKsvList,
	  uint16_t            uiRepeaterDeviceCount );

BERR_Code BHDM_MHL_P_Hdcp_GetRxPj
	( BHDM_Handle         hHdm,
	  uint8_t            *pucRxPj );

BERR_Code BHDM_MHL_P_Hdcp_GetRxRi
	( BHDM_Handle         hHdm,
	  uint16_t           *puiRxRi );

BERR_Code BHDM_MHL_P_Hdcp_SendTxAksv
	( BHDM_Handle         hHdm,
	  const uint8_t      *pucTxAksv );

BERR_Code BHDM_MHL_P_Hdcp_GetRxBksv
	( BHDM_Handle         hHdm,
	  unsigned char       aucRxBksv[BHDM_HDCP_KSV_LENGTH] );

BERR_Code BHDM_MHL_P_Hdcp_SendAnValue
	( BHDM_Handle         hHdm,
	  uint8_t            *pucAnValue );

BERR_Code BHDM_MHL_P_Hdcp_SendAinfoByte
	( BHDM_Handle         hHdm,
	  uint8_t            *pucAinfoByte );

#ifdef __cplusplus
}
#endif

#endif /* BHDM_MHL_HDCP_PRIV_H__ */
