/***************************************************************************
 *     Copyright (c) 2002-2009, Broadcom Corporation
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


#ifndef SI_VCT_H
#define SI_VCT_H

typedef enum
{
	ONE_PART,
	TWO_PART,
} VirtChanNumMode;

typedef enum
{
	SVCT_ST_ANALOG = 0x1,				/* analog TV service type. */
	SVCT_ST_ATSC_DIGITAL = 0x2, 		/* ATSC digital TV. */
	SVCT_ST_ATSC_AUDIO = 0x3, 			/* ATSC digital audio only. */
	SVCT_ST_ATSC_DATA = 0x4,			/* ATSC data broadcast. */
} VCT_SERVICE_TYPE;


#endif
