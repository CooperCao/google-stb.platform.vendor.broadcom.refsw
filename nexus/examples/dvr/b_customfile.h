/******************************************************************************
 *    (c)2011 Broadcom Corporation
 * 
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 *****************************************************************************/
#ifndef B_CUSTOMFILE_H__
#define B_CUSTOMFILE_H__

#include "nexus_types.h"
#include "nexus_file.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
Nexus' file architecture is extensible through wrappers.
The file module must exist in the same execution space as the application (for example,
even in a kernel mode configuration, the file module runs in the user space proxy)
therefore a non-thunked wrapper is acceptable.

This is an example of a customer wrapper which applies a beginning and ending offset to both stream and index data.
By providing an example, we enable each customer to customize and extend the design.

Underlying the NEXUS_FilePlayHandle is the BSEAV/lib/bfile architecture.
It uses an abstraction scheme to provide inheritance.
**/

struct b_customfile_bounds {
    off_t bof; /* sets new "beginning of file", in units of bytes */
    off_t eof; /* sets new "end of file", in units of bytes. if 0, then the file is unbounded. */
};

    /*
Summary:
settings for b_customfile_open
*/
typedef struct b_customfile_settings {
    struct b_customfile_bounds data;  /* bounds for data file */
    struct b_customfile_bounds index; /* bounds for index file */
} b_customfile_settings;

/*
Summary:
get default settings
*/
void b_customfile_get_default_settings(
    b_customfile_settings *pSettings /* [out] */
    );

/*
Summary:
This function is used to wrap NEXUS_FilePlay object to apply an offset for index and data
*/
NEXUS_FilePlayHandle b_customfile_open(
    NEXUS_FilePlayHandle file, /* file object to be wrapped */
    const b_customfile_settings *pSettings
    );

#ifdef __cplusplus
}
#endif

#endif
