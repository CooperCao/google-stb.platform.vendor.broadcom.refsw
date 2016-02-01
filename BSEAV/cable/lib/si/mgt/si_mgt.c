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
#include "si_aett.h"
#include "si_mgt.h"
#include "si_descriptors.h"


/* local function prototypes. */
static SI_RET_CODE SI_MGT_Upadte_Total_AEIT_NUMBER(unsigned long aeit_n);
static SI_RET_CODE SI_MGT_Upadte_Total_AETT_NUMBER(unsigned long aett_n);
static SI_RET_CODE SI_MGT_AEIT_Create_Slot (unsigned char *current, SI_AEIT_SLOT *prev_slot,
			unsigned long table_type, unsigned long version_number, unsigned long table_pid);
static SI_RET_CODE SI_MGT_AETT_Create_Slot (unsigned char *current, SI_AETT_SLOT *prev_slot,
			unsigned long table_type, unsigned long version_number, unsigned long table_pid);
static SI_RET_CODE SI_MGT_AEIT_Type_Receive(unsigned char *current, unsigned char aeit_n, unsigned long table_type);
static SI_RET_CODE SI_MGT_AETT_Type_Receive(unsigned char *current, unsigned char aett_n, unsigned long table_type);


struct aeit_slot_list head_aeit_slot, current_aeit0_slot;		/* always point to AEIT-0 */
struct aett_slot_list head_aett_slot, current_aett0_slot;		/* always point to AETT-0 */

unsigned char MGT_table_version;
unsigned short past_aeit_n, past_aett_n;  /* total number of slots we keep for the past. */
unsigned short mgt_aeit_n, mgt_aett_n;  /* total number of slots defined by MGT. */

SI_mutex m_aeit, m_aett;


void SI_MGT_Init (void)
{
	int i;

	SI_LST_D_INIT(&current_aeit0_slot);
	SI_LST_D_INIT(&current_aett0_slot);
	SI_LST_D_INIT(&head_aeit_slot);
	SI_LST_D_INIT(&head_aett_slot);

	MGT_table_version = 0xff;

	past_aeit_n = past_aett_n = 0;
	mgt_aeit_n = mgt_aett_n = 0;
	SI_mutex_init(m_aeit);
	SI_mutex_init(m_aett);
}

static SI_RET_CODE SI_MGT_Upadte_Total_AEIT_NUMBER(unsigned long aeit_n)
{
	SI_AEIT_SLOT * aeit_slot, *temps;
	int i;

	if (mgt_aeit_n > aeit_n)
	{
		/* if the new number of slots is smaller than current number. I don't know
			how possible it is?? */
		aeit_slot = SI_LST_D_FIRST(&current_aeit0_slot);
		for (i=0; ((i<aeit_n) && aeit_slot); i++)
			aeit_slot = SI_LST_D_NEXT(aeit_slot, slot_link);
		if (i != aeit_n || aeit_slot == NULL)
			return SI_AEIT_LIST_ERROR;

		/* free all the extra slots before. */
		for (i=aeit_n; i<mgt_aeit_n; i++)
		{
			if (aeit_slot)
			{
				temps = SI_LST_D_NEXT(aeit_slot, slot_link);
				SI_LST_D_REMOVE(&head_aeit_slot, aeit_slot, slot_link);
				SI_AEIT_Clear_Slot(aeit_slot);
				SI_free(aeit_slot);
				aeit_slot = temps;
			}
		}

	}

	mgt_aeit_n = aeit_n;

	return SI_SUCCESS;
}

