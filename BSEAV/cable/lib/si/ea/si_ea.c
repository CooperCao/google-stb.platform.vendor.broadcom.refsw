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
#include "si_ea.h"
#include "si_api_ea.h"
#include "si_descriptors.h"

static unsigned char ea_sequence_number, ea_flags = 0;


EA_MSG_INFO		ea_msg;
static 	void (*EA_message_received_cb)(EA_MSG_INFO*) = NULL;

#define EA_FLAGS_SEQ_NUM_VALID		0x01
#define EA_FLAGS_EVENT_ID_VALID		0x02

void SI_EA_Init (void (*cb))
{
	EA_message_received_cb = cb;
}

void SI_EA_UnInit (void)
{
	EA_message_received_cb = NULL;
}
/*********************************************************************
 Function : SI_EA_GetEventTime
 Description : Function to get event start time and duration
 Input : p_ea_start_time - pointer to store start time
       : p_ea_duration   - pointer to store duration
 Output : none.
**********************************************************************/
void SI_EA_GetEventTime (unsigned long *p_ea_start_time, unsigned short *p_ea_duration)
{
	*p_ea_start_time = ea_msg.ea_event_start_time;
	*p_ea_duration = ea_msg.ea_event_duration;
}

/*********************************************************************
 Function : SI_EA_GetEventTime
 Description : Callback function when an event expires
 Input : none.
 Output : none.
**********************************************************************/
void SI_EA_EventExpirationCb (void)
{
	ea_flags &= ~EA_FLAGS_EVENT_ID_VALID;
}

