/****************************************************************************
*
*  Copyright (c) 2011 Broadcom Corporation
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to
*  you under the terms of the GNU General Public License version 2 (the
*  "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php, with
*  the following added to such license:
*
*  As a special exception, the copyright holders of this software give you
*  permission to link this software with independent modules, and to copy
*  and distribute the resulting executable under terms of your choice,
*  provided that you also meet, for each linked independent module, the
*  terms and conditions of the license of that module.  An independent
*  module is a module which is not derived from this software.  The special
*  exception does not apply to any modifications of the software.
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
****************************************************************************/

#include <linux/version.h>
#if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,18)
#include <linux/config.h>
#endif
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/sched.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37)
#include <linux/mutex.h>
#include <linux/mount.h>
#include <linux/blkpg.h>
#endif

#include <linux/mtd/mtd.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37)
#include <linux/mtd/partitions.h>
#include <linux/mtd/map.h>
#else
#include <linux/mtd/compatmac.h>
#endif
#include <linux/kthread.h>   /* kthread */

#include <asm/uaccess.h>
#ifdef CONFIG_PROC_FS
#include <linux/proc_fs.h>
#endif

#ifdef CONFIG_MTD_BRCMNAND
#include <linux/vmalloc.h>
#if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,18)
#include <linux/mtd/brcmnand.h>
#else
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,37)
#include <../drivers/mtd/brcmnand/brcmnand.h>
#endif
#endif
#endif

#if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,18)
#include <asm/brcmstb/common/brcmstb.h>
#else
#include <asm/brcmstb/brcmstb.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,37)
#include <linux/smp_lock.h>
#endif
#endif

#define VERSION     "1.5"

#ifdef CONFIG_MTD_BRCMNAND
#define VER_STR     "v" VERSION "(NAND) " __DATE__ " " __TIME__
#else
#define VER_STR     "v" VERSION " " __DATE__ " " __TIME__
#endif
#ifndef MTD_READ
#define MTD_READ(mtd, args...) (*(mtd->read))(mtd, args)
#define MTD_WRITE(mtd, args...) (*(mtd->write))(mtd, args)
#endif

#define MTD_OCAP_MAJOR    98 /* not used by anyone */
#define OCAP_NONVOL_SEG_MAGIC 0x4d435242  /* magic number at the end of a segment */
#define OCAP_MAX_NONVOL_SIZE  0x20000  /* the size used by ocap, <= flash partition size */
#define TO_MTDOCAP_CTL(file) (struct ocap_struct *)((long)((file)->private_data) & ~3L)
#define TO_MTDOCAP(ctl) (struct mtd_info *)((struct ocap_struct *)(ctl))->mtd

#define DBG_INFO(arg...) if (debug) printk(arg)
#define DBG_WRN(arg...) printk(arg)
#define DBG_ERR(arg...) printk(arg)
#define SEG_EMPTY  -1
#define SEG_INVAL  -2
#define SEG_FULL    1
#define SEG_OK      0
//********************** Local Types *****************************************
#define SEEK_SET  0
#define SEEK_CUR  1
#define SEEK_END  2

static unsigned short debug = 1;
module_param(debug, ushort, S_IRUGO);

// The two sections that can be read/written.  Typically, Permanent settings
// are only written during manufacturing, but Dynamic settings can be written
// at any time during normal operation of the system.

#define PERM_OCAP_SECTION	0x01
#define DYN_OCAP_SECTION      0x00

#define MAX_MESSAGE_SIZE        1458
#define TFTP_BLOCK_SIZE         512

#define MAX_OCAP_DEV 7
#define OCAP_DEV_INVALID (MAX_OCAP_DEV +1)

#define OCAPDEV     "mtdocap"
#define N_RESERVED_BYTES 8 /* bytes reserved */

struct ocap_struct {
	/* fields updated by init */
	char name[64];
	int devnum;
	int erase_size;
	int size;
	int start;
	int physical_size;
	int minSegments;
	int dirty; /* cache is dirty, data should be read from cache */

#ifdef CONFIG_MTD_BRCMNAND
	int nand_buf_size;    /* nand flash has bad sectors, this size is the actual size used by ocap */
	uint8_t * nand_buf; /* nand flash is r/w in sectors, this buffer holds the data before flush to flash */
	int nand_buf_ready; /* the data in nand_buf is ready for use */
	int partition_erased; /* after driver loaded, before first write, need to erase the whole partition */
#endif
	uint8_t * buf;
	struct task_struct * flush_thread;
	int kill_thread;
	wait_queue_head_t wait;
	struct file_operations * fops;

	/* fields cleaned by each open/close */
	struct mtd_info * mtd;
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,17)
	enum mtd_file_modes mode;
#define MTD_MODE(file)		(((struct ocap_struct *)((file)->private_data))->mode)
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37)
	struct inode *ino;
#endif
	int pos;  /* offset in the segment, it increases with every read/write */

	/* fields cleand by flush thread */
	uint32_t options;
#define FLUSH    0x00000001

#define FLUSH_SET(ctr)  ((ctr)->options |= FLUSH)
#define FLUSH_CLEAN(ctr)  ((ctr)->options &= (~FLUSH))
#define FLUSH_CHECK(ctr)  ((ctr)->options & FLUSH)

};

#ifdef CONFIG_MTD_BRCMNAND
static uint8_t *nand_block_buf = NULL;
#endif

struct ocap_struct ocap_ctl[MAX_OCAP_DEV];

enum {
	mtdocap_dev_ecmboot,
	mtdocap_dev_dyncfg,
	mtdocap_dev_permcfg,
	mtdocap_dev_docsis0,
	mtdocap_dev_bootinfo,
	mtdocap_dev_app,
	mtdocap_dev_last
} mtdocap_dev_id;


struct ocap_partition {

	char name[32];
	char mtd_name[32];         /* mtd device name */
	int devnum;         /* mtd device number */
	int partition_size; /* partition size, assuming a 64M flash */
	int start;           /* sub-partition start addr, assuming a 64M flash */
	int physical_size;           /* sub-partition size , assuming a 64M flash*/
	int size;           /* actual size used, no matter how big the partition is */
	struct file_operations * fops;
};


#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37)
/*
 * Data structure to hold the pointer to the mtd device as well
 * as mode information ofr various use cases.
 */
struct mtd_file_info {
	struct mtd_info *mtd;
	struct inode *ino;
	enum mtd_file_modes mode;
};
#define MTD_INODE_FS_MAGIC 0x11307854
static struct vfsmount *mtd_inode_mnt __read_mostly;
#endif

static int mtdocap_open_normal(struct inode *inode, struct file *file);
static int mtdocap_close_normal(struct inode *inode, struct file *file);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37)
static long mtdocap_unlocked_ioctl_normal(struct file *file, u_int cmd, u_long arg);
#endif
static int mtdocap_ioctl_normal(struct inode *inode, struct file *file,u_int cmd, u_long arg);
static ssize_t mtdocap_write_normal(struct file *file, const char __user *buf, size_t count,loff_t *ppos);
static ssize_t mtdocap_read_normal(struct file *file, char __user *buf, size_t count,loff_t *ppos);
static loff_t mtdocap_llseek_normal(struct file *file, loff_t offset, int orig);


static int mtdocap_open_nonvol(struct inode *inode, struct file *file);
static int mtdocap_close_nonvol(struct inode *inode, struct file *file);
static ssize_t mtdocap_write_nonvol(struct file *file, const char __user *buf, size_t count,loff_t *ppos);
static ssize_t mtdocap_read_nonvol(struct file *file, char __user *buf, size_t count,loff_t *ppos);
static loff_t mtdocap_llseek_nonvol (struct file *file, loff_t offset, int orig);


/*
 * The physical size is based on a 64M flash system.
 * For 32M flash, the physical size is the same.
 * For bigger size flash, it is scaled by <flash size in MB> / 64
 */
static struct file_operations mtdocap_fops_normal = {
	.llseek		= mtdocap_llseek_normal,
	.read		= mtdocap_read_normal,
	.write		= mtdocap_write_normal,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37)
	.unlocked_ioctl = mtdocap_unlocked_ioctl_normal,
#else
	.ioctl		= mtdocap_ioctl_normal,
#endif
	.open		= mtdocap_open_normal,
	.release	= mtdocap_close_normal,
};
static struct file_operations mtdocap_fops_nonvol = {
	.llseek		= mtdocap_llseek_nonvol,
	.read		= mtdocap_read_nonvol,
	.write		= mtdocap_write_nonvol,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37)
	.unlocked_ioctl = mtdocap_unlocked_ioctl_normal,
#else
	.ioctl		= mtdocap_ioctl_normal, /* for erase */
#endif
	.open		= mtdocap_open_nonvol,
	.release	= mtdocap_close_nonvol
};
//#define MAX_OCAP_DEV 7
//struct ocap_struct ocap_ctl[MAX_OCAP_DEV];

#if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,18)
#ifdef CONFIG_MTD_NEW_PARTITION
/*
 * starting at 2.6.18-6.6, the (NAND) flash partition is changed, kernel provides the following paritions
 * mtd0: rootfs
 * mtd1: all flash less BBT0 (1MB) for flash <= 512MB
 * mtd2: Kernel (4MB)
 * mtd3: Data, for flash>512MB, from 512MB up to flash - BBT1 (4MB)
 * And CFE provides the following environment variable to mtdocap driver, indicating the start address
 * and offset of the flash area for mtdocap.
 * The mtd1 device is used.
 * OCAP_PART_STARTAD: start address of ocap flash area
 * OCAP_PART_SIZE: size of the ocap flash area
 *
 */
#define DEFAULT_AVAIL1_SIZE  0x02000000
#define DEFAULT_AVAIL1_OFFSET 0x00000000

#define DEFAULT_OCAP_SIZE    0x00600000
#define DEFAULT_OCAP_OFFSET ((DEFAULT_AVAIL1_OFFSET) + (DEFAULT_AVAIL1_SIZE) )

#define DEFAULT_TOTAL_OCAP_SIZE ((DEFAULT_OCAP_SIZE) + (DEFAULT_AVAIL1_SIZE))


struct ocap_partition  ocap_dev[MAX_OCAP_DEV] = {
	/* name, devnum, partition size, start, physical_size, size, fops */
	{"ecmboot", "all", OCAP_DEV_INVALID, DEFAULT_OCAP_SIZE, DEFAULT_OCAP_OFFSET + 0x00000000, 0x00080000, 0x00080000, &mtdocap_fops_normal},
	{"dyncfg", "all", OCAP_DEV_INVALID, DEFAULT_OCAP_SIZE, DEFAULT_OCAP_OFFSET + 0x00080000, 0x00080000, OCAP_MAX_NONVOL_SIZE, &mtdocap_fops_nonvol},
	{"permcfg", "all", OCAP_DEV_INVALID, DEFAULT_OCAP_SIZE, DEFAULT_OCAP_OFFSET + 0x00100000, 0x00080000, OCAP_MAX_NONVOL_SIZE, &mtdocap_fops_nonvol},
	{"docsis0", "all", OCAP_DEV_INVALID, DEFAULT_OCAP_SIZE, DEFAULT_OCAP_OFFSET + 0x00180000, 0x00400000, 0x00400000, &mtdocap_fops_normal},
	{"bootinfo", "all", OCAP_DEV_INVALID, DEFAULT_OCAP_SIZE, DEFAULT_OCAP_OFFSET + 0x00580000, 0x00080000, 0x00080000, &mtdocap_fops_normal},
	{"image0",  "all", OCAP_DEV_INVALID, DEFAULT_AVAIL1_SIZE, DEFAULT_AVAIL1_OFFSET, 0x01000000, 0x01000000, &mtdocap_fops_normal},
	{"image1",  "all", OCAP_DEV_INVALID, DEFAULT_AVAIL1_SIZE, DEFAULT_AVAIL1_OFFSET + 0x01000000, 0x01000000, 0x01000000, &mtdocap_fops_normal}
};
#else
#define DEFAULT_OCAP_SIZE    0x00600000
#define DEFAULT_AVAIL1_SIZE  0x02000000
struct ocap_partition  ocap_dev[MAX_OCAP_DEV] = {
	/* name, devnum, partition size, start, physical_size, size, fops */
	{"ecmboot", "ocap", OCAP_DEV_INVALID, DEFAULT_OCAP_SIZE, 0x00000000, 0x00080000, 0x00080000, &mtdocap_fops_normal},
	{"dyncfg", "ocap", OCAP_DEV_INVALID, DEFAULT_OCAP_SIZE, 0x00080000, 0x00080000, OCAP_MAX_NONVOL_SIZE, &mtdocap_fops_nonvol},
	{"permcfg", "ocap", OCAP_DEV_INVALID, DEFAULT_OCAP_SIZE, 0x00100000, 0x00080000, OCAP_MAX_NONVOL_SIZE, &mtdocap_fops_nonvol},
	{"docsis0", "ocap", OCAP_DEV_INVALID, DEFAULT_OCAP_SIZE, 0x00180000, 0x00400000, 0x00400000, &mtdocap_fops_normal},
	{"bootinfo", "ocap", OCAP_DEV_INVALID, DEFAULT_OCAP_SIZE, 0x00580000, 0x00080000, 0x00080000, &mtdocap_fops_normal},
	{"image0", "avail1", OCAP_DEV_INVALID, DEFAULT_AVAIL1_SIZE, 0x00000000, 0x01000000, 0x01000000, &mtdocap_fops_normal},
	{"image1", "avail1", OCAP_DEV_INVALID, DEFAULT_AVAIL1_SIZE, 0x01000000, 0x01000000, 0x01000000, &mtdocap_fops_normal}
};

#endif /* CONFIG_MTD_NEW_PARTITION */
#else
#define DEFAULT_AVAIL1_SIZE  0x02000000
#define DEFAULT_AVAIL1_OFFSET 0x00500000

#define DEFAULT_OCAP_SIZE    0x00600000
#define DEFAULT_OCAP_OFFSET ((DEFAULT_AVAIL1_OFFSET) + (DEFAULT_AVAIL1_SIZE) )

struct ocap_partition  ocap_dev[MAX_OCAP_DEV] = {
	/* name, devnum, partition size, start, physical_size, size, fops */
	{"ecmboot", "entire_device", OCAP_DEV_INVALID, DEFAULT_OCAP_SIZE, DEFAULT_OCAP_OFFSET + 0x00000000, 0x00080000, 0x00080000, &mtdocap_fops_normal},
	{"dyncfg", "entire_device", OCAP_DEV_INVALID, DEFAULT_OCAP_SIZE, DEFAULT_OCAP_OFFSET + 0x00080000, 0x00080000, OCAP_MAX_NONVOL_SIZE, &mtdocap_fops_nonvol},
	{"permcfg", "entire_device", OCAP_DEV_INVALID, DEFAULT_OCAP_SIZE, DEFAULT_OCAP_OFFSET + 0x00100000, 0x00080000, OCAP_MAX_NONVOL_SIZE, &mtdocap_fops_nonvol},
	{"docsis0", "entire_device", OCAP_DEV_INVALID, DEFAULT_OCAP_SIZE, DEFAULT_OCAP_OFFSET + 0x00180000, 0x00400000, 0x00400000, &mtdocap_fops_normal},
	{"bootinfo", "entire_device", OCAP_DEV_INVALID, DEFAULT_OCAP_SIZE, DEFAULT_OCAP_OFFSET + 0x00580000, 0x00080000, 0x00080000, &mtdocap_fops_normal},
	{"image0",  "entire_device", OCAP_DEV_INVALID, DEFAULT_AVAIL1_SIZE, DEFAULT_AVAIL1_OFFSET, 0x01000000, 0x01000000, &mtdocap_fops_normal},
	{"image1",  "entire_device", OCAP_DEV_INVALID, DEFAULT_AVAIL1_SIZE, DEFAULT_AVAIL1_OFFSET + 0x01000000, 0x01000000, 0x01000000, &mtdocap_fops_normal}
};
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37)
DEFINE_SEMAPHORE(flush_sema);
#else
DECLARE_MUTEX(flush_sema);
#endif


