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


#ifndef SI_LVCT_H
#define SI_LVCT_H

#include "si_main.h"

#define LVCT_ACCESS_CONTROLED_MASK		0x10
#define LVCT_HIDDEN_MASK		0x08
#define LVCT_PATH_SELECT_MASK		0x04
#define LVCT_OUT_OF_BAND_MASK		0x02
#define LVCT_HIDE_GUIDE_MASK		0x01

#define LVCT_SHORT_NAME_LENGTH			7
#define LVCT_LONG_NAME_LENGTH			128
typedef enum
{
	LVCT_MM_ANALOG = 0x1,			/* Analog TV. */
	LVCT_MM_SCTE1 = 0x2,			/* SCTE mode 1 typically QAM-64. */
	LVCT_MM_SCTE2 = 0x3,			/* SCTE mode 2 typically QAM-256. */
	LVCT_MM_8_VSB = 0x4, 			/* ATSC 8 VSB. */
	LVCT_MM_16_VSB = 0x5, 			/* ATSC 16 VSB. */
} LVCT_MOD_MODE;					/* modulation mode. */

/* time shifted service structure, carried in time shifted descriptor. */
typedef struct
{
	unsigned short time_shift;		/* time shift in seconds. */
	VirtChanNumMode vcn_mode; 		/* virtual channel number scheme. 0 onpart, 1 twopart. */
	unsigned short channum1; 		/* for one part mode, this is the virtual channel number. for two part, this will be the major part. */
	unsigned short channum2;		/* for one part mode, this is not used. for two part, this is the minor part. */
} TIME_SHIFT_SERV;



typedef struct _SI_LVCT_CHANNEL
{
	VirtChanNumMode vcn_mode; 		/* virtual channel number scheme. 0 onpart, 1 twopart. */
	unsigned short channum1; 		/* for one part mode, this is the virtual channel number. for two part, this will be the major part. */
	unsigned short channum2;		/* for one part mode, this is not used. for two part, this is the minor part. */
	unsigned short source_ID;		/* source_ID corresponding to the channel. */
	unsigned short short_name[LVCT_SHORT_NAME_LENGTH];	/* 7 unicode characters. */
	unsigned short tsid;			/* transport stream id for this channel. */
	unsigned long carrier_freq;		/* in Hz. */
	unsigned char mod_mode;
	unsigned short program_num;		/* program number for digital channel. 0xffff for analog channel. 0 for inactive channel. */
	unsigned char serv_type;	/* service type. */
	unsigned char chanbits;			/* includes masks for access_controled, hidden, path_select, out_of_band, hide_guide. */
	unsigned char long_name[LVCT_LONG_NAME_LENGTH];	/* extended channel name */
	unsigned char num_of_ts_serv;		/* number of time shifted services. */
	TIME_SHIFT_SERV * time_shifted;  /* time shifted service structure. */
	PROGRAM_INFO_T program_info;    /* PID information */
	SI_LST_D_ENTRY(_SI_LVCT_CHANNEL) chan_link;
} SI_LVCT_CHANNEL;

SI_LST_D_HEAD(lvct_channel_list, _SI_LVCT_CHANNEL);

/* For the following, refer to table 5.26 of ANSI/SCTE65 2002 (DVS234) */
/* for table header. */
#define LVCT_TABLE_ID_BYTE_INDX		0
#define LVCT_TABLE_ID_BYTE_NUM			1
#define LVCT_TABLE_ID_SHIFT			0
#define LVCT_TABLE_ID_MASK				0xff

#define LVCT_SECTION_LENGTH_BYTE_INDX    1
#define LVCT_SECTION_LENGTH_BYTE_NUM    2
#define LVCT_SECTION_LENGTH_SHIFT    0
#define LVCT_SECTION_LENGTH_MASK    0x0fff

#define LVCT_VERSION_NUMBER_BYTE_INDX    5
#define LVCT_VERSION_NUMBER_BYTE_NUM    1
#define LVCT_VERSION_NUMBER_SHIFT    1
#define LVCT_VERSION_NUMBER_MASK    0x1f

#define LVCT_CURRENT_NEXT_INDICATOR_BYTE_INDX    5
#define LVCT_CURRENT_NEXT_INDICATOR_BYTE_NUM    1
#define LVCT_CURRENT_NEXT_INDICATOR_SHIFT    0
#define LVCT_CURRENT_NEXT_INDICATOR_MASK    0x01

#define LVCT_SECTION_NUMBER_BYTE_INDX    6
#define LVCT_SECTION_NUMBER_BYTE_NUM    1
#define LVCT_SECTION_NUMBER_SHIFT    0
#define LVCT_SECTION_NUMBER_MASK    0xff

