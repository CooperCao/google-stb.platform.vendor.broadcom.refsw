/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
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
 * Module Description:
 *                     DO NOT EDIT THIS FILE DIRECTLY
 *
 * This module was generated magically with RDB from a source description
 * file. You must edit the source file for changes to be made to this file.
 *
 * The launch point for all information concerning RDB is found at:
 *   http://bcgbu.broadcom.com/RDB/SitePages/Home.aspx
 *
 * Date:           Generated on               Tue Jun 27 10:52:39 2017
 *                 Full Compile MD5 Checksum  de13a1e8011803b5a40ab14e4d71d071
 *                     (minus title and desc)
 *                 MD5 Checksum               b694fcab41780597392ed5a8f558ad3e
 *
 * lock_release:   r_1255
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     1570
 *                 unknown                    unknown
 *                 Perl Interpreter           5.014001
 *                 Operating System           linux
 *                 Script Source              home/pntruong/sbin/combo_header.pl
 *                 DVTSWVER                   LOCAL home/pntruong/sbin/combo_header.pl
 *
 *
********************************************************************************/

#ifndef BCHP_GIO_AON_H__
#define BCHP_GIO_AON_H__

/***************************************************************************
 *GIO_AON - GPIO
 ***************************************************************************/
#define BCHP_GIO_AON_ODEN_LO                     0x204174c0 /* [RW][32] GENERAL PURPOSE I/O OPEN DRAIN ENABLE FOR AON_GPIO[27:0] */
#define BCHP_GIO_AON_DATA_LO                     0x204174c4 /* [RW][32] GENERAL PURPOSE I/O DATA FOR AON_GPIO[27:0] */
#define BCHP_GIO_AON_IODIR_LO                    0x204174c8 /* [RW][32] GENERAL PURPOSE I/O DIRECTION FOR AON_GPIO[27:0] */
#define BCHP_GIO_AON_EC_LO                       0x204174cc /* [RW][32] GENERAL PURPOSE I/O EDGE CONFIGURATION FOR AON_GPIO[27:0] */
#define BCHP_GIO_AON_EI_LO                       0x204174d0 /* [RW][32] GENERAL PURPOSE I/O EDGE INSENSITIVE FOR AON_GPIO[27:0] */
#define BCHP_GIO_AON_MASK_LO                     0x204174d4 /* [RW][32] GENERAL PURPOSE I/O INTERRUPT MASK FOR AON_GPIO[27:0] */
#define BCHP_GIO_AON_LEVEL_LO                    0x204174d8 /* [RW][32] GENERAL PURPOSE I/O INTERRUPT TYPE FOR AON_GPIO[27:0] */
#define BCHP_GIO_AON_STAT_LO                     0x204174dc /* [RW][32] GENERAL PURPOSE I/O INTERRUPT STATUS FOR AON_GPIO[27:0] */
#define BCHP_GIO_AON_ODEN_EXT                    0x204174e0 /* [RW][32] GENERAL PURPOSE I/O OPEN DRAIN ENABLE FOR AON_SGPIO[5:0] */
#define BCHP_GIO_AON_DATA_EXT                    0x204174e4 /* [RW][32] GENERAL PURPOSE I/O DATA FOR AON_SGPIO[5:0] */
#define BCHP_GIO_AON_IODIR_EXT                   0x204174e8 /* [RW][32] GENERAL PURPOSE I/O DIRECTION FOR AON_SGPIO[5:0] */
#define BCHP_GIO_AON_EC_EXT                      0x204174ec /* [RW][32] GENERAL PURPOSE I/O EDGE CONFIGURATION FOR AON_SGPIO[5:0] */
#define BCHP_GIO_AON_EI_EXT                      0x204174f0 /* [RW][32] GENERAL PURPOSE I/O EDGE INSENSITIVE FOR AON_SGPIO[5:0] */
#define BCHP_GIO_AON_MASK_EXT                    0x204174f4 /* [RW][32] GENERAL PURPOSE I/O INTERRUPT MASK FOR AON_SGPIO[5:0] */
#define BCHP_GIO_AON_LEVEL_EXT                   0x204174f8 /* [RW][32] GENERAL PURPOSE I/O INTERRUPT TYPE FOR AON_SGPIO[5:0] */
#define BCHP_GIO_AON_STAT_EXT                    0x204174fc /* [RW][32] GENERAL PURPOSE I/O INTERRUPT STATUS FOR AON_SGPIO[5:0] */

