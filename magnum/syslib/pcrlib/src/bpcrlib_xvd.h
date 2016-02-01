/***************************************************************************
 *     Copyright (c) 2005, Broadcom Corporation
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
#ifndef BPCRLIB_XVD_H_
#define BPCRLIB_XVD_H_

#include "bxvd.h"

#ifndef BCHP_7411_VER
#include "bxpt_rave.h"
#endif


extern const BPCRlib_StcDecIface BPCRlib_Video_Xvd;

typedef struct {
	BXVD_ChannelHandle dec;
#ifndef BCHP_7411_VER
	BXPT_RaveCx_Handle rave;
#endif
} BPCRlib_Xvd_Decoder;

#endif /* BPCRLIB_XVD_H_ */



