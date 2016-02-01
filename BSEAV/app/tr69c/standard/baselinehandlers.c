/***************************************************************************
*     (c)2004-2012 Broadcom Corporation
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
***************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../inc/appdefs.h"
#include "../main/types.h"

#include "baselineparams.h"

/*#define DEBUG*/

static time_t  startTime;
extern ACSState    acsState;

TRX_STATUS getManufacturerOUI(char **value)
{
	*value = strdup("123456789012345678901234");
	return TRX_OK;
}

TRX_STATUS getManufacturer(char **value)
{
	*value = strdup("Broadcom");
	return TRX_OK;
}

TRX_STATUS getModelName(char **value)
{
	*value = strdup("BCM97425");
	return TRX_OK;
}

TRX_STATUS getProductClass (char **value)
{
	*value = strdup("STB");
	return TRX_OK;
}

TRX_STATUS getSerialNumber(char **value)
{
	*value = strdup("1452002");
	return TRX_OK;
}

TRX_STATUS getSoftwareVersion(char **value)
{
	*value = strdup("1");
	return TRX_OK;
}

TRX_STATUS getHardwareVersion(char **value)
{
	*value = strdup("1");
	return TRX_OK;
}

TRX_STATUS getSpecVersion(char **value)
{
	*value = strdup("1");
	return TRX_OK;
}

TRX_STATUS getProvisioningCode(char **value)
{
	*value = acsState.provisioningCode?strdup(acsState.provisioningCode):strdup("");
	return TRX_OK;
}

TRX_STATUS setProvisioningCode(char *value)
{
	if (acsState.provisioningCode)
		free(acsState.provisioningCode);
	acsState.provisioningCode = strdup(value);

	return TRX_OK;
}

TRX_STATUS getUpTime(char **value)
{
	time_t  upt;
	char buf[100];

	upt = time(NULL) - startTime;
	sprintf(buf, "%lu", upt);
	*value = strdup(buf);

	return TRX_OK;
}

TRX_STATUS getDeviceLog(char **value)
{
	BSTD_UNUSED(value);
	return TRX_OK;
}

void initBaslineProfile(void) {
	/* init start time */
	startTime = time(NULL);
}