/***************************************************************************
 *ODEN_LO - GENERAL PURPOSE I/O OPEN DRAIN ENABLE FOR AON_GPIO[27:0]
 ***************************************************************************/
/* GIO_AON :: ODEN_LO :: reserved0 [31:28] */
#define BCHP_GIO_AON_ODEN_LO_reserved0_MASK                        0xf0000000
#define BCHP_GIO_AON_ODEN_LO_reserved0_SHIFT                       28

/* GIO_AON :: ODEN_LO :: oden [27:00] */
#define BCHP_GIO_AON_ODEN_LO_oden_MASK                             0x0fffffff
#define BCHP_GIO_AON_ODEN_LO_oden_SHIFT                            0
#define BCHP_GIO_AON_ODEN_LO_oden_DEFAULT                          0x00000000

/***************************************************************************
 *DATA_LO - GENERAL PURPOSE I/O DATA FOR AON_GPIO[27:0]
 ***************************************************************************/
/* GIO_AON :: DATA_LO :: reserved0 [31:28] */
#define BCHP_GIO_AON_DATA_LO_reserved0_MASK                        0xf0000000
#define BCHP_GIO_AON_DATA_LO_reserved0_SHIFT                       28

/* GIO_AON :: DATA_LO :: data [27:00] */
#define BCHP_GIO_AON_DATA_LO_data_MASK                             0x0fffffff
#define BCHP_GIO_AON_DATA_LO_data_SHIFT                            0
#define BCHP_GIO_AON_DATA_LO_data_DEFAULT                          0x00000000

/***************************************************************************
 *IODIR_LO - GENERAL PURPOSE I/O DIRECTION FOR AON_GPIO[27:0]
 ***************************************************************************/
/* GIO_AON :: IODIR_LO :: reserved0 [31:28] */
#define BCHP_GIO_AON_IODIR_LO_reserved0_MASK                       0xf0000000
#define BCHP_GIO_AON_IODIR_LO_reserved0_SHIFT                      28

/* GIO_AON :: IODIR_LO :: iodir [27:00] */
#define BCHP_GIO_AON_IODIR_LO_iodir_MASK                           0x0fffffff
#define BCHP_GIO_AON_IODIR_LO_iodir_SHIFT                          0
#define BCHP_GIO_AON_IODIR_LO_iodir_DEFAULT                        0x0fffffff

/***************************************************************************
 *EC_LO - GENERAL PURPOSE I/O EDGE CONFIGURATION FOR AON_GPIO[27:0]
 ***************************************************************************/
/* GIO_AON :: EC_LO :: reserved0 [31:28] */
#define BCHP_GIO_AON_EC_LO_reserved0_MASK                          0xf0000000
#define BCHP_GIO_AON_EC_LO_reserved0_SHIFT                         28

/* GIO_AON :: EC_LO :: edge_config [27:00] */
#define BCHP_GIO_AON_EC_LO_edge_config_MASK                        0x0fffffff
#define BCHP_GIO_AON_EC_LO_edge_config_SHIFT                       0
#define BCHP_GIO_AON_EC_LO_edge_config_DEFAULT                     0x00000000

/***************************************************************************
 *EI_LO - GENERAL PURPOSE I/O EDGE INSENSITIVE FOR AON_GPIO[27:0]
 ***************************************************************************/
/* GIO_AON :: EI_LO :: reserved0 [31:28] */
#define BCHP_GIO_AON_EI_LO_reserved0_MASK                          0xf0000000
#define BCHP_GIO_AON_EI_LO_reserved0_SHIFT                         28

/* GIO_AON :: EI_LO :: edge_insensitive [27:00] */
#define BCHP_GIO_AON_EI_LO_edge_insensitive_MASK                   0x0fffffff
#define BCHP_GIO_AON_EI_LO_edge_insensitive_SHIFT                  0
#define BCHP_GIO_AON_EI_LO_edge_insensitive_DEFAULT                0x00000000

/***************************************************************************
 *MASK_LO - GENERAL PURPOSE I/O INTERRUPT MASK FOR AON_GPIO[27:0]
 ***************************************************************************/
/* GIO_AON :: MASK_LO :: reserved0 [31:28] */
#define BCHP_GIO_AON_MASK_LO_reserved0_MASK                        0xf0000000
#define BCHP_GIO_AON_MASK_LO_reserved0_SHIFT                       28

