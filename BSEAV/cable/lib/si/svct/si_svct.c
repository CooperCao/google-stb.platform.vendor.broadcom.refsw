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
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#include <stdlib.h>
#include "si.h"
#include "si_os.h"
#include "si_dbg.h"
#include "si_util.h"
#include "si_list.h"
#include "si_vct.h"
#include "si_svct_vcm.h"
#include "si_svct.h"
#include "si_descriptors.h"

unsigned char SVCT_VCM_version_number;
unsigned long SVCT_VCM_section_mask[8];
unsigned long current_VCT_ID = 0;
unsigned long SVCT_VCT_ID = -1;

void SI_SVCT_Init(void)
{
	unsigned long i;

	SI_SVCT_VCM_Init();
	SVCT_VCM_version_number = 0xff;
	for (i=0; i<8; i++)
		SVCT_VCM_section_mask[i] = 0;
	current_VCT_ID = 0;
	SVCT_VCT_ID = -1;
}

void SI_SVCT_SetID(unsigned long vct_id)
{
	SVCT_VCT_ID = vct_id;
	SI_DBG_PRINT(E_SI_DBG_MSG,("SVCT Table VCT ID is set %d.\n", vct_id));
}
unsigned long SI_SVCT_GetID()
{
	return current_VCT_ID;
}
static void SI_SVCT_SetMask()
{
	unsigned long i;

	if ( getenv("SVCT_RDD") == NULL)
		SVCT_VCM_version_number++; // use latest SVCT received if there is no revision descriptor
	else
		SVCT_VCM_version_number = 32; //use first SVCT recived if there is no revision descriptor

	for (i=0; i<8; i++)
		SVCT_VCM_section_mask[i] = -1;
}

/*********************************************************************
 Function : SI_SVCT_Get
 Description : Return the version numbers and masks.
 Input : none.
 Output : version number and pointers to section masks.
**********************************************************************/
void SI_SVCT_Get(unsigned char *p_SVCT_VCM_version_number,
				unsigned long **p_SVCT_VCM_section_mask
				)
{
	*p_SVCT_VCM_version_number = SVCT_VCM_version_number;
	*p_SVCT_VCM_section_mask = SVCT_VCM_section_mask;
}

