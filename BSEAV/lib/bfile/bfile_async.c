/***************************************************************************
 *     Copyright (c) 2007, Broadcom Corporation
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
 * Single thread asynchronous I/O
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#include "bstd.h"
#include "bfile_async.h"
#include "blst_queue.h"

BDBG_MODULE(bfile_async);

typedef struct b_file_async_item {
	BLST_Q_ENTRY(b_file_async_item) queue;
	void *buf; /* copy of buf */
	size_t length;
	void *cntx;
	bool write;
	void (*cont)(void *cntx, ssize_t size);
	union {
		bfile_io_read_t read;
		bfile_io_write_t write;
	} fd;
} b_file_async_item;

BLST_Q_HEAD(b_file_async_queue, b_file_async_item);

static struct {
	struct b_file_async_queue queue;
	struct b_file_async_queue free;
	b_file_async_item items[30];
} b_file_async;

void 
bfile_async_init(void)
{
	unsigned i;

	BLST_Q_INIT(&b_file_async.queue);
	BLST_Q_INIT(&b_file_async.free);
	for(i=0;i<sizeof(b_file_async.items)/sizeof(b_file_async.items[0]);i++) {
		BLST_Q_INSERT_TAIL(&b_file_async.free, &b_file_async.items[i], queue);
	}
	return;
}

void 
bfile_async_shutdown(void)
{
	return;
}

void
bfile_async_write(void *sync_cnxt, bfile_io_write_t fd, const void *buf, size_t length, void (*cont)(void *, ssize_t ), void *cntx)
{
	b_file_async_item *item;
	BSTD_UNUSED(sync_cnxt);
	item = BLST_Q_FIRST(&b_file_async.free);
	if(item){
		BLST_Q_REMOVE_HEAD(&b_file_async.free, queue);
		item->buf = (void *)buf;
		item->length = length;
		item->cntx = cntx;
		item->cont = cont;
		item->write = true;
		item->fd.write = fd;
		BLST_Q_INSERT_TAIL(&b_file_async.queue, item, queue);
		return;
	}
	BDBG_ERR(("bfile_async_write: No free I/O entries avaliable"));
	cont(cntx,-1);
	return;
}

void
bfile_async_read(void *sync_cnxt, bfile_io_read_t fd, void *buf, size_t length, void (*cont)(void *, ssize_t ), void *cntx)
{
	b_file_async_item *item;
	BSTD_UNUSED(sync_cnxt);
	item = BLST_Q_FIRST(&b_file_async.free);
	if(item){
		BLST_Q_REMOVE_HEAD(&b_file_async.free, queue);
		item->buf = buf;
		item->length = length;
		item->cntx = cntx;
		item->cont = cont;
		item->write = false;
		item->fd.read = fd;
		BLST_Q_INSERT_TAIL(&b_file_async.queue, item, queue);
		return;
	}
	BDBG_ERR(("bfile_async_write: No free I/O entries avaliable"));
	cont(cntx,-1);
	return;
}

void
bfile_async_process(void)
{
	b_file_async_item *item;

	while( NULL!=(item = BLST_Q_FIRST(&b_file_async.queue))) {
		ssize_t rc;
		if(item->write) {
			rc = item->fd.write->write(item->fd.write, item->buf, item->length);
		} else {
			rc = item->fd.read->read(item->fd.read, item->buf, item->length);
		}
		BLST_Q_REMOVE_HEAD(&b_file_async.queue, queue);
		BLST_Q_INSERT_HEAD(&b_file_async.free, item, queue);
		item->cont(item->cntx, rc);
	}
	return;
}


