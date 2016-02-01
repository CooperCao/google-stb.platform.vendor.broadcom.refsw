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
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/
#ifndef BBOX_VDC_PRIV_H__
#define BBOX_VDC_PRIV_H__

#include "bstd.h"
#include "berr_ids.h"    /* Error codes */
#include "bbox_vdc_priv.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
 * capability flags are or-ed during acquiring.
 */
typedef enum
{
	BBOX_Vdc_Resource_eMem0    =     (1<< 0),      /* able to access mem ctrl 0 */
	BBOX_Vdc_Resource_eMem1    =     (1<< 1),      /* able to access mem ctrl 1 */
	BBOX_Vdc_Resource_eMem2    =     (1<< 2),      /* able to access mem ctrl 2 */
	BBOX_Vdc_Resource_eAllSrc  =     (1<< 3),      /* able to use by all sources */
	BBOX_Vdc_Resource_eHd      =     (1<< 4),      /* able to handle HD size */
	BBOX_Vdc_Resource_eMadr0   =     (1<< 5),      /* able to handle transcode 0*/
	BBOX_Vdc_Resource_eMadr1   =     (1<< 6),      /* able to handle transcode 1*/
	BBOX_Vdc_Resource_eMadr2   =     (1<< 7),      /* able to handle transcode 2*/
	BBOX_Vdc_Resource_eMadr3   =     (1<< 8),      /* able to handle transcode 3*/
	BBOX_Vdc_Resource_eMadr4   =     (1<< 9),      /* able to handle transcode 4*/
	BBOX_Vdc_Resource_eMadr5   =     (1<<10),      /* able to handle transcode 5*/
	BBOX_Vdc_Resource_eHdmi0   =     (1<<11),      /* able to handle HDMI output 0 */
	BBOX_Vdc_Resource_eHdmi1   =     (1<<12),      /* able to handle HDMI output 1 */
	BBOX_Vdc_Resource_eInvalid =     (0xffff)      /* cause acquire to fail */

} BBOX_Vdc_Resource;

#ifdef __cplusplus
}
#endif

#endif/* BBOX_VDC_PRIV_H__ */
