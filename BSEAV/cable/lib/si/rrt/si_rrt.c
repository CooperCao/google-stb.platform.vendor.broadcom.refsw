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
#include "si_rrt.h"
#include "si_descriptors.h"


/* local function prototypes. */
static SI_RRT_REGION * SI_RRT_Create_Region (unsigned char region);
static SI_RET_CODE  SI_RRT_Clear_Region (SI_RRT_REGION * rrt_region);
static SI_RRT_REGION * SI_RRT_Find_Region (unsigned char region);


struct rrt_region_list RRT_Regions;
unsigned char Total_RRT_Regions;

SI_mutex m_rrt;


void SI_RRT_Init(void)
{
	SI_LST_D_INIT(&RRT_Regions);
	Total_RRT_Regions = 0;
	SI_mutex_init(m_rrt);

}

/*********************************************************************
 Function : SI_RRT_Create_Region
 Description : Function to allocate the space for an instance of
				 RRT.
 Input : unsigned char region : the rating region number.
 Output : pointer to the RRT instance structure allocated. Will
			return NULL if out of memory.
**********************************************************************/
static SI_RRT_REGION * SI_RRT_Create_Region (unsigned char region)
{
	SI_RRT_REGION * rrt_region;
	int	i;

	rrt_region = (SI_RRT_REGION *)SI_alloc(sizeof(SI_RRT_REGION));
	if (rrt_region == NULL)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("Failed to allocate an RRT instance!!!\n"));
		return NULL;
	}

	SI_LST_D_INIT_ENTRY(&(rrt_region->rrt_link));
	rrt_region->rating_region = region;
	rrt_region->rating_region_name_text = NULL;
	rrt_region->rrt_dimensions = NULL;
	rrt_region->dimensions_defined = 0;
	rrt_region->rating_region_name_length = 0;

	rrt_region->version_number = 0xff;

	SI_LST_D_INSERT_HEAD(&RRT_Regions, rrt_region, rrt_link);

	Total_RRT_Regions++;

	return rrt_region;
}


/*********************************************************************
 Function : SI_RRT_Clear_Region
 Description : Function to clear an instance of  RRT.
 Input : SI_RRT_REGION * rrt_region, points to the RRT instance
 				structure to be cleared.
 Output : SI_RET_CODE
**********************************************************************/
static SI_RET_CODE  SI_RRT_Clear_Region (SI_RRT_REGION * rrt_region)
{
	int i,j;

	if (rrt_region)
	{
		for (i=0; i<rrt_region->dimensions_defined; i++)
		{
			for (j=0; j<rrt_region->rrt_dimensions->values_defined; j++)
			{
				SI_free(rrt_region->rrt_dimensions->rrt_values->abbrev_rating_value_text);
				SI_free(rrt_region->rrt_dimensions->rrt_values->rating_value_text);
			}
			SI_free(rrt_region->rrt_dimensions->rrt_values);
			SI_free(rrt_region->rrt_dimensions->dimension_name_text);
		}
		if (rrt_region->rrt_dimensions)
			SI_free(rrt_region->rrt_dimensions);
		if (rrt_region->rating_region_name_text)
			SI_free(rrt_region->rating_region_name_text);
	}

	return SI_SUCCESS;
}


/*********************************************************************
 Function : SI_RRT_Find_Region
 Description : Function find instance of RRT of which the rating
 				region matches the specified one.
 Input : unsigned char region, rating region to match.
 Output : SI_RRT_REGION * points to the RRT instance structure that matches
 			the rating regoin code. NULL if no match can be found.
**********************************************************************/
static SI_RRT_REGION * SI_RRT_Find_Region (unsigned char region)
{
	SI_RRT_REGION * rrt_region;

	rrt_region = SI_LST_D_FIRST(&RRT_Regions);
	while (rrt_region)
	{
		if (rrt_region->rating_region == region)
			return rrt_region;
		rrt_region = SI_LST_D_NEXT(rrt_region, rrt_link);
	}

	return NULL;
}


