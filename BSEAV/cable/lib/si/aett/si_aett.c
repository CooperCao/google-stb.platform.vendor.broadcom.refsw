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
#include "si_aett.h"
#include "si_mgt.h"
#include "si_descriptors.h"

/* local function prototypes. */
static SI_AETT_SOURCE *SI_AETT_Create_Source (void);
static SI_AETT_TEXT *SI_AETT_Create_Text (void);
static SI_RET_CODE SI_AETT_Ins_Source_Text (SI_AETT_SLOT *slot, unsigned short srcid, SI_AETT_TEXT *new_text);


extern struct aett_slot_list head_aett_slot, current_aett0_slot;		/* always point to AETT-0 */
extern SI_mutex m_aett;



/*********************************************************************/
/* Function : SI_AETT_Create_Slot							 		 */
/* Description : Function to allocate the space for an instance of   */
/*				 AETT-n. Corresponding to text for events happening	 */
/*               in different channels(source) during a particular 3 */
/* 				 hour period of a day.			 					 */
/* Input : None.								         			 */
/* Output : pointer to the AETT-n instance structure allocated. Will */
/*			return NULL if out of memory. 							 */
/*********************************************************************/
SI_AETT_SLOT *SI_AETT_Create_Slot (void)
{
	SI_AETT_SLOT * slot;
	int	i;
	
	slot = (SI_AETT_SLOT *)SI_alloc(sizeof(SI_AETT_SLOT));
	if (slot == NULL)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("Failed to allocate an AETT slot!!!\n"));
		return NULL;
	}

	SI_LST_D_INIT_ENTRY(&(slot->slot_link));
	SI_LST_D_INIT(&(slot->aett_source));

	for (i=0; i<8; i++)
		slot->section_mask[i] = 0;

	slot->version_number = 0xff;
		
	return slot;
}

/*********************************************************************/
/* Function : SI_AETT_Create_Source							 		 */
/* Description : Function to allocate the space for the event text 	 */
/*				 link list of a paticular channel(source) within an  */
/*				 instance (slot) of AETT-n.							 */
/* Input : None.								         			 */
/* Output : pointer to the channel (source) structure within an 	 */
/*			AETT-n instance (slot). Will return NULL if out of 		 */
/*			memory. 												 */
/*********************************************************************/
static SI_AETT_SOURCE *SI_AETT_Create_Source (void)
{
	SI_AETT_SOURCE * source;
	
	source = (SI_AETT_SOURCE *)SI_alloc(sizeof(SI_AETT_SOURCE));
	if (source == NULL)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("Failed to allocate an AETT source!!!\n"));
		return NULL;
	}

	SI_LST_D_INIT_ENTRY(&(source->source_link));
	SI_LST_D_INIT(&(source->aett_text));

	return source;
}

/*********************************************************************/
/* Function : SI_AETT_Create_Text							 		 */
/* Description : Function to allocate the space for an actual event  */
/* 				 text structure that forms the event text link 		 */
/*				 list of a paticular channel(source) within an 		 */
/*				 instance (slot) of AETT-n.							 */
/* Input : None.								         			 */
/* Output : pointer to the event text structure for a channel 		 */
/*			(source) within an AETT-n instance (slot). Will return 	 */
/*			NULL if out of memory.									 */
/*********************************************************************/
static SI_AETT_TEXT *SI_AETT_Create_Text (void)
{
	SI_AETT_TEXT * text;
	
	text = (SI_AETT_TEXT *)SI_alloc(sizeof(SI_AETT_TEXT));
	if (text == NULL)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("Failed to allocate an AETT event text!!!\n"));
		return NULL;
	}

	text->extended_text_message = NULL;
	SI_LST_D_INIT_ENTRY(&(text->text_link));

	return text;
}


