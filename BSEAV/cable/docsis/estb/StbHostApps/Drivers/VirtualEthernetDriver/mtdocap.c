/****************************************************************************
*
*  Copyright (c) 2011-2012 Broadcom Corporation
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
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37) 
#include <generated/autoconf.h>
#elif ( LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30) ) && ( LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 37) )
#include <linux/autoconf.h>
#endif
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/mtd/mtd.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 37)
#include <linux/mtd/compatmac.h>
#endif
#include <linux/kthread.h>
#include <asm/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/vmalloc.h>
#include <asm/brcmstb/brcmstb.h>
#include <asm/brcmstb/brcmapi.h>

#define VERSION     "1.10"

#define VER_STR     "v" VERSION " " __DATE__ " " __TIME__

#define MTD_OCAP_MAJOR    98 /* not used by anyone */

#define TO_MTDOCAP_CTL(file) (ocap_struct *)((long)((file)->private_data) & ~3L)
#define TO_MTDOCAP(ctl) (struct mtd_info *)((ocap_struct *)(ctl))->mtd

#define DBG_INFO(arg...) if (debug) printk(arg)
#define DBG_ERR(arg...) printk(arg)

#define SEG_EMPTY  -1
#define SEG_INVAL  -2
#define SEG_FULL    1
#define SEG_OK      0

#define SEEK_SET  0
#define SEEK_CUR  1
#define SEEK_END  2

#define STATIC static

//********************** Local Types and variables *****************************************

STATIC unsigned short debug = 0;
module_param(debug, ushort, S_IRUGO);

STATIC unsigned long docsishighram = 0xFE00000;
module_param(docsishighram, ulong, S_IRUGO);
#define DOCSIS_HIGH_RAM_SIZE ((2 * 1024 * 1024) - (64 * 1024))

STATIC unsigned int perm[5] = { -1 };
STATIC int permCount = 0;
module_param_array(perm, int, &permCount, 0000);

STATIC unsigned int dyn[5] = { -1 };
STATIC int dynCount = 0;
module_param_array(dyn, int, &dynCount, 0000);

STATIC unsigned int docsis0[5] = { -1 };
STATIC int docsis0Count = 0;
module_param_array(docsis0, int, &docsis0Count, 0000);

STATIC unsigned int docsis1[5] = { -1 };
STATIC int docsis1Count = 0;
module_param_array(docsis1, int, &docsis1Count, 0000);

#define MAX_MESSAGE_SIZE        1458
#define TFTP_BLOCK_SIZE         512

#define OCAPDEV     "mtdocap"

typedef struct {
	/* fields updated by init */
	char name[64];
	unsigned int devnum;
	unsigned int erase_size;
	unsigned int size;
	unsigned int start;
	unsigned int physical_size;
	unsigned int nand_buf_size;     /* nand flash has bad sectors, this size is the actual size used by ocap */
	unsigned int pos;               /* offset in the segment, it increases with every read/write */
	unsigned int writesize;
	uint8_t *nand_buf;      		/* nand flash is r/w in sectors, this buffer holds the data before flush to flash */
	uint8_t *buf;
	struct task_struct *flush_thread;
	wait_queue_head_t wait;
	struct file_operations *fops;
	struct mtd_info *mtd;
	enum mtd_file_modes mode;   	/* fields cleaned by flush thread */
	unsigned char flush : 1;
	unsigned char flush_error : 1;
	unsigned char dirty : 1;        /* cache is dirty, data should be read from cache */
	unsigned char nand_buf_ready : 1; /* the data in nand_buf is ready for use */
	unsigned char partition_erased : 1; /* after driver loaded, before first write, need to erase the whole partition */
	unsigned char isNAND : 1;
	unsigned char docsiscopy : 1;   /* this partition must be copied to high memory */
} ocap_struct;

typedef struct {
	char name[32];
	int minor;          /* mtdocap minor device */
	int devnum;         /* mtd device number */
	int start;          /* sub-partition start addr, assuming a 64M flash */
	int physical_size;  /* sub-partition size , assuming a 64M flash*/
	int size;           /* actual size used, no matter how big the partition is */
	int optional;
	int *parmCount;
	int *parms;
	struct file_operations *fops;
} ocap_partition;

#define MTD_MODE(file)		(((ocap_struct *)((file)->private_data))->mode)

STATIC int mtdocap_open_normal(struct inode *inode, struct file *file);
STATIC int mtdocap_close_normal(struct inode *inode, struct file *file);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 37)
STATIC int mtdocap_ioctl_normal(struct inode *inode, struct file *file, u_int cmd, u_long arg);
#else
STATIC long mtdocap_ioctl_normal(struct file *file, u_int cmd, u_long arg);
#endif
STATIC ssize_t mtdocap_write_normal(struct file *file, const char __user *buf, size_t count,loff_t *ppos);
STATIC ssize_t mtdocap_read_normal(struct file *file, char __user *buf, size_t count,loff_t *ppos);
STATIC loff_t mtdocap_llseek_normal(struct file *file, loff_t offset, int orig);

STATIC int mtdocap_open_nonvol(struct inode *inode, struct file *file);
STATIC int mtdocap_close_nonvol(struct inode *inode, struct file *file);
STATIC ssize_t mtdocap_write_nonvol(struct file *file, const char __user *buf, size_t count,loff_t *ppos);
STATIC ssize_t mtdocap_read_nonvol(struct file *file, char __user *buf, size_t count,loff_t *ppos);
STATIC loff_t mtdocap_llseek_nonvol(struct file *file, loff_t offset, int orig);

/* 
 * The physical size is based on a 64M flash system. 
 * For 32M flash, the physical size is the same.
 * For bigger size flash, it is scaled by <flash size in MB> / 64
 */

struct file_operations mtdocap_fops_normal = {
    .llseek			= mtdocap_llseek_normal,
    .read			= mtdocap_read_normal,
    .write			= mtdocap_write_normal,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 37)
	.ioctl			= mtdocap_ioctl_normal,
#else
    .unlocked_ioctl	= mtdocap_ioctl_normal,
#endif
    .open			= mtdocap_open_normal,
    .release		= mtdocap_close_normal,
};

struct file_operations mtdocap_fops_nonvol = {
    .llseek		= mtdocap_llseek_nonvol,
    .read		= mtdocap_read_nonvol,
    .write		= mtdocap_write_nonvol,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 37)
	.ioctl			= mtdocap_ioctl_normal, /* for erase */
#else
    .unlocked_ioctl	= mtdocap_ioctl_normal,
#endif
    .open		= mtdocap_open_nonvol,
    .release	= mtdocap_close_nonvol
};

#define MAX_DEVS 4

ocap_partition ocap_devs[MAX_DEVS] = { /* name, devnum, start, physical_size, size, optional, fops */
	{"dyncfg", -1, 0, 0, 0, 0, 0, &dynCount, dyn, &mtdocap_fops_nonvol},
    {"permcfg", -1, 0, 0, 0, 0, 0, &permCount, perm, &mtdocap_fops_nonvol},
    {"docsis0", -1, 0, 0, 0, 0, 0, &docsis0Count, docsis0, &mtdocap_fops_normal},
    {"docsis1", -1, 0, 0, 0, 0, 1, &docsis1Count, docsis1, &mtdocap_fops_normal}
};

STATIC uint8_t *nand_block_buf = NULL;

#define MAX_OCAP_MINOR_DEV 16

STATIC ocap_struct ocap_ctl[MAX_OCAP_MINOR_DEV];

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 37)
DECLARE_MUTEX (flush_mutex);
#else
DEFINE_SEMAPHORE (flush_mutex);
#endif
//********************** Local Functions *************************************

STATIC void flash_erase_callback(struct erase_info *instr)
{
	wake_up((wait_queue_head_t *)instr->priv);
}

STATIC int flash_erase(struct mtd_info *mtd, struct erase_info *erase)
{
	int ret = 0;
	wait_queue_head_t waitq;
	DECLARE_WAITQUEUE(wait, current);

	init_waitqueue_head(&waitq);

	erase->mtd = mtd;
	erase->callback = flash_erase_callback;
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
	if (!ret)
	{
		set_current_state(TASK_UNINTERRUPTIBLE);
		add_wait_queue(&waitq, &wait);
		if (erase->state != MTD_ERASE_DONE &&
			erase->state != MTD_ERASE_FAILED)
			schedule();
		remove_wait_queue(&waitq, &wait);
		set_current_state(TASK_RUNNING);
		ret = (erase->state == MTD_ERASE_FAILED) ? -EIO : 0;
		// Due to early version driver bug we don't get an indication that the erase failed for NAND
		// but it does mark the block as bad, so check that.
		if ((ret == 0) && mtd->block_isbad)
		{
			ret = mtd->block_isbad(mtd, erase->addr) ? -EIO : 0;
		}
	}
	return ret;
}

STATIC int flash_write(struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen, const u_char *buf)
{
	int ret = mtd->write(mtd, to, len, retlen, buf);
	if (ret == 0)
	{
		char *tbuf = kmalloc(len, GFP_KERNEL);
		size_t rlen;
		if (buf)
		{
			ret = mtd->read(mtd, to, len, &rlen, tbuf);
			if (ret < 0)
			{
				unsigned int badblock = to & ~(mtd->erasesize - 1);
				DBG_ERR("Critical error, bad read after succesful write at %08x for len %08x, ret = %d\n",
                    (unsigned int)to, len, ret);
				DBG_ERR("Marking block bad at %08x\n", badblock);
				mtd->block_markbad(mtd, badblock);
			}
			kfree(tbuf);
		}
	}
	return ret;
}

