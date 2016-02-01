/***************************************************************************
*     Copyright (c) 2006-2010, Broadcom Corporation
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
*
* Module Description:
*	This file is part of image interface implementation for APE PI.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#include "bstd.h"
#include "bape_img.h"

#if BSTD_CPU_ENDIAN==BSTD_ENDIAN_BIG
#include "bape_img_mpeg_1_decoder_be.h"
#elif BSTD_CPU_ENDIAN==BSTD_ENDIAN_LITTLE
#include "bape_img_mpeg_1_decoder_le.h"
#else
#error "Not supported"
#endif