/*********************************************************************/
/* Function : SI_AETT_Clear_Slot							 		 */
/* Description : Function to claer the contents for an instance of   */
/*				 AETT-n. This is used when a new version of AETT-n 	 */
/*				 is received. It shall free all the structure		 */
/*				 allocated for the source and events. But not free	 */
/*				 the slot itself									 */
/* Input : Slot structure pointer allocated for AETT-n.				 */
/* Output : SI_RET_CODE												 */
/*********************************************************************/
SI_RET_CODE SI_AETT_Clear_Slot (SI_AETT_SLOT * slot)
{
	struct aett_source_list  *source_list; /* head of source list. */
	struct aett_text_list  *text_list; /* head of text list. */
	SI_AETT_SOURCE *source;
	SI_AETT_TEXT *text;
	unsigned long i;
	
	if (slot)
	{
		/* recursively free all sources in slot. */
		source_list = &(slot->aett_source);
		while ((source = SI_LST_D_FIRST(source_list)))
		{
			/* recursively free all texts in sources. */
			text_list = &(source->aett_text);
			while ((text = SI_LST_D_FIRST(text_list)))
			{
				SI_free(text->extended_text_message);
				SI_LST_D_REMOVE_HEAD(text_list, text_link); /* text_list will be updated. */
				SI_free(text);
			}
			SI_LST_D_REMOVE_HEAD(source_list, source_link);  /* source_list will be updated. */
			SI_free(source);
		}

		/* just to be sure. Should not need to do it. */
		SI_LST_D_INIT(&(slot->aett_source));

		for (i=0; i<8; i++)
			slot->section_mask[i] = 0;

		slot->version_number = 0xff;
	}
	
	return SI_SUCCESS;
}


/*********************************************************************/
/* Function : SI_AETT_Ins_Source_Text								 */
/* Description : Function to insert a newly received event text		 */
/* 				 structure for a channel(source) into the text link  */
/*				 list for a paticular channel(source) within an 	 */
/*				 instance (slot) of AETT-n. The link list of text	 */
/* 				 is sorted by the event ID of an event in 			 */
/*				 incremental order.									 */
/* Input : SI_AETT_SLOT *slot : points to the existing AETT-n 		 */
/*					instance for a particular time slot.   			 */
/*		   unsigned short srcid : source_ID for the text			 */
/*		   SI_AETT_TEXT *new_text : points to the newly received 	 */
/*					text structure for a channel(source) to be 		 */
/* 					inserted into the source corresponding to the 	 */
/*					srcid of the above AETT-n slot.					 */
/* Output : SI_RET_CODE.											 */
/*********************************************************************/
static SI_RET_CODE SI_AETT_Ins_Source_Text (SI_AETT_SLOT *slot, unsigned short srcid, SI_AETT_TEXT *new_text)
{
	struct aett_source_list  *source_list; /* head of source list. */
	struct aett_text_list  *text_list; /* head of text list. */
	SI_AETT_SOURCE *source, *new_source;
	SI_AETT_TEXT *text;
	
	if (slot == NULL)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("AETT slot is not yet allocated!!!\n"));
		return SI_NULL_POINTER;
	}

	if (new_text == NULL)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("new text is not yet allocated!!!\n"));
		return SI_NULL_POINTER;
	}

	/* first we create a new source structure for the new text. We may need to free it later. */
	new_source = SI_AETT_Create_Source();
	if (new_source == NULL)
		return SI_NO_MEMORY;

	/* insert the text into new source. */
	new_source->source_ID = srcid;
	SI_LST_D_INSERT_HEAD(&(new_source->aett_text), new_text, text_link);

	/* if we don't have a channel allocated yet, simply create it. and copy the text.*/
	source_list = &(slot->aett_source);
	if ( (source = SI_LST_D_FIRST(source_list)) == NULL)
	{
		/* simply insert the new source structure to the head of list. */
		SI_LST_D_INSERT_HEAD(source_list, new_source, source_link);
		return SI_SUCCESS;
	}

	/* try to see where we going to insert the source of if the source is there already. */
	while (source->source_ID < srcid)
	{
		if (SI_LST_D_NEXT(source, source_link) == NULL)   /* end of list. */
		{
			/* last source in AETT-n. create and insert at the end. */
			SI_LST_D_INSERT_AFTER(source, new_source, source_link);
			return SI_SUCCESS;
		}

		source = SI_LST_D_NEXT(source, source_link);
	}

	/* check to see if the same source has already been created. */
	if (source->source_ID == srcid)
	{
		/* we don't need this new source anymore. */
		SI_free(new_source);

		/* the channel(source) is already allocated. just insert the text. */
		text_list = &(source->aett_text);	
		if ( (text = SI_LST_D_FIRST(text_list)) == NULL)
		{
			/* this condition SHOULD NOT HAPPEN! */
			SI_DBG_PRINT(E_SI_WRN_MSG,("AETT text is missing from a channel!!!\n"));
			/* we just put the new text in. */
			SI_LST_D_INSERT_HEAD(text_list, new_text, text_link);
			return SI_SUCCESS;
		}

		/* recurse through the text list to find the insertion point. */
		while (text->event_ID < new_text->event_ID)
		{
			/* see if we reach the end of list. */
			if (SI_LST_D_NEXT(text, text_link) == NULL)
			{
				/* insert new text at the end of list. */
				SI_LST_D_INSERT_AFTER(text, new_text, text_link);
				return SI_SUCCESS;
			}
			text = SI_LST_D_NEXT(text, text_link);
		}

		/* at this point, we just insert before the current text. */
		SI_LST_D_INSERT_BEFORE(text_list, text, new_text, text_link);

		return SI_SUCCESS;
	}
	
	/* if source is not already allocated, Create the source, insert the text and 
	   insert the source into the source list BEFORE the current source for the slot. */
	SI_LST_D_INSERT_BEFORE(source_list, source, new_source, source_link);
	
	return SI_SUCCESS;
}