STATIC int flash_read_nand(ocap_struct *ctl, struct mtd_info *mtd, int offset, size_t len, size_t *retlen, u_char *buf)
{
	int ret;

	if (len > ctl->erase_size)
	{
		DBG_ERR("Logic error - function flash_read_nand should never be invoked to read more than %d bytes, called with %d\n",
            ctl->erase_size, len);
		return -EIO;
	}
	ret = mtd->read(mtd, offset, len, retlen, buf);
#if 0
    if (corrected != mtd->ecc_stats.corrected) {
        unsigned int eraseOffset = offset & ~(ctl->erase_size - 1);
        unsigned int blockOffset = offset & (ctl->erase_size - 1);
        DBG_INFO("Correctable at %08x, error count before %d, after %d\n", offset, corrected, mtd->ecc_stats.corrected);
	    // need to refresh after correctable errors
	    u_char *buffer;
	    int rlen;
	    DBG_INFO("Refreshing at %08x after recoverable error\n", blockOffset);
	    buffer = kmalloc(ctl->erase_size + ctl->writesize, GFP_KERNEL);
	    if (buffer == 0) {
		    DBG_ERR("Unable to get refresh buffer\n");
		    return ret;
	    }
        // read erase region up to read part
        if (blockOffset) {
            DBG_INFO("Reading at offset %08x, len %08x, to %08x\n", eraseOffset, blockOffset, 0);
            if (mtd->read(mtd, eraseOffset, blockOffset, &rlen, buffer) < 0) {
                DBG_ERR("Unable to read erase buffer for refresh at %08x\n", eraseOffset);
                goto free;
            }
        }
        // copy read part
        DBG_INFO("Copying len %08x, to %08x\n", len, blockOffset);
        memcpy(buffer + blockOffset, buf, len);
        // read erase region after read part
        if (ctl->erase_size - (blockOffset + len)) {
            DBG_INFO("Reading at offset %08x, len %08x, to %08x\n",
                eraseOffset + blockOffset + len, ctl->erase_size - (blockOffset + len), blockOffset + len);
            if (mtd->read(mtd, eraseOffset + blockOffset + len, ctl->erase_size - (blockOffset + len), &rlen, buffer + blockOffset + len) < 0) {
                DBG_ERR("Unable to read erase buffer for refresh at %08x\n", eraseOffset);
                goto free;
            }
        }
        DBG_ERR("Erasing for refresh at %08x\n", eraseOffset);
        erase.addr = eraseOffset;
        erase.len = mtd->erasesize;
        ret = flash_erase(mtd, &erase);
	    if (ret < 0) {
		    DBG_ERR("Unable to erase for refresh at %08x\n", eraseOffset);
		    goto free;
	    }
        memset(buffer + ctl->erase_size, 0xff, ctl->writesize);
        for (blockOffset = 0; blockOffset < ctl->erase_size; blockOffset += ctl->writesize) {
            // Only write to blocks that contain data
            if (memcmp(buffer + blockOffset, buffer + ctl->erase_size, ctl->writesize)) {
                DBG_INFO("writing at offset %08x, len %08x, from %08x\n",
                    eraseOffset + blockOffset, ctl->writesize, blockOffset);
                if ((ret = flash_write(mtd, eraseOffset + blockOffset, ctl->writesize, &rlen, buffer + blockOffset)) < 0) {
                    DBG_ERR("Unable to write for refresh at %08x\n", eraseOffset + blockOffset);
                    goto free;
                }
            }
        }
        ret = 0;
free:
	    kfree(buffer);
    }
#endif
	if (ret)
		DBG_ERR("NAND read error at %08x, ret = %d\n", offset, ret);
	return ret;
}

STATIC int mtd_read_skip_badblock(ocap_struct *ctl, int from, size_t len, size_t *retlen, u_char *buf)
{
	struct mtd_info *mtd = TO_MTDOCAP(ctl);
	unsigned int offset, bytes_read, to_read, sectorsize, erasesize;
	unsigned int blockstart = 1, badblock = 0;
	int ret = 0;
	unsigned int badblockoffset = 0;

	sectorsize = mtd->writesize;
	if (sectorsize == 0 || mtd->block_isbad == NULL)
	{
		DBG_ERR("mtd_read_skip_badblock unsupported operation\n");
		return -EOPNOTSUPP;
	}
	erasesize = mtd->erasesize;
	for (offset = ctl->start;
		(offset < ctl->start + ctl->physical_size) && (offset < ctl->start + from + badblockoffset);
		offset += sectorsize)
	{
		if (blockstart != (offset & (~(erasesize - 1))))
		{
			blockstart = offset & (~(erasesize - 1));
			badblock = mtd->block_isbad(mtd, blockstart);
			if (badblock)
			{
				/* skip the whole erase sector */
				DBG_INFO("Bad block at %08x size %08x skipped for read, case 1\n",
                    (uint32_t)blockstart, (uint32_t)erasesize);
				badblockoffset += erasesize;
			}
		}
	}
	if (offset >= ctl->start + ctl->physical_size)
	{
		DBG_ERR("mtd_read_skip_badblock start offset after bad block skip is out of range %08x\n", offset);
		return -EINVAL;
	}
	bytes_read = 0;
	for (offset = ctl->start + from + badblockoffset;
		offset < ctl->start + ctl->physical_size;
		offset += sectorsize)
	{
		if (blockstart != (offset & (~(erasesize - 1))))
		{
			blockstart = offset & (~(erasesize - 1));
			badblock = mtd->block_isbad(mtd, blockstart);
			if (badblock)
			{
				DBG_INFO("Bad block at %08x size %08x, skipped for read, case 2\n",
                    (uint32_t)blockstart, (uint32_t)erasesize);
			}
		}
		if (badblock)
		{
			/* skip the whole erase sector */
			continue;
		}

		to_read = (bytes_read + sectorsize > len) ? (len - bytes_read) : sectorsize;
		ret = flash_read_nand(ctl, mtd, offset, to_read, retlen, buf + bytes_read);
		if (ret < 0)
		{
			*retlen += bytes_read;
			DBG_ERR("Failed to read from NAND flash at %08x for %08x\n", offset, to_read);
			return ret;
		}
		if (*retlen != to_read)
		{
			*retlen += bytes_read;
			DBG_ERR("Not all data are read from NAND flash, read %d, expect %d\n", *retlen, sectorsize);
			ret = -EINVAL;
			return ret;
		}
		bytes_read += to_read;
		if (bytes_read >= len)
			break;
	}
	if (bytes_read != len)
	{
		*retlen = bytes_read;
		DBG_ERR("%s error reading NAND flash, read %x, expect %x\n", __FUNCTION__, (uint32_t)*retlen, (uint32_t)len);
		ret = -EINVAL;
		return ret;
	}
	*retlen = bytes_read;
	return ret;
}

STATIC int mtd_write_skip_badblock(ocap_struct *ctl, int from, size_t len, size_t *retlen, u_char *buf)
{
	int ret = 0;
	struct mtd_info *mtd = TO_MTDOCAP(ctl);
	unsigned int offset, sectorsize, bytes_written, to_write, erasesize;
	unsigned int blockstart = 1;
	unsigned int badblock = 0;
	unsigned int badblockoffset = 0;

	sectorsize = mtd->writesize;
	if (sectorsize == 0 || mtd->block_isbad == NULL)
		return -EOPNOTSUPP;
	erasesize = mtd->erasesize;
	for (offset = ctl->start;
		(offset < ctl->start + ctl->physical_size) && (offset < ctl->start + from + badblockoffset);
		offset += sectorsize)
	{
		if (blockstart != (offset & (~(erasesize - 1))))
		{
			blockstart = offset & (~(erasesize - 1));
			badblock = mtd->block_isbad(mtd, blockstart);
			if (badblock)
			{
				/* skip the whole erase sector */
				DBG_INFO("Bad block %08x size %08x, skipped for write, case 1\n",
                    (uint32_t)blockstart, (uint32_t)erasesize);
				badblockoffset += erasesize;
			}
		}
	}
	if (offset >= ctl->start + ctl->physical_size)
		return -ENOSPC;
	bytes_written = 0;
	for (offset = ctl->start + from + badblockoffset; offset < ctl->start + ctl->physical_size; offset += sectorsize)
	{
		if (blockstart != (offset & (~(erasesize - 1))))
		{
			blockstart = offset & (~(erasesize - 1));
			badblock = mtd->block_isbad(mtd, blockstart);
			if (badblock)
			{
				/* skip the whole erase sector */
				DBG_INFO("Bad block %08x size %08x, skipped for write, case 2\n",
                    (uint32_t)blockstart, (uint32_t)erasesize);
			}
		}
		if (badblock)
		{
			/* skip the whole erase sector */
			continue;
		}
		to_write = (bytes_written + sectorsize > len) ? (len - bytes_written) : sectorsize;
		/* padding */
		if (to_write != sectorsize)
		{
			/* 
			 * Should we just let the writing fail if it's not aligned?
			 * As padding is valid only at the end of the file. Padding could corrupt data
			 * if the write is not the end of the file.
			 * 
			 */
			memset(nand_block_buf, 0xff, sectorsize);
			memcpy(nand_block_buf, (void *)buf + bytes_written, to_write);
			DBG_INFO("Padding %d bytes at 0x%x\n", (uint32_t)(sectorsize - to_write), (uint32_t)(offset + to_write));
			/* write must be page aligned */
			ret = flash_write(mtd, offset, sectorsize, retlen, nand_block_buf);
			if (ret < 0)
			{
				DBG_ERR("Failed to write to NAND flash, return code = %d\n", ret);
				if (ret == -EIO)
				{
					if (mtd->block_markbad)
					{
						DBG_ERR("Marking block as bad at %08x\n", blockstart);
						mtd->block_markbad(mtd, blockstart);
					}
				}
				return ret;
			}
		}
		else
		{
			ret = flash_write(mtd, offset, sectorsize, retlen, buf + bytes_written);
			if (ret < 0)
			{
				//*retlen += bytes_written;
				DBG_ERR("Failed to write to NAND flash, return code = %d\n", ret);
				if (ret == -EIO)
				{
					if (mtd->block_markbad)
					{
						DBG_ERR("Marking block as bad at %08x\n", blockstart);
						mtd->block_markbad(mtd, blockstart);
					}
				}
				return ret;
			}
		}
		if (*retlen != sectorsize)
		{
			*retlen += bytes_written;			
			DBG_ERR("Not all data are written to NAND flash, write %d, expect %d\n", *retlen, sectorsize);
			return -EINVAL;
		}
		DBG_INFO("Write succedded at %08x for %08x\n", offset, sectorsize);
		/* do not count padding */
		bytes_written += to_write;
		if (bytes_written >= len)
			break;
	}
	if (bytes_written != len)
	{
		*retlen = bytes_written;
		DBG_ERR("%s error writing NAND flash, write 0x%x, expect 0x%x\n",
            __FUNCTION__, (uint32_t)*retlen, (uint32_t)len);
		return -EINVAL;
	}
	*retlen = bytes_written;
	return ret;
}

