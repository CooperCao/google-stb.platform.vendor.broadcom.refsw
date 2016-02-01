/***************************************************************************
*     (c)2008 Broadcom Corporation
*
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,
*  and may only be used, duplicated, modified or distributed pursuant to the terms and
*  conditions of a separate, written license agreement executed between you and Broadcom
*  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
*  no license (express or implied), right to use, or waiver of any kind with respect to the
*  Software, and Broadcom expressly reserves all rights in and to the Software and all
*  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
*  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
*  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
*  Except as expressly set forth in the Authorized License,
*
*  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
*  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
*  and to use this information only in connection with your use of Broadcom integrated circuit products.
*
*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
*  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
*  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
*  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
*  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
*  USE OR PERFORMANCE OF THE SOFTWARE.
*
*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
*  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
*  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
*  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
*  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
*  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
*  ANY LIMITED REMEDY.
*
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* Description: Sample header file for an App Lib
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#ifndef B_Dsmcc_LIB_PRIV_H__
#define B_Dsmcc_LIB_PRIV_H__

#define DSMCC_SECTION_TABLE_ID(a) ((a)->data[0])
#define DSMCC_SECTION_SYNTAX_INDICATOR(a) ((a)->data[1]&0x80)
#define DSMCC_SECTION_SECTION_LEN(a) (uint16_t)((((a)->data[1]&0x0f) << 8) | ((a)->data[2]))
#define DSMCC_SECTION_TABLE_ID_EXT(a) (uint16_t)(((a)->data[3] << 8) | ((a)->data[4]))
#define DSMCC_SECTION_VER(a) ( ((a)->data[5] & 0x3e) >> 1)
#define DSMCC_SECTION_CUR_NEXT_INDICATOR(a) ((a)->data[5] & 0x1)
#define DSMCC_SECTION_SECT_NUM(a) ((a)->data[6])
#define DSMCC_SECTION_LAST_SECT_NUM(a) ((a)->data[7])
#define DSMCC_SECTION_HEADER_LEN    8 

#define DSMCC_SECTION_TABLE_ID_EXT_OFFSET  3

#define DSMCC_TABLE_ID_DII_OR_DSI  0x3b /* DII or DSI message */
#define DSMCC_TABLE_ID_DDB   0x3c
#define MAX_SECTION_LEN 4096
/* 
 * 64k msg buffer causes message overflow when doing on-the-fly cdl, use 256k instead
 * #define MSG_BUFFER_SIZE (4096 * 16)  
 */
#define MSG_BUFFER_SIZE (4096 * 64)  


#define DSMCC_PROTOCOL_DISCRIMINITOR 0x11
#define DSMCC_TYPE   0x03
#define DSMCC_RESERVED 0xff
#define DSMCC_WINDOW_SIZE 0x00
#define DSMCC_ACK_PERIOD 0x00
#define DSMCC_TC_DOWNLOAD_WINDOW 0x00
#define DSMCC_MESSAGE_ID_DII  0x1002
#define DSMCC_MESSAGE_ID_DDB  0x1003




#pragma pack(push, 1)
struct dsmcc_download_data_header { 
	uint8_t protocol_discriminator;
	uint8_t dsmcc_type;
	uint16_t message_id;
	uint32_t download_id;
	uint8_t reserved;
	uint8_t adaptation_length;
	uint16_t message_length;
	/* uint8_t * dsmcc_adaptation_header; */
}; 

struct dsmcc_ddb { 
	struct dsmcc_download_data_header download_data_header;
	uint16_t module_id;
	uint8_t  module_version;
	uint8_t  reserved;
	uint16_t block_number;
	uint8_t * block_data_byte;
};

struct dsmcc_message_header { 
	uint8_t protocol_discriminator;
	uint8_t dsmcc_type;
	uint16_t message_id;
	uint32_t transaction_id;
	uint8_t reserved;
	uint8_t  adaptation_length;
	uint16_t message_length;
	/* uint8_t * dsmcc_adaptation_header; not exist if adaptation_length = 0 */

};

struct dsmcc_module_info {
	uint16_t module_id; /* 16 bits; */
	uint32_t module_size; /* 32 bits */
	uint8_t module_version; /* 8 bits */
	uint8_t module_info_length; /* 8 bits */
	uint8_t * module_info_byte; /* depends on module info length */
};

