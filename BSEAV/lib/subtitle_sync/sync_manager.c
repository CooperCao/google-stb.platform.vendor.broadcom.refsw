/***************************************************************************
 *     Copyright (c) 2007-2008, Broadcom Corporation
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
 * Subtitle sync library
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "blst_squeue.h"
#include "sync_manager.h"


BDBG_MODULE(sync_manager);
#define BDBG_MSG_TRACE(x) BDBG_MSG(x)


BDBG_OBJECT_ID(btsm_queue);


typedef struct b_tsm_queue_entry {
    btsm_queue_entry entry;
    BLST_SQ_ENTRY(b_tsm_queue_entry) link;	  	  
} b_tsm_queue_entry;

struct btsm_queue {
    BDBG_OBJECT(btsm_queue)
    BLST_SQ_HEAD(b_tsm_queue_fifo, b_tsm_queue_entry) free, queued;
    btsm_queue_status status;
    bool started;
    bool prev_timestamp_valid;
    uint32_t prev_timestamp;
    btsm_queue_settings settings;
    btsm_queue_config config;
    b_tsm_queue_entry entries[1]; /* variable size array */
};


void 
btsm_queue_get_default_config(btsm_queue_config *config)
{
    BDBG_MSG_TRACE(("btsm_queue_get_default_config"));
    BDBG_ASSERT(config);
    config->length = 64;
    return;
}

btsm_queue_t 
btsm_queue_create(const btsm_queue_config *config)
{
    btsm_queue_t  queue;
    BERR_Code rc;
    unsigned i;

    BDBG_ASSERT(config);
    BDBG_MSG_TRACE(("btsm_queue_create"));

    queue = BKNI_Malloc(sizeof(*queue)+sizeof(struct b_tsm_queue_entry)*config->length);
    if(!queue) { rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto err_alloc;}
    BDBG_OBJECT_INIT(queue, btsm_queue);
    BKNI_Memset(&queue->status, 0, sizeof(queue->status));
    queue->started = false;
    queue->prev_timestamp_valid = false;
    queue->settings.entry_duration = 1;
    queue->settings.drop_threshold = 0;
    queue->config = *config;
	BLST_SQ_INIT(&queue->queued);
	BLST_SQ_INIT(&queue->free);
    for(i=0;i<config->length;i++) {
        BLST_SQ_INSERT_TAIL(&queue->free, queue->entries+i, link);
    }
    return queue;

err_alloc:
    return NULL;
}

void 
btsm_queue_destroy(btsm_queue_t queue)
{
    BDBG_MSG_TRACE(("btsm_queue_destroy:%#lx", (unsigned)queue));
    BDBG_OBJECT_ASSERT(queue, btsm_queue);
    BDBG_OBJECT_DESTROY(queue, btsm_queue);
    BKNI_Free(queue);
    return;
}

void 
btsm_queue_get_settings(btsm_queue_t queue, btsm_queue_settings *settings)
{
    BDBG_MSG_TRACE(("btsm_queue_get_settings:%#lx", (unsigned)queue));
    BDBG_OBJECT_ASSERT(queue, btsm_queue);
    BDBG_ASSERT(settings);
    *settings = queue->settings;
    return;
}

int
btsm_queue_start(btsm_queue_t queue, const btsm_queue_settings *settings)
{
    b_tsm_queue_entry *entry;

    BDBG_MSG_TRACE(("btsm_queue_start:%#lx", (unsigned)queue));
    BDBG_OBJECT_ASSERT(queue, btsm_queue);
    if(settings) {
        queue->settings = *settings;
    }

    while(NULL!=(entry=BLST_SQ_FIRST(&queue->queued))) {
        BLST_SQ_REMOVE_HEAD(&queue->queued, link);
        BLST_SQ_INSERT_HEAD(&queue->free, entry, link);
    }
    BKNI_Memset(&queue->status, 0, sizeof(queue->status));
    queue->status.estimated_entry_duration = queue->settings.entry_duration;
    queue->prev_timestamp_valid = false;
    queue->started = true;

    return 0;
}

void 
btsm_queue_stop(btsm_queue_t queue)
{
    BDBG_MSG_TRACE(("btsm_queue_stop:%#lx", (unsigned)queue));
    BDBG_OBJECT_ASSERT(queue, btsm_queue);
    queue->started = false;
    return;
}

void btsm_queue_flush(btsm_queue_t queue)
{
    b_tsm_queue_entry *entry;

    BDBG_MSG_TRACE(("btsm_queue_flush:%#lx", (unsigned)queue));
    BDBG_OBJECT_ASSERT(queue, btsm_queue);
    while(NULL!=(entry=BLST_SQ_FIRST(&queue->queued))) {
        queue->status.dropped++;
        queue->status.queued--;
        BLST_SQ_REMOVE_HEAD(&queue->queued, link);
        BLST_SQ_INSERT_HEAD(&queue->free, entry, link);
    }
	BDBG_ASSERT(queue->status.queued==0);
    return;
}