STATIC int mtdocap_fill_buf(ocap_struct *ctl, int force)
{
	int ret = 0;

	if (ctl->isNAND)
	{
		if (ctl->nand_buf_ready == 0 || force)
		{
			size_t retlen = 0;

			ret = mtd_read_skip_badblock(ctl, 0, ctl->nand_buf_size, &retlen, ctl->nand_buf);
			if (ret == 0)
			{
				if (retlen != ctl->nand_buf_size)
				{
					DBG_ERR("Something wrong while reading NAND flash, read %d, expect %d\n", retlen, ctl->nand_buf_size);
					ret = -EINVAL;
				}
			}
			if (ret < 0)
			{
				// Something's wrong, assume unprogrammed flash
				DBG_ERR("Error filling NAND buffer\n");
				memset(ctl->nand_buf, 0xff, ctl->nand_buf_size);
				ret = 0;
			}
			ctl->nand_buf_ready = 1;
		}
	}
	return ret;
}

STATIC int mtd_read_from_buf(ocap_struct *ctl, loff_t from, size_t len, size_t *retlen, u_char *buf)
{
	int ret = 0;
	if (ctl->isNAND)
	{
		/* nand flash requires r/w in sectors, so random r/w will actually access buffered data */
		*retlen = 0;
		if (ctl->nand_buf_ready)
		{
			if (from > ctl->nand_buf_size)
			{
				return -EINVAL;
			}
			*retlen = ((from + len) <= ctl->nand_buf_size) ? len : (ctl->nand_buf_size - from);
			memcpy(buf, ctl->nand_buf + from, *retlen);
		}
		else
		{
			DBG_ERR("data not ready\n");
			return -EINVAL;
		}
	}
	else
	{
		struct mtd_info *mtd = TO_MTDOCAP(ctl);
		ret = mtd->read(mtd, ctl->start + from, len, retlen, buf);
	}
	return ret;
}

STATIC int mtd_write_to_buf(ocap_struct *ctl, loff_t from, size_t len, size_t *retlen, const u_char *buf)
{
	int ret = 0;
	if (ctl->isNAND)
	{
		/* nand flash requires r/w in sectors, so random r/w will actually access buffered data */
		*retlen = 0;
		if (ctl->nand_buf_ready)
		{
			if (from > ctl->nand_buf_size)
			{
				return -EINVAL;
			}
			*retlen = ((from + len) <= ctl->nand_buf_size) ? len : ((from + len) - ctl->nand_buf_size);
			memcpy(ctl->nand_buf + from, buf, *retlen);
		}
		else
		{
			DBG_ERR("data not ready\n");
			return -EINVAL;
		}
	}
	else
	{
		struct mtd_info *mtd = TO_MTDOCAP(ctl);
		ret = flash_write(mtd, ctl->start + from, len, retlen, buf);
	}

	return ret;
}
/*
 * This is the function that actually write to nand flash,  
 * the offset and len must at least aligned to page boundary of MTD device,
 * error will be reported by MTD_WRITE
 */
STATIC int FlashDriverFlush(ocap_struct *ctl, int offset, uint32_t len)
{
	int ret = 0;
	if (ctl->isNAND)
	{
		size_t retlen = 0;
		DBG_INFO("flush 0x%x bytes to 0x%x\n", len, offset);
		if ((ret = mtd_write_skip_badblock(ctl, offset, len, &retlen, ctl->nand_buf + offset)) < 0)
		{
			DBG_ERR("flash write failed\n");
		}
	}
	return ret;
}


STATIC uint32_t FlashDriverRead32(ocap_struct *ctl, int offset)
{
	uint32_t val;
	size_t retlen = 0;

	if (mtd_read_from_buf(ctl, offset, 4, &retlen, (uint8_t *)&val) < 0)
	{
		DBG_ERR("FlashDriverRead32 failed at %08x\n", offset);
		val = 0xffffffff;
	}
	return val;
}

STATIC int FlashDriverReadArray(ocap_struct *ctl, uint32_t offset, uint8_t *pBuffer, uint32_t numberOfBytes)
{
	int ret;
	size_t retlen = 0;

	if ((ret = mtd_read_from_buf(ctl, offset, numberOfBytes, &retlen, (uint8_t *)pBuffer)) < 0)
	{
		DBG_INFO("FlashDriverReadArray failed, num bytes 0x%x at 0x%x\n", numberOfBytes, offset);
		retlen = ret;
	}
	return retlen;
}

STATIC int FlashDriverWriteArray(ocap_struct *ctl, uint32_t offset, const uint8_t *pBuffer, uint32_t numberOfBytes)
{
	int ret;
	size_t retlen = 0;
	if ((ret = mtd_write_to_buf(ctl, offset, numberOfBytes, &retlen, pBuffer)) < 0)
	{
		DBG_INFO("FlashDriverWriteArray failed, num of bytes 0x%x at 0x%x\n", numberOfBytes, offset);
	}
	return retlen;
}

STATIC int memunlock(struct mtd_info *mtd, struct erase_info *erase)
{
	int ret;
	if (!mtd->unlock)
		ret = -EOPNOTSUPP;
	else
		ret = mtd->unlock(mtd, erase->addr, erase->len);
	return ret;
}

STATIC int region_erase(struct mtd_info *mtd, int start, int count, int unlock, int regcount)
{
	int i, j, ret = 0;
	struct mtd_erase_region_info *reginfo;

	DBG_INFO("Region erase start %08x, count %08x\n", start, count);

	reginfo = kmalloc((regcount * sizeof(struct region_info_user)), GFP_KERNEL);

	for (i = 0; i < regcount; i++)
	{
		memcpy(&reginfo[i], &(mtd->eraseregions[i]), sizeof(struct mtd_erase_region_info));
	}

	// We have all the information about the chip we need.

	for (i = 0; i < regcount; i++)
	{ //Loop through the regions
		struct mtd_erase_region_info *r = &(reginfo[i]);

		if ((start >= reginfo[i].offset) &&
			(start < (r->offset + r->numblocks * r->erasesize)))
			break;
	}

	if (i >= regcount)
	{
		DBG_ERR("Starting offset %x not within chip.\n", start);
		return 8;
	}

	//We are now positioned within region i of the chip, so start erasing
	//count sectors from there.

	for (j = 0; (j < count) && (i < regcount); j++)
	{
		struct erase_info erase;
		struct mtd_erase_region_info *r = &(reginfo[i]);

		erase.addr = start;
		erase.len = r->erasesize;

		if (unlock != 0)
		{ //Unlock the sector first.
			if (memunlock(mtd, &erase) != 0)
			{
				DBG_ERR("\nMTD Unlock failure");
				return 8;
			}
		}
		DBG_INFO("Performing Flash Region Erase of length %llu at offset 0x%llx\n",
														erase.len, erase.addr);

		if ((ret = flash_erase(mtd, &erase)) != 0)
		{
			DBG_ERR("MTD Erase failure, return code %d\n", ret);
			if (ret == -EIO)
			{
				if (mtd->block_markbad)
				{
					mtd->block_markbad(mtd, start);
				}
			}
		}

		start += erase.len;
		if (start >= (r->offset + r->numblocks * r->erasesize))
		{ //We finished region i so move to region i+1
			DBG_INFO("\nMoving to region %d\n", i+1);
			i++;
		}
	}
	
	return ret;
}

