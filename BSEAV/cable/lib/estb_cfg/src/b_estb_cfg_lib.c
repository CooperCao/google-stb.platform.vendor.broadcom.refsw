/******************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 ******************************************************************************/


#if defined(_CFE_)
#include "lib_types.h"
#include "lib_string.h"
#include "lib_queue.h"
#include "lib_malloc.h"
#include "lib_printf.h"
#include "lib_string.h"
#include "bsp_config.h"
#include "addrspace.h"
#include "cfe_cfg_data.h"
#elif  defined (NOOS)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bsp_config.h"
#include "cfe_cfg_data.h"
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#endif
#include "b_estb_cfg_lib.h"
#include "crc_host.h"

/*
 * HOST_ONLY is for PC host
 * _CFE_ is for CFE
 * NOOS for NO-OS diag
 */
#if defined(HOST_ONLY) || defined(_CFE_) || defined(NOOS)
/* debug macros */
#define BDBG_NOP() (void ) 0
#if defined (HOST_ONLY)
#define BDBG_MODULE(module)
#else
#define BDBG_MODULE(module) extern int bdbg_unused
#endif
/*
#define BDBG_MSG(format) BDBG_NOP()
#define BDBG_WRN(format) BDBG_NOP()
*/
#define BDBG_MSG(format) BDBG_NOP()
#define BDBG_WRN(format...) printf format
#define BDBG_ERR(format...)  printf format

#define BDBG_ENTER(function)
#define BDBG_LEAVE(function)
#else
#include "bstd.h" /* for BDBG... */
#include "b_os_lib.h"
#endif

#ifdef _CFE_

    #if CFG_RUNFROMKSEG0
    #define MAP_ADDR(addr) KERNADDR(addr);
    #else
    #define MAP_ADDR(addr) UNCADDR(addr);
    #endif
#endif

#ifdef NOOS
#define xprintf printf
#endif

#if defined(HOST_ONLY) || defined(NOOS)
#define BDBG_DSP BDBG_WRN
#else
#define BDBG_DSP BDBG_MSG
#endif

#if defined(NOOS) || defined (_CFE_)
#define ESTB_CFG_SEM_DEF
#define ESTB_CFG_SEM_INIT(ctl)
#define ESTB_CFG_SEM_WAIT(ctl)
#define ESTB_CFG_SEM_POST(ctl)

#else

#define ESTB_CFG_SEM_DEF sem_t sem
#define ESTB_CFG_SEM_INIT(ctl) sem_init(&ctl->sem, 0, 1)
#define ESTB_CFG_SEM_WAIT(ctl) sem_wait(&ctl->sem)
#define ESTB_CFG_SEM_POST(ctl) sem_post(&ctl->sem)

#endif

BDBG_MODULE(b_estb_cfg_lib);

static int pad_array[4] = {0x0,0x3, 0x2, 0x1};
#define PADDING(a) pad_array[a & 0x03]

/* macros */

#define NODE_STATE_INIT  0   /* the node has just been initialized */
#define NODE_STATE_DEFAULT  1 /* the node is set to default value */
#define NODE_STATE_LOADED  2 /* value loaded successfully from file or commandline */

#define IDX_INVALID (short)0xffff
#define NAME_LEN 128

#define SET_DEFAULT_DATA_VALUE(file_id, idx, src) do \
        {  \
                struct b_estb_cfg_ctl * ctl = &ctl_block[file_id]; \
                struct b_estb_cfg_node * node = &ctl->node_block[idx];\
                node->state = NODE_STATE_DEFAULT; \
                node->v.data.off_w = (int) &src; \
                node->v.data.len = (int ) sizeof(src); \
                node->v.data.pad = PADDING(node->v.data.len); \
        } while (0)

#define SET_DEFAULT_NODE_VALUE(dst, src) do \
        { \
		memmove((char *)dst, (char *)src, sizeof(src));\
	} while (0)

#define SET_DEFAULT_CTL_VALUE(file_id, src) do \
        { \
		memmove((char *)&ctl_block[file_id], (char *)&src, sizeof(src));\
	} while (0)

/* structures */

/* file header, per config file */
struct b_estb_cfg_hdr {
	int file_size; /* big endian */
	int hdr_size;
	char magic[16];
	char ver_str[16];
	short ver_major;
	short ver_minor;
	unsigned int checksum; /* data checksum */
};

/* data header, per variable */
struct b_estb_cfg_data_hdr {
	int label; /* label embedded in data block */
	int off; /* data offset in bytes */
	int len; /* data len in bytes */
	int pad; /* pad to 4 byte alignment */
};


/*
 * node block, for savin variable properties and navigating the variables. the actual data is
 * save in the data block. The node block is not saved.
 * - data node : represents a unique variable in the config.
 * - dir node: (not used for now). Manage the dirctory hierarchy, no data correspondig to it.
 */

struct b_estb_cfg_node {
	short idx;
	unsigned char flag;
	unsigned char type;
	unsigned int label;

	/* following 3 fields are not used right now */
	short parent_idx; /* parent dir index, not used now */
	short next_idx; /* next sibling idx, not used now */
	short prev_idx; /* previous sibling idx, not used now */
	short state;
	int field_len; /* for fixed length variable */

	char name[NAME_LEN]; /* full path name for now */

	union {
		/* for data node,  */
		struct {
			int off_w; /* data offset in 4 byte words
                        * if data is not initialized, off_w is the pointer
                        * to default value.
                        * When the data has been initialized (read from file),
                        * the off_w is the offset in the data array.
                        */
			int len; /* data len in bytes */
			int pad; /* pad to 4 byte alignment */
		} data;
		/* for dir node, not used right now */
		struct {
			int first_child_idx; /* first child directory index */
			int first_data_idx; /* first data node index */
			int unused;
		} dir;
	} v;
};

/* control block, per config file, each config file is mounted to a top dir */
struct b_estb_cfg_ctl {
	int id; /* file id */
	char file_name[NAME_LEN]; /* config file name */
	int reload;
	struct b_estb_cfg_hdr hdr_block;
	struct b_estb_cfg_node * node_block;
	int num_nodes;
	unsigned char * data_block;
	int data_len;
	int max_data_len; /* malloc extra space to avoid frequent alloc */
	int initialized;
    sem_t sem;
    /*    ESTB_CFG_SEM_DEF; */
};

/* read-only info, per config file */
struct b_estb_cfg_info {
	int id; /* file id */
	char top_dir[NAME_LEN]; /* top directory name, mounting point for a config file */
};

/* function prototypes */
static int b_estb_cfg_init_hdr(int idx) ;
static int b_estb_cfg_write_by_ctl (struct b_estb_cfg_ctl * ctl);
static int b_estb_cfg_read_by_ctl (struct b_estb_cfg_ctl * ctl);
static int b_estb_cfg_assemble_data(struct b_estb_cfg_ctl * ctl);
static int b_estb_cfg_get_property_by_label(unsigned int label, struct b_estb_cfg_property * prop);
static unsigned int my_ntohl(unsigned int data);
static unsigned int my_htonl (unsigned int val);


/* static variables */
#define DEFAULT_B_ESTB_CFG_HDR {0, 0, B_ESTB_CFG_VENDOR_MAGIC, B_ESTB_CFG_VENDOR_VER_STR, B_ESTB_CFG_VENDOR_VER_MAJOR, B_ESTB_CFG_VENDOR_VER_MINOR, 0}


/* !!! the following data should be generated by build_b_estb_cfg.pl script automatically */

#if 1
#include "b_estb_cfg_values.c"
#else

/* samples for test only */
#define B_ESTB_CFG_NUM_CFG_FILES  (B_ESTB_CFG_LABEL_DYN + 1)  /* number of files */
#define NUM_NODES_B_ESTB_CFG_LABEL_PERM  5


struct b_estb_cfg_node default_node_block_B_ESTB_CFG_LABEL_PERM [NUM_NODES_B_ESTB_CFG_LABEL_PERM] = {
    /* idx, flag, type, label, parent, next, prev, name, data ptr/sub dir, data len/data node */
    {0, 0, NODE_TYPE_DIR, B_ESTB_CFG_LABEL_RSVD0, 0, IDX_INVALID, IDX_INVALID, 0, 0, "/perm", {{2, IDX_INVALID,0}}},
    {1, 0, NODE_TYPE_DIR, B_ESTB_CFG_LABEL_RSVD0, 1, IDX_INVALID, IDX_INVALID, 0, 0, "/perm/estb", {{IDX_INVALID, 3,0}}},
    {2, 0, NODE_TYPE_UINT32, B_ESTB_CFG_LABEL_PERM_ESTB_VENDOR_ID, 2, 4, IDX_INVALID, NODE_STATE_INIT, 4, "/perm/estb/vendor_id", {{0, 4,0}}},
    {3, 0, NODE_TYPE_BIN, B_ESTB_CFG_LABEL_PERM_ESTB_MAC_ADDR, 2, IDX_INVALID, 4, NODE_STATE_DEFAULT, 6, "/perm/estb/mac_addr", {{4, 6, 2}}},
};
struct b_estb_cfg_node node_block_B_ESTB_CFG_LABEL_PERM [NUM_NODES_B_ESTB_CFG_LABEL_PERM];


