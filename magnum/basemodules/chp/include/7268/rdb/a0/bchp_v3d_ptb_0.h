/****************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * Module Description:
 *                     DO NOT EDIT THIS FILE DIRECTLY
 *
 * This module was generated magically with RDB from a source description
 * file. You must edit the source file for changes to be made to this file.
 *
 *
 * Date:           Generated on               Mon Aug 24 11:29:33 2015
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

#ifndef BCHP_V3D_PTB_0_H__
#define BCHP_V3D_PTB_0_H__

/***************************************************************************
 *V3D_PTB_0 - V3D Primitive Tile Binner Registers
 ***************************************************************************/
#define BCHP_V3D_PTB_0_BPCA                      0x21208300 /* [RO] Current Address of Binning Memory Pool */
#define BCHP_V3D_PTB_0_BPCS                      0x21208304 /* [RO] Remaining Size of Binning Memory Pool */
#define BCHP_V3D_PTB_0_BPOA                      0x21208308 /* [RW] Address of Overspill Binning Memory Block */
#define BCHP_V3D_PTB_0_BPOS                      0x2120830c /* [RW] Size of Overspill Binning Memory Block */
#define BCHP_V3D_PTB_0_BXCF                      0x21208310 /* [WO] Binner Debug */

/***************************************************************************
 *BPCA - Current Address of Binning Memory Pool
 ***************************************************************************/
/* V3D_PTB_0 :: BPCA :: BMPCA [31:00] */
#define BCHP_V3D_PTB_0_BPCA_BMPCA_MASK                             0xffffffff
#define BCHP_V3D_PTB_0_BPCA_BMPCA_SHIFT                            0
#define BCHP_V3D_PTB_0_BPCA_BMPCA_DEFAULT                          0x00000000

/***************************************************************************
 *BPCS - Remaining Size of Binning Memory Pool
 ***************************************************************************/
/* V3D_PTB_0 :: BPCS :: BMPRS [31:00] */
#define BCHP_V3D_PTB_0_BPCS_BMPRS_MASK                             0xffffffff
#define BCHP_V3D_PTB_0_BPCS_BMPRS_SHIFT                            0
#define BCHP_V3D_PTB_0_BPCS_BMPRS_DEFAULT                          0x00000000

/***************************************************************************
 *BPOA - Address of Overspill Binning Memory Block
 ***************************************************************************/
/* V3D_PTB_0 :: BPOA :: BMPOA [31:00] */
#define BCHP_V3D_PTB_0_BPOA_BMPOA_MASK                             0xffffffff
#define BCHP_V3D_PTB_0_BPOA_BMPOA_SHIFT                            0
#define BCHP_V3D_PTB_0_BPOA_BMPOA_DEFAULT                          0x00000000

/***************************************************************************
 *BPOS - Size of Overspill Binning Memory Block
 ***************************************************************************/
/* V3D_PTB_0 :: BPOS :: BMPOS [31:00] */
#define BCHP_V3D_PTB_0_BPOS_BMPOS_MASK                             0xffffffff
#define BCHP_V3D_PTB_0_BPOS_BMPOS_SHIFT                            0
#define BCHP_V3D_PTB_0_BPOS_BMPOS_DEFAULT                          0x00000000

/***************************************************************************
 *BXCF - Binner Debug
 ***************************************************************************/
/* V3D_PTB_0 :: BXCF :: reserved0 [31:02] */
#define BCHP_V3D_PTB_0_BXCF_reserved0_MASK                         0xfffffffc
#define BCHP_V3D_PTB_0_BXCF_reserved0_SHIFT                        2

/* V3D_PTB_0 :: BXCF :: RWORDERDISA [01:01] */
#define BCHP_V3D_PTB_0_BXCF_RWORDERDISA_MASK                       0x00000002
#define BCHP_V3D_PTB_0_BXCF_RWORDERDISA_SHIFT                      1
#define BCHP_V3D_PTB_0_BXCF_RWORDERDISA_DEFAULT                    0x00000000

/* V3D_PTB_0 :: BXCF :: CLIPDISA [00:00] */
#define BCHP_V3D_PTB_0_BXCF_CLIPDISA_MASK                          0x00000001
#define BCHP_V3D_PTB_0_BXCF_CLIPDISA_SHIFT                         0
#define BCHP_V3D_PTB_0_BXCF_CLIPDISA_DEFAULT                       0x00000000

#endif /* #ifndef BCHP_V3D_PTB_0_H__ */

/* End of File */
