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
#ifndef _BTSM_QUEUE_H__
#define _BTSM_QUEUE_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*================== Module Overview =====================================
This module is a helper moduie that provides implementation of time queue.
Elements could be pushed into the queue, popped from the queue, or could be 
extracted from the queue appplying timestamp manangement (TSM) algorigthm.

This modules uses 32bit time to communication timestamps, those timestamps are in the user defined units,
it could be either miliseconds, 45KHz or 90KHz units. Eventhough module doesn't need to know what units are, 
it's the users responsibility to use consistant units with a given instance of btsm_queue_t.

This module doesn't reorder entires, and output order matches input order. What it only does
it paces elements based on their timestamp and/or interpolated duration.

========================================================================*/


/**
Summary:
  btsm_queue_t is the object that encapsulates state of the module.
**/
typedef struct btsm_queue *btsm_queue_t;

/**
Summary:
  This structure is used to specify configuration of queue. 

Description:
  These settings provided during creation time and can't be changed
**/
typedef struct btsm_queue_config {
    unsigned length; /* size of the queue */
} btsm_queue_config;

/**
Summary:
  This structure is used to specify run-time settings of the queue.
**/
typedef struct btsm_queue_settings {
    uint32_t entry_duration; /* entry duration, it's used for timestamp interpolation */
    uint32_t drop_threshold; /* drop threshold, if data is outside of +- drop_threshold, then element would be dropped */
} btsm_queue_settings;

/**
Summary:
  This structure is defines basic element that is used to queue up and return user data.
**/
typedef struct btsm_queue_entry {
    bool timestamp_valid; /* true if timestamp is valid */
    uint32_t timestamp; /* user provided timestamp  */
    void *element; /* user provided element */
    uint8_t userdata[8]; /* user provided custom data */
} btsm_queue_entry;

/**
Summary:
  This structure is used to return status of the btsm_queue_t instance 
**/
typedef struct btsm_queue_status {
    unsigned queued; /* number of queued elements */
    unsigned dropped; /* count of dropped elements */
    unsigned displayed; /* count of displayed elements */
    unsigned repeated; /* count of repeated elements */
    uint32_t estimated_entry_duration; /* estimated durationf of entries */
} btsm_queue_status;

/**
Summary:
  This structure is used to return result of TSM equation
**/
typedef enum btsm_queue_action {
    btsm_queue_action_drop, /* element should be dropped */
    btsm_queue_action_display, /* element should be displayed */
    btsm_queue_action_repeat   /* repeat currently displayed entry */
} btsm_queue_action;

/**
Summary:
  This enum should be used to intialuze the btsm_queue_entry 
**/
#define BTSM_QUEUE_ENTRY_INIT(entry) do {(entry)->timestamp_valid=false;(entry)->timestamp = 0;(entry)->element=NULL;}while(0)

/**
Summary:
  This enum should be used to set timestamp
**/
#define BTSM_QUEUE_ENTRY_SET_TIMESTAMP(entry, ts) do {(entry)->timestamp_valid = true; (entry)->timestamp = ts;}while(0)

/**
Summary:
  This functions returns default configuration
**/
void btsm_queue_get_default_config(
        btsm_queue_config *config
        );

/**
Summary:
  This functions creates new instance of btsm_queue_t
**/
btsm_queue_t btsm_queue_create(
        const btsm_queue_config *config
        );

/**
Summary:
  This functions destroys instance of btsm_queue_t and releases all internally allocated resources 
**/
void btsm_queue_destroy(
        btsm_queue_t queue
        );

/**
Summary:
  This functions returns currect configuration
**/
void btsm_queue_get_settings(btsm_queue_t queue, btsm_queue_settings *settings);

/**
Summary:
  Prepares instance of btsm_queue_t to accumulate enrries.
**/
int btsm_queue_start(
        btsm_queue_t queue, 
        const btsm_queue_settings *settings /* pointer to new configuration, could be NULL if used doesn't want to change configuration */
        );

/**
Summary:
  Stops operation of btsm_queue_t.
**/
void btsm_queue_stop(
        btsm_queue_t queue
        );

/**
Summary:
  Removes all entries accumulated in the queue.
**/
void btsm_queue_flush(
        btsm_queue_t queue
        );

/**
Summary:
  Returns status of the queue.
**/
void btsm_queue_get_status(btsm_queue_t queue, btsm_queue_status *status);

/**
Summary:
  Adds entry into the bottom of the queue.
**/
int btsm_queue_push(btsm_queue_t queue, const btsm_queue_entry *entry);

/**
Summary:
  Removes and returns top element from the queue.
**/
int btsm_queue_pop(btsm_queue_t queue, btsm_queue_entry *entry);

/**
Summary:
    Performs TSM algorithm for the top element in the queue.
Description:
    This function runs TSM algorithm and returns element and advised action to the caller.
    When function returns btsm_queue_action_drop, it's advisable to call function again with the same timestamp, 
    since it's possible that there is another entry in the queue that should be dropped or displayed.
Returns:
    btsm_queue_action_repeat  - no new data avaliable, entry is left in undefined state 
    btsm_queue_action_display -  removes top element from the queue and returns it to application, it's advised to display  returned element
    btsm_queue_action_drop - removes top element from the queue and returns it to application, it's advised to drop  returned element. 
**/
btsm_queue_action btsm_queue_get(
        btsm_queue_t queue, 
        uint32_t timestamp, 
        btsm_queue_entry *entry
        );

#ifdef __cplusplus
}
#endif


#endif /* _BTSM_QUEUE_H__ */

