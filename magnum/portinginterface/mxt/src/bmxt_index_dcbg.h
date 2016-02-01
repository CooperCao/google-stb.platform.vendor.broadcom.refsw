/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
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

#ifndef BMXT_INDEX_DCBG_H__
#define BMXT_INDEX_DCBG_H__

#define BMXT_RESOURCE_MINI_PID_PARSER0_ATS_SNAPSHOT           0
#define BMXT_RESOURCE_SLOT_MANAGEMENT_BAND0_SLOTS_ALLOCATED   1
#define BMXT_RESOURCE_SLOT_MANAGEMENT_MEM0_SLOTS_ALLOCATED    2
#define BMXT_RESOURCE_ISSY_CNTR0_CTRL                         3
#define BMXT_RESOURCE_ISSY_PACING0_CTRL                       4
#define BMXT_RESOURCE_ISSY_RATIO_CNTR0_CTRL                   5
#define BMXT_RESOURCE_DCBG0_CTRL                              6
#define BMXT_RESOURCE_ISSY_DISCONT_ERROR_INFO1_IBP0           7
#define BMXT_RESOURCE_ISSY_PACING0_ERROR_INFO1                8
#define BMXT_RESOURCE_DCBG0_ISSY_PID_MATCH_ERROR_INFO1        9
#define BMXT_RESOURCE_OP_PIPE_BAND0_ADD_ATS_CONSTANT_MOD300  10

