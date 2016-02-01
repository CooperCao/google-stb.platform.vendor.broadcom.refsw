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
#include "si_descriptors.h"


static SI_NTT_SNS_LINK * SI_NTT_SNS_Create_Link (void);
static SI_RET_CODE SI_NTT_SNS_Free_Link(SI_NTT_SNS_LINK *sns);
static SI_RET_CODE SI_NTT_SNS_Ins_Link(SI_NTT_SNS_LINK *new_sns);


struct NTT_SNS_List_Struct NTT_SNS_List;
SI_mutex m_ntt_sns;

void SI_NTT_SNS_Init(void)
{
	SI_LST_D_INIT(&NTT_SNS_List);
	SI_mutex_init(m_ntt_sns);
}

/*********************************************************************
 Function : SI_NTT_SNS_Create_Link
 Description : Function to allocate the space for an SNS link.
 Input : none.
 Output : pointer to the SNS link structure allocated. Will
			return NULL if out of memory.
**********************************************************************/
static SI_NTT_SNS_LINK * SI_NTT_SNS_Create_Link (void)
{
	SI_NTT_SNS_LINK * sns;

	sns = (SI_NTT_SNS_LINK *)SI_alloc(sizeof(SI_NTT_SNS_LINK));
	if (sns == NULL)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("Failed to allocate an NTT SNS link!!!\n"));
		return NULL;
	}

	SI_LST_D_INIT_ENTRY(&(sns->sns_link));
	sns->source_name = (unsigned char *)0;

	return sns;
}


/*********************************************************************
 Function : SI_NTT_SNS_Free_Link
 Description : Function to free an NTT SNS link from the NTT SNS
 				list. the link structure will be freed but
 				not removed from the link list. WE ASSUME THAT WHEN
 				CALLING THIS FUNCTION, THE LINK HAS NOT BEEN ADDED
 				TO THE LIST YET!!!
 Input : SI_NTT_SNS_LINK *sns.	pointer to  NTT SNS link
 			structure to be freed.
 Output : SI_RET_CODE.
**********************************************************************/
static SI_RET_CODE SI_NTT_SNS_Free_Link(SI_NTT_SNS_LINK *sns)
{
	if (sns)
	{
		if (sns->source_name)
			SI_free(sns->source_name);
		SI_free(sns);
	}

	return SI_SUCCESS;
}


/*********************************************************************
 Function : SI_NTT_SNS_Free_List
 Description : Function to free the whole NTT SNS list.
 Input : None.
 Output : SI_RET_CODE.
**********************************************************************/
SI_RET_CODE SI_NTT_SNS_Free_List(void)
{
	SI_NTT_SNS_LINK *sns;

	SI_mutex_lock(m_ntt_sns);

	while ((sns = SI_LST_D_FIRST(&NTT_SNS_List)))
	{
		SI_LST_D_REMOVE_HEAD(&NTT_SNS_List, sns_link);
		SI_NTT_SNS_Free_Link(sns);
	}

	SI_mutex_unlock(m_ntt_sns);

	return SI_SUCCESS;
}


