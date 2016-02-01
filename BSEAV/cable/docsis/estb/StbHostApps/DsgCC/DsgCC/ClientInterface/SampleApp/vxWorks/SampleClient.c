/***********************************************************************/
/*  Sample DSG client thread for single process vxWorks DSG model      */
/*                                                                     */
/***********************************************************************/
#include "vxsys_conf.h"
#include "vxboard.h"
#include <taskLib.h>
#include <stdio.h>
#include "dsgcc.h"
#include "dsgcc_clnt_if.h"
#include "SampleClient.h"

#define CLIENT_ID 3
#define CLIENT_ID_TYPE 3

/* Prototype the thead function */
void dsgThreadThreadEntryPt(void);
int ClientPacketHandler(char * pkt, int pktLen);

/* Called by root to start the DSG client */
void StartDsgTask(void)
{
	int rc;

	rc = taskSpawn(
		"tDSGClient", 6, 0,								/* This vxWorks task will be called tDSGClient */
		8 * 1024,													/* Will have an 8K stack */
		(FUNCPTR)dsgThreadThreadEntryPt,
		1,2,3,4,5,6,7,8,9,0);

	if (rc == ERROR) /* Spit out warning if something should fail */
		printf("Failed to spawn tStartup task..\n");

	return;
}

/***********************************************************************/
/*  startupThreadEntryPt() -                                           */
/*                                                                     */
/*  This function is the entry point into the Dsg Client thread.       */
/*                                                                     */
/***********************************************************************/
void dsgThreadThreadEntryPt(void)
{
	struct dsgClientRegInfo myInfo;
	int retVal, keepalives;

	printf("*** Sample Client started\n");
	printf("*** Sample Client sleeping for 20 seconds to allow Client Controller to initialize\n");

	taskDelay(20 *sysClkRateGet());

	printf("*** Sample Client registering myself with Client Controller\n");

	myInfo.clientId = CLIENT_ID;
	myInfo.idType = CLIENT_ID_TYPE;
	myInfo.clientName[0] = 0;
	myInfo.clientPort = (int)ClientPacketHandler;
	myInfo.clientProgNum = 0;
	retVal = registerdsgclient(&myInfo);
	printf("*** Sample Client registerdsgclient() returned %d\n", retVal);
	keepalives = 15;
	while (--keepalives)
	{
		taskDelay(2 *sysClkRateGet());
		keepalivedsgclient(&myInfo);
	}
	taskDelay(2 *sysClkRateGet());
	retVal = unregisterdsgclient(&myInfo);
	printf("*** Sample Client unregisterdsgclient() returned %d\n", retVal);
}

int ClientPacketHandler(char * pkt, int pktLen)
{
	printf(".");
	return 0;
}
