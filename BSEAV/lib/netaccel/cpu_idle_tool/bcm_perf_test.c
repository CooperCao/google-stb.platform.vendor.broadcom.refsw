#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
char bcmPerfDriverCmd[512] = "insmod bcm_perf_drv.ko";

#define MAX_ETH_INTERFACE	4
//char arg1[30]=" g_eth_if_name0=eth";
char arg2[30]=" g_cpu_idle_stats=";
char arg3[30]=" g_log_type=2";
char bcmProcBuff[2024];
char procStatBuffer[2048];
char proc_interrupt_data[512];
int firstExecution = 1;
int currCumulativeIntrptCnt = 0;
int prevCumulativeIntrptCnt = 0;
int currCumulativeEthIntrptCnt[MAX_ETH_INTERFACE] = {0,0,0,0};
int prevCumulativeEthIntrptCnt[MAX_ETH_INTERFACE] = {0,0,0,0};


char Usage[] = "\
Usage: \n\
	bcm_perf_test -e [-options] -- 0 eth0, 1-> eth1 .Ex: -e0,1 means collect stats for eth0 and eth1 \n\
	bcm_perf_test -t [-options] -- 1 capture log every 1 second, 2 capture log every 2 second \n\
	bcm_perf_test -n [-options] -- number of times read the info \n\
";

typedef int bool;
#define true 1
#define false 0

typedef struct bperf_eth_info
{
	bool	enable;
	int ethIntrfcNo;
	int interruptNo;
}bperf_eth_info;

bperf_eth_info ethInfo[MAX_ETH_INTERFACE];


void strip_newline( char *str, int size )
{
    int i;

    /* remove the null terminator */
    for (  i = 0; i < size; ++i )
    {
        if ( str[i] == '\n' )
        {
            str[i] = '\0';
            /* we're done, so just exit the function by returning */
            return;   
        }
    }
}


