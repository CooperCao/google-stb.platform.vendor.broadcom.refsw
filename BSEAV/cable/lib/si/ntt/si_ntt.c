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
#include "si_list.h"
#include "si_ntt_sns.h"
#include "si_ntt.h"
#include "si_descriptors.h"


unsigned char NTT_SNS_version_number;
unsigned long NTT_SNS_section_mask[8];
unsigned long NTT_SNS_Language_Code;

void SI_NTT_Init(void)
{
	unsigned long i;

	SI_NTT_SNS_Init();
	NTT_SNS_version_number = 0xff;
	for (i=0; i<8; i++)
		NTT_SNS_section_mask[i] = 0;
}

/*********************************************************************
 Function : SI_NTT_Get
 Description : Return the version numbers and masks.
 Input : none.
 Output : version number and pointers to section masks.
**********************************************************************/
void SI_NTT_Get(unsigned char *p_NTT_SNS_version_number,
				unsigned long **p_NTT_SNS_section_mask
				)
{
	*p_NTT_SNS_version_number = NTT_SNS_version_number;
	*p_NTT_SNS_section_mask = NTT_SNS_section_mask;
}

/*********************************************************************
 Function : SI_NTT_parse
 Description : Function to parse the NTT table.
 Input : unsigned char * table: point to the current NTT table section.
 Output : SI_RET_CODE.
**********************************************************************/
SI_RET_CODE SI_NTT_parse (unsigned char * table)
{
	unsigned long temp;
	unsigned long section_length;
	unsigned char table_subtype;
	unsigned long desc_tag, desc_len, len;
	unsigned char version_number, section_number, last_section_number;
	unsigned char *current, *crc_start;
	int i, j,found_rdd;

	found_rdd = 0;
	SI_DBG_PRINT(E_SI_DBG_MSG,("NTT Table received.\n"));

	temp = *table;
	if (temp != SI_NTT_TABLE_ID)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("NTT Table ID error!!! %x\n", temp));
		return SI_TABLE_ID_ERROR;
	}

	/* calculate and check section length. */
	section_length = SI_Construct_Data(table,
					NTT_SECTION_LENGTH_BYTE_INDX,
					NTT_SECTION_LENGTH_BYTE_NUM,
					NTT_SECTION_LENGTH_SHIFT,
					NTT_SECTION_LENGTH_MASK);
	section_length += NTT_SECTION_LENGTH_BYTE_INDX+NTT_SECTION_LENGTH_BYTE_NUM;
	if (section_length > SI_NORMAL_SECTION_LENGTH)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("NTT Table section length error!!! %x\n", section_length));
		return SI_SECTION_LENGTH_ERROR;
	}

	/* We do the CRC check here to verify the contents of this section. */
	if (SI_CRC32_Check(table, section_length) != SI_SUCCESS)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("NTT Table section CRC error!!!\n"));
		return SI_CRC_ERROR;
	}

	/* check protocol version. It should be zero for now. */
	temp = SI_Construct_Data(table,
				NTT_PROTOCOL_VERSION_BYTE_INDX,
				NTT_PROTOCOL_VERSION_BYTE_NUM,
				NTT_PROTOCOL_VERSION_SHIFT,
				NTT_PROTOCOL_VERSION_MASK);
	if (temp != SI_CURRENT_PROTOCOL_VERSION)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("NTT Table PROTOCOL version error!!! %x\n", temp));
		return SI_PROTOCOL_VER_ERROR;
	}

	/* we just need to handle SNS. Ignore others now. */
	table_subtype = SI_Construct_Data(table,
					NTT_TABLE_SUBTYPE_BYTE_INDX,
					NTT_TABLE_SUBTYPE_BYTE_NUM,
					NTT_TABLE_SUBTYPE_SHIFT,
					NTT_TABLE_SUBTYPE_MASK);
	if (table_subtype != NTT_SNS_SUBTYPE)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("We can only handle SNS of NTT right now.\n"));
		return SI_TABLE_ID_ERROR;
	}

	/* we get a SNS record. see if we have any table level
		descriptors for revision dectection. */
	current = table + NTT_TABLE_SUBTYPE_BYTE_INDX + NTT_TABLE_SUBTYPE_BYTE_NUM;

	/* point to the start of table level descriptor. */
	current = SI_NTT_SNS_Pointer(current);
	crc_start = table + section_length - SI_CRC_LENGTH;

	SI_DBG_PRINT(E_SI_DBG_MSG,("table %x, current %x, crc_start %x\n", table, current, crc_start));
	/* go through all descriptors. */
	while ((unsigned long)current < (unsigned long)crc_start)
	{
		desc_tag = *(current++);
		len = *(current++);
		switch(desc_tag)
		{
			case SI_DESC_REVISION_DETECTION:
				/* we got a revision dectection descriptor. */
				found_rdd = 1;
				SI_DBG_PRINT(E_SI_DBG_MSG,("NTT SNS revision detection descriptors found.\n"));
				version_number = SI_Construct_Data( current,
								DESC_REV_DECTECT_VER_NUM_BYTE_INDEX,
								DESC_REV_DECTECT_VER_NUM_BYTE_NUM,
								DESC_REV_DECTECT_VER_NUM_SHIFT,
								DESC_REV_DECTECT_VER_NUM_MASK);
				section_number = SI_Construct_Data( current,
								DESC_REV_DECTECT_SEC_NUM_BYTE_INDEX,
								DESC_REV_DECTECT_SEC_NUM_BYTE_NUM,
								DESC_REV_DECTECT_SEC_NUM_SHIFT,
								DESC_REV_DECTECT_SEC_NUM_MASK);
				last_section_number = SI_Construct_Data( current,
								DESC_REV_DECTECT_LAST_SEC_NUM_BYTE_INDEX,
								DESC_REV_DECTECT_LAST_SEC_NUM_BYTE_NUM,
								DESC_REV_DECTECT_LAST_SEC_NUM_SHIFT,
								DESC_REV_DECTECT_LAST_SEC_NUM_MASK);
				if (version_number == NTT_SNS_version_number)
				{
					/* same version number, check section mask. */
					if (SI_Chk_Section_mask(NTT_SNS_section_mask, section_number))
						return SI_SUCCESS;		/* no need to update. */
				}
				else
				{
					/* different CDS version, init the CDS table and section mask .*/
					SI_NTT_SNS_Free_List();
					NTT_SNS_version_number = version_number;
					SI_Init_Section_Mask(NTT_SNS_section_mask, last_section_number);
				}
				/* update the mask */
				SI_Set_Section_mask(NTT_SNS_section_mask, section_number);
			break;

			default:
				SI_DBG_PRINT(E_SI_WRN_MSG,("NTT SNS table descriptor %x received! Ignoring!\n", desc_tag));
			break;
		}

		/* update pointer. */
		current += len;
	}

	/* check for desc length. */
	if ((unsigned long)current != (unsigned long)crc_start)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("NTT Table length error!!!\n"));
		return SI_SECTION_LENGTH_ERROR;
	}
#ifdef CONFIG_RDD_REQUIRED
	if (found_rdd == 0)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("Revistion Detection Descriptor is required!!!\n"));
		return SI_DESCRIPTOR_ERROR;
	}
#endif
	/* we either need to update the channel table or we don't have a revision detection. */
	NTT_SNS_Language_Code = SI_Construct_Data(table,
							NTT_ISO_639_CODE_BYTE_INDX,
							NTT_ISO_639_CODE_BYTE_NUM,
							NTT_ISO_639_CODE_SHIFT,
							NTT_ISO_639_CODE_MASK);

	current = table + NTT_TABLE_SUBTYPE_BYTE_INDX + NTT_TABLE_SUBTYPE_BYTE_NUM;
	return SI_NTT_SNS_Parse(current,NTT_SNS_Language_Code);
}