/*********************************************************************
 Function : SI_NTT_SNS_Ins_Link
 Description : Function to insert an NTT SNS link into the NTT SNS
 				list. The order is source_ID in incrementing order.
 Input : SI_NTT_SNS_LINK *new_sns.	pointer to new NTT SNS link
 			structure to be inserted.
 Output : SI_RET_CODE.
**********************************************************************/
static SI_RET_CODE SI_NTT_SNS_Ins_Link(SI_NTT_SNS_LINK *new_sns)
{
	SI_NTT_SNS_LINK * sns;
	unsigned char comp;
	int	i;

	SI_mutex_lock(m_ntt_sns);

	sns = SI_LST_D_FIRST(&NTT_SNS_List);
	/* if the list is empty, just put the new sns link in. */
	if (sns == NULL)
	{
		SI_LST_D_INSERT_HEAD(&NTT_SNS_List, new_sns, sns_link);
		SI_mutex_unlock(m_ntt_sns);
		return SI_SUCCESS;
	}

	/* search for the the place to insert. */
	while ( (sns->source_ID < new_sns->source_ID) && SI_LST_D_NEXT(sns, sns_link))
		sns = SI_LST_D_NEXT(sns, sns_link);

	if (sns->source_ID < new_sns->source_ID)
	{
		/* we got to the end of list. insert after current element. */
		SI_LST_D_INSERT_AFTER(sns, new_sns, sns_link);
	}
	else if (sns->source_ID > new_sns->source_ID)
	{
		/* insert before the current element. */
		SI_LST_D_INSERT_BEFORE(&NTT_SNS_List, sns, new_sns, sns_link);
	}
	else
	{
		/* equal! It should only happen when we do not have the revision descriptor,
			simply keep the old one and free the new one. */
		SI_NTT_SNS_Free_Link(new_sns);
	}

	SI_mutex_unlock(m_ntt_sns);

	return SI_SUCCESS;
}
/*********************************************************************
 Function : SI_NTT_SNS_Find_SrcId
 Description : Function to return name based on Source ID
 Input : unsigned short source_id: Source Id to match
         unsigned char *name: channel name
         int max_len: max channel length
 Output : if matched return 0; else return -1;.
**********************************************************************/
int SI_NTT_SNS_Find_SrcId(unsigned short id, unsigned char *name, int max_len)
{
	SI_NTT_SNS_LINK * sns;

	SI_mutex_lock(m_ntt_sns);

	sns = SI_LST_D_FIRST(&NTT_SNS_List);
	if (sns == NULL)
	{
		SI_mutex_unlock(m_ntt_sns);
		return -1;
	}

	while ( sns ) {
		if (sns->source_ID  == id) break;
		sns = SI_LST_D_NEXT(sns, sns_link);
	}

	/* TODO::only handle ASCII MTS for the time being*/
	if (sns == NULL || sns->source_ID != id || sns->source_name[0] != 0) {
		SI_mutex_unlock(m_ntt_sns);
		return -1;
	}
	if (max_len > sns->source_name[1]) max_len = sns->source_name[1];
	memcpy(name, &sns->source_name[2], max_len);
	name[max_len] = 0 ; /* add NULL terminator*/
	SI_mutex_unlock(m_ntt_sns);
	return 0;
}


/*********************************************************************
 Function : SI_NTT_SNS_Pointer
 Description : Function to point past newly received NTT SNS table
 				section inorder to get to table descriptors.
 Input : unsigned char *table : point to the start of SNS table data.
 Output : unsigned char * points to the end of SNS record plus 1 which is the
 					start of NTT table descriptors.
**********************************************************************/
unsigned char * SI_NTT_SNS_Pointer (unsigned char *table)
{
	unsigned char num_sns_rec, name_len;
	unsigned long desc_start, desc_cnt;
	unsigned long desc_tag, desc_len, len, i, j;
	unsigned char *current;

	/* get number of SNS records. */
	num_sns_rec = SI_Construct_Data(table,
				NTT_SNS_NUM_REC_BYTE_INDX,
				NTT_SNS_NUM_REC_BYTE_NUM,
				NTT_SNS_NUM_REC_SHIFT,
				NTT_SNS_NUM_REC_MASK);

	current = table + NTT_SNS_NUM_REC_BYTE_INDX + NTT_SNS_NUM_REC_BYTE_NUM;

	/* go through all the SNS records. */
	for (i=0; i<num_sns_rec; i++)
	{
		name_len = SI_Construct_Data(current,
				NTT_SNS_NAME_LEN_BYTE_INDX,
				NTT_SNS_NAME_LEN_BYTE_NUM,
				NTT_SNS_NAME_LEN_SHIFT,
				NTT_SNS_NAME_LEN_MASK);

		current += NTT_SNS_NAME_LEN_BYTE_INDX+NTT_SNS_NAME_LEN_BYTE_NUM+name_len;

		desc_cnt = SI_Construct_Data(current,
				NTT_SNS_DESC_CNT_BYTE_INDX,
				NTT_SNS_DESC_CNT_BYTE_NUM,
				NTT_SNS_DESC_CNT_SHIFT,
				NTT_SNS_DESC_CNT_MASK);

		current += NTT_SNS_DESC_CNT_BYTE_INDX+NTT_SNS_DESC_CNT_BYTE_NUM;

		/* go through all descriptors. */
		for (j=0; j<desc_cnt; j++)
		{
			desc_tag = *(current++);
			len = *(current++);
			/* update current pointer. */
			current += len;
		}
	}

	return current;
}