struct b_estb_cfg_ctl default_ctl_block_B_ESTB_CFG_LABEL_PERM = {
	B_ESTB_CFG_LABEL_PERM,
	B_ESTB_CFG_DEFAULT_PERM_CFG,
	0,
	DEFAULT_B_ESTB_CFG_HDR ,
	node_block_B_ESTB_CFG_LABEL_PERM,
	NUM_NODES_B_ESTB_CFG_LABEL_PERM,
	NULL,
	0,
	0,
	0,
};

unsigned int default_B_ESTB_CFG_LABEL_PERM_ESTB_VENDOR_ID = 0x1234;
char default_B_ESTB_CFG_LABEL_PERM_ESTB_MAC_ADDR[6] = {
	0x00, 0x01, 0x18, 0x00, 0x00, 0x01,
};


#define NUM_NODES_B_ESTB_CFG_LABEL_DYN 4

struct b_estb_cfg_node default_node_block_B_ESTB_CFG_LABEL_DYN [NUM_NODES_B_ESTB_CFG_LABEL_DYN] = {
    /* idx, flag, type, label, parent, next, prev, name, data ptr/sub dir, data len/data node */
    {0, 0, NODE_TYPE_DIR, B_ESTB_CFG_LABEL_RSVD0, 0, IDX_INVALID, IDX_INVALID, 0, 0, "/dyn", {{2, IDX_INVALID, 0}}},
    {1, 0, NODE_TYPE_DIR, B_ESTB_CFG_LABEL_RSVD0, 1, IDX_INVALID, IDX_INVALID, 0, 0, "/dyn/estb", {{IDX_INVALID, 3, 0}}},
    {2, 0, NODE_TYPE_UINT32, B_ESTB_CFG_LABEL_DYN_ESTB_HOST_ID, 2, 4, IDX_INVALID, NODE_STATE_INIT, 4, "/dyn/estb/host_id", {{0, 4, 0}}},
    {3, 0, NODE_TYPE_TXT, B_ESTB_CFG_LABEL_DYN_ESTB_FW_NAME, 2, 4, IDX_INVALID, NODE_STATE_INIT, 24, "/dyn/estb/fw_name", {{0, 4, 0}}},
};
struct b_estb_cfg_node node_block_B_ESTB_CFG_LABEL_DYN [NUM_NODES_B_ESTB_CFG_LABEL_DYN];


struct b_estb_cfg_ctl default_ctl_block_B_ESTB_CFG_LABEL_DYN = {
	B_ESTB_CFG_LABEL_DYN,
	B_ESTB_CFG_DEFAULT_DYN_CFG,
	0,
	DEFAULT_B_ESTB_CFG_HDR,
	node_block_B_ESTB_CFG_LABEL_DYN,
	NUM_NODES_B_ESTB_CFG_LABEL_DYN,
	NULL,
	0,
	0,
	0,
};

unsigned int default_B_ESTB_CFG_LABEL_DYN_ESTB_HOST_ID = 0x4321;
char default_B_ESTB_CFG_LABEL_DYN_ESTB_FW_NAME[] = "abc";



static struct b_estb_cfg_ctl ctl_block[B_ESTB_CFG_NUM_CFG_FILES];

static struct b_estb_cfg_info info_block[B_ESTB_CFG_NUM_CFG_FILES]=  {
	{B_ESTB_CFG_LABEL_PERM, "/perm"},
	{B_ESTB_CFG_LABEL_DYN, "/dyn"},
};

static void b_estb_cfg_init_values(int idx) {

	if (idx == B_ESTB_CFG_LABEL_PERM) {
		SET_DEFAULT_NODE_VALUE(node_block_B_ESTB_CFG_LABEL_PERM, default_node_block_B_ESTB_CFG_LABEL_PERM);

		SET_DEFAULT_CTL_VALUE(B_ESTB_CFG_LABEL_PERM, default_ctl_block_B_ESTB_CFG_LABEL_PERM);

		SET_DEFAULT_DATA_VALUE(B_ESTB_CFG_LABEL_PERM, 2, default_B_ESTB_CFG_LABEL_PERM_ESTB_VENDOR_ID);
		SET_DEFAULT_DATA_VALUE(B_ESTB_CFG_LABEL_PERM, 3, default_B_ESTB_CFG_LABEL_PERM_ESTB_MAC_ADDR);
	} else if (idx == B_ESTB_CFG_LABEL_DYN) {
		SET_DEFAULT_NODE_VALUE(node_block_B_ESTB_CFG_LABEL_DYN, default_node_block_B_ESTB_CFG_LABEL_DYN);
		SET_DEFAULT_CTL_VALUE(B_ESTB_CFG_LABEL_DYN, default_ctl_block_B_ESTB_CFG_LABEL_DYN);

		SET_DEFAULT_DATA_VALUE(B_ESTB_CFG_LABEL_DYN, 2, default_B_ESTB_CFG_LABEL_DYN_ESTB_HOST_ID);
		SET_DEFAULT_DATA_VALUE(B_ESTB_CFG_LABEL_DYN, 3, default_B_ESTB_CFG_LABEL_DYN_ESTB_FW_NAME);
	}
}

#endif
/* !!! the above data should be generated by a script automatically */



/* return 1 if succeed, 0 if fail */
static int b_estb_cfg_is_initialized(struct b_estb_cfg_ctl *ctl) {
	return ctl->initialized;
}

static unsigned long b_estb_cfg_checksum(struct b_estb_cfg_ctl * ctl) {
	unsigned long checksum = 0;
	unsigned char * data;
	size_t len;

	if (ctl->data_block == NULL) return 0;
	data = ctl->data_block + sizeof(struct b_estb_cfg_hdr);
	len = ctl->data_len - sizeof(struct b_estb_cfg_hdr);
	/* BDBG_MSG(("%s: data_len %d\n", BSTD_FUNCTION, len)); */
	checksum = FastCrc32_host(data, len);
	return checksum;
}

static int b_estb_cfg_init_hdr(int idx) {
	struct b_estb_cfg_ctl * ctl = &ctl_block[idx];
	struct b_estb_cfg_hdr *hdr_block = &ctl->hdr_block;
	memset(hdr_block, 0, sizeof(struct b_estb_cfg_hdr));
	memmove(hdr_block->magic, B_ESTB_CFG_VENDOR_MAGIC, sizeof(B_ESTB_CFG_VENDOR_MAGIC));
	memmove(hdr_block->ver_str, B_ESTB_CFG_VENDOR_VER_STR, sizeof(B_ESTB_CFG_VENDOR_VER_STR));
	hdr_block->ver_major =  B_ESTB_CFG_VENDOR_VER_MAJOR;
	hdr_block->ver_minor =  B_ESTB_CFG_VENDOR_VER_MINOR;

	hdr_block->hdr_size = sizeof(struct b_estb_cfg_hdr);

	hdr_block->checksum = 0;
	return 0;
}
static int b_estb_cfg_init_sem(int idx) {
	struct b_estb_cfg_ctl * ctl = &ctl_block[idx];
    ESTB_CFG_SEM_INIT(ctl);
	return 0;
}

/* return >= 0 if succeed, <0 if fail */
static int b_estb_cfg_get_file_id(char * dir, int *min, int * max) {
	struct b_estb_cfg_info * info;
	int i = 0;

	*min = -1; *max = -1;
	for ( i = 0; i < B_ESTB_CFG_NUM_CFG_FILES; i++) {
		info = &info_block[i];
		if (strncmp(dir, info->top_dir, sizeof(info->top_dir)) == 0) {
			*min = i; *max = i + 1;
			return 0;
		}
	}
	if (strncmp(dir, "/", 2) == 0) {
		*min = 0; *max = B_ESTB_CFG_NUM_CFG_FILES;
		return 0;
	}

	BDBG_ERR(("Cannot init %s, init applies to top directory only\n", dir));
	return -1;
}

static int b_estb_cfg_init_by_id(int idx) {
	struct b_estb_cfg_ctl * ctl = &ctl_block[idx];

	memset(ctl, 0, sizeof(struct b_estb_cfg_ctl));
	b_estb_cfg_init_values(idx);
	b_estb_cfg_init_hdr(idx);
	b_estb_cfg_init_sem(idx);
	if (b_estb_cfg_assemble_data(ctl) < 0 ) {
		return -1;
	}


	ctl->initialized = 1;
	return 0;
}
static int b_estb_cfg_uninit_by_id(int idx) {
	struct b_estb_cfg_ctl * ctl = &ctl_block[idx];

#ifdef _CFE_
#else
	if (ctl->data_block) free(ctl->data_block);
#endif
	memset(ctl, 0, sizeof(struct b_estb_cfg_ctl));

	return 0;
}



