/***************************************************************************
 *     Copyright (c) 2005, Broadcom Corporation
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

#include "bstd.h"
#include "bfifo.h"

BDBG_MODULE(bfifo);

void
bfifo_init(bfifo *fifo, void *base, size_t size)
{
	fifo->base = base;
	fifo->last= (uint8_t*)base + size;
	fifo->read_ptr = fifo->write_ptr = base;
	fifo->read_cnt = fifo->write_cnt = 0;
	return;
}

void *
bfifo_write_peek(bfifo *fifo, size_t *size)
{
	if (fifo->write_cnt > fifo->read_cnt && fifo->read_ptr == fifo->write_ptr) {
		*size = 0;
		BDBG_MSG(("write_peek W:%#x R:%#x %u",  (unsigned)fifo->write_ptr, (unsigned)fifo->read_ptr, *size));
		return NULL; /* no space avaliable */
	}
	/* |====W---R===| */
	if (fifo->write_ptr < fifo->read_ptr) {
		*size = (uint8_t *)fifo->read_ptr - (uint8_t *)fifo->write_ptr;
		BDBG_MSG(("write_peek W:%#x R:%#x %u",  (unsigned)fifo->write_ptr, (unsigned)fifo->read_ptr, *size));
		return fifo->write_ptr;
	} else {
	/* |---R===W---| */
		*size = (uint8_t *)fifo->last - (uint8_t *)fifo->write_ptr;
		BDBG_MSG(("write_peek R:%#x W:%#x L:%#x %u",  (unsigned)fifo->read_ptr, (unsigned)fifo->write_ptr, (unsigned)fifo->last, *size));
		return fifo->write_ptr;
	}
}

void
bfifo_write_commit(bfifo *fifo, size_t size)
{
	size_t test_size;
	BSTD_UNUSED(test_size);

	BDBG_MSG(("write_commit %u", size));
	BDBG_ASSERT(bfifo_write_peek(fifo, &test_size)!=NULL);
	BDBG_ASSERT(test_size >= size);

	fifo->write_ptr = (uint8_t *)fifo->write_ptr + size;
	if ((uint8_t *)fifo->write_ptr == fifo->last) {
		fifo->write_ptr = fifo->base;
		fifo->write_cnt ++;
	}
	return;
}

void *
bfifo_read_peek(bfifo *fifo, size_t *size)
{
	if (fifo->write_cnt == fifo->read_cnt && fifo->read_ptr == fifo->write_ptr) {
		*size = 0;
		BDBG_MSG(("read_peek R:%#x W:%#x %u", (unsigned)fifo->read_ptr,  (unsigned)fifo->write_ptr, *size));
		return NULL; /* no data avaliable */
	}
	/* |====W---R===| */
	if (fifo->write_ptr <= fifo->read_ptr) {
		*size = (uint8_t *)fifo->last - (uint8_t *)fifo->read_ptr;
		BDBG_MSG(("read_peek W:%#x R:%#x L:%x %u",  (unsigned)fifo->write_ptr, (unsigned)fifo->read_ptr, (unsigned)fifo->last, *size));
		return fifo->write_ptr;
	} else {
	/* |---R===W---| */
		*size =  (uint8_t *)fifo->write_ptr - (uint8_t *)fifo->read_ptr;
		BDBG_MSG(("read_peek R:%#x W:%#x %u", (unsigned)fifo->read_ptr,  (unsigned)fifo->write_ptr, *size));
		return fifo->write_ptr;
	}
}

void
bfifo_read_commit(bfifo *fifo, size_t size)
{
	size_t test_size;
	BSTD_UNUSED(test_size);

	BDBG_MSG(("read_commit %u", size));

	BDBG_ASSERT(bfifo_read_peek(fifo, &test_size)!=NULL);
	BDBG_ASSERT(test_size >= size);

	fifo->read_ptr = (uint8_t *)fifo->read_ptr + size;
	if (fifo->read_ptr == fifo->last) {
		fifo->read_ptr = fifo->base;
		fifo->read_cnt ++;
	}
	return;
}


