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

#ifndef SI_DESCRIPTORS_H
#define SI_DESCRIPTORS_H

/* defines for all descriptor tags. */
/* For the following, refer to table 6.1 of ANSI/SCTE65 2002 (DVS234) */
#define SI_DESC_STUFFING		0x80
#define SI_DESC_AC3_AUDIO		0x81
#define SI_DESC_CAPTION_SERVICE		0x86
#define SI_DESC_CONTENT_ADVISORY	0x87
#define SI_DESC_REVISION_DETECTION	0x93
#define SI_DESC_TWO_PART_CHANNEL_NO	0x94
#define SI_DESC_CHANNEL_PROPERTIES	0x95
#define SI_DESC_DAYLIGHT_SAVINGS_TIME	0x96
#define SI_DESC_EXTENDED_CHANNEL_NAME	0xa0
#define SI_DESC_SERVICE_LOCATION		0xa1
#define SI_DESC_TIME_SHIFTED_SERVICE	0xa2
#define SI_DESC_COMPONENT_NAME		0xa3
#define SI_DESC_USER_PRIVATE		0xc0

/* Daylight Savings Time descriptor. */
/* For the following, refer to table 6.10 of ANSI/SCTE65 2002 (DVS234) */
#define DESC_DST_DS_STATUS_LEN			2

#define DESC_DST_DS_STATUS_BYTE_INDEX    2
#define DESC_DST_DS_STATUS_BYTE_NUM    1
#define DESC_DST_DS_STATUS_SHIFT    15
#define DESC_DST_DS_STATUS_MASK    0x01

#define DESC_DST_DS_DAY_BYTE_INDEX    2
#define DESC_DST_DS_DAY_BYTE_NUM    1
#define DESC_DST_DS_DAY_SHIFT    15
#define DESC_DST_DS_DAY_MASK    0x01

#define DESC_DST_DS_HOUR_BYTE_INDEX    3
#define DESC_DST_DS_HOUR_BYTE_NUM    1
#define DESC_DST_DS_HOUR_SHIFT    0
#define DESC_DST_DS_HOUR_MASK    0xff

/* revision dectection descriptor. */
/* For the following, refer to table 6.4 of ANSI/SCTE65 2002 (DVS234) */
#define DESC_REV_DECTECT_LEN			3

#define DESC_REV_DECTECT_VER_NUM_BYTE_INDEX		0
#define DESC_REV_DECTECT_VER_NUM_BYTE_NUM		1
#define DESC_REV_DECTECT_VER_NUM_SHIFT		0
#define DESC_REV_DECTECT_VER_NUM_MASK		0x1f

#define DESC_REV_DECTECT_SEC_NUM_BYTE_INDEX		1
#define DESC_REV_DECTECT_SEC_NUM_BYTE_NUM		1
#define DESC_REV_DECTECT_SEC_NUM_SHIFT		0
#define DESC_REV_DECTECT_SEC_NUM_MASK		0xff

#define DESC_REV_DECTECT_LAST_SEC_NUM_BYTE_INDEX		2
#define DESC_REV_DECTECT_LAST_SEC_NUM_BYTE_NUM		1
#define DESC_REV_DECTECT_LAST_SEC_NUM_SHIFT		0
#define DESC_REV_DECTECT_LAST_SEC_NUM_MASK		0xff

/* time shifted service descriptor. */
/* For the following, refer to table 6.8 of ANSI/SCTE65 2002 (DVS234) */
#define DESC_TSS_NUM_OF_SERV_BYTE_INDEX		0
#define DESC_TSS_NUM_OF_SERV_BYTE_NUM		1
#define DESC_TSS_NUM_OF_SERV_SHIFT		0
#define DESC_TSS_NUM_OF_SERV_MASK		0x1f

/* in service loop. relative offset. */
#define DESC_TSS_TIME_SHIFT_BYTE_INDEX		0
#define DESC_TSS_TIME_SHIFT_BYTE_NUM		2
#define DESC_TSS_TIME_SHIFT_SHIFT		0
#define DESC_TSS_TIME_SHIFT_MASK		0x3ff

#define DESC_TSS_MAJOR_NUM_BYTE_INDEX		2
#define DESC_TSS_MAJOR_NUM_BYTE_NUM		2
#define DESC_TSS_MAJOR_NUM_SHIFT		2
#define DESC_TSS_MAJOR_NUM_MASK		0x3ff

#define DESC_TSS_MINOR_NUM_BYTE_INDEX		3
#define DESC_TSS_MINOR_NUM_BYTE_NUM		2
#define DESC_TSS_MINOR_NUM_SHIFT		0
#define DESC_TSS_MINOR_NUM_MASK		0x3ff


/* two part channel number descriptor. */
/* For the following, refer to table 6.5 of ANSI/SCTE65 2002 (DVS234) */
#define DESC_TPCND_MAJOR_NUMBER_BYTE_INDEX		0
#define DESC_TPCND_MAJOR_NUMBER_BYTE_NUM		2
#define DESC_TPCND_MAJOR_NUMBER_SHIFT		0
#define DESC_TPCND_MAJOR_NUMBER_MASK		0x3ff

#define DESC_TPCND_MINOR_NUMBER_BYTE_INDEX		2
#define DESC_TPCND_MINOR_NUMBER_BYTE_NUM		2
#define DESC_TPCND_MINOR_NUMBER_SHIFT		0
#define DESC_TPCND_MINOR_NUMBER_MASK		0x3ff


/* channel properties descriptor. */
/* For the following, refer to table 6.6 of ANSI/SCTE65 2002 (DVS234) */
#define DESC_CPD_CHANNEL_TSID_BYTE_INDEX		0
#define DESC_CPD_CHANNEL_TSID_BYTE_NUM		2
#define DESC_CPD_CHANNEL_TSID_SHIFT		0
#define DESC_CPD_CHANNEL_TSID_MASK		0xffff

#define DESC_CPD_CHANNEL_BITS_BYTE_INDEX	2
#define DESC_CPD_CHANNEL_BITS_BYTE_NUM		2
#define DESC_CPD_CHANNEL_BITS_SHIFT		7
#define DESC_CPD_CHANNEL_BITS_MASK		0x7

#define DESC_CPD_SERVICE_TYPE_BYTE_INDEX	3
#define DESC_CPD_SERVICE_TYPE_BYTE_NUM		1
#define DESC_CPD_SERVICE_TYPE_SHIFT		0
#define DESC_CPD_SERVICE_TYPE_MASK		0x3f



#endif