static SI_RET_CODE SI_MGT_Upadte_Total_AETT_NUMBER(unsigned long aett_n)
{
	SI_AETT_SLOT * aett_slot, *temps;
	int i;

	if (mgt_aett_n > aett_n)
	{
		/* if the new number of slots is smaller than current number. I don't know
			how possible it is?? */
		aett_slot = SI_LST_D_FIRST(&current_aett0_slot);
		for (i=0; ((i<aett_n) && aett_slot); i++)
			aett_slot = SI_LST_D_NEXT(aett_slot, slot_link);
		if (i != aett_n || aett_slot == NULL)
			return SI_AETT_LIST_ERROR;

		/* free all the extra slots before. */
		for (i=aett_n; i<mgt_aett_n; i++)
		{
			if (aett_slot)
			{
				temps = SI_LST_D_NEXT(aett_slot, slot_link);
				SI_LST_D_REMOVE(&head_aett_slot, aett_slot, slot_link);
				SI_AETT_Clear_Slot(aett_slot);
				SI_free(aett_slot);
				aett_slot = temps;
			}
		}

	}

	mgt_aett_n = aett_n;

	return SI_SUCCESS;
}


SI_RET_CODE SI_MGT_AEIT_Del_All_Slot(void)
{
	SI_AEIT_SLOT * aeit_slot;

	while ((aeit_slot = SI_LST_D_FIRST(&head_aeit_slot)))
	{
		SI_LST_D_REMOVE_HEAD(&head_aeit_slot, slot_link);
		SI_AEIT_Clear_Slot(aeit_slot);
		SI_free(aeit_slot);
	}

	SI_LST_D_INIT(&current_aeit0_slot);
	SI_LST_D_INIT(&head_aeit_slot);
	past_aeit_n = 0;
	mgt_aeit_n = 0;

	return SI_SUCCESS;
}

SI_RET_CODE SI_MGT_AETT_Del_All_Slot(void)
{
	SI_AETT_SLOT * aett_slot;

	while ((aett_slot = SI_LST_D_FIRST(&head_aett_slot)))
	{
		SI_LST_D_REMOVE_HEAD(&head_aett_slot, slot_link);
		SI_AETT_Clear_Slot(aett_slot);
		SI_free(aett_slot);
	}

	SI_LST_D_INIT(&current_aett0_slot);
	SI_LST_D_INIT(&head_aett_slot);
	past_aett_n = 0;
	mgt_aett_n = 0;

	return SI_SUCCESS;
}

static SI_RET_CODE SI_MGT_AEIT_Create_Slot (unsigned char *current, SI_AEIT_SLOT *prev_slot,
			unsigned long table_type, unsigned long version_number, unsigned long table_pid)
{
	SI_AEIT_SLOT * aeit_slot;

	if ((aeit_slot = SI_AEIT_Create_Slot()) == NULL)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("MGT failed to create AEIT slot!!!\n"));
		return SI_NO_MEMORY;
	}

	aeit_slot->MGT_tag = (table_type&0xff);
	aeit_slot->MGT_version_number = version_number;
	aeit_slot->pid = table_pid;

	if (prev_slot)
		SI_LST_D_INSERT_AFTER(prev_slot, aeit_slot, slot_link);
	else
	{
		/* in this case the slot list MUST be EMPTY! */
		/* create the first element in aeit slot list. init head and current aeit0 pointer. */
		SI_LST_D_INSERT_HEAD(&current_aeit0_slot, aeit_slot, slot_link);
		SI_LST_D_FIRST(&head_aeit_slot) = SI_LST_D_FIRST(&current_aeit0_slot);
	}

	return SI_SUCCESS;
}

static SI_RET_CODE SI_MGT_AETT_Create_Slot (unsigned char *current, SI_AETT_SLOT *prev_slot,
			unsigned long table_type, unsigned long version_number, unsigned long table_pid)
{
	SI_AETT_SLOT * aett_slot;

	if ((aett_slot = SI_AETT_Create_Slot()) == NULL)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("MGT failed to create AETT slot!!!\n"));
		return SI_NO_MEMORY;
	}

	aett_slot->MGT_tag = (table_type&0xff);
	aett_slot->MGT_version_number = version_number;
	aett_slot->pid = table_pid;

	if (prev_slot)
		SI_LST_D_INSERT_AFTER(prev_slot, aett_slot, slot_link);
	else
	{
		/* in this case the slot list MUST be EMPTY! */
		/* create the first element in aett slot list. init head and current aett0 pointer. */
		SI_LST_D_INSERT_HEAD(&current_aett0_slot, aett_slot, slot_link);
		SI_LST_D_FIRST(&head_aett_slot) = SI_LST_D_FIRST(&current_aett0_slot);
	}

	return SI_SUCCESS;
}