//********************** Local Variables *************************************


//********************** Local Functions *************************************
#ifdef CONFIG_MTD_BRCMNAND
int mtd_read_skip_badblock(struct ocap_struct * ctl, int from, size_t len,
	size_t *retlen, u_char *buf)
{
	int ret = 0;
	struct mtd_info * mtd = TO_MTDOCAP(ctl);
	int ofs, bs, bytes_read, to_read;
	int erasesize;
	int blockstart = 1;
	int badblock = 0;
#ifdef USE_WRITESIZE
	bs = mtd->writesize;
#else
	bs = mtd->oobblock;
#endif
	if (bs == 0 || mtd->block_isbad == NULL)
		return -EOPNOTSUPP;
	erasesize = mtd->erasesize;
	bytes_read = 0;
	for (ofs = ctl->start + from; ofs < ctl->start + ctl->physical_size; ofs+=bs) {
		if (blockstart != (ofs & (~erasesize + 1))) {
			blockstart = ofs & (~erasesize + 1);
			badblock = mtd->block_isbad(mtd, blockstart);
			if (badblock) {
				/* skip the whole erase sector */
				DBG_ERR("Bad block @0x%x  size 0x%x, skipped\n",
					(uint32_t)blockstart, (uint32_t)erasesize);
                ofs += (erasesize - bs);
				continue;
			}
		}
		if (badblock) {
			continue;
		}
		to_read = (bytes_read + bs > len) ? (len - bytes_read) : bs;
		/* DBG_INFO("ofs %x to_read %x, retlen %x, bytes_read %x, len %x\n",
			 (uint32_t)ofs, (uint32_t)to_read, (uint32_t)*retlen,
			 (uint32_t)bytes_read, (uint32_t)len); */
		if ((ret = MTD_READ(mtd, ofs, to_read, retlen,
				    buf + bytes_read)) < 0) {
			*retlen += bytes_read;
			DBG_ERR("Failed to read from NAND flash\n");
			return ret;
		}
		if (*retlen != to_read) {
			*retlen += bytes_read;
			DBG_ERR("Not all data are read from NAND flash, read %d, expect %d\n", *retlen, bs);
			ret = -EINVAL;
			return ret;
		}
		bytes_read += to_read;
		if (bytes_read >= len) break;
	}
	if (bytes_read != len) {
		*retlen = bytes_read;
		DBG_ERR("%s error reading NAND flash, read %x, expect %x\n", __FUNCTION__, (uint32_t)*retlen, (uint32_t)len);
		ret = -EINVAL;
		return ret;
	}
	*retlen = bytes_read;
	return ret;
}


int mtd_write_skip_badblock(struct ocap_struct * ctl, int from, size_t len,
	size_t *retlen, u_char *buf)
{
	int ret = 0;
	struct mtd_info * mtd = TO_MTDOCAP(ctl);
	int ofs, bs, bytes_written, to_write;
	int erasesize;
	int blockstart = 1;
	int badblock = 0;

#ifdef USE_WRITESIZE
	bs = mtd->writesize;
#else
	bs = mtd->oobblock;
#endif
	if (bs == 0 || mtd->block_isbad == NULL)
		return -EOPNOTSUPP;
	erasesize = mtd->erasesize;
	bytes_written = 0;
	for (ofs = ctl->start + from; ofs < ctl->start + ctl->physical_size; ofs+=bs) {
		if (blockstart != (ofs & (~erasesize + 1))) {
			blockstart = ofs & (~erasesize + 1);
			badblock = mtd->block_isbad(mtd, blockstart);
			if (badblock) {
				/* skip the whole erase sector */
				DBG_ERR("Bad block @0x%x  size 0x%x, skipped\n",
					(uint32_t)blockstart, (uint32_t)erasesize);
                ofs += (erasesize - bs);
				continue;
			}
		}
		if (badblock) {
			continue;
		}
		to_write = (bytes_written + bs > len) ? (len - bytes_written) : bs;
		/* DBG_INFO("ofs %x to_write %x, retlen %x, bytes_read %x, len %x\n",
			 (uint32_t)ofs, (uint32_t)to_write, (uint32_t)*retlen,
			 (uint32_t)bytes_written,
			 (uint32_t)len); */
		/* padding */
		if (to_write != bs) {
			/*
			 * Should we just let the writing fail if it's not aligned?
			 * As padding is valid only at the end of the file. Padding could corrupt data
			 * if the write is not the end of the file.
			 *
			 */
			if (nand_block_buf == NULL) {
				nand_block_buf = kmalloc(bs, GFP_KERNEL);
			}
			if (nand_block_buf == NULL) {
				DBG_ERR("malloc nand_block_buf failed,  out of memory!\n");
				*retlen += bytes_written;
				DBG_ERR("Failed to write to NAND flash\n");
				ret = -ENOMEM;
				return ret;
			}

			memset(nand_block_buf, 0xff, bs);
			memcpy(nand_block_buf, (void *)buf + bytes_written, to_write);
			DBG_WRN("Padding %d bytes at 0x%x\n", (uint32_t)(bs - to_write), (uint32_t)(ofs + to_write));
			/* write must be page aligned */
			if ((ret = MTD_WRITE(mtd, ofs, bs, retlen,
					     nand_block_buf)) < 0) {
				*retlen += bytes_written;
				DBG_ERR("Failed to write to NAND flash\n");
				return ret;
			}
		} else {
			if ((ret = MTD_WRITE(mtd, ofs, bs, retlen,
					     buf + bytes_written)) < 0) {
				*retlen += bytes_written;
				DBG_ERR("Failed to write to NAND flash\n");
				return ret;
			}
		}
		if (*retlen != bs) {
			*retlen += bytes_written;
			DBG_ERR("Not all data are written to NAND flash, write %d, expect %d\n", *retlen, bs);
			ret = -EINVAL;
			return ret;
		}
		/* do not count padding */
		bytes_written += to_write;
		if (bytes_written >= len) break;
	}
	if (bytes_written != len) {
		*retlen = bytes_written;
		DBG_ERR("%s error reading NAND flash, write 0x%x, expect 0x%x\n", __FUNCTION__,
			(uint32_t)*retlen, (uint32_t)len);
		ret = -EINVAL;
		return ret;
	}
	*retlen = bytes_written;
	return ret;
}
#endif /* CONFIG_MTD_BRCMNAND */

static int mtdocap_fill_buf(struct ocap_struct * ctl, int force ) {
    int ret = 0;

#ifdef CONFIG_MTD_BRCMNAND
    if (ctl->nand_buf_ready == 0 || force) {
	size_t retlen=0;

	ret = mtd_read_skip_badblock(ctl, 0, ctl->nand_buf_size, &retlen, ctl->nand_buf);
	if (ret < 0) {
		return ret;
	}
	if (retlen != ctl->nand_buf_size) {
		DBG_ERR("Something wrong while reading NAND flash, read %d, expect %d\n", retlen, ctl->nand_buf_size);
		ret = -EINVAL;
		return ret;
	}
	ctl->nand_buf_ready = 1;
    }
#endif
    return ret;
}

static int mtd_read_from_buf(struct ocap_struct * ctl, loff_t from, size_t len,
			 size_t *retlen, u_char *buf) {
	int ret = 0;
#ifdef CONFIG_MTD_BRCMNAND
	/* nand flash requires r/w in sectors, so random r/w will actually access buffered data */
	*retlen = 0;
	if (ctl->nand_buf_ready) {
		if (from > ctl->nand_buf_size) {
			return -EINVAL;
		}
		*retlen = ((from + len) <= ctl->nand_buf_size) ? len : (ctl->nand_buf_size - from);
		memcpy(buf, ctl->nand_buf + from, *retlen);
	} else {
		DBG_ERR("data not ready\n");
		return -EINVAL;
	}
#else
	struct mtd_info * mtd = TO_MTDOCAP(ctl);
	ret = MTD_READ(mtd, ctl->start + from, len, retlen, buf);
#endif
	return ret;
}

static int mtd_write_to_buf(struct ocap_struct * ctl, loff_t from, size_t len,
			 size_t *retlen, const u_char *buf) {
	int ret = 0;
#ifdef CONFIG_MTD_BRCMNAND
	/* nand flash requires r/w in sectors, so random r/w will actually access buffered data */
	*retlen = 0;
	if (ctl->nand_buf_ready) {
		if (from > ctl->nand_buf_size) {
			return -EINVAL;
		}
		*retlen = ((from + len) <= ctl->nand_buf_size) ? len : ((from + len) - ctl->nand_buf_size );
		memcpy(ctl->nand_buf + from, buf, *retlen);
	} else {
		DBG_ERR("data not ready\n");
		return -EINVAL;
	}
#else
	struct mtd_info * mtd = TO_MTDOCAP(ctl);
	ret = MTD_WRITE(mtd, ctl->start + from, len, retlen, buf);
#endif

	return ret;
}
/*
 * This is the function that actually write to nand flash,
 * the offset and len must at least aligned to page boundary of MTD device,
 * error will be reported by MTD_WRITE
 */
static int FlashDriverFlush(struct ocap_struct * ctl, int offset, uint32_t len) {
	int ret = 0;
#ifdef CONFIG_MTD_BRCMNAND
	size_t retlen=0;

	DBG_INFO("flush 0x%x bytes to 0x%x\n", len, offset);
	if ((ret = mtd_write_skip_badblock(ctl, offset, len, &retlen, ctl->nand_buf + offset)) < 0) {
		DBG_ERR("flash write failed\n");
	}
#endif
	return ret;
}


static uint32_t FlashDriverRead32(struct ocap_struct * ctl, int offset)
{
	uint32_t val;
	size_t retlen=0;

	if (mtd_read_from_buf(ctl, offset, 4, &retlen, (uint8_t *)&val) < 0) {
	    DBG_INFO("FlashDriverRead32 failed at 0x%x\n", offset);
	    val =  0xffffffff;
	}
	/* DBG_INFO("FlashDriverRead32 0x%x at 0x%x\n", val, offset); */
	return val;
}

static int FlashDriverWrite32(struct ocap_struct * ctl, int offset, uint32_t val)
{
	int ret;
	size_t retlen=0;

	if ((ret = mtd_write_to_buf(ctl, offset, 4, &retlen, (uint8_t *)&val)) < 0) {
	    DBG_INFO("FlashDriverWrite32 failed at 0x%x, ret %d\n", offset, ret);
	}
	/* DBG_INFO("FlashDriverWrite32 0x%x at 0x%x\n", val, offset); */
	return retlen;
}

// Reads the 2 bytes segment size from the control word.

// Note that this can return 0 if ocap has not yet been initialized or if
// the control word has been corrupted.
//
// Parameters:
//      section - the section to query.
//
// Returns:
//      The segment size for the section of ocap.
//
static uint32_t ReadTrueSegmentSizeBytes(struct ocap_struct * ptr)
{
	uint32_t segmentSize, minSegmentSize, maxSegmentSize;
	int size, minSegments;
	uint32_t magic;

	size = ptr->size;
	minSegments = ptr->minSegments;

	// The segment size (in the control word) is 8 bytes from the beginning.
	segmentSize = FlashDriverRead32(ptr, 8);

	if (segmentSize > size)
	    goto err;
	magic = FlashDriverRead32(ptr, segmentSize - N_RESERVED_BYTES);
	if (magic != OCAP_NONVOL_SEG_MAGIC)
		goto err;
	// Calculate the minimum size that is allowed for a segment.  Since we only
	// have 16 bits to mark segment usage, the minimum size is 1/16th of the
	// region size.
	minSegmentSize =  size / 16;

	// Calculate the maximum size that is allowed for a segment, which depends
	// on the minimum number of segments that we must have in ocap.
	//
	// PR2292 - we now keep track of permanent and dynamic separately.
	maxSegmentSize = size / minSegments;

	//DBG_INFO("segmentSize = %d; minSegmentSize = %d; maxSegmentSize = %d\n",
	//              segmentSize, minSegmentSize, maxSegmentSize);

	// Check the value returned - it is possible that the flash block was erased
	// or otherwise corrupted.
	if ((segmentSize > maxSegmentSize) || (segmentSize < minSegmentSize)) {
		DBG_INFO("Segment Size in segment control word not valid; returning 0!\n");
		segmentSize = 0;
	}

	return segmentSize;
 err:
	DBG_ERR("invalid segment size %d\n", segmentSize);
	segmentSize = 0;
	return segmentSize;
}



static int FlashDriverReadArray(struct ocap_struct * ctl, uint32_t offset, uint8_t * pBuffer,
				uint32_t numberOfBytes)
{
	int ret;
	size_t retlen=0;

	if ( (ret = mtd_read_from_buf(ctl, offset, numberOfBytes, &retlen, (uint8_t *)pBuffer)) < 0) {
	    DBG_INFO("FlashDriverReadArray failed, num bytes 0x%x at 0x%x\n", numberOfBytes, offset);
	    retlen = ret;
	}
	return retlen;
}

static int FlashDriverWriteArray(struct ocap_struct * ctl, uint32_t offset, const uint8_t * pBuffer,
				 uint32_t numberOfBytes)
{
	int ret;
	size_t retlen=0;

	if ((ret = mtd_write_to_buf(ctl, offset, numberOfBytes, &retlen, pBuffer)) < 0) {
	    DBG_INFO("FlashDriverWriteArray failed, num of bytes 0x%x at 0x%x\n", numberOfBytes, offset);
	}
	return retlen;
}

static int memunlock(struct mtd_info * mtd, struct erase_info * erase) {
	int ret;
	if (!mtd->unlock)
		ret = -EOPNOTSUPP;
	else
		ret = mtd->unlock(mtd, erase->addr, erase->len);
	return ret;
}

static void mtdocap_erase_callback (struct erase_info *instr)
{
	wake_up((wait_queue_head_t *)instr->priv);
}

static int memerase(struct mtd_info * mtd, struct erase_info *erase) {
	int ret = 0;

	wait_queue_head_t waitq;
	DECLARE_WAITQUEUE(wait, current);

	init_waitqueue_head(&waitq);

	erase->mtd = mtd;
	erase->callback = mtdocap_erase_callback;
	erase->priv = (unsigned long)&waitq;

	/*
	  FIXME: Allow INTERRUPTIBLE. Which means
	  not having the wait_queue head on the stack.

	  If the wq_head is on the stack, and we
	  leave because we got interrupted, then the
	  wq_head is no longer there when the
	  callback routine tries to wake us up.
	*/
	ret = mtd->erase(mtd, erase);
	if (!ret) {
		set_current_state(TASK_UNINTERRUPTIBLE);
		add_wait_queue(&waitq, &wait);
		if (erase->state != MTD_ERASE_DONE &&
		    erase->state != MTD_ERASE_FAILED)
			schedule();
		remove_wait_queue(&waitq, &wait);
		set_current_state(TASK_RUNNING);

		ret = (erase->state == MTD_ERASE_FAILED)?-EIO:0;
	}
	return ret;
}

