/***************************************************************************
 *     Copyright (c) 2002-2009, Broadcom Corporation
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
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/


#include "si.h"
#include "si_os.h"
#include "si_dbg.h"
#include "si_util.h"
#include "si_list.h"
#include "si_aeit.h"
#include "si_mgt.h"
#include "si_descriptors.h"


/* local function prototypes. */
static SI_AEIT_SOURCE *SI_AEIT_Create_Source (void);
static SI_AEIT_EVENT *SI_AEIT_Create_Event (void);
static SI_RET_CODE SI_AEIT_Ins_Source (SI_AEIT_SLOT *slot, SI_AEIT_SOURCE *new_source);
static SI_RET_CODE SI_AEIT_Ins_Event (SI_AEIT_SOURCE *source, SI_AEIT_EVENT *new_event);


extern struct aeit_slot_list head_aeit_slot, current_aeit0_slot;		/* always point to AEIT-0 */
extern SI_mutex m_aeit;


/*********************************************************************/
/* Function : SI_AEIT_Create_Slot							 		 */
/* Description : Function to allocate the space for an instance of   */
/*				 AEIT-n. Corresponding to events happening		 	 */
/*               in different channels(source) during a particular 3 */
/* 				 hour period of a day.			 					 */
/* Input : None.								         			 */
/* Output : pointer to the AEIT-n instance structure allocated. Will */
/*			return NULL if out of memory. 							 */
/*********************************************************************/
SI_AEIT_SLOT *SI_AEIT_Create_Slot (void)
{
	SI_AEIT_SLOT * slot;
	int	i;
	
	slot = (SI_AEIT_SLOT *)SI_alloc(sizeof(SI_AEIT_SLOT));
	if (slot == NULL)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("Failed to allocate an AEIT slot!!!\n"));
		return NULL;
	}

	SI_LST_D_INIT_ENTRY(&(slot->slot_link));
	SI_LST_D_INIT(&(slot->aeit_source));

	for (i=0; i<8; i++)
		slot->section_mask[i] = 0;

	slot->version_number = 0xff;
		
	return slot;
}

/*********************************************************************/
/* Function : SI_AEIT_Create_Source							 		 */
/* Description : Function to allocate the space for the event link 	 */
/*				 list of a paticular channel(source) within an 		 */
/*				 instance (slot) of AEIT-n.							 */
/* Input : None.								         			 */
/* Output : pointer to the channel (source) structure within an 	 */
/*			AEIT-n instance (slot). Will return NULL if out of 		 */
/*			memory. 												 */
/*********************************************************************/
static SI_AEIT_SOURCE *SI_AEIT_Create_Source (void)
{
	SI_AEIT_SOURCE * source;
	
	source = (SI_AEIT_SOURCE *)SI_alloc(sizeof(SI_AEIT_SOURCE));
	if (source == NULL)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("Failed to allocate an AEIT source!!!\n"));
		return NULL;
	}

	SI_LST_D_INIT_ENTRY(&(source->source_link));
	SI_LST_D_INIT(&(source->aeit_event));

	return source;
}

/*********************************************************************/
/* Function : SI_AEIT_Create_Event							 		 */
/* Description : Function to allocate the space for an actual event  */
/* 				 structure that forms the event link 	 			 */
/*				 list of a paticular channel(source) within an 		 */
/*				 instance (slot) of AEIT-n.							 */
/* Input : None.								         			 */
/* Output : pointer to the event structure for a channel (source) 	 */
/*			within an AEIT-n instance (slot). Will return NULL if 	 */
/*			out of memory.											 */
/*********************************************************************/
static SI_AEIT_EVENT *SI_AEIT_Create_Event (void)
{
	SI_AEIT_EVENT * event;
	
	event = (SI_AEIT_EVENT *)SI_alloc(sizeof(SI_AEIT_EVENT));
	if (event == NULL)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("Failed to allocate an AEIT event!!!\n"));
		return NULL;
	}

	event->title_text = NULL;
	SI_LST_D_INIT_ENTRY(&(event->event_link));

	return event;
}