/********************************************************************
 Function : SI_MGT_AEIT_Type_Receive
 Description : Function to handle the receiving of AEIT table type
				 for MGT table. It will create and
				 keep track of AEIT instances.
 Input : current, point to the current mgt table type data.
		   aeit_n: the instance of AEIT type received.
		   tag : the table type of this aeit slot.
 Output : Success code.
********************************************************************/
static SI_RET_CODE SI_MGT_AEIT_Type_Receive(unsigned char *current, unsigned char aeit_n, unsigned long table_type)
{
	SI_AEIT_SLOT * aeit_slot, *prev_aeit_slot, *new_aeit_slot;
	int i, j;
	unsigned long version_number, table_pid;

	version_number = SI_Construct_Data(current,
			MGT_TABLE_TYPE_VERSION_NUMBER_BYTE_INDX,
			MGT_TABLE_TYPE_VERSION_NUMBER_BYTE_NUM,
			MGT_TABLE_TYPE_VERSION_NUMBER_SHIFT,
			MGT_TABLE_TYPE_VERSION_NUMBER_MASK);
	table_pid =	SI_Construct_Data(current,
				MGT_TABLE_TYPE_PID_BYTE_INDX,
				MGT_TABLE_TYPE_PID_BYTE_NUM,
				MGT_TABLE_TYPE_PID_SHIFT,
				MGT_TABLE_TYPE_PID_MASK);

	/* find where aeit-n points to. */
	aeit_slot = SI_LST_D_FIRST(&current_aeit0_slot);
	if (aeit_slot)
		prev_aeit_slot = SI_LST_D_PREV(aeit_slot, slot_link);
	else
		prev_aeit_slot = NULL;
	for (i=0; ((i<aeit_n) && aeit_slot); i++)
	{
		prev_aeit_slot = aeit_slot;
		aeit_slot = SI_LST_D_NEXT(aeit_slot, slot_link);
	}

	/* if the slot does not exist. */
	if (aeit_slot == NULL)
		return SI_MGT_AEIT_Create_Slot(current, prev_aeit_slot, table_type, version_number, table_pid);

	/* the slot is already there. */
	if (aeit_slot->MGT_tag != (table_type&0xff))
	{
		/* the slot may have moved. We SHOULD only be able to get here the first time!!! Since
			after we moved the first time, everything else should be aligned. */
		if (aeit_n == 0)
		{
			/* for AEIT-0. */
			/* find the MGT_tag down the line that matches the current received one.
				should be right after the current slot. */
			j=0;
			while (aeit_slot->MGT_tag != (table_type&0xff))
			{
				if (SI_LST_D_NEXT(aeit_slot, slot_link) == NULL)
					break;
				j++;
				aeit_slot = SI_LST_D_NEXT(aeit_slot, slot_link);
			}

			if (aeit_slot->MGT_tag != (table_type&0xff))
			{
				SI_DBG_PRINT(E_SI_ERR_MSG,("MGT failed to find AEIT-%d slot!!!\n", aeit_n));
				/* we should free everything and start over again. */
				SI_MGT_AEIT_Del_All_Slot();
				return SI_AEIT_LIST_ERROR;
			}

			if (j>1)
			{
				/* if the AEIT slot has shifted for more than 1 spot. We should give
					a warning. */
				SI_DBG_PRINT(E_SI_WRN_MSG,("MGT AEIT-%d shifted %d slots\n", aeit_n, j));
			}

			/* modify the current_aeit_slot pointer. */
			SI_LST_D_FIRST(&current_aeit0_slot) = aeit_slot;

			/* Check the version and do as needed. */
			if (version_number != aeit_slot->MGT_version_number)
			{
				/* The version number changed. clear the old slot
					and put new data in it. */
				SI_AEIT_Clear_Slot(aeit_slot);
				aeit_slot->MGT_tag = (table_type&0xff);
				aeit_slot->MGT_version_number = version_number;
			}
			/* always update the pid. */
			aeit_slot->pid = table_pid;

			/* check to see if we need to delete any past AEIT slots. */
			past_aeit_n += j;
			if (past_aeit_n > MAX_PAST_AEXT_N)
			{
				for (j=0; ((j<(past_aeit_n-MAX_PAST_AEXT_N)) && ((aeit_slot = SI_LST_D_FIRST(&head_aeit_slot)) != NULL)); j++)
				{
					SI_LST_D_REMOVE_HEAD(&head_aeit_slot, slot_link);
					SI_AEIT_Clear_Slot(aeit_slot);
					SI_free(aeit_slot);
				}
			}
		}
		else
		{
			/* for AEIT-n where n>0. should not happen. */
			SI_DBG_PRINT(E_SI_ERR_MSG,("MGT AEIT-%d slot does not match new one!!!\n", aeit_n));
			/* we should just replace it with the new one */
			SI_AEIT_Clear_Slot(aeit_slot);
			aeit_slot->MGT_tag = (table_type&0xff);
			aeit_slot->MGT_version_number = version_number;
			aeit_slot->pid = table_pid;
			/* we still want to keep going after this . */
		}
	}
	else  /*if (aeit_slot->MGT_tag == (table_type&0xff))*/
	{
		/* the slot has not moved yet. Check the version and do as needed. */
		if (version_number != aeit_slot->MGT_version_number)
		{
			/* The version number changed. clear the old slot
				and put new data in it. */
			SI_AEIT_Clear_Slot(aeit_slot);
			aeit_slot->MGT_tag = (table_type&0xff);
			aeit_slot->MGT_version_number = version_number;
		}
		aeit_slot->pid = table_pid; /* always update the pid. */
	}


	return SI_SUCCESS;
}

