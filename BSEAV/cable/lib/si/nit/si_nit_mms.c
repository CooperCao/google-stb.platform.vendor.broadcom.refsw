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
#include "si_nit_mms.h"

NIT_MMS_RECORD NIT_MMS_Table[NIT_MMS_MAX_TABLE_SIZE];  
unsigned long Total_NIT_MMS_entries =0;

void SI_NIT_MMS_Init (void)
{
	SI_memset(NIT_MMS_Table, 0, sizeof(NIT_MMS_Table));
	Total_NIT_MMS_entries =0;
}

/*********************************************************************/
/* Function : SI_NIT_MMS_parse										 */
/* Description : Function to parse the MMS subtable and populate the */
/*               carrier freq table. 								 */
/* Input : mms_record, point to the current ith MMS subtable         */
/*            defined in NIT table section. There may be up to       */
/*            number_of_record MMS subtables defined in NIT table    */
/*            section. See table 5.1 of ANSI/SCTE65 2002 (DVS234).	 */
/*         idx, point to the location where this MMS will be         */
/*			  located in the overall MMS table.	  					 */
/* Output : none													 */
/*********************************************************************/
void SI_NIT_MMS_parse (unsigned char * mms_record, unsigned char idx)
{
	unsigned long temp;

	SI_DBG_PRINT(E_SI_DBG_MSG,("NIT MMS Table received.\n"));

	NIT_MMS_Table[idx].transmission_system = SI_Construct_Data(mms_record, 
						NIT_MMS_TRANSMISSION_SYSTEM_BYTE_INDX,
						NIT_MMS_TRANSMISSION_SYSTEM_BYTE_NUM,
						NIT_MMS_TRANSMISSION_SYSTEM_SHIFT,
						NIT_MMS_TRANSMISSION_SYSTEM_MASK);
								
	NIT_MMS_Table[idx].inner_coding_mode = SI_Construct_Data(mms_record, 
						NIT_MMS_INNER_CODING_MODE_BYTE_INDX,
						NIT_MMS_INNER_CODING_MODE_BYTE_NUM,
						NIT_MMS_INNER_CODING_MODE_SHIFT,
						NIT_MMS_INNER_CODING_MODE_MASK);
						
	NIT_MMS_Table[idx].split_bitstream_mode = SI_Construct_Data(mms_record, 
						NIT_MMS_SPLIT_BITSTREAM_MODE_BYTE_INDX,
						NIT_MMS_SPLIT_BITSTREAM_MODE_BYTE_NUM,
						NIT_MMS_SPLIT_BITSTREAM_MODE_SHIFT,
						NIT_MMS_SPLIT_BITSTREAM_MODE_MASK);
						
	NIT_MMS_Table[idx].modulation_format = SI_Construct_Data(mms_record, 
						NIT_MMS_MODULATION_FORMAT_BYTE_INDX,
						NIT_MMS_MODULATION_FORMAT_BYTE_NUM,
						NIT_MMS_MODULATION_FORMAT_SHIFT,
						NIT_MMS_MODULATION_FORMAT_MASK);
						
	NIT_MMS_Table[idx].symbol_rate = SI_Construct_Data(mms_record, 
						NIT_MMS_SYMBOL_RATE_BYTE_INDX,
						NIT_MMS_SYMBOL_RATE_BYTE_NUM,
						NIT_MMS_SYMBOL_RATE_SHIFT,
						NIT_MMS_SYMBOL_RATE_MASK);
	
	NIT_MMS_Table[idx].idx = idx;
	Total_NIT_MMS_entries++;
	SI_DBG_PRINT(E_SI_DBG_MSG,("NIT MMS Entry: index %x, xmit sys  %x, inner code %x, mod %x, symb rate %x symb/s\n", idx,NIT_MMS_Table[idx].transmission_system,NIT_MMS_Table[idx].inner_coding_mode,NIT_MMS_Table[idx].modulation_format,NIT_MMS_Table[idx].symbol_rate));
}