int main(int argc ,char *argv[])
{
	int c = 0, n = 1,i,j,k, time;
	int error = 0;
	char temp_val[10]="0";
	FILE    *proc_bcm_drv_file;
	FILE    *proc_stat_file;
	FILE    *proc_intrpt_file;
	int prcStaterror=0;
	int tempEthNo =0;
	
	char *ret=NULL;
	char *next_token = NULL;
	//int totalInterrupts = 0;
	
	char temp_buf[100];
	/** following each is to carry different eth interface name **/
	char eth_buf[30];


	/** Initialize eth interface data ***/
	for(i=0; i < MAX_ETH_INTERFACE; i++)
	{
		ethInfo[i].enable = false;
		ethInfo[i].interruptNo = -1;/** assume that eth interrupt number can't be 0xFFFFFFFF **/
	}
	
	if(argc < 1) goto usage;

	while((c = getopt(argc, argv,"he:t:n:")) != -1)
	{
		switch(c)
		{
		case 'e':
			sprintf(temp_val, optarg, sizeof(optarg));//ethIntrfNum = optarg;

			ret = strtok_r(temp_val,",",&next_token);
			while(ret != NULL)
			{
				tempEthNo = atoi(ret);

				ethInfo[tempEthNo].enable = true;
				ethInfo[tempEthNo].ethIntrfcNo = tempEthNo;
				ret = strtok_r(NULL,",",&next_token);
			}
			
			//strcat(arg1,temp_val);
			//ethNo = atoi(optarg);
			/*	printf("\n %s value of arg1 \n",arg1 );	*/
		break;

		case 't':
			sprintf(temp_val, optarg, sizeof(optarg));
			strcat(arg2,temp_val);
			time = atoi(optarg);

			/*	printf("\n %s value of arg2 \n",arg2 );	*/
		break;
		case 'n':
			n =  atoi(optarg);
			printf("\n Value of n is ==========%d\n", n);
		break;
		case 'h':
		goto usage;
		break;
        default:
           fprintf(stderr,Usage);
           break;
        }
	}


	for(i=0; i < MAX_ETH_INTERFACE; i++)
	{
		if(ethInfo[i].enable == true)
		{
			
			sprintf(eth_buf," g_eth_if_name%d=eth%d",ethInfo[i].ethIntrfcNo,ethInfo[i].ethIntrfcNo);
			strcat(bcmPerfDriverCmd,eth_buf);

		}
	}
	

	strcat(bcmPerfDriverCmd,arg2);
	strcat(bcmPerfDriverCmd,arg3);
	printf("\n %s value of bcmPerfDriverCmd \n",bcmPerfDriverCmd );


	error=system(bcmPerfDriverCmd);

	if(error == -1 )
	{
		printf("\n Can't insert the bcm_perf_driver \n");
		return (error);
	}

	proc_bcm_drv_file = fopen("/proc/bcm_perf_drv/info", "r");
	proc_stat_file = fopen("/proc/stat", "rt");
	
	if(proc_stat_file == NULL)
	{
		printf("\n Can't open /proc/stat \n");
		prcStaterror = 1;
	}

	proc_intrpt_file =fopen("/proc/interrupts", "rt"); 


	/** the following if case is used to find out the interrupt number for all eth interfaces , 
		which will be used as an offset to find eth intrpt from /proc/stat	**/
	if(proc_intrpt_file)
	{


		while (fgets(proc_interrupt_data, sizeof(proc_interrupt_data), proc_intrpt_file)) {
			char *res=NULL;

			for(i = 0; i < MAX_ETH_INTERFACE; i++)
			{
				if((ethInfo[i].enable== true) && (ethInfo[i].interruptNo == -1))
				{
					sprintf(eth_buf,"eth%d",ethInfo[i].ethIntrfcNo);
					res = strstr(proc_interrupt_data , eth_buf);
					if(res != NULL)
 			        {
 						ret = strtok_r(proc_interrupt_data," ",&next_token);
 					    ethInfo[i].interruptNo = atoi(ret);
 			        }					

				}

			}		    
	    }
	    
	}/**** Done: found eth interface intrpt numbers ***/
	else
	{
		printf("\n Can't open /proc/interrupts \n");
	}
	

	sleep(2);

	if (proc_bcm_drv_file) {
		//while (fscanf(proc_bcm_drv_file, "%s", bcmProcBuff)!=EOF)
		for(i=0; i < n ; i++)
		{
			if(!prcStaterror)
			{			
				fread(procStatBuffer, sizeof(char), sizeof(procStatBuffer), proc_stat_file);
				ret = strstr(procStatBuffer,"intr");

				ret = strtok_r(ret, " ",&next_token);
				ret = strtok_r(NULL, " ",&next_token);
				currCumulativeIntrptCnt = atoi(ret);

				/** find total eth interrupt for eth_intrpt_no which could be for eth0 or eth1 or eth2.. ***/
				//totalethIntrptCnt = 
				j=0; /**we do this to get to the eth interrupt count value in the buffer **/
				for(k=0; k < MAX_ETH_INTERFACE; k++)
				{
					if(ethInfo[k].enable == true)
					{
						do{
							ret = strtok_r(NULL, " ",&next_token);
							j++;

							if(j==ethInfo[k].interruptNo)
							{
								currCumulativeEthIntrptCnt[k] = atoi(ret);
								break;
							}

						}while(ret != NULL);

					}
				}
								

				/******* Done finding total currCumulativeEthIntrptCnt *****************/ 
				
				
				//ret++;
				//printf("Here is procStatBuffer substring after intr--------------> %s\n",ret);
				
#if	BCM_PERF_DEBUG
				printf("currCumulativeEthIntrptCnt ====================== %d\n", currCumulativeEthIntrptCnt[0]);
				printf("currCumulativeIntrptCnt--------------> %d\n",currCumulativeIntrptCnt);
#endif				
				if(firstExecution)
				{
					prevCumulativeIntrptCnt = currCumulativeIntrptCnt;
					prevCumulativeEthIntrptCnt[0] = currCumulativeEthIntrptCnt[0];
					prevCumulativeEthIntrptCnt[1] = currCumulativeEthIntrptCnt[1];
					prevCumulativeEthIntrptCnt[2] = currCumulativeEthIntrptCnt[2];
					prevCumulativeEthIntrptCnt[3] = currCumulativeEthIntrptCnt[3];
					firstExecution = 0;
				}

#if	BCM_PERF_DEBUG
				printf("EthIntrptCnt ====================== %d\n", (currCumulativeEthIntrptCnt[0]-prevCumulativeEthIntrptCnt[0]));
				printf("TotalIntrptCnt--------------> %d\n",(currCumulativeIntrptCnt-prevCumulativeIntrptCnt));
#endif				
			
				//printf("Here is procStatBuffer --------------> %s\n",procStatBuffer);				

			}


			fgets( bcmProcBuff, sizeof(bcmProcBuff),proc_bcm_drv_file);

			j = strlen(bcmProcBuff);/* this will remove the new line char so that we can concat next string in same line **/
			if(j> 0 && bcmProcBuff[j-1] == '\n')
			{
				bcmProcBuff[j-1] = 0;
			}

					
			/**following section prints Total and  the eth interrupt  ***/
			sprintf(temp_buf, "INTRPT [TOTAL:%5d", (currCumulativeIntrptCnt-prevCumulativeIntrptCnt));
			strcat(bcmProcBuff,temp_buf);

			for(k=0; k < MAX_ETH_INTERFACE; k++)
			{
				if(ethInfo[k].enable == true)
				{
					sprintf(temp_buf, " Eth%d:%5d", ethInfo[k].ethIntrfcNo,(currCumulativeEthIntrptCnt[k]-prevCumulativeEthIntrptCnt[k]));
					strcat(bcmProcBuff,temp_buf);
				}
			}
			sprintf(temp_buf,"]\n");
			strcat(bcmProcBuff,temp_buf);

			printf("%s",bcmProcBuff);
			
#if	0
			printf("Total Intrpt %d , eth%d intrpt %d\n",(currCumulativeIntrptCnt-prevCumulativeIntrptCnt),ethNo,(currCumulativeEthIntrptCnt-prevCumulativeEthIntrptCnt));
#endif			
			if(!prcStaterror)
			{
				prevCumulativeIntrptCnt = currCumulativeIntrptCnt;
				prevCumulativeEthIntrptCnt[0] = currCumulativeEthIntrptCnt[0];
				prevCumulativeEthIntrptCnt[1] = currCumulativeEthIntrptCnt[1];
				prevCumulativeEthIntrptCnt[2] = currCumulativeEthIntrptCnt[2];
				prevCumulativeEthIntrptCnt[3] = currCumulativeEthIntrptCnt[3];
				rewind(proc_stat_file);
			}
			
			sleep(1);
			rewind(proc_bcm_drv_file);
			
		}
		fclose(proc_bcm_drv_file);
	}
	else
	{
		printf("\n Not able to open the proc file \n");
	}

	printf("\n we are done -------------------------\n");


	exit(0);
usage:
    fprintf(stderr,Usage);
    exit(1);
}