/*********************************************************************/
/* Function : SI_AEIT_Clear_Slot							 		 */
/* Description : Function to clear the contents for an instance of   */
/*				 AEIT-n. This is used when a new version of AEIT-n 	 */
/*				 is received. It shall free all the structure		 */
/*				 allocated for the source and events. But not free 	 */
/*				 the slot itself. 									 */
/* Input : Slot structure pointer allocated for AEIT-n.				 */
/* Output : SI_RET_CODE												 */
/*********************************************************************/
SI_RET_CODE SI_AEIT_Clear_Slot (SI_AEIT_SLOT * slot)
{
	struct aeit_source_list  *source_list; /* head of source list. */
	struct aeit_event_list  *event_list; /* head of event list. */
	SI_AEIT_EVENT * event;
	SI_AEIT_SOURCE * source;
	unsigned long i;
	
	if (slot)
	{
		source_list = &(slot->aeit_source);
		/* recursively free all sources in slot. */
		while ((source = SI_LST_D_FIRST(source_list)))
		{
			/* recursively free all events in sources. */
			event_list = &(source->aeit_event);
			while ((event = SI_LST_D_FIRST(event_list)))
			{
				/* free the event title, event link, and event itself. */
				SI_free(event->title_text);
				SI_LST_D_REMOVE_HEAD(event_list, event_link); /* event_list will be updated. */
				SI_free(event);
			}

			/* free the source link and source */ 
			SI_LST_D_REMOVE_HEAD(source_list, source_link);  /* source_list will be updated. */
			SI_free(source);
		}

		/* just to be sure. Should not need to do it. */
		SI_LST_D_INIT(&(slot->aeit_source));

		for (i=0; i<8; i++)
			slot->section_mask[i] = 0;
		
		slot->version_number = 0xff;
		
	}
	
	return SI_SUCCESS;
}

/*********************************************************************/
/* Function : SI_AEIT_Ins_Source							 		 */
/* Description : Function to insert a newly received channel(source) */
/*				 structure into an instance (slot) of AEIT-n table.	 */
/*				 The channel(source) link list is sorted by 		 */
/*				 source_ID in incrementing order.					 */
/* Input : SI_AEIT_SLOT *slot : points to the existing AEIT-n(slot)  */
/*					instance structure.				       			 */
/*		   SI_AEIT_SOURCE *new_source : points to the newly received */
/*					channel event link list.						 */
/* Output : SI_RET_CODE.											 */
/*********************************************************************/
static SI_RET_CODE SI_AEIT_Ins_Source (SI_AEIT_SLOT *slot, SI_AEIT_SOURCE *new_source)
{
	struct aeit_source_list  *source_list; /* head of source list. */
	struct aeit_event_list  *event_list; /* head of event list. */
	SI_AEIT_SOURCE *source;
	SI_AEIT_EVENT *event;
	
	if (slot == NULL)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("No Valid slot allocated!!!\n"));
		return SI_NULL_POINTER;
	}

	if (new_source == NULL)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("source is not yet allocated!!!\n"));
		return SI_NULL_POINTER;
	}

	source_list = &(slot->aeit_source);
	source = SI_LST_D_FIRST(source_list);
	/* find the place to insert the new source link. */
	if (source == NULL)
	{
		/* if the source list is empty */
		SI_LST_D_INSERT_HEAD(source_list, new_source, source_link);
		return SI_SUCCESS;
	}
	
	while (source->source_ID <= new_source->source_ID)
	{
		if (new_source->source_ID == source->source_ID)
		{
			/* if the source already exists, I don't know how possible this is, since it means
				that the AEIT table will carry events of the same channel in different sections.
				Well, the spec does not say it is not possible. so we have to waste some code to 
				handle it.
			*/
			event_list = &(new_source->aeit_event);
			event = SI_LST_D_FIRST(event_list);
			while (event)
			{
				/* insert event into source structure. */
				SI_AEIT_Ins_Event(source, event);
				event = SI_LST_D_NEXT(event, event_link);
			}
			/* finally we need to free the source. */
			SI_free(new_source);
			return SI_SUCCESS;
		}

		if (SI_LST_D_NEXT(source, source_link))
			source = SI_LST_D_NEXT(source, source_link);
		else
		{
			/* we arrived at the end of list. */
			SI_LST_D_INSERT_AFTER(source, new_source, source_link);
			return SI_SUCCESS;
		}
	}

	/* when we get to this point, the source_list points to the point BEFORE which the new source 
		will be inserted.
	*/
	SI_LST_D_INSERT_BEFORE(source_list, source, new_source, source_link);	
	
	return SI_SUCCESS;
}