static int region_erase(struct mtd_info * mtd, int start, int count, int unlock, int regcount)
{
	int i, j;
	struct mtd_erase_region_info * reginfo;
	int ret = 0;

	reginfo = kmalloc((regcount * sizeof(struct region_info_user)), GFP_KERNEL);

	for(i = 0; i < regcount; i++)
	{
		memcpy(&reginfo[i], &(mtd->eraseregions[i]), sizeof(struct mtd_erase_region_info));
	}

	// We have all the information about the chip we need.

	for(i = 0; i < regcount; i++)
	{ //Loop through the regions
		struct mtd_erase_region_info * r = &(reginfo[i]);

		if((start >= reginfo[i].offset) &&
				(start < (r->offset + r->numblocks*r->erasesize)))
			break;
	}

	if(i >= regcount)
	{
		DBG_ERR("Starting offset %x not within chip.\n", start);
		return 8;
	}

	//We are now positioned within region i of the chip, so start erasing
	//count sectors from there.

	for(j = 0; (j < count)&&(i < regcount); j++)
	{
		struct erase_info erase;
		struct mtd_erase_region_info * r = &(reginfo[i]);

		erase.addr = start;
		erase.len = r->erasesize;

		if(unlock != 0)
		{ //Unlock the sector first.
			if ( memunlock(mtd, &erase) != 0) {
				DBG_ERR("\nMTD Unlock failure");
				return 8;
			}


		}
		DBG_INFO("region_erase\n");
#if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,18)
		DBG_INFO("\rPerforming Flash Erase of length %u at offset 0x%llx", erase.len, erase.addr);
#else
		DBG_INFO("\rPerforming Flash Erase of length %llx at offset 0x%llx", erase.len, erase.addr);
#endif
		if((ret = memerase(mtd, &erase)) != 0)
		{
			DBG_ERR("\nMTD Erase failure");
			return ret;
		}


		start += erase.len;
		if(start >= (r->offset + r->numblocks*r->erasesize))
		{ //We finished region i so move to region i+1
			DBG_INFO("\nMoving to region %d\n", i+1);
			i++;
		}
	}

	DBG_INFO(" done\n");

	return ret;
}

static int non_region_erase(struct mtd_info * mtd, int start, int count, int unlock)
{
	struct erase_info erase;
	int ret = 0;

	erase.addr = start;

	erase.len = mtd->erasesize;

	for (; count > 0; count--) {
		/*
		DBG_INFO("non_region_erase\n");
		DBG_INFO("\rPerforming Flash Erase of length %u at offset 0x%x",
			 erase.len, erase.addr);
		*/
		if(unlock != 0)
		{
			//Unlock the sector first.
			/* DBG_INFO("\rPerforming Flash unlock at offset 0x%x",erase.addr); */
			if ( memunlock(mtd, &erase) != 0) {
				DBG_ERR("\nMTD Unlock failure");
				return 8;
			}
		}
		if((ret = memerase(mtd, &erase)) != 0) {
			DBG_ERR("\nMTD Erase failure");
			return ret;
		}
		erase.addr += mtd->erasesize;
	}
	/*DBG_INFO(" done\n");*/
	return ret;
}

#ifdef CONFIG_MTD_BRCMNAND
/*
 * erase the region, (count) blocks, skip bad block when encountered.
 * return < 0 if cannot erase (count) blocks block.
 * >= 0 if succeed.
 */
static int FlashDriverEraseRegion(struct ocap_struct * ctl, int start, int count)
{
	int ret = 0;
	int regcount;
	int unlock = 0;
    int ofs;
	int erasesize;
	int blockstart = 1;
	int badblock = 0;
    int blocks_erased = 0;
	struct mtd_info * mtd = TO_MTDOCAP(ctl);

	if (start % ctl->erase_size) {
		DBG_ERR("Erase failure, start address %x not aligned to erase size %d", start, ctl->erase_size);
		return -1;
	}
    erasesize = ctl->erase_size;
	for (ofs = ctl->start + start; ofs < ctl->start + ctl->physical_size; ofs+=erasesize) {
        DBG_INFO("ctl->start %x, start %x, ofs %x, blockstart %x, blocks_erased %x\n",
                 ctl->start, start, ofs, blockstart, blocks_erased);
        if (blocks_erased >= count) break;
		if (blockstart != (ofs & (~erasesize + 1))) {
			blockstart = ofs & (~erasesize + 1);
			badblock = mtd->block_isbad(mtd, blockstart);
			if (badblock) {
				/* skip the whole erase sector */
				DBG_ERR("Bad block @0x%x  size 0x%x, skipped\n",
					(uint32_t)blockstart, (uint32_t)erasesize);
				continue;
            }
            regcount = mtd->numeraseregions;
            if(regcount == 0) {
                ret = non_region_erase(mtd, ofs, 1, unlock);
            }
            else {
                ret = region_erase(mtd, ofs, 1, unlock, regcount);
            }
            if (ret >= 0) {
                blocks_erased++;
            }
		}
	}
    if (blocks_erased < count) {
        /* too many bad blocks. */
        DBG_ERR("Erased %d blocks, expect %d blocks, ret %d. Maybe too many bad blocks, continue anyway", blocks_erased, count, ret);
    }
	return ret;
}
#else
static int FlashDriverEraseRegion(struct ocap_struct * ctl, int start, int count)
{
	int ret = 0;
	int regcount;
	int unlock = 0;
	struct mtd_info * mtd = TO_MTDOCAP(ctl);

	if (start % ctl->erase_size) {
		DBG_ERR("Erase failure, start address %x not aligned to erase size %d", start, ctl->erase_size);
		return -1;
	}
	regcount = mtd->numeraseregions;
	if(regcount == 0) {
		ret = non_region_erase(mtd, ctl->start + start, count, unlock);
	}
	else {
		ret = region_erase(mtd, ctl->start + start, count, unlock, regcount);
	}
	return ret;
}
#endif /* CONFIG_MTD_BRCMNAND */

/*
 * Get the start address of the current segment
 * return SEG_EMPTY if all the segments are empty, likely the flash has just been erased
 *        SEG_FULL if all segments are used. the last segment is still valid.
 *        SEG_INVAL if segment invalid;
 *        SEG_OK if succeed.
 * The region start address is in regionOffset;
 */
static int GetCurrentRegion(struct ocap_struct * ctl, unsigned int segSize,
			    unsigned int * regionOffset) {
	unsigned int offset;
	int size = ctl->size;
	int ret = -SEG_EMPTY;
	unsigned int magic = 0;

	if (segSize == 0) {
		DBG_ERR("Invalid segment size (%d)!  There is no valid data to read.\n", segSize);
		*regionOffset = 0;
		return SEG_EMPTY;
	}

	for (offset = segSize; offset <= (size - segSize); offset += segSize) {
		magic = FlashDriverRead32(ctl, offset + segSize - N_RESERVED_BYTES);
		if (magic != OCAP_NONVOL_SEG_MAGIC)
			break;
	}
	if (magic != 0xffffffff && magic != OCAP_NONVOL_SEG_MAGIC) {
		DBG_ERR("Invalid Segment(offset 0x%x, size 0x%x). There is no valid data to read.\n", offset , size);
		*regionOffset = offset;
		ret = SEG_INVAL;
		return ret;
	}
	if (offset == segSize) {
		DBG_ERR("Segment empty (offset 0x%x, size 0x%x). There is no valid data to read.\n", offset, size);
		*regionOffset = offset;
		ret = SEG_EMPTY;
	} else if (offset > (size - segSize)) {
		DBG_ERR("Segment full (offset 0x%x, size 0x%x).\n", offset, size);
		*regionOffset = (offset - segSize);
		ret = SEG_FULL;
	} else {
		*regionOffset = (offset - segSize);
		ret = SEG_OK;
	}

	return ret;
}

// Reads a segment of ocap data from the specified section in flash memory to
// the buffer specified.  It will read the smaller of the current segment size
// or the specified buffer size.  It always reads the most recently written
// segment, as specified by the segment bitmask in the control word.
//
// There is no indication as to the number of bytes of valid data that were
// previously written to the segment, so there is no indication of how many
// valid bytes were read.  The client must embed this information in the data
// if it is desired.
//
static int OcapDriverRead(struct ocap_struct * ctl, uint8_t *pBuffer, loff_t offset,
			    unsigned int bufferSize)
{
	int result = 0;
    unsigned int segmentSizeBytes;
    unsigned int regionOffset;
    int size, erase_size;

    size = ctl->size;
    erase_size = ctl->erase_size;

    // Query the current segment size.  This has to be done before we open the
    // driver in this function because the driver is opened in
    // ReadTrueSegmentSizeBytes(), and we can't open the driver twice from
    // the same thread.
    segmentSizeBytes = ReadTrueSegmentSizeBytes(ctl);

    if (segmentSizeBytes == 0)
    {
        DBG_INFO( "Failed to query the segment size (returned 0 bytes)!  There is no valid data to read.\n");

        return -EFAULT;
    }

    if (GetCurrentRegion(ctl, segmentSizeBytes, &regionOffset) < 0) {
	    return -EFAULT;
    }
    // Calculate the minimum size to copy, which is either the buffer size or
    // the segment size, whichever is smaller.
    if ((segmentSizeBytes) < bufferSize)
    {
        bufferSize = segmentSizeBytes;
    }

    // Make sure we aren't trying to read beyond the end of the flash region.
    // This will generally only happen if the segment bitmask is invalid or if
    // ocap hasn't been written yet.

    offset += regionOffset;

    if ((offset + bufferSize) <= size)
    {

	    if ( (result = FlashDriverReadArray(ctl, offset, pBuffer, bufferSize)) >= 0) {
		    // DBG_INFO("%d bytes read from flash\n", result);
	    } else {
		    DBG_ERR("Failed to read from ocap\n");
		    result = -EFAULT;
	    }
    }
    else
    {
        DBG_INFO("%s Attempted to read outside bounds of flash memory region!\n"
	       "This usually means the control word was corrupted or not initialized!", ctl->name);

        result = -EFAULT;
    }

    return result;
}


// Writes the buffer of data to the specified section in flash memory.  If
// possible, it will simply append the data as the next segment.  If the flash
// block is full (no segments available), or if the numberOfBytes is larger than
// the current segment size, then it will erase the flash block and write the
// segment.
//
// After the data is written, it will verify the data, retrying the write as
// needed.
//
// Parameters:
//      section - the section to write.
//      pBuffer - pointer to a buffer of data to be written to the device.
//      numberOfBytes - the number of bytes to write.
//
// Returns:
//      0 if the data was written successfully.
//      -1 if the data could not be written correctly.
//
static int OcapDriverWrite(struct ocap_struct * ctl, const uint8_t *pBuffer,
			    uint32_t numberOfBytes)
{
	int result = 0;
	unsigned int segmentSizeBytes;

	unsigned int regionOffset;

	unsigned int eraseRegion = 0;
	unsigned int regionErased;
	int size, erase_size, minSegments;

	size = ctl->size;
	erase_size = ctl->erase_size;
	minSegments = ctl->minSegments;

	// Error check - make sure the number of bytes is less than the max
	// segment size.  If it's greater, then we can't possibly write it.
	if ((numberOfBytes)> size ) {
		DBG_INFO("Too much data to fit in ocap!  Buffer size %u > max segment size %d!\n",
			 (numberOfBytes), size);

		return -EFAULT;
	}


	// Query the current segment size.  This has to be done before we open the
	// driver in this function because the driver is opened in
	// ReadTrueSegmentSizeBytes(), and we can't open the driver twice from
	// the same thread.
	segmentSizeBytes = ReadTrueSegmentSizeBytes(ctl);

	// See if we need to increase the segment size.
	if (segmentSizeBytes < (numberOfBytes  + N_RESERVED_BYTES)) {
		DBG_INFO("Current segment size %u < buffer size %u !  Increasing the segment size!\n",
			 segmentSizeBytes, (numberOfBytes + N_RESERVED_BYTES) );

		// Set this so that we will erase the flash region the first time
		// through, which will cause the new segmentSize to be written.
		eraseRegion = 1;
	}
	/* erase the region if the segments are full */
	result = GetCurrentRegion(ctl, segmentSizeBytes, &regionOffset);
	if (result == SEG_FULL || result == SEG_INVAL) {
		eraseRegion = 1;
	}

#ifdef CONFIG_MTD_BRCMNAND
	/*
	 * Erase the region if this is the first write after insmod.
	 * NAND flash does not allow to write twice after each erase, even write 0xff.
	 * Since the content in flash could be flashed from CFE, and we have no way to know it,
	 * we will force erase the nonvol flash partition after insmod.
	 */
	if (ctl->partition_erased == 0) {
		eraseRegion = 1;
		ctl->partition_erased = 1;
	}

#endif

	// Keep going until we succeed, or we try too many times.
	do {
		regionErased = 0;


		// If we need to erase the flash region, then do so, and write the
		// segment size.
		if (eraseRegion) {
			unsigned int regionDivisor;

			eraseRegion = 0;
			regionErased = 1;

			// Recalculate the segment size so that it is optimal for the amount
			// of data that is going to be written, using a multiple of the
			// total size (region size divided by 8, 7, 6, etc) so that we get
			// maximal use of the segments.
			//
			// Start with the smallest possible segment, looking for the
			// smallest size that will satisfy the number of bytes that are
			// needed.  I know that we will find at least one possible segment
			// that is large enough because of the error checking at the top of
			// the function.
			//
			for (regionDivisor = 16; regionDivisor >= minSegments; regionDivisor /= 2) {
				// Divide the ocap region into the number of segments for the
				// current iteration.
				segmentSizeBytes = size / regionDivisor;

				// Make sure this is multiple of 4.  Since the segments are
				// appended to each other, one segment would start on an even
				// byte boundary, then the next would start on an odd boundary,
				// etc.  Given the way we use the flash driver now, this really
				// isn't a problem, but if we start reading uint16_t or uint32_t
				// values directly in the future then it would be a problem.
				segmentSizeBytes -= segmentSizeBytes & 0x03;

				// If the number of bytes to be written is less than or equal
				// the segment size
				// then select this one.
				if ((segmentSizeBytes) >= ( numberOfBytes + N_RESERVED_BYTES)) {
					break;
				}
			}

			DBG_INFO("Recalculated segmentSizeBytes = %d\n", segmentSizeBytes);

			DBG_INFO("Erasing flash region; a power failure here will cause settings to be lost!\n");

			// Erase the region so that we can start over.
			result = FlashDriverEraseRegion(ctl, 0, ctl->physical_size / erase_size);

			if (result < 0)  {
				DBG_INFO("Failed to erase the flash region!\n");
				break;
			}
			/* reload the data from flash */
			mtdocap_fill_buf(ctl, 1);

			// Write the segment size to the control word.  This goes 8 bytes
			// from the beginning, since NAND requires sequencial writing.
			FlashDriverWrite32(ctl, 8, segmentSizeBytes);
			FlashDriverWrite32(ctl, segmentSizeBytes - N_RESERVED_BYTES, OCAP_NONVOL_SEG_MAGIC);
			if (FlashDriverFlush(ctl, 0, segmentSizeBytes) < 0) {
				DBG_INFO("Failed to write the segment size to the control word!\n");
				break;
			}
		}

		result = GetCurrentRegion(ctl, segmentSizeBytes, &regionOffset);
		DBG_INFO("segment size = %d\n", segmentSizeBytes);

		if (result == SEG_EMPTY) {
			regionOffset = segmentSizeBytes;
		} else if (result == SEG_OK) {
			regionOffset += segmentSizeBytes;
		} else {
			eraseRegion = 1;
			continue;
		}

		// Write the data.
		result = FlashDriverWriteArray(ctl, regionOffset, pBuffer, numberOfBytes);
		if (result < 0)    {
			DBG_ERR("Failed to write the settings data!\n");
			break;
		}
		// Write the segment magic number.
		result = FlashDriverWrite32(ctl, regionOffset + segmentSizeBytes - N_RESERVED_BYTES,
					    OCAP_NONVOL_SEG_MAGIC);
		if (result < 0 )   {
			DBG_ERR("Failed to write the segment magic!\n");
			break;
		}
		if (FlashDriverFlush(ctl, regionOffset, segmentSizeBytes) < 0) {
			DBG_ERR("flash write failed\n");
			break;
		} else   {
			// Get a pointer to this offset for use in memcmp.

			// PR2439 - need to specify uncached.
			uint8_t * tmpbuf = kmalloc(numberOfBytes, GFP_KERNEL);
			if (!tmpbuf) {
				DBG_ERR("out of memory\n");
				result = -1;
				break;
			}

			/* reload the data from flash */
			mtdocap_fill_buf(ctl, 1);

			result = FlashDriverReadArray(ctl, regionOffset, tmpbuf, numberOfBytes);

			if (result < 0) {
				DBG_ERR("Failed to read from ocap\n");
				result = -EFAULT;
				kfree(tmpbuf);
				break;

			}

			// Compare the data.
			if (memcmp(pBuffer, tmpbuf, numberOfBytes) != 0) {
				DBG_ERR("Failed to verify the settings data!\n");
				result = -EFAULT;
				break;
			} else   {
				DBG_INFO("%s: Write %d at %d succeeded\n", ctl->name, numberOfBytes, regionOffset);
			}
			kfree(tmpbuf);
		}

	} while (eraseRegion);

	return result;
}


