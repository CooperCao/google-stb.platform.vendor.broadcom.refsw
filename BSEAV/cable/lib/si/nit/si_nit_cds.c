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
#include "si_nit_cds.h"

/* the carrier frequency table as defined by one or more CDS subtable in NIT */
/* later in SVCT, the CDS__reference should be the index for this table to find */
/* a particular carrier frequency for a Virtual channel. */
/* the frequency is described in KHz unit. */
unsigned long NIT_CDS_Freq_KHZ_Table[NIT_CDS_MAX_NUM_OF_CARRIRE_INDEX];
unsigned long Total_NIT_CDS_carriers = 0;

void SI_NIT_CDS_Init (void)
{
	SI_memset(NIT_CDS_Freq_KHZ_Table, 0, sizeof(NIT_CDS_Freq_KHZ_Table));
	Total_NIT_CDS_carriers = 0;
}

/*********************************************************************/
/* Function : SI_NIT_CDS_parse										 */
/* Description : Function to parse the CDS subtable and populate the */
/*               carrier freq table. 								 */
/* Input : cds_record, point to the current ith CDS subtable         */
/*            defined in NIT table section. There may be up to       */
/*            number_of_record CDS subtables defined in NIT table    */
/*            section. See table 5.1 of DVS234.						 */
/*         idx, point to the carrier index where the carriers        */
/*			  defined in this CDS will be located. 					 */
/* Output : The number of carriers defined in this CDS. It can be    */
/* 			used to increment the first_index in the NIT table  to   */
/* 			be passed as idx of this function.						 */
/*********************************************************************/
unsigned char SI_NIT_CDS_parse (unsigned char * cds_record, unsigned char idx)
{
	unsigned long num_of_carriers;
	unsigned long frequency_spacing;
	unsigned long first_carrier_frequency;
	unsigned long unit, unitnum;
	long	i;

	SI_DBG_PRINT(E_SI_DBG_MSG,("NIT CDS Table received.\n"));

	num_of_carriers = SI_Construct_Data(cds_record,
						NIT_CDS_NUMBER_OF_CARRIERS_BYTE_INDX,
						NIT_CDS_NUMBER_OF_CARRIERS_BYTE_NUM,
						NIT_CDS_NUMBER_OF_CARRIERS_SHIFT,
						NIT_CDS_NUMBER_OF_CARRIERS_MASK);

	unit = SI_Construct_Data(cds_record,
			NIT_CDS_SPACING_UNIT_BYTE_INDX,
			NIT_CDS_SPACING_UNIT_BYTE_NUM,
			NIT_CDS_SPACING_UNIT_BYTE_SHIFT,
			NIT_CDS_SPACING_UNIT_BYTE_MASK);
	unitnum = (unit?NIT_CDS_SPACING_UNIT1:NIT_CDS_SPACING_UNIT0);
	frequency_spacing = SI_Construct_Data(cds_record,
			NIT_CDS_FREQUENCY_SPACING_BYTE_INDX,
			NIT_CDS_FREQUENCY_SPACING_BYTE_NUM,
			NIT_CDS_FREQUENCY_SPACING_BYTE_SHIFT,
			NIT_CDS_FREQUENCY_SPACING_BYTE_MASK);
	SI_DBG_PRINT(E_SI_DBG_MSG,("freq space %x, unit %d kHz, real space %d\n", frequency_spacing, unitnum, frequency_spacing*unitnum));		
	frequency_spacing *= unitnum;

	unit = SI_Construct_Data(cds_record,
			NIT_CDS_FREQUENCY_UNIT_BYTE_INDX,
			NIT_CDS_FREQUENCY_UNIT_BYTE_NUM,
			NIT_CDS_FREQUENCY_UNIT_BYTE_SHIFT,
			NIT_CDS_FREQUENCY_UNIT_BYTE_MASK);
	unitnum = (unit?NIT_CDS_FREQUENCY_UNIT1:NIT_CDS_FREQUENCY_UNIT0);

	first_carrier_frequency = SI_Construct_Data(cds_record,
			NIT_CDS_FIRST_CARRIER_FREQUENCY_BYTE_INDX,
			NIT_CDS_FIRST_CARRIER_FREQUENCY_BYTE_NUM,
			NIT_CDS_FIRST_CARRIER_FREQUENCY_BYTE_SHIFT,
			NIT_CDS_FIRST_CARRIER_FREQUENCY_BYTE_MASK);
	SI_DBG_PRINT(E_SI_DBG_MSG,("1st freq %x, unit %d kHz, real freq %d\n", first_carrier_frequency, unitnum, first_carrier_frequency*unitnum));		
	first_carrier_frequency *= unitnum;

	SI_DBG_PRINT(E_SI_DBG_MSG,("NIT CDS num of carriers %x, spacing %d kHz, first freq %d kHz.\n", num_of_carriers, frequency_spacing, first_carrier_frequency));
	SI_DBG_PRINT(E_SI_DBG_MSG,("NIT CDS Actual Freqs defined (from index %x):\n", idx));
	for (i=0; i<num_of_carriers; i++)
	{
		NIT_CDS_Freq_KHZ_Table[idx+i] = first_carrier_frequency + i*frequency_spacing;
		SI_DBG_PRINT(E_SI_DBG_MSG,("%d kHz, ", NIT_CDS_Freq_KHZ_Table[idx+i]));
	}
	SI_DBG_PRINT(E_SI_DBG_MSG,("\n"));

	Total_NIT_CDS_carriers = num_of_carriers;
	return (unsigned char)num_of_carriers;
}

