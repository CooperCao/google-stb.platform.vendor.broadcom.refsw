/***************************************************************************
 *     Copyright (c) 1999-2012, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *
 * THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 * AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 * EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *                     DO NOT EDIT THIS FILE DIRECTLY
 *
 * This module was generated magically with RDB from a source description
 * file. You must edit the source file for changes to be made to this file.
 *
 *
 * Date:           Generated on         Fri Jul 20 11:24:20 2012
 *                 MD5 Checksum         d41d8cd98f00b204e9800998ecf8427e
 *
 * Compiled with:  RDB Utility          combo_header.pl
 *                 RDB Parser           3.0
 *                 unknown              unknown
 *                 Perl Interpreter     5.008008
 *                 Operating System     linux
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef BCHP_SDS_FEC_3_H__
#define BCHP_SDS_FEC_3_H__

/***************************************************************************
 *SDS_FEC_3 - SDS FEC Register Set 3
 ***************************************************************************/
#define BCHP_SDS_FEC_3_FECTL                     0x000d2440 /* Reed-Solomon Control Register */
#define BCHP_SDS_FEC_3_FSYN                      0x000d2444 /* RS sync acquisition/retention total and bad headers */
#define BCHP_SDS_FEC_3_FRS                       0x000d2448 /* RS sync retention total and bad blocks and sync inverse max */
#define BCHP_SDS_FEC_3_FMOD                      0x000d244c /* RS Mode */
#define BCHP_SDS_FEC_3_FERR                      0x000d2450 /* RS correctable and uncorrectable blocks */
#define BCHP_SDS_FEC_3_FRSV                      0x000d2454 /* RS Digicipher II packet header reserved words */

#endif /* #ifndef BCHP_SDS_FEC_3_H__ */

/* End of File */
