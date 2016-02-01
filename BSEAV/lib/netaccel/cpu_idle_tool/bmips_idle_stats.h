/***************************************************************************
 *     Copyright (c) 2006-2009, Broadcom Corporation
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
 *************************************************************/ 
#ifndef __BMIPS_IDLE_STATS_H__
#define __BMIPS_IDLE_STATS_H__

#ifdef __cplusplus
extern "C"
{
#endif
extern int bmips_idle_stats_init(void);
extern void bmips_idle_stats_exit(void);
extern void bmips_idle_stats_print(void);
#ifdef __cplusplus
}
#endif

#endif /* __BMIPS_IDLE_STATS_H__ */

