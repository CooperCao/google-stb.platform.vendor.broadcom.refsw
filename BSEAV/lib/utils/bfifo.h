/***************************************************************************
 *     Copyright (c) 2005-2007, Broadcom Corporation
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
 * Utility to manage circular FIFO 
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#ifndef BFIFO_H__
#define BFIFO_H__

typedef struct bfifo bfifo;

/* this function initialized FIFO */
void bfifo_init(bfifo *fifo, void *base, size_t size);

/* this function returns size of continous space aliable for write operation */
void *bfifo_write_peek(bfifo *fifo, size_t *size);

/* this function advances write pointer */
void bfifo_write_commit(bfifo *fifo, size_t size);

/* this function returns size of continous space aliable for read operation */
void *bfifo_read_peek(bfifo *fifo, size_t *size);

/* this function advances read pointer */
void bfifo_read_commit(bfifo *fifo, size_t size);

/* this structure shall not be ever used directly, all access to the FIFO's should go thru the function above */
struct bfifo {
	void *base; /* FIFO's start address */
	void *last; /* FIFO's end address */
	void *read_ptr; 
	void *write_ptr;
	unsigned read_cnt; /* wraparound counter for the read pointer */
	unsigned write_cnt; /* wraparound counter for the write pointer */
};

#define BFIFO_HEAD(name, type) struct name { type *bf_base; type *bf_last; type *bf_read; type *bf_write; int bf_wrap; }

#define BFIFO_INIT(fifo, base, size) do {(fifo)->bf_wrap=0; \
	(fifo)->bf_base=(fifo)->bf_read=(fifo)->bf_write=(base);\
	(fifo)->bf_last=(fifo)->bf_base+(size);}while(0)

#define BFIFO_WRITE(fifo) (fifo)->bf_write
#define BFIFO_READ(fifo) (fifo)->bf_read

#define BFIFO_WRITE_PEEK(fifo) \
	/* |====W---R===| */ ((unsigned)(((fifo)->bf_write < (fifo)->bf_read) ? (fifo)->bf_read - (fifo)->bf_write : ( \
	/* |---R===W---| */ ((fifo)->bf_write > (fifo)->bf_read) ? (fifo)->bf_last - (fifo)->bf_write  : ( \
	/* |---RW---| */ (fifo)->bf_wrap ? 0 : (fifo)->bf_last - (fifo)->bf_write))))

#define BFIFO_READ_PEEK(fifo) \
	/* |====W---R===| */ ((unsigned)(((fifo)->bf_write < (fifo)->bf_read) ? (fifo)->bf_last - (fifo)->bf_read : ( \
	/* |---R===W---| */ ((fifo)->bf_write > (fifo)->bf_read) ? (fifo)->bf_write - (fifo)->bf_read : ( \
	/* |---RW---| */ (fifo)->bf_wrap ? (fifo)->bf_last - (fifo)->bf_read:0))))

#define BFIFO_WRITE_COMMIT(fifo, size) do { \
	BDBG_ASSERT((size)>0 && BFIFO_WRITE_PEEK(fifo) >= (size));  \
	(fifo)->bf_write += (size); \
	if ((fifo)->bf_write >= (fifo)->bf_last) {(fifo)->bf_write = (fifo)->bf_base;(fifo)->bf_wrap++;}  \
	} while(0)

#define BFIFO_READ_COMMIT(fifo, size) do { \
	BDBG_ASSERT(BFIFO_READ_PEEK(fifo) >= (unsigned)(size)); \
	(fifo)->bf_read += (size); \
	if ((fifo)->bf_read >= (fifo)->bf_last) {(fifo)->bf_read = (fifo)->bf_base;(fifo)->bf_wrap--;}  \
	} while(0)

#define BFIFO_WRITE_LEFT(fifo)  \
	/* |====W---R===| */ (((fifo)->bf_write < (fifo)->bf_read) ? (fifo)->bf_read - (fifo)->bf_write : ( \
	/* |---R===W---| */ ((fifo)->bf_write > (fifo)->bf_read) ? ((fifo)->bf_read - (fifo)->bf_base) + ((fifo)->bf_last - (fifo)->bf_write)  : ( \
	/* |---RW---| */ (fifo)->bf_wrap ? 0 : (fifo)->bf_last - (fifo)->bf_base)))

#define BFIFO_READ_LEFT(fifo) \
	/* |====W---R===| */ (((fifo)->bf_write < (fifo)->bf_read) ? ((fifo)->bf_write - (fifo)->bf_base) + (fifo)->bf_last - (fifo)->bf_read : ( \
	/* |---R===W---| */ ((fifo)->bf_write > (fifo)->bf_read) ? (fifo)->bf_write - (fifo)->bf_read : ( \
	/* |---RW---| */ (fifo)->bf_wrap ? (fifo)->bf_last - (fifo)->bf_base:0)))

#define BFIFO_VALIDATE(fifo) do { \
		BDBG_ASSERT((fifo)->bf_wrap==0 || (fifo)->bf_wrap==1); \
		BDBG_ASSERT((fifo)->bf_read>=(fifo)->bf_base && (fifo)->bf_read<(fifo)->bf_last); \
		BDBG_ASSERT((fifo)->bf_write>=(fifo)->bf_base && (fifo)->bf_write<(fifo)->bf_last); \
	} while(0)

#define BFIFO_STATUS(DBG, header,fifo)	DBG((header ":(%#lx:%#lx) write (%u/%u/%#lx), read (%u/%u/%#lx)", (unsigned long)(fifo)->bf_base, (unsigned long)(fifo)->bf_last, BFIFO_WRITE_PEEK(fifo), BFIFO_WRITE_LEFT(fifo), (unsigned long)BFIFO_WRITE(fifo), BFIFO_READ_PEEK(fifo), BFIFO_READ_LEFT(fifo), (unsigned long)BFIFO_READ(fifo)))


#endif /* BFIFO_H__ */