/*********************************************************************
 Function : SI_EA_parse
 Description : Function to parse the EA message
 Input : ea_table, point to the EA message data
 Output : Success code.
**********************************************************************/
SI_RET_CODE SI_EA_Parse (unsigned char * ea_table)
{
	int				i;
	unsigned char 	temp, eas_event_code_length, nature_of_act_text_length;
	unsigned short	temp16;
	unsigned short	ea_alert_text_length, ea_descriptors_length;
	unsigned long	temp32, ea_byte_index;
	unsigned long	section_length;

	SI_DBG_PRINT(E_SI_DBG_MSG,("EA Message received.\n"));

	ea_msg.process = 1;

	temp = *ea_table;
	if (temp != SI_EA_TABLE_ID)
	{
		ea_msg.process = 0;
		SI_DBG_PRINT(E_SI_ERR_MSG,("EA Table ID error!!! %x\n", temp));
		return SI_TABLE_ID_ERROR;
	}

	/*
	 * Get section length
	 */
	section_length = SI_Construct_Data(ea_table,
				EA_SECTION_LENGTH_BYTE_INDX,
				EA_SECTION_LENGTH_BYTE_NUM,
				EA_SECTION_LENGTH_SHIFT,
				EA_SECTION_LENGTH_MASK);
	section_length += EA_SECTION_LENGTH_BYTE_INDX+EA_SECTION_LENGTH_BYTE_NUM;
	if (section_length > SI_NORMAL_SECTION_LENGTH)
	{
		ea_msg.process = 0;
		SI_DBG_PRINT(E_SI_ERR_MSG,("EA Table section length error!!! %x\n", section_length));
		return SI_SECTION_LENGTH_ERROR;
	}
	SI_DBG_PRINT(E_SI_DBG_MSG,("EA Table section length = %d\n", section_length));

#if 0
printf("Data:");
for (i=0; i < section_length; i++)
{
	if (!(i & 0x0f)) printf("\n");
	printf("%02x ", ea_table[i]);
}
printf("\n");
#endif

	/*
	 * Check table id extension
	 */
	temp = SI_Construct_Data(ea_table,
				EA_TABLE_ID_EXTENSION_BYTE_INDX,
				EA_TABLE_ID_EXTENSION_BYTE_NUM,
				EA_TABLE_ID_EXTENSION_SHIFT,
				EA_TABLE_ID_EXTENSION_MASK);
	if (temp != 0)
	{
		ea_msg.process = 0;
		SI_DBG_PRINT(E_SI_ERR_MSG,("EA Table ID extension error!!! %x\n", temp));
		return SI_TABLE_ID_ERROR;
	}

	/*
	 * Check protocol version
	 */
	temp = SI_Construct_Data(ea_table,
				EA_PROTOCOL_VERSION_BYTE_INDX,
				EA_PROTOCOL_VERSION_BYTE_NUM,
				EA_PROTOCOL_VERSION_SHIFT,
				EA_PROTOCOL_VERSION_MASK);
	if (temp != 0)				/* only support protocol version 0 */
	{
		ea_msg.process = 0;
		SI_DBG_PRINT(E_SI_ERR_MSG,("EA Table PROTOCOL version error!!! %x\n", temp));
		return SI_PROTOCOL_VER_ERROR;
	}

	/*
	 * Get sequence number
	 */
	temp32 = SI_Construct_Data(ea_table,
						EA_SEQUENCE_NUMBER_BYTE_INDX,
						EA_SEQUENCE_NUMBER_BYTE_NUM,
						EA_SEQUENCE_NUMBER_SHIFT,
						EA_SEQUENCE_NUMBER_MASK);

	if ((ea_flags & EA_FLAGS_SEQ_NUM_VALID) && (temp32 == ea_sequence_number))
	{
		ea_msg.process = 0;
		SI_DBG_PRINT(E_SI_ERR_MSG,("Duplicate sequence number %x, discarding message\n", temp32));
		return SI_SUCCESS;
	}

	ea_flags |= EA_FLAGS_SEQ_NUM_VALID;
	ea_sequence_number = temp32;
	SI_DBG_PRINT(E_SI_DBG_MSG,("EA Table sequence number = %x\n", ea_sequence_number));

	/*
	 * Get Event ID
	 */
	temp16 = SI_Construct_Data(ea_table,
						EA_EAS_EVENT_ID_BYTE_INDX,
						EA_EAS_EVENT_ID_BYTE_NUM,
						EA_EAS_EVENT_ID_SHIFT,
						EA_EAS_EVENT_ID_MASK);
	if ((ea_flags & EA_FLAGS_EVENT_ID_VALID) && (temp32 == ea_msg.eas_event_id))
	{
		ea_msg.process = 0;
		SI_DBG_PRINT(E_SI_ERR_MSG,("Duplicate event id %x, discarding message\n", temp16));
		return SI_SUCCESS;
	}

	ea_flags |= EA_FLAGS_EVENT_ID_VALID;
	ea_msg.eas_event_id = temp16;
	SI_DBG_PRINT(E_SI_DBG_MSG,("EA event id = %x\n", ea_msg.eas_event_id));

	/*
	 * Get EAS originator code
	 */
	ea_msg.eas_originator_code[0] = ea_table[EA_EAS_ORIGINATOR_CODE_BYTE_INDX];
	ea_msg.eas_originator_code[1] = ea_table[EA_EAS_ORIGINATOR_CODE_BYTE_INDX + 1];
	ea_msg.eas_originator_code[2] = ea_table[EA_EAS_ORIGINATOR_CODE_BYTE_INDX + 2];
	ea_msg.eas_originator_code[3] = 0;
	SI_DBG_PRINT(E_SI_DBG_MSG,("EAS originator code = %s\n", ea_msg.eas_originator_code));

	/*
	 * Get EAS event code
	 */
	eas_event_code_length = SI_Construct_Data(ea_table,
						EA_EAS_EVENT_CODE_LENGTH_BYTE_INDX,
						EA_EAS_EVENT_CODE_LENGTH_BYTE_NUM,
						EA_EAS_EVENT_CODE_LENGTH_SHIFT,
						EA_EAS_EVENT_CODE_LENGTH_MASK);

	/*
	 * Get EAS event code
	 */
	for (i=0; i < eas_event_code_length; i++)
		ea_msg.eas_event_code[i] = ea_table[EA_EAS_EVENT_CODE_BYTE_INDX + i];
	ea_msg.eas_event_code[i] = 0;
	SI_DBG_PRINT(E_SI_DBG_MSG,("EAS event code = %s\n", ea_msg.eas_event_code));

	ea_byte_index = EA_EAS_EVENT_CODE_BYTE_INDX + eas_event_code_length;
	SI_DBG_PRINT(E_SI_DBG_MSG,("EA byte index = %d\n", ea_byte_index));

	/*
	 * Get nature of activation text fields
	 */
	nature_of_act_text_length = ea_table[ea_byte_index++];
	SI_DBG_PRINT(E_SI_DBG_MSG,("Nature of activation text length = %d\n", nature_of_act_text_length));
	if (nature_of_act_text_length)
	{
		SI_DBG_PRINT(E_SI_DBG_MSG,("Nature of activation text = "));
		for (i=0; i < nature_of_act_text_length; i++)
		{
			ea_msg.nature_of_act_text[i] = ea_table[ea_byte_index++];
			SI_DBG_PRINT(E_SI_DBG_MSG,("%c", ea_msg.nature_of_act_text[i]));
		}
		SI_DBG_PRINT(E_SI_DBG_MSG,("\n"));
		ea_msg.nature_of_act_text[i] = 0;
	}
	else
		ea_msg.nature_of_act_text[0] = 0;

	/*
	 * Alert message time remaining
	 */
	ea_msg.ea_alert_message_time_rem = ea_table[ea_byte_index++];
	SI_DBG_PRINT(E_SI_DBG_MSG,("Alert message time remaining = %d\n", ea_msg.ea_alert_message_time_rem));

	/*
	 * Event start time
	 */
	ea_msg.ea_event_start_time = SI_Construct_Data(ea_table, ea_byte_index, 4, 0, 0xffffffff);
	SI_DBG_PRINT(E_SI_DBG_MSG,("Event start time = %x\n", ea_msg.ea_event_start_time));
	ea_byte_index += 4;

	/*
	 * Event duration
	 */
	ea_msg.ea_event_duration = SI_Construct_Data(ea_table, ea_byte_index, 2, 0, 0xffff);
	SI_DBG_PRINT(E_SI_DBG_MSG,("Event duration = %x\n", ea_msg.ea_event_duration));
	ea_byte_index += 2;

	/*
	 * Alert priority
	 */
	ea_msg.ea_alert_priority = SI_Construct_Data(ea_table, ea_byte_index, 2, 0, 0x000f);
	SI_DBG_PRINT(E_SI_DBG_MSG,("Alert priority = %x\n", ea_msg.ea_alert_priority));
	ea_byte_index += 2;

	/*
	 * Details OOB source ID
	 */
	ea_msg.ea_details_oob_source_id = SI_Construct_Data(ea_table, ea_byte_index, 2, 0, 0xffff);
	SI_DBG_PRINT(E_SI_DBG_MSG,("Details OOB source id = %x\n", ea_msg.ea_details_oob_source_id));
	ea_byte_index += 2;

	/*
	 * Details major channel number
	 */
	ea_msg.ea_details_major_chan_number = SI_Construct_Data(ea_table, ea_byte_index, 2, 0, 0x03ff);
	SI_DBG_PRINT(E_SI_DBG_MSG,("Details major channel number = %x\n", ea_msg.ea_details_major_chan_number));
	ea_byte_index += 2;

	/*
	 * Details minor channel number
	 */
	ea_msg.ea_details_minor_chan_number = SI_Construct_Data(ea_table, ea_byte_index, 2, 0, 0x03ff);
	SI_DBG_PRINT(E_SI_DBG_MSG,("Details minor channel number = %x\n", ea_msg.ea_details_minor_chan_number));
	ea_byte_index += 2;

	/*
	 * Audio OOB source id
	 */
	ea_msg.ea_audio_oob_source_id = SI_Construct_Data(ea_table, ea_byte_index, 2, 0, 0xffff);
	SI_DBG_PRINT(E_SI_DBG_MSG,("Audio OOB source id = %x\n", ea_msg.ea_audio_oob_source_id));
	ea_byte_index += 2;

	/*
	 * Alert text length, alert text
	 */
	ea_alert_text_length = SI_Construct_Data(ea_table, ea_byte_index, 2, 0, 0xffff);
	SI_DBG_PRINT(E_SI_DBG_MSG,("Alert text length = %x\n", ea_alert_text_length));
	ea_byte_index += 2;
	if (ea_alert_text_length)
	{
		SI_DBG_PRINT(E_SI_DBG_MSG,("EA alert text = "));
		for (i=0; i < ea_alert_text_length; i++)
		{
			ea_msg.ea_alert_text[i] = ea_table[ea_byte_index++];
			SI_DBG_PRINT(E_SI_DBG_MSG,("%c", ea_msg.ea_alert_text[i]));
		}
		SI_DBG_PRINT(E_SI_DBG_MSG,("\n"));
		ea_msg.ea_alert_text[i] = 0;
	}
	else
		ea_msg.ea_alert_text[0] = 0;

	/*
	 * Location code
	 */
	ea_msg.ea_location_code_count = SI_Construct_Data(ea_table, ea_byte_index, 1, 0, 0xff);
	SI_DBG_PRINT(E_SI_DBG_MSG,("Location code count = %x\n", ea_msg.ea_location_code_count));
	ea_byte_index++;

	if (ea_msg.ea_location_code_count)
	{
		/* Get location codes */
		for (i=0; i < ea_msg.ea_location_code_count; i++)
		{
			ea_msg.ea_location_code[i].state_code = SI_Construct_Data(ea_table, ea_byte_index, 1, 0, 0xff);
			ea_byte_index++;
			ea_msg.ea_location_code[i].county_subdevision = SI_Construct_Data(ea_table, ea_byte_index, 1, 4, 0x0f);
			ea_msg.ea_location_code[i].county_code = SI_Construct_Data(ea_table, ea_byte_index, 2, 0, 0x03ff);
			ea_byte_index += 2;
			SI_DBG_PRINT(E_SI_DBG_MSG,("  Location code %d state_code = %x\n", i, ea_msg.ea_location_code[i].state_code));
			SI_DBG_PRINT(E_SI_DBG_MSG,("  Location code %d county_subdevision = %x\n", i, ea_msg.ea_location_code[i].county_subdevision));
			SI_DBG_PRINT(E_SI_DBG_MSG,("  Location code %d county_code = %x\n", i, ea_msg.ea_location_code[i].county_code));
		}
	}

	/*
	 * Exceptions
	 */
	ea_msg.ea_exception_count = SI_Construct_Data(ea_table, ea_byte_index, 1, 0, 0xff);
	SI_DBG_PRINT(E_SI_DBG_MSG,("Exception count = %x\n", ea_msg.ea_exception_count));
	ea_byte_index++;
	if (ea_msg.ea_exception_count)
	{
		temp = SI_Construct_Data(ea_table, ea_byte_index, 1, 0, 0xff);
		ea_byte_index++;
		if (temp & 0x80)			/* if inband reference */
		{
			for (i=0; i < ea_msg.ea_exception_count; i++)
			{
				ea_msg.ea_exception_major_chan_no[i] = SI_Construct_Data(ea_table, ea_byte_index, 2, 0, 0x03ff);
				ea_byte_index += 2;
				ea_msg.ea_exception_minor_chan_no[i] = SI_Construct_Data(ea_table, ea_byte_index, 2, 0, 0x03ff);
				ea_byte_index += 2;
			}
		}
		else						/* out of band */
		{
			for (i=0; i < ea_msg.ea_exception_count; i++)
			{
				ea_byte_index += 2;
				ea_msg.ea_exception_oob_source_id[i] = SI_Construct_Data(ea_table, ea_byte_index, 2, 0, 0xffff);
				ea_byte_index += 2;
			}
		}
	}

	/*
	 * Descriptors
	 */
	ea_descriptors_length = SI_Construct_Data(ea_table, ea_byte_index, 2, 0, 0x03ff);
	SI_DBG_PRINT(E_SI_DBG_MSG,("Descriptors length = %x\n", ea_descriptors_length));
	ea_byte_index += 2;
	if (ea_descriptors_length)
	{
		unsigned char	desc_tag, desc_length;
		unsigned long	company_id;

		desc_tag = ea_table[ea_byte_index++];
		if (desc_tag >= 0xc0)
		{
			/*
			 * User private descriptor
			 */
			desc_length = ea_table[ea_byte_index++];
			company_id = SI_Construct_Data(ea_table, ea_byte_index, 3, 0, 0xffffff);
			ea_byte_index += 3;
			for (i=0; i < desc_length; i++)
			{
				/* private data */
			}
		}
		else
		{
			/*
			 * Standard descriptor
			 */
		}
	}

	/*
	 * Call application callback to let them know an EA message has been received
	 */
	if (EA_message_received_cb)
		(EA_message_received_cb)(&ea_msg);

	return SI_SUCCESS;
}

