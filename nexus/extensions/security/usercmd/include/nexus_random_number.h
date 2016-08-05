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
#ifndef NEXUS_RANDOM_NUMBER_H__
#define NEXUS_RANDOM_NUMBER_H__

#include "nexus_security_datatypes.h"
#include "nexus_security.h"


#ifdef __cplusplus
extern "C" {
#endif


#define NEXUS_MAX_RANDOM_NUMBER_LENGTH           352


/**
Summary:
This enum defines the supported random number types.
**/
typedef enum NEXUS_RandomNumberType
{
    NEXUS_RandomNumberType_eRngRaw,
    NEXUS_RandomNumberType_eFips186X,
    NEXUS_RandomNumberType_eFips186K,
    NEXUS_RandomNumberType_eRngSha1,

    /* Add new random number type definition before this line */
    NEXUS_RandomNumberType_eMax
}   NEXUS_RandomNumberType;

/**
Summary:
**/
typedef enum NEXUS_RandomNumberDestination
{
    NEXUS_RandomNumberDestination_eHost            = 0,
    NEXUS_RandomNumberDestination_eFirstRNRam1024  = 20,
    NEXUS_RandomNumberDestination_eSecondRNRam1024 = 21,

    /* Add new random number type definition before this line */
    NEXUS_RandomNumberDestination_eMax
}   NEXUS_RandomNumberDestination;


/**************************************************************************************************
Summary:
Settings used in NEXUS_RandomNumber_Generate
**************************************************************************************************/
typedef struct NEXUS_RandomNumberGenerateSettings
{
    NEXUS_RandomNumberType type;

    /* Random number destination setting:
        Host                    : up to 360 bytes long or as specified
        FirstRNRam1024     : up to 256 bytes (2048 bits) long or as specified
        SecondRNRam1024 : up to 256 bytes (2048 bits) long or as specified
    */
    NEXUS_RandomNumberDestination destination;
    unsigned int randomNumberSize;  /* size in bytes of the requested random number */

} NEXUS_RandomNumberGenerateSettings;


/**************************************************************************************************
Summary:
Output from NEXUS_RandomNumber_Generate
**************************************************************************************************/
typedef struct NEXUS_RandomNumberOutput
{
    unsigned char buffer[NEXUS_MAX_RANDOM_NUMBER_LENGTH];      /* Buffer to hold the generated random number          */
    unsigned int  size;                            /* Actual size of the returned random number in bytes */

} NEXUS_RandomNumberOutput;

/*****************************************************************************
Summary:
Get default settings
*****************************************************************************/
void NEXUS_RandomNumber_GetDefaultGenerateSettings(
    NEXUS_RandomNumberGenerateSettings *pSettings /* [out] */
    );

/*****************************************************************************
Summary:
Generate a random number
*****************************************************************************/
NEXUS_Error NEXUS_RandomNumber_Generate(
    const NEXUS_RandomNumberGenerateSettings *pSettings,      /* structure holding input parameters */
    NEXUS_RandomNumberOutput                 *pOutput         /* [out] structure holding random number buffer and size */
    );


#ifdef __cplusplus
}
#endif

#endif
