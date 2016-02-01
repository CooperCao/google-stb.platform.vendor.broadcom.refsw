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

#ifndef POD_API_EA_H
#define POD_API_EA_H

#ifdef __cplusplus
extern "C" {
#endif

#define EA_TEST_MSG_PRIORITY			0
#define EA_LOW_PRIORITY					3
#define EA_MEDIUM_PRIORITY				7
#define EA_HIGH_PRIORITY				11
#define EA_MAX_PRIORITY					15

#define EA_MAX_LOCATION_CODE_COUNT		31

typedef struct ea_location_code_struct
{
	unsigned char	state_code;
	unsigned char	county_subdevision;
	unsigned short	county_code;
} EA_LOCATION_CODE;

typedef struct ea_msg_info_struct
{
	/* Private information */
	unsigned char	process:1;
	unsigned char	in_progress:1;

	/* Info parsed from EA message */
	unsigned short	eas_event_id;
	char			eas_originator_code[3];
	char			eas_event_code[10];				/* NULL-terminated string */
	char			nature_of_act_text[100];		/* NULL-terminated string */
	unsigned char	ea_alert_message_time_rem;
	unsigned long	ea_event_start_time;
	unsigned short	ea_event_duration;
	unsigned char	ea_alert_priority;
	unsigned short	ea_details_oob_source_id;
	unsigned short	ea_details_major_chan_number;
	unsigned short	ea_details_minor_chan_number;
	unsigned short	ea_audio_oob_source_id;
	char			ea_alert_text[65536];			/* NULL-terminated string */
	unsigned char	ea_location_code_count;
	EA_LOCATION_CODE ea_location_code[EA_MAX_LOCATION_CODE_COUNT];
	unsigned char	ea_exception_count;
	unsigned short	ea_exception_major_chan_no[256];
	unsigned short	ea_exception_minor_chan_no[256];
	unsigned short	ea_exception_oob_source_id[256];
} EA_MSG_INFO;


#ifdef __cplusplus
}
#endif

#endif

