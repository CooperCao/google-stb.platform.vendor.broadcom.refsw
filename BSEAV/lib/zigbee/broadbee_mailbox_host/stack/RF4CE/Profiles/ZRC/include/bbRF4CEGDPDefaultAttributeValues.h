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

/******************************************************************************
 *
 * DESCRIPTION:
 *      This is the header file for the RF4CE GDP profile attributes default values setup.
 *
*******************************************************************************/

#ifndef BBRF4CEGDPDEFAULTATTRIBUTEVALUES_H
#define BBRF4CEGDPDEFAULTATTRIBUTEVALUES_H

#define RF4CE_GDP2_POLL_CONSTRAINTS \
{ \
    { \
        .pollingMethod = RF4CE_GDP_POLLING_DISABLED, \
        .pollingTriggerCapabilities = RF4CE_GDP_POLLING_TRIGGER_TIME_BASED, \
        .minPollingKeyPressCounter = 0, \
        .maxPollingKeyPressCounter = 0, \
        .minPollingInterval = 0, \
        .maxPollingInterval = 0 \
    } \
}

#define RF4CE_GDP2_APPLICATION_SPECIFIC_BO_USER_STRING \
    { \
        'G', 'D', 'P', '2', '0', 'C', 'T', 'L', /* Application specific user string. See GDP 2.0 Spec Figure 34. */ \
    }

#ifdef RF4CE_TARGET
#define RF4CE_GDP2_APPLICATION_SPECIFIC_BR_USER_STRING \
    { \
        'G', 'D', 'P', '2', '0', 'T', 'G', 'T', /* Application specific user string. See GDP 2.0 Spec Figure 34. */ \
    }
#endif /* RF4CE_TARGET */

#endif // BBRF4CEGDPDEFAULTATTRIBUTEVALUES_H

/* eof bbRF4CEGDPDefaultAttributeValues.h */