STATIC int non_region_erase_adjusted_for_bad_blocks(ocap_struct *ctl, int start, int count, int unlock)
{
	struct erase_info erase;
	struct mtd_info *mtd = ctl->mtd;
	int ret = 0;
	unsigned int badblockoffset = 0;
	unsigned offset;

	DBG_INFO("non_region_erase_adjusted_for_bad_blocks(%08x, %08x, %08x)\n", start, count, unlock);

	// Adjust offset for any bad blocks before the erase blocks
	if (mtd->block_isbad)
	{
		for (offset = ctl->start;
			(offset < ctl->start + ctl->physical_size) && (offset < (start + badblockoffset));
			offset += ctl->erase_size)
		{
			if (mtd->block_isbad(mtd, offset))
			{
				/* skip the whole erase sector */
				DBG_INFO("Non Region Erase Bad block %08x  size %08x, skipped\n",
                    (uint32_t)offset, (uint32_t)ctl->erase_size);
				badblockoffset += ctl->erase_size;
			}
		}
	}

	// try to erase up to 'count' blocks
	erase.addr = start + badblockoffset;
	erase.len = mtd->erasesize;

	while ((count > 0) && (erase.addr < (ctl->start + ctl->physical_size)))
	{
		bool isgood = true;
		if (mtd->block_isbad)
			isgood = !mtd->block_isbad(mtd, erase.addr);

		if (isgood)
		{
			DBG_INFO("Performing Flash Non Region Erase of length %llu at offset 0x%llx\n",
                erase.len, erase.addr);
    		
			if (unlock != 0)
			{
				//Unlock the sector first.
				if (memunlock(mtd, &erase) != 0)
				{
					DBG_ERR("non_region_erase MTD Unlock failure\n");
					return -EIO;
				}
			}
			DBG_INFO("Erasing %08x for %08x\n", (unsigned int)erase.addr, (unsigned int)erase.len);
			if ((ret = flash_erase(mtd, &erase)) < 0)
			{
				DBG_ERR("non_region_erase MTD Erase failure\n");
				if (ret == -EIO)
				{
					if (mtd->block_markbad)
						mtd->block_markbad(mtd, start);
				}
				isgood = false;
			}
		}
		else
		{
			DBG_INFO("Non Region Erase Bad block %08x  size %08x, skipped\n",
                (uint32_t)erase.addr, (uint32_t)erase.len);
		}
		if (isgood)
			count--;
		start += mtd->erasesize;
		erase.addr = start + badblockoffset;
		erase.len = mtd->erasesize;
	}
	return (count == 0) ? 0 : -EIO;
}

STATIC int FlashDriverEraseRegion(ocap_struct *ctl, int start, int count)
{
	int ret = 0, regcount, unlock = 0;
	struct mtd_info *mtd = TO_MTDOCAP(ctl);
	DBG_INFO("FlashDriverEraseRegion(%08x, %08x)\n", start, count);

	if (start % ctl->erase_size)
	{
		DBG_ERR("Erase failure, start address %x not aligned to erase size %d\n", start, ctl->erase_size);
		return -1;
	}
	regcount = mtd->numeraseregions;
	if (regcount == 0)
	{
		ret = non_region_erase_adjusted_for_bad_blocks(ctl, ctl->start + start, count, unlock);
	}
	else
	{
		ret = region_erase(mtd, ctl->start + start, count, unlock, regcount);
	}
	return ret;
}

/*
 * Get the start address of the current segment
 * return SEG_EMPTY if all the segments are empty, likely the flash has just been erased
 *        SEG_FULL if all segments are used. the last segment is still valid.
 *        SEG_INVAL if segment invalid;
 *        SEG_OK if succeed. 
 * The region start address is in regionOffset;
 */
