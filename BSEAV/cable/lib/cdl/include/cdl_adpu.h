/***************************************************************************
 *     (c)2007-2008 Broadcom Corporation
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
 * Module Description:  OpenCable Common Download storage interface. Functions
 *                      in this module provides CVT interface
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef CDL_ADPU_H
#define CDL_ADPU_H
#ifdef MPOD_SUPPORT
#include "mpod_download.h"
#endif

struct cdl_host_info {
	unsigned long supported_dl_type;
	void *host_info;
};

struct cdl_cvt_recv {
	void *data;
	unsigned long len;
};

typedef struct
{
	unsigned char	tag;
	unsigned long	len;
	unsigned char	data[128];
} DL_DESCRIPTOR;


/*
 * CVT
 */

/* download type */
typedef enum
{
	INBAND_FAT = 0,
	DSG_DSM_CC,
	DOCSIS_TFTP
} E_CVT_DL_TYPE;

/* download command */
typedef enum
{
	DOWNLOAD_NOW = 0,
	DOWNLOAD_DEFER
} E_CVT_DL_COMMAND;

/* download command */
typedef enum
{
	XPORT_DOCSIS = 0,
	XPORT_Q64,
	XPORT_Q256
} E_DL_TRANSPORT_VAL;


typedef struct
{
	unsigned short	freq;
	unsigned char	mod_type;
	unsigned short	pid;
} CVT_LOCATION_TYPE1;

typedef struct
{
	unsigned short	freq;
	unsigned char	mod_type;
	unsigned short	program_number;
} CVT_LOCATION_TYPE2;

typedef struct
{
	unsigned char	dsg_tunnel_addr[6];
	unsigned char	src_ip_addr[16];
	unsigned char	dst_ip_addr[16];
	unsigned short	src_port;
	unsigned short	dst_port;
} CVT_LOCATION_TYPE3;

typedef struct
{
	unsigned char	location_type;
	union
	{
		unsigned short	source_id; /* location type 0 */
		CVT_LOCATION_TYPE1	type1;
		CVT_LOCATION_TYPE2 type2;
	} location_data;
} CVT_DOWNLOAD_TYPE0;


typedef struct
{
	unsigned char	location_type;
	union
	{
		CVT_LOCATION_TYPE3	type3;
		unsigned short	application_id; /* location type 4 */
	} location_data;
} CVT_DOWNLOAD_TYPE1;

/*
 * DT_CVT_OBJ is basically a CVT1, except object type.
 */
typedef struct
{
	E_CVT_DL_TYPE		download_type;		/* 4 bits */
	E_CVT_DL_COMMAND	download_command;	/* 4 bits */

	union
	{
		CVT_DOWNLOAD_TYPE0 type0;
		CVT_DOWNLOAD_TYPE1 type1;
		char	 tftp_server_addr[128]; /* type 2 */
	} download_data;

	unsigned short		freq;
	E_DL_TRANSPORT_VAL	transport_val;
	unsigned short		pid; /* 13 bits */

    unsigned short      obj_type; /* only for cvt2 v2*/
	unsigned char		obj_data_length; /* only for cvt2 v2*/
	unsigned char		obj_data[128]; /* only for cvt2 v2*/

	unsigned char		code_filename_length;
	unsigned char		code_filename[128];
} DL_CVT_OBJ;

/* set the number of objects to a  reasonable value  */
#define CDL_ADPU_NUM_OBJ 10

typedef struct
{
    unsigned char       type;  
	unsigned char		protocol_version;
	unsigned char		ccc; /* configuration_count_change */
	unsigned char		num_descriptors;
	DL_DESCRIPTOR		descriptor[128];
    
	unsigned char		num_obj; /* number of objects */
    DL_CVT_OBJ          obj[CDL_ADPU_NUM_OBJ];

	unsigned char		num_cvc;
#define CVT_MANUF_CVC 0x00
#define CVT_COSIGNER_CVC 0x01
	/* the following is not part of CVT ADPU packet, the raw pointer is copied
	 * to the cvc instead */
	unsigned char *        cvc;
	unsigned long           cvc_len;

} DL_CVT2;
/*
 * Host Download Command: 2-way inband
 */

typedef enum
{
	USE_SOURCE_ID = 0,
	USE_CVDT
} E_DL_LOCATION_TYPE;

typedef enum
{
	USE_PID = 0,
	USE_PROG_NUM
} E_STREAM_ID;

#ifdef __cplusplus
extern "C" {
#endif

int cdl_adpu_init(void);
int cdl_adpu_uninit(void);
int cdl_adpu_cvt_type1_process(void * unused1, unsigned char *data, unsigned long len);
int cdl_adpu_cvt_type2_process(void * unused1, unsigned char *data, unsigned long len);
int cdl_adpu_cvt_wait(void);
int cdl_adpu_get_host_info(void * unused1, void *cdl_host_info) ;
#ifdef MPOD_SUPPORT
void cdl_adpu_send_host_download_control(B_MPOD_DL_HOST_COMMAND command);
#endif
cdl_adpu_cvt_t cdl_adpu_cvt_get_next_object(void);
#ifdef __cplusplus
}
#endif

#endif  /* CDL_ADPU_CVT_H */