static int flush_func(void * arg) {
	struct ocap_struct *ctl = (struct ocap_struct *) arg;
	uint8_t * buf;
	int actual_size;

	while (1) {
		wait_event_interruptible_timeout(ctl->wait, FLUSH_CHECK(ctl), 200);
		down_interruptible(&flush_sema);
		buf = ctl->buf;
		actual_size = ctl->pos;
		if (FLUSH_CHECK(ctl)) {
			DBG_INFO("Flushing to flash, size %d\n", actual_size);
			if (OcapDriverWrite(ctl, buf, actual_size) < 0) {
				DBG_ERR ("ocap: flash write failed!\n");
				up(&flush_sema);
				break;
			}
			FLUSH_CLEAN(ctl);
			ctl->pos = 0; /* reset position */
			ctl->dirty = 0;

		}
		up(&flush_sema);
		if (kthread_should_stop())
			break;
		//set_current_state(TASK_INTERRUPTIBLE);
		//schedule_timeout(300);
	}
	return 0;

}


static int OcapDriverFileSize(struct ocap_struct * ctl) {
	int len;
	if (ctl->dirty) {
		/* data has not flushed yet, read from mem */
		len = *(int*)(ctl->buf);
	} else {
		if (OcapDriverRead(ctl, (uint8_t *) &len, 0, 4) < 0) {
			/* no valid data in the flash, the file size is 0*/
			len = 0;
		}
	}
	len = ntohl(len);
	if (len < 0)  len = 0; /* empty file */
	DBG_INFO("%s file size %d\n", ctl->name, len);
	return len;
}

static loff_t mtdocap_llseek_nonvol (struct file *file, loff_t offset, int orig)
{
	struct ocap_struct * ctl = (struct ocap_struct *)file->private_data;
	loff_t ret = 0;
	int data_size = OcapDriverFileSize(ctl);

	if (data_size == 0) {
		/* data has been written to buffer, but not flushed to flash yet */
		data_size = ctl->pos;
	}
	down_interruptible(&flush_sema);
	switch (orig) {
	case 0: /*SEEK_SET */
		break;
	case 1: /* SEEK_CUR */
		offset += ctl->pos;
		break;
	case 2: /* SEEK_END */
		offset += data_size;
		break;
	default:
		ret = -EFAULT;
	}
	if ((offset >= 0) && (offset <= data_size) && (ret >=0)) {
		ctl->pos = offset;
		ret = offset;
	} else {
		DBG_INFO("%s lseek error! pos %d, max %d, ret %lld\n", ctl->name, ctl->pos, ctl->size - 8, ret);
		ret = -EFAULT;
	}
	DBG_INFO("%s: lseek pos %d data_size %d\n", ctl->name, ctl->pos, data_size);
	up(&flush_sema);
	return ret;
}

static int mtdocap_open_nonvol(struct inode *inode, struct file *file)
{
	struct mtd_info *mtd;
	struct ocap_struct * ctl;
	unsigned int minor = iminor(inode);
	int ret = 0;

	DBG_INFO( "%s %u\n", __FUNCTION__, minor);
	if (minor >= MAX_OCAP_DEV)
		return -ENODEV;
	ctl = &ocap_ctl[minor];

	/* You can't open the RO devices RW */
	/*
	if ((file->f_mode & 2) && (minor & 1))
		return -EACCES;
	*/
	mtd = get_mtd_device(NULL, ctl->devnum);

	if (!mtd) {
		put_mtd_device(mtd);
		ret =  -ENODEV;
		goto err;
	}
	if (MTD_ABSENT == mtd->type) {
		put_mtd_device(mtd);
		ret =  -ENODEV;
		goto err;
	}


	down_interruptible(&flush_sema);
	ctl->mtd = mtd;
	ctl->pos = 0;
	ctl->dirty = 0;
	up(&flush_sema);
	file->private_data = ctl;

	if ((ret = mtdocap_fill_buf(ctl, 1)) < 0) {
		return ret;
	}

	return 0;
 err:
	return ret;
}

/*====================================================================*/

static int mtdocap_close_nonvol(struct inode *inode, struct file *file)
{
	struct ocap_struct * ctl ;
	struct mtd_info *mtd;

	ctl = TO_MTDOCAP_CTL(file);
	DBG_INFO("%s: MTD_close\n", ctl->name);
	mtd = TO_MTDOCAP(ctl);
	ctl->dirty = 0;

	if (mtd->sync)
		mtd->sync(mtd);
	put_mtd_device(mtd);

	return 0;
}

static ssize_t mtdocap_read_nonvol(struct file *file, char __user *buf, size_t count,loff_t *ppos)
{
	int ret=0;
	struct ocap_struct * ctl = TO_MTDOCAP_CTL(file);
	int len = 0;
	DBG_INFO( "%s\n", __FUNCTION__);

#ifdef CONFIG_MTD_BRCMNAND
	/*
	 * read the whole nand buffer, this is not efficient. But in cases where
	 * other utilities can modify the flash content (nandwrite, eraseall, etc),
	 * this seems to be the  only way to guarantee data consistency.
	 * In production, force should be set to 0
	 */
	if ((ret = mtdocap_fill_buf(ctl, 1)) < 0) {
		return ret;
	}
#endif

	len = OcapDriverFileSize(ctl);
	if (len < 0) {
		DBG_ERR("read error %d\n", len);
		return ret;
	}

	if (len <= ctl->pos)
		return 0;

	count = (len < count) ? len : count;
	if ((ret = OcapDriverRead(ctl, ctl->buf, ctl->pos, count)) < 0) {
		DBG_ERR("read error %d\n", ret);
		return ret;
	} else {
		if (copy_to_user(buf, ctl->buf, ret)) {
			ret = -EFAULT;
		} else {
			*ppos += ret;
			ctl->pos += ret;
		}
	}
	return ret;
}

static ssize_t mtdocap_write_nonvol(struct file *file, const char __user *buf, size_t count,loff_t *ppos)
{
	struct ocap_struct * ctl = (struct ocap_struct *)file->private_data;
	uint8_t * dest = ctl->buf;
	int offset = ctl->pos;
	int size = ctl->size;
	ssize_t ret = count;

	down_interruptible(&flush_sema);
	/* attach the buffer */
	if ((offset + count) <= size) {
		DBG_INFO("%s: write offset %d, count %d\n", ctl->name, offset, count);
		memcpy(dest + offset, buf, count);
		offset += count;
		ctl->pos = offset;
		/*
		 * We need to wait for the whole section (dynamic or permanent config)
		 * to be filled before writing to flash.
		 *
		 * rocaphost: If the received packet length is not 1450 (MAX_MESSAGE_SIZE - 8),
		 * the section is consdered to be finished.
		 *
		 * tftpd: The section is finished if the packat size is not
		 * 512 (TFTP_BLOCK_SIZE);
		 *
		 * otherwise, flush whatever in the buffer into flash. bulkdma fails into this sceneria, since
		 * it always write the whole section in one request.
		 */
		if ( !((count == (MAX_MESSAGE_SIZE - 8)) || (count == TFTP_BLOCK_SIZE))) {
			DBG_INFO("%s: need flush, offset %d, count %d\n", ctl->name, offset, count);
			FLUSH_SET(ctl);
			wake_up_interruptible(&ctl->wait);
		}
		ctl->dirty = 1;
	}
	else {
		DBG_ERR("write out of boundary\n");
		ret = -EFAULT;
	}
	up(&flush_sema);
	return ret;
}


/*
 * functions copied from mtdchar.c, with minor changes to work for mtdocap,
 * mainly for read/write and erase region, not fully verified
 */

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,17)

static loff_t mtdocap_llseek_normal (struct file *file, loff_t offset, int orig)
{
	struct ocap_struct * ctl ;
	struct mtd_info *mtd;

	ctl = TO_MTDOCAP_CTL(file);
	mtd = TO_MTDOCAP(ctl);

	switch (orig) {
	case SEEK_SET:
		break;
	case SEEK_CUR:
		offset += file->f_pos;
		break;
	case SEEK_END:
		offset += mtd->size;
		break;
	default:
		return -EINVAL;
	}

	if (offset >= 0 && offset <= mtd->size)
		return file->f_pos = offset;

	return -EINVAL;
}



static int mtdocap_open_normal(struct inode *inode, struct file *file)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37)
	struct ocap_struct * ctl;
	struct mtd_info *mtd;
	unsigned int minor = iminor(inode);
	int devnum = minor >> 1;
	struct inode *mtd_ino;
	int ret = 0;

	DBG_INFO( "%s %u\n", __FUNCTION__, minor);
	if (minor >= MAX_OCAP_DEV)
		return -ENODEV;

	ctl = &ocap_ctl[minor];

	/* You can't open the RO devices RW
	if ((file->f_mode & FMODE_WRITE) && (minor & 1))
		return -EACCES;
	*/

	down_interruptible(&flush_sema);
	mtd = get_mtd_device(NULL, ctl->devnum);

	if (IS_ERR(mtd)) {
		ret = PTR_ERR(mtd);
		goto out;
	}

	if (mtd->type == MTD_ABSENT) {
		put_mtd_device(mtd);
		ret = -ENODEV;
		goto out;
	}

	mtd_ino = iget_locked(mtd_inode_mnt->mnt_sb, devnum);
	if (!mtd_ino) {
		put_mtd_device(mtd);
		ret = -ENOMEM;
		goto out;
	}
	if (mtd_ino->i_state & I_NEW) {
		mtd_ino->i_private = mtd;
		mtd_ino->i_mode = S_IFCHR;
		mtd_ino->i_data.backing_dev_info = mtd->backing_dev_info;
		unlock_new_inode(mtd_ino);
	}
	file->f_mapping = mtd_ino->i_mapping;

	/* You can't open it RW if it's not a writeable device */
	if ((file->f_mode & FMODE_WRITE) && !(mtd->flags & MTD_WRITEABLE)) {
		iput(mtd_ino);
		put_mtd_device(mtd);
		ret = -EACCES;
		goto out;
	}

	ctl->ino = mtd_ino;
	ctl->mtd = mtd;
	DBG_INFO("%s ctl %p, mtd %p\n", __FUNCTION__, ctl, mtd);
	file->private_data = ctl;

out:
	up(&flush_sema);
	return ret;
#elif LINUX_VERSION_CODE == KERNEL_VERSION(2,6,18)
	struct ocap_struct * ctl;
	struct mtd_info *mtd;
	unsigned int minor = iminor(inode);

	DBG_INFO( "%s %u\n", __FUNCTION__, minor);
	if (minor >= MAX_OCAP_DEV)
		return -ENODEV;

	ctl = &ocap_ctl[minor];

	/* mtd = get_mtd_device(NULL, ctl->devnum); */

	/* You can't open the RO devices RW
	if ((file->f_mode & 2) && (minor & 1))
		return -EACCES;
	*/

	mtd = get_mtd_device(NULL, ctl->devnum);

	if (!mtd)
		return -ENODEV;

	if (MTD_ABSENT == mtd->type) {
		put_mtd_device(mtd);
		return -ENODEV;
	}

	/* You can't open it RW if it's not a writeable device */
	if ((file->f_mode & 2) && !(mtd->flags & MTD_WRITEABLE)) {
		put_mtd_device(mtd);
		return -EACCES;
	}

	ctl->mtd = mtd;
	DBG_INFO("%s ctl %p, mtd %p\n", __FUNCTION__, ctl, mtd);
	file->private_data = ctl;

	return 0;
#else
	struct ocap_struct * ctl;
	struct mtd_info *mtd;
	unsigned int minor = iminor(inode);
	int ret = 0;

	DBG_INFO( "%s %u\n", __FUNCTION__, minor);
	if (minor >= MAX_OCAP_DEV)
		return -ENODEV;

	ctl = &ocap_ctl[minor];

	/* You can't open the RO devices RW
	if ((file->f_mode & FMODE_WRITE) && (minor & 1))
		return -EACCES;
	*/

	lock_kernel();
	mtd = get_mtd_device(NULL, ctl->devnum);

	if (IS_ERR(mtd)) {
		ret = PTR_ERR(mtd);
		goto out;
	}

	if (mtd->type == MTD_ABSENT) {
		put_mtd_device(mtd);
		ret = -ENODEV;
		goto out;
	}

	if (mtd->backing_dev_info)
		file->f_mapping->backing_dev_info = mtd->backing_dev_info;

	/* You can't open it RW if it's not a writeable device */
	if ((file->f_mode & FMODE_WRITE) && !(mtd->flags & MTD_WRITEABLE)) {
		put_mtd_device(mtd);
		ret = -EACCES;
		goto out;
	}

	ctl->mtd = mtd;
	DBG_INFO("%s ctl %p, mtd %p\n", __FUNCTION__, ctl, mtd);
	file->private_data = ctl;

out:
	unlock_kernel();
	return ret;
#endif
} /* mtd_open */

/*====================================================================*/

static int mtdocap_close_normal(struct inode *inode, struct file *file)
{
	struct ocap_struct * ctl ;
	struct mtd_info *mtd;

	ctl = TO_MTDOCAP_CTL(file);
	mtd = TO_MTDOCAP(ctl);

#if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,18)
	if (mtd->sync)
#else
	/* Only sync if opened RW */
	if ((file->f_mode & FMODE_WRITE) && mtd->sync)
#endif
		mtd->sync(mtd);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37)
	iput(ctl->ino);
#endif

	put_mtd_device(mtd);
	file->private_data = NULL;
	return 0;
} /* mtd_close */

/* FIXME: This _really_ needs to die. In 2.5, we should lock the
   userspace buffer down and use it directly with readv/writev.
*/
#define MAX_KMALLOC_SIZE 0x20000

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37)
#define MTD_MODE_OTP_FACTORY MTD_FILE_MODE_OTP_FACTORY
#define MTD_MODE_OTP_USER MTD_FILE_MODE_OTP_USER
#define MTD_MODE_RAW MTD_FILE_MODE_RAW
#define MTD_OOB_RAW MTD_OPS_RAW
#define MTD_MODE_NORMAL MTD_FILE_MODE_NORMAL
#endif