/* return >= 0 if succeed, <0 if fail */
static int b_estb_cfg_get_ctl_block_by_label(unsigned int label, struct b_estb_cfg_ctl ** ctl) {
	unsigned int idx;
	idx = B_ESTB_CFG_GET_FILE_ID(label);
	if (idx >= B_ESTB_CFG_NUM_CFG_FILES) {
		*ctl = NULL;
		BDBG_ERR(("Invalid file id %x for label 0x%x\n", idx, label));
		return -1;
	}
	if (b_estb_cfg_is_initialized(&ctl_block[idx])== 0) {
		BDBG_ERR(("%s not initialized\n", info_block[idx].top_dir));
		*ctl = NULL;
		return -1;
	} else {
		*ctl = &ctl_block[idx];
	}
	return 0;
}
/* return >= 0 if succeed, <0 if fail */
static int b_estb_cfg_get_label(char *path_name, unsigned int * label) {
	int ret = -1;
	int i,j;
	struct b_estb_cfg_ctl * ctl;

	for (i = 0; i < B_ESTB_CFG_NUM_CFG_FILES; i++) {
		ctl = & ctl_block[i];
		if (b_estb_cfg_is_initialized(ctl) == 0) continue;

		for ( j = 0; j < ctl->num_nodes; j++ ) {
			if (strncmp(ctl->node_block[j].name, path_name, NAME_LEN) == 0) {
				*label = ctl->node_block[j].label;
				ret = 0;
				return ret;
			}
		}
	}
	BDBG_ERR(("Cannot find label for %s\n", path_name));
	return ret;

}

/* return >= 0 if succeed, <0 if fail */
static int b_estb_cfg_get_value_by_label(unsigned int label, unsigned char *ptr, int * len, int type){
	int ret = -1;
	int i;
	struct b_estb_cfg_ctl * ctl = NULL;
	struct b_estb_cfg_node *  node;

	if (b_estb_cfg_get_ctl_block_by_label(label, &ctl) < 0) {
		return ret;
	}
	if (ctl->reload) b_estb_cfg_read_by_ctl(ctl);
	for ( i = 0; i < ctl->num_nodes; i++ ) {
		node = &ctl->node_block[i];
		/*
		 * BDBG_MSG(("%s: node type %02x, label 0x%x, %02x, 0x%x\n", BSTD_FUNCTION, node->type, node->label, type, label));
		 */
		if (node->type == type
		    && node->label == label) {
			if (type == NODE_TYPE_UINT32) {
				if (node->state == NODE_STATE_DEFAULT) {
					memmove(ptr,(unsigned char *)node->v.data.off_w, 4);
					*len = 4;
					ret = 0;
					return ret;
				} else if (node->state == NODE_STATE_LOADED) {
					memmove(ptr,(unsigned char *)(ctl->data_block + node->v.data.off_w), 4);
					*len = 4;
					ret = 0;
					return ret;
				}
			} else if (type == NODE_TYPE_TXT) {
				if (node->state == NODE_STATE_DEFAULT) {
					strncpy((char *)ptr, (char *)node->v.data.off_w,
						node->v.data.len);
					*len = node->v.data.len + 1; /* including '\0' */
					ret = 0;
					return ret;
				} else if (node->state == NODE_STATE_LOADED) {
					memmove(ptr,(unsigned char *)(ctl->data_block + node->v.data.off_w),
					       node->v.data.len);
					*len = node->v.data.len;
					ret = 0;
					return ret;
				}
			} else if (type == NODE_TYPE_BIN) {
				if (node->state == NODE_STATE_DEFAULT) {
					memmove(ptr, (unsigned char *)node->v.data.off_w, node->v.data.len);
					*len = node->v.data.len;
					ret = 0;
					return ret;
				} else if (node->state == NODE_STATE_LOADED) {
					memmove(ptr,(unsigned char *)(ctl->data_block + node->v.data.off_w),
					       node->v.data.len);
					*len = node->v.data.len;
					ret = 0;
					return ret;
				}
			}
		}
	}
	BDBG_ERR(("failed to get value for  label 0x%x\n", label));
	return ret;
}

/* return >= 0 if succeed, <0 if fail */
static int b_estb_cfg_get_uint32_by_label(unsigned int label, unsigned int *ptr){
	int ret = -1;
	int len = 4;
	if ((ret = b_estb_cfg_get_value_by_label(label, (unsigned char *)ptr, &len, NODE_TYPE_UINT32)) >= 0) {
		return ret;
	}
	BDBG_ERR(("failed to get uint32 for  label 0x%x\n", label));
	return ret;
}


/* return >= 0 if succeed, <0 if fail */
static int b_estb_cfg_get_txt_by_label(unsigned int label, char *ptr, int *len){
	int ret = -1;

	if ((ret = b_estb_cfg_get_value_by_label(label, (unsigned char *)ptr, len, NODE_TYPE_TXT)) >= 0) {
		return ret;
	}
	BDBG_ERR(("failed to get txt for  label 0x%x\n", label));
	return ret;

}


/* return >= 0 if succeed, <0 if fail */
static int b_estb_cfg_get_bin_by_label(unsigned int label, unsigned char *ptr, int * len){
	int ret = -1;
	if ((ret = b_estb_cfg_get_value_by_label(label, ptr, len, NODE_TYPE_BIN)) >= 0) {
		return ret;
	}
	return ret;
}

/* return >= 0 if succeed, <0 if fail */
static int b_estb_cfg_adjust_data_block(unsigned int label, int len, int type) {
	int ret = -1;
	int i, j;
	struct b_estb_cfg_ctl * ctl = NULL;
	struct b_estb_cfg_node *  node;
	int diff = 0, tmp_len = 0;
	unsigned char * tmp_buf = NULL;
	struct b_estb_cfg_data_hdr * dhdr;
	unsigned char * ptr;
	int pad = 0;
    int total_len = 0;

	if (b_estb_cfg_get_ctl_block_by_label(label, &ctl) < 0) {
		return ret;
	}
	for ( i = 0; i < ctl->num_nodes; i++ ) {
		node = &ctl->node_block[i];
		if (node->type == type && node->label == label) {
			diff = len - (node->v.data.len + node->v.data.pad);
			if (diff == 0) continue;
			pad = PADDING(diff);
			if (ctl->max_data_len - ctl->data_len >= (diff + pad)) {
				/* likely, there's enough space in current data block buffer, reuse it */
				tmp_buf = ctl->data_block;
			} else 	if (ctl->max_data_len - ctl->data_len < (diff + pad)) {
				/* need to create new data buffer */
				tmp_len = ctl->data_len + diff + pad;
#ifdef _CFE_
				tmp_buf = (unsigned char *) MAP_ADDR(ESTB_CFG_ADJUST_BUF_BASE + NONVOL_MAX_SIZE * ctl->id);
#else

				/* alloc new buffer, allocate 2xlen to avoid frequent malloc */
				tmp_buf = (unsigned char *)malloc(2 * tmp_len);
				if (!tmp_buf) {
					BDBG_ERR(("%s: malloc %d bytes failed", BSTD_FUNCTION, 2 * tmp_len));
					ret = -1;
					return ret;
				}
#endif
				ctl->max_data_len = 2 * tmp_len;
			}
				memmove(tmp_buf, ctl->data_block, node->v.data.off_w);
            /* BDBG_MSG((("diff %d, pad %d, off_w %d, data.len %d pad %d, data_len %d, tmp_len %d\n", diff, pad, node->v.data.off_w,
                       node->v.data.len, node->v.data.pad, ctl->data_len, tmp_len)); */
            if ( (diff + pad) >= 0) {
                for (j = ctl->data_len - 1; j >= (node->v.data.off_w + node->v.data.len + node->v.data.pad); j--) {
                    tmp_buf[j + (diff + pad)] = ctl->data_block[j];
                }
            } else {
                for (j = (node->v.data.off_w + node->v.data.len + node->v.data.pad); j <= ctl->data_len - 1;j++) {
                    tmp_buf[j + (diff + pad)] = ctl->data_block[j];
                }
            }
            memset(tmp_buf + node->v.data.off_w, 0, len + pad);
            /* fill zero for the tail */
            if ((diff+pad) <0)
                memset(&tmp_buf[ctl->data_len + (diff+pad)], 0x0, -(diff + pad));

			if (tmp_buf != ctl->data_block) {
#ifdef _CFE_
				memmove(ctl->data_block, tmp_buf, tmp_len);
#else

				/* free old buffer */
				free(ctl->data_block);
				ctl->data_block = tmp_buf;
#endif

			}
			node->v.data.len = len;
			node->v.data.pad = pad;
			continue;
		}
		node->v.data.off_w += (diff + pad);
	}

	/* assemble data */
	ptr = ctl->data_block + sizeof(struct b_estb_cfg_hdr);
	total_len = sizeof(struct b_estb_cfg_hdr);
	for ( i = 0; i < ctl->num_nodes; i++ ) {
		dhdr = (struct b_estb_cfg_data_hdr *)ptr;
		node = &ctl->node_block[i];
		if (node->type == NODE_TYPE_UINT32
		    || node->type == NODE_TYPE_TXT
		    || node->type == NODE_TYPE_BIN) {
			ptr += sizeof( struct b_estb_cfg_data_hdr);
			dhdr->label = node->label;
			dhdr->off = node->v.data.off_w;
			dhdr->len = node->v.data.len;
			dhdr->pad = node->v.data.pad;
			ptr += (dhdr->len + dhdr->pad);
			total_len += (dhdr->len + dhdr->pad);
			total_len += sizeof(struct b_estb_cfg_data_hdr);
		}
	}
    BDBG_MSG(("%s: total len %d\n", BSTD_FUNCTION, total_len));
	ctl->data_len = total_len;

	return 0;
}

