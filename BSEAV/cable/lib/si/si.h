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

#ifndef SI_H
#define SI_H

#include "bstd.h"

#define SI_NIT_TABLE_ID 0xc2
#define SI_NTT_TABLE_ID 0xc3
#define SI_SVCT_TABLE_ID 0xc4
#define SI_STT_TABLE_ID 0xc5
#define SI_MGT_TABLE_ID 0xc7
#define SI_LVCT_TABLE_ID 0xc9
#define SI_TVCT_TABLE_ID 0xc8
#define SI_RRT_TABLE_ID 0xca
#define SI_AEIT_TABLE_ID	0xd6
#define SI_AETT_TABLE_ID	0xd7
#define SI_EA_TABLE_ID	0xd8

#define SI_CURRENT_PROTOCOL_VERSION 0x00
#define SI_CURRENT_AEIT_SUBTYPE  0x00
#define SI_CURRENT_AETT_SUBTYPE  0x00

#define SI_NORMAL_SECTION_LENGTH	1024    /* 1024  */
#define SI_LONG_SECTION_LENGTH	4096    /* 4096 */

#define SI_CRC_LENGTH		4

/* AEIT0-1/AETT0-1, AEIT2-3/AETT2-3, AEIT4-255/AETT4-255. */
#define SI_MAX_AEIT_AETT_NUM_PID	3

#define MAX_PAST_AEXT_N		8    /* keep 24hr of history. */

typedef enum
{
	SI_SUCCESS,
	SI_WARNING,
	SI_TABLE_ID_ERROR,
	SI_PROTOCOL_VER_ERROR,
	SI_SECTION_LENGTH_ERROR,
	SI_SECTION_NUMBER_ERROR,
	SI_CURRENT_NEXT_INDICATOR_ERROR,
	SI_DESCRIPTOR_ERROR,
	SI_CRC_ERROR,
	SI_APP_ERROR,
	SI_NO_MEMORY,
	SI_NULL_POINTER,
	SI_AEIT_LIST_ERROR,
	SI_AETT_LIST_ERROR,
	SI_AEIT_LIST_NOT_READY,
	SI_AETT_LIST_NOT_READY,
	SI_OTHER_ERROR
}SI_RET_CODE;



#endif
