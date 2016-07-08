/****************************************************************************
 *     Copyright (c) 1999-2015, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *
 * THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 * AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 * EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * Module Description:
 *                     DO NOT EDIT THIS FILE DIRECTLY
 *
 * This module was generated magically with RDB from a source description
 * file. You must edit the source file for changes to be made to this file.
 *
 *
 * Date:           Generated on               Mon Aug 24 11:29:32 2015
 *                 Full Compile MD5 Checksum  cecd4eac458fcdc4b77c82d0630f17be
 *                     (minus title and desc)
 *                 MD5 Checksum               c9a18191e1cdbfad4487ef21d91e95fc
 *
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     126
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *                 Script Source              /tools/dvtsw/current/Linux/combo_header.pl
 *                 DVTSWVER                   current
 *
 *
 ***************************************************************************/

#ifndef BCHP_SCPU_GLOBALRAM_H__
#define BCHP_SCPU_GLOBALRAM_H__

/***************************************************************************
 *SCPU_GLOBALRAM - Global 1Kbyte Data RAM
 ***************************************************************************/

/***************************************************************************
 *DMEM%i - Global 1K Data Memory Address 0..255
 ***************************************************************************/
#define BCHP_SCPU_GLOBALRAM_DMEMi_ARRAY_BASE                       0x20310000
#define BCHP_SCPU_GLOBALRAM_DMEMi_ARRAY_START                      0
#define BCHP_SCPU_GLOBALRAM_DMEMi_ARRAY_END                        255
#define BCHP_SCPU_GLOBALRAM_DMEMi_ARRAY_ELEMENT_SIZE               32

/***************************************************************************
 *DMEM%i - Global 1K Data Memory Address 0..255
 ***************************************************************************/
/* SCPU_GLOBALRAM :: DMEMi :: DATA [31:00] */
#define BCHP_SCPU_GLOBALRAM_DMEMi_DATA_MASK                        0xffffffff
#define BCHP_SCPU_GLOBALRAM_DMEMi_DATA_SHIFT                       0


#endif /* #ifndef BCHP_SCPU_GLOBALRAM_H__ */

/* End of File */
