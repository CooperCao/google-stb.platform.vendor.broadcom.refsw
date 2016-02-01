//****************************************************************************
//
// Copyright (c) 2008-2009 Broadcom Corporation
//
// This program is the proprietary software of Broadcom Corporation and/or
// its licensors, and may only be used, duplicated, modified or distributed
// pursuant to the terms and conditions of a separate, written license
// agreement executed between you and Broadcom (an "Authorized License").
// Except as set forth in an Authorized License, Broadcom grants no license
// (express or implied), right to use, or waiver of any kind with respect to
// the Software, and Broadcom expressly reserves all rights in and to the
// Software and all intellectual property rights therein.  IF YOU HAVE NO
// AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY,
// AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE
// SOFTWARE.  
//
// Except as expressly set forth in the Authorized License,
//
// 1.     This program, including its structure, sequence and organization,
// constitutes the valuable trade secrets of Broadcom, and you shall use all
// reasonable efforts to protect the confidentiality thereof, and to use this
// information only in connection with your use of Broadcom integrated circuit
// products.
//
// 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
// "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
// OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
// RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
// IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
// A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
// ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
// THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
//
// 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
// OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
// INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
// RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
// HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
// EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
// WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
// FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
//****************************************************************************
//
//  Filename:       rProgramStoreCustomerApi.c
//  Author:         Pinar Taskiran
//				    
//  Creation Date:  Feb 14, 2007
//
//*****************************************************************************
//  Description:
//      
// 
//*****************************************************************************
#include "rProgramStoreCustomerApi.h"

#include <stdio.h>
#include <stdlib.h>

static int (* cbEstbSwUpgrade_InProgressCallback) (TftpDownloadTrigger) = NULL; 
static int (* cbEstbSwUpgrade_IsUpgradePermittedNowCallback) (void ) = NULL;
static unsigned int (* cbEstbSwUpgrade_IsMonolithicImageHeaderValid) (void) = NULL;
static unsigned int (* cbEstbSwUpgrade_IsMonolithicImageValid) (void) = NULL;

/****************************************************
 *
 *	EstbSwUpgrade_InProgress
 *	
 ****************************************************/
int EstbSwUpgrade_InProgress(TftpDownloadTrigger trigger)
{
	// - TO BE IMPLEMENTED BY THE CUSTOMER - 
	printf("EstbSwUpgrade_InProgress - \n");
	if (cbEstbSwUpgrade_InProgressCallback) 
	{
		return (cbEstbSwUpgrade_InProgressCallback)(trigger);
	}

	printf("*** Vendor Extension *** - EstbSwUpgrade_InProgress - NOT IMPLEMENTED - Returning default value: true!\n");

	return 0;
}

void EstbSwUpgrade_InProgressSetCallback( int (*callback)(TftpDownloadTrigger)) {
	cbEstbSwUpgrade_InProgressCallback = callback;
}

/****************************************************
 *
 *	EstbSwUpgrade_IsUpgradePermittedNow
 *	
 ****************************************************/
unsigned int EstbSwUpgrade_IsUpgradePermittedNow(void)
{
	if (cbEstbSwUpgrade_IsUpgradePermittedNowCallback) 
	{
		//  - TO BE IMPLEMENTED BY THE CUSTOMER - 
		return (cbEstbSwUpgrade_IsUpgradePermittedNowCallback)();
	}

	printf("*** Vendor Extension *** - EstbSwUpgrade_IsUpgradePermittedNow - NOT IMPLEMENTED - Returning default value: true!\n");	
	return 1;
}

void EstbSwUpgrade_IsUpgradePermittedNowSetCallback( int (*callback)(void)) 
{
	cbEstbSwUpgrade_IsUpgradePermittedNowCallback = callback;
}


/****************************************************
 *
 *	EstbSwUpgrade_IsMonolithicImageHeaderValid
 *	
 ****************************************************/
unsigned int EstbSwUpgrade_IsMonolithicImageHeaderValid(void)
{
	if (cbEstbSwUpgrade_IsMonolithicImageHeaderValid) {
		return (cbEstbSwUpgrade_IsMonolithicImageHeaderValid)();
	}
	// - TO BE IMPLEMENTED  - 
	printf("*** Vendor Extension *** - EstbSwUpgrade_IsMonolithicImageHeaderValid - NOT IMPLEMENTED - Returning default value: true!\n");
	return 1;	
}
void EstbSwUpgrade_IsMonolithicImageHeaderValidSetCallback(unsigned int (*callback) (void ) )
{
	cbEstbSwUpgrade_IsMonolithicImageHeaderValid = callback;
}

/****************************************************
 *
 *	EstbSwUpgrade_IsMonolithicImageValid
 *	
 ****************************************************/
unsigned int EstbSwUpgrade_IsMonolithicImageValid(void)
{
	if (cbEstbSwUpgrade_IsMonolithicImageValid) {
		return (cbEstbSwUpgrade_IsMonolithicImageValid)();
	}
	// - TO BE IMPLEMENTED  - 
	printf("*** Vendor Extension *** - EstbSwUpgrade_IsMonolithicImageValid - NOT IMPLEMENTED - Returning default value: true!\n");
	return 1;
}

void EstbSwUpgrade_IsMonolithicImageValidSetCallback(unsigned int (*callback) (void) ) 
{
	cbEstbSwUpgrade_IsMonolithicImageValid = callback;
}

void EstbSwUpgrade_Notification(unsigned int eventType)
{
	// - TO BE IMPLEMENTED BY THE CUSTOMER - 
}