STATIC int GetCurrentSegment(ocap_struct *ctl, unsigned int *segmentOffset, unsigned int *length)
{

	unsigned int lastSegment = 0xffffffff, currentSegment = 0, len, lastLen, block, mask;
	int ret = SEG_OK;

	*segmentOffset = 0;
	*length = 0;
	lastLen = 0;
	while (1)
	{
		if (currentSegment >= ctl->size)
		{
			DBG_ERR("Unexpected end of flash!!!\n");
			ret = SEG_INVAL;
			break;
		}
		len = ntohl(FlashDriverRead32(ctl, currentSegment));
		if (len == 0xffffffff)
		{
			if (lastSegment == 0xffffffff)
			{
				DBG_INFO("Unprogrammed nonvol!!!\n");
				ret = SEG_EMPTY;
				break;
			}
			*segmentOffset = lastSegment;
			*length = lastLen;
			if (lastSegment >= ctl->size - 4)
				ret = SEG_FULL;
			break;
		}
		if ((len & 0xffff0000) || (len < 256))
		{
			DBG_ERR("Unexpected invalid length %d encountered!!!\n", len);
			ret = SEG_INVAL;
			break;
		}
		lastSegment = currentSegment;
		lastLen = len;
		block = 3;
		if (ctl->isNAND)
		{
			block = ctl->writesize - 1;
		}
		mask = ~block;
		currentSegment += (len + block) & mask;
	}
	DBG_INFO("GetCurrentSegment return offset = %08x, length = %08x\n", *segmentOffset, *length);
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
STATIC int OcapDriverRead(ocap_struct *ctl, uint8_t *pBuffer, loff_t offset, unsigned int bufferSize)
{
	int result;
	unsigned int segmentOffset, segmentSize;

	result = GetCurrentSegment(ctl, &segmentOffset, &segmentSize);

	if ((result == SEG_INVAL) || (result == SEG_EMPTY))
	{
		return -EFAULT;
	}
	result = 0;

	// Calculate the minimum size to copy, which is either the buffer size or
	// the segment size, whichever is smaller.
	if ((segmentSize - offset) < bufferSize)
	{
		bufferSize = segmentSize - offset;
	}

	if ((result = FlashDriverReadArray(ctl, segmentOffset + offset, pBuffer, bufferSize)) < 0)
	{
		DBG_ERR("Failed to read from ocap\n");
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

STATIC int OcapDriverWrite(ocap_struct *ctl, const uint8_t *pBuffer, uint32_t numberOfBytes)
{
	int result = 0;
	unsigned int segmentSizeBytes, segmentOffset, eraseRegion = 0;

	// Error check - make sure the number of bytes is less than the max
	// segment size.  If it's greater, then we can't possibly write it.
	if (numberOfBytes > (ctl->size - 4))
	{
		DBG_ERR("Too much data to fit in ocap!  Buffer size %u > max segment size %d!\n",
            numberOfBytes, ctl->size);
		
		return -EFAULT;
	}

	/* erase the region if the segments are full */
	result = GetCurrentSegment(ctl, &segmentOffset, &segmentSizeBytes);

	if ((result == SEG_FULL) || (result == SEG_INVAL))
	{
		eraseRegion = 1;
	}
	else
	{
		if ((segmentOffset + segmentSizeBytes + numberOfBytes) > (ctl->size - 4))
		{
			eraseRegion = 1;
		}
	}

	if (ctl->isNAND)
	{
		/* 
		 * Erase the region if this is the first write after insmod.
		 * NAND flash does not allow to write twice after each erase, even write 0xff.
		 * Since the content in flash could be flashed from CFE, and we have no way to know it,
		 * we will force erase the nonvol flash partition after insmod. 
		 */
		if (ctl->partition_erased == 0)
		{
			eraseRegion = 1;
			ctl->partition_erased = 1;
		}
	}
	
	// If we need to erase the flash region, then do so, and write the
	// segment size.
	if (eraseRegion)
	{
		// Erase the region so that we can start over.
		DBG_INFO("Physical size = %08x, size = %08x, erase size = %08x\n",
            ctl->physical_size, ctl->size, ctl->erase_size);
		result = FlashDriverEraseRegion(ctl, 0, ctl->size / ctl->erase_size);

		if (result < 0)
		{
			DBG_ERR("Failed to erase the flash region!\n");
			return result;
		}
		/* reload the data from flash */
		mtdocap_fill_buf(ctl, 1);
		result = GetCurrentSegment(ctl, &segmentOffset, &segmentSizeBytes);
	}

	if (result == SEG_EMPTY)
	{
		segmentOffset = 0;
	}
	else if (result == SEG_OK)
	{
		unsigned int block = 3;
		unsigned int mask;
		if (ctl->isNAND)
		{
			block = ctl->writesize - 1;
		}
		mask = ~block;
		segmentOffset += (segmentSizeBytes + block) & mask;
	}
	else
	{
		DBG_ERR("Failed to write nonvol!\n");
	}

	DBG_INFO("regionOffset = %08x\n", segmentOffset);

	// Write the data.
	result = FlashDriverWriteArray(ctl, segmentOffset, pBuffer, numberOfBytes);
	if (result < 0)
	{
		DBG_ERR("Failed to write the settings data!\n");
		return result;
	}
	result = FlashDriverFlush(ctl, segmentOffset, numberOfBytes);
	if (result < 0)
	{
		DBG_ERR("flash write failed\n");
		return result;
	}
	else
	{
		// Get a pointer to this offset for use in memcmp. 

		// PR2439 - need to specify uncached.
		uint8_t *tmpbuf = kmalloc(numberOfBytes, GFP_KERNEL);
		if (!tmpbuf)
		{
			DBG_ERR("out of memory\n");
			return -1;
		}

		DBG_INFO("Verifying write\n");

		/* reload the data from flash */
		mtdocap_fill_buf(ctl, 1);

		result = FlashDriverReadArray(ctl, segmentOffset, tmpbuf, numberOfBytes);
		
		if (result < 0)
		{
			DBG_ERR("Failed to read from ocap\n");
			kfree(tmpbuf);
			return -EFAULT;
		}

		// Compare the data.
		if (memcmp(pBuffer, tmpbuf, numberOfBytes) != 0)
		{
			DBG_ERR("Failed to verify the settings data!\n");
			result = -EIO;
		}
		else
		{
			DBG_INFO("%s: Write %d at %d succeeded\n", ctl->name, numberOfBytes, segmentOffset);
		}
		kfree(tmpbuf);
	}

	return result;
}

STATIC int flush_func(void *arg)
{
	ocap_struct *ctl = (ocap_struct *)arg;
	uint8_t *buf;
	int actual_size;

	while (1)
	{
		wait_event_interruptible_timeout(ctl->wait, (ctl->flush), 200);
		down_interruptible(&flush_mutex);
		buf = ctl->buf;
		actual_size = ctl->pos;
		if (ctl->flush)
		{
			DBG_INFO("Flushing to flash, size %d\n", actual_size);
			if (OcapDriverWrite(ctl, buf, actual_size) < 0)
			{
				ctl->flush_error = 1;
				DBG_ERR ("ocap: flash write failed!\n");
			}
			ctl->flush = 0;
			ctl->pos = 0; /* reset position */
			ctl->dirty = 0;

		}
		up(&flush_mutex);
		if (kthread_should_stop())
			break;
	}
	return 0;
}

STATIC int OcapDriverFileSize(ocap_struct *ctl)
{
	int len;
	if (ctl->dirty)
	{
		/* data has not flushed yet, read from mem */
		len = *(int*)(ctl->buf);
	}
	else
	{
		if (OcapDriverRead(ctl, (uint8_t *)&len, 0, 4) < 0)
		{
			/* no valid data in the flash, the file size is 0*/
			len = 0;
		}
	}
	len = ntohl(len);
	if (len < 0)
		len = 0; /* empty file */
	return len;
}

STATIC loff_t mtdocap_llseek_nonvol(struct file *file, loff_t offset, int orig)
{
	ocap_struct *ctl = (ocap_struct *)file->private_data;
	loff_t ret = 0;
	int data_size = OcapDriverFileSize(ctl);

	if (data_size == 0)
	{
		/* data has been written to buffer, but not flushed to flash yet */
		data_size = ctl->pos;
	}
	down_interruptible(&flush_mutex);
	switch (orig)
	{
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
	if ((offset >= 0) && (offset <= data_size) && (ret >= 0))
	{
		ctl->pos = offset;
		ret = offset;
	}
	else
	{
		DBG_ERR("%s lseek error! pos %d, max %d, ret %lld\n", ctl->name, ctl->pos, ctl->size - 8, ret);
		ret = -EFAULT;
	}
	DBG_INFO("%s: lseek pos %d data_size %d\n", ctl->name, ctl->pos, data_size);
	up(&flush_mutex);
	return ret;
}

STATIC ocap_struct *mtdocap_getctl(struct inode *inode)
{
	unsigned int minor = iminor(inode);
	ocap_struct *ctl;
	if (minor >= MAX_OCAP_MINOR_DEV)
		return NULL;
	ctl = &ocap_ctl[minor];
	if (ctl->devnum == -1)
		return NULL;
	return ctl;
}

STATIC int mtdocap_open_nonvol(struct inode *inode, struct file *file)
{
	struct mtd_info *mtd;
	int ret = 0;
	ocap_struct *ctl = mtdocap_getctl(inode);
	if (ctl == NULL)
		return -ENODEV;

	/* You can't open the RO devices RW */
	/*
	if ((file->f_mode & 2) && (minor & 1))
	return -EACCES;
	*/
	DBG_INFO( "%s devnum = %u\n", __FUNCTION__, ctl->devnum);
	mtd = get_mtd_device(NULL, ctl->devnum);

	if (!mtd)
	{
		put_mtd_device(mtd);
		DBG_INFO( "%s no mtd\n", __FUNCTION__);
		return -ENODEV;
	}
	if (MTD_ABSENT == mtd->type)
	{
		put_mtd_device(mtd);
		DBG_INFO( "%s MTD_ABSENT\n", __FUNCTION__);
		return -ENODEV;
	}

	down_interruptible(&flush_mutex);
	ctl->mtd = mtd;
	ctl->pos = 0;
	ctl->dirty = 0;
	ctl->flush_error = 0;
	up(&flush_mutex);
	file->private_data = ctl;

	if (ctl->isNAND)
	{
		if ((ret = mtdocap_fill_buf(ctl, 1)) < 0)
		{
			DBG_INFO( "%s fill err\n", __FUNCTION__);
		}
	}
	return 0;
}

/*====================================================================*/

#define MAX_TIMEOUT (5*HZ) /* 5 second */
#define TIMEOUT_MS (HZ / 20) /* 50 ms */

STATIC int mtdocap_close_nonvol(struct inode *inode, struct file *file)
{
	ocap_struct *ctl;
	struct mtd_info *mtd;
	unsigned long timeout = 0;

	ctl = TO_MTDOCAP_CTL(file);
	DBG_INFO("%s: MTD_close\n", ctl->name);
	mtd = TO_MTDOCAP(ctl);
	DBG_INFO("TIMEOUT_MS = %x, HZ = %x\n", TIMEOUT_MS, HZ);
	/* wait for the data to be flushed */
	while (ctl->dirty && (timeout < MAX_TIMEOUT))
	{
		set_current_state(TASK_INTERRUPTIBLE);
		schedule_timeout(TIMEOUT_MS);
		timeout += TIMEOUT_MS;
	}
	if (timeout >= MAX_TIMEOUT)
	{
		DBG_ERR("Waiting too long for data flush, abort.\n");
		ctl->dirty = 0;
	}
	if (mtd->sync)
		mtd->sync(mtd);
	put_mtd_device(mtd);

	return 0;
}

STATIC ssize_t mtdocap_read_nonvol(struct file *file, char __user *buf, size_t count,loff_t *ppos)
 {
	int ret=0;
	ocap_struct * ctl = TO_MTDOCAP_CTL(file);

	DBG_INFO( "%s %s\n", __FUNCTION__, ctl->name);

    if (ctl->isNAND) {
	/*
	 * read the whole nand buffer, this is not efficient. But in cases where
	 * other utilities can modify the flash content (nandwrite, eraseall, etc), 
	 * this seems to be the  only way to guarantee data consistency.
	 * In production, force should be set to 0
	 */
        if ((ret = mtdocap_fill_buf(ctl, 1)) < 0) {
            return 0;
        }
    }

	if ((ret = OcapDriverRead(ctl, ctl->buf, ctl->pos, count)) < 0) {
	    DBG_ERR("read error %d\n", ret);
		return 0;
	} else {
		if (copy_to_user(buf, ctl->buf, ret)) {
			ret = 0;
		} else {
			*ppos += ret;
			ctl->pos += ret;
		}
	}
	return ret;
}

STATIC ssize_t mtdocap_write_nonvol(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
 {
	ocap_struct * ctl = (ocap_struct *)file->private_data;
	uint8_t * dest = ctl->buf;
	int offset = ctl->pos;
	int size = ctl->size;
	ssize_t ret = count;
    int need_to_wait = 0;

	down_interruptible(&flush_mutex);
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
	     * the section is considered to be finished.
	     *
	     * tftpd: The section is finished if the packet size is not 
	     * 512 (TFTP_BLOCK_SIZE);
	     *
	     * otherwise, flush whatever in the buffer into flash. bulkdma fails into this scenerio, since
	     * it always write the whole section in one request.
	     */
		if (!((count == (MAX_MESSAGE_SIZE - 8)) || (count == TFTP_BLOCK_SIZE))) {
	        DBG_INFO("%s: need flush, offset %d, count %d\n", ctl->name, offset, count);
			ctl->flush = 1;
            need_to_wait = 1;
			wake_up_interruptible(&ctl->wait);
		}
		ctl->dirty = 1;
	}
	else {
	    DBG_ERR("write out of boundary\n");
		ret = 0;
	}
	up(&flush_mutex);
    if (need_to_wait) {
        unsigned long timeout = 0;
        /* wait for the data to be flushed */
        while (ctl->dirty && (timeout < MAX_TIMEOUT))
        {
            set_current_state(TASK_INTERRUPTIBLE);
            schedule_timeout(TIMEOUT_MS);
            timeout += TIMEOUT_MS;
        }
        if (timeout >= MAX_TIMEOUT)
        {
            ret = 0;
        }
    }
    if (ret >= 0) {
        if (ctl->flush_error) {
            ctl->flush_error = 0;
            ret = 0;
        }
    }
	return ret;
}


/* 
 * functions copied from mtdchar.c, with minor changes to work for mtdocap,
 * mainly for read/write and erase region, not fully verified
 */

STATIC loff_t mtdocap_llseek_normal(struct file *file, loff_t offset, int orig)
{
	ocap_struct *ctl;
	struct mtd_info *mtd;

	ctl = TO_MTDOCAP_CTL(file);
	mtd = TO_MTDOCAP(ctl);

	switch (orig)
	{
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

STATIC int mtdocap_open_normal(struct inode *inode, struct file *file)
{
	struct mtd_info *mtd;
	ocap_struct *ctl = mtdocap_getctl(inode);
	if (ctl == NULL)
		return -ENODEV;

	ctl->docsiscopy = 0;
	mtd = get_mtd_device(NULL, ctl->devnum);

	if (!mtd)
		return -ENODEV;

	if (MTD_ABSENT == mtd->type)
	{
		put_mtd_device(mtd);
		return -ENODEV;
	}

	/* You can't open it RW if it's not a writeable device */
	if ((file->f_mode & 2) && !(mtd->flags & MTD_WRITEABLE))
	{
		put_mtd_device(mtd);
		return -EACCES;
	}

	ctl->mtd = mtd;
	DBG_INFO("%s ctl %p, mtd %p\n", __FUNCTION__, ctl, mtd);
	file->private_data = ctl;

	return 0;
} /* mtd_open */

/*====================================================================*/

STATIC int 
mtdocap_close_normal(struct inode *inode, struct file *file)
{
	ocap_struct *ctl;
	struct mtd_info *mtd;
	unsigned long buffer[4];

	ctl = TO_MTDOCAP_CTL(file);
	mtd = TO_MTDOCAP(ctl);

	// If this is a docsis partition and the copy to ram ioctl has been called then copy the
	// image to high memory
	if (ctl->docsiscopy)
	{
		int ret;
		int retlen;
		if (ctl->isNAND)
			ret = mtd_read_skip_badblock(ctl, 0, sizeof(buffer), &retlen, (void *)buffer);
		else
			ret = mtd->read(mtd, ctl->start, sizeof(buffer), &retlen, (void *)buffer);
		if (ret == 0)
		{
			buffer[0] = ntohl(buffer[3]);
			DBG_INFO("Image size = %d\n", (int)buffer[0]);
			buffer[0] += 0x5c;
			if (buffer[0] <= DOCSIS_HIGH_RAM_SIZE)
			{
				DBG_INFO("Moving docsis image to high memory\n");
				if (ctl->isNAND)
					ret = mtd_read_skip_badblock(ctl, 0, buffer[0], &retlen, (void *)(docsishighram | 0xa0000000));
				else
					ret = mtd->read(mtd, ctl->start, buffer[0], &retlen, (void *)(docsishighram | 0xa0000000));
			}
		}
		if (ret < 0)
		{
			DBG_ERR("Error moving docsis image to high memory\n");
		}
	}

	if (mtd->sync)
		mtd->sync(mtd);

	put_mtd_device(mtd);

	file->private_data = NULL;

	return 0;
} /* mtd_close */

/* FIXME: This _really_ needs to die. In 2.5, we should lock the
   userspace buffer down and use it directly with readv/writev.
   */
#define MAX_KMALLOC_SIZE 0x20000

STATIC ssize_t mtdocap_read_normal(struct file *file, char __user *buf, size_t count,loff_t *ppos) {
	ocap_struct * ctl;
	struct mtd_info *mtd;
	size_t retlen=0, total_retlen=0, ret=0, size;
	int len, start;
	char *kbuf;

	ctl = TO_MTDOCAP_CTL(file);
	mtd = TO_MTDOCAP(ctl);
	size = ctl->size;
	start = ctl->start;
	if (*ppos + count > size)
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
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 37)
		case MTD_MODE_OTP_FACTORY:
#else
		case MTD_FILE_MODE_OTP_FACTORY:
#endif		
			ret = mtd->read_fact_prot_reg(mtd, start + *ppos, len, &retlen, kbuf);
			break;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 37)
		case MTD_MODE_OTP_USER:
#else
		case MTD_FILE_MODE_OTP_USER:
#endif		
			ret = mtd->read_user_prot_reg(mtd, start + *ppos, len, &retlen, kbuf);
			break;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 37)
		case MTD_MODE_RAW:
#else
		case MTD_FILE_MODE_RAW:
#endif
 {
			    struct mtd_oob_ops ops;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 37)
			    ops.mode = MTD_OOB_RAW;
#else
			    ops.mode = MTD_OPS_RAW;
#endif				
			    ops.datbuf = kbuf;
			    ops.oobbuf = NULL;
			    ops.len = len;

			    ret = mtd->read_oob(mtd, start + *ppos, &ops);
			    retlen = ops.retlen;
		    }
            break;
		default:
            if (ctl->isNAND) {
			    ret = mtd_read_skip_badblock(ctl, *ppos, len, &retlen, kbuf);
            }
            else {
                ret = mtd->read(mtd, start + *ppos, len, &retlen, kbuf);
            }
            break;
		}
	// Nand returns -EBADMSG on ecc errors, but it returns
	// the data. For our userspace tools it is important
	// to dump areas with ecc errors !
	// For kernel internal usage it also might return -EUCLEAN
	// to signal the caller that a bitflip has occured and has
	// been corrected by the ECC algorithm.
	// Userspace software which accesses NAND this way
	// must be aware of the fact that it deals with NAND
		if (!ret || (ret == -EUCLEAN) || (ret == -EBADMSG)) {
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

STATIC ssize_t mtdocap_write_normal(struct file *file, const char __user *buf, size_t count, loff_t *ppos) {
	ocap_struct * ctl;
	struct mtd_info *mtd;
	char *kbuf;
	size_t retlen, total_retlen=0, size;
	int ret=0, len = 0, start, erase_size = 0;
	
	ctl = TO_MTDOCAP_CTL(file);
	mtd = TO_MTDOCAP(ctl);
	size = ctl->size;
	start = ctl->start;
	DBG_INFO("mtdocap_write_normal %08x %d\n", (unsigned int)(*ppos), (unsigned int)count);
	if (*ppos == size)
		return -ENOSPC;
	if (*ppos + count > size)
		count = size - *ppos;
	if (!count)
		return 0;

	/* erase the flash if necessary */
	erase_size = ctl->erase_size;
	if ((*ppos & (erase_size - 1)) == 0) {
		if (FlashDriverEraseRegion(ctl, *ppos, 1) < 0) {
			return 0;
		}
	}

	len = (*ppos + erase_size) & (~(erase_size - 1));

	while (len < (*ppos + count)) {
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
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 37)
		case MTD_MODE_OTP_FACTORY:
#else
		case MTD_FILE_MODE_OTP_FACTORY:
#endif		
			ret = -EROFS;
			break;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 37)
		case MTD_MODE_OTP_USER:
#else
		case MTD_FILE_MODE_OTP_USER:
#endif		
			if (!mtd->write_user_prot_reg) {
				ret = -EOPNOTSUPP;
				break;
			}
			ret = mtd->write_user_prot_reg(mtd, start + *ppos, len, &retlen, kbuf);
			break;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 37)
		case MTD_MODE_RAW:
#else
		case MTD_FILE_MODE_RAW:
#endif		
            {
			    struct mtd_oob_ops ops;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 37)
			    ops.mode = MTD_OOB_RAW;
#else
			    ops.mode = MTD_OPS_RAW;
#endif		
			    ops.datbuf = kbuf;
			    ops.oobbuf = NULL;
			    ops.len = len;

	            DEBUG(MTD_DEBUG_LEVEL0,"MTD_write_oob(%08x, len=%d)\n", (unsigned int)(start + *ppos), len);
			    ret = mtd->write_oob(mtd, ctl->start + *ppos, &ops);
			    retlen = ops.retlen;
		    }
            break;

		default:
            if (ctl->isNAND) {
                DEBUG(MTD_DEBUG_LEVEL0,"mtd_write_skip_badblock(%08x, len=%d)\n", (unsigned int)*ppos, len);
                ret = mtd_write_skip_badblock(ctl, *ppos, len, &retlen, kbuf);
            }
            else {
                DEBUG(MTD_DEBUG_LEVEL0,"MTD_write(%08x, len=%d)\n", (unsigned int)(start + *ppos), len);
                ret = flash_write(mtd, start + *ppos, len, &retlen, kbuf);
            }
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
STATIC void mtdchar_erase_callback(struct erase_info *instr)
{
	wake_up((wait_queue_head_t *)instr->priv);
}

#if defined(CONFIG_MTD_OTP) || defined(CONFIG_MTD_ONENAND_OTP)
STATIC int otp_select_filemode(struct file *file, int mode)
{
	ocap_struct * ctl ;
	struct mtd_info *mtd;
	int ret = 0;

	ctl = TO_MTDOCAP_CTL(file);
	mtd = TO_MTDOCAP(ctl);
	switch (MTD_MODE(file)) {
	case MTD_OTP_FACTORY:
		if (!mtd->read_fact_prot_reg)
			ret = -EOPNOTSUPP;
		else
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 37)
			ctl->mode = MTD_MODE_OTP_FACTORY;
#else
			ctl->mode = MTD_FILE_MODE_OTP_FACTORY;
#endif			
		break;
	case MTD_OTP_USER:
		if (!mtd->read_fact_prot_reg)
			ret = -EOPNOTSUPP;
		else
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 37)
			ctl->mode = MTD_MODE_OTP_USER;
#else
			ctl->mode = MTD_FILE_MODE_OTP_USER;
#endif			
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

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 37)
STATIC int mtdocap_ioctl_normal(struct inode *inode, struct file *file, u_int cmd, u_long arg)
#else
STATIC long mtdocap_ioctl_normal(struct file *file, u_int cmd, u_long arg)
#endif
{
	ocap_struct *ctl;
	struct mtd_info *mtd;
	void
	__user *argp = (void __user *)arg;
	int ret = 0;
	u_long size;
	struct mtd_info_user info;

	ctl = TO_MTDOCAP_CTL(file);
	mtd = TO_MTDOCAP(ctl);

	size = (cmd & IOCSIZE_MASK) >> IOCSIZE_SHIFT;
	if (cmd & IOC_IN)
	{
		if (!access_ok(VERIFY_READ, argp, size))
			return -EFAULT;
	}
	if (cmd & IOC_OUT)
	{
		if (!access_ok(VERIFY_WRITE, argp, size))
			return -EFAULT;
	}

	switch (cmd)
	{
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
		    if (copy_to_user(argp, &(mtd->eraseregions[ur.regionindex]), sizeof(struct mtd_erase_region_info)))
			    return -EFAULT;
	    }
		break;

	case MEMGETINFO:
		info.type = mtd->type;
		info.flags = mtd->flags;
		info.size = ctl->size;
		info.erasesize = mtd->erasesize;
		/* TDT breaks our mtd-utils */
		info.writesize = mtd->writesize;
		info.oobsize = mtd->oobsize;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 37)
		info.ecctype = -1;
		info.eccsize = 0;
