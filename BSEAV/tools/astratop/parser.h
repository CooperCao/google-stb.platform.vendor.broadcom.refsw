/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *****************************************************************************/

/***************************************************************************
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * file : Parser.h
 *
 ***************************************************************************/

#ifndef _PARSER_H_
#define _PARSER_H_

#define TASK_INFO_32b_TASK_PRIORITY_SHIFT 0
#define TASK_INFO_32b_TASK_PRIORITY_MASK 0x000003ff

#define TASK_INFO_32b_TASK_ID_SHIFT 10
#define TASK_INFO_32b_TASK_ID_MASK 0x003FFC00

#define TASK_INFO_32b_CPU_ID_SHIFT 20
#define TASK_INFO_32b_CPU_ID_MASK 0x00300000

#define TASK_INFO_32b_MODULE_ID_SHIFT 24
#define TASK_INFO_32b_MODULE_ID_MASK 0x0F000000

#define TASK_INFO_32b_SEQUENCE_SHIFT 28
#define TASK_INFO_32b_SEQUENCE_MASK 0x10000000

#define TASK_INFO_32b_TASK_STATUS_SHIFT 0
#define TASK_INFO_32b_TASK_STATUS_MASK 0x00000001

#define TASK_INFO_32b_CPU_PERCENTAGE_FRACTION_SHIFT 1
#define TASK_INFO_32b_CPU_PERCENTAGE_FRACTION_MASK 0x0000001E

#define TASK_INFO_32b_CPU_PERCENTAGE_INTEGRAL_SHIFT 5
#define TASK_INFO_32b_CPU_PERCENTAGE_INTEGRAL_MASK 0x00000FE0

#define TASK_INFO_32b_TASK_LOAD_SHIFT 12
#define TASK_INFO_32b_TASK_LOAD_MASK 0x003FF000

#define EXTRACT_BITS(WORD, MASK, SHIFT) ((WORD & MASK) >> SHIFT)

#define SCHEDULER_MODULE_ID 1

struct taskInformation {
    unsigned int moduleID            :4;
    unsigned int CPUID               :2;
    unsigned int taskID              :10;
    union{
        /*Valid for CFS Tasks*/
        unsigned int priority            :10;
        /*Valid for EDF Tasks*/
        unsigned int taskLoad            :10;
    };
    unsigned int CPUperIntegral      :7;
    unsigned int CPUperFraction      :4;
    unsigned int taskStatus          :1;
};
#endif /* _PARSER_H_ */
