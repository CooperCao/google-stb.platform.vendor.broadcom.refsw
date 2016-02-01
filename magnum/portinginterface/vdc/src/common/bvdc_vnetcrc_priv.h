/***************************************************************************
 *     Copyright (c) 2004-2013, Broadcom Corporation
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
#ifndef BVDC_VNETCRC_PRIV_H__
#define BVDC_VNETCRC_PRIV_H__

#include "breg_mem.h"      /* Chip register access (memory mapped). */
#include "bchp_vnet_b.h"
#include "bvdc_common_priv.h"
#include "bvdc_subrul_priv.h"
#include "bvdc_window_priv.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
 * {private}
 *
 */

/*-------------------------------------------------------------------------
 * macro used by VnetCrc sub-module
 */
#ifndef BCHP_VNET_B_CRC_0_SRC
#define BCHP_VNET_B_CRC_0_SRC BCHP_VNET_B_CRC_SRC
#endif	
#define BVDC_P_VnetCrc_MuxAddr(hVnetCrc)   (BCHP_VNET_B_CRC_0_SRC + (hVnetCrc)->eId * sizeof(uint32_t))

#define BVDC_P_VnetCrc_SetVnet_isr(hVnetCrc) \
   BVDC_P_SubRul_SetVnet_isr(&((hVnetCrc)->SubRul), (hVnetCrc)->ulSrcMuxValue, (hVnetCrc)->eVnetPatchMode)

#define BVDC_P_VNETCRC_SRC_NULL      0xffffffff	
#define BVDC_P_VNET_CRC_PROBE_RATE   BCHP_VNET_B_CRC_CTRL_PROBE_RATE_ONE_PICTURE_PERIOD
/*-------------------------------------------------------------------------
 * VnetCrc main context
 */
typedef struct BVDC_P_VnetCrcContext
{
	BDBG_OBJECT(BVDC_VNETCRC)

	/* VnetCrc Id */
	BVDC_P_VnetCrcId                 eId;
	uint32_t                         ulRegOffset;

	/* static info from creating */
	BREG_Handle                      hRegister;

	/* Which window it connect to */
	BVDC_Window_Handle               hWindow;

	/* sub-struct to manage vnet and rul build opreations */
	BVDC_P_SubRulContext             SubRul;

	/* src of vnetCrc, 0xffffffff indicates not really used */
	uint32_t                         ulSrcMuxValue;
	BVDC_P_VnetPatch                 eVnetPatchMode;
	
	/* vnet crc data */
	uint32_t                         ulCrcLuma;
	uint32_t                         ulCrcChroma;

} BVDC_P_VnetCrcContext;


/***************************************************************************
 * private functions
***************************************************************************/
/***************************************************************************
 * {private}
 *
 * BVDC_P_VnetCrc_Create
 *
 * called by BVDC_Open only
 */
BERR_Code BVDC_P_VnetCrc_Create
	( BVDC_P_VnetCrc_Handle            *phVnetCrc,
	  BVDC_P_VnetCrcId                  eVnetCrcId,
	  BREG_Handle                       hRegister,
	  BVDC_P_Resource_Handle            hResource );

/***************************************************************************
 * {private}
 *
 * BVDC_P_VnetCrc_Destroy
 *
 * called by BVDC_Close only
 */
BERR_Code BVDC_P_VnetCrc_Destroy
	( BVDC_P_VnetCrc_Handle             hVnetCrc );

/***************************************************************************
 * {private}
 *
 * BVDC_P_VnetCrc_AcquireConnect_isr
 *
 * It is called by BVDC_Window_Validate after changing from diabling VnetCrc to
 * enabling VnetCrc.
 */
BERR_Code BVDC_P_VnetCrc_AcquireConnect_isr
	( BVDC_P_VnetCrc_Handle             hVnetCrc,
	  BVDC_Window_Handle                hWindow);

/***************************************************************************
 * {private}
 *
 * BVDC_P_VnetCrc_ReleaseConnect_isr
 *
 * It is called after window/VnetCrc decided that VnetCrc is no-longer used
 * by HW (i.e. it is really shut down and teared off from vnet).
 */
BERR_Code BVDC_P_VnetCrc_ReleaseConnect_isr
	( BVDC_P_VnetCrc_Handle            *phVnetCrc );

/***************************************************************************
 * {private}
 *
 * BVDC_P_VnetCrc_BuildRul_isr
 *
 * called by BVDC_Window_BuildRul_isr at every src vsync. It builds RUL for
 * vnet crc HW module and sample the crc data.
 *
 * It will reset *phVnetCrc to NULL if the HW module is no longer used by
 * any window.
 */
void BVDC_P_VnetCrc_BuildRul_isr
	( BVDC_P_VnetCrc_Handle            *phVnetCrc,
	  BVDC_P_ListInfo                  *pList,
	  BVDC_P_State                      eVnetState,
	  BVDC_P_PicComRulInfo             *pPicComRulInfo,
	  bool                              bEnable);

/***************************************************************************
 * {private}
 *
 * BVDC_P_VnetCrc_DecideVnetMode_isr
 *
 * called by BVDC_P_VnetCrc_AcquireConnect_isr or
 * BVDC_P_Window_DecideVnetMode_isr
 * return true if vnet reconfigure is needed
 */
bool BVDC_P_VnetCrc_DecideVnetMode_isr
	( BVDC_Window_Handle                   hWindow,
	  BVDC_P_VnetCrc_Handle                hVnetCrc,
	  BVDC_P_VnetMode                     *pVnetMode);

/***************************************************************************
 * {private}
 *
 */
void BVDC_P_VnetCrc_UnsetVnet_isr
	( BVDC_P_VnetCrc_Handle                hVnetCrc );
	
#ifdef __cplusplus
}
#endif

#endif /* #ifndef BVDC_VNETCRC_PRIV_H__ */
/* End of file. */
