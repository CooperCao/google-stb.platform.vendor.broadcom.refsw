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
#include "si_stt.h"
#include "si_descriptors.h"

unsigned long SI_UTC_Time;
unsigned long SI_GPS_UTC_OFFSET;
unsigned char SI_DS_status;
unsigned char SI_DS_day_of_month;
unsigned char SI_DS_hour;

static 	void (*STT_received_cb)(unsigned long , bool) = NULL;

void SI_STT_Init (void (*cb))
{
	SI_UTC_Time = 0;
	SI_GPS_UTC_OFFSET = 0;
	SI_DS_status = 0;
	SI_DS_day_of_month = 0;
	SI_DS_hour = 0;
	STT_received_cb = cb;
}

void SI_STT_UnInit (void)
{
	STT_received_cb = NULL;
}

/*********************************************************************
 Function : SI_STT_Get_Sys_Time
 Description : Function get the current the system
               UTC time.
 Input : None.
 Output : current system time.
**********************************************************************/
unsigned long SI_STT_Get_Sys_Time (void)
{
	return SI_UTC_Time;
}

/*********************************************************************
 Function : SI_STT_Get_GPS_UTC_Offset
 Description : Function get the current the system
               GPS UTC offset.
 Input : None.
 Output : current system time.
**********************************************************************/
unsigned long SI_STT_Get_GPS_UTC_Offset (void)
{
	return SI_GPS_UTC_OFFSET;
}

/*********************************************************************
 Function : SI_STT_Conv_To_UTC_Time
 Description : Function get convert the GPS time to the system
               UTC time.
 Input : unsigned long time: GPS_time.
 Output : converted to UTC time.
**********************************************************************/
unsigned long SI_STT_Conv_To_UTC_Time (unsigned long time)
{
	return (time - SI_GPS_UTC_OFFSET);
}


/*********************************************************************
 Function : SI_STT_parse
 Description : Function to parse the STT table and set the system
               UTC time.
 Input : stt_table, point to the STT table data.
 Output : Success code.
**********************************************************************/
SI_RET_CODE SI_STT_parse (unsigned char * stt_table)
{
	unsigned char temp;
	unsigned long section_length;
	unsigned long CRC_start;
	unsigned long desc_start = STT_GPS_UTC_OFFSET_BYTE_INDX+STT_GPS_UTC_OFFSET_BYTE_NUM;
	unsigned long desc_tag, desc_len;
	unsigned char *desc;

	SI_DBG_PRINT(E_SI_DBG_MSG,("STT Table received.\n"));

	temp = *stt_table;
	if (temp != SI_STT_TABLE_ID)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("STT Table ID error!!! %x\n", temp));
		return SI_TABLE_ID_ERROR;
	}

	section_length = SI_Construct_Data(stt_table,
				STT_SECTION_LENGTH_BYTE_INDX,
				STT_SECTION_LENGTH_BYTE_NUM,
				STT_SECTION_LENGTH_SHIFT,
				STT_SECTION_LENGTH_MASK);
	section_length += STT_SECTION_LENGTH_BYTE_INDX+STT_SECTION_LENGTH_BYTE_NUM;
	if (section_length > SI_NORMAL_SECTION_LENGTH)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("STT Table section length error!!! %x\n", section_length));
		return SI_SECTION_LENGTH_ERROR;
	}

	/* We do the CRC check here to verify the contents of this section. */
	if (SI_CRC32_Check(stt_table, section_length) != SI_SUCCESS)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("STT Table section CRC error!!!\n"));
		return SI_CRC_ERROR;
	}

	/* check to make sure protocol version is zero. */
	temp = SI_Construct_Data(stt_table,
				STT_PROTOCOL_VERSION_BYTE_INDX,
				STT_PROTOCOL_VERSION_BYTE_NUM,
				STT_PROTOCOL_VERSION_SHIFT,
				STT_PROTOCOL_VERSION_MASK);
	if (temp != SI_CURRENT_PROTOCOL_VERSION)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("STT Table PROTOCOL version error!!! %x\n", temp));
		return SI_PROTOCOL_VER_ERROR;
	}

	CRC_start = section_length-SI_CRC_LENGTH;

	SI_UTC_Time = SI_Construct_Data(stt_table,
				STT_SYSTEM_TIME_BYTE_INDX,
				STT_SYSTEM_TIME_BYTE_NUM,
				STT_SYSTEM_TIME_SHIFT,
				STT_SYSTEM_TIME_MASK);
	SI_GPS_UTC_OFFSET = SI_Construct_Data(stt_table,
				STT_GPS_UTC_OFFSET_BYTE_INDX,
				STT_GPS_UTC_OFFSET_BYTE_NUM,
				STT_GPS_UTC_OFFSET_SHIFT,
				STT_GPS_UTC_OFFSET_MASK);
	SI_UTC_Time -= SI_GPS_UTC_OFFSET;

	SI_DBG_PRINT(E_SI_DBG_MSG,("STT UTC time %x, GPS UTC offset %x.\n", SI_UTC_Time, SI_GPS_UTC_OFFSET));


	/* Now let's worry about descriptors. */
	while (desc_start < CRC_start)
	{
		desc = (stt_table + desc_start);
		desc_tag = desc[0];
		desc_len = desc[1];
		switch (desc_tag)
		{
			case SI_DESC_DAYLIGHT_SAVINGS_TIME:
				SI_DS_status = SI_Construct_Data(desc,
									DESC_DST_DS_STATUS_BYTE_INDEX,
									DESC_DST_DS_STATUS_BYTE_NUM,
									DESC_DST_DS_STATUS_SHIFT,
									DESC_DST_DS_STATUS_MASK);
				SI_DS_day_of_month = SI_Construct_Data(desc,
									DESC_DST_DS_DAY_BYTE_INDEX,
									DESC_DST_DS_DAY_BYTE_NUM,
									DESC_DST_DS_DAY_SHIFT,
									DESC_DST_DS_DAY_MASK);
				SI_DS_hour = SI_Construct_Data(desc,
									DESC_DST_DS_HOUR_BYTE_INDEX,
									DESC_DST_DS_HOUR_BYTE_NUM,
									DESC_DST_DS_HOUR_SHIFT,
									DESC_DST_DS_HOUR_MASK);

				SI_DBG_PRINT(E_SI_DBG_MSG,("STT Table have DS descriptor: Status %d, day %d, hr %d.\n", SI_DS_status, SI_DS_day_of_month, SI_DS_hour));
			break;

			case SI_DESC_STUFFING:
				SI_DBG_PRINT(E_SI_DBG_MSG,("STT Table have stuffing descriptor.\n"));
			break;

			default:
				if (desc_tag >= SI_DESC_USER_PRIVATE)
				{
					SI_DBG_PRINT(E_SI_WRN_MSG,("STT Table have user private descriptor. And we don't know what to do with it.\n"));
				}
				else
				{
					SI_DBG_PRINT(E_SI_ERR_MSG,("STT Table have bad descriptor %x!\n", desc_tag));
					return SI_DESCRIPTOR_ERROR;
				}
			break;
		}

		desc_start += desc_len+2; /* plus the tag and length. */
	}

	if (desc_start != CRC_start)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("STT Table descriptor length error!\n"));
		return SI_DESCRIPTOR_ERROR;
	}

	if (STT_received_cb)
		STT_received_cb( SI_UTC_Time ,(bool)SI_DS_status);
	return SI_SUCCESS;
}