/*********************************************************************/
/* Function : SI_AEIT_Ins_Event								 		 */
/* Description : Function to insert a newly received event  		 */
/* 				 structure into the event link 	 					 */
/*				 list for a paticular channel(source) within an 	 */
/*				 instance (slot) of AEIT-n. The link list of events	 */
/* 				 is sorted by the starting time of an event.		 */
/* Input : SI_AEIT_SOURCE *source : points to the existing 			 */
/*					channel(source) event link list.       			 */
/*		   SI_AEIT_EVENT *new_event : points to the newly received 	 */
/*					event for a channel(source) to be inserted into	 */
/* 					the above source structure.						 */
/* Output : SI_RET_CODE.											 */
/*********************************************************************/
static SI_RET_CODE SI_AEIT_Ins_Event (SI_AEIT_SOURCE *source, SI_AEIT_EVENT *new_event)
{
	struct aeit_event_list  *event_list; /* head of event list. */
	SI_AEIT_EVENT *event;
	
	if (source == NULL)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("source is not yet allocated!!!\n"));
		return SI_NULL_POINTER;
	}

	if (new_event == NULL)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("new event is not yet allocated!!!\n"));
		return SI_NULL_POINTER;
	}

	event_list = &(source->aeit_event);
	if ( (event = SI_LST_D_FIRST(event_list)) == NULL)
	{
		/* if event list is original empty. */
		SI_LST_D_INSERT_HEAD(event_list, new_event, event_link);
		return SI_SUCCESS;
	}

	/* find the event that starts later than the new event. */
	while (event->start_time < new_event->start_time)
	{
		if (SI_LST_D_NEXT(event, event_link) == NULL)  /* end of list */
		{
			SI_LST_D_INSERT_AFTER(event, new_event, event_link);
			return SI_SUCCESS;
		}
		event = SI_LST_D_NEXT(event, event_link);
	}

	/* at this point, we just insert before the current event. */
	SI_LST_D_INSERT_BEFORE(event_list, event, new_event, event_link);

	return SI_SUCCESS;
}


