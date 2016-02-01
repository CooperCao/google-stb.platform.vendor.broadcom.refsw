/*
 * $Id$
 *
 * Character-device access to raw MTD devices.
 * Change log
 * 02/11/08, - detect erase_size at mtd_open,
 *           - add dirty flag to get file size from buffer
 *           - add debug flag at run time
 *           - NAND flash: erase nonvol before first write, to avoid write error.
 * 
 *
 */

#include <linux/config.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/sched.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/compatmac.h>
#include <linux/kthread.h>   /* kthread */

#include <asm/uaccess.h>
#include <linux/version.h>

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,17)
#define MTD_READ(mtd, args...) (*(mtd->read))(mtd, args)
#define MTD_WRITE(mtd, args...) (*(mtd->write))(mtd, args)
#endif

#define MTD_NONVOL_MAJOR    98 /* not used by anyone */
#define NONVOL_SEG_MAGIC 0x4d435242  /* magic number at the end of a segment */
#define NONVOL_MAX_SIZE  0x20000  /* the size used by nonvol, <= flash partition size */
#define TO_MTDNONVOL_CTL(file) (struct nonvol_struct *)((long)((file)->private_data) & ~3L)
#define TO_MTDNONVOL(ctl) (struct mtd_info *)((struct nonvol_struct *)(ctl))->mtd

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

static unsigned short debug = 0;
module_param(debug, ushort, S_IRUGO);

// The two sections that can be read/written.  Typically, Permanent settings
// are only written during manufacturing, but Dynamic settings can be written
// at any time during normal operation of the system.

#define PERM_NONVOL_SECTION	0x01
#define DYN_NONVOL_SECTION      0x00

#define MAX_MESSAGE_SIZE        1458
#define TFTP_BLOCK_SIZE         512

#define MAX_NONVOL_DEV 2
#define NONVOLDEV     "nonvol"
#define N_RESERVED_BYTES 8 /* bytes reserved */

struct nonvol_struct {
	/* fields updated by init */
	char name[64];
	int devnum;
	int erase_size;  
	int size;
	int minSegments;
	int dirty; /* cache is dirty, data should be read from cache */

#ifdef CONFIG_MTD_BRCMNAND
	int nand_buf_size;    /* nand flash has bad sectors, this size is the actual size used by nonvol */
	uint8_t * nand_buf; /* nand flash is r/w in sectors, this buffer holds the data before flush to flash */
	int nand_buf_ready; /* the data in nand_buf is ready for use */
	int partition_erased; /* after driver loaded, before first write, need to erase the whole partition */
#endif
	uint8_t * buf; 
	struct task_struct * flush_thread;
	int kill_thread;
	wait_queue_head_t wait;

	/* fields cleaned by each open/close */
	struct mtd_info * mtd;
	int pos;  /* offset in the segment, it increases with every read/write */

	/* fields cleand by flush thread */
	uint32_t options; 
#define FLUSH    0x00000001

#define FLUSH_SET(ctr)  ((ctr)->options |= FLUSH)
#define FLUSH_CLEAN(ctr)  ((ctr)->options &= (~FLUSH))
#define FLUSH_CHECK(ctr)  ((ctr)->options & FLUSH)

};



struct nonvol_struct nonvol_ctl[MAX_NONVOL_DEV];

struct nonvol_partition {
	char name[32];
	int devnum;         /* mtd device number */
	int size;           /* mtd partition size */
#ifdef CONFIG_MTD_BRCMNAND
	int nand_size;    /* nand flash has bad sectors, 
			   * this size is the actual size used by nonvol,
			   * it should be (erase_szie * n), n = 1, 2, ... */
#endif
};
/* 
 * The nonvol device should change accordingly when flash partition in
 * bcm9xxxx-flash.c and cfe are changed
 */
