/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/
/*
PCIE WINDOW
    Configure what memory can be accessed over PCIe.
    Some CaVendors may require to be able to set a maximum window size.
*/

#ifndef NEXUS_PCIE_WINDOW__H_
#define NEXUS_PCIE_WINDOW__H_

#include "nexus_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define NEXUS_PCIE_MAX_WINDOW_RAW_COMMAND_SIZE  (1220)



typedef struct NEXUS_SecurityPciEMaxWindowSizeSettings
{
    uint8_t  signedCommand[NEXUS_PCIE_MAX_WINDOW_RAW_COMMAND_SIZE];  /* The CA vendor supplied signedCommand. */
    unsigned signedCommandLength;                                    /* The size of the signedCommand. */
    NEXUS_SigningAuthority signingAuthority;                         /* Specify who is responsible for the RSA root
                                                                        key. Valid for Zeus5 only.*/
}NEXUS_SecurityPciEMaxWindowSizeSettings;


void NEXUS_Security_GetDefaultPciEMaxWindowSizeSettings(
    NEXUS_SecurityPciEMaxWindowSizeSettings *pConfig   /* [out] */
    );
/*
    Function to set the maximum possible size of memory that can be addressiable over PCIE. This function takes a
    signed command from the controlling CaVendor. The requirement for this function to be called before
    NEXUS_Security_SetPciERestrictedRange is determined by the NEXUS_OTPMSP_PCIE0_MWIN_SIZE_ENFORCE_ENABLE MSP OTP.
*/
NEXUS_Error NEXUS_Security_SetPciEMaxWindowSize (
    const NEXUS_SecurityPciEMaxWindowSizeSettings *pConfig
    );


/*
    Function to sets the location and size of memory that can be access from PCIE.
    See also function NEXUS_Security_SetPciEMaxWindowSize.
*/
NEXUS_Error NEXUS_Security_SetPciERestrictedRange (
    NEXUS_Addr baseOffset,      /* start of memory that can be access from PCIe */
    size_t size,                /* size of memory that can be accesed from PCIe*/
    unsigned index              /* identify particular PCIe interface */
    );


#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
