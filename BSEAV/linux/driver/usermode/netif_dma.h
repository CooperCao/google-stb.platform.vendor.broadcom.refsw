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
#ifndef __NETIF_DMA_H__
#define __NETIF_DMA_H__

#include "blst_slist.h"
#ifdef __KERNEL__
#define BNETIF_DMA_INVALID_HANDLE	NULL
#else
#define BNETIF_DMA_INVALID_HANDLE	0
#endif
/* type for the generic handles */
typedef unsigned bnetif_dma_handle_t;

struct bnetif_dma_user_cookie {
	bnetif_dma_handle_t cookie;
};

struct bnetif_dma_cookie {
	bnetif_dma_handle_t cookie;
};

struct bnetif_dma_user_filter {
	uint16_t src_port; /* source port in the netwoek byte order */
	uint16_t src_port_mask; /* source port mask in the netwoek byte order */
	uint16_t dst_port; /* destination port in the netwoek byte order */
	uint16_t dst_port_mask/* destination port mask in the netwoek byte order */;
	uint32_t src_addr[4]; /* source address */
	uint32_t src_addr_mask[4]; /* source address mask */
	uint32_t dst_addr[4]; /* destination address */
	uint32_t dst_addr_mask[4]; /* destination address mask */
	uint16_t protocol; /* UDP/RTP/HTTP */
};

struct bnetif_user_block {
	uint32_t offset;
	unsigned len;
	struct bnetif_dma_user_cookie user_cookie;
	struct bnetif_dma_cookie net_cookie;
};

struct bnetif_net_block {
	struct bnetif_dma_cookie net_cookie;
};

struct bnetif_dma_data {
	unsigned user_len;
	struct bnetif_user_block *user;
	unsigned net_len;
	struct bnetif_net_block *net;
	unsigned noverflows;
};

struct bnetif_dma_get_header {
	uint32_t offset;
	unsigned len;
	unsigned *user_bfr;
};

struct bnetif_dma_add_filter {
	struct bnetif_dma_user_filter desc;
	struct bnetif_dma_user_cookie cookie;
	bnetif_dma_handle_t filter;
};

struct bnetif_dma_del_filter {
	bnetif_dma_handle_t filter;
};


#define BNETIF_DMA_DATA _IOWR('B', 0x01, struct bnetif_dma_data)
#define BNETIF_DMA_ADD_FILTER _IOWR('B', 0x02, struct bnetif_dma_add_filter)
#define BNETIF_DMA_DEL_FILTER _IOW('B', 0x03, struct bnetif_dma_del_filter)
#define BNETIF_DMA_STOP _IO('B', 0x04)
#define BNETIF_DMA_GET_HEADER _IOWR('B', 0x05, struct bnetif_dma_get_header)

#ifdef __KERNEL__
#include <linux/netdevice.h>
int bnetif_ioctl(struct inode *inode, struct file * file, unsigned int cmd, unsigned long arg);
typedef struct bnetif_dma_cb {
    struct bnetif_filter * (*bnetif_dma_add_filter)(struct bnetif_dma_user_filter *desc, struct bnetif_dma_user_cookie *cookie);
    int (*bnetif_dma_delete_filter)(struct bnetif_filter *filter);
    int (*bnetif_dma_data)(unsigned *, struct bnetif_user_block *, unsigned , const struct bnetif_net_block *, unsigned *);
    int (*bnetif_ioctl)(struct inode *, struct file *, unsigned int, unsigned long);
    void (*bnetif_dma_stop)(void);
} bnetif_dma_cb_t;

struct bne_t;
struct bnetif_filter {
    BLST_S_ENTRY(bnetif_filter) link;
    atomic_t  enabled;
    struct bnetif_dma_user_filter desc;
    struct bnetif_dma_user_cookie cookie;
    int (*install)(struct bnetif_filter *f, struct bne_t *b);
    int (*uninstall)(struct bnetif_filter *f);
	void (*config)(struct bnetif_filter *f, struct bnetif_dma_user_filter *desc, struct bnetif_dma_user_cookie *cookie);

    struct bne_t *pb;
    struct bne_t *dest; /* destination element */
};


int init_bnetif_dma(void);
void cleanup_bnetif_dma(void);
void bnetif_dma_stop(void);
int bnetif_dma_delete_filter(struct bnetif_filter *filter);
struct bnetif_filter * bnetif_dma_add_filter(struct bnetif_dma_user_filter *desc, struct bnetif_dma_user_cookie *cookie);
int bnetif_dma_data(unsigned *user_len, struct bnetif_user_block *user, unsigned net_len, const struct bnetif_net_block *net, unsigned *noverflows);
#endif

#endif /* __NETIF_DMA_H__ */ 
