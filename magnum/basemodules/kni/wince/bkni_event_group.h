/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
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
 * Implementatation of the Magnum KNI for user space Linux applications.
 *
 * Revision History:
 *
 * $brcm_Log: $ *
 * 
 ***************************************************************************/
#ifndef BKNI_EVENT_GROUP_H_
#define BKNI_EVENT_GROUP_H_

#include "bkni_multi.h"

/*=*************************************************************************
Summary:
The event group implements event multiplexing. An application
can group multiple events and then wait in the single thread any of these
events to be set.

The BKNI_EventGroup kernel interface is an optional component of the kernel interface.
It shall not be used by the porting interface, base modules or common utilites.
It is only for application needs.

Example:
  #include "bkni_event_group.h"

  ...
  BKNI_EventHandle event1, event2;
  BKNI_EventHandle events[2];
  unsigned nevents;

  BKNI_EventGroupHandle group;

  BKNI_CreateEvent(&event1);
  BKNI_CreateEvent(&event2);

  BKNI_CreateEventGroup(&group);

  BKNI_AddEventGroup(group, event1);
  BKNI_AddEventGroup(group, event2);

  BKNI_WaitForGroup(group, 1000, events, 2, &nevents);

  printf("Got %d events", nevents);


****************************************************************************/


/**************************************************************************
Summary:
 Event group handle
**************************************************************************/
typedef struct BKNI_GroupObj *BKNI_EventGroupHandle;
#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************
Summary:
 This function is used to create a new event group

Description:
 This function create a new evetn group

Returns:
	BERR_SUCCESS - Event groupt was created successfully.
	BERR_OS_ERROR - The system failed and group wasn't created
**************************************************************************/
BERR_Code BKNI_CreateEventGroup(
		BKNI_EventGroupHandle *pGroup
		);

/**************************************************************************
Summary:
 This function is used to destroy an event group

Description:
 This function destroy a new evetn group.
Note: application shall not be inside BKNI_WaitForGroup while calling this function.

Returns:
  N/A
**************************************************************************/
void BKNI_DestroyEventGroup(
		BKNI_EventGroupHandle group
		);

/**************************************************************************
Summary:
 This function is used to add an event into the event group

Description:
 This function adds an event into the event group. If event is already
 set off, active BKNI_WaitForGroup would be woken up immediatly.

Note: Event could belong only to the one group at the time.

Returns:
	BERR_SUCCESS - Event was added into the group successfully
	BERR_OS_ERROR - The system failed and event wasn't added
**************************************************************************/
BERR_Code
BKNI_AddEventGroup(
		BKNI_EventGroupHandle group,  /* Event group */
		BKNI_EventHandle event /* Event to add into the group */
		);

/**************************************************************************
Summary:
 This function is used to remove an event from the event group

Description:
 This function removes to event from the event group

Returns:
	BERR_SUCCESS - Event was removed from the group successfully
	BERR_OS_ERROR - The system failed and the group state is unspecified
**************************************************************************/
BERR_Code
BKNI_RemoveEventGroup(
		BKNI_EventGroupHandle group,  /* Event group */
		BKNI_EventHandle event /* Event to remove from the group */
		);

/**************************************************************************
Summary:
 This function is used to wait for the any event from the group.

Description:
 This function waits for the any event from the group. This function
 could exit either because of timeout, or when some event[s] are set.
 In the latter case it would save the number of events and events themselves in
 the events structure.
 After the event is saved in the user specified area, the event is cleared out.

Returns:
	BERR_SUCCESS - Event was removed from the group successfully
	BERR_OS_ERROR - The system failed and the group state is unspecified
**************************************************************************/
BERR_Code
BKNI_WaitForGroup(
		BKNI_EventGroupHandle group, /* event group */
		int timeoutMsec,  /* timeout in milliseconds, use BKNI_INFINITE to wait without timeout */
		BKNI_EventHandle *events, /* [out] pass out the events that were triggered
							(specified by *nevents) */
		unsigned max_events,  /* the maximum number of events which can be passed out using
							the events parameter. Generally this should be equal to the
							number of events in the BKNI_EventGroup. */
		unsigned *nevents /* [out] number of events which were stored in the user specified area */
		);

#ifdef __cplusplus
}
#endif
#endif /* BKNI_EVENT_GROUP_H_ */


