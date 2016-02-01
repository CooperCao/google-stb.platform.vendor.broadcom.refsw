/***************************************************************************
 *     Copyright (c) 2002-2010, Broadcom Corporation
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
 * Module Description: Software startcode index generator
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ****************************************************************/
 
#ifndef BCMSCT_INDEX_H__
#define BCMSCT_INDEX_H__

#include "bstd.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Only upper byte of recordByteCountHi is used to store the MSB of the offset */
#define REC_BYTE_COUNT_HI_OFFSET    (24)
#define REC_BYTE_COUNT_HI_MASK      (0x00FFFFFF)

/**
* Summary:
* Data structure for 4 word start code table (SCT) index entry. 
*
* Description:
* In practice, you don't need to know anything about the interals of this structure. Just read data
* from the Start Code Detect (SCD) hardware in chunks equal to sizeof(BSCT_Entry). 
* Bcmindexer will parse it and produce the Broadcom index.
**/
typedef struct
{
	uint32_t startCodeBytes;		/* 31:24 Start Code, 23:8 Two Bytes after SC, 7:0 Byte offset into trans packet */
	uint32_t recordByteCount;	/* Byte offset from start of record to start of trans packet */
	uint32_t recordByteCountHi;	/* 31:24 MSB Record Offset, 23:0 reserved */
	uint32_t flags;				/* various flags */
} BSCT_Entry;

/**
Summary:
Structure on a 6 word SCT entry.
**/
typedef struct BSCT_SixWord_Entry
{
	uint32_t word0;				/* 31:24 SC_ENTRY_TYPE, 20:08 TF_PID */
	uint32_t word1;				/* 31:0 reserved */
	uint32_t startCodeBytes;		/* 31:24 Start Code, 23:8 Two Bytes after SC, 7:0 Byte offset into trans packet */
	uint32_t recordByteCount;	/* Byte offset from start of record to start of trans packet */
	uint32_t recordByteCountHi;	/* 31:24 MSB Record Offset, 
		23:0 additional ES payload data, bytes 2, 3 and 4 */
	uint32_t flags;				/* 31:24 flags, 
		23:0 additional ES payload data, bytes 5, 6 and 7 */
} BSCT_SixWord_Entry;

#ifdef __cplusplus
}
#endif

#endif /* BCMSCT_INDEX_H__ */
