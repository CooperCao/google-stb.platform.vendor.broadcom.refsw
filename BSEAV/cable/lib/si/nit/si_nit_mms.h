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

#ifndef SI_NIT_MMS_H
#define SI_NIT_MMS_H

#define NIT_MMS_MAX_TABLE_SIZE	256

#define NIT_MMS_SUBTABLE_SIZE	6		/* size of MMS in bytes. */


/* For the following, refer to table 5.3 of ANSI/SCTE65 2002 (DVS234) */
#define NIT_MMS_TRANSMISSION_SYSTEM_BYTE_INDX    0
#define NIT_MMS_TRANSMISSION_SYSTEM_BYTE_NUM    1
#define NIT_MMS_TRANSMISSION_SYSTEM_SHIFT    4
#define NIT_MMS_TRANSMISSION_SYSTEM_MASK    0x0f

#define NIT_MMS_INNER_CODING_MODE_BYTE_INDX    0
#define NIT_MMS_INNER_CODING_MODE_BYTE_NUM    1
#define NIT_MMS_INNER_CODING_MODE_SHIFT    0
#define NIT_MMS_INNER_CODING_MODE_MASK    0x0f

#define NIT_MMS_SPLIT_BITSTREAM_MODE_BYTE_INDX    1
#define NIT_MMS_SPLIT_BITSTREAM_MODE_BYTE_NUM    1
#define NIT_MMS_SPLIT_BITSTREAM_MODE_SHIFT    7
#define NIT_MMS_SPLIT_BITSTREAM_MODE_MASK    0x01

#define NIT_MMS_MODULATION_FORMAT_BYTE_INDX    1
#define NIT_MMS_MODULATION_FORMAT_BYTE_NUM    1
#define NIT_MMS_MODULATION_FORMAT_SHIFT    0
#define NIT_MMS_MODULATION_FORMAT_MASK    0x1f

#define NIT_MMS_SYMBOL_RATE_BYTE_INDX    2
#define NIT_MMS_SYMBOL_RATE_BYTE_NUM    4
#define NIT_MMS_SYMBOL_RATE_SHIFT    0
#define NIT_MMS_SYMBOL_RATE_MASK    0x0fffffff


/* Transmission system define see table 5.7 of ANSI/SCTE65 2002 (DVS234) */
typedef enum
{
	UNKNOWN_TX_SYS = 0,
	RESERVED_ETSI_TX_SYS = 1,
	ITU_T_ANNEX_B_TX_SYS = 2,
	OTHER_TX_SYS = 3,
	ATSC_TX_SYS = 4,
	RESERVED_SAT1_TX_SYS = 5,
	RESERVED_SAT2_TX_SYS = 6,
	RESERVED_SAT3_TX_SYS = 7,
	RESERVED_SAT4_TX_SYS = 8,
	RESERVED_SAT5_TX_SYS = 9,
	RESERVED_SAT6_TX_SYS = 10,
	RESERVED_SAT7_TX_SYS = 11,
	RESERVED_SAT8_TX_SYS = 12,
	RESERVED_SAT9_TX_SYS = 13,
	RESERVED_SAT10_TX_SYS = 14,
	RESERVED_SAT11_TX_SYS = 15,
}NIT_MMS_TRANSMISSION_SYSTEM;

/* inner coding mode define, see table 5.8 of ANSI/SCTE65 2002 (DVS234) */
typedef enum
{
	RATE_5_11 = 0,
	RATE_1_2 = 1,
	RATE_RESERVED1 = 2,
	RATE_3_5 = 3,
	RATE_RESERVED2 = 4,
	RATE_2_3 = 5,
	RATE_RESERVED3 = 6,
	RATE_3_4 = 7,
	RATE_4_5 = 8,
	RATE_5_6 = 9,
	RATE_RESERVED4 = 10,
	RATE_7_8 = 11,
	RATE_RESERVED5 = 12,
	RATE_RESERVED6 = 13,
	RATE_RESERVED7 = 14,
	RATE_NONE = 15,
}NIT_MMS_INNER_CODING_MODE;

/* modulation format, see table 5.9 of ANSI/SCTE65 2002 (DVS234) */
typedef enum
{
	MOD_UNKNOWN,
	MOD_QPSK,
	MOD_BPSK,
	MOD_OQPSK,
	MOD_VSB8,
	MOD_VSB16,
	MOD_QAM16,
	MOD_QAM32,
	MOD_QAM64,
	MOD_QAM80,
	MOD_QAM96,
	MOD_QAM112,
	MOD_QAM128,
	MOD_QAM160,
	MOD_QAM192,
	MOD_QAM224,
	MOD_QAM256,
	MOD_QAM320,
	MOD_QAM384,
	MOD_QAM448,
	MOD_QAM512,
	MOD_QAM640,
	MOD_QAM768,
	MOD_QAM896,
	MOD_QAM1024,
	MOD_RESVERED1,
	MOD_RESVERED2,
	MOD_RESVERED3,
	MOD_RESVERED4,
	MOD_RESVERED5,
	MOD_RESVERED6,
	MOD_RESVERED7,
}NIT_MMS_MODULATION_FORMAT;

/* The MMS table we keep for reference of SVCT. refer to table 5.6 of ANSI/SCTE65 2002 (DVS234). */
typedef struct
{
	unsigned char transmission_system;
	unsigned char inner_coding_mode;
	unsigned char split_bitstream_mode;
	unsigned char modulation_format;
	unsigned long symbol_rate;
	unsigned char idx;
}NIT_MMS_RECORD;

#ifdef __cplusplus
extern "C" {
#endif

void SI_NIT_MMS_Init (void);
void SI_NIT_MMS_parse (unsigned char * mms_record, unsigned char idx);

#ifdef __cplusplus
}
#endif

#endif