void 
btsm_queue_get_status(btsm_queue_t queue, btsm_queue_status *status)
{
    BDBG_MSG_TRACE(("btsm_queue_get_status:%#lx", (unsigned)queue));
    BDBG_OBJECT_ASSERT(queue, btsm_queue);
    BDBG_ASSERT(status);

    *status = queue->status;
    return;
}

int 
btsm_queue_push(btsm_queue_t queue, const btsm_queue_entry *entry)
{
    b_tsm_queue_entry *queue_entry;
    BDBG_MSG_TRACE(("btsm_queue_push:%#lx %u", (unsigned)queue, entry->timestamp));
    BDBG_OBJECT_ASSERT(queue, btsm_queue);
    BDBG_ASSERT(entry);

    queue_entry = BLST_SQ_FIRST(&queue->free);
    if(queue_entry==NULL) {
        return -1;
    }
    queue_entry->entry = *entry;
    BLST_SQ_REMOVE_HEAD(&queue->free, link);
    BLST_SQ_INSERT_TAIL(&queue->queued, queue_entry, link);
    queue->status.queued++;
    return 0;
}

int 
btsm_queue_pop(btsm_queue_t queue, btsm_queue_entry *entry)
{
    b_tsm_queue_entry *queue_entry;
    BDBG_MSG_TRACE(("btsm_queue_pop:%#lx", (unsigned)queue));
    BDBG_OBJECT_ASSERT(queue, btsm_queue);
    BDBG_ASSERT(entry);
    queue_entry = BLST_SQ_FIRST(&queue->queued);
    if(queue_entry==NULL) {
        return -1;
    }
    queue_entry->entry = *entry;
    BLST_SQ_REMOVE_HEAD(&queue->queued, link);
    BLST_SQ_INSERT_HEAD(&queue->free, queue_entry, link);
    queue->status.queued--;
    queue->status.dropped++;
    return 0;
}

btsm_queue_action 
btsm_queue_get(btsm_queue_t queue, uint32_t timestamp, btsm_queue_entry *entry)
{
    b_tsm_queue_entry *queue_entry;
    btsm_queue_action action;
    int32_t diff;
	uint32_t half_frame = queue->settings.entry_duration;

    BDBG_MSG_TRACE(("btsm_queue_get:%#lx", (unsigned)queue));
    BDBG_OBJECT_ASSERT(queue, btsm_queue);
    BDBG_ASSERT(entry);
    queue_entry = BLST_SQ_FIRST(&queue->queued);
    if(queue_entry==NULL) {
		BDBG_ASSERT(queue->status.queued==0);
        action = btsm_queue_action_repeat;
		goto done;
    }
	BDBG_ASSERT(queue->status.queued>0);
    if(queue_entry->entry.timestamp_valid) {
        diff = queue_entry->entry.timestamp - timestamp;
        BDBG_MSG(("btsm_queue_get: %#lx TSM %u:%u -> %d", (unsigned long)queue, (unsigned)timestamp, (unsigned)queue_entry->entry.timestamp, (int)diff));
        if(diff<=(int)half_frame && diff>=-(int)half_frame) {
			/* in range */
			queue->prev_timestamp_valid = true;
			queue->prev_timestamp = timestamp;
			action = btsm_queue_action_display;
		} else if(diff>0) {
			/* sample is earlier */
            if(queue->settings.drop_threshold && diff>(int)queue->settings.drop_threshold) {
				/* sample is too early */
                action = btsm_queue_action_drop;
            } else {
                action = btsm_queue_action_repeat;
				goto done;
            }
        } else { /* diff < 0 */
			/* sample is late */
            if(queue->settings.drop_threshold && -diff>(int)queue->settings.drop_threshold) {
	            /* sample is too late */
                action = btsm_queue_action_drop;
			} else {
    			b_tsm_queue_entry *next_entry = BLST_SQ_NEXT(queue_entry, link);
				if(next_entry==NULL || (int32_t)(next_entry->entry.timestamp - timestamp) >0) {
					queue->prev_timestamp_valid = true;
					queue->prev_timestamp = timestamp;
					action = btsm_queue_action_display;
				} else {
                	action = btsm_queue_action_drop;
				}
            } 
        }
    } else {
        /* no timestamp, pace based on entry_duration */
        if(queue->prev_timestamp_valid) {
            diff = timestamp - queue->prev_timestamp;
            if(diff >= 0 && diff<(int)queue->settings.entry_duration) {
                action = btsm_queue_action_repeat;
				goto done;
            }
        }
        queue->prev_timestamp_valid = true;
        queue->prev_timestamp = timestamp;
        action = btsm_queue_action_display;
    }
    *entry = queue_entry->entry;
	queue->status.queued--;
    BLST_SQ_REMOVE_HEAD(&queue->queued, link);
    BLST_SQ_INSERT_HEAD(&queue->free, queue_entry, link);
done:
	switch(action) {
	case btsm_queue_action_repeat:
		queue->status.repeated ++;
		break;
	case btsm_queue_action_drop:
		queue->status.dropped ++;
		break;
	case btsm_queue_action_display:
		queue->status.displayed ++;
		break;
	}
    return action;
}