static ssize_t mtdocap_read_normal(struct file *file, char __user *buf, size_t count,loff_t *ppos)
{
	struct ocap_struct * ctl ;
	struct mtd_info *mtd;
	size_t retlen=0;
	size_t total_retlen=0;
	int ret=0;
	int len;
	char *kbuf;
	size_t size;
	int start;

	ctl = TO_MTDOCAP_CTL(file);
	mtd = TO_MTDOCAP(ctl);
	size = ctl->size;
	start = ctl->start;
	if ( *ppos + count > size)
		count = size - *ppos;
	if (!count)
		return 0;

	/* FIXME: Use kiovec in 2.5 to lock down the user's buffers
	   and pass them directly to the MTD functions */

	if (count > MAX_KMALLOC_SIZE)
		kbuf=kmalloc(MAX_KMALLOC_SIZE, GFP_KERNEL);
	else
		kbuf=kmalloc(count, GFP_KERNEL);

	if (!kbuf)
		return -ENOMEM;

	while (count) {

		if (count > MAX_KMALLOC_SIZE)
			len = MAX_KMALLOC_SIZE;
		else
			len = count;

		switch (MTD_MODE(file)) {
		case MTD_MODE_OTP_FACTORY:
			ret = mtd->read_fact_prot_reg(mtd, start + *ppos, len, &retlen, kbuf);
			break;
		case MTD_MODE_OTP_USER:
			ret = mtd->read_user_prot_reg(mtd, start + *ppos, len, &retlen, kbuf);
			break;
		case MTD_MODE_RAW:
		{
			struct mtd_oob_ops ops;

			ops.mode = MTD_OOB_RAW;
			ops.datbuf = kbuf;
			ops.oobbuf = NULL;
			ops.len = len;

			ret = mtd->read_oob(mtd, start + *ppos, &ops);
			retlen = ops.retlen;
			break;
		}
		default:
#ifdef CONFIG_MTD_BRCMNAND
			ret = mtd_read_skip_badblock(ctl, *ppos, len, &retlen, kbuf);
#else
			ret = mtd->read(mtd, start + *ppos, len, &retlen, kbuf);
#endif
		}
		/* Nand returns -EBADMSG on ecc errors, but it returns
		 * the data. For our userspace tools it is important
		 * to dump areas with ecc errors !
		 * For kernel internal usage it also might return -EUCLEAN
		 * to signal the caller that a bitflip has occured and has
		 * been corrected by the ECC algorithm.
		 * Userspace software which accesses NAND this way
		 * must be aware of the fact that it deals with NAND
		 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37)
		if (!ret || mtd_is_bitflip_or_eccerr(ret)) {
#else
		if (!ret || (ret == -EUCLEAN) || (ret == -EBADMSG)) {
#endif
			*ppos += retlen;
			if (copy_to_user(buf, kbuf, retlen)) {
				kfree(kbuf);
				return -EFAULT;
			}
			else
				total_retlen += retlen;

			count -= retlen;
			buf += retlen;
			if (retlen == 0)
				count = 0;
		}
		else {
			kfree(kbuf);
			return ret;
		}

	}

	kfree(kbuf);
	return total_retlen;
}

static ssize_t mtdocap_write_normal(struct file *file, const char __user *buf, size_t count,loff_t *ppos)
{
	struct ocap_struct * ctl ;
	struct mtd_info *mtd;
	char *kbuf;
	size_t retlen;
	size_t total_retlen=0;
	int ret=0;
	int len = 0;
	size_t size;
	int start;
	int erase_size = 0;

	ctl = TO_MTDOCAP_CTL(file);
	mtd = TO_MTDOCAP(ctl);
	size = ctl->size;
	start = ctl->start;
	DBG_INFO(".");
	if (*ppos == size)
		return -ENOSPC;
	if (*ppos + count > size)
		count = size - *ppos;
	if (!count)
		return 0;

	/* erase the flash if necessary */
	erase_size = ctl->erase_size;
	if ( (*ppos & (erase_size - 1)) == 0)  {
		if (FlashDriverEraseRegion(ctl, *ppos, 1) < 0) {
			return 0;
		}
	}

	len = (*ppos + erase_size) & (~(erase_size - 1));

	while ( len < (*ppos + count)) {
		if (FlashDriverEraseRegion(ctl, len, 1) < 0) {
			return 0;
		}
		len += erase_size;
	}

	if (count > MAX_KMALLOC_SIZE)
		kbuf=kmalloc(MAX_KMALLOC_SIZE, GFP_KERNEL);
	else
		kbuf=kmalloc(count, GFP_KERNEL);

	if (!kbuf)
		return -ENOMEM;

	while (count) {

		if (count > MAX_KMALLOC_SIZE)
			len = MAX_KMALLOC_SIZE;
		else
			len = count;

		if (copy_from_user(kbuf, buf, len)) {
			kfree(kbuf);
			return -EFAULT;
		}

		switch (MTD_MODE(file)) {
		case MTD_MODE_OTP_FACTORY:
			ret = -EROFS;
			break;
		case MTD_MODE_OTP_USER:
			if (!mtd->write_user_prot_reg) {
				ret = -EOPNOTSUPP;
				break;
			}
			ret = mtd->write_user_prot_reg(mtd, start + *ppos, len, &retlen, kbuf);
			break;

		case MTD_MODE_RAW:
		{
			struct mtd_oob_ops ops;

			ops.mode = MTD_OOB_RAW;
			ops.datbuf = kbuf;
			ops.oobbuf = NULL;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37)
			ops.ooboffs = 0;
#endif
			ops.len = len;

			DEBUG(MTD_DEBUG_LEVEL0,"MTD_write_oob(%08x, len=%d)\n", start + (int)(*ppos), len);
			ret = mtd->write_oob(mtd, ctl->start + *ppos, &ops);
			retlen = ops.retlen;
			break;
		}

		default:
#ifdef CONFIG_MTD_BRCMNAND
			DEBUG(MTD_DEBUG_LEVEL0,"mtd_write_skip_badblock(%08x, len=%d)\n", (int)(*ppos), len);
			ret = mtd_write_skip_badblock(ctl, *ppos, len, &retlen, kbuf);
#else
			DEBUG(MTD_DEBUG_LEVEL0,"MTD_write(%08x, len=%d)\n", start + *ppos, len);
			ret = (*(mtd->write))(mtd, start + *ppos, len, &retlen, kbuf);
#endif
		}
		if (!ret) {
			*ppos += retlen;
			total_retlen += retlen;
			count -= retlen;
			buf += retlen;
		}
		else {
			kfree(kbuf);
			return ret;
		}
	}

	kfree(kbuf);
	return total_retlen;
}

/*======================================================================

    IOCTL calls for getting device parameters.

======================================================================*/
static void mtdchar_erase_callback (struct erase_info *instr)
{
	wake_up((wait_queue_head_t *)instr->priv);
}

#if defined(CONFIG_MTD_OTP) || defined(CONFIG_MTD_ONENAND_OTP) || defined(CONFIG_HAVE_MTD_OTP)
static int otp_select_filemode(struct file *file, int mode)
{
	struct ocap_struct * ctl ;
	struct mtd_info *mtd;
	int ret = 0;

	ctl = TO_MTDOCAP_CTL(file);
	mtd = TO_MTDOCAP(ctl);
	switch (MTD_MODE(file)) {
	case MTD_OTP_FACTORY:
		if (!mtd->read_fact_prot_reg)
			ret = -EOPNOTSUPP;
		else
			ctl->mode = MTD_MODE_OTP_FACTORY;
		break;
	case MTD_OTP_USER:
		if (!mtd->read_fact_prot_reg)
			ret = -EOPNOTSUPP;
		else
			ctl->mode = MTD_MODE_OTP_USER;
		break;
	default:
		ret = -EINVAL;
	case MTD_OTP_OFF:
		break;
	}
	return ret;
}
#else
# define otp_select_filemode(f,m)	-EOPNOTSUPP
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37)
static long mtdocap_unlocked_ioctl_normal(struct file *file, u_int cmd, u_long arg)
{
	struct inode *inode = file->f_path.dentry->d_inode;
	int ret;

	down_interruptible(&flush_sema);
	ret = mtdocap_ioctl_normal(inode, file, cmd, arg);
	up(&flush_sema);

	return ret;
}
#endif

static int mtdocap_ioctl_normal(struct inode *inode, struct file *file, u_int cmd, u_long arg)
{
	struct ocap_struct * ctl ;
	struct mtd_info *mtd;
	void __user *argp = (void __user *)arg;
	int ret = 0;
	u_long size;
	struct mtd_info_user info;
#if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,18)
#ifdef MEMGETINFO64
	struct mtd_info_user64 info64;
#endif
#endif

	ctl = TO_MTDOCAP_CTL(file);
	mtd = TO_MTDOCAP(ctl);

	size = (cmd & IOCSIZE_MASK) >> IOCSIZE_SHIFT;
	if (cmd & IOC_IN) {
		if (!access_ok(VERIFY_READ, argp, size))
			return -EFAULT;
	}
	if (cmd & IOC_OUT) {
		if (!access_ok(VERIFY_WRITE, argp, size))
			return -EFAULT;
	}

	switch (cmd) {
	case MEMGETREGIONCOUNT:
		if (copy_to_user(argp, &(mtd->numeraseregions), sizeof(int)))
			return -EFAULT;
		break;

	case MEMGETREGIONINFO:
	{
#if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,18)
		struct region_info_user ur;

		if (copy_from_user(&ur, argp, sizeof(struct region_info_user)))
			return -EFAULT;

		if (ur.regionindex >= mtd->numeraseregions)
			return -EINVAL;
		if (copy_to_user(argp, &(mtd->eraseregions[ur.regionindex]),
				sizeof(struct mtd_erase_region_info)))
			return -EFAULT;
		break;
#else
		uint32_t ur_idx;
		struct mtd_erase_region_info *kr;
		struct region_info_user *ur = (struct region_info_user *) argp;

		if (get_user(ur_idx, &(ur->regionindex)))
			return -EFAULT;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37)
		if (ur_idx >= mtd->numeraseregions)
			return -EINVAL;
#endif

		kr = &(mtd->eraseregions[ur_idx]);

		if (put_user(kr->offset, &(ur->offset))
			|| put_user(kr->erasesize, &(ur->erasesize))
			|| put_user(kr->numblocks, &(ur->numblocks)))
			return -EFAULT;

		break;
#endif
	}

	case MEMGETINFO:
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37)
		memset(&info, 0, sizeof(info));
#endif
		info.type	= mtd->type;
		info.flags	= mtd->flags;
		info.size	= ctl->size;
		info.erasesize	= mtd->erasesize;
/* TDT breaks our mtd-utils */
#ifdef USE_WRITESIZE
		info.writesize	= mtd->writesize;
#else
		info.oobblock	= mtd->writesize;
#endif
		info.oobsize	= mtd->oobsize;
#if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,18)
		info.ecctype	= mtd->ecctype;
		info.eccsize	= mtd->eccsize;
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37)
		info.padding	= 0;
#endif
		if (copy_to_user(argp, &info, sizeof(struct mtd_info_user)))
			return -EFAULT;
		break;

#if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,18)
#ifdef MEMGETINFO64
	case MEMGETINFO64:
		info64.type		= mtd->type;
		info64.flags		= mtd->flags;
		info64.size	        = ctl->size;
		info64.erasesize	= mtd->erasesize;
		info64.writesize	= mtd->writesize;
		info64.oobsize		= mtd->oobsize;
		info64.ecctype		= mtd->ecctype;
		info64.eccsize		= mtd->eccsize;
		if (copy_to_user(argp, &info64, sizeof(struct mtd_info_user64)))
			return -EFAULT;
		break;
#endif
#endif

	case MEMERASE:
	{
		struct erase_info *erase;

#if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,18)
		if(!(file->f_mode & 2))
			return -EPERM;

		erase=kmalloc(sizeof(struct erase_info),GFP_KERNEL);
		if (!erase)
			ret = -ENOMEM;
		else {
			wait_queue_head_t waitq;
			DECLARE_WAITQUEUE(wait, current);

			init_waitqueue_head(&waitq);

			memset (erase,0,sizeof(struct erase_info));
			if (copy_from_user(&erase->addr, argp,
				    sizeof(struct erase_info_user))) {
				kfree(erase);
				return -EFAULT;
			}
#else
		if(!(file->f_mode & FMODE_WRITE))
			return -EPERM;

		erase=kzalloc(sizeof(struct erase_info),GFP_KERNEL);
		if (!erase)
			ret = -ENOMEM;
		else {
			struct erase_info_user einfo;

			wait_queue_head_t waitq;
			DECLARE_WAITQUEUE(wait, current);

			init_waitqueue_head(&waitq);

			if (copy_from_user(&einfo, argp,
					sizeof(struct erase_info_user))) {
				kfree(erase);
				return -EFAULT;
			}
            erase->addr = einfo.start;
            erase->len = einfo.length;
#endif
			erase->mtd = mtd;
			erase->callback = mtdchar_erase_callback;
			erase->priv = (unsigned long)&waitq;
			erase->addr += ctl->start;

			/*
			  FIXME: Allow INTERRUPTIBLE. Which means
			  not having the wait_queue head on the stack.

			  If the wq_head is on the stack, and we
			  leave because we got interrupted, then the
			  wq_head is no longer there when the
			  callback routine tries to wake us up.
			*/
			ret = mtd->erase(mtd, erase);
			if (!ret) {
				set_current_state(TASK_UNINTERRUPTIBLE);
				add_wait_queue(&waitq, &wait);
				if (erase->state != MTD_ERASE_DONE &&
				    erase->state != MTD_ERASE_FAILED)
					schedule();
				remove_wait_queue(&waitq, &wait);
				set_current_state(TASK_RUNNING);

				ret = (erase->state == MTD_ERASE_FAILED)?-EIO:0;
			}
			kfree(erase);
		}
		break;
	}

#if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,18)
#ifdef MEMERASE64
	case MEMERASE64:
	{
		struct erase_info *erase;

		if(!(file->f_mode & 2))
			return -EPERM;

		erase=kmalloc(sizeof(struct erase_info),GFP_KERNEL);
		if (!erase)
			ret = -ENOMEM;
		else {
			wait_queue_head_t waitq;
			DECLARE_WAITQUEUE(wait, current);

			init_waitqueue_head(&waitq);

			memset (erase,0,sizeof(struct erase_info));
			if (copy_from_user(&erase->addr, argp,
				    sizeof(struct erase_info_user64))) {
				kfree(erase);
				return -EFAULT;
			}
			erase->mtd = mtd;
			erase->callback = mtdchar_erase_callback;
			erase->priv = (unsigned long)&waitq;
			erase->addr += ctl->start;

			/*
			  FIXME: Allow INTERRUPTIBLE. Which means
			  not having the wait_queue head on the stack.

			  If the wq_head is on the stack, and we
			  leave because we got interrupted, then the
			  wq_head is no longer there when the
			  callback routine tries to wake us up.
			*/
			ret = mtd->erase(mtd, erase);
			if (!ret) {
				set_current_state(TASK_UNINTERRUPTIBLE);
				add_wait_queue(&waitq, &wait);
				if (erase->state != MTD_ERASE_DONE &&
				    erase->state != MTD_ERASE_FAILED)
					schedule();
				remove_wait_queue(&waitq, &wait);
				set_current_state(TASK_RUNNING);

				ret = (erase->state == MTD_ERASE_FAILED)?-EIO:0;
			}
			kfree(erase);
		}
		break;
	}
#endif
#endif

	case MEMWRITEOOB:
	{
		struct mtd_oob_buf buf;
		struct mtd_oob_ops ops;
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,18)
		struct mtd_oob_buf __user *user_buf = argp;
        uint32_t retlen;
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37)
		struct mtd_file_info *mfi = file->private_data;