#endif
		if (copy_to_user(argp, &info, sizeof(struct mtd_info_user)))
			return -EFAULT;
		break;
	case MEMERASE:
	    {
		    struct erase_info *erase;

		    if (!(file->f_mode & 2))
			    return -EPERM;

		    erase = kmalloc(sizeof(struct erase_info), GFP_KERNEL);
		    if (!erase)
			    ret = -ENOMEM;
		    else
		    {
			    wait_queue_head_t waitq;
			    DECLARE_WAITQUEUE(wait, current);

			    init_waitqueue_head(&waitq);

			    memset(erase, 0, sizeof(struct erase_info));
			    if (copy_from_user(&erase->addr, argp, sizeof(struct erase_info_user)))
			    {
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
			    if (!ret)
			    {
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
	    }
		break;

	case MEMWRITEOOB:
	    {
		    struct mtd_oob_buf buf;
		    struct mtd_oob_ops ops;

		    if (!(file->f_mode & 2))
			    return -EPERM;

		    if (copy_from_user(&buf, argp, sizeof(struct mtd_oob_buf)))
			    return -EFAULT;

		    if (buf.length > 4096)
			    return -EINVAL;

		    if (!mtd->write_oob)
			    ret = -EOPNOTSUPP;
		    else
			    ret = access_ok(VERIFY_READ, buf.ptr, buf.length) ? 0 : EFAULT;

		    if (ret)
			    return ret;

		    ops.len = buf.length;
		    ops.ooblen = buf.length;
		    ops.ooboffs = buf.start & (mtd->oobsize - 1);
		    ops.datbuf = NULL;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 37)
		    ops.mode = MTD_OOB_PLACE;
#else
		    ops.mode = MTD_OPS_PLACE_OOB;
#endif			

		    if (ops.ooboffs && ops.len > (mtd->oobsize - ops.ooboffs))
			    return -EINVAL;

		    ops.oobbuf = kmalloc(buf.length, GFP_KERNEL);
		    if (!ops.oobbuf)
			    return -ENOMEM;

		    if (copy_from_user(ops.oobbuf, buf.ptr, buf.length))
		    {
			    kfree(ops.oobbuf);
			    return -EFAULT;
		    }

		    buf.start &= ~(mtd->oobsize - 1);
		    ret = mtd->write_oob(mtd, ctl->start + buf.start, &ops);

		    if (copy_to_user(argp + sizeof(uint32_t), &ops.retlen, sizeof(uint32_t)))
			    ret = -EFAULT;

		    kfree(ops.oobbuf);

	    }
		break;

	case MEMREADOOB:
	    {
		    struct mtd_oob_buf buf;
		    struct mtd_oob_ops ops;

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

		    ops.len = buf.length;
		    ops.ooblen = buf.length;
		    ops.ooboffs = buf.start & (mtd->oobsize - 1);
		    ops.datbuf = NULL;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 37)
		    ops.mode = MTD_OOB_PLACE;
#else
		    ops.mode = MTD_OPS_PLACE_OOB;
#endif			

		    if (ops.ooboffs && ops.len > (mtd->oobsize - ops.ooboffs))
			    return -EINVAL;

		    ops.oobbuf = kmalloc(buf.length, GFP_KERNEL);
		    if (!ops.oobbuf)
			    return -ENOMEM;

		    buf.start &= ~(mtd->oobsize - 1);
		    ret = mtd->read_oob(mtd, ctl->start + buf.start, &ops);

		    if (put_user(ops.retlen, (uint32_t __user *)argp))
			    ret = -EFAULT;
		    else if (ops.retlen && copy_to_user(buf.ptr, ops.oobbuf, ops.retlen))
			    ret = -EFAULT;

		    kfree(ops.oobbuf);
	    }
		break;

	case MEMLOCK:
	    {
		    struct erase_info_user info;

		    if (copy_from_user(&info, argp, sizeof(info)))
			    return -EFAULT;

		    if (!mtd->lock)
			    ret = -EOPNOTSUPP;
		    else
			    ret = mtd->lock(mtd, info.start, info.length);
	    }
		break;

	case MEMUNLOCK:
	    {
		    struct erase_info_user info;

		    if (copy_from_user(&info, argp, sizeof(info)))
			    return -EFAULT;

		    if (!mtd->unlock)
			    ret = -EOPNOTSUPP;
		    else
			    ret = mtd->unlock(mtd, info.start, info.length);
	    }
		break;

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
		    memcpy(&oi.oobfree, mtd->ecclayout->oobfree, sizeof(oi.oobfree));

		    if (copy_to_user(argp, &oi, sizeof(struct nand_oobinfo)))
			    return -EFAULT;
	    }
		break;

	case MEMGETBADBLOCK:
	    {
		    loff_t offs;

		    if (copy_from_user(&offs, argp, sizeof(loff_t)))
			    return -EFAULT;
		    if (!mtd->block_isbad)
			    ret = -EOPNOTSUPP;
		    else
			    return mtd->block_isbad(mtd, offs);
	    }
		break;

	case MEMSETBADBLOCK:
	    {
		    loff_t offs;

		    if (copy_from_user(&offs, argp, sizeof(loff_t)))
			    return -EFAULT;
		    if (!mtd->block_markbad)
			    ret = -EOPNOTSUPP;
		    else
			    return mtd->block_markbad(mtd, offs);
	    }
		break;