/*********************************************************************
 Function : SI_AETT_Parse	
 Description : Function to parse a newly received AETT-n table and put
 				it into the AETT-n slot link list created by MGT parse.
 				Before we create the AETT-n slot from MGT, we simply 
 				ignore the AETT-n table.
 Input : unsigned char *aett_table : newly received aett table data.						 
 Output : SI_RET_CODE.											 
**********************************************************************/
SI_RET_CODE SI_AETT_Parse (unsigned char *aett_table)
{
	unsigned long temp, i, j;
	unsigned long section_length, ETM_ID, source_ID, version_number;
	unsigned long MGT_tag, section_number, last_section_number, num_blocks_in_section, num_events;
	unsigned long desc_start;
	unsigned long desc_tag, desc_len;
	unsigned char *current, desc;
	SI_AETT_SLOT *slot;
	SI_AETT_TEXT *text;
	SI_RET_CODE result;

	temp = *aett_table;
	if (temp != SI_AETT_TABLE_ID)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("AETT Table ID error!!! %x\n", temp));
		return SI_TABLE_ID_ERROR;
	}

	/* calculate and check section length. */
	section_length = SI_Construct_Data(aett_table, 
				AETT_SECTION_LENGTH_BYTE_INDX,
				AETT_SECTION_LENGTH_BYTE_NUM,
				AETT_SECTION_LENGTH_SHIFT,
				AETT_SECTION_LENGTH_MASK);
	section_length += AETT_SECTION_LENGTH_BYTE_INDX+AETT_SECTION_LENGTH_BYTE_NUM;
	if (section_length > SI_LONG_SECTION_LENGTH)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("AETT Table section length error!!! %x\n", section_length));
		return SI_SECTION_LENGTH_ERROR;
	}
	
	/* We do the CRC check here to verify the contents of this section. */
	if (SI_CRC32_Check(aett_table, section_length) != SI_SUCCESS)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("AETT Table section CRC error!!!\n"));
		return SI_CRC_ERROR;
	}

	/* check to make sure AETT_subtype is zero. */
	temp = SI_Construct_Data(aett_table, 
				AETT_AETT_SUBTYPE_BYTE_INDX,
				AETT_AETT_SUBTYPE_BYTE_NUM,
				AETT_AETT_SUBTYPE_SHIFT,
				AETT_AETT_SUBTYPE_MASK);
	if (temp != SI_CURRENT_AETT_SUBTYPE)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("AETT Table AETT_subtype Not what we can handle, Ignore the table!!! %x\n", temp));
		return SI_PROTOCOL_VER_ERROR;
	}
	
	/* get MGT_tag. */
	MGT_tag = SI_Construct_Data(aett_table, 
				AETT_MGT_TAG_BYTE_INDX,
				AETT_MGT_TAG_BYTE_NUM,
				AETT_MGT_TAG_SHIFT,
				AETT_MGT_TAG_MASK);

	/* lock mutex for AETT table access. */
	SI_mutex_lock(m_aett);

	/* search for the AETT-n slot link list for the MGT_tag. */
	if ((slot = SI_LST_D_FIRST(&current_aett0_slot)) ==NULL)
	{
		SI_mutex_unlock(m_aett);
		SI_DBG_PRINT(E_SI_ERR_MSG,("AETT-n slot link list not generated from MGT yet, Ignore the table!!!\n"));
		return SI_AETT_LIST_NOT_READY;
	}
	while (slot)
	{
		if (slot->MGT_tag == MGT_tag)
			break;
		slot = SI_LST_D_NEXT(slot, slot_link);
	}

	if (slot == NULL)
	{
		SI_mutex_unlock(m_aett);
		SI_DBG_PRINT(E_SI_ERR_MSG,("AETT-n slot link list does not contain the slot that matches the MGT_tag, Ignore the table!!\n"));
		return SI_AETT_LIST_NOT_READY;
	}

	/* look at current_next_indicator. It should be 1 for AETT. */
	temp = SI_Construct_Data(aett_table, 
				AETT_CURRENT_NEXT_INDICATOR_BYTE_INDX,
				AETT_CURRENT_NEXT_INDICATOR_BYTE_NUM,
				AETT_CURRENT_NEXT_INDICATOR_SHIFT,
				AETT_CURRENT_NEXT_INDICATOR_MASK);
	if (temp != 1)
	{
		SI_mutex_unlock(m_aett);
		SI_DBG_PRINT(E_SI_ERR_MSG,("AETT Table current_next_indicator not one!!! %x\n", temp));
		return SI_CURRENT_NEXT_INDICATOR_ERROR;
	}

	/* now we know where the slot is in the link list, See if we need to update. */
	version_number = SI_Construct_Data(aett_table, 
						AETT_VERSION_NUMBER_BYTE_INDX,
						AETT_VERSION_NUMBER_BYTE_NUM,
						AETT_VERSION_NUMBER_SHIFT,
						AETT_VERSION_NUMBER_MASK);
	section_number = SI_Construct_Data(aett_table, 
						AETT_SECTION_NUMBER_BYTE_INDX,
						AETT_SECTION_NUMBER_BYTE_NUM,
						AETT_SECTION_NUMBER_SHIFT,
						AETT_SECTION_NUMBER_MASK);
	if (slot->version_number == version_number)
	{
		/* the same version number. Now check if the section number has already be processed. */
		if (SI_Chk_Section_mask(&(slot->section_mask[0]), section_number))
		{
			/* section already processed, we are done! */
			SI_mutex_unlock(m_aett);
			return SI_SUCCESS;
		}
	}
	else
	{
		/* different version number. The slot should have already been init by MGT parse. */
		SI_DBG_PRINT(E_SI_DBG_MSG,("New AETT Table received!\n"));
		if (slot->MGT_version_number != version_number)
		{
			/* this should not happen, but what the heck..... */
			SI_DBG_PRINT(E_SI_WRN_MSG,("AETT-n slot mgt version does not match new version, reinit slot!!\n"));
			temp = slot->pid; /* at least trust the pid. */
			SI_AETT_Clear_Slot(slot);
			slot->MGT_tag = MGT_tag;
			slot->MGT_version_number = version_number;
			slot->pid = temp;
		}
		slot->version_number = version_number;
		/* init section mask. */
		last_section_number = SI_Construct_Data(aett_table, 
							AETT_LAST_SECTION_NUMBER_BYTE_INDX,
							AETT_LAST_SECTION_NUMBER_BYTE_NUM,
							AETT_LAST_SECTION_NUMBER_SHIFT,
							AETT_LAST_SECTION_NUMBER_MASK);
		SI_Init_Section_Mask(&(slot->section_mask[0]), last_section_number);
	}

	/* update section mask here. */
	SI_Set_Section_mask(&(slot->section_mask[0]), section_number);
	
	/* new AETT table section received!!! */
	SI_DBG_PRINT(E_SI_DBG_MSG,("New AETT Table section received!\n"));

	num_blocks_in_section = SI_Construct_Data(aett_table, 
							AETT_NUM_BLOCKS_IN_SECTION_BYTE_INDX,
							AETT_NUM_BLOCKS_IN_SECTION_BYTE_NUM,
							AETT_NUM_BLOCKS_IN_SECTION_SHIFT,
							AETT_NUM_BLOCKS_IN_SECTION_MASK);

	current = aett_table + AETT_NUM_BLOCKS_IN_SECTION_BYTE_INDX + 
							AETT_NUM_BLOCKS_IN_SECTION_BYTE_NUM; /* points to first block byte. */

	for (i=0; i<num_blocks_in_section; i++)
	{
		/* create and stuff the text structure. */
		if ((text = SI_AETT_Create_Text()) == NULL)
		{
			SI_mutex_unlock(m_aett);
			SI_DBG_PRINT(E_SI_ERR_MSG,("AETT cannot create text structure!!!\n"));
			return SI_NO_MEMORY;
		}
		ETM_ID = SI_Construct_Data(current, 
								AETT_ETM_ID_BYTE_INDX,
								AETT_ETM_ID_BYTE_NUM,
								AETT_ETM_ID_SHIFT,
								AETT_ETM_ID_MASK);
		source_ID = ((ETM_ID>>16)&0xffff);
		text->event_ID = (ETM_ID&0xffff);
		text->extended_text_length = SI_Construct_Data(current, 
								AETT_ETT_LENGTH_BYTE_INDX,
								AETT_ETT_LENGTH_BYTE_NUM,
								AETT_ETT_LENGTH_SHIFT,
								AETT_ETT_LENGTH_MASK);
		current += AETT_ETT_LENGTH_BYTE_INDX+AETT_ETT_LENGTH_BYTE_NUM; /* points to text. */
		if ((text->extended_text_message = (unsigned char *)SI_alloc(text->extended_text_length)) == NULL)
		{
			SI_mutex_unlock(m_aett);
			SI_DBG_PRINT(E_SI_ERR_MSG,("AETT cannot allocate mem for text!!!\n"));
			return SI_NO_MEMORY;
		}
		SI_memcpy(text->extended_text_message, current, text->extended_text_length);
		current += text->extended_text_length; /* points to next block. */

		/* insert the text into the AETT-n source link list. */
		if ((result = SI_AETT_Ins_Source_Text(slot,source_ID, text)) != SI_SUCCESS)
		{
			SI_mutex_unlock(m_aett);
			SI_DBG_PRINT(E_SI_ERR_MSG,("AETT cannot insert source!!!\n"));
			return result;
		}
	}

	/* lock mutex for AETT table access. */
	SI_mutex_unlock(m_aett);
	
	/* verify section length. */
	if (((unsigned long)current - (unsigned long)aett_table) != (section_length - SI_CRC_LENGTH))
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("AETT Table length error!\n"));
		return SI_SECTION_LENGTH_ERROR;
	}
	
	return SI_SUCCESS;
}

