/***************************************************************************
*     Copyright (c) 2003-2010, Broadcom Corporation
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
 
#ifndef AUDIO_DSP_HOST_COMMON_H__
#define AUDIO_DSP_HOST_COMMON_H__

/* Section load address's for Host pre-boot initialization of DSP local memory, (word address) */
/* These values must match those specified in linker build scripts */
#define HOST_APP_IBOOT_LOAD_ADDRESS			0x0000  /* Load address in DSP IMEM for 'iboot' interrupt vector section */
#define HOST_APP_TEXT_LOAD_ADDRESS			0x0100  /* Load address in DSP IMEM for 'text' scheduler executable section */
#define HOST_APP_DATA_LOAD_ADDRESS			0x0000  /* Load address in DSP DMEM for 'data' scheduler data section */

#define BAF_INVALID							0xffff
#define BAF_ALGORITHM_INVALID				BAF_INVALID
#define BAF_FRAME_SYNC_INVALID				BAF_INVALID
#define BAF_FRAME_CONTINUATION_INVALID		BAF_INVALID
#define BAF_PP_INVALID						BAF_INVALID

#if (BRAP_DVD_FAMILY == 1)

/* decode algorithm support */
#define BAF_ALGORITHM_MPEG_L1				0x00
#define BAF_ALGORITHM_MPEG_L2				0x01
#define BAF_ALGORITHM_MP3					0x02
#define BAF_ALGORITHM_AAC					0x03
#define BAF_ALGORITHM_AAC_HE				0x04
#define BAF_ALGORITHM_AAC_HE_1				0x05
#define BAF_ALGORITHM_AC3					0x06
#define BAF_ALGORITHM_DDP					0x07
#define BAF_ALGORITHM_DDP_1					0x08
#define BAF_ALGORITHM_DD_LOSSLESS			BAF_ALGORITHM_DDP
#define BAF_ALGORITHM_DD_LOSSLESS_1			BAF_ALGORITHM_DDP_1

#define BAF_ALGORITHM_LPCM_CUSTOM           0x0A
#define BAF_ALGORITHM_BD_LPCM				0x0B

#define BAF_ALGORITHM_LPCM_HDDVD			0x0C
#define BAF_ALGORITHM_MPEG_MC				0x0E

/* Support for Encoder Scheduler */
#define BAF_ALGORITHM_ENCODER_SCHEDULER  0x0F

#define BAF_ALGORITHM_LPCM_DVD				0x10
#define BAF_ALGORITHM_WMA_STD				0x11
#define BAF_ALGORITHM_MLP					0x12

#define BAF_ALGORITHM_DDP_7_1				0x13
#define BAF_ALGORITHM_DDP_7_1_1				0x14

#define BAF_ALGORITHM_DTS_LBR               0x15 
#define BAF_ALGORITHM_DTS_LBR_1             0x16 
#define BAF_ALGORITHM_DTS_LBR_2             0x17
/* BAF_ALGORITHM_DTS_ENC 0x18 is in brap_hostencoderinterface.h. Should be moved here*/
#define BAF_ALGORITHM_DTS					0x19              /* Common for DTS-HD and DTS */
#define BAF_ALGORITHM_DTS_0					BAF_ALGORITHM_DTS	/* Common for DTS-HD and DTS */
#define BAF_ALGORITHM_DTS_1					0x1A	/* Common for DTS-HD and DTS */
#define BAF_ALGORITHM_DTSHD_0				BAF_ALGORITHM_DTS_0
#define BAF_ALGORITHM_DTSHD_1				BAF_ALGORITHM_DTS_1
#define BAF_ALGORITHM_DTSHD_2				0x1B
#define BAF_ALGORITHM_DTSHD_3				0x1C
#define BAF_ALGORITHM_DTSHD_4				0x1D

#define BAF_ALGORITHM_DTSHD_SUB_0			0x1E
#define BAF_ALGORITHM_DTSHD_SUB_1			0x1F
#define BAF_ALGORITHM_DVD_A_LPCM            0x20