/********************************************************************
 Function : SI_MGT_AETT_Type_Receive
 Description : Function to handle the receiving of AETT table type
				 for MGT table. It will create and
				 keep track of AETT instances.
 Input : current, point to the current mgt table type data.
		   aett_n: the instance of AETT type received.
		   tag : the table type of this aett slot.
 Output : Success code.
********************************************************************/
static SI_RET_CODE SI_MGT_AETT_Type_Receive(unsigned char *current, unsigned char aett_n, unsigned long table_type)
{
	SI_AETT_SLOT * aett_slot, *prev_aett_slot, *new_aett_slot;
	int i, j;
	unsigned long version_number, table_pid;

	version_number = SI_Construct_Data(current,
			MGT_TABLE_TYPE_VERSION_NUMBER_BYTE_INDX,
			MGT_TABLE_TYPE_VERSION_NUMBER_BYTE_NUM,
			MGT_TABLE_TYPE_VERSION_NUMBER_SHIFT,
			MGT_TABLE_TYPE_VERSION_NUMBER_MASK);
	table_pid =	SI_Construct_Data(current,
				MGT_TABLE_TYPE_PID_BYTE_INDX,
				MGT_TABLE_TYPE_PID_BYTE_NUM,
				MGT_TABLE_TYPE_PID_SHIFT,
				MGT_TABLE_TYPE_PID_MASK);

	/* find where aett-n points to. */
	aett_slot = SI_LST_D_FIRST(&current_aett0_slot);
	if (aett_slot)
		prev_aett_slot = SI_LST_D_PREV(aett_slot, slot_link);
	else
		prev_aett_slot = NULL;
	for (i=0; ((i<aett_n) && aett_slot); i++)
	{
		prev_aett_slot = aett_slot;
		aett_slot = SI_LST_D_NEXT(aett_slot, slot_link);
	}

	/* if the slot does not exist. */
	if (aett_slot == NULL)
		return SI_MGT_AETT_Create_Slot(current, prev_aett_slot, table_type, version_number, table_pid);

	/* the slot is already there. */
	if (aett_slot->MGT_tag != (table_type&0xff))
	{
		/* the slot may have moved. We SHOULD only be able to get here the first time!!! Since
			after we moved the first time, everything else should be aligned. */
		if (aett_n == 0)
		{
			/* for AETT-0. */
			/* find the MGT_tag down the line that matches the current received one.
				should be right after the current slot. */
			j=0;
			while (aett_slot->MGT_tag != (table_type&0xff))
			{
				if (SI_LST_D_NEXT(aett_slot, slot_link) == NULL)
					break;
				j++;
				aett_slot = SI_LST_D_NEXT(aett_slot, slot_link);
			}

			if (aett_slot->MGT_tag != (table_type&0xff))
			{
				SI_DBG_PRINT(E_SI_ERR_MSG,("MGT failed to find AETT-%d slot!!!\n", aett_n));
				/* we should free everything and start over again. */
				SI_MGT_AETT_Del_All_Slot();
				return SI_AETT_LIST_ERROR;
			}

			if (j>1)
			{
				/* if the AETT slot has shifted for more than 1 spot. We should give
					a warning. */
				SI_DBG_PRINT(E_SI_WRN_MSG,("MGT AETT-%d shifted %d slots\n", aett_n, j));
			}

			/* modify the current_aett_slot pointer. */
			SI_LST_D_FIRST(&current_aett0_slot) = aett_slot;

			/* Check the version and do as needed. */
			if (version_number != aett_slot->MGT_version_number)
			{
				/* The version number changed. clear the old slot
					and put new data in it. */
				SI_AETT_Clear_Slot(aett_slot);
				aett_slot->MGT_tag = (table_type&0xff);
				aett_slot->MGT_version_number = version_number;
			}
			/* always update the pid. */
			aett_slot->pid = table_pid;

			/* check to see if we need to delete any past AETT slots. */
			past_aett_n += j;
			if (past_aett_n > MAX_PAST_AEXT_N)
			{
				for (j=0; ((j<(past_aett_n-MAX_PAST_AEXT_N)) && ((aett_slot = SI_LST_D_FIRST(&head_aett_slot)) != NULL)); j++)
				{
					SI_LST_D_REMOVE_HEAD(&head_aett_slot, slot_link);
					SI_AETT_Clear_Slot(aett_slot);
					SI_free(aett_slot);
				}
			}
		}
		else
		{
			/* for AETT-n where n>0. It should not happen. */
			SI_DBG_PRINT(E_SI_ERR_MSG,("MGT AETT-%d slot does not match new one!!!\n", aett_n));
			/* we should just replace it with the new one */
			SI_AETT_Clear_Slot(aett_slot);
			aett_slot->MGT_tag = (table_type&0xff);
			aett_slot->MGT_version_number = version_number;
			aett_slot->pid = table_pid;
			/* we still want to keep going after this . */
		}
	}
	else /*if (aett_slot->MGT_tag == (table_type&0xff))*/
	{
		/* the slot has not moved yet. Check the version and do as needed. */
		if (version_number != aett_slot->MGT_version_number)
		{
			/* The version number changed. clear the old slot
				and put new data in it. */
			SI_AETT_Clear_Slot(aett_slot);
			aett_slot->MGT_tag = (table_type&0xff);
			aett_slot->MGT_version_number = version_number;
		}
		aett_slot->pid = table_pid; /* always update the pid. */
	}


	return SI_SUCCESS;
}