#if defined(CONFIG_MTD_OTP) || defined(CONFIG_MTD_ONENAND_OTP)
	case OTPSELECT:
	    {
		    int mode;
		    if (copy_from_user(&mode, argp, sizeof(int)))
			    return -EFAULT;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 37)
		    ctl->mode = MTD_MODE_NORMAL;
#else
		    ctl->mode = MTD_FILE_MODE_NORMAL;
#endif			

		    ret = otp_select_filemode(file, mode);

		    file->f_pos = 0;
	    }
        break;

	case OTPGETREGIONCOUNT:
	case OTPGETREGIONINFO:
	    {
		    struct otp_info *buf = kmalloc(4096, GFP_KERNEL);
		    if (!buf)
			    return -ENOMEM;
		    ret = -EOPNOTSUPP;
		    switch (MTD_MODE(file)) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 37)
		    case MTD_MODE_OTP_FACTORY:
#else
		    case MTD_FILE_MODE_OTP_FACTORY:
#endif			
			    if (mtd->get_fact_prot_info)
				    ret = mtd->get_fact_prot_info(mtd, buf, 4096);
			    break;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 37)
		    case MTD_MODE_OTP_USER:
#else
		    case MTD_FILE_MODE_OTP_USER:
#endif			
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
	    }
        break;

	case OTPLOCK:
	    {
		    struct otp_info info;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 37)
		    if (MTD_MODE(file) != MTD_MODE_OTP_USER)
#else
		    if (MTD_MODE(file) != MTD_FILE_MODE_OTP_USER)
#endif			
			    return -EINVAL;
		    if (copy_from_user(&info, argp, sizeof(info)))
			    return -EFAULT;
		    if (!mtd->lock_user_prot_reg)
			    return -EOPNOTSUPP;
		    ret = mtd->lock_user_prot_reg(mtd, info.start, info.length);
	    }
        break;
#endif

	case ECCGETLAYOUT:
	{
		if (!mtd->ecclayout)
			return -EOPNOTSUPP;

		if (copy_to_user(argp, &mtd->ecclayout, sizeof(struct nand_ecclayout)))
			return -EFAULT;
	}
		break;

	case ECCGETSTATS:
	{
		if (copy_to_user(argp, &mtd->ecc_stats, sizeof(struct mtd_ecc_stats)))
			return -EFAULT;
	}
		break;

	case MTDFILEMODE:
	{
		ctl->mode = 0;

		switch (arg)
		{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 37)
		case MTD_MODE_OTP_FACTORY:
		case MTD_MODE_OTP_USER:
#else
		case MTD_FILE_MODE_OTP_FACTORY:
		case MTD_FILE_MODE_OTP_USER:
#endif		
			ret = otp_select_filemode(file, arg);
			break;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 37)
		case MTD_MODE_RAW:
#else
		case MTD_FILE_MODE_RAW:
#endif		
			if (!mtd->read_oob || !mtd->write_oob)
				return -EOPNOTSUPP;
			ctl->mode = arg;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 37)
		case MTD_MODE_NORMAL:
#else
		case MTD_FILE_MODE_NORMAL:
#endif		
			break;
		default:
			ret = -EINVAL;
		}
		file->f_pos = 0;
	}
		break;

	case SIOCDEVPRIVATE:
	{
		if (memcmp(ctl->name, "docsis", 6))
			return -EINVAL;
		ctl->docsiscopy = 1;
	}
		break;

	default:
		ret = -ENOTTY;
		break;
	}

	return ret;
} /* memory_ioctl */

STATIC int mtdocap_open(struct inode *inode, struct file *file)
{
	ocap_struct *ctl = mtdocap_getctl(inode);
	if (ctl == NULL)
		return -ENODEV;
	return ctl->fops->open(inode, file);
}

STATIC int mtdocap_close(struct inode *inode, struct file *file)
{
	ocap_struct *ctl = mtdocap_getctl(inode);
	if (ctl == NULL)
		return -ENODEV;
	return ctl->fops->release(inode, file);
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 37)
STATIC int mtdocap_ioctl(struct inode *inode, struct file *file, u_int cmd, u_long arg)
{
	ocap_struct *ctl = mtdocap_getctl(inode);
	if (ctl == NULL)
		return -ENODEV;
	return ctl->fops->ioctl(inode, file, cmd, arg);
}
#else
STATIC long mtdocap_ioctl(struct file *file, u_int cmd, u_long arg)
{
	struct inode *inode = file->f_path.dentry->d_inode;
	ocap_struct *ctl = mtdocap_getctl(inode);
	if (ctl == NULL)
		return -ENODEV;
	return ctl->fops->unlocked_ioctl(file, cmd, arg);
}
#endif