#endif

#if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,18)
		if(!(file->f_mode & 2))
#else
		if(!(file->f_mode & FMODE_WRITE))
#endif
			return -EPERM;

		if (copy_from_user(&buf, argp, sizeof(struct mtd_oob_buf)))
			return -EFAULT;

		if (buf.length > 4096)
			return -EINVAL;

		if (!mtd->write_oob)
			ret = -EOPNOTSUPP;
		else
			ret = access_ok(VERIFY_READ, buf.ptr, buf.length) ? 0 : -EFAULT;

		if (ret)
			return ret;

#if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,18)
		ops.len = buf.length;
#endif
		ops.ooblen = buf.length;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37)
		ops.ooboffs = buf.start & (mtd->writesize - 1);
#else
		ops.ooboffs = buf.start & (mtd->oobsize - 1);
#endif
		ops.datbuf = NULL;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37)
		ops.mode = (mfi->mode == MTD_FILE_MODE_RAW) ? MTD_OPS_RAW : MTD_OPS_PLACE_OOB;
#else
		ops.mode = MTD_OOB_PLACE;
#endif

#if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,18)
		if (ops.ooboffs && ops.len > (mtd->oobsize - ops.ooboffs))
#else
		if (ops.ooboffs && ops.ooblen > (mtd->oobsize - ops.ooboffs))
#endif
			return -EINVAL;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37)
		ops.oobbuf = memdup_user(buf.ptr, buf.length);
		if (IS_ERR(ops.oobbuf))
			return PTR_ERR(ops.oobbuf);

		buf.start &= ~(mtd->writesize - 1);
#else
		ops.oobbuf = kmalloc(buf.length, GFP_KERNEL);
		if (!ops.oobbuf)
			return -ENOMEM;

		if (copy_from_user(ops.oobbuf, buf.ptr, buf.length)) {
			kfree(ops.oobbuf);
			return -EFAULT;
		}
		buf.start &= ~(mtd->oobsize - 1);
#endif
		ret = mtd->write_oob(mtd, ctl->start + buf.start, &ops);

#if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,18)
		if (copy_to_user(argp + sizeof(uint32_t), &ops.retlen,
				 sizeof(uint32_t)))
#else
		if (ops.oobretlen > 0xFFFFFFFFU)
			ret = -EOVERFLOW;

		retlen = ops.oobretlen;
		if (copy_to_user(&user_buf->length, &retlen, sizeof(buf.length)))
#endif
			ret = -EFAULT;

		kfree(ops.oobbuf);
		break;

	}

	case MEMREADOOB:
	{
		struct mtd_oob_buf buf;
		struct mtd_oob_ops ops;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37)
		struct mtd_file_info *mfi = file->private_data;
#endif

		if (copy_from_user(&buf, argp, sizeof(struct mtd_oob_buf)))
			return -EFAULT;

		if (buf.length > 4096)
			return -EINVAL;

		if (!mtd->read_oob)
			ret = -EOPNOTSUPP;
		else
			ret = access_ok(VERIFY_WRITE, buf.ptr, buf.length) ? 0 : -EFAULT;
		if (ret)
			return ret;

#if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,18)
		ops.len = buf.length;
#endif
		ops.ooblen = buf.length;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37)
		ops.ooboffs = buf.start & (mtd->writesize - 1);
#else
		ops.ooboffs = buf.start & (mtd->oobsize - 1);
#endif
		ops.datbuf = NULL;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37)
		ops.mode = (mfi->mode == MTD_FILE_MODE_RAW) ? MTD_OPS_RAW : MTD_OPS_PLACE_OOB;
#else
		ops.mode = MTD_OOB_PLACE;
#endif

#if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,18)
		if (ops.ooboffs && ops.len > (mtd->oobsize - ops.ooboffs))
#else
		if (ops.ooboffs && ops.ooblen > (mtd->oobsize - ops.ooboffs))
#endif
			return -EINVAL;

		ops.oobbuf = kmalloc(buf.length, GFP_KERNEL);
		if (!ops.oobbuf)
			return -ENOMEM;

		buf.start &= ~(mtd->oobsize - 1);
		ret = mtd->read_oob(mtd, ctl->start + buf.start, &ops);

#if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,18)
		if (put_user(ops.retlen, (uint32_t __user *)argp))
#else
		if (put_user(ops.oobretlen, (uint32_t __user *)argp))
#endif
			ret = -EFAULT;
#if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,18)
		else if (ops.retlen && copy_to_user(buf.ptr, ops.oobbuf, ops.retlen))
#else
		else if (ops.oobretlen && copy_to_user(buf.ptr, ops.oobbuf, ops.oobretlen))
#endif
			ret = -EFAULT;

		kfree(ops.oobbuf);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37)
		/*
		 * NAND returns -EBADMSG on ECC errors, but it returns the OOB
		 * data. For our userspace tools it is important to dump areas
		 * with ECC errors!
		 * For kernel internal usage it also might return -EUCLEAN
		 * to signal the caller that a bitflip has occured and has
		 * been corrected by the ECC algorithm.
		 *
		 * Note: currently the standard NAND function, nand_read_oob_std,
		 * does not calculate ECC for the OOB area, so do not rely on
		 * this behavior unless you have replaced it with your own.
		 */
		if (mtd_is_bitflip_or_eccerr(ret))
			ret = 0;
#endif
		break;
	}

	case MEMLOCK:
	{
		struct erase_info_user info;

		if (copy_from_user(&info, argp, sizeof(info)))
			return -EFAULT;

		if (!mtd->lock)
			ret = -EOPNOTSUPP;
		else
			ret = mtd->lock(mtd, info.start, info.length);
		break;
	}

	case MEMUNLOCK:
	{
		struct erase_info_user info;

		if (copy_from_user(&info, argp, sizeof(info)))
			return -EFAULT;

		if (!mtd->unlock)
			ret = -EOPNOTSUPP;
		else
			ret = mtd->unlock(mtd, info.start, info.length);
		break;
	}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37)
	case MEMISLOCKED:
	{
		struct erase_info_user info;

		if (copy_from_user(&info, argp, sizeof(info)))
			return -EFAULT;

		if (!mtd->is_locked)
			ret = -EOPNOTSUPP;
		else
			ret = mtd->is_locked(mtd, info.start, info.length);
		break;
	}
#endif

	/* Legacy interface */
	case MEMGETOOBSEL:
	{
		struct nand_oobinfo oi;

		if (!mtd->ecclayout)
			return -EOPNOTSUPP;
		if (mtd->ecclayout->eccbytes > ARRAY_SIZE(oi.eccpos))
			return -EINVAL;

		oi.useecc = MTD_NANDECC_AUTOPLACE;
		memcpy(&oi.eccpos, mtd->ecclayout->eccpos, sizeof(oi.eccpos));
		memcpy(&oi.oobfree, mtd->ecclayout->oobfree,
		       sizeof(oi.oobfree));

		if (copy_to_user(argp, &oi, sizeof(struct nand_oobinfo)))
			return -EFAULT;
		break;
	}

	case MEMGETBADBLOCK:
	{
		loff_t offs;

		if (copy_from_user(&offs, argp, sizeof(loff_t)))
			return -EFAULT;
		if (!mtd->block_isbad)
			ret = -EOPNOTSUPP;
		else
			return mtd->block_isbad(mtd, offs);
		break;
	}

	case MEMSETBADBLOCK:
	{
		loff_t offs;

		if (copy_from_user(&offs, argp, sizeof(loff_t)))
			return -EFAULT;
		if (!mtd->block_markbad)
			ret = -EOPNOTSUPP;
		else
			return mtd->block_markbad(mtd, offs);
		break;
	}

#if defined(CONFIG_MTD_OTP) || defined(CONFIG_MTD_ONENAND_OTP) || defined(CONFIG_HAVE_MTD_OTP)
	case OTPSELECT:
	{
		int mode;
		if (copy_from_user(&mode, argp, sizeof(int)))
			return -EFAULT;

		ctl->mode = MTD_MODE_NORMAL;

		ret = otp_select_filemode(file, mode);

		file->f_pos = 0;
		break;
	}

	case OTPGETREGIONCOUNT:
	case OTPGETREGIONINFO:
	{
		struct otp_info *buf = kmalloc(4096, GFP_KERNEL);
		if (!buf)
			return -ENOMEM;
		ret = -EOPNOTSUPP;
		switch (MTD_MODE(file)) {
		case MTD_MODE_OTP_FACTORY:
			if (mtd->get_fact_prot_info)
				ret = mtd->get_fact_prot_info(mtd, buf, 4096);
			break;
		case MTD_MODE_OTP_USER:
			if (mtd->get_user_prot_info)
				ret = mtd->get_user_prot_info(mtd, buf, 4096);
			break;
		default:
			break;
		}
		if (ret >= 0) {
			if (cmd == OTPGETREGIONCOUNT) {
				int nbr = ret / sizeof(struct otp_info);
				ret = copy_to_user(argp, &nbr, sizeof(int));
			} else
				ret = copy_to_user(argp, buf, ret);
			if (ret)
				ret = -EFAULT;
		}
		kfree(buf);
		break;
	}

	case OTPLOCK:
	{
		struct otp_info info;

		if (MTD_MODE(file) != MTD_MODE_OTP_USER)
			return -EINVAL;
		if (copy_from_user(&info, argp, sizeof(info)))
			return -EFAULT;
		if (!mtd->lock_user_prot_reg)
			return -EOPNOTSUPP;
		ret = mtd->lock_user_prot_reg(mtd, info.start, info.length);
		break;
	}
#endif

	case ECCGETLAYOUT:
	{
		if (!mtd->ecclayout)
			return -EOPNOTSUPP;

		if (copy_to_user(argp, &mtd->ecclayout,
				 sizeof(struct nand_ecclayout)))
			return -EFAULT;
		break;
	}

	case ECCGETSTATS:
	{
		if (copy_to_user(argp, &mtd->ecc_stats,
				 sizeof(struct mtd_ecc_stats)))
			return -EFAULT;
		break;
	}

	case MTDFILEMODE:
	{
		ctl->mode = 0;

		switch(arg) {
		case MTD_MODE_OTP_FACTORY:
		case MTD_MODE_OTP_USER:
			ret = otp_select_filemode(file, arg);
			break;

		case MTD_MODE_RAW:
			if (!mtd->read_oob || !mtd->write_oob)
				return -EOPNOTSUPP;
			ctl->mode = arg;

		case MTD_MODE_NORMAL:
			break;
		default:
			ret = -EINVAL;
		}
		file->f_pos = 0;
		break;
	}

	default:
		ret = -ENOTTY;
	}

	return ret;
} /* memory_ioctl */

#else /* LINUX_VERSION_CODE > KERNEL_VERSION(2,6,17) */


#define MTD_MODE_OTP_FACT	1
#define MTD_MODE_OTP_USER	2
#define MTD_MODE(file)		((long)((file)->private_data) & 3)


static int mtdocap_open_normal(struct inode *inode, struct file *file)
{
	struct mtd_info *mtd;
	struct ocap_struct * ctl;

	ctl = &ocap_ctl[minor];

	mtd = get_mtd_device(NULL, ctl->devnum);

	if (!mtd)
		return -ENODEV;

	if (MTD_ABSENT == mtd->type) {
		put_mtd_device(mtd);
		return -ENODEV;
	}
	ctl->mtd = mtd;
	file->private_data = ctl;

	return 0;

}
static int mtdocap_close_normal(struct inode *inode, struct file *file)
{
	struct ocap_struct * ctl ;
	struct mtd_info *mtd;

	ctl = TO_MTDOCAP_CTL(file);
	mtd = TO_MTDOCAP(ctl);

	if (mtd->sync)
		mtd->sync(mtd);
	put_mtd_device(mtd);
	file->private_data = NULL;
	return 0;
}
static loff_t mtdocap_llseek_normal(struct file *file, loff_t offset, int orig)
{
	struct ocap_struct * ctl ;
	struct mtd_info *mtd;

	DBG_INFO("%s\n", __FUNCTION__);
	ctl = TO_MTDOCAP_CTL(file);
	mtd = TO_MTDOCAP(ctl);

	switch (orig) {
	case 0:
		/* SEEK_SET */
		break;
	case 1:
		/* SEEK_CUR */
		offset += file->f_pos;
		break;
	case 2:
		/* SEEK_END */
		offset += mtd->size;
		break;
	default:
		return -EINVAL;
	}

	if (offset >= 0 && offset < mtd->size)
		return file->f_pos = offset;

	return -EINVAL;
}

/* FIXME: This _really_ needs to die. In 2.5, we should lock the
   userspace buffer down and use it directly with readv/writev.
*/
#define MAX_KMALLOC_SIZE 0x20000

static ssize_t mtdocap_read_normal(struct file *file, char __user *buf, size_t count,loff_t *ppos)
{
	struct ocap_struct * ctl ;
	struct mtd_info *mtd;
	size_t retlen=0;
	size_t total_retlen=0;
	int ret=0;
	int len;
	char *kbuf;
	size_t size;
	int start;

	ctl = TO_MTDOCAP_CTL(file);
	mtd = TO_MTDOCAP(ctl);
	size = ctl->size;
	start = ctl->start;
	if ( *ppos + count > size)
		count = size - *ppos;

	if (!count)
		return 0;

	/* FIXME: Use kiovec in 2.5 to lock down the user's buffers
	   and pass them directly to the MTD functions */
	while (count) {
		if (count > MAX_KMALLOC_SIZE)
			len = MAX_KMALLOC_SIZE;
		else
			len = count;

		kbuf=kmalloc(len,GFP_KERNEL);
		if (!kbuf)
			return -ENOMEM;

		switch (MTD_MODE(file)) {
		case MTD_MODE_OTP_FACT:
			ret = mtd->read_fact_prot_reg(mtd, start + *ppos, len, &retlen, kbuf);
			break;
		case MTD_MODE_OTP_USER:
			ret = mtd->read_user_prot_reg(mtd, start + *ppos, len, &retlen, kbuf);
			break;
		default:
#ifdef CONFIG_MTD_BRCMNAND
			ret = mtd_read_skip_badblock(ctl, *ppos, len, &retlen, kbuf);
#else
			ret = MTD_READ(mtd, start + *ppos, len, &retlen, kbuf);
#endif
		}
		/* Nand returns -EBADMSG on ecc errors, but it returns
		 * the data. For our userspace tools it is important
		 * to dump areas with ecc errors !
		 * Userspace software which accesses NAND this way
		 * must be aware of the fact that it deals with NAND
		 */
		if (!ret || (ret == -EBADMSG)) {
			*ppos += retlen;
			if (copy_to_user(buf, kbuf, retlen)) {
			        kfree(kbuf);
				return -EFAULT;
			}
			else
				total_retlen += retlen;

			count -= retlen;
			buf += retlen;
			if (retlen == 0)
				count = 0;
		}
		else {
			kfree(kbuf);
			return ret;
		}

		kfree(kbuf);
	}

	return total_retlen;
}