/**********************************************************************
 Function : SI_RRT_parse
 Description : Function to parse the RRT table. It will create and
				 keep track of RRT instances.
 Input : rrt_table, point to the rrt table data.
 Output : Success code.
***********************************************************************/
SI_RET_CODE SI_RRT_parse (unsigned char * rrt_table)
{
	unsigned long temp, i, j, k;
	unsigned long section_length;
	unsigned long CRC_start;
	unsigned long tot_desc_len, offset;
	unsigned long desc_tag, desc_len;
	unsigned char region;
	unsigned char *current;
	SI_RRT_REGION *rrt_region;
	SI_RET_CODE result;

	SI_DBG_PRINT(E_SI_DBG_MSG,("RRT Table received.\n"));

	temp = *rrt_table;
	if (temp != SI_RRT_TABLE_ID)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("RRT Table ID error!!! %x\n", temp));
		return SI_TABLE_ID_ERROR;
	}

	/* calculate and check section length. */
	section_length = SI_Construct_Data(rrt_table,
				RRT_SECTION_LENGTH_BYTE_INDX,
				RRT_SECTION_LENGTH_BYTE_NUM,
				RRT_SECTION_LENGTH_SHIFT,
				RRT_SECTION_LENGTH_MASK);
printf("section len %x\n", section_length);
	section_length += RRT_SECTION_LENGTH_BYTE_INDX+RRT_SECTION_LENGTH_BYTE_NUM;
	if (section_length > SI_NORMAL_SECTION_LENGTH)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("RRT Table section length error!!! %x\n", section_length));
		return SI_SECTION_LENGTH_ERROR;
	}

	/* We do the CRC check here to verify the contents of this section. */
	if (SI_CRC32_Check(rrt_table, section_length) != SI_SUCCESS)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("RRT Table section CRC error!!!\n"));
		/*QY NEED TO FIX   return SI_CRC_ERROR;*/
	}

	/* check to make sure protocol version is zero. */
	temp = SI_Construct_Data(rrt_table,
				RRT_PROTOCOL_VERSION_BYTE_INDX,
				RRT_PROTOCOL_VERSION_BYTE_NUM,
				RRT_PROTOCOL_VERSION_SHIFT,
				RRT_PROTOCOL_VERSION_MASK);
	if (temp != SI_CURRENT_PROTOCOL_VERSION)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("RRT Table PROTOCOL version error!!! %x\n", temp));
		return SI_PROTOCOL_VER_ERROR;
	}

	/* make sure that section_number and last_section_number are both 0. */
	temp = SI_Construct_Data(rrt_table,
				RRT_SECTION_NUMBER_BYTE_INDX,
				RRT_SECTION_NUMBER_BYTE_NUM,
				RRT_SECTION_NUMBER_SHIFT,
				RRT_SECTION_NUMBER_MASK);
	if (temp != 0)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("RRT Table section_number not zero!!! %x\n", temp));
		return SI_SECTION_NUMBER_ERROR;
	}
	temp = SI_Construct_Data(rrt_table,
				RRT_LAST_SECTION_NUMBER_BYTE_INDX,
				RRT_LAST_SECTION_NUMBER_BYTE_NUM,
				RRT_LAST_SECTION_NUMBER_SHIFT,
				RRT_LAST_SECTION_NUMBER_MASK);
	if (temp != 0)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("RRT Table last_section_number not zero!!! %x\n", temp));
		return SI_SECTION_NUMBER_ERROR;
	}

	/* look at current_next_indicator. It should be 1 for RRT. */
	temp = SI_Construct_Data(rrt_table,
				RRT_CURRENT_NEXT_INDICATOR_BYTE_INDX,
				RRT_CURRENT_NEXT_INDICATOR_BYTE_NUM,
				RRT_CURRENT_NEXT_INDICATOR_SHIFT,
				RRT_CURRENT_NEXT_INDICATOR_MASK);
	if (temp != 1)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("RRT Table current_next_indicator not one!!! %x\n", temp));
		return SI_CURRENT_NEXT_INDICATOR_ERROR;
	}

	/* check to see if we already have the region. */
	region = SI_Construct_Data(rrt_table,
				RRT_RATING_REGION_BYTE_INDX,
				RRT_RATING_REGION_BYTE_NUM,
				RRT_RATING_REGION_SHIFT,
				RRT_RATING_REGION_MASK);

	/* locking mutex for data access. */
	SI_mutex_lock(m_rrt);

	if ((rrt_region = SI_RRT_Find_Region(region)) == NULL)
	{
		/* we do not have this rating region defined yet. */
		if ((rrt_region = SI_RRT_Create_Region(region)) == NULL)
		{
			SI_mutex_unlock(m_rrt);
			return SI_NO_MEMORY;
		}
	}

	SI_DBG_PRINT(E_SI_DBG_MSG,("RRT region %x.\n", region));

	/* Now check the version number and decide whether we want to update or not */
	temp = SI_Construct_Data(rrt_table,
				RRT_VERSION_NUMBER_BYTE_INDX,
				RRT_VERSION_NUMBER_BYTE_NUM,
				RRT_VERSION_NUMBER_SHIFT,
				RRT_VERSION_NUMBER_MASK);
	if (rrt_region->version_number == temp)
	{
		SI_mutex_unlock(m_rrt);
		return SI_SUCCESS; /* do not need to do anything when it is the same version */
	}
	rrt_region->version_number = temp;

	/* new RRT table received!!! */
	SI_DBG_PRINT(E_SI_DBG_MSG,("New RRT Table received! Deleting the old one.\n"));

	SI_RRT_Clear_Region (rrt_region);

	/* get rating region name. */
	rrt_region->rating_region_name_length = SI_Construct_Data(rrt_table,
										RRT_REGION_NAME_LENGTH_BYTE_INDX,
										RRT_REGION_NAME_LENGTH_BYTE_NUM,
										RRT_REGION_NAME_LENGTH_SHIFT,
										RRT_REGION_NAME_LENGTH_MASK);
	rrt_region->rating_region_name_text = (char *)SI_alloc(rrt_region->rating_region_name_length);
	if (rrt_region->rating_region_name_text == NULL)
	{
		SI_mutex_unlock(m_rrt);
		SI_DBG_PRINT(E_SI_ERR_MSG,("RRT failed to alloc rating region name text!!!\n"));
		return SI_NO_MEMORY;
	}
	current = rrt_table + RRT_REGION_NAME_LENGTH_BYTE_INDX+RRT_REGION_NAME_LENGTH_BYTE_NUM;
	SI_memcpy(rrt_region->rating_region_name_text, current, rrt_region->rating_region_name_length);
	current += rrt_region->rating_region_name_length;

	/* debug print for region name!!! */
	SI_DBG_PRINT(E_SI_DBG_MSG,("RRT region name:\n"));
	for (k=0; k<rrt_region->rating_region_name_length; k++)
		if (k>7)
			SI_DBG_PRINT(E_SI_DBG_MSG,("%c", (char)rrt_region->rating_region_name_text[k]));
	SI_DBG_PRINT(E_SI_DBG_MSG,("\n"));

	/* get the dimension number. */
	rrt_region->dimensions_defined = SI_Construct_Data(current,
										RRT_DIMENSIONS_DEFINED_BYTE_INDX,
										RRT_DIMENSIONS_DEFINED_BYTE_NUM,
										RRT_DIMENSIONS_DEFINED_SHIFT,
										RRT_DIMENSIONS_DEFINED_MASK);
	current += RRT_DIMENSIONS_DEFINED_BYTE_NUM;
	/* create dimensions. */
	rrt_region->rrt_dimensions = (SI_RRT_DIMENSION *)SI_alloc(rrt_region->dimensions_defined*sizeof(SI_RRT_DIMENSION));
	if (rrt_region->rrt_dimensions == NULL)
	{
		SI_mutex_unlock(m_rrt);
		SI_DBG_PRINT(E_SI_ERR_MSG,("RRT failed to alloc rating region dimensions text!!!\n"));
		return SI_NO_MEMORY;
	}

	/* go through all dimensions. */
	for (i=0; i<rrt_region->dimensions_defined; i++)
	{
		rrt_region->rrt_dimensions[i].dimension_name_length = SI_Construct_Data(current,
															RRT_DIMENSION_NAME_LENGTH_BYTE_INDX,
															RRT_DIMENSION_NAME_LENGTH_BYTE_NUM,
															RRT_DIMENSION_NAME_LENGTH_SHIFT,
															RRT_DIMENSION_NAME_LENGTH_MASK);
		current += RRT_DIMENSION_NAME_LENGTH_BYTE_INDX+RRT_DIMENSION_NAME_LENGTH_BYTE_NUM;
		rrt_region->rrt_dimensions[i].dimension_name_text = (char *)SI_alloc(rrt_region->rrt_dimensions[i].dimension_name_length);
		if (rrt_region->rrt_dimensions[i].dimension_name_text == NULL)
		{
			SI_mutex_unlock(m_rrt);
			SI_DBG_PRINT(E_SI_ERR_MSG,("RRT failed to alloc rating dimemsion name text!!!\n"));
			return SI_NO_MEMORY;
		}
		SI_memcpy(rrt_region->rrt_dimensions[i].dimension_name_text, current, rrt_region->rrt_dimensions[i].dimension_name_length);
		current += rrt_region->rrt_dimensions[i].dimension_name_length;
		rrt_region->rrt_dimensions[i].graduated_scale = SI_Construct_Data(current,
														RRT_GRADUATED_SCALE_BYTE_INDX,
														RRT_GRADUATED_SCALE_BYTE_NUM,
														RRT_GRADUATED_SCALE_SHIFT,
														RRT_GRADUATED_SCALE_MASK);
		rrt_region->rrt_dimensions[i].values_defined = SI_Construct_Data(current,
														RRT_VALUES_DEFINED_BYTE_INDX,
														RRT_VALUES_DEFINED_BYTE_NUM,
														RRT_VALUES_DEFINED_SHIFT,
														RRT_VALUES_DEFINED_MASK);
		current += RRT_VALUES_DEFINED_BYTE_INDX+RRT_VALUES_DEFINED_BYTE_NUM;

		/* debug print for dimension name!!! */
		SI_DBG_PRINT(E_SI_DBG_MSG,("RRT dimension name: "));
		for (k=0; k<rrt_region->rrt_dimensions[i].dimension_name_length; k++)
			if (k>7)
				SI_DBG_PRINT(E_SI_DBG_MSG,("%c", (char)rrt_region->rrt_dimensions[i].dimension_name_text[k]));
		SI_DBG_PRINT(E_SI_DBG_MSG,("\n"));

		/* create the values structure. */
		rrt_region->rrt_dimensions[i].rrt_values = (SI_RRT_VALUE *)SI_alloc(rrt_region->rrt_dimensions[i].values_defined*sizeof(SI_RRT_VALUE));
		if (rrt_region->rrt_dimensions[i].rrt_values == NULL)
		{
			SI_mutex_unlock(m_rrt);
			SI_DBG_PRINT(E_SI_ERR_MSG,("RRT failed to alloc rating region value structure!!!\n"));
			return SI_NO_MEMORY;
		}

		/* go through all values. */
		for (j=0; j<rrt_region->rrt_dimensions[i].values_defined; j++)
		{
			/* abbrev rating value text. */
			rrt_region->rrt_dimensions[i].rrt_values[j].abbrev_rating_value_length =
						SI_Construct_Data(current,
											RRT_ABBREV_VALUE_LENGTH_BYTE_INDX,
											RRT_ABBREV_VALUE_LENGTH_BYTE_NUM,
											RRT_ABBREV_VALUE_LENGTH_SHIFT,
											RRT_ABBREV_VALUE_LENGTH_MASK);
			current += RRT_ABBREV_VALUE_LENGTH_BYTE_INDX+RRT_ABBREV_VALUE_LENGTH_BYTE_NUM;
			rrt_region->rrt_dimensions[i].rrt_values[j].abbrev_rating_value_text = (char *)SI_alloc(rrt_region->rrt_dimensions[i].rrt_values[j].abbrev_rating_value_length);
			if (rrt_region->rrt_dimensions[i].rrt_values[j].abbrev_rating_value_text == NULL)
			{
				SI_mutex_unlock(m_rrt);
				SI_DBG_PRINT(E_SI_ERR_MSG,("RRT failed to alloc abbrev rating value text!!!\n"));
				return SI_NO_MEMORY;
			}
			SI_memcpy(rrt_region->rrt_dimensions[i].rrt_values[j].abbrev_rating_value_text, current,
				rrt_region->rrt_dimensions[i].rrt_values[j].abbrev_rating_value_length );
			current += rrt_region->rrt_dimensions[i].rrt_values[j].abbrev_rating_value_length;

			/* full rating value text. */
			rrt_region->rrt_dimensions[i].rrt_values[j].rating_value_length =
						SI_Construct_Data(current,
											RRT_VALUE_LENGTH_BYTE_INDX,
											RRT_VALUE_LENGTH_BYTE_NUM,
											RRT_VALUE_LENGTH_SHIFT,
											RRT_VALUE_LENGTH_MASK);
			current += RRT_VALUE_LENGTH_BYTE_NUM;
			rrt_region->rrt_dimensions[i].rrt_values[j].rating_value_text = (char *)SI_alloc(rrt_region->rrt_dimensions[i].rrt_values[j].rating_value_length);
			if (rrt_region->rrt_dimensions[i].rrt_values[j].rating_value_text == NULL)
			{
				SI_mutex_unlock(m_rrt);
				SI_DBG_PRINT(E_SI_ERR_MSG,("RRT failed to alloc rating value text!!!\n"));
				return SI_NO_MEMORY;
			}
			SI_memcpy(rrt_region->rrt_dimensions[i].rrt_values[j].rating_value_text, current,
				rrt_region->rrt_dimensions[i].rrt_values[j].rating_value_length );
			current += rrt_region->rrt_dimensions[i].rrt_values[j].rating_value_length;

			/* debug print for rate value!!! */
			for (k=0; k<rrt_region->rrt_dimensions[i].rrt_values[j].abbrev_rating_value_length; k++)
				if (k>7)
					SI_DBG_PRINT(E_SI_DBG_MSG,("%c", (char)rrt_region->rrt_dimensions[i].rrt_values[j].abbrev_rating_value_text[k]));
			SI_DBG_PRINT(E_SI_DBG_MSG,(",   "));
			for (k=0; k<rrt_region->rrt_dimensions[i].rrt_values[j].rating_value_length; k++)
				if (k>7)
					SI_DBG_PRINT(E_SI_DBG_MSG,("%c", (char)rrt_region->rrt_dimensions[i].rrt_values[j].rating_value_text[k]));
			SI_DBG_PRINT(E_SI_DBG_MSG,("\n"));

		}
	}

	/* unlock mutex for data access. */
	SI_mutex_unlock(m_rrt);


	/* we only process stuffing descriptor for now. */
	tot_desc_len = SI_Construct_Data(current,
					RRT_DESCRIPTORS_LENGTH_BYTE_INDX,
					RRT_DESCRIPTORS_LENGTH_BYTE_NUM,
					RRT_DESCRIPTORS_LENGTH_SHIFT,
					RRT_DESCRIPTORS_LENGTH_MASK);
	current += RRT_DESCRIPTORS_LENGTH_BYTE_INDX+RRT_DESCRIPTORS_LENGTH_BYTE_NUM;

	offset = 0;
	while (offset < tot_desc_len)
	{
		if (*(current++) != SI_DESC_STUFFING)
			SI_DBG_PRINT(E_SI_WRN_MSG,("RRT descriptor %x received! Ignoring!\n", desc_tag));

		desc_len = *(current++);
		current += desc_len;
		offset += desc_len+2;
	}

	if (offset != tot_desc_len)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("RRT Table section descriptor length error!!!\n"));
		return SI_DESCRIPTOR_ERROR;
	}

	temp = (unsigned long)(current - rrt_table);
	if (temp != section_length-SI_CRC_LENGTH)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("RRT Table section length error!!!\n"));
		return SI_SECTION_LENGTH_ERROR;
	}

	return SI_SUCCESS;
}