/* GIO_AON :: MASK_LO :: irq_mask [27:00] */
#define BCHP_GIO_AON_MASK_LO_irq_mask_MASK                         0x0fffffff
#define BCHP_GIO_AON_MASK_LO_irq_mask_SHIFT                        0
#define BCHP_GIO_AON_MASK_LO_irq_mask_DEFAULT                      0x00000000

/***************************************************************************
 *LEVEL_LO - GENERAL PURPOSE I/O INTERRUPT TYPE FOR AON_GPIO[27:0]
 ***************************************************************************/
/* GIO_AON :: LEVEL_LO :: reserved0 [31:28] */
#define BCHP_GIO_AON_LEVEL_LO_reserved0_MASK                       0xf0000000
#define BCHP_GIO_AON_LEVEL_LO_reserved0_SHIFT                      28

/* GIO_AON :: LEVEL_LO :: level [27:00] */
#define BCHP_GIO_AON_LEVEL_LO_level_MASK                           0x0fffffff
#define BCHP_GIO_AON_LEVEL_LO_level_SHIFT                          0
#define BCHP_GIO_AON_LEVEL_LO_level_DEFAULT                        0x00000000

/***************************************************************************
 *STAT_LO - GENERAL PURPOSE I/O INTERRUPT STATUS FOR AON_GPIO[27:0]
 ***************************************************************************/
/* GIO_AON :: STAT_LO :: reserved0 [31:28] */
#define BCHP_GIO_AON_STAT_LO_reserved0_MASK                        0xf0000000
#define BCHP_GIO_AON_STAT_LO_reserved0_SHIFT                       28

/* GIO_AON :: STAT_LO :: irq_status [27:00] */
#define BCHP_GIO_AON_STAT_LO_irq_status_MASK                       0x0fffffff
#define BCHP_GIO_AON_STAT_LO_irq_status_SHIFT                      0
#define BCHP_GIO_AON_STAT_LO_irq_status_DEFAULT                    0x00000000

/***************************************************************************
 *ODEN_EXT - GENERAL PURPOSE I/O OPEN DRAIN ENABLE FOR AON_SGPIO[5:0]
 ***************************************************************************/
/* GIO_AON :: ODEN_EXT :: reserved0 [31:06] */
#define BCHP_GIO_AON_ODEN_EXT_reserved0_MASK                       0xffffffc0
#define BCHP_GIO_AON_ODEN_EXT_reserved0_SHIFT                      6

/* GIO_AON :: ODEN_EXT :: oden [05:00] */
#define BCHP_GIO_AON_ODEN_EXT_oden_MASK                            0x0000003f
#define BCHP_GIO_AON_ODEN_EXT_oden_SHIFT                           0
#define BCHP_GIO_AON_ODEN_EXT_oden_DEFAULT                         0x00000000

/***************************************************************************
 *DATA_EXT - GENERAL PURPOSE I/O DATA FOR AON_SGPIO[5:0]
 ***************************************************************************/
/* GIO_AON :: DATA_EXT :: reserved0 [31:06] */
#define BCHP_GIO_AON_DATA_EXT_reserved0_MASK                       0xffffffc0
#define BCHP_GIO_AON_DATA_EXT_reserved0_SHIFT                      6

/* GIO_AON :: DATA_EXT :: data [05:00] */
#define BCHP_GIO_AON_DATA_EXT_data_MASK                            0x0000003f
#define BCHP_GIO_AON_DATA_EXT_data_SHIFT                           0
#define BCHP_GIO_AON_DATA_EXT_data_DEFAULT                         0x00000000

/***************************************************************************
 *IODIR_EXT - GENERAL PURPOSE I/O DIRECTION FOR AON_SGPIO[5:0]
 ***************************************************************************/
/* GIO_AON :: IODIR_EXT :: reserved0 [31:06] */
#define BCHP_GIO_AON_IODIR_EXT_reserved0_MASK                      0xffffffc0
#define BCHP_GIO_AON_IODIR_EXT_reserved0_SHIFT                     6

/* GIO_AON :: IODIR_EXT :: iodir [05:00] */
#define BCHP_GIO_AON_IODIR_EXT_iodir_MASK                          0x0000003f
#define BCHP_GIO_AON_IODIR_EXT_iodir_SHIFT                         0
#define BCHP_GIO_AON_IODIR_EXT_iodir_DEFAULT                       0x0000003f

