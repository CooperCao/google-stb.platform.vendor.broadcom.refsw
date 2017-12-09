/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#ifndef BHSM_SECURITY__H_
#define BHSM_SECURITY__H_

#include "bchp.h"
#include "bint.h"
#include "bmma.h"
#include "bmem.h"
#include "berr_ids.h"
#include "breg_mem.h"
#include "bhsm_common.h"

#define BHSM_ZEUS_VERSION_CALC(major,minor) (((major)<<16)|(minor))

/* BHSM_ZEUS_VERSION resolves to the ZEUS version of the comiled for chip. */
#define BHSM_ZEUS_VERSION    BHSM_ZEUS_VERSION_CALC(BHSM_ZEUS_VER_MAJOR,BHSM_ZEUS_VER_MINOR)

/* requried by BSP headers. DO NOT USE IN CODE. */
#define ZEUS_VERSION   BHSM_ZEUS_VERSION                /* required by BSP headers, DO NOT USE IN CODE. */
#define ZEUS_4_2       BHSM_ZEUS_VERSION_CALC(4,2)      /* required by BSP headers, DO NOT USE IN CODE. */

/* HSM status/error codes. Range 0x0000 to 0xFFFF */
#define BHSM_STATUS_FAILED                           BERR_MAKE_CODE(BERR_ICM_ID, 1)  /* Return code for general failure. */
#define BHSM_STATUS_TIME_OUT                         BERR_MAKE_CODE(BERR_ICM_ID, 2)
#define BHSM_STATUS_PARM_LEN_ERR                     BERR_MAKE_CODE(BERR_ICM_ID, 3)
#define BHSM_STATUS_INPUT_PARM_ERR                   BERR_MAKE_CODE(BERR_ICM_ID, 4)
#define BHSM_STATUS_IRDY_ERR                         BERR_MAKE_CODE(BERR_ICM_ID, 5)
#define BHSM_STATUS_BSP_ERROR                        BERR_MAKE_CODE(BERR_ICM_ID, 6)
#define BHSM_STATUS_RESOURCE_ALLOCATION_ERROR        BERR_MAKE_CODE(BERR_ICM_ID, 7)
#define BHSM_STATUS_IN_PROGRESS                      BERR_MAKE_CODE(BERR_ICM_ID, 8)
#define BHSM_STATUS_STATE_ERROR                      BERR_MAKE_CODE(BERR_ICM_ID, 9)
#define BHSM_NOT_SUPPORTED_ERR                       BERR_MAKE_CODE(BERR_ICM_ID, 10)
#define BHSM_STATUS_REGION_VERIFICATION_NOT_ENABLED  BERR_MAKE_CODE(BERR_ICM_ID, 11)
#define BHSM_STATUS_REGION_ALREADY_CONFIGURED        BERR_MAKE_CODE(BERR_ICM_ID, 12)
#define BHSM_STATUS_OTP_PROGRAM_ERROR                BERR_MAKE_CODE(BERR_ICM_ID, 13)
#define BHSM_STATUS_REGION_VERIFICATION_FAILED       BERR_MAKE_CODE(BERR_ICM_ID, 14)



#define BHSM_ANY_ID (0xFEFEFEFE)

typedef struct BHSM_P_Handle *BHSM_Handle;

/*
    A callback function used by BHSM
*/
typedef void (*BHSM_Callback)(void *context, int param);

/*
    a structure to specify a callback function and associated paramters.
*/
typedef struct BHSM_CallbackDesc {
    BHSM_Callback callback;  /* Function pointer */
    void *context;           /* First parameter to callback function. */
    int param;               /* Second parameter to callback function. */
} BHSM_CallbackDesc;


/* paramters required to configure the HSM module on platform initialisation. */
typedef struct{
    BREG_Handle   hReg;
    BCHP_Handle   hChip;
    BINT_Handle   hInterrupt;
    BMMA_Heap_Handle mmaHeap;
    bool          sageEnabled;                                  /* only required on HOST side */
    unsigned      numKeySlotsForType[BHSM_KeyslotType_eMax];    /* only required on HOST side */

    BMEM_Heap_Handle  hHeap;    /* Enables HSM to allocate contigious memory for internal use. */

     struct{
        unsigned size;          /* Size. Needs to be >= BHSM_SECURE_MEMORY_SIZE */
        void    *p;             /* Pointer to memory */
     }secureMemory;             /* Secure memory. Only required for sage cleint. */

}BHSM_ModuleSettings;

/* Security module capabilities and configuration. */
typedef struct BHSM_ModuleCapabilities
{
   struct
   {
       struct
       {
            unsigned major;
            unsigned minor;
            unsigned subminor;
       }zeus,bfw;
   } version;                 /* version information */

   unsigned numKeyslotsForType[BHSM_KeyslotType_eMax];   /* Number of each type of keyslot. */

}BHSM_ModuleCapabilities;


/*
Description:
    Initialises HSM module and returns handle to it.
    To be called once during system initialisation.
*/
BHSM_Handle BHSM_Open( const BHSM_ModuleSettings *pSettings );

/*
Description:
   Releases HSM resources. To called once during system uninit.
*/
BERR_Code BHSM_Close( BHSM_Handle hHsm );

/*
Description:
   Returns capabilities and configuration of HSM.
*/
void BHSM_GetCapabilities( BHSM_Handle hHsm,  BHSM_ModuleCapabilities *pCaps );

#endif
