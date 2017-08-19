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

#ifndef _SECURITY_MAIN__
#define _SECURITY_MAIN__

typedef enum securityTestGroup
{
    securityTestGroup_keyslot,
    securityTestGroup_keyladder,
    securityTestGroup_otpMsp,
    securityTestGroup_otpDatasection,
    securityTestGroup_randomNumber,
    securityTestGroup_hash,
    securityTestGroup_hmac,
    securityTestGroup_rsa,
    securityTestGroup_regionVerification,

    securityTestGroup_max
}securityTestGroup;


typedef struct securityTestConfig
{
    bool enquire;
    securityTestGroup group;
    unsigned number;            /* the number of the test. Unique, min value is 1. */
    char name[64];              /* short name for test */
    char description[256];      /* description of test. */

    unsigned argsInts[10];      /* max 10 integer arguments */
    unsigned argsStrs[10][64];  /* max 10 strings arguments. */

}securityTestConfig;

typedef int (*securityTestFn)(struct securityTestConfig *pArg );

/* Register a Test Group */
void  securityFramework_registerGroup( securityTestGroup groupId, char *pGroupName );

/* Register an individual test */
void  securityFramework_registerTest( securityTestFn registeredTest );


#define MIN(a,b) (((a)<(b))?(a):(b))

#define SECURITY_FRAMEWORK_SET_NAME(a) do{  memset( pArgs->name, 0, sizeof(pArgs->name) ); \
                                            memcpy( pArgs->name, a, MIN(strlen(a), sizeof(pArgs->name)-1 )); }while(0);
#define SECURITY_FRAMEWORK_SET_DESCRIPTION(a) do{ memset( pArgs->description, 0, sizeof(pArgs->description) ); \
                                                  memcpy( pArgs->description, a, MIN(strlen(a), sizeof(pArgs->description)-1 ));  }while(0);
#define SECURITY_FRAMEWORK_SET_GROUP(a) do{ pArgs->group = a; }while(0);
#define SECURITY_FRAMEWORK_SET_NUMBER(a) do{ pArgs->number = a; }while(0);

/* Initilise Test Groups */
void initTestGroup_keyslot( void );
void initTestGroup_keyladder ( void );
void initTestGroup_randomNumber ( void );
void initTestGroup_hash ( void );
void initTestGroup_hmac ( void );
void initTestGroup_rsa ( void );
void initTestGroup_regionVerification ( void );

#endif /*_SECURITY_MAIN_ */