/*********************************************************************
 Function : SI_AEIT_Parse	
 Description : Function to parse a newly received AEIT-n table and put
 				it into the AEIT-n slot link list created by MGT parse.
 				Before we create the AEIT-n slot from MGT, we simply 
 				ignore the AEIT-n table.
 Input : unsigned char *aeit_table : newly received aeit table data.						 
 Output : SI_RET_CODE.											 
**********************************************************************/
SI_RET_CODE SI_AEIT_Parse (unsigned char *aeit_table)
{
	unsigned long temp, i, j;
	unsigned long section_length, version_number;
	unsigned long MGT_tag, section_number, last_section_number, num_sources_in_section, num_events;
	unsigned long desc_start, len;
	unsigned long desc_tag, desc_len;
	unsigned char *current;
	SI_AEIT_SLOT *slot;
	SI_AEIT_SOURCE *source;
	SI_AEIT_EVENT *event;
	SI_RET_CODE result;

	temp = *(aeit_table);
	if (temp != SI_AEIT_TABLE_ID)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("AEIT Table ID error!!! %x\n", temp));
		return SI_TABLE_ID_ERROR;
	}

	/* calculate and check section length. */
	section_length = SI_Construct_Data(aeit_table, 
				AEIT_SECTION_LENGTH_BYTE_INDX,
				AEIT_SECTION_LENGTH_BYTE_NUM,
				AEIT_SECTION_LENGTH_SHIFT,
				AEIT_SECTION_LENGTH_MASK);
	section_length += AEIT_SECTION_LENGTH_BYTE_INDX+AEIT_SECTION_LENGTH_BYTE_NUM;
	if (section_length > SI_LONG_SECTION_LENGTH)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("AEIT Table section length error!!! %x\n", section_length));
		return SI_SECTION_LENGTH_ERROR;
	}
	
	/* We do the CRC check here to verify the contents of this section. */
	if (SI_CRC32_Check(aeit_table, section_length) != SI_SUCCESS)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("AEIT Table section CRC error!!!\n"));
		return SI_CRC_ERROR;
	}

	/* check to make sure AEIT_subtype is zero. */
	temp = SI_Construct_Data(aeit_table, 
				AEIT_AEIT_SUBTYPE_BYTE_INDX,
				AEIT_AEIT_SUBTYPE_BYTE_NUM,
				AEIT_AEIT_SUBTYPE_SHIFT,
				AEIT_AEIT_SUBTYPE_MASK);
	if (temp != SI_CURRENT_AEIT_SUBTYPE)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("AEIT Table AEIT_subtype Not what we can handle, Ignore the table!!! %x\n", temp));
		return SI_PROTOCOL_VER_ERROR;
	}
	
	/* get MGT_tag. */
	MGT_tag = SI_Construct_Data(aeit_table, 
				AEIT_MGT_TAG_BYTE_INDX,
				AEIT_MGT_TAG_BYTE_NUM,
				AEIT_MGT_TAG_SHIFT,
				AEIT_MGT_TAG_MASK);

	/* lock mutex for AEIT table access. */
	SI_mutex_lock(m_aeit);

	/* search for the AEIT-n slot link list for the MGT_tag. */
	if ((slot = SI_LST_D_FIRST(&current_aeit0_slot)) ==NULL)
	{
		SI_mutex_unlock(m_aeit);
		SI_DBG_PRINT(E_SI_ERR_MSG,("AEIT-n slot link list not generated from MGT yet, Ignore the table!!!\n"));
		return SI_AEIT_LIST_NOT_READY;
	}
	while (slot)
	{
		if (slot->MGT_tag == MGT_tag)
			break;
		slot = SI_LST_D_NEXT(slot, slot_link);
	}

	if (slot == NULL)
	{
		SI_mutex_unlock(m_aeit);
		SI_DBG_PRINT(E_SI_ERR_MSG,("AEIT-n slot link list does not contain the slot that matches the MGT_tag, Ignore the table!!\n"));
		return SI_AEIT_LIST_NOT_READY;
	}

	/* look at current_next_indicator. It should be 1 for AEIT. */
	temp = SI_Construct_Data(aeit_table, 
				AEIT_CURRENT_NEXT_INDICATOR_BYTE_INDX,
				AEIT_CURRENT_NEXT_INDICATOR_BYTE_NUM,
				AEIT_CURRENT_NEXT_INDICATOR_SHIFT,
				AEIT_CURRENT_NEXT_INDICATOR_MASK);
	if (temp != 1)
	{
		SI_mutex_unlock(m_aeit);
		SI_DBG_PRINT(E_SI_ERR_MSG,("AEIT Table current_next_indicator not one!!! %x\n", temp));
		return SI_CURRENT_NEXT_INDICATOR_ERROR;
	}

	/* now we know where the slot is in the link list, See if we need to update. */
	version_number = SI_Construct_Data(aeit_table, 
						AEIT_VERSION_NUMBER_BYTE_INDX,
						AEIT_VERSION_NUMBER_BYTE_NUM,
						AEIT_VERSION_NUMBER_SHIFT,
						AEIT_VERSION_NUMBER_MASK);
	section_number = SI_Construct_Data(aeit_table, 
						AEIT_SECTION_NUMBER_BYTE_INDX,
						AEIT_SECTION_NUMBER_BYTE_NUM,
						AEIT_SECTION_NUMBER_SHIFT,
						AEIT_SECTION_NUMBER_MASK);
	if (slot->version_number == version_number)
	{
		/* the same version number. Now check if the section number has already be processed. */
		if (SI_Chk_Section_mask(&(slot->section_mask[0]), section_number))
		{
			/* section already processed, we are done! */
			SI_mutex_unlock(m_aeit);
			return SI_SUCCESS;
		}
	}
	else
	{
		/* different version number. The slot should have already been init by MGT parse. */
		SI_DBG_PRINT(E_SI_DBG_MSG,("New AEIT Table received!\n"));
		if (slot->MGT_version_number != version_number)
		{
			/* this should not happen, but what the heck..... */
			SI_DBG_PRINT(E_SI_WRN_MSG,("AEIT-n slot mgt version does not match new version, reinit slot!!\n"));
			temp = slot->pid; /* at least trust the pid. */
			SI_AEIT_Clear_Slot(slot);
			slot->MGT_tag = MGT_tag;
			slot->MGT_version_number = version_number;
			slot->pid = temp;
		}
		slot->version_number = version_number;
		/* init section mask. */
		last_section_number = SI_Construct_Data(aeit_table, 
							AEIT_LAST_SECTION_NUMBER_BYTE_INDX,
							AEIT_LAST_SECTION_NUMBER_BYTE_NUM,
							AEIT_LAST_SECTION_NUMBER_SHIFT,
							AEIT_LAST_SECTION_NUMBER_MASK);
		SI_Init_Section_Mask(&(slot->section_mask[0]), last_section_number);
	}

	/* update section mask here. */
	SI_Set_Section_mask(&(slot->section_mask[0]), section_number);
	
	/* new AEIT table section received!!! */
	SI_DBG_PRINT(E_SI_DBG_MSG,("New AEIT Table section received!\n"));

	num_sources_in_section = SI_Construct_Data(aeit_table, 
							AEIT_NUM_SOURCES_IN_SECTION_BYTE_INDX,
							AEIT_NUM_SOURCES_IN_SECTION_BYTE_NUM,
							AEIT_NUM_SOURCES_IN_SECTION_SHIFT,
							AEIT_NUM_SOURCES_IN_SECTION_MASK);

	current = aeit_table + AEIT_NUM_SOURCES_IN_SECTION_BYTE_INDX + 
							AEIT_NUM_SOURCES_IN_SECTION_BYTE_NUM; /* points to first source byte. */
	for (i=0; i<num_sources_in_section; i++)
	{
		if ((source = SI_AEIT_Create_Source()) == NULL)
		{
			SI_mutex_unlock(m_aeit);
			SI_DBG_PRINT(E_SI_ERR_MSG,("AEIT cannot create source structure!!!\n"));
			return SI_NO_MEMORY;
		}
		source->source_ID = SI_Construct_Data(current, 
							AEIT_SOURCE_ID_BYTE_INDX,
							AEIT_SOURCE_ID_BYTE_NUM,
							AEIT_SOURCE_ID_SHIFT,
							AEIT_SOURCE_ID_MASK);
		num_events = SI_Construct_Data(current, 
						AEIT_NUM_EVENTS_BYTE_INDX,
						AEIT_NUM_EVENTS_BYTE_NUM,
						AEIT_NUM_EVENTS_SHIFT,
						AEIT_NUM_EVENTS_MASK);
		current += AEIT_NUM_EVENTS_BYTE_INDX+AEIT_NUM_EVENTS_BYTE_NUM; /* points to 1st event byte. */
		for (j=0; j<num_events; j++)
		{
			/* create and stuff the event structure. */
			if ((event = SI_AEIT_Create_Event()) == NULL)
			{
				SI_mutex_unlock(m_aeit);
				SI_DBG_PRINT(E_SI_ERR_MSG,("AEIT cannot create event structure!!!\n"));
				return SI_NO_MEMORY;
			}
			event->event_ID = SI_Construct_Data(current, 
							AEIT_EVENT_ID_BYTE_INDX,
							AEIT_EVENT_ID_BYTE_NUM,
							AEIT_EVENT_ID_SHIFT,
							AEIT_EVENT_ID_MASK);
			event->start_time = SI_Construct_Data(current, 
							AEIT_START_TIME_BYTE_INDX,
							AEIT_START_TIME_BYTE_NUM,
							AEIT_START_TIME_SHIFT,
							AEIT_START_TIME_MASK);
			event->ETM_present = SI_Construct_Data(current, 
							AEIT_ETM_PRESENT_BYTE_INDX,
							AEIT_ETM_PRESENT_BYTE_NUM,
							AEIT_ETM_PRESENT_SHIFT,
							AEIT_ETM_PRESENT_MASK);
			event->duration = SI_Construct_Data(current, 
							AEIT_DURATION_BYTE_INDX,
							AEIT_DURATION_BYTE_NUM,
							AEIT_DURATION_SHIFT,
							AEIT_DURATION_MASK);
			event->title_length = SI_Construct_Data(current, 
							AEIT_TITLE_LENGTH_BYTE_INDX,
							AEIT_TITLE_LENGTH_BYTE_NUM,
							AEIT_TITLE_LENGTH_SHIFT,
							AEIT_TITLE_LENGTH_MASK);
			if ((event->title_text = (unsigned char *)SI_alloc(event->title_length)) == NULL)
			{
				SI_mutex_unlock(m_aeit);
				SI_DBG_PRINT(E_SI_ERR_MSG,("AEIT cannot allocate mem for event title!!!\n"));
				return SI_NO_MEMORY;
			}
			current += AEIT_TITLE_LENGTH_BYTE_INDX+AEIT_TITLE_LENGTH_BYTE_NUM; /* points to title text. */
			SI_memcpy(event->title_text, current, event->title_length);
			current += event->title_length; /* points to desc length. */

			desc_len = SI_Construct_Data(current, 
							AEIT_DESC_LENGTH_BYTE_INDX,
							AEIT_DESC_LENGTH_BYTE_NUM,
							AEIT_DESC_LENGTH_SHIFT,
							AEIT_DESC_LENGTH_MASK);
							
			/* TBD Not processing descriptors yet. */
			current += AEIT_DESC_LENGTH_BYTE_INDX+AEIT_DESC_LENGTH_BYTE_NUM; /* point to the 1st desc. */
			desc_start = 0;
			while (desc_start < desc_len)
			{
				desc_tag = *(current++);
				len = *(current++);
				switch (desc_tag)
				{
					case SI_DESC_AC3_AUDIO:
						SI_DBG_PRINT(E_SI_DBG_MSG,("AEIT Table have AC3 descriptor.\n"));
					break;

					case SI_DESC_CAPTION_SERVICE:
						SI_DBG_PRINT(E_SI_DBG_MSG,("AEIT Table have caption descriptor.\n"));
					break;

					case SI_DESC_CONTENT_ADVISORY:
						SI_DBG_PRINT(E_SI_DBG_MSG,("AEIT Table have content advisory descriptor.\n"));
					break;

					case SI_DESC_STUFFING:
						SI_DBG_PRINT(E_SI_DBG_MSG,("AEIT Table have stuffing descriptor.\n"));
					break;

					default:
						if (desc_tag >= SI_DESC_USER_PRIVATE)
						{
							SI_DBG_PRINT(E_SI_WRN_MSG,("AEIT Table have user private descriptor. And we don't know what to do with it.\n"));
						}
						else
						{
							SI_mutex_unlock(m_aeit);
							SI_DBG_PRINT(E_SI_ERR_MSG,("AEIT Table have bad descriptor %x!\n", desc_tag));
							return SI_DESCRIPTOR_ERROR;
						}
					break;
				}
				current += len;

				desc_start += len+2; /* plus the tag and length. */
			}

			/* verify descriptor length. */
			if (desc_start != desc_len)
			{
				SI_mutex_unlock(m_aeit);
				SI_DBG_PRINT(E_SI_ERR_MSG,("AEIT Table descriptor length error!\n"));
				return SI_DESCRIPTOR_ERROR;
			}

			/* insert event into event link list for this channel */
			if ((result = SI_AEIT_Ins_Event (source, event)) != SI_SUCCESS)
			{
				SI_mutex_unlock(m_aeit);
				SI_DBG_PRINT(E_SI_ERR_MSG,("AEIT cannot insert event!!!\n"));
				return result;
			}
		}

		/* insert the source structure into AEIT-n source link list. */
		if ((result = SI_AEIT_Ins_Source(slot,source)) != SI_SUCCESS)
		{
			SI_mutex_unlock(m_aeit);
			SI_DBG_PRINT(E_SI_ERR_MSG,("AEIT cannot insert source!!!\n"));
			return result;
		}
	}

	/* unlock mutex for AEIT table access. */
	SI_mutex_unlock(m_aeit);
	
	if (((unsigned long)current - (unsigned long)aeit_table) != (section_length - SI_CRC_LENGTH))
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("AEIT Table length error!\n"));
		return SI_SECTION_LENGTH_ERROR;
	}
	
	return SI_SUCCESS;
}