static ssize_t mtdocap_write_normal(struct file *file, const char __user *buf, size_t count,loff_t *ppos)
{
	struct ocap_struct * ctl ;
	struct mtd_info *mtd;
	char *kbuf;
	size_t retlen;
	size_t total_retlen=0;
	int ret=0;
	int len;
	size_t size;
	int start;
	int erase_size = 0;


	ctl = TO_MTDOCAP_CTL(file);
	mtd = TO_MTDOCAP(ctl);
	size = ctl->size;
	start = ctl->start;
	DBG_INFO(".");

	if (*ppos == size)
		return -ENOSPC;

	if (*ppos + count > size)
		count = size - *ppos;

	if (!count)
		return 0;

	/* erase the flash if necessary */
	erase_size = ctl->erase_size;
	if ( (*ppos & (erase_size - 1)) == 0)  {
		if (FlashDriverEraseRegion(ctl, *ppos, 1) < 0) {
			return 0;
		}
	}

	len = (*ppos + erase_size) & (~(erase_size - 1));

	while ( len < (*ppos + count)) {
		if (FlashDriverEraseRegion(ctl, len, 1) < 0) {
			return 0;
		}
		len += erase_size;
	}

	while (count) {
		if (count > MAX_KMALLOC_SIZE)
			len = MAX_KMALLOC_SIZE;
		else
			len = count;

		kbuf=kmalloc(len,GFP_KERNEL);
		if (!kbuf) {
			DBG_ERR("kmalloc is null\n");
			return -ENOMEM;
		}

		if (copy_from_user(kbuf, buf, len)) {
			kfree(kbuf);
			return -EFAULT;
		}

		switch (MTD_MODE(file)) {
		case MTD_MODE_OTP_FACT:
			ret = -EROFS;
			break;
		case MTD_MODE_OTP_USER:
			if (!mtd->write_user_prot_reg) {
				ret = -EOPNOTSUPP;
				break;
			}
			ret = mtd->write_user_prot_reg(mtd, start + *ppos, len, &retlen, kbuf);
			break;
		default:
#ifdef CONFIG_MTD_BRCMNAND
			ret = mtd_write_skip_badblock(mtd, *ppos, len, &retlen, kbuf);
#else
			ret = (*(mtd->write))(mtd, start + *ppos, len, &retlen, kbuf);
#endif
		}
		if (!ret) {
			*ppos += retlen;
			total_retlen += retlen;
			count -= retlen;
			buf += retlen;
		}
		else {
			kfree(kbuf);
			return ret;
		}

		kfree(kbuf);
	}

	return total_retlen;
}

static int mtdocap_ioctl_normal(struct inode *inode, struct file *file,
		     u_int cmd, u_long arg)
{
	struct ocap_struct * ctl ;
	struct mtd_info *mtd;
	void __user *argp = (void __user *)arg;
	int ret = 0;
	u_long size;
	struct mtd_info mtdocap;

	ctl = TO_MTDOCAP_CTL(file);
	mtd = TO_MTDOCAP(ctl);
	memcpy(&mtdocap, mtd, sizeof(struct mtd_info));

	size = (cmd & IOCSIZE_MASK) >> IOCSIZE_SHIFT;
	if (cmd & IOC_IN) {
		if (!access_ok(VERIFY_READ, argp, size))
			return -EFAULT;
	}
	if (cmd & IOC_OUT) {
		if (!access_ok(VERIFY_WRITE, argp, size))
			return -EFAULT;
	}
	DBG_INFO("ioctl cmd 0x%x\n", cmd);
	switch (cmd) {
	case MEMGETREGIONCOUNT:
		if (copy_to_user(argp, &(mtd->numeraseregions), sizeof(int)))
			return -EFAULT;
		break;

	case MEMGETREGIONINFO:
	{
		struct region_info_user ur;

		if (copy_from_user(&ur, argp, sizeof(struct region_info_user)))
			return -EFAULT;

		if (ur.regionindex >= mtd->numeraseregions)
			return -EINVAL;
		if (copy_to_user(argp, &(mtd->eraseregions[ur.regionindex]),
				sizeof(struct mtd_erase_region_info)))
			return -EFAULT;
		break;
	}

	case MEMGETINFO:

		mtdocap.size = ctl->physical_size;
		if (copy_to_user(argp, &mtdocap, sizeof(struct mtd_info_user)))
			return -EFAULT;
		break;

	case MEMERASE:
	{
		struct erase_info *erase;

		if(!(file->f_mode & 2))
			return -EPERM;

		erase=kmalloc(sizeof(struct erase_info),GFP_KERNEL);
		if (!erase)
			ret = -ENOMEM;
		else {
			wait_queue_head_t waitq;
			DECLARE_WAITQUEUE(wait, current);

			init_waitqueue_head(&waitq);

			memset (erase,0,sizeof(struct erase_info));
			if (copy_from_user(&erase->addr, argp,
				    sizeof(struct erase_info_user))) {
				kfree(erase);
				return -EFAULT;
			}
			erase->mtd = mtd;
			erase->callback = mtdocap_erase_callback;
			erase->priv = (unsigned long)&waitq;
			erase->addr += ctl->start;
			/*
			  FIXME: Allow INTERRUPTIBLE. Which means
			  not having the wait_queue head on the stack.

			  If the wq_head is on the stack, and we
			  leave because we got interrupted, then the
			  wq_head is no longer there when the
			  callback routine tries to wake us up.
			*/
			ret = mtd->erase(mtd, erase);
			if (!ret) {
				set_current_state(TASK_UNINTERRUPTIBLE);
				add_wait_queue(&waitq, &wait);
				if (erase->state != MTD_ERASE_DONE &&
				    erase->state != MTD_ERASE_FAILED)
					schedule();
				remove_wait_queue(&waitq, &wait);
				set_current_state(TASK_RUNNING);

				ret = (erase->state == MTD_ERASE_FAILED)?-EIO:0;
			}
			kfree(erase);
		}
		break;
	}

	case MEMWRITEOOB:
	{
		struct mtd_oob_buf buf;
		void *databuf;
		ssize_t retlen;

		if(!(file->f_mode & 2))
			return -EPERM;

		if (copy_from_user(&buf, argp, sizeof(struct mtd_oob_buf)))
			return -EFAULT;

		if (buf.length > 0x4096)
			return -EINVAL;

		if (!mtd->write_oob)
			ret = -EOPNOTSUPP;
		else
			ret = access_ok(VERIFY_READ, buf.ptr,
					buf.length) ? 0 : EFAULT;

		if (ret)
			return ret;

		databuf = kmalloc(buf.length, GFP_KERNEL);
		if (!databuf)
			return -ENOMEM;

		if (copy_from_user(databuf, buf.ptr, buf.length)) {
			kfree(databuf);
			return -EFAULT;
		}

		ret = (mtd->write_oob)(mtd, ctl->start + buf.start, buf.length, &retlen, databuf);

		if (copy_to_user(argp + sizeof(uint32_t), &retlen, sizeof(uint32_t)))
			ret = -EFAULT;

		kfree(databuf);
		break;

	}

	case MEMREADOOB:
	{
		struct mtd_oob_buf buf;
		void *databuf;
		ssize_t retlen;

		if (copy_from_user(&buf, argp, sizeof(struct mtd_oob_buf)))
			return -EFAULT;

		if (buf.length > 0x4096)
			return -EINVAL;

		if (!mtd->read_oob)
			ret = -EOPNOTSUPP;
		else
			ret = access_ok(VERIFY_WRITE, buf.ptr,
					buf.length) ? 0 : -EFAULT;

		if (ret)
			return ret;

		databuf = kmalloc(buf.length, GFP_KERNEL);
		if (!databuf)
			return -ENOMEM;

		ret = (mtd->read_oob)(mtd, ctl->start + buf.start, buf.length, &retlen, databuf);

		if (put_user(retlen, (uint32_t __user *)argp))
			ret = -EFAULT;
		else if (retlen && copy_to_user(buf.ptr, databuf, retlen))
			ret = -EFAULT;

		kfree(databuf);
		break;
	}

	case MEMLOCK:
	{
		struct erase_info_user info;

		if (copy_from_user(&info, argp, sizeof(info)))
			return -EFAULT;

		if (!mtd->lock)
			ret = -EOPNOTSUPP;
		else
			ret = mtd->lock(mtd, info.start, info.length);
		break;
	}

	case MEMUNLOCK:
	{
		struct erase_info_user info;

		if (copy_from_user(&info, argp, sizeof(info)))
			return -EFAULT;

		if (!mtd->unlock)
			ret = -EOPNOTSUPP;
		else
			ret = mtd->unlock(mtd, info.start, info.length);
		break;
	}

	case MEMSETOOBSEL:
	{
		if (copy_from_user(&mtd->oobinfo, argp, sizeof(struct nand_oobinfo)))
			return -EFAULT;
		break;
	}

	case MEMGETOOBSEL:
	{
		if (copy_to_user(argp, &(mtd->oobinfo), sizeof(struct nand_oobinfo)))
			return -EFAULT;
		break;
	}

	case MEMGETBADBLOCK:
	{
		loff_t offs;

		if (copy_from_user(&offs, argp, sizeof(loff_t)))
			return -EFAULT;
		if (!mtd->block_isbad)
			ret = -EOPNOTSUPP;
		else
			return mtd->block_isbad(mtd, offs);
		break;
	}

	case MEMSETBADBLOCK:
	{
		loff_t offs;

		if (copy_from_user(&offs, argp, sizeof(loff_t)))
			return -EFAULT;
		if (!mtd->block_markbad)
			ret = -EOPNOTSUPP;
		else
			return mtd->block_markbad(mtd, offs);
		break;
	}
 	case MEMGETOOBAVAIL:
 	{
 		if (copy_to_user(argp, &(mtd->oobavail), sizeof(mtd->oobavail)))
 			return -EFAULT;
 		break;
 	}

 	case MEMWRITEOOBFREE:
 	{
 		struct mtd_oob_buf buf;
 		void *databuf;
 		ssize_t retlen;

 		if(!(file->f_mode & 2))
 			return -EPERM;

 		if (copy_from_user(&buf, argp, sizeof(struct mtd_oob_buf)))
 			return -EFAULT;

 		if (buf.length > 0x4096)
 			return -EINVAL;

 		if (!mtd->write_oobfree)
 			ret = -EOPNOTSUPP;
 		else
 			ret = access_ok(VERIFY_READ, buf.ptr,
 					buf.length) ? 0 : EFAULT;

 		if (ret)
 			return ret;

 		databuf = kmalloc(buf.length, GFP_KERNEL);
 		if (!databuf)
 			return -ENOMEM;

 		if (copy_from_user(databuf, buf.ptr, buf.length)) {
 			kfree(databuf);
 			return -EFAULT;
 		}

//printk("Calling mtd->write_oobfree(buf.start=%08x, buf.length=%d\n", (unsigned long) buf.start, buf.length);
 		ret = (mtd->write_oobfree)(mtd, ctl->start + buf.start, buf.length, &retlen, databuf);

		/* THT: If there is a bad block (ret=-EBADMSG), the driver must have added it to BBT, so we should retry */
		if (ret == -EBADMSG) {
			printk("%s: MEMWRITEOOBFREE(start=%08x) returns %d, block will be marked bad\n",
				__FUNCTION__, (unsigned int) buf.start, ret);
			mtd->block_markbad(mtd, buf.start & ~(mtd->erasesize-1));
		}

if (ret)

 		if (copy_to_user(argp + sizeof(uint32_t), &retlen, sizeof(uint32_t)))
 			ret = -EFAULT;

 		kfree(databuf);
 		break;

 	}


 	case MEMREADOOBFREE:
 	{
 		struct mtd_oob_buf buf;
 		void *databuf;
 		ssize_t retlen;

 		if (copy_from_user(&buf, argp, sizeof(struct mtd_oob_buf)))
 			return -EFAULT;

 		if (buf.length > 0x4096)
 			return -EINVAL;

 		if (!mtd->read_oobfree)
 			ret = -EOPNOTSUPP;
 		else
 			ret = access_ok(VERIFY_WRITE, buf.ptr,
 					buf.length) ? 0 : -EFAULT;

 		if (ret)
 			return ret;

 		databuf = kmalloc(buf.length, GFP_KERNEL);
 		if (!databuf)
 			return -ENOMEM;

 		ret = (mtd->read_oobfree)(mtd, ctl->start + buf.start, buf.length, &retlen, databuf);

 		if (put_user(retlen, (uint32_t __user *)argp))
 			ret = -EFAULT;
 		else if (retlen && copy_to_user(buf.ptr, databuf, retlen))
 			ret = -EFAULT;

 		kfree(databuf);
 		break;
 	}


#ifdef CONFIG_MTD_BRCMNAND
	case MTD_BRCMNAND_READNORFLASH:
	{
		struct brcmnand_readNorFlash buf;
		struct brcmnand_readNorFlash* pUserArg = (struct brcmnand_readNorFlash*) argp;

 		void *databuf;
 		int retVal;

 		if (copy_from_user(&buf, argp, sizeof(struct brcmnand_readNorFlash)))
 			return -EFAULT;

 		ret = access_ok(VERIFY_WRITE, buf.buff,
 					buf.len) ? 0 : -EFAULT;

 		if (ret)
 			return ret;

		/* Make sure that buff, offset and len are word (16bit) aligned */
		if (((unsigned int) buf.buff & 0x1) || (buf.offset & 0x1) || (buf.len & 0x1)) {
			printk("MTD_BRCMNAND_READNORFLASH: buffer, offset and length must be word aligned\n");
			return (-EINVAL);
		}

 		databuf = vmalloc(buf.len);
 		if (!databuf)
 			return -ENOMEM;

 		retVal = brcmnand_readNorFlash(mtd, databuf, buf.offset, buf.len);

 		if (put_user(retVal, (int __user *) &pUserArg->retVal))
 			ret = -EFAULT;
 		else if (0 == retVal && copy_to_user(buf.buff, databuf, buf.len))
 			ret = -EFAULT;

 		vfree(databuf);
 		break;
	}
#endif


#if defined(CONFIG_MTD_OTP) || defined(CONFIG_MTD_ONENAND_OTP)
	case OTPSELECT:
	{
		int mode;
		if (copy_from_user(&mode, argp, sizeof(int)))
			return -EFAULT;
		SET_MTD_MODE(file, 0);
		switch (mode) {
		case MTD_OTP_FACTORY:
			if (!mtd->read_fact_prot_reg)
				ret = -EOPNOTSUPP;
			else
				SET_MTD_MODE(file, MTD_MODE_OTP_FACT);
			break;
		case MTD_OTP_USER:
			if (!mtd->read_fact_prot_reg)
				ret = -EOPNOTSUPP;
			else
				SET_MTD_MODE(file, MTD_MODE_OTP_USER);
			break;
		default:
			ret = -EINVAL;
		case MTD_OTP_OFF:
			break;
		}
		file->f_pos = 0;
		break;
	}

	case OTPGETREGIONCOUNT:
	case OTPGETREGIONINFO:
	{
		struct otp_info *buf = kmalloc(4096, GFP_KERNEL);
		if (!buf)
			return -ENOMEM;
		ret = -EOPNOTSUPP;
		switch (MTD_MODE(file)) {
		case MTD_MODE_OTP_FACT:
			if (mtd->get_fact_prot_info)
				ret = mtd->get_fact_prot_info(mtd, buf, 4096);
			break;
		case MTD_MODE_OTP_USER:
			if (mtd->get_user_prot_info)
				ret = mtd->get_user_prot_info(mtd, buf, 4096);
			break;
		}
		if (ret >= 0) {
			if (cmd == OTPGETREGIONCOUNT) {
				int nbr = ret / sizeof(struct otp_info);
				ret = copy_to_user(argp, &nbr, sizeof(int));
			} else
				ret = copy_to_user(argp, buf, ret);
			if (ret)
				ret = -EFAULT;
		}
		kfree(buf);
		break;
	}

	case OTPLOCK:
	{
		struct otp_info info;

		if (MTD_MODE(file) != MTD_MODE_OTP_USER)
			return -EINVAL;
		if (copy_from_user(&info, argp, sizeof(info)))
			return -EFAULT;
		if (!mtd->lock_user_prot_reg)
			return -EOPNOTSUPP;
		ret = mtd->lock_user_prot_reg(mtd, info.start, info.length);
		break;
	}
#endif

	default:
		ret = -ENOTTY;
	}

	return ret;

}