/*********************************************************************
 Function : SI_SVCT_parse
 Description : Function to parse the S-VCT table.
 Input : unsigned char * table: point to the current S-VCT table section.
 Output : SI_RET_CODE.
**********************************************************************/
SI_RET_CODE SI_SVCT_parse (unsigned char * table)
{
	unsigned long temp;
	unsigned long section_length;
	unsigned char table_subtype;
	unsigned long desc_tag, desc_len, len;
	unsigned char version_number, section_number, last_section_number;
	unsigned char *current, *crc_start;
	int i, j,found_rdd;

	found_rdd = 0;
	SI_DBG_PRINT(E_SI_DBG_MSG,("SVCT Table received.\n"));

	temp = *table;
	if (temp != SI_SVCT_TABLE_ID)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("S-VCT Table ID error!!! %x\n", temp));
		return SI_TABLE_ID_ERROR;
	}

	/* calculate and check section length. */
	section_length = SI_Construct_Data(table,
				SVCT_SECTION_LENGTH_BYTE_INDX,
				SVCT_SECTION_LENGTH_BYTE_NUM,
				SVCT_SECTION_LENGTH_SHIFT,
				SVCT_SECTION_LENGTH_MASK);
	section_length += SVCT_SECTION_LENGTH_BYTE_INDX+SVCT_SECTION_LENGTH_BYTE_NUM;
	if (section_length > SI_NORMAL_SECTION_LENGTH)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("S-VCT Table section length error!!! %x\n", section_length));
		return SI_SECTION_LENGTH_ERROR;
	}

	/* We do the CRC check here to verify the contents of this section. */
	if (SI_CRC32_Check(table, section_length) != SI_SUCCESS)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("S-VCT Table section CRC error!!!\n"));
		return SI_CRC_ERROR;
	}

	/* check protocol version. It should be zero for now. */
	temp = SI_Construct_Data(table,
				SVCT_PROTOCOL_VERSION_BYTE_INDX,
				SVCT_PROTOCOL_VERSION_BYTE_NUM,
				SVCT_PROTOCOL_VERSION_SHIFT,
				SVCT_PROTOCOL_VERSION_MASK);
	if (temp != SI_CURRENT_PROTOCOL_VERSION)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("S-VCT Table PROTOCOL version error!!! %x\n", temp));
		return SI_PROTOCOL_VER_ERROR;
	}

	/* we just need to handle VCM. Ignore the ICM or DCM for now. */
	table_subtype = SI_Construct_Data(table,
					SVCT_TABLE_SUBTYPE_BYTE_INDX,
					SVCT_TABLE_SUBTYPE_BYTE_NUM,
					SVCT_TABLE_SUBTYPE_SHIFT,
					SVCT_TABLE_SUBTYPE_MASK);

	current_VCT_ID = SI_Construct_Data(table,
					SVCT_VCT_ID_BYTE_INDX,
					SVCT_VCT_ID_BYTE_NUM,
					SVCT_VCT_ID_SHIFT,
					SVCT_VCT_ID_MASK);
	if (SVCT_VCT_ID != -1 && current_VCT_ID != SVCT_VCT_ID)
	{
		SI_DBG_PRINT(E_SI_DBG_MSG,("VCT_ID doesn't match. skip SVCT table.\n"));
		return SI_SUCCESS;
	}

	current = table + SVCT_VCT_ID_BYTE_INDX + SVCT_VCT_ID_BYTE_NUM;

	if (table_subtype != VCM)
	{
		//TODO Handle DCM and ICM
		SI_DBG_PRINT(E_SI_DBG_MSG,("We are only interested in VCM of SVCT right now.\n"));
		//skip all ICM and DCM for the time being
		if (table_subtype == ICM)
		{
			int record_count;
			current += 2;
			record_count = *current++;
			current += (record_count*4);
		}
		else //DCM
		{
			int record_count;
			current += 2;
			record_count = *current++;
			current += record_count;
		}
		//return SI_SUCCESS;
	}
	else
	{
		/* point to the start of table level descriptor. */
		current = SI_SVCT_VCM_Pointer(current);
	}
	crc_start = table + section_length - SI_CRC_LENGTH;

	/* go through all descriptors. */
	while ((unsigned long)current < (unsigned long)crc_start)
	{
		desc_tag = *(current++);
		len = *(current++);
		switch(desc_tag)
		{
			case SI_DESC_REVISION_DETECTION:
				found_rdd = 1;
				/* we got a revision dectection descriptor. */
				SI_DBG_PRINT(E_SI_DBG_MSG,("SVCT VCM revision detection descriptors found.\n"));
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
				SI_DBG_PRINT(E_SI_DBG_MSG,("SVCT VCM revision detection descriptors found. %d\n", version_number));
				if (version_number == SVCT_VCM_version_number)
				{
					/* same version number, check section mask. */
					if (SI_Chk_Section_mask(SVCT_VCM_section_mask, section_number))
						return SI_SUCCESS;		/* no need to update. */
					SI_DBG_PRINT(E_SI_DBG_MSG,("SVCT VCM SI_Chk_Section_mask. %d\n", section_number));
				}
				else
				{
					/* different CDS version, init the CDS table and section mask .*/
					SI_SVCT_VCM_Free_List();
					SVCT_VCM_version_number = version_number;
					SI_Init_Section_Mask(SVCT_VCM_section_mask, last_section_number);
					SI_DBG_PRINT(E_SI_DBG_MSG,("SVCT VCM SI_Init_Section_Mask. %d\n", last_section_number));
				}
				/* update the mask */
				SI_Set_Section_mask(SVCT_VCM_section_mask, section_number);
				SI_DBG_PRINT(E_SI_DBG_MSG,("SVCT VCM SI_Set_Section_mask. 0x%x  %d\n", SVCT_VCM_section_mask[0], section_number));
			break;

			default:
				SI_DBG_PRINT(E_SI_WRN_MSG,("SVCT VCM table descriptor %x received! Ignoring!\n", desc_tag));
			break;
		}

		/* update pointer. */
		current += len;
	}

	/* check for desc length. */
	if ((unsigned long)current != (unsigned long)crc_start)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("S-VCT Table length error!!!\n"));
		return SI_SECTION_LENGTH_ERROR;
	}
	if (found_rdd == 0)
	{
		SI_DBG_PRINT(E_SI_DBG_MSG,("Revistion Detection Descriptor is required!!!\n"));
		SI_SVCT_SetMask();
	}

	/* we either need to update the channel table or we don't have a revision detection. */
	current = table + SVCT_VCT_ID_BYTE_INDX + SVCT_VCT_ID_BYTE_NUM;

	if (table_subtype == VCM)
		return SI_SVCT_VCM_Parse(current);
	else
		return SI_SUCCESS;
}
