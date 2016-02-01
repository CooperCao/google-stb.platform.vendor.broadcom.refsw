/***************************************************************************
 *     Copyright (c) 2003-2015, Broadcom Corporation
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
#ifndef BSTD_FILE_H__
#define BSTD_FILE_H__

#if BSTD_FILE_DISABLED
#define BSTD_FILE "unknown"
#define BSTD_LINE 0
#else
#define BSTD_FILE __FILE__
#define BSTD_LINE __LINE__
#endif

#endif /* #ifndef BSTD_FILE_H__ */