#endif /* LINUX_VERSION_CODE > KERNEL_VERSION(2,6,17) */

/* ========== */
static int mtdocap_open(struct inode *inode, struct file *file)
{
	struct ocap_struct * ctl;
	unsigned int minor = iminor(inode);
	if (minor >= MAX_OCAP_DEV)
		return -ENODEV;
	ctl = &ocap_ctl[minor];
	return ctl->fops->open(inode, file);
}
static int mtdocap_close(struct inode *inode, struct file *file)
{
	struct ocap_struct * ctl;
	unsigned int minor = iminor(inode);
	if (minor >= MAX_OCAP_DEV)
		return -ENODEV;
	ctl = &ocap_ctl[minor];
	return ctl->fops->release(inode, file);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37)
static long mtdocap_unlocked_ioctl(struct file *file, u_int cmd, u_long arg)
{
	struct inode *inode = file->f_path.dentry->d_inode;
	unsigned int minor = iminor(inode);
	struct ocap_struct * ctl;
	long ret;

	down_interruptible(&flush_sema);
	if (minor >= MAX_OCAP_DEV)
		return -ENODEV;
	ctl = &ocap_ctl[minor];
	ret = ctl->fops->unlocked_ioctl(file, cmd, arg);
	up(&flush_sema);

	return ret;
 }
#else
static int mtdocap_ioctl(struct inode *inode, struct file *file, u_int cmd, u_long arg)
{
	struct ocap_struct * ctl;
	unsigned int minor = iminor(inode);
	if (minor >= MAX_OCAP_DEV)
		return -ENODEV;
	ctl = &ocap_ctl[minor];
	return ctl->fops->ioctl(inode, file, cmd, arg);
}
#endif

static ssize_t mtdocap_write(struct file *file, const char __user *buf, size_t count,loff_t *ppos)
{
	struct ocap_struct * ctl = TO_MTDOCAP_CTL(file);
	if (ctl->devnum >= MAX_OCAP_DEV)
		return -EFAULT;
	return ctl->fops->write(file, buf, count, ppos);
}
static ssize_t mtdocap_read(struct file *file, char __user *buf, size_t count,loff_t *ppos)
{
	struct ocap_struct * ctl = TO_MTDOCAP_CTL(file);
	if (ctl->devnum >= MAX_OCAP_DEV)
		return -EFAULT;
	return ctl->fops->read(file, buf, count, ppos);
}
static loff_t mtdocap_llseek (struct file *file, loff_t offset, int orig)
{
	struct ocap_struct * ctl = TO_MTDOCAP_CTL(file);
	if (ctl->devnum >= MAX_OCAP_DEV)
		return -EFAULT;
	return ctl->fops->llseek(file, offset, orig);
}

#ifdef CONFIG_PROC_FS
static struct proc_dir_entry *proc_mtdocap;

static int mtdocap_read_proc (char *page, char **start, off_t off, int count,
			  int *eof, void *data_unused)
{
	int len, l, i;
        off_t   begin = 0;
	char * buf;
	struct ocap_struct * ctl;

	len = sprintf(page, "dev:      start    phy-size size     erasesize  name\n");
        for (i=0; i< MAX_OCAP_DEV; i++) {
		ctl = &ocap_ctl[i];
		buf = page + len;
		l = sprintf(buf, "%s%d: %8.8x %8.8x %8.8x %8.8x \"/dev/mtd%d:%s\"\n", OCAPDEV, i, ctl->start,
			    ctl->physical_size, ctl->size, ctl->erase_size, ctl->devnum, ctl->name);
                len += l;
                if (len+begin > off+count)
                        goto done;
                if (len+begin < off) {
                        begin += len;
                        len = 0;
                }
        }

        *eof = 1;

done:
        if (off >= len+begin)
                return 0;
        *start = page + (off-begin);
        return ((count < begin+len-off) ? count : begin+len-off);
}

#endif /* CONFIG_PROC_FS */

static struct file_operations mtdocap_fops = {
	.owner		= THIS_MODULE,
	.llseek		= mtdocap_llseek,
	.read		= mtdocap_read,
	.write		= mtdocap_write,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37)
	.unlocked_ioctl = mtdocap_unlocked_ioctl,
#else
	.ioctl		= mtdocap_ioctl,
#endif
	.open		= mtdocap_open,
	.release	= mtdocap_close,
};

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37)
static struct dentry *mtd_inodefs_mount(struct file_system_type *fs_type,
				int flags, const char *dev_name, void *data)
{
	return mount_pseudo(fs_type, "mtdocap_inode:", NULL, MTD_INODE_FS_MAGIC);
}

static struct file_system_type mtd_inodefs_type = {
       .name = "mtdocap_inodefs",
       .mount = mtd_inodefs_mount,
       .kill_sb = kill_anon_super,
};

static void mtdchar_notify_add(struct mtd_info *mtd)
{
}

static void mtdchar_notify_remove(struct mtd_info *mtd)
{
	struct inode *mtd_ino = ilookup(mtd_inode_mnt->mnt_sb, mtd->index);

	if (mtd_ino) {
		/* Destroy the inode if it exists */
		mtd_ino->i_nlink = 0;
		iput(mtd_ino);
	}
}

static struct mtd_notifier mtdchar_notifier = {
	.add = mtdchar_notify_add,
	.remove = mtdchar_notify_remove,
};
#endif

static int __init init_mtdocap(void)
{
	int ret = -ENODEV;
	int minor = 0;
	struct ocap_struct * ctl;
	struct mtd_info *mtd;
	char str[64];
#if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,18)
#ifdef CONFIG_MTD_NEW_PARTITION
	int ocap_offset = 0;
	int e; // Index into Env vars
#else
	int scale = 0;
#endif
#endif
	DBG_INFO("init mtdocap\n");
	printk(KERN_INFO "mtdocap: Broadcom MTDOCAP driver " VER_STR "\n");

#if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,18)
#ifdef CONFIG_MTD_NEW_PARTITION
	/* find the ocap partition info */
	/* gCfeEnvVarPairs should be export by future kernel */
	for (e=0; e < gCfePartitions.numParts; e++) {
		int p = gCfePartitions.parts[e].part;
		if (p == OCAP_PT ) {
			DBG_INFO("CFE environment variable found: %s: %08x, %s: %08x\n",
				 "OCAP_PART_STARTAD", gCfePartitions.parts[e].offset,
				 "OCAP_PART_SIZE", gCfePartitions.parts[e].size);
			if (gCfePartitions.parts[e].size != (DEFAULT_TOTAL_OCAP_SIZE)) {
				DBG_ERR("Invalid OCAP size from CFE : %s: %08x, expect %08x\n",
					"OCAP_PART_SIZE", gCfePartitions.parts[e].size,
					(DEFAULT_TOTAL_OCAP_SIZE));
				goto err;
			}
			ocap_offset = gCfePartitions.parts[e].offset;
			break;
		}
	}
	if (e == gCfePartitions.numParts) {
		DBG_WRN("CFE environment variable not found: %s %s, use offset %08x\n",
			"OCAP_PART_STARTAD", "OCAP_PART_SIZE", ocap_offset);
		DBG_WRN("CFE and Linux flash partition may be out of sync\n");
	}
#endif
#endif

	memset(ocap_ctl, 0, sizeof(ocap_ctl));

	/* check to see if /dev/mtd1 (entire_device) exists */
	for (minor = 0; minor < MAX_OCAP_DEV; minor++) {
		int j;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37)
		for (j = 0;; j++) {
#else
		for (j = 0; j < MAX_MTD_DEVICES; j++) {
#endif
			mtd = get_mtd_device(NULL, j);
			if (!mtd) continue;
			if (strncmp(ocap_dev[minor].mtd_name, mtd->name, 32) == 0) {
				ocap_dev[minor].devnum = mtd->index;
				DBG_INFO("Found /dev/mtd%d(%s) for %s\n", mtd->index, mtd->name,
					 ocap_dev[minor].name);
				put_mtd_device(mtd);
				break;
			}
			put_mtd_device(mtd);
		}
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,37)
		if (j == MAX_MTD_DEVICES)
			DBG_WRN("Cannot find mtd device for %s\n", ocap_dev[minor].name);
#endif
	}

	/* zero size partition */
//	if (mtd->sync)
//		mtd->sync(mtd);
//	put_mtd_device(mtd);

	for (minor = 0; minor < MAX_OCAP_DEV; minor++) {
		ctl = &ocap_ctl[minor];
		sprintf(ctl->name, "%s", ocap_dev[minor].name);
		ctl->devnum = ocap_dev[minor].devnum;
		ctl->fops= ocap_dev[minor].fops;
		ctl->size = ocap_dev[minor].size; /* size cannot expand with flash size, since malloc may fail for large flash */
		ctl->minSegments = 1;
#ifdef CONFIG_MTD_BRCMNAND
		ctl->nand_buf_size = ocap_dev[minor].size;
#endif
		mtd = get_mtd_device(NULL, ctl->devnum);


		if (!mtd) {
			put_mtd_device(mtd);
			ret =  -ENODEV;
			goto err;
		}
		if (MTD_ABSENT == mtd->type) {
			put_mtd_device(mtd);
			ret =  -ENODEV;
			goto err;
		}

		if (mtd->size && ocap_dev[minor].partition_size) {
			ctl->erase_size = mtd->erasesize;
#if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,18)
#ifdef CONFIG_MTD_NEW_PARTITION
			ctl->physical_size = ocap_dev[minor].physical_size;
			ctl->start = ocap_dev[minor].start + ocap_offset;
#else
			scale = mtd->size / ocap_dev[minor].partition_size;
			scale = (scale) ? scale : 1;
			ctl->physical_size = ocap_dev[minor].physical_size * scale;
			ctl->start = ocap_dev[minor].start * scale;
#endif
#else
			ctl->physical_size = ocap_dev[minor].physical_size;
			ctl->start = ocap_dev[minor].start;
#endif
			if (mtd->sync)
				mtd->sync(mtd);
			put_mtd_device(mtd);
		} else {
			/* zero size partition */
			if (mtd->sync)
				mtd->sync(mtd);
			put_mtd_device(mtd);
		}

		if ( (strncmp(ctl->name, "dyncfg", 64) == 0) || (strncmp(ctl->name, "permcfg", 64) == 0)) {
			ctl->buf = kmalloc(ctl->size, GFP_KERNEL);
			if (!ctl->buf) {
				DBG_ERR("ctl->buf out of memory!\n");
				ret = -ENOMEM;
				goto err;
			}
#ifdef CONFIG_MTD_BRCMNAND
			ctl->nand_buf = kmalloc(ctl->nand_buf_size, GFP_KERNEL);
			if (!ctl->nand_buf) {
				DBG_ERR("ctl->nand_buf out of memory!\n");
				ret = -ENOMEM;
				goto err;
			}
			ctl->nand_buf_ready = 0;
#endif
			init_waitqueue_head(&ctl->wait);
			sprintf(str, "%s", ctl->name);
			ctl->flush_thread = kthread_run(flush_func, ctl, str);
			if (IS_ERR(ctl->flush_thread)) {
				goto err;
			}
		}

	}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37)
	ret = __register_chrdev(MTD_OCAP_MAJOR, 0, 1 << MINORBITS, OCAPDEV, &mtdocap_fops);
	if (ret < 0) {
		DBG_ERR("Can't allocate major number %d for "
				"Memory Technology Devices.\n", MTD_CHAR_MAJOR);
		return ret;
	}

	ret = register_filesystem(&mtd_inodefs_type);
	if (ret) {
		pr_notice("Can't register mtd_inodefs filesystem: %d\n", ret);
		goto err_unregister_chdev;
	}

	mtd_inode_mnt = kern_mount(&mtd_inodefs_type);
	if (IS_ERR(mtd_inode_mnt)) {
		ret = PTR_ERR(mtd_inode_mnt);
		pr_notice("Error mounting mtd_inodefs filesystem: %d\n", ret);
		goto err_unregister_filesystem;
	}
	register_mtd_user(&mtdchar_notifier);
#else
	if ((ret = register_chrdev(MTD_OCAP_MAJOR, OCAPDEV, &mtdocap_fops))) {
		DBG_ERR("unable to get major %d for misc devices\n",
		       MTD_OCAP_MAJOR);
		ret = -EIO;
		goto err;
	}
#endif

#ifdef CONFIG_PROC_FS
	if ((proc_mtdocap = create_proc_entry( OCAPDEV, 0, NULL )))
		proc_mtdocap->read_proc = mtdocap_read_proc;
#endif


	DBG_INFO("%s initalized\n", OCAPDEV);
	return 0;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37)
err_unregister_filesystem:
	unregister_filesystem(&mtd_inodefs_type);
err_unregister_chdev:
	__unregister_chrdev(MTD_CHAR_MAJOR, 0, 1 << MINORBITS, OCAPDEV);
#endif
err:
	DBG_ERR("%s failed to initialize\n", OCAPDEV);
	for (minor = 0; minor < MAX_OCAP_DEV; minor++) {
		ctl = &ocap_ctl[minor];
		if (IS_ERR(ctl->flush_thread)) {
			kthread_stop(ctl->flush_thread);
		}
		if (ctl->buf) {
			kfree(ctl->buf);
			ctl->buf = NULL;
		}
#ifdef CONFIG_MTD_BRCMNAND
		if (ctl->nand_buf) {
			kfree(ctl->nand_buf);
			ctl->nand_buf = NULL;
		}
		ctl->nand_buf_ready = 0;
#endif
	}
	return ret;

}

static void __exit cleanup_mtdocap(void)
{
	int minor;
	struct ocap_struct * ctl;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37)
	unregister_mtd_user(&mtdchar_notifier);
	mntput(mtd_inode_mnt);
	unregister_filesystem(&mtd_inodefs_type);
	__unregister_chrdev(MTD_OCAP_MAJOR, 0, 1 << MINORBITS, OCAPDEV);
#else
	unregister_chrdev(MTD_OCAP_MAJOR, OCAPDEV);
#endif
#ifdef CONFIG_PROC_FS
        if (proc_mtdocap)
		remove_proc_entry( OCAPDEV, NULL);
#endif
	for (minor = 0; minor < MAX_OCAP_DEV; minor++) {
		ctl = &ocap_ctl[minor];
		if (ctl->flush_thread)
			kthread_stop(ctl->flush_thread);
		if (ctl->buf) {
			kfree(ctl->buf);
			ctl->buf = NULL;
		}
#ifdef CONFIG_MTD_BRCMNAND
		if (ctl->nand_buf) {
			kfree(ctl->nand_buf);
			ctl->nand_buf = NULL;
		}
		ctl->nand_buf_ready = 0;
#endif
	}

#ifdef CONFIG_MTD_BRCMNAND
	if (nand_block_buf != NULL)  {
		kfree(nand_block_buf);
		nand_block_buf = NULL;
	}
#endif
	DBG_INFO("%s removed\n", OCAPDEV);
}

module_init(init_mtdocap);
module_exit(cleanup_mtdocap);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("");
MODULE_DESCRIPTION("Direct access to MTD ocap devices");
