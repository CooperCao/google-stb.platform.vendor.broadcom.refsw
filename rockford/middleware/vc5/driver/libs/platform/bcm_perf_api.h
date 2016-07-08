/*=============================================================================
Broadcom Proprietary and Confidential. (c)2013 Broadcom.
All rights reserved.

Project  :  khronos

FILE DESCRIPTION
Performance counter api
=============================================================================*/

#ifndef BCM_PERF_API_H_
#define BCM_PERF_API_H_

#include "bcm_perf_structs.h"

/* Somehow, somewhere, bool has been #defined by including stdbool.h.
 * I can't find out where though - and this works around it.
 */
#ifndef bool
#include <stdbool.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * PERFORMANCE COUNTERS
 *******************************************************************************/

/* Each group's scheduler may have a different set of performance counters that
 * they want to expose. We should not be limited to just the hardware performance
 * counters. The scheduler is likely to have other useful internal software
 * counters.
 *
 * In order to support this, the counters must be queryable.
 */

/* Returns the number of counter groups that are supported */
extern uint32_t bcm_sched_get_num_counter_groups(void);

/* Request the set of counters available in the given group.
 *
 * Valid group indices range from 0 to bcm_sched_get_num_counter_groups() - 1.
 * The groups structure is large, but typically this will only be called
 * once to enumerate the available counters.
 *
 * If the group given is invalid, or no counter groups are supported, false is
 * returned and group_desc is unchanged.
 */
extern bool bcm_sched_enumerate_group_counters(
   uint32_t                              group,
   struct bcm_sched_counter_group_desc  *group_desc);

/* Enables or disables a list of counter values for a particular group
 *
 * Multiple groups may be active at any time, in which case multiple calls
 * to this function will be required.
 *
 * If arguments are valid, returns true.
 * Returns false if the arguments were invalid. All counters will be disabled.
 */
extern bool bcm_sched_select_group_counters(
   const struct bcm_sched_group_counter_selector *selector);

/* Sets the state of the counter sampling.
 *
 * BCM_CTR_ACQUIRE : Requests exclusive counter access for the current process.
 *                   If already acquired by another process, will return false.
 * BCM_CTR_RELEASE : Hands back counter access lock.
 * BCM_CTR_START   : Starts sampling.
 * BCM_CTR_STOP    : Stops sampling.
 *
 * Returns false if counters are currently acquired by a different client.
 */
extern bool bcm_sched_set_counter_collection(
   enum bcm_sched_counter_state  state);

/* Retrieves counter values.
 *
 * Called with counters == NULL and max_counters == 0 will return the total
 * number of active counters that can be fetched.
 *
 * Called with counters != NULL, will fill at most max_counters counter
 * entries into counters. Returns the number of counters actually written.
 *
 * if reset_counts is true, the counters will be zeroed once copied.
 */
extern uint32_t bcm_sched_get_counters(
   struct bcm_sched_counter  *counters,
   uint32_t                   max_counters,
   bool                       reset_counts);


/*******************************************************************************
 * EVENT TIMELINE
 *******************************************************************************/

/* Event data could possibly be returned as part of job completion, but I chose
 * to avoid polluting the existing APIs. Doing so would mean the completion
 * structure would grow, and since event gathering is off most of the time I
 * thought it best to isolate its APIs.
 *
 * The events can be polled as often or as infrequently as required. If the
 * internal storage for the event stream becomes too large, behavior is
 * undefined. Data may be capped or overwritten for example.
 */

/* Returns the number of event tracks that are supported */
extern uint32_t bcm_sched_get_num_event_tracks(void);

/* Get information about an event track.
 *
 * If the track_index is invalid or an implementation doesn't support event capture,
 * this function will return false.
 */
extern bool bcm_sched_describe_event_track(
   uint32_t                            track_index,
   struct bcm_sched_event_track_desc   *track_desc);

/* Returns the number of events that are supported */
extern uint32_t bcm_sched_get_num_events(void);

/* Get information about an event type.
 *
 * If the event_index is invalid or an implementation doesn't support event capture,
 * this function will return false.
 */
extern bool bcm_sched_describe_event(
   uint32_t                      event_index,
   struct bcm_sched_event_desc   *event_desc);

/* Get information about one of an event's data fields.
 *
 * Events can have an arbitrary number of data fields.
 * Typical examples might include job id, session id etc.
 * This function enumerates those fields.
 *
 * If the event_index or field_index is invalid or an implementation
 * doesn't support event capture, this function will return false.
*/
extern bool bcm_sched_describe_event_data(
   uint32_t                            event_index,
   uint32_t                            field_index,
   struct bcm_sched_event_field_desc   *field_desc);

/* Sets the state of the event timeline collection.
 *
 * BCM_EVENT_ACQUIRE : Requests exclusive access to the samplers.
 * BCM_EVENT_RELEASE : Relinquishes exclusive access.
 * BCM_EVENT_START   : Starts sampling.
 * BCM_EVENT_STOP    : Stops sampling.
 *
 * Returns false if acquire failed due to another client currently having
 * exclusive access.
 */
extern bool bcm_sched_set_event_collection(
   enum bcm_sched_event_state    state);

/* Poll for event timeline data
 *
 * Called with event_buffer == NULL or event_buffer_bytes == 0 will return
 * the size (in bytes) of the event buffer needed to extract the logged events.
 *
 * Called with event_buffer != NULL will fill event_buffer with at most
 * event_buffer_bytes and will return the number of bytes actually copied.
 * Note that the last event in the buffer may be incomplete if the number of
 * bytes in the buffer does not match. The remaining bytes will be delivered
 * at the start of the next buffer collected. Once event gathering is stopped
 * all remaining data bytes can be fully retrieved.
 *
 * lost_data will be set to true if the event data filled the internal buffer
 * and was capped or overwritten. This indicates some events will have been lost.
 *
 * The current timestamp is returned in the timestamp_us argument. This allows
 * the caller to closely align the user-space time with the module's internal
 * time.
 *
 * The event_buffer will be filled with event data as follows.
 * For each event:
 * ----------------------------------------------------------------------------
 * |   64-bit  |  32-bit  |   32-bit    |       32-bit         |   variable   |
 * | timestamp |   id     | event index | bcm_sched_event_type | <field_data> |
 * ----------------------------------------------------------------------------
 *
 * The id is used to match begin events with end events - they will have the same id.
 *
 * The variable field data will consist of the set of data types that were enumerated
 * using bcm_sched_describe_event_data for the given event index.
 */
extern uint32_t bcm_sched_poll_event_timeline(
   size_t                   event_buffer_bytes,
   void                     *event_buffer,
   bool                     *lost_data,
   uint64_t                 *timestamp_us);

#ifdef __cplusplus
}
#endif

#endif /* BCM_PERF_API_H_ */