/*********************************************************************/
/* Function : SI_MGT_parse							 			 	 */
/* Description : Function to parse the MGT table. It will create and */
/*				 keep track of AEIT and AETT instances. 			 */
/* Input : mgt_table, point to the mgt table data.         			 */
/* Output : Success code.    										 */
/*********************************************************************/
SI_RET_CODE SI_MGT_parse (unsigned char * mgt_table)
{
	unsigned long temp, i;
	unsigned long section_length;
	unsigned long tables_defined, table_type, num_bytes;
	unsigned long CRC_start;
	unsigned long desc_start, len;
	unsigned long desc_tag, desc_len;
	unsigned char *desc, *current;
	unsigned char curr_aeit_n, curr_aett_n;
	SI_RET_CODE result;

	SI_DBG_PRINT(E_SI_DBG_MSG,("MGT Table received.\n"));

	temp = *mgt_table;
	if (temp != SI_MGT_TABLE_ID)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("MGT Table ID error!!! %x\n", temp));
		return SI_TABLE_ID_ERROR;
	}

	/* calculate and check section length. */
	section_length = SI_Construct_Data(mgt_table,
				MGT_SECTION_LENGTH_BYTE_INDX,
				MGT_SECTION_LENGTH_BYTE_NUM,
				MGT_SECTION_LENGTH_SHIFT,
				MGT_SECTION_LENGTH_MASK);
	section_length += MGT_SECTION_LENGTH_BYTE_INDX+MGT_SECTION_LENGTH_BYTE_NUM;
	if (section_length > SI_LONG_SECTION_LENGTH)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("MGT Table section length error!!! %x\n", section_length));
		return SI_SECTION_LENGTH_ERROR;
	}

	/* We do the CRC check here to verify the contents of this section. */
	if (SI_CRC32_Check(mgt_table, section_length) != SI_SUCCESS)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("MGT Table section CRC error!!!\n"));
		/*return SI_CRC_ERROR;*/
	}

	/* check to make sure protocol version is zero. */
	temp = SI_Construct_Data(mgt_table,
				MGT_PROTOCOL_VERSION_BYTE_INDX,
				MGT_PROTOCOL_VERSION_BYTE_NUM,
				MGT_PROTOCOL_VERSION_SHIFT,
				MGT_PROTOCOL_VERSION_MASK);
	if (temp != SI_CURRENT_PROTOCOL_VERSION)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("MGT Table PROTOCOL version error!!! %x\n", temp));
		return SI_PROTOCOL_VER_ERROR;
	}

	/* make sure that section_number and last_section_number are both 0. */
	temp = SI_Construct_Data(mgt_table,
				MGT_SECTION_NUMBER_BYTE_INDX,
				MGT_SECTION_NUMBER_BYTE_NUM,
				MGT_SECTION_NUMBER_SHIFT,
				MGT_SECTION_NUMBER_MASK);
	if (temp != 0)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("MGT Table section_number not zero!!! %x\n", temp));
		return SI_SECTION_NUMBER_ERROR;
	}
	temp = SI_Construct_Data(mgt_table,
				MGT_LAST_SECTION_NUMBER_BYTE_INDX,
				MGT_LAST_SECTION_NUMBER_BYTE_NUM,
				MGT_LAST_SECTION_NUMBER_SHIFT,
				MGT_LAST_SECTION_NUMBER_MASK);
	if (temp != 0)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("MGT Table last_section_number not zero!!! %x\n", temp));
		return SI_SECTION_NUMBER_ERROR;
	}

	/* look at current_next_indicator. It should be 1 for MGT. */
	temp = SI_Construct_Data(mgt_table,
				MGT_CURRENT_NEXT_INDICATOR_BYTE_INDX,
				MGT_CURRENT_NEXT_INDICATOR_BYTE_NUM,
				MGT_CURRENT_NEXT_INDICATOR_SHIFT,
				MGT_CURRENT_NEXT_INDICATOR_MASK);
	if (temp != 1)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("MGT Table current_next_indicator not one!!! %x\n", temp));
		return SI_CURRENT_NEXT_INDICATOR_ERROR;
	}

	/* Now check the version number and decide whether we want to update or not */
	temp = SI_Construct_Data(mgt_table,
				MGT_VERSION_NUMBER_BYTE_INDX,
				MGT_VERSION_NUMBER_BYTE_NUM,
				MGT_VERSION_NUMBER_SHIFT,
				MGT_VERSION_NUMBER_MASK);
	if (MGT_table_version == temp)
		return SI_SUCCESS; /* do not need to do anything when it is the same version */

	/* new MGT table received!!! */
	MGT_table_version = temp;
	SI_DBG_PRINT(E_SI_DBG_MSG,("New MGT Table received!\n"));

	tables_defined = SI_Construct_Data(mgt_table,
				MGT_TABLES_DEFINED_BYTE_INDX,
				MGT_TABLES_DEFINED_BYTE_NUM,
				MGT_TABLES_DEFINED_SHIFT,
				MGT_TABLES_DEFINED_MASK);
