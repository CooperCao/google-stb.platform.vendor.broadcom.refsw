/***************************************************************************
*     (c)2008 Broadcom Corporation
*
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
*
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* Description: Sample header file for an App Lib
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#ifndef B_Dsmcc_LIB_H__
#define B_Dsmcc_LIB_H__

#ifdef __cplusplus
extern "C" {
#endif


/***************************************************************************
Summary:
Dsmcc App Lib settings structure
***************************************************************************/
#define B_DSMCC_INPUT_TYPE_INBAND 0
#define B_DSMCC_INPUT_TYPE_FILE 1
#define B_DSMCC_INPUT_TYPE_DSG 2

#ifndef HOST_ONLY
#include "nexus_base_types.h"
#endif
typedef struct B_Dsmcc_Settings
{
    int inputType;

    /* for inband */
    /* NEXUS_PidChannelHandle pidChannel; */
    void * pidChannel;

    /* for file */
    char inputFile[128];
    unsigned short pid;
    /* for dsg tunnel */
#ifdef HOST_ONLY
    void* tunnel;
#else
    NEXUS_Callback tunnel; /* tunnel callback for dsg carousel download */
#endif
} B_Dsmcc_Settings;

#define DEFAULT_TIMEOUT_MS 1200000 /* timeout to wait for DDB.
                                    * 20 minute seems to be reasonable 
                                    */
#define DEFAULT_TIMEOUT_DII_MS 600000 /* wait 10 minute for DII */
#define B_DSMCC_ERR_NO_ERROR 0
#define B_DSMCC_ERR_MODULE_NOT_FOUND 1
#define B_DSMCC_ERR_TIMEOUT 2

typedef struct B_Dsmcc_Status {
    unsigned int inbandCarouselTimeoutCount; /* These values increment each time an implementation
                                              * specific Inband Carousel timeout has occurred since the last
                                              * boot or reset.
                                              */
    unsigned int inbandCarouselTimeoutUs; /* download timeout period in microseconds */
    unsigned int error; /* indicates last error status */

    /* status of currently download module */
	int module_id;
	unsigned int offset;
	unsigned int module_size; 
	unsigned char module_info_length; /* 8 bits */
	unsigned char module_info_byte[256]; /* depends on module info length */
	unsigned short block_size; 

} B_Dsmcc_Status;

/***************************************************************************
Summary:
Public handle for Frontend App Lib
***************************************************************************/
typedef void * B_Dsmcc_Handle;

/***************************************************************************
Summary:
Returns the default and recommended values for the App Lib public settings.

Description:
This function returs the default values for Dsmcc settings. A pointer to a
valid B_Dsmcc_Settings structure must be provide or an error will be returned.
Input:
	pDsmccSettings - pointer to an existing settings structure

Returns:
	BERR_SUCCESS - If settings where able to be set
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.
***************************************************************************/
B_Error B_Dsmcc_GetDefaultSettings(B_Dsmcc_Settings *pDsmccSettings);


/***************************************************************************
Summary:
This function initializes the DSMCC channel

Description:
This function initializes the App Lib based on the settings selected. The
following actions are taken in this function, setup message filter, tune
to the specified pid channel, and parse the DSMCC DII message. Return DSMCC
handle if succeed, NULL if failed.

Input:
	pDsmccSettings - pointer to an existing settings structure

Returns:
	Opaque module handle used in subsequent API calls to this module
	NULL - If pDsmccSettings was NULL or module initalization could
        not finish
***************************************************************************/
B_Dsmcc_Handle * B_Dsmcc_Open(B_Dsmcc_Settings *pDsmccSettings);

/***************************************************************************
Summary:
This function de-initializes the DSMCC channel

Description:
This function de-initializes the DSMCC channel, and free up resources allocated
in open function.

Input:
	h - Handle to the open Sample App Lib

Returns:
	BERR_SUCCESS - If the module was able to de-initialize.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.
***************************************************************************/
B_Error B_Dsmcc_Close(B_Dsmcc_Handle h);

/***************************************************************************
Summary:
This function reads the data from dsmcc data carousel.

Description:
This function reads the data from dsmcc data carousel, with specified module_index.
The read is like reading from a file, with the offset increased (internally) after
each read.
This function waits for the next block number in data carousel before returning.

Input:
	h - Handle to the open Dsmcc App Lib
        module_index - Module index from DII message
        len - number of bytes to read
Returns:
        If succeed, return the number of bytes
        If end of module, return 0
        If error, return < 0.
***************************************************************************/
size_t B_Dsmcc_Read(B_Dsmcc_Handle h, int module_index, unsigned char * buf, size_t len);

/***************************************************************************
Summary:
This function reads the data from dsmcc data carousel.

Description:
This function reads the data from dsmcc data carousel, with specified module_index.
The read is like reading from a file, with the offset increased (internally) after
each read.

Unlike B_Dsmcc_Read, this function returns immediately when data is available, along 
with the block number. Returned data always started at the (block number * block size)
offset. Upper layer function should handle the offset.  

Input:
	h - Handle to the open Dsmcc App Lib
        module_index - Module index from DII message
        len - number of bytes to read
        block_nr - the returned data started at the begining of block_nr
Returns:
        If succeed, return the number of bytes
        If end of module, return 0
        If error, return < 0.
***************************************************************************/
size_t B_Dsmcc_Read_NoWait(B_Dsmcc_Handle h, int module_id, unsigned char * buf, size_t len, 
                           uint16_t * block_nr);


/***************************************************************************
Summary:
This function returns number of modules in DSMCC.

Description:
The function returns number of modules in DSMCC in DII message

Input:
	h - Handle to the open Dsmcc App Lib
Returns:
        If succeed, return number of modules in DSMCC
        If fail, return <0 .
***************************************************************************/
int B_Dsmcc_GetNumOfModules(B_Dsmcc_Handle h);


/***************************************************************************
Summary:
This function returns module id from module name. 

Description:
This function returns module id from module name. Note this applies to 
Common Download only, as in CDL the ModuleInfoBytes is the name of the 
mono image.

Input:
	h - Handle to the open Dsmcc App Lib
    name - name of the module
    len - length for the name
Returns:
        If succeed, return module id
        If fail, return <0 .
***************************************************************************/
int B_Dsmcc_GetModuleIdFromName(B_Dsmcc_Handle h, uint8_t * name, int len);

/***************************************************************************
Summary:
Get the DSMCC status.

Description:
This function returns DSMCC status
Input:
	B_Dsmcc_Status * pStatus
Returns:
	>= 0 - if succeed
    < 0 - if fail
***************************************************************************/
int B_Dsmcc_GetStatus(B_Dsmcc_Status * pStatus);

/***************************************************************************
Summary:
Initialize DSMCC.

Description:
This function initialize the CDL internal parameters.
Input:
	None
Returns:
        None
***************************************************************************/

void B_Dsmcc_Init(B_Dsmcc_Settings * params);

/***************************************************************************
Summary:
Uninitialize Dsmcc.

Description:
This function uninitialize the DSMCC.
Input:
	None
Returns:
        None
***************************************************************************/
void B_Dsmcc_Uninit(void);


#ifdef __cplusplus
}
#endif

#endif /* #ifndef B_Dsmcc_LIB_H__ */
