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
#include "si_nit_mms.h"
#include "si_nit.h"
#include "si_descriptors.h"

unsigned char NIT_Version_Number;
unsigned long NIT_Section_Mask[8];

void SI_NIT_Init (void)
{
	int i;

	SI_NIT_CDS_Init();
	SI_NIT_MMS_Init();

	NIT_Version_Number = 0xff;
	for (i=0; i<8; i++)
	{
		NIT_Section_Mask[i] = 0;
	}
}
void SI_NIT_SetMask(void)
{
	int i;

	NIT_Version_Number = 32;
	for (i=0; i<8; i++)
	{
		NIT_Section_Mask[i] = -1;
	}

}


/*********************************************************************
 Function : SI_NIT_Get
 Description : Return the version numbers and masks.
 Input : none.
 Output : version number and pointers to section masks.
**********************************************************************/
void SI_NIT_Get(unsigned char *p_NIT_Version_Number,
				unsigned long **p_NIT_Section_Mask
				)
{
	*p_NIT_Version_Number = NIT_Version_Number;
	*p_NIT_Section_Mask = NIT_Section_Mask;
}

/*********************************************************************
 Function : SI_NIT_parse
 Description : Function to parse the NIT table.
 Input : nit_table, point to the current NIT table section.
 Output : SI_RET_CODE.
**********************************************************************/
SI_RET_CODE SI_NIT_parse (unsigned char * nit_table)
{
	unsigned long temp;
	unsigned long section_length, offset, subtable_len;
	unsigned char first_index, number_of_records, table_subtype;
	unsigned char desc_count, desc_tag, desc_len;
	unsigned char version_number, section_number, last_section_number;
	int i, j, found_rdd;
	found_rdd = 0;

	SI_DBG_PRINT(E_SI_DBG_MSG,("NIT Table received.\n"));

	temp = *nit_table;
	if (temp != SI_NIT_TABLE_ID)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("NIT Table ID error!!! %x\n", temp));
		return SI_TABLE_ID_ERROR;
	}

	/* calculate and check section length. */
	section_length = SI_Construct_Data(nit_table,
				NIT_SECTION_LENGTH_BYTE_INDX,
				NIT_SECTION_LENGTH_BYTE_NUM,
				NIT_SECTION_LENGTH_SHIFT,
				NIT_SECTION_LENGTH_MASK);
	section_length += NIT_SECTION_LENGTH_BYTE_INDX+NIT_SECTION_LENGTH_BYTE_NUM;
	if (section_length > SI_NORMAL_SECTION_LENGTH)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("NIT Table section length error!!! %x\n", section_length));
		return SI_SECTION_LENGTH_ERROR;
	}

	/* We do the CRC check here to verify the contents of this section. */
	if (SI_CRC32_Check(nit_table, section_length) != SI_SUCCESS)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("NIT Table section CRC error!!!\n"));
		return SI_CRC_ERROR;
	}

	/* check protocol version. It should be zero for now. */
	temp = SI_Construct_Data(nit_table,
				NIT_PROTOCOL_VERSION_BYTE_INDX,
				NIT_PROTOCOL_VERSION_BYTE_NUM,
				NIT_PROTOCOL_VERSION_SHIFT,
				NIT_PROTOCOL_VERSION_MASK);
	if (temp != SI_CURRENT_PROTOCOL_VERSION)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("NIT Table PROTOCOL version error!!! %x\n", temp));
		return SI_PROTOCOL_VER_ERROR;
	}

	first_index = SI_Construct_Data(nit_table,
					NIT_FIRST_INDEX_BYTE_INDX,
					NIT_FIRST_INDEX_BYTE_NUM,
					NIT_FIRST_INDEX_SHIFT,
					NIT_FIRST_INDEX_MASK);
	/* make sure the first_index not zero */
	if (first_index == 0)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("NIT Table first index zero!!!"));
		return SI_OTHER_ERROR;
	}
	number_of_records = SI_Construct_Data(nit_table,
						NIT_NUMBER_OF_RECORDS_BYTE_INDX,
						NIT_NUMBER_OF_RECORDS_BYTE_NUM,
						NIT_NUMBER_OF_RECORDS_SHIFT,
						NIT_NUMBER_OF_RECORDS_MASK);
	table_subtype = SI_Construct_Data(nit_table,
					NIT_TABLE_SUBTYPE_BYTE_INDX,
					NIT_TABLE_SUBTYPE_BYTE_NUM,
					NIT_TABLE_SUBTYPE_SHIFT,
					NIT_TABLE_SUBTYPE_MASK);

	/* Now we need to check for the revision descriptors(if there is any) to get the section
		number and revision info. This descriptor should be at the end of the table section just
		before the CRC bytes. NOT at the end of each table subtype. */
	offset = NIT_TABLE_SUBTYPE_BYTE_INDX + NIT_TABLE_SUBTYPE_BYTE_NUM;
	switch (table_subtype)
	{
		case CDS_SUBTYPE:
			subtable_len = NIT_CDS_SUBTABLE_SIZE;
		break;
		case MMS_SUBTYPE:
			subtable_len = NIT_MMS_SUBTABLE_SIZE;
		break;
		default:
			SI_DBG_PRINT(E_SI_ERR_MSG,("NIT Table subtype error!!!\n"));
			return SI_TABLE_ID_ERROR;
		break;
	}
	/* go through all subtables and subtable descriptors to get to the NIT table descriptor. */
	for (i=0; i<number_of_records; i++)
	{
		offset += subtable_len;
		desc_count = *(nit_table + offset++);
		for (j=0; j<desc_count; j++)
		{
			desc_tag = *(nit_table + offset++);
			desc_len = *(nit_table + offset++);
			offset += desc_len;
		}
	}
	if (offset == section_length-SI_CRC_LENGTH)
	{
		SI_DBG_PRINT(E_SI_DBG_MSG,("NIT no table level descriptors found.\n"));
	}
	else if (offset > section_length-SI_CRC_LENGTH)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("NIT Table length error!!!\n"));
		return SI_SECTION_LENGTH_ERROR;
	}
	else
	{
		/* we got more descriptors. */
		while (offset < section_length-SI_CRC_LENGTH)
		{
			desc_tag = *(nit_table + offset++);
			desc_len = *(nit_table + offset++);
			if (desc_tag == SI_DESC_REVISION_DETECTION)
			{
				found_rdd = 1;
				/* we got a revision dectection descriptor. */
				SI_DBG_PRINT(E_SI_DBG_MSG,("NIT revision detection descriptors found.\n"));
				if (desc_len != DESC_REV_DECTECT_LEN)
				{
					SI_DBG_PRINT(E_SI_ERR_MSG,("NIT Table Rev detect descriptor length error!!!\n"));
					return SI_SECTION_LENGTH_ERROR;
				}
				version_number = SI_Construct_Data( (nit_table + offset),
								DESC_REV_DECTECT_VER_NUM_BYTE_INDEX,
								DESC_REV_DECTECT_VER_NUM_BYTE_NUM,
								DESC_REV_DECTECT_VER_NUM_SHIFT,
								DESC_REV_DECTECT_VER_NUM_MASK);
				section_number = SI_Construct_Data( (nit_table + offset),
								DESC_REV_DECTECT_SEC_NUM_BYTE_INDEX,
								DESC_REV_DECTECT_SEC_NUM_BYTE_NUM,
								DESC_REV_DECTECT_SEC_NUM_SHIFT,
								DESC_REV_DECTECT_SEC_NUM_MASK);
				last_section_number = SI_Construct_Data( (nit_table + offset),
								DESC_REV_DECTECT_LAST_SEC_NUM_BYTE_INDEX,
								DESC_REV_DECTECT_LAST_SEC_NUM_BYTE_NUM,
								DESC_REV_DECTECT_LAST_SEC_NUM_SHIFT,
								DESC_REV_DECTECT_LAST_SEC_NUM_MASK);
#if 0
				if (table_subtype == CDS_SUBTYPE)
				{
					/* CDS subtable. */
					if (version_number == NIT_CDS_Version_Number)
					{
						/* same version number, check section mask. */
						if (SI_Chk_Section_mask(NIT_CDS_Section_Mask, section_number))
							return SI_SUCCESS;		/* no need to update. */
					}
					else
					{
						/* different CDS version, init the CDS table and section mask .*/
						SI_NIT_CDS_Init();
						NIT_CDS_Version_Number = version_number;
						SI_Init_Section_Mask(NIT_CDS_Section_Mask, last_section_number);
					}
					/* update the mask */
					SI_Set_Section_mask(NIT_CDS_Section_Mask, section_number);
				}
				else
				{
					/* MMS subtable. */
					if (version_number == NIT_MMS_Version_Number)
					{
						/* same version number, check section mask. */
						if (SI_Chk_Section_mask(NIT_MMS_Section_Mask, section_number))
							return SI_SUCCESS;		/* no need to update. */
					}
					else
					{
						/* different MMS version, init the MMS table and section mask .*/
						SI_NIT_MMS_Init();
						NIT_MMS_Version_Number = version_number;
						SI_Init_Section_Mask(NIT_MMS_Section_Mask, last_section_number);
					}
					/* update the mask */
					SI_Set_Section_mask(NIT_MMS_Section_Mask, section_number);
				}
#else
					if (version_number == NIT_Version_Number)
					{
						/* same version number, check section mask. */
						if (SI_Chk_Section_mask(NIT_Section_Mask, section_number))
							return SI_SUCCESS;		/* no need to update. */
					}
					else
					{
						/* different  version, init the table and section mask .*/
						SI_NIT_CDS_Init();
						SI_NIT_MMS_Init();
						NIT_Version_Number = version_number;
						SI_Init_Section_Mask(NIT_Section_Mask, last_section_number);
					}
					/* update the mask */
					SI_Set_Section_mask(NIT_Section_Mask, section_number);
#endif
			}
			else if (desc_tag != SI_DESC_STUFFING)
				SI_DBG_PRINT(E_SI_WRN_MSG,("NIT table level descriptor %x we can not handle!\n", desc_tag));
				/* we just don't handle it. */

			offset += desc_len;
		}
	}
	if (found_rdd == 0)
	{
		SI_NIT_SetMask();
		SI_DBG_PRINT(E_SI_DBG_MSG,("Revistion Detection Descriptor is required!!!\n"));
	}

	if (offset != section_length-SI_CRC_LENGTH)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("NIT Table descriptor length error!!!(%d, %d, %d)\n",offset,section_length,section_length-SI_CRC_LENGTH));
		return SI_SECTION_LENGTH_ERROR;
	}


	/* now just update the table. */
	offset = NIT_TABLE_SUBTYPE_BYTE_INDX + NIT_TABLE_SUBTYPE_BYTE_NUM;
	for (i=0; i<number_of_records; i++)
	{
		if (table_subtype == CDS_SUBTYPE)
			first_index += SI_NIT_CDS_parse( (nit_table + offset), first_index );
		else
			SI_NIT_MMS_parse ( (nit_table + offset), first_index++ );

		offset += subtable_len;
		desc_count = *(nit_table + offset++);
		for (j=0; j<desc_count; j++)
		{
			desc_tag = *(nit_table + offset++);
			if (desc_tag != SI_DESC_STUFFING)
				SI_DBG_PRINT(E_SI_WRN_MSG,("NIT Subtable descriptor %x we can not handle!\n", desc_tag));
				/* we just don't handle it. */
			desc_len = *(nit_table + offset++);
			offset += desc_len;
		}
	}

	return SI_SUCCESS;
}