/* return >= 0 if succeed, <0 if fail */
static int b_estb_cfg_set_value_by_label(unsigned int label, unsigned char * ptr, int len, int type) {
	int ret = -1;
	int i;
	struct b_estb_cfg_ctl * ctl = NULL;
	struct b_estb_cfg_node *  node;
	struct b_estb_cfg_property prop;

	if (b_estb_cfg_get_property_by_label(label, &prop) < 0) {
		return ret;
	}
	if (prop.field_len < len ) {
		BDBG_ERR(("%s: length %d exceeds field len %d\n", BSTD_FUNCTION, len, prop.len));
		return ret;
	}

	if (b_estb_cfg_adjust_data_block(label, len, type) < 0) {
		return ret;
	}

	if (b_estb_cfg_get_ctl_block_by_label(label, &ctl) < 0) {
		return ret;
	}
	for ( i = 0; i < ctl->num_nodes; i++ ) {
		node = &ctl->node_block[i];
		if (node->type == type && node->label == label) {

			if (type == NODE_TYPE_UINT32) {
				*(unsigned int *)&ctl->data_block[node->v.data.off_w] = *(unsigned int *)ptr;
				ret = 0;
				break;
			} else if (type == NODE_TYPE_TXT) {
				memset(&ctl->data_block[node->v.data.off_w], 0,
				       node->v.data.len);
				strncpy((char *)&ctl->data_block[node->v.data.off_w], (const char *)ptr, len);
				node->v.data.len = len;
				node->v.data.pad = PADDING(len);
				ret = 0;
			} else if (type == NODE_TYPE_BIN) {
				memmove((unsigned char *)&ctl->data_block[node->v.data.off_w], ptr, len);
				node->v.data.len = len;
				node->v.data.pad = PADDING(len);
				ret = 0;
			}
		}
	}
	if (ret == 0) {
		return b_estb_cfg_write_by_ctl(ctl);
	}
	BDBG_ERR(("failed to set value for  label 0x%x\n", label));
	return ret ;
}


/* return >= 0 if succeed, <0 if fail */
static int b_estb_cfg_set_uint32_by_label(unsigned int label, unsigned int val){
	int ret = -1;

	if ((ret = b_estb_cfg_set_value_by_label(label, (unsigned char *)&val, 4,
						 NODE_TYPE_UINT32)) >= 0) {
		return ret;
	}
	BDBG_ERR(("failed to set uint32 for  label 0x%x\n", label));
	return ret ;
}

/* return >= 0 if succeed, <0 if fail */
static int b_estb_cfg_set_txt_by_label(unsigned int label, char *ptr, int len){
	int ret = -1;

	if ((ret = b_estb_cfg_set_value_by_label(label, (unsigned char *)ptr, len,
						 NODE_TYPE_TXT)) >= 0) {
		return ret;
	}
	BDBG_ERR(("failed to set txt for  label 0x%x\n", label));
	return ret;
}

/* return >= 0 if succeed, <0 if fail */
static int b_estb_cfg_set_bin_by_label(unsigned int label, unsigned char *ptr, int len){
	int ret = -1;

	if ((ret = b_estb_cfg_set_value_by_label(label, ptr, len,
						 NODE_TYPE_BIN)) >= 0) {
		return ret;
	}
	BDBG_ERR(("failed to set bin for  label 0x%x\n", label));
	return ret;
}


/* return >= 0 if succeed, <0 if fail */
static int b_estb_cfg_print_by_label(unsigned int label){
	int ret = -1;
	int i, j;
	struct b_estb_cfg_node *  node;
	unsigned char * ptr;
	struct b_estb_cfg_ctl * ctl = NULL;

	if (b_estb_cfg_get_ctl_block_by_label(label, &ctl) < 0) {
		return ret;
	}

	for ( i = 0; i < ctl->num_nodes; i++ ) {
		node = &ctl->node_block[i];
		if (node->label == label) {
			if (node->type == NODE_TYPE_TXT) {
                /*
				BDBG_MSG(("0x%08x: %s (%d) = (txt) %s\n", label, node->name, node->v.data.len,
				       (char *)&ctl->data_block[node->v.data.off_w]));
                */
				BDBG_DSP(("%s (0x%x)= %s\n", node->name, node->label, (char *)&ctl->data_block[node->v.data.off_w]));
				ret = 0;
			} else if (node->type == NODE_TYPE_UINT32) {
                /*
				BDBG_MSG(("0x%08x: %s (%d) = (uint32) 0x%08x\n", label, node->name,  node->v.data.len,
				       *(unsigned int *)&ctl->data_block[node->v.data.off_w]));
                       */
				BDBG_DSP(("%s (0x%x)= 0x%08x\n", node->name, node->label,  *(unsigned int *)&ctl->data_block[node->v.data.off_w]));
				ret = 0;
			} else if (node->type == NODE_TYPE_BIN) {
				ptr = (unsigned char *) &ctl->data_block[node->v.data.off_w];
                /*
				BDBG_MSG(("0x%08x: %s (%d) = (bin)\n", label, node->name, node->v.data.len));
				for (j = 0; j < node->v.data.len; j++) {
					BDBG_MSG(("%02x ",  *ptr++));
					if ((j & 0xf) == 0xf) BDBG_MSG(("\n"));
				}
				BDBG_MSG(("\n"));
                */
				BDBG_DSP(("%s (0x%x)= \n", node->name, node->label));
				for (j = 0; j < node->v.data.len; j++) {
					BDBG_DSP(("%02x ",  *ptr++));
					if ((j & 0xf) == 0xf) BDBG_DSP(("\n"));
				}
				BDBG_DSP(("\n"));
				ret = 0;
			} else if (node->type == NODE_TYPE_DIR) {
				/* skip */
			} else {
				BDBG_ERR(("invalid type %x\n", node->type));
				ret = -1;
			}
			return ret;
		}
	}
	return ret;
}