printf("MGT tables define %x\n", tables_defined);

	/* going through all the tables. */
	current = mgt_table + MGT_TABLES_DEFINED_BYTE_INDX + MGT_TABLES_DEFINED_BYTE_NUM;
	curr_aeit_n = curr_aett_n = 0;
	for (i=0; i<tables_defined; i++)
	{
		table_type = SI_Construct_Data(current,
				MGT_TABLE_TYPE_BYTE_INDX,
				MGT_TABLE_TYPE_BYTE_NUM,
				MGT_TABLE_TYPE_SHIFT,
				MGT_TABLE_TYPE_MASK);
		num_bytes = SI_Construct_Data(current,
				MGT_NUMBER_BYTES_BYTE_INDX,
				MGT_NUMBER_BYTES_BYTE_NUM,
				MGT_NUMBER_BYTES_SHIFT,
				MGT_NUMBER_BYTES_MASK);
		desc_len = SI_Construct_Data(current,
				MGT_TABLE_TYPE_DESC_LENGTH_BYTE_INDX,
				MGT_TABLE_TYPE_DESC_LENGTH_BYTE_NUM,
				MGT_TABLE_TYPE_DESC_LENGTH_SHIFT,
				MGT_TABLE_TYPE_DESC_LENGTH_MASK);

		/* check for  AEIT/AETT types. */
		if (table_type>=MGT_TABLE_TYPE_AEIT_START && table_type<=MGT_TABLE_TYPE_AEIT_END)
		{
			SI_DBG_PRINT(E_SI_DBG_MSG,("MGT: AEIT type received.\n"));

			/* lock mutex for AEIT table access. */
			SI_mutex_lock(m_aeit);

			/* handle the AEIT types. */
			result = SI_MGT_AEIT_Type_Receive(current, curr_aeit_n, table_type);
			if (result == SI_AEIT_LIST_ERROR)
			{
				/* error happened, */
				/* for AEIT_LIST_ERROR, we already cleared the slot list. Now reprocess it. */
				if ((result = SI_MGT_AEIT_Type_Receive(current, curr_aeit_n, table_type)) != SI_SUCCESS)
				{
					SI_mutex_unlock(m_aeit);
					return result;
				}
			}
			else if (result != SI_SUCCESS)
			{
				/*Other errors, we simply return. */
				SI_mutex_unlock(m_aeit);
				return result;
			}

			curr_aeit_n++;		/* incrementing current number of AEIT's received. */

			/* unlock mutex for AEIT table access. */
			SI_mutex_unlock(m_aeit);
		}
		else if (table_type>=MGT_TABLE_TYPE_AETT_START && table_type<=MGT_TABLE_TYPE_AETT_END)
		{
			SI_DBG_PRINT(E_SI_DBG_MSG,("MGT: AETT type received.\n"));

			/* lock mutex for AETT table access. */
			SI_mutex_lock(m_aett);

			/* handle the AETT types. */
			result = SI_MGT_AETT_Type_Receive(current, curr_aett_n, table_type);
			if (result == SI_AETT_LIST_ERROR)
			{
				/* error happened, */
				/* for AETT_LIST_ERROR, we already cleared the slot list. Now reprocess it. */
				if ((result = SI_MGT_AETT_Type_Receive(current, curr_aett_n, table_type)) != SI_SUCCESS)
				{
					SI_mutex_unlock(m_aett);
					return result;
				}
			}
			else if (result != SI_SUCCESS)
			{
				/*Other errors, we simply return. */
				SI_mutex_unlock(m_aett);
				return result;
			}

			curr_aett_n++;		/* incrementing current number of AETT's received. */

			/* unlock mutex for AETT table access. */
			SI_mutex_unlock(m_aett);
		}

#if 0	/* I don't knopw what num_bytes are for? */
		/* for now just handle the AEIT/AETT table type. */
		if (num_bytes != desc_len + MGT_TABLE_TYPE_DESC_LENGTH_BYTE_NUM)
		{
			SI_DBG_PRINT(E_SI_ERR_MSG,("MGT Table type %x, number_bytes = %d, table type descriptor length %d!!!\n", table_type, num_bytes, desc_len));
			return SI_SECTION_LENGTH_ERROR;
		}
#endif

		/* only handle stuffing descriptors for now. */
		current += MGT_TABLE_TYPE_DESC_LENGTH_BYTE_INDX+MGT_TABLE_TYPE_DESC_LENGTH_BYTE_NUM;
		desc_start = 0;
		while (desc_start < desc_len)
		{
			if ( (desc_tag = *(current++)) != SI_DESC_STUFFING)
				SI_DBG_PRINT(E_SI_WRN_MSG,("MGT table type descriptor %x received! Ignoring!\n", desc_tag));

			len = *(current++); /* desc length. */
			current += len;
			desc_start += len+2;
		}

		if (desc_start != desc_len)
		{
			SI_DBG_PRINT(E_SI_ERR_MSG,("MGT Table type descriptor length error!!!\n"));
			return SI_DESCRIPTOR_ERROR;
		}

	}		/* end of for (i=0; i<tables_defined; i++) */

	/* check for total number of AEIT/AETT-n's received. */
	SI_mutex_lock(m_aeit);
	SI_MGT_Upadte_Total_AEIT_NUMBER (curr_aeit_n);
	SI_mutex_unlock(m_aeit);

	SI_mutex_lock(m_aett);
	SI_MGT_Upadte_Total_AETT_NUMBER (curr_aett_n);
	SI_mutex_unlock(m_aett);

	/* just for fun, go through table desc and make sure the section length works out. */
	desc_len = SI_Construct_Data(current,
				MGT_DESC_LENGTH_BYTE_INDX,
				MGT_DESC_LENGTH_BYTE_NUM,
				MGT_DESC_LENGTH_SHIFT,
				MGT_DESC_LENGTH_MASK);
	current += MGT_DESC_LENGTH_BYTE_INDX+MGT_DESC_LENGTH_BYTE_NUM;
	desc_start = 0;
	while (desc_start < desc_len)
	{
		if ( (desc_tag = *(current++)) != SI_DESC_STUFFING)
			SI_DBG_PRINT(E_SI_WRN_MSG,("MGT table descriptor %x received! Ignoring!\n", desc_tag));

		len = *(current++); /* desc length. */
		current += len;
		desc_start += len+2;
	}

	if (desc_start != desc_len)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("MGT Table descriptor length error!!!\n"));
		return SI_DESCRIPTOR_ERROR;
	}

	if (current+SI_CRC_LENGTH != (mgt_table + section_length))
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("MGT Table length error!!!\n"));
		return SI_SECTION_LENGTH_ERROR;
	}

	return SI_SUCCESS;
}
