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


#ifndef SI_SVCT_VCM_H
#define SI_SVCT_VCM_H

typedef enum
{
	MPEG2_XPORT,
	NON_MPEG2_XPORT,
} SI_SVCT_VCM_XPORT_TYPE;

typedef enum
{
	NORMAL_CHAN,
	HIDDEN_CHAN,
} SI_SVCT_VCM_SERV_TYPE;

typedef enum
{
	NTSC,
	PAL_625,
	PAL_525,
	SECAM,
	MAC,
} SI_SVCT_VCM_VIDEO_STANDARD;

typedef struct
{
	unsigned short prog_num;
	unsigned char MMS_reference;
} SVCT_VCM_MPEG_PROPERTY;

typedef struct
{
	unsigned char scrambled;
	unsigned char video_standard;
} SVCT_VCM_NONMPEG_PROPERTY;

#define SVCT_VCM_OUT_OF_BAND_MASK		0x04
#define SVCT_VCM_ACCESS_CONTROLED_MASK		0x02
#define SVCT_VCM_HIDE_GUIDE_MASK		0x01

typedef struct _SI_SVCT_VCM_CHANNEL
{
	VirtChanNumMode vcn_mode; 		/* virtual channel number scheme. 0 onpart, 1 twopart. */
	unsigned short channum1; 		/* for one part mode, this is the virtual channel number. for two part, this will be the major part. */
	unsigned short channum2;		/* for one part mode, this is not used. for two part, this is the minor part. */
	unsigned char appflag;			/* 1 for app, 0 for source. */
	unsigned short source_ID;		/* source_ID corresponding to the channel. */
	unsigned char path_select;		/* path_select bit. */
	unsigned char transport_type; 	/* transport type. */
	unsigned char channel_type;		/* channel type. */
	unsigned char CDS_reference;	/* CDS_reference. */
	union
	{
		SVCT_VCM_MPEG_PROPERTY mpeg_prop;
		SVCT_VCM_NONMPEG_PROPERTY nonmpeg_prop;
	} ChanPropUnion;
	unsigned char more_prop;		/* indicator that more info is available through descriptor. */
	unsigned short tsid;			/* transport stream id for this channel. */
	unsigned char serv_type;		/* service type. */
	unsigned char chanbits;			/* includes masks for out_of_band, access_controled, hide_guide. */
	SI_LST_D_ENTRY(_SI_SVCT_VCM_CHANNEL) chan_link;
} SI_SVCT_VCM_CHANNEL;

SI_LST_D_HEAD(svct_vcm_channel_list, _SI_SVCT_VCM_CHANNEL);

/* For the following, refer to table 5.16, 5.17 of ANSI/SCTE65 2002 (DVS234) */
/* for VCM header. relative offset to VCM structure. */
#define SVCT_VCM_DESC_INCL_BYTE_INDX		0
#define SVCT_VCM_DESC_INCL_BYTE_NUM			1
#define SVCT_VCM_DESC_INCL_SHIFT			5
#define SVCT_VCM_DESC_INCL_MASK				0x01

#define SVCT_VCM_ACTIVE_TIME_BYTE_INDX		2
#define SVCT_VCM_ACTIVE_TIME_BYTE_NUM			4
#define SVCT_VCM_ACTIVE_TIME_SHIFT			0
#define SVCT_VCM_ACTIVE_TIME_MASK				0xffffffff

#define SVCT_VCM_NUM_VC_REC_BYTE_INDX		6
#define SVCT_VCM_NUM_VC_REC_BYTE_NUM			1
#define SVCT_VCM_NUM_VC_REC_SHIFT			0
#define SVCT_VCM_NUM_VC_REC_MASK				0xff

/* in the VCM loop, relative to the start of VCM loop. */
#define SVCT_VCM_VC_NUM_BYTE_INDX		0
#define SVCT_VCM_VC_NUM_BYTE_NUM			2
#define SVCT_VCM_VC_NUM_SHIFT			0
#define SVCT_VCM_VC_NUM_MASK				0xfff

#define SVCT_VCM_APP_VIRT_CHAN_BYTE_INDX		2
#define SVCT_VCM_APP_VIRT_CHAN_BYTE_NUM			1
#define SVCT_VCM_APP_VIRT_CHAN_SHIFT			7
#define SVCT_VCM_APP_VIRT_CHAN_MASK				0x1

#define SVCT_VCM_PATH_SEL_BYTE_INDX		2
#define SVCT_VCM_PATH_SEL_BYTE_NUM			1
#define SVCT_VCM_PATH_SEL_SHIFT			5
#define SVCT_VCM_PATH_SEL_MASK				0x1

#define SVCT_VCM_XPORT_TYPE_BYTE_INDX		2
#define SVCT_VCM_XPORT_TYPE_BYTE_NUM			1
#define SVCT_VCM_XPORT_TYPE_SHIFT			4
#define SVCT_VCM_XPORT_TYPE_MASK				0x1

#define SVCT_VCM_CHAN_TYPE_BYTE_INDX		2
#define SVCT_VCM_CHAN_TYPE_BYTE_NUM			1
#define SVCT_VCM_CHAN_TYPE_SHIFT			0
#define SVCT_VCM_CHAN_TYPE_MASK				0xf

#define SVCT_VCM_SOURCE_ID_BYTE_INDX		3
#define SVCT_VCM_SOURCE_ID_BYTE_NUM			2
#define SVCT_VCM_SOURCE_ID_SHIFT			0
#define SVCT_VCM_SOURCE_ID_MASK				0xffff

#define SVCT_VCM_CDS_REF_BYTE_INDX		5
#define SVCT_VCM_CDS_REF_BYTE_NUM			1
#define SVCT_VCM_CDS_REF_SHIFT			0
#define SVCT_VCM_CDS_REF_MASK				0xff

#define SVCT_VCM_PROG_NUM_BYTE_INDX		6
#define SVCT_VCM_PROG_NUM_BYTE_NUM			2
#define SVCT_VCM_PROG_NUM_SHIFT			0
#define SVCT_VCM_PROG_NUM_MASK				0xffff

#define SVCT_VCM_MMS_REF_BYTE_INDX		8
#define SVCT_VCM_MMS_REF_BYTE_NUM			1
#define SVCT_VCM_MMS_REF_SHIFT			0
#define SVCT_VCM_MMS_REF_MASK				0xff

#define SVCT_VCM_SCRAMBLED_BYTE_INDX		6
#define SVCT_VCM_SCRAMBLED_BYTE_NUM			1
#define SVCT_VCM_SCRAMBLED_SHIFT			7
#define SVCT_VCM_SCRAMBLED_MASK				0x1

#define SVCT_VCM_VIDEO_STANDARD_BYTE_INDX		6
#define SVCT_VCM_VIDEO_STANDARD_BYTE_NUM			1
#define SVCT_VCM_VIDEO_STANDARD_SHIFT			0
#define SVCT_VCM_VIDEO_STANDARD_MASK				0xf

#define SVCT_VCM_DESC_CNT_BYTE_INDX		9
#define SVCT_VCM_DESC_CNT_BYTE_NUM			1
#define SVCT_VCM_DESC_CNT_SHIFT			0
#define SVCT_VCM_DESC_CNT_MASK				0xff



#ifdef __cplusplus
extern "C" {
#endif

void SI_SVCT_VCM_Init(void);
unsigned char * SI_SVCT_VCM_Pointer (unsigned char *table);
SI_RET_CODE SI_SVCT_VCM_Parse (unsigned char *table);
SI_RET_CODE SI_SVCT_VCM_Free_List(void);

#ifdef __cplusplus
}
#endif


#endif
