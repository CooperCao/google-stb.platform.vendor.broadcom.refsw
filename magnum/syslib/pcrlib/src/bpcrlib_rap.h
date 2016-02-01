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
#ifndef BPCRLIB_RAP_H_
#define BPCRLIB_RAP_H_

#include "brap.h"

#ifndef BCHP_7411_VER
#include "bxpt_rave.h"
#endif


typedef struct {
	BRAP_ChannelHandle dec;
#ifdef BCHP_7411_VER
	BRAP_TRANS_ChannelHandle trans;
#else
	BXPT_RaveCx_Handle rave;
#endif
} BPCRlib_Rap_Decoder;

extern const BPCRlib_StcDecIface BPCRlib_Audio_Rap;

#endif /* BPCRLIB_XVD_H_ */