#define BCHP_DEMOD_XPT_FE_DCB_DEBUG0                                                        0
#define BCHP_DEMOD_XPT_FE_MINI_PID_PARSER0_DVB_CTRL1                                        1
#define BCHP_DEMOD_XPT_FE_MINI_PID_PARSER0_PARSER31_ATS_ISSY_SNAPSHOT_CTRL1                 2
#define BCHP_DEMOD_XPT_FE_MINI_PID_PARSER0_PARSER31_ATS_ISSY_SNAPSHOT_VALID                 3
#define BCHP_DEMOD_XPT_FE_MINI_PID_PARSER0_ATS_SNAPSHOT                                     4
#define BCHP_DEMOD_XPT_FE_MINI_PID_PARSER0_SEQUENCER_ATS_SNAPSHOT                           5
#define BCHP_DEMOD_XPT_FE_MINI_PID_PARSER0_RECEIVED_ISSY_SNAPSHOT                           6
#define BCHP_DEMOD_XPT_FE_MINI_PID_PARSER0_PACING_ISSY_SNAPSHOT                             7
#define BCHP_DEMOD_XPT_FE_SLOT_MANAGEMENT_BAND0_SLOTS_ALLOCATED                             8
#define BCHP_DEMOD_XPT_FE_SLOT_MANAGEMENT_BAND0_FILLED_SLOTS                                9
#define BCHP_DEMOD_XPT_FE_SLOT_MANAGEMENT_BAND0_PREV_SLOT_ID                               10
#define BCHP_DEMOD_XPT_FE_SLOT_MANAGEMENT_BAND0_CURR_SLOT_ID                               11
#define BCHP_DEMOD_XPT_FE_SLOT_MANAGEMENT_BAND0_FIRST_SLOT_ID                              12
#define BCHP_DEMOD_XPT_FE_SLOT_MANAGEMENT_BAND0_SLOT_WATERMARK                             13
#define BCHP_DEMOD_XPT_FE_SLOT_MANAGEMENT_BAND0_SLOTS_REL_THRESHOLD                        14
#define BCHP_DEMOD_XPT_FE_SLOT_MANAGEMENT_BAND0_BAND31_DATA_RDY                            15
#define BCHP_DEMOD_XPT_FE_SLOT_MANAGEMENT_BAND0_BAND31_BUF_ALLOCATION_DIS                  16
#define BCHP_DEMOD_XPT_FE_SLOT_MANAGEMENT_BAND_MAX_POSSIBLE_SLOT_ALLOCATION                17
#define BCHP_DEMOD_XPT_FE_SLOT_MANAGEMENT_MEM0_SLOTS_ALLOCATED                             18
#define BCHP_DEMOD_XPT_FE_SLOT_MANAGEMENT_MEM0_MAX_NUM_SLOTS                               19
#define BCHP_DEMOD_XPT_FE_BAND_DROP_TILL_LAST_SET                                          20
#define BCHP_DEMOD_XPT_FE_BAND_DROP_TILL_LAST_STATUS                                       21
#define BCHP_DEMOD_XPT_FE_BAND_SLOT_STITCHING_EN                                           22
#define BCHP_DEMOD_XPT_FE_BAND_ISSY_DELTA_EN                                               23
#define BCHP_DEMOD_XPT_FE_BAND_ISSY_EXTRAPOLATE_EN                                         24
#define BCHP_DEMOD_XPT_FE_BAND_ATS_ADJUST_EN                                               25
#define BCHP_DEMOD_XPT_FE_BAND_ATS_ADJUST_MODE                                             26
#define BCHP_DEMOD_XPT_FE_BAND_BLOCKOUT_EN                                                 27
#define BCHP_DEMOD_XPT_FE_SLOT_RD_BAND_RD_IN_PROGRESS                                      28
#define BCHP_DEMOD_XPT_FE_PACING_BAND_RD_IN_PROGRESS                                       29
#define BCHP_DEMOD_XPT_FE_SLOT_RD_BAND_PAUSE_EN                                            30
#define BCHP_DEMOD_XPT_FE_PACING_BAND_PAUSE_EN                                             31
#define BCHP_DEMOD_XPT_FE_SLOT_RD_BAND_PAUSE_STATUS                                        32
#define BCHP_DEMOD_XPT_FE_PACING_BAND_PAUSE_STATUS                                         33
#define BCHP_DEMOD_XPT_FE_DCB_MISC_CFG                                                     34
#define BCHP_DEMOD_XPT_FE_BAND_ISSY_DISCONT_ERROR_DROP_EN                                  35
#define BCHP_DEMOD_XPT_FE_BO_IBP0                                                          36
#define BCHP_DEMOD_XPT_FE_ISSY_DELTA_IBP0                                                  37
#define BCHP_DEMOD_XPT_FE_ISSY_DELTA_ERR_THRESH_IBP0                                       38
#define BCHP_DEMOD_XPT_FE_ISSY_NUM_IBP0                                                    39
#define BCHP_DEMOD_XPT_FE_ISSY_PREV_IBP0                                                   40
#define BCHP_DEMOD_XPT_FE_SLOT_STATUS_IBP0                                                 41
#define BCHP_DEMOD_XPT_FE_ISSY_CNTR0_CTRL                                                  42
#define BCHP_DEMOD_XPT_FE_ISSY_CNTR0                                                       43
#define BCHP_DEMOD_XPT_FE_ISSY_PACING0_CTRL                                                44
#define BCHP_DEMOD_XPT_FE_ISSY_PACING0_ERR_THRESH_EARLY                                    45
#define BCHP_DEMOD_XPT_FE_ISSY_PACING0_ERR_THRESH_LATE                                     46
#define BCHP_DEMOD_XPT_FE_ISSY_RATIO_CNTR0_CTRL                                            47
#define BCHP_DEMOD_XPT_FE_ATS_RATIO_CNTR0_CTRL                                             48
#define BCHP_DEMOD_XPT_FE_ISSY_RATIO_CNTR0                                                 49
#define BCHP_DEMOD_XPT_FE_ATS_RATIO_CNTR0                                                  50
#define BCHP_DEMOD_XPT_FE_RATIO_SNAPSHOT0_CTRL                                             51
#define BCHP_DEMOD_XPT_FE_RATIO_SNAPSHOT0_STATUS                                           52
#define BCHP_DEMOD_XPT_FE_RATIO_SNAPSHOT0_CAPTURE_COUNT                                    53
#define BCHP_DEMOD_XPT_FE_RATIO_SNAPSHOT0_ATS_CNTR                                         54
#define BCHP_DEMOD_XPT_FE_RATIO_SNAPSHOT0_ISSY_CNTR                                        55
#define BCHP_DEMOD_XPT_FE_DCBG0_CTRL                                                       56
#define BCHP_DEMOD_XPT_FE_DCBG0_MAP_VECTOR                                                 57
#define BCHP_DEMOD_XPT_FE_DCBG0_BO                                                         58
#define BCHP_DEMOD_XPT_FE_DCBG0_STATUS                                                     59
#define BCHP_DEMOD_XPT_FE_DCBG0_NEXT_BAND_TO_RD                                            60
#define BCHP_DEMOD_XPT_FE_DCBG0_BAND_THRESHOLD                                             61
#define BCHP_DEMOD_XPT_FE_DCBG0_CURR_ISSY_STATUS                                           62
#define BCHP_DEMOD_XPT_FE_DCBG0_PREV_ISSY_STATUS                                           63
#define BCHP_DEMOD_XPT_FE_ISSY_DISCONT_ERROR_INFO1_IBP0                                    64
#define BCHP_DEMOD_XPT_FE_ISSY_DISCONT_ERROR_INFO2_IBP0                                    65
#define BCHP_DEMOD_XPT_FE_ISSY_PACING0_ERROR_INFO1                                         66
#define BCHP_DEMOD_XPT_FE_ISSY_PACING0_ERROR_INFO2                                         67
#define BCHP_DEMOD_XPT_FE_DCBG0_ISSY_PID_MATCH_ERROR_INFO1                                 68
#define BCHP_DEMOD_XPT_FE_DCBG0_ISSY_PID_MATCH_ERROR_INFO2                                 69
#define BCHP_DEMOD_XPT_FE_OP_PIPE_BAND0_ADD_ATS_CONSTANT_MOD300                            70
#define BCHP_DEMOD_XPT_FE_OP_PIPE_BAND0_ADD_ATS_CONSTANT_BINARY                            71

#endif
