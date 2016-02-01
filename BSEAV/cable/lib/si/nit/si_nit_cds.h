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

#ifndef SI_NIT_CDS_H
#define SI_NIT_CDS_H


#define NIT_CDS_MAX_NUM_OF_CARRIRE_INDEX	256    /* p16 of ANSI/SCTE65 2002 (DVS234) */

#define NIT_CDS_SPACING_UNIT0	10			/* in KHz */
#define NIT_CDS_SPACING_UNIT1	125			/* in KHz */

#define NIT_CDS_FREQUENCY_UNIT0	10			/* in KHz */
#define NIT_CDS_FREQUENCY_UNIT1	125			/* in KHz */

#define NIT_CDS_SUBTABLE_SIZE	5		/* size of CDS in bytes. */

/* For the following, refer to table 5.3 of ANSI/SCTE65 2002 (DVS234) */
#define NIT_CDS_NUMBER_OF_CARRIERS_BYTE_INDX    0
#define NIT_CDS_NUMBER_OF_CARRIERS_BYTE_NUM    1
#define NIT_CDS_NUMBER_OF_CARRIERS_SHIFT    0
#define NIT_CDS_NUMBER_OF_CARRIERS_MASK    0xff

#define NIT_CDS_SPACING_UNIT_BYTE_INDX    1
#define NIT_CDS_SPACING_UNIT_BYTE_NUM    1
#define NIT_CDS_SPACING_UNIT_BYTE_SHIFT    7
#define NIT_CDS_SPACING_UNIT_BYTE_MASK    0x01

#define NIT_CDS_FREQUENCY_SPACING_BYTE_INDX    1
#define NIT_CDS_FREQUENCY_SPACING_BYTE_NUM    2
#define NIT_CDS_FREQUENCY_SPACING_BYTE_SHIFT    0
#define NIT_CDS_FREQUENCY_SPACING_BYTE_MASK    0x3fff

#define NIT_CDS_FREQUENCY_UNIT_BYTE_INDX    3
#define NIT_CDS_FREQUENCY_UNIT_BYTE_NUM    1
#define NIT_CDS_FREQUENCY_UNIT_BYTE_SHIFT    7
#define NIT_CDS_FREQUENCY_UNIT_BYTE_MASK    0x01

#define NIT_CDS_FIRST_CARRIER_FREQUENCY_BYTE_INDX    3
#define NIT_CDS_FIRST_CARRIER_FREQUENCY_BYTE_NUM    2
#define NIT_CDS_FIRST_CARRIER_FREQUENCY_BYTE_SHIFT    0
#define NIT_CDS_FIRST_CARRIER_FREQUENCY_BYTE_MASK    0x7fff

#ifdef __cplusplus
extern "C" {
#endif

void SI_NIT_CDS_Init (void);
unsigned char SI_NIT_CDS_parse (unsigned char * cds_record, unsigned char idx);

#ifdef __cplusplus
}
#endif

#endif