/*********************************************************************
 Function : SI_NTT_SNS_Parse
 Description : Function to parse a newly received NTT SNS table section and
 				put it into the NTT SNS link list
 Input : unsigned char *table : point to the start of SNS table data.
 Output : SI_RET_CODE.
**********************************************************************/
SI_RET_CODE SI_NTT_SNS_Parse (unsigned char *table,unsigned int iso639)
{
	unsigned long temp, i, j, offset;
	unsigned char num_sns_rec;
	unsigned long desc_start, desc_cnt;
	unsigned long desc_tag, desc_len, len;
	unsigned char *current;
	SI_NTT_SNS_LINK * sns;
	SI_RET_CODE result;


	SI_DBG_PRINT(E_SI_DBG_MSG,("NTT SNS Table received.\n"));

	/* get number of VC records. */
	num_sns_rec = SI_Construct_Data(table,
				NTT_SNS_NUM_REC_BYTE_INDX,
				NTT_SNS_NUM_REC_BYTE_NUM,
				NTT_SNS_NUM_REC_SHIFT,
				NTT_SNS_NUM_REC_MASK);
	SI_DBG_PRINT(E_SI_DBG_MSG,("NTT SNS num of rec's %x.\n", num_sns_rec));

	current = table + NTT_SNS_NUM_REC_BYTE_INDX + NTT_SNS_NUM_REC_BYTE_NUM;

	/* go through all the SNS records. */
	for (i=0; i<num_sns_rec; i++)
	{
		/* create the sns link struct. */
		if ((sns = SI_NTT_SNS_Create_Link()) == NULL)
		{
			SI_DBG_PRINT(E_SI_ERR_MSG,("NTT SNS cannot create link structure!!!\n"));
			return SI_NO_MEMORY;
		}

		sns->appflag = SI_Construct_Data(current,
						NTT_SNS_APP_TYPE_BYTE_INDX,
						NTT_SNS_APP_TYPE_BYTE_NUM,
						NTT_SNS_APP_TYPE_SHIFT,
						NTT_SNS_APP_TYPE_MASK);

		sns->source_ID = SI_Construct_Data(current,
						NTT_SNS_SOURCE_ID_BYTE_INDX,
						NTT_SNS_SOURCE_ID_BYTE_NUM,
						NTT_SNS_SOURCE_ID_SHIFT,
						NTT_SNS_SOURCE_ID_MASK);

		sns->name_len = SI_Construct_Data(current,
						NTT_SNS_NAME_LEN_BYTE_INDX,
						NTT_SNS_NAME_LEN_BYTE_NUM,
						NTT_SNS_NAME_LEN_SHIFT,
						NTT_SNS_NAME_LEN_MASK);
		sns->source_name = (unsigned char *)SI_alloc(sns->name_len);
		if (sns->source_name == NULL)
		{
			SI_DBG_PRINT(E_SI_ERR_MSG,("Failed to allocate mem for source name!!!\n"));
			return SI_NO_MEMORY;
		}

		current += NTT_SNS_NAME_LEN_BYTE_INDX+NTT_SNS_NAME_LEN_BYTE_NUM;
		SI_memcpy(sns->source_name, current, sns->name_len);

		SI_DBG_PRINT(E_SI_DBG_MSG,("NTT SNS appflag %x, source id %x :\n", sns->appflag, sns->source_ID));
		for (j=0; j<sns->name_len; j++)
		{
			if (j > 1)
				SI_DBG_PRINT(E_SI_DBG_MSG,("%c", (char)sns->source_name[j]));
		}
		SI_DBG_PRINT(E_SI_DBG_MSG,("\n"));

		current += sns->name_len;

		/* get descriptors count. */
		desc_cnt = SI_Construct_Data(current,
					NTT_SNS_DESC_CNT_BYTE_INDX,
					NTT_SNS_DESC_CNT_BYTE_NUM,
					NTT_SNS_DESC_CNT_SHIFT,
					NTT_SNS_DESC_CNT_MASK);
		current += NTT_SNS_DESC_CNT_BYTE_INDX+NTT_SNS_DESC_CNT_BYTE_NUM;

		/* go through all descriptors. */
		for (j=0; j<desc_cnt; j++)
		{
			desc_tag = *(current++);
			len = *(current++);
			switch(desc_tag)
			{
				default:
					SI_DBG_PRINT(E_SI_WRN_MSG,("NTT SNS table descriptor %x received! Ignoring!\n", desc_tag));
				break;
			}

			/* update current pointer. */
			current += len;
		}
		if (sns->appflag) {
			SI_NTT_SNS_Free_Link(sns);
		} else
			/* insert the sns link */
			SI_NTT_SNS_Ins_Link(sns);
	}

	return SI_SUCCESS;
}