#ifdef CONFIG_MTD_BRCMNAND
struct nonvol_partition  nonvol_dev[MAX_NONVOL_DEV] = {
	{"dyncfg", 2, 0x20000, NONVOL_MAX_SIZE}, /* dyncfg, mtd2, only part of the dyncfg partition is used */
	{"permcfg", 3, 0x20000, NONVOL_MAX_SIZE}  /* permcfg, mtd3, only part of the dyncfg partition is used */
};
#else
struct nonvol_partition  nonvol_dev[MAX_NONVOL_DEV] = {
	{"dyncfg", 2, 0x20000}, /* dyncfg, mtd2 */
	{"permcfg", 3, 0x20000}  /* permcfg, mtd3 */
};
#endif
DECLARE_MUTEX(flush_mutex);

//********************** Local Variables *************************************

static unsigned int gMinSegments[2] =
{
    1,      // kNonVolDriverDynamicSection
    1       // kNonVolDriverPermanentSection
};

//********************** Local Functions *************************************

static int mtdnonvol_fill_buf(struct nonvol_struct * ctl, int force ) {
	int ret = 0;

#ifdef CONFIG_MTD_BRCMNAND
	if (ctl->nand_buf_ready == 0 || force) {
		struct mtd_info * mtd = TO_MTDNONVOL(ctl);
		size_t retlen=0;
		if ((ret = MTD_READ(mtd, 0, ctl->nand_buf_size, &retlen, ctl->nand_buf)) < 0) {
			DBG_ERR("Failed to read from NAND flash\n");
			return ret;
		}
		if (retlen != ctl->nand_buf_size) {
			DBG_ERR("Not all data are read from NAND flash, read %d, expect %d\n", retlen, ctl->nand_buf_size);
			ret = -EINVAL;
			return ret;
		}
		ctl->nand_buf_ready = 1;
	}
#endif

	return ret;
}

static int mtd_read_from_buf(struct nonvol_struct * ctl, loff_t from, size_t len,
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
	struct mtd_info * mtd = TO_MTDNONVOL(ctl);
	ret = MTD_READ(mtd, from, len, retlen, buf);
#endif
	return ret;
}

static int mtd_write_to_buf(struct nonvol_struct * ctl, loff_t from, size_t len,
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
	struct mtd_info * mtd = TO_MTDNONVOL(ctl);
	ret = MTD_WRITE(mtd, from, len, retlen, buf);
#endif

	return ret;
}
/*
 * This is the function that actually write to nand flash,  
 * the offset and len must at least aligned to page boundary of MTD device,
 * error will be reported by MTD_WRITE
 */
static int FlashDriverFlush(struct nonvol_struct * ctl, int offset, uint32_t len) {
	int ret = 0;
#ifdef CONFIG_MTD_BRCMNAND
	struct mtd_info * mtd = TO_MTDNONVOL(ctl);
	size_t retlen=0;

	DBG_INFO("flush 0x%x bytes to 0x%x\n", len, offset);
	if ((ret = MTD_WRITE(mtd, offset, len, &retlen, ctl->nand_buf + offset)) < 0) {
		DBG_ERR("flash write failed\n");
	}
#endif
	return ret;
}


static uint32_t FlashDriverRead32(struct nonvol_struct * ctl, int offset)
{
	int val;
	size_t retlen=0;

	if (mtd_read_from_buf(ctl, offset, 4, &retlen, (uint8_t *)&val) < 0) {
	    DBG_INFO("FlashDriverRead32 failed at 0x%x\n", offset);
	    val =  0xffffffff;
	}
	/* DBG_INFO("FlashDriverRead32 0x%x at 0x%x\n", val, offset); */
	return val;
}

static int FlashDriverWrite32(struct nonvol_struct * ctl, int offset, uint32_t val)
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

// Note that this can return 0 if nonvol has not yet been initialized or if
// the control word has been corrupted.
//
// Parameters:
//      section - the section to query.
//
// Returns:
//      The segment size for the section of nonvol.
//
static uint32_t ReadTrueSegmentSizeBytes(struct nonvol_struct * ptr)
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
	if (magic != NONVOL_SEG_MAGIC) 
		goto err;
	// Calculate the minimum size that is allowed for a segment.  Since we only
	// have 16 bits to mark segment usage, the minimum size is 1/16th of the
	// region size.
	minSegmentSize =  size / 16;
	
	// Calculate the maximum size that is allowed for a segment, which depends
	// on the minimum number of segments that we must have in nonvol.
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