#define BAF_ALGORITHM_WMA_PRO_0				0x21
#define BAF_ALGORITHM_WMA_PRO_1				0x22

#define BAF_ALGORITHM_AACHE_0		        0x23
#define BAF_ALGORITHM_AACHE_1		        0x24
#define BAF_ALGORITHM_AACHE_2		        0x25
#define BAF_ALGORITHM_AACHE_3		        0x26
#define BAF_ALGORITHM_AACHE_4		        0x27


#else

/* decode algorithm support */
#define BAF_ALGORITHM_MPEG_L1				0x00
#define BAF_ALGORITHM_MPEG_L2				0x01
#define BAF_ALGORITHM_MP3					0x02
#define BAF_ALGORITHM_AAC					0x03
#define BAF_ALGORITHM_AAC_HE				0x04
#define BAF_ALGORITHM_AAC_HE_1				0x05
#define BAF_ALGORITHM_AC3					0x06
#define BAF_ALGORITHM_DDP					0x07
#define BAF_ALGORITHM_DDP_1					0x08
/* BAF_ALGORITHM_DTS and BAF_ALGORITHM_DTSHD below are kept for old fw executables. 
   For new executables that has support for extensions, please use 
   BAF_ALGORITHM_DTS_x and BAF_ALGORITHM_DTSHD_x */
#define BAF_ALGORITHM_DTS					0x09              /* Common for DTS-HD and DTS */
#define BAF_ALGORITHM_DTSHD					BAF_ALGORITHM_DTS /* Common for DTS-HD and DTS */
#define BAF_ALGORITHM_LPCM_CUSTOM           0x0A
#define BAF_ALGORITHM_BD_LPCM				0x0B
/*#define BAF_ALGORITHM_DV25					0x0C
#define BAF_ALGORITHM_LPCM_HDDVD			0x0C
#define BAF_ALGORITHM_PCM_PLAYBACK			0x0D
#define BAF_ALGORITHM_AUTO_DETECT			0x0E*/
#define BAF_ALGORITHM_LPCM_HDDVD			0x0C
/*#define BAF_ALGORITHM_MPEG_MC				0x0E*/
#define BAF_ALGORITHM_LPCM_DVD				0x10
#define BAF_ALGORITHM_WMA_STD				0x11
#define BAF_ALGORITHM_DD_LOSSLESS			BAF_ALGORITHM_DDP
#define BAF_ALGORITHM_DD_LOSSLESS_1			BAF_ALGORITHM_DDP_1
#define BAF_ALGORITHM_MLP					0x12

#if ((BCHP_CHIP == 7401) || (BCHP_CHIP == 7118) || (BCHP_CHIP == 7403) || (BCHP_CHIP == 7400) || (BCHP_CHIP == 7405) || (BCHP_CHIP == 7325) || (BCHP_CHIP == 7335))
#define BAF_ALGORITHM_WMA_PRO_0				0x0D
#define BAF_ALGORITHM_WMA_PRO_1				0x0E
#endif

#ifndef BCHP_7411_VER /* For chips other than 7411 */
/* Support for Encoder Scheduler */
#define BAF_ALGORITHM_ENCODER_SCHEDULER  0x0F
/*#define BAF_ALGORITHM_ENCODER_SCHEDULER  (BAF_HOST_MAX_DL_MODULE - 1)*/
#endif

#if (BCHP_CHIP == 7401) || (BCHP_CHIP == 7403)|| (BCHP_CHIP == 7118)|| (BCHP_CHIP == 7400)
#define BAF_ALGORITHM_AACHE_0		0x13
#define BAF_ALGORITHM_AACHE_1		0x14
#define BAF_ALGORITHM_AACHE_2		0x15
#define BAF_ALGORITHM_AACHE_3		0x16
#define BAF_ALGORITHM_AACHE_4		0x17
#else
#define BAF_ALGORITHM_AAC_HE_LPSBR_0		0x16
#define BAF_ALGORITHM_AAC_HE_LPSBR_1		0x17
#define BAF_ALGORITHM_AAC_HE_LPSBR_2		0x18
#endif

