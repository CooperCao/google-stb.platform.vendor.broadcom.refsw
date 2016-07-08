/****************************************************************************
 *     Copyright (c) 1999-2016, Broadcom Corporation
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
 * Date:           Generated on               Mon Feb  8 12:53:15 2016
 *                 Full Compile MD5 Checksum  7c463a9180016920b3e03273285ff33d
 *                     (minus title and desc)
 *                 MD5 Checksum               30fed0099690880293569d98807ed1d8
 *
 * lock_release:   n/a
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     749
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *                 Script Source              /tools/dvtsw/current/Linux/combo_header.pl
 *                 DVTSWVER                   current
 *
 *
 ***************************************************************************/

#ifndef BCHP_AVS_CPU_DATA_MEM_H__
#define BCHP_AVS_CPU_DATA_MEM_H__

/***************************************************************************
 *AVS_CPU_DATA_MEM - CPU Data Memory
 ***************************************************************************/

/***************************************************************************
 *WORD%i - Data RAM
 ***************************************************************************/
#define BCHP_AVS_CPU_DATA_MEM_WORDi_ARRAY_BASE                     0x004c4000
#define BCHP_AVS_CPU_DATA_MEM_WORDi_ARRAY_START                    0
#define BCHP_AVS_CPU_DATA_MEM_WORDi_ARRAY_END                      767
#define BCHP_AVS_CPU_DATA_MEM_WORDi_ARRAY_ELEMENT_SIZE             32

/***************************************************************************
 *WORD%i - Data RAM
 ***************************************************************************/
/* AVS_CPU_DATA_MEM :: WORDi :: WORD [31:00] */
#define BCHP_AVS_CPU_DATA_MEM_WORDi_WORD_MASK                      0xffffffff
#define BCHP_AVS_CPU_DATA_MEM_WORDi_WORD_SHIFT                     0


#endif /* #ifndef BCHP_AVS_CPU_DATA_MEM_H__ */

/* End of File */
