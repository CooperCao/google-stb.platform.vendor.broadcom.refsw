/***************************************************************************
 *     Copyright (c) 2006-2007, Broadcom Corporation
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

#ifndef __BNE_H__
#define __BNE_H__

typedef enum BNE {
	eBNE_Generic,
	eBNE_Playpump,
	eBNE_Playback,
	eBNE_Memdma,
	eBNE_Fec,
	eBNE_Cstoe,
	eBNE_Stoe,
	eBNE_Xmit,
} BNE;

typedef struct bne_t {
		BNE	type;
		spinlock_t 	lock;
		int  (*init)(struct bne_t *h, int ch, void *params);
		void (*done)(struct bne_t *h);
		void (*free)(struct bne_t *h);

		int  (*submit)(struct bne_t *h, struct sk_buff *skb, unsigned src_cookie);
		int  (*ioctl)(struct bne_t *h, unsigned int cmd, unsigned long arg);

		void (*get_params)(struct bne_t *h, void *params);
		void (*set_params)(struct bne_t *h, void *params);
		ssize_t (*read)(struct bne_t *h, char *buf, size_t count);
        unsigned int (*poll)(struct bne_t *h, struct file *file, poll_table * wait);

		struct bne_t *next;
		struct bne_t *prev;
} bne_t;

bne_t *bne_alloc(BNE t);
#endif /* __BNE_H__ */

