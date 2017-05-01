/******************************************************************************
 *  Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include "bstd.h"           /* standard types */
#include "berr.h"           /* error code */
#include "bkni.h"           /* kernel interface */
#include "bkni_multi.h"           /* kernel interface */
#include "brpc.h"
#include "brpc_3255.h"
#include "brpc_socket.h"

BDBG_MODULE(test_rpc);


#define sizeInLong(x)	(sizeof(x)/sizeof(uint32_t))
#define	CHK_RETCODE( rc, func )				\
do {										\
	if( (rc = BERR_TRACE(func)) != BERR_SUCCESS ) \
	{										\
		goto done;							\
	}										\
} while(0)


static BRPC_Handle rpc_handle;
static bool check_rpc_enabled = true;


static void check_rpc(BRPC_Handle *rpc_handle)
{

	while (check_rpc_enabled)
	{
		uint32_t device_id, event;

		BRPC_CheckNotification(*rpc_handle,  &device_id, &event, 0);
		if ((event>>16)) {
			printf("check_rpc(): notified by server (device_id = %08x) event is %x\n", device_id, event);
			switch (device_id) {
				case BRPC_DevId_3255:
				case BRPC_DevId_3255_US0:
					printf(" notification from 3255 OOB US\n");
					break;
				case BRPC_DevId_3255_OB0:
					printf(" notification from 3255 OOB DS\n");
					break;
				case BRPC_DevId_3255_DS0:
				case BRPC_DevId_3255_DS1:
				case BRPC_DevId_3255_DS2:
					printf(" notification from 3255 Inband DS%d\n", device_id-BRPC_DevId_3255_DS0);
					break;
				case BRPC_DevId_3255_TNR0:
				case BRPC_DevId_3255_TNR1:
				case BRPC_DevId_3255_TNR2:
					printf(" notification from 3255 Inband Tuner%d\n", device_id-BRPC_DevId_3255_TNR0);
					break;
				case BRPC_DevId_3255_TNR0_OOB:
					printf(" notification from 3255 OOB tuner\n");
					break;
				case BRPC_DevId_3255_POD:
					printf(" notification from POD\n");
					break;
				default:
					break;
			}
		} else {
			BKNI_Sleep(100); /* this is inefficient. use BRPC_GetSocketDescriptor to be efficient */
		}
	}
}
/* The entry point of the notification thread who listens to NOTIFICAION messages
 * from the server and sends NOTIFICAION_ACK message back to the server
 */
void * notification_thread(void *arg)
{
	check_rpc((BRPC_Handle *)arg);
	return NULL;
}


int main()
{
	pthread_t rpc_thread;
	BRPC_OpenSocketImplSettings socketSettings;
	BERR_Code retVal;
	BERR_Code retCode = BERR_SUCCESS;

    printf("Initializing Broadcom kernel interface ...\n");
    if (BKNI_Init() != BERR_SUCCESS) {
        return -1;
    }

    printf("Initializing Broadcom debug interface ...\n");
    if (BDBG_Init() != BERR_SUCCESS) {
        return -1;
    }


	BRPC_GetDefaultOpenSocketImplSettings(&socketSettings);

	BDBG_MSG(("rpc_main(): Got default settings: ipaddr = %x port_req = %04x port_ack = %04x\n", socketSettings.host,socketSettings.port_req,socketSettings.port_ack));

	BRPC_Open_SocketImpl(&rpc_handle, &socketSettings);

	if (rpc_handle == NULL) {
		BDBG_ERR((" RPC is NOT initialized between 3255 and 740x"));
		goto done;
	}
	/* start RPC ntotification thread*/
	pthread_create(&rpc_thread, NULL, (void *(*)(void *))(notification_thread) , (void*)&rpc_handle);


#if 0 /*example code to test 3255 DS1 tuning RPC*/
	{
		BRPC_Param_TNR_Open Param;

        Param.devId = BRPC_DevId_3255_TNR1;
        CHK_RETCODE( retCode, BRPC_CallProc(rpc_handle, BRPC_ProcId_TNR_Open, (const uint32_t *)&Param, sizeInLong(Param), NULL, 0, &retVal) );
        CHK_RETCODE( retCode, retVal );

	}
	{
		BRPC_Param_ADS_OpenChannel      Param;
        /* RPC open channel*/
        Param.devId = BRPC_DevId_3255_DS1 ;
        CHK_RETCODE( retCode, BRPC_CallProc(rpc_handle, BRPC_ProcId_ADS_OpenChannel, (const uint32_t *)&Param, sizeInLong(Param), NULL, 0, &retVal));
        CHK_RETCODE( retCode, retVal );
	}

	{
		BRPC_Param_TNR_SetRfFreq Param;

        Param.devId = BRPC_DevId_3255_TNR1;
        Param.rfFreq = 447000000;
        Param.tunerMode = 0;

        CHK_RETCODE( retCode, BRPC_CallProc(rpc_handle, BRPC_ProcId_TNR_SetRfFreq, (const uint32_t *)&Param, sizeInLong(Param), NULL, 0, &retVal) );
        CHK_RETCODE( retCode, retVal);
	}

	{
		BRPC_Param_ADS_Acquire Param;
        Param.devId = BRPC_DevId_3255_DS1;
        Param.modType = 2;
        Param.symbolRate = 5056900;
        CHK_RETCODE( retCode, BRPC_CallProc(rpc_handle, BRPC_ProcId_ADS_Acquire, (const uint32_t *)&Param, sizeInLong(Param), NULL, 0, &retVal));
        CHK_RETCODE( retCode, retVal );
	}
	printf(" wait for ADS locking event\n");
	getchar();
#else
	/* add your RPC test call codes here*/

	getchar();
#endif


done:
	if( retCode != BERR_SUCCESS )
	{
		printf(" error \n");
	}

	check_rpc_enabled= false;
	pthread_join(rpc_thread, NULL);

	BRPC_Close_SocketImpl(rpc_handle);
	return 0;
}
