/***************************************************************************
*  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
***************************************************************************/

#ifndef NEXUS_AVS_H__
#define NEXUS_AVS_H__

#include "nexus_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
Summary:
Settings used to pass external parameters to the Adaptive Voltage Scaling (AVS) mechanism.
**/
typedef struct NEXUS_AvsSettings
{
    /* The following supported only for 65nm parts that support AVS hardware. */
    unsigned hardStopOffset;         /* Range 0-15. This parameter should be programmed as per the
                                        board voltage drop (through PCB and filter components) between 
                                        the output of the regulator [VREG_VFB_1P2 net] and the 
                                        supply of the chip [D1.2V_BCM75XX]. Each unit is ~12.5mV.   
                                        If the board design  has 20mV of drop then hardStopOffset=2 
                                        should be programmed */
    unsigned maxVoltageStopOffset;   /* Percentage reduction 0-15 from maximum +16% Vreg setting (at regulator).
                                        Set this parameter to 4 if ambient temperature is expected
                                        to rise above +70C for extended period. */                             
} NEXUS_AvsSettings;

/*
Summary:
Get AVS settings.
*/
void NEXUS_GetAvsSettings(
    NEXUS_AvsSettings *pSettings /* [out] the current settings */
    );

/*
Summary:
Set AVS settings. 
*/
NEXUS_Error NEXUS_SetAvsSettings(
    const NEXUS_AvsSettings *pSettings /* the new settings */
    );

/**
Summary:
Get AVS specific data and status from the Adaptive Voltage Scaling (AVS) mechanism.
**/
typedef struct NEXUS_AvsStatus
{
    /* The following are for 40nm parts that support AVS hardware. */
    unsigned voltage;     /* last measured voltage (in millivolts).  To convert use: float V = voltage/1000.; */
    signed temperature;   /* last measured temperature in thousands.  To convert use: float celcius = temperature/1000.; */
    bool enabled;         /* AVS is enabled in the bootloader/CFE */
    bool tracking;        /* AVS-tracking is enabled in Nexus system software */
    unsigned heartbeat;   /* AVS background counter, increments once a second, if alive */
} NEXUS_AvsStatus;

/*
Summary:
Get AVS domain-specific status.
*/
typedef enum NEXUS_AvsDomain
{
    NEXUS_AvsDomain_eMain, /* used on single or dual domain systems */
    NEXUS_AvsDomain_eCpu,  /* only used on dual domain systems */
    NEXUS_AvsDomain_eMax
} NEXUS_AvsDomain;

NEXUS_Error NEXUS_GetAvsDomainStatus(
    NEXUS_AvsDomain domain,  /* [in] index of domain to fetch status */
    NEXUS_AvsStatus *pStatus /* [out] the current domain-specific status */
    );

#define NEXUS_GetAvsStatus(PSTATUS) NEXUS_GetAvsDomainStatus(NEXUS_AvsDomain_eMain, PSTATUS)

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_AVS_H__ */
