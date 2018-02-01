/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

 ******************************************************************************/

/*
 There are about seven OTP (One Time Programmable) data sections on a chip. Each section will have a pupose particular to the deployment.
 This API enables read/programm access to these OTP data sections.
 Note:
    - Not all OTP data sections can be read, in which case a read attempt will only return a SHA1 of the data.
    - Not all OTP data sections can be programmed.
*/

#include "bhsm.h"

#ifndef BHSM_MSP_DATASECTION__H_
#define BHSM_MSP_DATASECTION__H_

#ifdef __cplusplus
extern "C"
{
#endif

#define BHSM_DATA_SECTION_LENGTH           (32)     /* in bytes*/

typedef struct
{
    /* input */
    unsigned index;                            /* The OTP datasection to be read */

    /* output */
    bool accessible;                           /* if true, OTP data is readable and returned. If false, SHA1 of data section is retured instead */
    uint8_t data[BHSM_DATA_SECTION_LENGTH];
    unsigned dataLength;                       /* length of data to be read. Max is BHSM_DATA_SECTION_LENGTH */

}BHSM_DataSectionRead;


typedef struct
{
    /* input */
    unsigned index;                           /* The OTP datasection to be written to. */

    /* output */
    uint8_t  data[BHSM_DATA_SECTION_LENGTH];
    unsigned dataLength;                      /* length of data to be written. Max is BHSM_DATA_SECTION_LENGTH */

}BHSM_DataSectionWrite;

/*
    Read an OTP data section
*/
BERR_Code BHSM_OtpDataSection_Read( BHSM_Handle hHsm,
                                    BHSM_DataSectionRead *pParam
                                  );

/*
    Program an OTP data section.
*/
BERR_Code BHSM_OtpDataSection_Write( BHSM_Handle hHsm,
                                     const BHSM_DataSectionWrite *pParam
                                   );

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