struct dsmcc_compatibility_descriptor {
	uint16_t length;
	/*	uint16_t info; not exist if length = 0 */
};
struct dsmcc_dii_msg {
	struct dsmcc_message_header dsmcc_message_header; 
	uint32_t download_id; 
	uint16_t block_size; 
	uint8_t window_size; 
	uint8_t ack_period; 
	uint32_t t_download_window; 
	uint32_t t_download_scenario; 

	uint16_t compatibility_descriptor_length; /* for common download, always 0 */

	uint16_t number_of_modules; 
	struct dsmcc_module_info module_info; 
	/* private data not exist for common download */
};


struct dsmcc_section_header {
	uint8_t data[DSMCC_SECTION_HEADER_LEN];
};
#pragma pack(pop)


#define DSMCC_DDB_MODULE_ID_OFFSET         (DSMCC_SECTION_HEADER_LEN +  \
						       sizeof(struct dsmcc_download_data_header))
#define DSMCC_DDB_MODULE_VERSION_OFFSET    (DSMCC_DDB_MODULE_ID_OFFSET + 2)
#define DSMCC_DDB_BLOCK_NUMBER_OFFSET      (DSMCC_DDB_MODULE_ID_OFFSET + 4)
#define DSMCC_DDB_BLOCK_DATA_BYTE_OFFSET   (DSMCC_DDB_MODULE_ID_OFFSET + 6)
#define DSMCC_DDB_MODULE_ID(b)             (*(uint16_t *)((uint8_t *)b + DSMCC_DDB_MODULE_ID_OFFSET))
#define DSMCC_DDB_MODULE_VERSION(b)        (*(uint8_t *) ((uint8_t *)b + DSMCC_DDB_MODULE_VERSION_OFFSET))
#define DSMCC_DDB_BLOCK_NUMBER(b)          (*(uint16_t *)((uint8_t *)b + DSMCC_DDB_BLOCK_NUMBER_OFFSET))
#define DSMCC_DDB_BLOCK_DATA_BYTE(b)       (*(uint8_t *) ((uint8_t *)b + DSMCC_DDB_BLOCK_DATA_BYTE_OFFSET))

struct dsmcc_module_status {
	int module_id;
	uint32_t offset;
	uint16_t next_block;
#define STATE_INIT 0
#define STATE_FILTER_STARTED 1
#define STATE_READ_STARTED 2
#define STATE_DONE 3
	uint8_t state;
};

/***************************************************************************
Summary:
Private handle for Dsmcc App Lib
***************************************************************************/
struct B_Dsmcc_P_Struct;
typedef struct B_Dsmcc_P_Struct
{
    /* raw DII data */
    uint8_t data[MAX_SECTION_LEN];

    /* pointers to DII */
    struct dsmcc_section_header *p_sect; /* pointer to the DSMCC section */
    struct dsmcc_dii_msg * p_dii;
    struct dsmcc_ddb * p_ddb;
    
    struct dsmcc_module_info ** p_module_info; 

     /* pointers for message filtering */
    B_Dsmcc_Settings settings;    
    /* top level status */
    B_Dsmcc_Status * status;
    
    /* private handler for inputs */
    void * priv;

    /* private methods defined in b_dsmcc_lib_priv_xxx.c, called in b_dsmcc_lib.c */
    int (*open)(struct B_Dsmcc_P_Struct * hDsmcc, B_Dsmcc_Settings *pDsmccSettings);
    int (*close)(struct B_Dsmcc_P_Struct * hDsmcc);
    void (*stop)(struct B_Dsmcc_P_Struct * hDsmcc);
    int (*start) (struct B_Dsmcc_P_Struct * hDsmcc, int module_id);
    int (*get_buffer) (struct B_Dsmcc_P_Struct * hDsmcc, unsigned char ** buffer, size_t * size);
    int (*read_complete) (struct B_Dsmcc_P_Struct * hDsmcc, size_t size);

#if 0
    NEXUS_MessageHandle msg;
    BKNI_EventHandle message_ready_event;
#endif
    void *buffer;

    /* downloading status for the modules */
    int current_module;
    struct dsmcc_module_status ** p_module_status; 

} *B_Dsmcc_P_Handle;

    /*
     * API's shared by private functions
     */
struct dsmcc_module_status * get_module_status_from_module_id(B_Dsmcc_P_Handle hDsmcc, int module_id);
struct dsmcc_module_info * get_module_info_from_module_id(B_Dsmcc_P_Handle hDsmcc, int module_id);
void parse_dsmcc_sect(B_Dsmcc_P_Handle h, uint8_t *data);

#endif /* #ifndef B_Dsmcc_LIB_PRIV_H__ */
