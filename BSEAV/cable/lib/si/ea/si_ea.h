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

#ifndef SI_EA_H
#define SI_EA_H

#define EA_TABLE_ID_BYTE_INDX    0
#define EA_TABLE_ID_BYTE_NUM    1
#define EA_TABLE_ID_SHIFT    0
#define EA_TABLE_ID_MASK    0xff

#define EA_SECTION_LENGTH_BYTE_INDX    1
#define EA_SECTION_LENGTH_BYTE_NUM    2
#define EA_SECTION_LENGTH_SHIFT    0
#define EA_SECTION_LENGTH_MASK    0x0fff

#define EA_TABLE_ID_EXTENSION_BYTE_INDX    3
#define EA_TABLE_ID_EXTENSION_BYTE_NUM    2
#define EA_TABLE_ID_EXTENSION_SHIFT    0
#define EA_TABLE_ID_EXTENSION_MASK    0xffff

#define EA_SEQUENCE_NUMBER_BYTE_INDX    5
#define EA_SEQUENCE_NUMBER_BYTE_NUM    1
#define EA_SEQUENCE_NUMBER_SHIFT    1
#define EA_SEQUENCE_NUMBER_MASK    0x1f

#define EA_PROTOCOL_VERSION_BYTE_INDX    8
#define EA_PROTOCOL_VERSION_BYTE_NUM    1
#define EA_PROTOCOL_VERSION_SHIFT    0
#define EA_PROTOCOL_VERSION_MASK    0xff

#define EA_EAS_EVENT_ID_BYTE_INDX    9
#define EA_EAS_EVENT_ID_BYTE_NUM    2
#define EA_EAS_EVENT_ID_SHIFT    0
#define EA_EAS_EVENT_ID_MASK    0xffff

#define EA_EAS_ORIGINATOR_CODE_BYTE_INDX		11

#define EA_EAS_EVENT_CODE_LENGTH_BYTE_INDX    14
#define EA_EAS_EVENT_CODE_LENGTH_BYTE_NUM    1
#define EA_EAS_EVENT_CODE_LENGTH_SHIFT    0
#define EA_EAS_EVENT_CODE_LENGTH_MASK    0xff

#define EA_EAS_EVENT_CODE_BYTE_INDX    15

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Public function prototypes
 */
void SI_EA_Init (void (*cb));
void SI_EA_UnInit (void);
void SI_EA_GetEventTime (unsigned long *p_ea_start_time, unsigned short *p_ea_duration);
void SI_EA_EventExpirationCb (void);
SI_RET_CODE SI_EA_Parse (unsigned char * ea_table);

#ifdef __cplusplus
}
#endif



#endif