#define LVCT_LAST_SECTION_NUMBER_BYTE_INDX    7
#define LVCT_LAST_SECTION_NUMBER_BYTE_NUM    1
#define LVCT_LAST_SECTION_NUMBER_SHIFT    0
#define LVCT_LAST_SECTION_NUMBER_MASK    0xff

#define LVCT_PROTOCOL_VERSION_BYTE_INDX    8
#define LVCT_PROTOCOL_VERSION_BYTE_NUM    1
#define LVCT_PROTOCOL_VERSION_SHIFT    0
#define LVCT_PROTOCOL_VERSION_MASK    0xff

#define LVCT_NUM_CHANNELS_BYTE_INDX    9
#define LVCT_NUM_CHANNELS_BYTE_NUM    1
#define LVCT_NUM_CHANNELS_SHIFT    0
#define LVCT_NUM_CHANNELS_MASK    0xff

/* for inside channel loop. relative offset after the short name only. */
#define LVCT_MAJOR_NUMBER_BYTE_INDX		0
#define LVCT_MAJOR_NUMBER_BYTE_NUM			2
#define LVCT_MAJOR_NUMBER_SHIFT			2
#define LVCT_MAJOR_NUMBER_MASK				0x3ff

#define LVCT_MINOR_NUMBER_BYTE_INDX		1
#define LVCT_MINOR_NUMBER_BYTE_NUM			2
#define LVCT_MINOR_NUMBER_SHIFT			0
#define LVCT_MINOR_NUMBER_MASK				0x3ff

#define LVCT_MODULATION_MODE_BYTE_INDX		3
#define LVCT_MODULATION_MODE_BYTE_NUM			1
#define LVCT_MODULATION_MODE_SHIFT			0
#define LVCT_MODULATION_MODE_MASK				0xff

#define LVCT_CARRIER_FREQUENCY_BYTE_INDX		4
#define LVCT_CARRIER_FREQUENCY_BYTE_NUM			4
#define LVCT_CARRIER_FREQUENCY_SHIFT			0
#define LVCT_CARRIER_FREQUENCY_MASK				0xffffffff

#define LVCT_CHANNEL_TSID_BYTE_INDX			8
#define LVCT_CHANNEL_TSID_BYTE_NUM			2
#define LVCT_CHANNEL_TSID_SHIFT			0
#define LVCT_CHANNEL_TSID_MASK				0xffff

#define LVCT_PROGRAM_NUMBER_BYTE_INDX			10
#define LVCT_PROGRAM_NUMBER_BYTE_NUM			2
#define LVCT_PROGRAM_NUMBER_SHIFT			0
#define LVCT_PROGRAM_NUMBER_MASK				0xffff

#define LVCT_CHANNEL_BITS_BYTE_INDX			12
#define LVCT_CHANNEL_BITS_BYTE_NUM			1
#define LVCT_CHANNEL_BITS_SHIFT			1
#define LVCT_CHANNEL_BITS_MASK				0x1f

#define LVCT_SERVICE_TYPE_BYTE_INDX			13
#define LVCT_SERVICE_TYPE_BYTE_NUM			1
#define LVCT_SERVICE_TYPE_SHIFT			0
#define LVCT_SERVICE_TYPE_MASK				0x3f

#define LVCT_SOURCE_ID_BYTE_INDX			14
#define LVCT_SOURCE_ID_BYTE_NUM			2
#define LVCT_SOURCE_ID_SHIFT			0
#define LVCT_SOURCE_ID_MASK				0xffff

#define LVCT_DESC_LENGTH_BYTE_INDX			16
#define LVCT_DESC_LENGTH_BYTE_NUM			2
#define LVCT_DESC_LENGTH_SHIFT			0
#define LVCT_DESC_LENGTH_MASK				0x3ff

/* after the channel loop. */
#define LVCT_ADD_DESC_LENGTH_BYTE_INDX			0
#define LVCT_ADD_DESC_LENGTH_BYTE_NUM			2
#define LVCT_ADD_DESC_LENGTH_SHIFT			0
#define LVCT_ADD_DESC_LENGTH_MASK				0x3ff



#ifdef __cplusplus
extern "C" {
#endif

void SI_LVCT_Init(void);
SI_RET_CODE SI_LVCT_Parse (unsigned char *lvct_table,  VCT_MODE mode);
SI_RET_CODE SI_LVCT_Free_List(void);
void SI_LVCT_Get(unsigned char *version_number,
				unsigned long **p_section_mask
				);
SI_RET_CODE SI_LVCT_Ins_Channel(SI_LVCT_CHANNEL *new_channel);
SI_LVCT_CHANNEL * SI_LVCT_Create_Channel (void);
#ifdef __cplusplus
}
#endif


#endif