/***************************************************************************
 *EC_EXT - GENERAL PURPOSE I/O EDGE CONFIGURATION FOR AON_SGPIO[5:0]
 ***************************************************************************/
/* GIO_AON :: EC_EXT :: reserved0 [31:06] */
#define BCHP_GIO_AON_EC_EXT_reserved0_MASK                         0xffffffc0
#define BCHP_GIO_AON_EC_EXT_reserved0_SHIFT                        6

/* GIO_AON :: EC_EXT :: edge_config [05:00] */
#define BCHP_GIO_AON_EC_EXT_edge_config_MASK                       0x0000003f
#define BCHP_GIO_AON_EC_EXT_edge_config_SHIFT                      0
#define BCHP_GIO_AON_EC_EXT_edge_config_DEFAULT                    0x00000000

/***************************************************************************
 *EI_EXT - GENERAL PURPOSE I/O EDGE INSENSITIVE FOR AON_SGPIO[5:0]
 ***************************************************************************/
/* GIO_AON :: EI_EXT :: reserved0 [31:06] */
#define BCHP_GIO_AON_EI_EXT_reserved0_MASK                         0xffffffc0
#define BCHP_GIO_AON_EI_EXT_reserved0_SHIFT                        6

/* GIO_AON :: EI_EXT :: edge_insensitive [05:00] */
#define BCHP_GIO_AON_EI_EXT_edge_insensitive_MASK                  0x0000003f
#define BCHP_GIO_AON_EI_EXT_edge_insensitive_SHIFT                 0
#define BCHP_GIO_AON_EI_EXT_edge_insensitive_DEFAULT               0x00000000

/***************************************************************************
 *MASK_EXT - GENERAL PURPOSE I/O INTERRUPT MASK FOR AON_SGPIO[5:0]
 ***************************************************************************/
/* GIO_AON :: MASK_EXT :: reserved0 [31:06] */
#define BCHP_GIO_AON_MASK_EXT_reserved0_MASK                       0xffffffc0
#define BCHP_GIO_AON_MASK_EXT_reserved0_SHIFT                      6

/* GIO_AON :: MASK_EXT :: irq_mask [05:00] */
#define BCHP_GIO_AON_MASK_EXT_irq_mask_MASK                        0x0000003f
#define BCHP_GIO_AON_MASK_EXT_irq_mask_SHIFT                       0
#define BCHP_GIO_AON_MASK_EXT_irq_mask_DEFAULT                     0x00000000

/***************************************************************************
 *LEVEL_EXT - GENERAL PURPOSE I/O INTERRUPT TYPE FOR AON_SGPIO[5:0]
 ***************************************************************************/
/* GIO_AON :: LEVEL_EXT :: reserved0 [31:06] */
#define BCHP_GIO_AON_LEVEL_EXT_reserved0_MASK                      0xffffffc0
#define BCHP_GIO_AON_LEVEL_EXT_reserved0_SHIFT                     6

/* GIO_AON :: LEVEL_EXT :: level [05:00] */
#define BCHP_GIO_AON_LEVEL_EXT_level_MASK                          0x0000003f
#define BCHP_GIO_AON_LEVEL_EXT_level_SHIFT                         0
#define BCHP_GIO_AON_LEVEL_EXT_level_DEFAULT                       0x00000000

/***************************************************************************
 *STAT_EXT - GENERAL PURPOSE I/O INTERRUPT STATUS FOR AON_SGPIO[5:0]
 ***************************************************************************/
/* GIO_AON :: STAT_EXT :: reserved0 [31:06] */
#define BCHP_GIO_AON_STAT_EXT_reserved0_MASK                       0xffffffc0
#define BCHP_GIO_AON_STAT_EXT_reserved0_SHIFT                      6

/* GIO_AON :: STAT_EXT :: irq_status [05:00] */
#define BCHP_GIO_AON_STAT_EXT_irq_status_MASK                      0x0000003f
#define BCHP_GIO_AON_STAT_EXT_irq_status_SHIFT                     0
#define BCHP_GIO_AON_STAT_EXT_irq_status_DEFAULT                   0x00000000

#endif /* #ifndef BCHP_GIO_AON_H__ */

/* End of File */