/* for little endian only */
static unsigned int my_ntohl(unsigned int data) {
	unsigned int ret;
	unsigned char *ptr = (unsigned char *) &data;
	ret = (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | (ptr[3]);
	return ret;

}
static unsigned int my_htonl (unsigned int val) {
  return ((val & 0x000000ff) << 24) | ((val & 0x0000ff00) <<  8) |
	  ((val & 0x00ff0000) >>  8) | ((val & 0xff000000) >> 24);
}

/* return >= 0 if succeed, <0 if fail */
static int b_estb_cfg_write_by_ctl (struct b_estb_cfg_ctl * ctl){
	int ret = -1;
	int fd;
	int n = 0;
	int len = 0;
	char * fn_name = ctl->file_name;
	struct b_estb_cfg_hdr * hdr_block = &ctl->hdr_block;
    ESTB_CFG_SEM_WAIT(ctl);

	if (strlen(fn_name) == 0) {
		BDBG_ERR(("Est Cfg File name not set, cannot write\n"));
		return -1;
	}
	hdr_block->checksum = b_estb_cfg_checksum(ctl);
	hdr_block->file_size = my_htonl(ctl->data_len);

	/* copy header */
	memmove(ctl->data_block, (unsigned char *) hdr_block, sizeof(struct b_estb_cfg_hdr));

	/* write out */
    /*
	BDBG_MSG(("write to %s, checksum 0x%08x, data len %d\n", fn_name, hdr_block->checksum, ctl->data_len));
    */
#if defined(_CFE_) || defined(NOOS)
	len = ctl->data_len;

	/* if dest flash device is cfg device, write with cfg fs */
	if (cfe_is_cfg_data(fn_name)) {
		/* this flash device contains config data, handle separately */
		BDBG_MSG(("Programming (%s, %d bytes) with cfg format...", fn_name, len));

	    ret = cfe_save_cfg_data(fn_name, "", ctl->data_block, len);
	    if ( ret < 0) {
		    xprintf("Cannot write to Device %s.\n",fn_name);
		    ret = -1;
		    return ret;
	    }
	    BDBG_MSG(("Done\n"));
	} else {
	    xprintf("Device %s not cfg device\n", fn_name);
	    ret = -1;
	}
    ESTB_CFG_SEM_POST(ctl);
	return ret;
#else
	if ((fd = open(fn_name, O_CREAT | O_WRONLY | O_TRUNC)) < 0) {
		BDBG_ERR(("Cannot write to %s\n",fn_name));
        ESTB_CFG_SEM_POST(ctl);
		return -1;
	}
	len = ctl->data_len;
    BDBG_MSG(("%s %d bytes written\n", BSTD_FUNCTION, len));
	if (len) {
        n = write(fd, ctl->data_block, len);
		if (n != len) {
			BDBG_ERR(("write error, write %d, expect %d\n", n, len));
            ret = -1;
			goto out;
		}
	} else {
		BDBG_ERR(("no non-default data\n"));
		ret = 0;
		goto out;
	}
	ret = 0;
 out:
	close(fd);
    ESTB_CFG_SEM_POST(ctl);
	return ret;
#endif
}
static void dump_bytes(unsigned char * ptr, int len) {
	int i;
	for (i = 0; i < len; i++) {
		BDBG_MSG(("%02x ", ptr[i]));
		if ((i & 0xfffffff0) == 0xf) BDBG_MSG(("\n"));
	}
	BDBG_MSG(("\n"));
}

/* return >= 0 if succeed, <0 if fail */
static int b_estb_cfg_is_header_valid(struct b_estb_cfg_ctl * ctl, struct b_estb_cfg_hdr * nhdr, int file_len){
	struct b_estb_cfg_hdr * hdr_block = &ctl->hdr_block;

	if (my_ntohl(nhdr->file_size) != (unsigned int)file_len) {
		BDBG_ERR(("file length does not match, claim %d, actual %d\n", my_ntohl(nhdr->file_size), file_len));
		return -1;
	}
	if (memcmp(hdr_block->magic, nhdr->magic, sizeof(hdr_block->magic)) != 0) {
		BDBG_ERR(("%s header magic not match, expect %s, get %s, %d bytes\n", ctl->file_name, hdr_block->magic,
                  nhdr->magic, sizeof(hdr_block->magic)));
		dump_bytes((unsigned char *)hdr_block->magic, sizeof(hdr_block->magic));
		return -1;
	}

	/* how about backward compatibility ? */
	if (memcmp(hdr_block->ver_str, nhdr->ver_str, sizeof(hdr_block->ver_str)) != 0) {
		BDBG_ERR(("header version not match, expect %s, get %s\n", hdr_block->ver_str, nhdr->ver_str));
		dump_bytes((unsigned char *)hdr_block->ver_str, sizeof(hdr_block->ver_str));
		return -1;
	}

	if (hdr_block->ver_major != nhdr->ver_major) {
		BDBG_ERR(("header major version not match, expect %d, get %d\n",
			  hdr_block->ver_major, nhdr->ver_major));
		return -1;
	}
	if (hdr_block->ver_minor != nhdr->ver_minor) {
		BDBG_ERR(("header minor version not match, expect %d, get %d\n",
			  hdr_block->ver_minor, nhdr->ver_minor));
		return -1;
	}

	if (hdr_block->hdr_size != nhdr->hdr_size) {
		BDBG_ERR(("header size not match, expect %d, get %d\n",
			  hdr_block->hdr_size, nhdr->hdr_size));
		return -1;
	}

	return 0;

}

/* return >= 0 if succeed, <0 if fail */
static int b_estb_cfg_is_data_valid(unsigned char * data, int data_len) {
	unsigned int off_c;
	struct b_estb_cfg_data_hdr * dhdr;
	/*
	 * variables  are saved back-to-back as
	 * | variable 1 | variable 2 | ... |
	 * for each variable,
	 * | label (4 byte) | offset (4 byte) | length (4 byte) |
	 *
	 * Verifiy the calculated data offset matches the offset read from buffer
	 */
	/* skip label */

	/* offset read from buffer */
	off_c = sizeof(struct b_estb_cfg_hdr);
	dhdr = (struct b_estb_cfg_data_hdr *)(data + off_c);
	off_c += sizeof(struct b_estb_cfg_data_hdr);

	do {
		/* BDBG_MSG(("read offset %d, calculated offset %d, len %d\n", dhdr->off, off_c, dhdr->len)); */
		if (dhdr->off != (int)off_c || (int)off_c > data_len) {
			BDBG_ERR(("%s: read offset %d and calculated offset %d does not match, or out of bound %d\n",
				  BSTD_FUNCTION, dhdr->off, off_c, data_len));
#if 0
			/* debug only */
			exit(1);
#endif
			return -1;
		}
		/* might be a problem if not aligned to word boundary ??? */
		off_c += (dhdr->len + dhdr->pad);
		dhdr = (struct b_estb_cfg_data_hdr *)(data + off_c);
		off_c += sizeof(struct b_estb_cfg_data_hdr);
        /* BDBG_MSG(("%s: off_C %d data_len %d\n", BSTD_FUNCTION, off_c, data_len)); */

	} while ((int)off_c <= data_len);
	return 0;
}
/*
 * return >= 0 if succeed, <0 if fail
 * before calling this function, b_estb_cfg_is_data_valid() should have been called.
 */
static int b_estb_cfg_update_data(struct b_estb_cfg_ctl * ctl, unsigned char * data, int data_len){
	int i;
	unsigned int off_c;
	struct b_estb_cfg_data_hdr * dhdr;
	struct b_estb_cfg_node *  node = NULL;
	unsigned int label = 0;

    /* BDBG_MSG(("%s\n", BSTD_FUNCTION)); */
	off_c = sizeof(struct b_estb_cfg_hdr);
	dhdr = (struct b_estb_cfg_data_hdr *)(data + off_c);
	off_c += sizeof(struct b_estb_cfg_data_hdr);

	do {
		if (dhdr->off != (int)off_c || (int)off_c > data_len) {
			BDBG_ERR(("%s: read offset %d and calculated offset %d does not match, or out of bound %d\n",
				  BSTD_FUNCTION, dhdr->off, off_c, data_len));
			return -1;
		}
		for ( i = 0; i < ctl->num_nodes; i++ ) {
			node = &ctl->node_block[i];
			label = node->label;
			if ((int)label == dhdr->label)
				break;
		}
        /*
        BDBG_MSG(("%s i = %d, dhdr->off 0x%x, len 0x%x, pad 0x%x dhdr->label %x label %x\n",
                  BSTD_FUNCTION, i, dhdr->off, dhdr->len, dhdr->pad, dhdr->label, label));
        */
		if ( (int)label == dhdr->label) {
			/* found node */
			node->v.data.off_w = dhdr->off;
			node->v.data.len = dhdr->len;
			node->v.data.pad = dhdr->pad;
			node->state = NODE_STATE_LOADED;
		}  else {
			BDBG_WRN(("Label 0x%x not found, skip\n", dhdr->label));
		}

		off_c += (dhdr->len + dhdr->pad);
		dhdr = (struct b_estb_cfg_data_hdr *)(data + off_c);
		off_c += sizeof(struct b_estb_cfg_data_hdr);
        /* BDBG_MSG(("%s: off_C %d data_len %d\n", BSTD_FUNCTION, off_c, data_len)); */

	} while ((int)off_c <= data_len);

	return 0;
}

static int b_estb_cfg_assemble_data(struct b_estb_cfg_ctl * ctl){
	int i;
	int len;
	struct b_estb_cfg_node *  node;
	unsigned char * tmp_buf, * ptr;
	struct b_estb_cfg_data_hdr * dhdr;
	int ret = -1;

    /* BDBG_MSG(("%s\n", BSTD_FUNCTION));*/
#ifdef _CFE_
	if (ctl->data_block == NULL)
		ctl->data_block = (unsigned char *) MAP_ADDR(ESTB_CFG_DATA_BUF_BASE + NONVOL_MAX_SIZE * ctl->id);
#endif
	/*
	 * now a valid config has been generated, but it's scattered among different buffer,
	 * we need to assembly it into single buffer.
	 */
	len = sizeof(struct b_estb_cfg_hdr);
	for ( i = 0; i < ctl->num_nodes; i++ ) {
		node = &ctl->node_block[i];
		if (node->type == NODE_TYPE_UINT32
		    || node->type == NODE_TYPE_TXT
		    || node->type == NODE_TYPE_BIN) {
			len += (node->v.data.len + node->v.data.pad);
			len += sizeof(struct b_estb_cfg_data_hdr);
		}
	}
	if (len > ctl->max_data_len) {
#ifdef _CFE_
		tmp_buf = (unsigned char *) MAP_ADDR(ESTB_CFG_ASSEMBLE_BUF_BASE + NONVOL_MAX_SIZE * ctl->id);
#else
		/* alloc new buffer, allocate 2xlen to avoid frequent malloc */
		tmp_buf = (unsigned char *)malloc(2 * len);
		if (!tmp_buf) {
			BDBG_ERR(("%s: malloc %d bytes failed", BSTD_FUNCTION, 2 * len));
			ret = -1;
			goto out;
		}
#endif
		ctl->max_data_len = 2 * len;
	} else {
		/* reuse data_block */
		tmp_buf = ctl->data_block;
	}

	/* assemble data */
	ptr = tmp_buf + sizeof(struct b_estb_cfg_hdr);
	for ( i = 0; i < ctl->num_nodes; i++ ) {
		dhdr = (struct b_estb_cfg_data_hdr *)ptr;
		node = &ctl->node_block[i];
		/*		BDBG_MSG(("i = %d, label 0x%x type 0x%x, tmp_buf%p, ctl->data_block %p\n", i, node->label, node->type, tmp_buf, ctl->data_block));
		 */
		if (node->type == NODE_TYPE_UINT32
		    || node->type == NODE_TYPE_TXT
		    || node->type == NODE_TYPE_BIN) {
			/* BDBG_MSG(("i = %d, data.off_w 0x%x, data.len 0x%x, state %d\n", i, node->v.data.off_w, node->v.data.len, node->state)); */

			ptr += sizeof( struct b_estb_cfg_data_hdr);
			dhdr->label = node->label;
			dhdr->off = ptr - tmp_buf;
			/* BDBG_MSG(("ptr %p\n", ptr)); */
			if (node->state == NODE_STATE_DEFAULT) {
				memmove(ptr, (unsigned char *)node->v.data.off_w, node->v.data.len);
                if (b_estb_cfg_is_initialized(ctl)) {
                    node->v.data.off_w = dhdr->off;
                    node->state = NODE_STATE_LOADED;
                }
			} else {
				memmove(ptr, &ctl->data_block[node->v.data.off_w], node->v.data.len);
                node->v.data.off_w = dhdr->off;
			}
			dhdr->len = node->v.data.len;
			dhdr->pad = node->v.data.pad;
			ptr += (dhdr->len + dhdr->pad);
			/* BDBG_MSG(("new ptr %p, dhdr->len %d, dhdr->pad %d, off_w %d\n", ptr, dhdr->len, dhdr->pad, node->v.data.off_w));  */

		}
	}

	if (tmp_buf != ctl->data_block) {
#ifdef _CFE_

		memmove(ctl->data_block, tmp_buf, len);
#else
		if (ctl->data_block) free(ctl->data_block);
		ctl->data_block = tmp_buf;
#endif
	}

	ctl->data_len = len;

	ctl->hdr_block.checksum = b_estb_cfg_checksum(ctl);

	ctl->hdr_block.file_size = my_htonl(ctl->data_len);

	/* copy header */
	memmove(ctl->data_block, &ctl->hdr_block, sizeof(struct b_estb_cfg_hdr));
	ret = 0;

 out:
	return ret;

}
/* return >= 0 if succeed, <0 if fail */
static int b_estb_cfg_read_by_ctl(struct b_estb_cfg_ctl * ctl){
	int ret = -1;
	int fd;
	int n = 0;
	char * fn_name = ctl->file_name;
	unsigned char * tmp_buf = NULL;
	unsigned char * tmp_data_buf = NULL;
	int tmp_len = 0, tmp_data_len = 0;
	struct b_estb_cfg_hdr * tmp_hdr = NULL;
	unsigned long tmp_checksum = 0;

	/* BDBG_MSG(("%s : file_name %s\n", BSTD_FUNCTION, ctl->file_name));  */
    ESTB_CFG_SEM_WAIT(ctl);

#if defined(_CFE_) || defined(NOOS)

#ifdef _CFE_
	/* ??? use another buffer (assemble?) to avoid corrupting data_buf */
	tmp_buf = (unsigned char *) MAP_ADDR(ESTB_CFG_DATA_BUF_BASE + NONVOL_MAX_SIZE * ctl->id);
#else
	tmp_buf = (unsigned char *)malloc(NONVOL_MAX_SIZE);
#endif
	tmp_hdr = (struct b_estb_cfg_hdr *) tmp_buf;
	tmp_data_buf = (unsigned char *) tmp_buf + sizeof(struct b_estb_cfg_hdr);

	if (cfe_is_cfg_data(fn_name)) {
		tmp_len = (tmp_len > NONVOL_MAX_SIZE)? NONVOL_MAX_SIZE : tmp_len;

		ret = cfe_load_cfg_data(fn_name, "", tmp_buf, &tmp_len);
		if (ret < 0 || !tmp_buf) {
			BDBG_ERR(("Cannot read cfg from %s\n",fn_name));
            ESTB_CFG_SEM_POST(ctl);
			return -1;
		}
	} else {
		BDBG_ERR(("Cannot read cfg from %s\n",fn_name));
        ESTB_CFG_SEM_POST(ctl);
		return -1;
	}

	tmp_data_len = tmp_len - sizeof(struct b_estb_cfg_hdr);
#else
	if ((fd = open(fn_name, O_RDONLY)) < 0) {
		BDBG_ERR(("Cannot read from %s\n",fn_name));
        ESTB_CFG_SEM_POST(ctl);
		return -1;
	}

	if (strlen(fn_name) == 0) {
		BDBG_ERR(("Est Cfg File name not set, cannot read\n"));
        ESTB_CFG_SEM_POST(ctl);
		return -1;
	}

	/* read the whole file and verify the format */
	tmp_len = lseek(fd, 0, SEEK_END);
	tmp_buf = (unsigned char *)malloc(tmp_len);
	if (!tmp_buf) {
		BDBG_ERR(("%s: malloc %d bytes failed", BSTD_FUNCTION, tmp_len));
		ret = -1;
		goto out;
	}
	lseek(fd, 0, SEEK_SET);
	if ((n = read(fd, tmp_buf, tmp_len)) != tmp_len) {
		BDBG_ERR(("Read error, read %d, expect %d\n", n, tmp_len));
        ret = -1;
		goto out;
	}
	BDBG_MSG(("Read %d bytes\n", tmp_len));
	tmp_hdr = (struct b_estb_cfg_hdr *) tmp_buf;
	tmp_data_buf = (unsigned char *) tmp_buf + sizeof(struct b_estb_cfg_hdr);
	tmp_data_len = tmp_len - sizeof(struct b_estb_cfg_hdr);
#endif
	/* check the header */
	if (b_estb_cfg_is_header_valid(ctl, tmp_hdr, tmp_len) < 0) {
        ret = -1;
		goto out;
	}
	/* verify checksum */
	tmp_checksum = FastCrc32_host(tmp_data_buf, tmp_data_len);
	BDBG_MSG(("Read from %s, checksum 0x%08x, calculated 0x%08lx, len %d\n", fn_name,
		  tmp_hdr->checksum, tmp_checksum, tmp_data_len));

	if ( tmp_hdr->checksum != tmp_checksum) {
		BDBG_ERR(("checksum failed, use default cfg\n"));
		b_estb_cfg_init_by_id(ctl->id);
        ret = -1;
		goto out;
	}

	/*
	 * verify data format
	 */
	if (b_estb_cfg_is_data_valid(tmp_buf, tmp_len) < 0 ) {
		ret = -1;
		goto out;
	}
	/* update node */
	if (b_estb_cfg_update_data(ctl, tmp_buf, tmp_len) < 0 ) {
		ret = -1;
		goto out;
	}
#ifdef _CFE_
#else
	if (ctl->data_block) free(ctl->data_block);
#endif
	ctl->data_block = tmp_buf;
	ctl->data_len = tmp_len;
	ctl->max_data_len = tmp_len;
	ctl->reload = 0;
	ret = 0;
	tmp_buf = NULL;

	/* assemble data */
	if (b_estb_cfg_assemble_data(ctl) < 0 ) {
		ret = -1;
		goto out;
	}
    ret = 0;
 out:
#ifdef _CFE_
#elif defined(NOOS)
	if (tmp_buf) free(tmp_buf);
#else
	if (tmp_buf) free(tmp_buf);
	close(fd);
#endif
    ESTB_CFG_SEM_POST(ctl);
    return ret;
}


/* return >= 0 if succeed, <0 if fail */
static int b_estb_cfg_get_index_by_label(unsigned int label) {
	int ret = -1;
	int i;
	struct b_estb_cfg_ctl * ctl = NULL;

	if (b_estb_cfg_get_ctl_block_by_label(label, &ctl) < 0) {
		return ret;
	}

	for ( i = 0; i < ctl->num_nodes; i++ ) {
		if (ctl->node_block[i].label == label) {
			return i;
		}
	}
	BDBG_ERR(("failed to get index for  label 0x%x\n", label));
	return -1;
}

/* return >= 0 if succeed, <0 if fail */
static int b_estb_cfg_set_file_name(char * dir, char * fn) {
	struct b_estb_cfg_ctl * ctl;
	int i;
	for ( i = 0; i < B_ESTB_CFG_NUM_CFG_FILES; i++) {
		ctl = &ctl_block[i];
		if (b_estb_cfg_is_initialized(ctl)== 0) {
			continue;
		}
		if (strncmp(dir, ctl->node_block[0].name, sizeof(ctl->node_block[0].name)) == 0) {
			break;
		}
	}
	if (i >= B_ESTB_CFG_NUM_CFG_FILES) {
		BDBG_ERR(("Cannot set %s with %s, this func applies to top directory only\n", dir, fn));
		return -1;
	}

	strncpy(ctl->file_name, fn, NAME_LEN);
	/* BDBG_MSG(("%s : new file name %s\n", BSTD_FUNCTION, ctl->file_name)); */
	ctl->reload = 1;
	return 0;
}

/* return >= 0 if succeed, <0 if fail */
static int b_estb_cfg_get_property_by_label(unsigned int label, struct b_estb_cfg_property * prop){
	int ret = -1;
	int i;
	struct b_estb_cfg_ctl * ctl = NULL;
	struct b_estb_cfg_node *  node;


	if (b_estb_cfg_get_ctl_block_by_label(label, &ctl) < 0) {
		return ret;
	}
	for ( i = 0; i < ctl->num_nodes; i++ ) {
		node = &ctl->node_block[i];
		/*
		 * BDBG_MSG(("%s: node type %02x, label 0x%x, %02x, 0x%x\n", BSTD_FUNCTION, node->type, node->label, type, label));
		 */
		if (node->label == label) {
			prop->type = node->type;
			prop->len = node->v.data.len;
			prop->field_len = node->field_len;
			prop->flag = node->flag;
			memmove(prop->name, node->name, NAME_LEN);
			return 0;
		}
	}
	BDBG_ERR(("failed to get value for  label 0x%x\n", label));
	return ret;
}


/***************************************************************************
Summary:
Initialize the internal structure pointed to by dir.

Description:
This function generates initializes the internal data structure pointed to by dir.
It restores the default data value. The dir most be either "/" or one of the top
directories.

A top directory must be initialized before being opened.

Input:
	char * path - specifies the path to initialize. Must be either "/"
                      (for all data structure). Or one of the top directories
                      (for individual config file),
                      like "/perm", "/dyn".
                      .
Returns:
	>= 0 - if succeed
        < 0 - if fail
***************************************************************************/

int B_Estb_cfg_Init(char * dir) {
	int ret = -1;
	int i = 0, min = -1, max = -1;

	if ((ret = b_estb_cfg_get_file_id(dir, &min, &max))< 0) {
		return ret;
	}

	for ( i = min; i <max; i++) {
		b_estb_cfg_init_by_id(i);
	}
	return 0;
}

/***************************************************************************
Summary:
Freeup resource.

Description:
This function frees up the resource used by b_estb_cfg module

Input:
        none
Returns:
	>= 0 - if succeed
        < 0 - if fail

***************************************************************************/

int B_Estb_cfg_Uninit(char * dir) {
	int ret = -1;
	int i = 0, min = -1, max = -1;

	if ((ret = b_estb_cfg_get_file_id(dir, &min, &max))< 0) {
		return ret;
	}

	for ( i = min; i <max; i++) {
		b_estb_cfg_uninit_by_id(i);
	}
	return 0;
}

/***************************************************************************
Summary:
Get the uint32 variable represented by name.

Description:
This function retrieves uint32 data represented by name.

Input:
	char * name - the name as represented in the config file
                      like "/perm/estb/vendor_id"
        unsigned int * val - the pointer for the returned data.

Returns:
	>= 0 - if succeed
        < 0 - if fail
***************************************************************************/
int B_Estb_cfg_Get_uint32(char * name, unsigned int * val){
	int ret = -1;
	unsigned int label = 0;

	if ((ret = b_estb_cfg_get_label(name, &label)) >= 0) {
		ret = b_estb_cfg_get_uint32_by_label(label, val);
	} else {
		BDBG_ERR(("failed to get label for %s\n", name));
	}
	return ret;
}

/***************************************************************************
Summary:
Set the uint32 variable represented by name.

Description:
This function saves uint32 data represented by name.

Input:
	char * name - the name as represented in the config file
                      like "/perm/estb/vendor_id"
        unsigned int val - the new value to save.

Returns:
	>= 0 - if succeed
        < 0 - if fail
***************************************************************************/
int B_Estb_cfg_Set_uint32(char * name, unsigned int val){
	int ret = -1;
	unsigned int label = 0;

	if ((ret = b_estb_cfg_get_label(name, &label)) >= 0) {
		ret = b_estb_cfg_set_uint32_by_label(label, val);
	} else {
		BDBG_ERR(("failed to get label for %s\n", name));
	}
	return ret;
}

/***************************************************************************
Summary:
Get the txt variable represented by name.

Description:
This function retrieves text data represented by name.

Input:
	char * name - the name as represented in the config file
                      like "/perm/estb/vendor_id"
        char * val - the pointer for the returned text.
        int * len - the pointer for the returned text length.

Returns:
	>= 0 - if succeed
        < 0 - if fail
***************************************************************************/

int B_Estb_cfg_Get_txt(char * name, char * val, int * len){
	int ret = -1;
	unsigned int label = 0;

	if ((ret = b_estb_cfg_get_label(name, &label)) >= 0) {
		ret = b_estb_cfg_get_txt_by_label(label, val, len);
	} else {
		BDBG_ERR(("failed to get label for %s\n", name));
	}
	return ret;
}

/***************************************************************************
Summary:
Set the text variable represented by name.

Description:
This function saves text data represented by name.

Input:
	char * name - the name as represented in the config file
                      like "/perm/estb/vendor_id"
        char * val - the pointer to the new text to save.
        int len  - the length of the new text to save.

Returns:
	>= 0 - if succeed
        < 0 - if fail
***************************************************************************/
int B_Estb_cfg_Set_txt(char * name, char * val, int len){
	int ret = -1;
	unsigned int label = 0;

	if ((ret = b_estb_cfg_get_label(name, &label)) >= 0) {
		ret = b_estb_cfg_set_txt_by_label(label, val, len);
	} else {
		BDBG_ERR(("failed to get label for %s\n", name));
	}
	return ret;
}

/***************************************************************************
Summary:
Get the binary sting represented by name.

Description:
This function retrieves binary string represented by name.

Input:
	char * name - the name as represented in the config file
                      like "/perm/estb/mac_addr"
        unsigned char * val - the pointer for the returned binary.
        int * len - the pointer for the returned binary length.

Returns:
	>= 0 - if succeed
        < 0 - if fail
***************************************************************************/
int B_Estb_cfg_Get_bin(char * name, unsigned char * val, int * len){
	int ret = -1;
	unsigned int label = 0;

	if ((ret = b_estb_cfg_get_label(name, &label)) >= 0) {
		ret = b_estb_cfg_get_bin_by_label(label, val, len);
	} else {
		BDBG_ERR(("failed to get label for %s\n", name));
	}
	return ret;
}

/***************************************************************************
Summary:
Set the binary sting represented by name.

Description:
This function saves binary string represented by name.

Input:
	char * name - the name as represented in the config file
                      like "/perm/estb/mac_addr"
        unsigned char * val - the pointer for the new binary.
        int len - the new binary string length.

Returns:
	>= 0 - if succeed
        < 0 - if fail
***************************************************************************/
int B_Estb_cfg_Set_bin(char * name, unsigned char * val, int len){
	int ret = -1;
	unsigned int label = 0;

	if ((ret = b_estb_cfg_get_label(name, &label)) >= 0) {
		ret = b_estb_cfg_set_bin_by_label(label, val, len);
	} else {
		BDBG_ERR(("failed to get label for %s\n", name));
	}
	return ret;
}

/***************************************************************************
Summary:
Print the value represented by name.

Description:
This function prints the value represented by name, regardless of the type
of the value.

Input:
	char * name - the name as represented in the config file
                      like "/perm/estb/mac_addr"
Returns:
	>= 0 - if succeed
        < 0 - if fail
***************************************************************************/
int B_Estb_cfg_Print(char * name){
	int ret = -1;
	unsigned int label = 0;

	if ((ret = b_estb_cfg_get_label(name, &label)) >= 0) {
		ret = b_estb_cfg_print_by_label(label);
	} else {
		BDBG_ERR(("failed to get label for %s\n", name));
	}
	return ret;
}

/***************************************************************************
Summary:
Print all the values under a directory. (top directory only)


Description:
This function prints all the value under a directory. Currently it works for
top dirctory only.

Input:
	char * dir - the directory to list

Returns:
	>= 0 - if succeed
        < 0 - if fail
***************************************************************************/

void B_Estb_cfg_List(char * dir) {
	struct b_estb_cfg_node *  node;
	unsigned int label;

	struct b_estb_cfg_ctl * ctl;
	int i = 0, min = -1, max = -1;

	if (b_estb_cfg_get_file_id(dir, &min, &max)< 0) {
		return;
	}

	for ( i = min; i <max; i++) {
		ctl = &ctl_block[i];
		/* BDBG_MSG(("min %d, max %d\n", min, max)); */
		if (b_estb_cfg_is_initialized(&ctl_block[i])== 0) {
			BDBG_ERR(("%s not initialized\n", info_block[i].top_dir));
			continue;
		}
		BDBG_MSG(("Estb CFG from %s version %s %d.%d\n", ctl->hdr_block.magic,
		       ctl->hdr_block.ver_str, ctl->hdr_block.ver_major,
		       ctl->hdr_block.ver_minor));
		BDBG_MSG(("Label: name = value\n"));
		for ( i = 0; i < ctl->num_nodes; i++ ) {
			node = &ctl->node_block[i];
			label = node->label;
			if (label != B_ESTB_CFG_LABEL_RSVD0)
				b_estb_cfg_print_by_label(label);
		}
	}

}

/***************************************************************************
Summary:
Open a config file and save it under a top directory

Description:
This function reads a config file and saves all the value under a top directory.

Input:
	char * dir - the directory to read to. Must be a top directory like "/perm"
	char * fn - config file name

Returns:
	>= 0 - if succeed
        < 0 - if fail
***************************************************************************/
int B_Estb_cfg_Open(char * dir, char * fn){
	int ret = -1;
	struct b_estb_cfg_ctl * ctl;
	int i = 0, min = -1, max = -1;

	if (b_estb_cfg_get_file_id(dir, &min, &max)< 0) {
		return ret;
	}

	/* open one top dir at a time */
	if ( (min+1) != max) {
		BDBG_ERR(("can not open %s with %s.\n", dir, fn));
		BDBG_ERR(("Available directories that can open\n"));
		for (i = 0; i < B_ESTB_CFG_NUM_CFG_FILES; i++) {
			BDBG_ERR(("%s  ", info_block[i].top_dir));
		}
		BDBG_ERR(("\n"));
		return -1;
	}
	if (b_estb_cfg_init_by_id(min) < 0) {
		return ret;
	}

	if (b_estb_cfg_set_file_name(dir, fn) <0 ) {
		return -1;
	}
	for ( i = min; i <max; i++) {
		ctl = &ctl_block[i];
		if (b_estb_cfg_read_by_ctl(ctl) < 0) {
			BDBG_ERR(("read idx %d failed\n", i));
			BDBG_ERR(("Restoring %s to default\n", dir));
			b_estb_cfg_init_by_id(i);
			BDBG_ERR(("Link %s to %s\n", dir, fn));
			if (b_estb_cfg_set_file_name(dir, fn) <0 ) {
				return -1;

			}

			BDBG_ERR(("Write to %s\n", fn));
			if ((ret = b_estb_cfg_write_by_ctl(ctl)) < 0) {
				BDBG_ERR(("Write failed\n"));
				return -1;
			}
            /* try to read again */
			BDBG_ERR(("Try to read %s again\n", fn));
            if (b_estb_cfg_read_by_ctl(ctl) < 0) {
                return -1;
            }
		}
	}

	return 0;
}


/***************************************************************************
Summary:
Close a config file.


Description:
This function restore the default value under a directory by closing the current
config file. Currently it works for top dirctory only.
Input:
	char * dir - the directory to read to. Must be a top directory like "/perm"

Returns:
	>= 0 - if succeed
        < 0 - if fail
***************************************************************************/
int B_Estb_cfg_Close(char * dir){
	int ret = -1;
	int i = 0, min = -1, max = -1;

	if (b_estb_cfg_get_file_id(dir, &min, &max)< 0) {
		return ret;
	}

	if ( (min+1) != max) {
		BDBG_ERR(("can not close %s \n", dir));
		BDBG_ERR(("Available directories that can close\n"));
		for (i = 0; i < B_ESTB_CFG_NUM_CFG_FILES; i++) {
			BDBG_ERR(("%s  ", info_block[i].top_dir));
		}
		BDBG_ERR(("\n"));
		return -1;
	}
	for ( i = min; i <max; i++) {
		if (b_estb_cfg_init_by_id(i) < 0)
			return -1;
	}

	return 0;
}



/***************************************************************************
Summary:
Import a bin stream from a file

Description:
This function fills a bin type variable (e.g.  a certificate) with a bin file.

Input:
	char * field_name - the name of the bin variable (e.g. "/perm/estb/ca_cert")
	char * fn_name - the file that contains the raw bin data

Returns:
	>= 0 - if succeed
        < 0 - if fail
***************************************************************************/
int B_Estb_cfg_Import_bin_from_file(char * field_name, char * fn_name) {
	int ret = -1;
	unsigned int label;
	int i;
	int fd;
	int n = 0;
	int len = 0;
	struct b_estb_cfg_ctl * ctl = NULL;
	unsigned char * tmp_data = NULL;

	if ((ret = b_estb_cfg_get_label(field_name, &label)) >= 0) {

		if ( (i = b_estb_cfg_get_index_by_label(label)) < 0) {
			BDBG_ERR(("failed to get index for label %x\n", label));
			return -1;
		}
		if (b_estb_cfg_get_ctl_block_by_label(label, &ctl) < 0) {
			BDBG_ERR(("failed to get ctl block for label %x\n", label));
			return ret;
		}

	} else {
		BDBG_ERR(("failed to get label for %s\n", field_name));
		return -1;
	}

	BDBG_ERR(("label %x, index %d, fn_name %s\n", label, i, fn_name));
#if defined(_CFE_) || defined(NOOS)
	/* how are we going to import data in CFE? */
	xprintf("TBD: how are we going to import data in CFE? \n");
	return -1;
#else
	if ((fd = open(fn_name, O_RDONLY)) < 0) {
		BDBG_ERR(("Cannot read from %s\n",fn_name));
		return -1;
	}
	len = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	tmp_data = (unsigned char *) malloc(len);
	if (tmp_data == NULL) {
		BDBG_ERR(("%s malloc %d bytes fail\n", BSTD_FUNCTION, len));
		ret = -1;
		close(fd);
		return ret;
	}
	if ((n = read(fd, tmp_data, len)) != len) {
		BDBG_ERR(("read error, read %d, expect %d\n", n, len));
		ret = -1;
		free(tmp_data);
		close(fd);
		return ret;
	}
	close(fd);

	/* data is ready, adjust the buffer and copy it */
	if (b_estb_cfg_adjust_data_block(label, len, NODE_TYPE_BIN) < 0) {
		free(tmp_data);
		return ret;
	}
	memmove(ctl->data_block + ctl->node_block[i].v.data.off_w, tmp_data, len);
	free(tmp_data);

	/* BDBG_ERR(("offset %d, len %d, id %d\n", ctl->node_block[i].v.data.off_w, ctl->node_block[i].v.data.len, ctl->id)); */
	if ((ret = b_estb_cfg_write_by_ctl(ctl)) < 0) {
		BDBG_ERR(("Write failed\n"));
		return -1;
	}

	ret = 0;
	return ret;
#endif
}


/***************************************************************************
Summary:
Get the property of a variable.

Description:
This function retrieves variable property represented by name.

Input:
	char * name - the name as represented in the config file
                      like "/perm/estb/mac_addr"
        struct b_estb_cfg_property *prop - the property of the name

Returns:
	>= 0 - if succeed
        < 0 - if fail
***************************************************************************/
int B_Estb_cfg_Get_Property(char * name, struct b_estb_cfg_property * prop){
	int ret = -1;
	unsigned int label = 0;

	if ((ret = b_estb_cfg_get_label(name, &label)) < 0) {
		return ret;
	}


	if (b_estb_cfg_get_property_by_label(label, prop) < 0) {
		return ret;
	}
	return 0;
}