#define BAF_ALGORITHM_DDP_7_1				0x13
#define BAF_ALGORITHM_DDP_7_1_1				0x14


#define BAF_ALGORITHM_DTS_0					0x19	/* Common for DTS-HD and DTS */
#define BAF_ALGORITHM_DTS_1					0x1A	/* Common for DTS-HD and DTS */
#define BAF_ALGORITHM_DTSHD_0				BAF_ALGORITHM_DTS_0
#define BAF_ALGORITHM_DTSHD_1				BAF_ALGORITHM_DTS_1
#define BAF_ALGORITHM_DTSHD_2				0x1B
#define BAF_ALGORITHM_DTSHD_3				0x1C
#define BAF_ALGORITHM_DTSHD_4				0x1D
#define BAF_ALGORITHM_PCM_PASSTHRU          0x1E
#define BAF_ALGORITHM_DRA   				0x1F
#define BAF_ALGORITHM_PCMWAV                0x20

#endif

/* frame sync support */
#define BAF_FRAME_SYNC_MPEG					0
#define BAF_FRAME_SYNC_DD_LOSSLESS			1
#define BAF_FRAME_SYNC_AAC					2
#define BAF_FRAME_SYNC_WMA_STD				3
#define BAF_FRAME_SYNC_AC3					4
#define BAF_FRAME_SYNC_DTS_HD_HDDVD			5
#define BAF_FRAME_SYNC_AAC_HE				6
#define BAF_FRAME_SYNC_MLP					7
#define BAF_FRAME_SYNC_DDP					8
#define BAF_FRAME_SYNC_MLP_HDDVD			9
#define BAF_FRAME_SYNC_DTS					10
/* Changed for 3563 to handle PCM Capture from external input & subsequent PP cases */
/*#define BAF_FRAME_CONTINUATION_DTS			11*/
#define BAF_FRAME_SYNC_PES         			11
#define BAF_FRAME_SYNC_BD_LPCM				12
#define BAF_FRAME_SYNC_DDP_7_1			    13
/*#define BAF_FRAME_CONTINUATION_BD_LPCM		13*/
#define BAF_FRAME_SYNC_HDDVD_LPCM			14
#define BAF_FRAME_SYNC_WMA_PRO				15
/*#define BAF_FRAME_CONTINUATION_HDDVD_LPCM	15*/
#define BAF_FRAME_SYNC_DTSHD					16
#define BAF_FRAME_CONTINUATION_DTSHD		17
#define BAF_FRAME_SYNC_DVD_LPCM				18
#define BAF_FRAME_CONTINUATION_DVD_LPCM		19
#define BAF_FRAME_SYNC_DTS_LBR              20 
#define BAF_FRAME_SYNC_MPEG_MC				21
#define BAF_FRAME_SYNC_DTSHD_SUB            22 
#define BRAF_FRAME_SYNC_DVD_A_LPCM          23
#define BAF_FRAME_SYNC_DTSHD_NBC_CORE       24
#define BAF_FRAME_SYNC_DTSHD_STG1           25
#define BAF_FRAME_SYNC_DRA					26
#define BAF_FRAME_PCM_PASSTHRU				27
#define BAF_FRAME_SYNC_PCMWAV				28

/* SPDIF parser support */
#define BAF_PASS_THRU_MPEG				0
#define BAF_PASS_THRU_AAC					1
#define BAF_PASS_THRU_AC3					2
#define BAF_PASS_THRU_AAC_HE				3
#define BAF_PASS_THRU_DDP					4
#define BAF_PASS_THRU_DTS					5
#define BAF_PASS_THRU_HDDVD_LPCM               6
#define BAF_CONVERT_DDP					7

