/******************************************************************************
* Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
*
* This program is the proprietary software of Broadcom and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant
* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied), right
* to use, or waiver of any kind with respect to the Software, and Broadcom
* expressly reserves all rights in and to the Software and all intellectual
* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. , WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/
#ifndef NEXUS_SECURITY_PRIV_H__
#define NEXUS_SECURITY_PRIV_H__

/*=***********************************
*************************************/

#include "bhsm.h"
#include "nexus_security_datatypes.h"


#ifdef __cplusplus
extern "C"
{
#endif

/**
If you get the HSM handle, you must first lock the security module, get the handle, 
use the handle, then unlock the security module. After unlocking the security module
you may not use the hsm handle.

This is allowed:
    NEXUS_Module_Lock(securityModule);
    {
        BHSM_Handle hsm;
        NEXUS_Security_GetHsm_priv(&hsm);
        BHSM_Foo(hsm);
    }
    NEXUS_Module_Unlock(securityModule);

This is not allowed:
    BHSM_Handle hsm;
    NEXUS_Module_Lock(securityModule);
    NEXUS_Security_GetHsm_priv(&hsm);
    NEXUS_Module_Unlock(securityModule);
    BHSM_Foo(hsm); <-- this is a bug
    
To ensure you are following these rules, we recommend that you do not cache or otherwise store
the HSM handle. Only use a local variable that goes out of scope before unlocking the module (as above).
**/
void NEXUS_Security_GetHsm_priv(
    BHSM_Handle *pHsm /* [out] */
    );

NEXUS_OBJECT_CLASS_DECLARE(NEXUS_KeySlot);

void NEXUS_SecurityModule_Sweep_priv(void);


/*
Summary:
    Dumps to the console details on MEMC ARCH violations that have occured since the function was
    last called.
*/
void NEXUS_Security_PrintArchViolation_priv( void );


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
