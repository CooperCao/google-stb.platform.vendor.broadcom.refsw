/***************************************************************************
 *     Copyright (c) 2006-2013, Broadcom Corporation
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
#ifndef __BARM_IDLE_STATS_H__
#define __BARM_IDLE_STATS_H__

#ifdef __cplusplus
extern "C"
{
#endif
extern int barm_idle_stats_init(void);
extern void barm_idle_stats_exit(void);
extern void barm_idle_stats_print(void);

extern void proc_read_info(void);


#ifdef __cplusplus
}
#endif

#endif /* __BARM_IDLE_STATS_H__ */