#define BAF_PASS_THRU_DTSHD	                BAF_PASS_THRU_DTS /* Common for DTS-HD and DTS */
#define BAF_PASS_THRU_DD_LOSSLESS		BAF_PASS_THRU_DDP
#define BAF_PASS_THRU_MPEG_MC		8
#define BAF_PASS_THRU_WMA_PRO		9

#define BAF_PASS_THRU_MLP			10
#define BAF_PASS_THRU_DDP_7_1		11

#define BAF_PASS_THRU_DTSHD_HBR	       12
#define BAF_PASS_THRU_DRA				13

/* post processing support */ 
#define BAF_PP_SRS_TRUSURROUND			0
#define BAF_PP_SRC							1
#define BAF_PP_KARAOKE						2
#define BAF_PP_DDBM						2
#define BAF_PP_DOWNMIX					3
#define BAF_PP_DOWNMIX_USER1				4
#define BAF_PP_DOWNMIX_AAC				5
#define BAF_PP_DOWNMIX_AAC_USER1			6
#define BAF_PP_DOWNMIX_AC3				7
#define BAF_PP_KARAOKE_UNAWARE			8
#define BAF_PP_CUSTOM_SURROUND                         8
#define BAF_PP_KARAOKE_AWARE				9
#define BAF_PP_CUSTOM_BASS                                 9
#define BAF_PP_KARAOKE_CAPABLE			10
#define BAF_PP_CUSTOM_VOICE                      10
#define BAF_PP_KARAOKE_USER1				11
#define BAF_PP_PEQ                                        11
#define BAF_PP_DOWNMIX_DD_LOSSLESS		BAF_PP_DOWNMIX_AC3
#define BAF_PP_AVL                                         12
#define BAF_PP_PLII                                         13
#define BAF_PP_XEN					             14
#define BAF_PP_BBE					             15
#define BAF_PP_DSOLA                                     16
#define BAF_PP_DTS_NEO                                 17
#define BAF_PP_AD_FADECTRL                  18
#define BAF_PP_AD_PANCTRL                   19
#define BAF_PP_DOBLYVOLUME							   20
#define BAF_PP_SRSTRUVOL					   21

/* Table support for AAC decode processing */
#define BAF_AAC_TABLE_SWBTABLE				0
#define BAF_AAC_TABLE_HUFFMAN				1
#define BAF_AAC_TABLE_PRETWIDDLE			2
#define BAF_AAC_TABLE_IQTSCL				3
#define BAF_AAC_TABLE_TWIDDLE				4
#define BAF_AAC_TABLE_WINCOEF				5
#define BAF_AAC_TABLE_DELAY					6

/* Table support for AC3 decode processing */
#define BAF_AC3_TABLE_IMDCT					0
#define BAF_AC3_TABLE_16_BITS				1
#define BAF_AC3_TABLE_DELAY					2
#define BAF_AC3_TABLE_ASSEMBLY_LABEL		3

/* Table support for MPEG decode processing */
#define BAF_MPEG_TABLE_ASSEMBLY_LABEL		0
#define BAF_MPEG_TABLE_COMMON				1
#define BAF_MPEG_TABLE_PARSER_L1L2			2
#define BAF_MPEG_TABLE_PARSER_L3LSF			3
#define BAF_MPEG_TABLE_HUFFMAN				4
#define BAF_MPEG_TABLE_HUFFMAN_DATA			5
#define BAF_MPEG_TABLE_HYBRID				6
#define BAF_MPEG_TABLE_SUBBAND				7
#define BAF_MPEG_TABLE_DELAY				8
#define BAF_MPEG_TABLE_STRUCT				9

/* Table support for SRS */
#define BAF_PP_SRS_TABLE_TRU_SURROUND		0
#define BAF_PP_SRS_TABLE_FILTER_COEFF		1 
#define BAF_PP_SRS_TABLE_FILTER_STATES		2

#endif