static int FlashDriverReadArray(struct nonvol_struct * ctl, uint32_t offset, uint8_t * pBuffer, 
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

static int FlashDriverWriteArray(struct nonvol_struct * ctl, uint32_t offset, const uint8_t * pBuffer, 
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

static void mtdnonvol_erase_callback (struct erase_info *instr)
{
	wake_up((wait_queue_head_t *)instr->priv);
}

static int memerase(struct mtd_info * mtd, struct erase_info *erase) {
	int ret = 0;

	wait_queue_head_t waitq;
	DECLARE_WAITQUEUE(wait, current);

	init_waitqueue_head(&waitq);
	
	erase->mtd = mtd;
	erase->callback = mtdnonvol_erase_callback;
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
		DBG_INFO("\rPerforming Flash Erase of length %u at offset 0x%x",
				erase.len, erase.addr);

		if(memerase(mtd, &erase) != 0)
		{
			DBG_ERR("\nMTD Erase failure");
			return 8;
		}


		start += erase.len;
		if(start >= (r->offset + r->numblocks*r->erasesize))
		{ //We finished region i so move to region i+1
			DBG_INFO("\nMoving to region %d\n", i+1);
			i++;
		}
	}
	
	DBG_INFO(" done\n");
	
	return 0;
}

static int non_region_erase(struct mtd_info * mtd, int start, int count, int unlock)
{
	struct erase_info erase;

	erase.addr = start;

	erase.len = mtd->erasesize;

	for (; count > 0; count--) {
		DBG_INFO("non_region_erase\n");
		DBG_INFO("\rPerforming Flash Erase of length %u at offset 0x%x",
			 erase.len, erase.addr);
		if(unlock != 0)
		{
			//Unlock the sector first.
			DBG_INFO("\rPerforming Flash unlock at offset 0x%x",erase.addr);
			if ( memunlock(mtd, &erase) != 0) {
				DBG_ERR("\nMTD Unlock failure");
				return 8;
			}
		}
		if(memerase(mtd, &erase) != 0) {
			DBG_ERR("\nMTD Erase failure");
			return 8;
		}
		erase.addr += mtd->erasesize;
	}
	DBG_INFO(" done\n");
	return 0;
}


static int FlashDriverEraseRegion(struct mtd_info * mtd, int count) 
{
	int ret = 0;
	int regcount;
	int start =0;
	int unlock = 0;

	regcount = mtd->numeraseregions;
	if(regcount == 0) {
		ret = non_region_erase(mtd, start, count, unlock);
	}
	else {
		ret = region_erase(mtd, start, count, unlock, regcount);
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
static int GetCurrentRegion(struct nonvol_struct * ctl, unsigned int segSize, 
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
		if (magic != NONVOL_SEG_MAGIC) 
			break;
	}
	if (magic != 0xffffffff && magic != NONVOL_SEG_MAGIC) {
		DBG_ERR("Invalid Segment(offset 0x%x, size 0x%x). There is no valid data to read.\n", offset , size);
		*regionOffset = offset;
		ret = SEG_INVAL;
		return ret;
	}
	if (offset == segSize) {
		DBG_INFO("Segment empty (offset 0x%x, size 0x%x). There is no valid data to read.\n", offset, size);
		*regionOffset = offset;
		ret = SEG_EMPTY;
	} else if (offset > (size - segSize)) {
		DBG_INFO("Segment full (offset 0x%x, size 0x%x).\n", offset, size);
		*regionOffset = (offset - segSize);
		ret = SEG_FULL;
	} else {
		*regionOffset = (offset - segSize);
		ret = SEG_OK;
	}

	return ret;
}

// Reads a segment of nonvol data from the specified section in flash memory to
// the buffer specified.  It will read the smaller of the current segment size
// or the specified buffer size.  It always reads the most recently written
// segment, as specified by the segment bitmask in the control word.
//
// There is no indication as to the number of bytes of valid data that were
// previously written to the segment, so there is no indication of how many
// valid bytes were read.  The client must embed this information in the data
// if it is desired.
//
static int NonVolDriverRead(struct nonvol_struct * ctl, uint8_t *pBuffer, loff_t offset,
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
    // nonvol hasn't been written yet.

    offset += regionOffset;

    if ((offset + bufferSize) <= size)
    {

	    if ( (result = FlashDriverReadArray(ctl, offset, pBuffer, bufferSize)) >= 0) {
		    // DBG_INFO("%d bytes read from flash\n", result);
	    } else {
		    DBG_ERR("Failed to read from nonvol\n");
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
static int NonVolDriverWrite(struct nonvol_struct * ctl, const uint8_t *pBuffer, 
			    uint32_t numberOfBytes)
{
	int result = 0;
	unsigned int segmentSizeBytes;

	unsigned int regionOffset;

	unsigned int eraseRegion = 0;
	unsigned int regionErased;
	int size, erase_size, minSegments;
	struct mtd_info * mtd = TO_MTDNONVOL(ctl);

	size = ctl->size;
	erase_size = ctl->erase_size;
	minSegments = ctl->minSegments;
	
	// Error check - make sure the number of bytes is less than the max
	// segment size.  If it's greater, then we can't possibly write it.
	if ((numberOfBytes)> size ) {
		DBG_INFO("Too much data to fit in nonvol!  Buffer size %u > max segment size %d!\n",
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
			for (regionDivisor = 16; regionDivisor >= minSegments; regionDivisor--) {
				// Divide the nonvol region into the number of segments for the
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
			result = FlashDriverEraseRegion(mtd, size / erase_size);

			if (result < 0)  {
				DBG_INFO("Failed to erase the flash region!\n");
				break;
			}
			/* reload the data from flash */
			mtdnonvol_fill_buf(ctl, 1);

			// Write the segment size to the control word.  This goes 8 bytes
			// from the beginning, since NAND requires sequencial writing.
			FlashDriverWrite32(ctl, 8, segmentSizeBytes);
			FlashDriverWrite32(ctl, segmentSizeBytes - N_RESERVED_BYTES, NONVOL_SEG_MAGIC);
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
		result = FlashDriverWrite32(ctl, regionOffset + segmentSizeBytes - N_RESERVED_BYTES, NONVOL_SEG_MAGIC);
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
			mtdnonvol_fill_buf(ctl, 1);

			result = FlashDriverReadArray(ctl, regionOffset, tmpbuf, numberOfBytes);
			
			if (result < 0) {
				DBG_ERR("Failed to read from nonvol\n");
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
	struct nonvol_struct *ctl = (struct nonvol_struct *) arg;
	uint8_t * buf;
	int actual_size;

	while (1) {
		wait_event_interruptible_timeout(ctl->wait, FLUSH_CHECK(ctl), 200);
		down_interruptible(&flush_mutex);
		buf = ctl->buf;
		actual_size = ctl->pos;
		if (FLUSH_CHECK(ctl)) {
			DBG_INFO("%s: flushing to flash, size %d\n", ctl->name, actual_size);
			if (NonVolDriverWrite(ctl, buf, actual_size) < 0) {
				DBG_ERR ("nonvol: flash write failed!\n");
				up(&flush_mutex);
				break;
			}
			FLUSH_CLEAN(ctl);
			ctl->pos = 0; /* reset position */
			ctl->dirty = 0;

		}
		up(&flush_mutex);
		if (kthread_should_stop())
			break;
		//set_current_state(TASK_INTERRUPTIBLE);
		//schedule_timeout(300);
	}
	return 0;

}


/* hardcode the mtd dev num */
static int search_nonvol_dev(void) {
	int ret = 0;
	int minor = 0;
	struct nonvol_struct * ctl;
	char str[64];

	DBG_INFO("searching nondev flash\n");
	for (minor = 0; minor < MAX_NONVOL_DEV; minor ++) {
		ctl = &nonvol_ctl[minor];
		sprintf(ctl->name, "%s", nonvol_dev[minor].name);
		ctl->devnum = nonvol_dev[minor].devnum;
		ctl->size = nonvol_dev[minor].size;
		ctl->minSegments = gMinSegments[minor]; 
		ctl->dirty = 0;
		ctl->buf = kmalloc(ctl->size, GFP_KERNEL);
		if (!ctl->buf) {
			DBG_ERR("ctl->buf out of memory!\n");
			ret = -ENOMEM;
			goto err;
		}
#ifdef CONFIG_MTD_BRCMNAND
		ctl->nand_buf_size = nonvol_dev[minor].nand_size;
		ctl->nand_buf = kmalloc(ctl->nand_buf_size, GFP_KERNEL);
		if (!ctl->nand_buf) {
			DBG_ERR("ctl->nand_buf out of memory!\n");
			ret = -ENOMEM;
			goto err;
		}
		ctl->nand_buf_ready = 0;
		ctl->partition_erased = 0;
#endif
		sprintf(str, "%s", (minor == DYN_NONVOL_SECTION) ? "dyncfg" : "permcfg");
		ctl->flush_thread = kthread_run(flush_func, ctl, str);
		if (IS_ERR(ctl->flush_thread)) {
			goto err;
		}
		init_waitqueue_head(&ctl->wait);
	}

	return ret; 
err:
	for (minor = 0; minor < MAX_NONVOL_DEV; minor++) {
		ctl = &nonvol_ctl[minor];
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

static int NonVolDriverFileSize(struct nonvol_struct * ctl) {
	int len;
	if (ctl->dirty) {
		/* data has not flushed yet, read from mem */
		len = *(int*)(ctl->buf);
	} else {
		if (NonVolDriverRead(ctl, (uint8_t *) &len, 0, 4) < 0) {
			/* no valid data in the flash, the file size is 0*/
			len = 0;
		}
	}
	len = ntohl(len);
	if (len < 0)  len = 0; /* empty file */
	/* DBG_INFO("%s file size %d\n", ctl->name, len); */
	return len;
}

static loff_t mtdnonvol_llseek (struct file *file, loff_t offset, int orig)
{
	struct nonvol_struct * ctl = (struct nonvol_struct *)file->private_data;
	loff_t ret = 0;
	int data_size = NonVolDriverFileSize(ctl);


	if (data_size == 0) {
		/* data has been written to buffer, but not flushed to flash yet */
		data_size = ctl->pos;
	}
	down_interruptible(&flush_mutex);
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
	up(&flush_mutex);
	return ret;
}



static int mtdnonvol_open(struct inode *inode, struct file *file)
{
	struct mtd_info *mtd;
	struct nonvol_struct * ctl;
	unsigned int minor = iminor(inode);
	int ret = 0;

	if (minor >= MAX_NONVOL_DEV) 
		return -ENODEV;
	ctl = &nonvol_ctl[minor];
	DBG_INFO( "%s: mtdnonvol_open %u\n", ctl->name, minor);

	/* You can't open the RO devices RW */
	/*
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
	down_interruptible(&flush_mutex);
	ctl->mtd = mtd;
	ctl->pos = 0;
	ctl->dirty = 0;
	ctl->erase_size = mtd->erasesize;
	up(&flush_mutex);
	file->private_data = ctl;

	/* You can't open it RW if it's not a writeable device */
	/*
	if ((file->f_mode & 2) && !(mtd->flags & MTD_WRITEABLE)) {
		put_mtd_device(mtd);
		return -EACCES;
	}
	*/

	if ((ret = mtdnonvol_fill_buf(ctl, 1)) < 0) {
		return ret;
	}

	return 0;
} 

/*====================================================================*/

static int mtdnonvol_close(struct inode *inode, struct file *file)
{
	struct nonvol_struct * ctl ;
	struct mtd_info *mtd;

	ctl = TO_MTDNONVOL_CTL(file);
	DBG_INFO("%s: MTD_close\n", ctl->name);
	mtd = TO_MTDNONVOL(ctl);
	ctl->dirty = 0;

	if (mtd->sync)
		mtd->sync(mtd);
	put_mtd_device(mtd);
	return 0;
} 

static ssize_t mtdnonvol_read(struct file *file, char __user *buf, size_t count,loff_t *ppos)
{
	int ret=0;
	struct nonvol_struct * ctl = TO_MTDNONVOL_CTL(file);

#ifdef CONFIG_MTD_BRCMNAND
	/*
	 * read the whole nand buffer, this is not efficient. But in cases where
	 * other utilities can modify the flash content (nandwrite, eraseall, etc), 
	 * this seems to be the  only way to guarantee data consistency.
	 * In production, force should be set to 0
	 */
	if ((ret = mtdnonvol_fill_buf(ctl, 1)) < 0) {
		return ret;
	}
#endif
	DBG_INFO("%s: read offset %d, count %d\n", ctl->name, ctl->pos, count);

	if ((ret = NonVolDriverRead(ctl, ctl->buf, ctl->pos, count)) < 0) {
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

static ssize_t mtdnonvol_write(struct file *file, const char __user *buf, size_t count,loff_t *ppos)
{
	struct nonvol_struct * ctl = (struct nonvol_struct *)file->private_data;
	uint8_t * dest = ctl->buf;
	int offset = ctl->pos;
	int size = ctl->size;
	ssize_t ret = count;

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
		 * rnonvolhost: If the received packet length is not 1450 (MAX_MESSAGE_SIZE - 8), 
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
		/* printk("write %d offset %d\n", count, offset); */
	}
	else {
		DBG_ERR("write out of boundary\n");
		ret = -EFAULT;
	}
	up(&flush_mutex);
	return ret;
}


/*======================================================================

    IOCTL calls for getting device parameters.

======================================================================*/

static int mtdnonvol_ioctl(struct inode *inode, struct file *file,
		     u_int cmd, u_long arg)
{
	int ret = -EFAULT;
	DBG_INFO("MTD_ioctl\n");
	return ret;
} 

static struct file_operations mtdnonvol_fops = {
	.owner		= THIS_MODULE,
	.llseek		= mtdnonvol_llseek,
	.read		= mtdnonvol_read,
	.write		= mtdnonvol_write,
	.ioctl		= mtdnonvol_ioctl,
	.open		= mtdnonvol_open,
	.release	= mtdnonvol_close,
};

static int __init init_mtdnonvol(void)
{
	int ret = -ENODEV;
	/* search for flash partition */

	if ((ret = search_nonvol_dev())<0) {
		DBG_ERR("nonvol device init failed\n");
		goto init_search_err;
	}

	if (register_chrdev(MTD_NONVOL_MAJOR, NONVOLDEV, &mtdnonvol_fops)) {
		DBG_ERR("unable to get major %d for misc devices\n",
		       MTD_NONVOL_MAJOR);
		ret = -EIO;
		goto init_register_err;
	}

	DBG_INFO("%s initalized\n", NONVOLDEV);
 init_register_err:
	
 init_search_err:
	return ret;

}

static void __exit cleanup_mtdnonvol(void)
{
	int minor;
	struct nonvol_struct * ctl;
	
	for (minor = 0; minor < MAX_NONVOL_DEV; minor++) {
		ctl = &nonvol_ctl[minor];
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

	unregister_chrdev(MTD_NONVOL_MAJOR, NONVOLDEV);

	DBG_INFO("%s removed\n", NONVOLDEV);
}

module_init(init_mtdnonvol);
module_exit(cleanup_mtdnonvol);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("");
MODULE_DESCRIPTION("Direct access to MTD nonvol devices");