STATIC ssize_t mtdocap_write(struct file *file, const char __user *buf, size_t count,loff_t *ppos)
 {
	ocap_struct *ctl = TO_MTDOCAP_CTL(file);
	return ctl->fops->write(file, buf, count, ppos);
}

STATIC ssize_t mtdocap_read(struct file *file, char __user *buf, size_t count,loff_t *ppos)
 {
	ocap_struct *ctl = TO_MTDOCAP_CTL(file);
	return ctl->fops->read(file, buf, count, ppos);
}

STATIC loff_t mtdocap_llseek(struct file *file, loff_t offset, int orig)
{
	ocap_struct *ctl = TO_MTDOCAP_CTL(file);
	return ctl->fops->llseek(file, offset, orig);
}

STATIC struct proc_dir_entry *proc_mtdocap;

STATIC int mtdocap_read_proc(char *page, char **start, off_t off, int count, int *eof, void *data_unused)
{
	int len, l, i;
	off_t begin = 0;
	char *buf;
	ocap_struct *ctl;

	len = sprintf(page, "dev:      start    phy-size size     erasesiz writesiz NAND mtd  name\n");
	for (i = 0; i < MAX_OCAP_MINOR_DEV; i++)
	{
		ctl = &ocap_ctl[i];
		if (ctl->devnum == -1)
			continue;
		buf = page + len;
		l = sprintf(buf, "%s%d: %8.8x %8.8x %8.8x %8.8x %8.8x   %d  mtd%d %s\n",
					OCAPDEV, i, ctl->start, ctl->physical_size, ctl->size, ctl->erase_size, ctl->writesize,
					ctl->isNAND, ctl->devnum, ctl->name);
		len += l;
		if (len + begin > off + count)
			goto done;
		if (len + begin < off)
		{
			begin += len;
			len = 0;
		}
	}
	*eof = 1;

done:
	if (off >= len + begin)
		return 0;
	*start = page + (off - begin);
	return ((count < begin + len - off) ? count : begin + len - off);
}

STATIC int ParsePartitionParm(ocap_partition *part)
{
	if (*(part->parmCount) == 0)
	{
		if (part->optional)
		{
			return 1;
		}
		DBG_ERR("Mandatory parameter %s for partition missing\n", part->name);
		return 0;
	}
	if (*(part->parmCount) != 5)
	{
		DBG_ERR("Invalid parameter: 5 values need to be specified for partition %s\n", part->name);
		return 0;
	}
	part->minor = part->parms[0];
	part->devnum = part->parms[1];
	part->start = part->parms[2];
	part->physical_size = part->parms[3];
	part->size = part->parms[4];
	if (part->minor >= MAX_OCAP_MINOR_DEV)
	{
		DBG_ERR("Minor device number exceeds maximum for partition %s\n", part->name);
		part->minor = -1;
		return 0;
	}
	if ((part->size == 0) || (part->physical_size == 0))
	{
		DBG_ERR("Invalid parameter: partition size for partition %s cannot be 0\n", part->name);
		part->minor = -1;
		return 0;
	}
	if (part->size > part->physical_size)
	{
		DBG_ERR("Invalid parameter: logical partition size for partition %s cannot be greater than physical size\n",
            		part->name);
		part->minor = -1;
		return 0;
	}
	return 1;
}

STATIC struct file_operations mtdocap_fops = {
	.owner			= THIS_MODULE,
	.llseek			= mtdocap_llseek,
	.read			= mtdocap_read,
	.write			= mtdocap_write,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 37)
	.ioctl			= mtdocap_ioctl,
#else
	.unlocked_ioctl	= mtdocap_ioctl,
#endif
	.open			= mtdocap_open,
	.release		= mtdocap_close,
};

STATIC int __init init_mtdocap(void)
{
	int ret = -ENODEV;
	int minordev = 0;
	int dev;
	ocap_struct *ctl = 0;
	struct mtd_info *mtd;
	char str[64];
	int foundNAND = 0;
	printk(KERN_INFO "mtdocap: Broadcom MTDOCAP driver " VER_STR "\n");
	
	memset(ocap_ctl, 0, sizeof(ocap_ctl));

	for (minordev = 0; minordev < MAX_OCAP_MINOR_DEV; minordev++)
		ocap_ctl[minordev].devnum = -1;

	for (dev = 0; dev < MAX_DEVS; dev++)
	{
		if (!ParsePartitionParm(&ocap_devs[dev]))
		{
			ret = -ENODEV;
			goto err;
		}
		if (ocap_devs[dev].minor == -1)
			continue;
		ctl = &ocap_ctl[ocap_devs[dev].minor];
		ctl->devnum = ocap_devs[dev].devnum;
		strcpy(ctl->name, ocap_devs[dev].name);
		ctl->fops = ocap_devs[dev].fops;
		ctl->size = ocap_devs[dev].
			size; /* size cannot expand with flash size, since malloc may fail for large flash */
		mtd = get_mtd_device(NULL, ctl->devnum);

		if (!mtd)
		{
			put_mtd_device(mtd);
			ret = -ENODEV;
			goto err;
		}
		if (MTD_ABSENT == mtd->type)
		{
			put_mtd_device(mtd);
			ret = -ENODEV;
			goto err;
		}
		ctl->writesize = mtd->writesize;
		if (mtd->type == MTD_NANDFLASH)
		{
			foundNAND = 1;
			ctl->isNAND = 1;
			ctl->nand_buf_size = ocap_devs[dev].size;
			//mtd->block_markbad(mtd, 0x7b00000); // JCYR FOR TEST PURPOSE
			//mtd->block_markbad(mtd, 0x7b20000); // JCYR FOR TEST PURPOSE
		}
		else
		{
			ctl->nand_buf_size = 0;
			ctl->isNAND = 0;
		}

		if (mtd->size)
		{
			ctl->erase_size = mtd->erasesize;
			ctl->physical_size = ocap_devs[dev].physical_size;
			ctl->start = ocap_devs[dev].start;
			if (mtd->sync)
				mtd->sync(mtd);
			put_mtd_device(mtd);
		}
		else
		{
			/* zero size partition */
			if (mtd->sync)
				mtd->sync(mtd);
			put_mtd_device(mtd);
		}

		if ((strncmp(ctl->name, "dyncfg", 64) == 0) || (strncmp(ctl->name, "permcfg", 64) == 0))
		{
			ctl->buf = kmalloc(ctl->size, GFP_KERNEL);
			if (!ctl->buf)
			{
				DBG_ERR("ctl->buf out of memory!\n");
				ret = -ENOMEM;
				goto err;
			}
			ctl->nand_buf = 0;
			if (ctl->nand_buf_size)
			{
				ctl->nand_buf = kmalloc(ctl->nand_buf_size, GFP_KERNEL);
				if (!ctl->nand_buf)
				{
					DBG_ERR("ctl->nand_buf out of memory!\n");
					ret = -ENOMEM;
					goto err;
				}
				ctl->nand_buf_ready = 0;
			}
			sprintf(str, "%s", ctl->name);
			init_waitqueue_head(&ctl->wait);
			ctl->flush_thread = kthread_run(flush_func, ctl, str);
			if (IS_ERR(ctl->flush_thread))
			{
				goto err;
			}
		}
	}

	if ((ret = register_chrdev(MTD_OCAP_MAJOR, OCAPDEV, &mtdocap_fops)))
	{
		DBG_ERR("unable to get major %d for misc devices\n",
            		MTD_OCAP_MAJOR);
		ret = -EIO;
		goto err;
	}
	if ((proc_mtdocap = create_proc_entry(OCAPDEV, 0, NULL)))
		proc_mtdocap->read_proc = mtdocap_read_proc;

	if (foundNAND)
	{
		nand_block_buf = kmalloc(ctl->size, GFP_KERNEL);
		if (nand_block_buf == NULL)
		{
			DBG_ERR("nand_block_buf out of memory!\n");
			ret = -ENOMEM;
			goto err;
		}
	}

	DBG_INFO("%s initalized\n", OCAPDEV);
	return 0;

err:
	for (minordev = 0; minordev < MAX_OCAP_MINOR_DEV; minordev++)
	{
		ctl = &ocap_ctl[minordev];
		if (ctl->devnum == -1)
			continue;
		if (IS_ERR(ctl->flush_thread))
		{
			kthread_stop(ctl->flush_thread);
		}
		if (ctl->buf)
		{
			kfree(ctl->buf);
			ctl->buf = NULL;
		}
		if (ctl->nand_buf)
		{
			kfree(ctl->nand_buf);
			ctl->nand_buf = NULL;
		}
		ctl->nand_buf_ready = 0;
	}
	return ret;

}

STATIC void __exit cleanup_mtdocap(void)
{
	int dev;
	ocap_struct *ctl;

	unregister_chrdev(MTD_OCAP_MAJOR, OCAPDEV);
	if (proc_mtdocap)
		remove_proc_entry(OCAPDEV, NULL);
	for (dev = 0; dev < MAX_DEVS; dev++)
	{
		ctl = &ocap_ctl[dev];
		if (ctl->devnum == -1)
			continue;
		if (ctl->flush_thread)
			kthread_stop(ctl->flush_thread);
		if (ctl->buf)
		{
			kfree(ctl->buf);
			ctl->buf = NULL;
		}
		if (ctl->nand_buf)
		{
			kfree(ctl->nand_buf);
			ctl->nand_buf = NULL;
		}
		ctl->nand_buf_ready = 0;
	}

	if (nand_block_buf != NULL)
	{
		kfree(nand_block_buf);
		nand_block_buf = NULL;
	}
	DBG_INFO("%s removed\n", OCAPDEV);
}

module_init (init_mtdocap);
module_exit (cleanup_mtdocap);

MODULE_LICENSE("GPL");
