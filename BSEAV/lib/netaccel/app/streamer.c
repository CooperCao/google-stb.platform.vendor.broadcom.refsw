/***************************************************************************
 *     Copyright (c) 2006-2007, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 * Video Streaming Driver for BCM7401
 * Supports both Live Record Streaming and User Playback Streaming
 * Uses Direct TX by the Ethernet Driver
 *  
 * Revision History:
 *    
 * $brcm_Log: $
 * 
 *************************************************************/ 

#include "streamer.h"

/****************************************************************
* main
*
* INPUTS:		arc - argument count
*				argv - array of arguments
* OUTPUTS:		none
* RETURNS:		none
* FUNCTION:		main entry point
* DESCRIPTION:  Provide a textual command line interface to prompt 
*               the user for various information to setup the driver 
*               for playing video and audio input.
*
****************************************************************/
int stream_create_index(streamer_t *p);
int streamer_udp_start(streamer_t *p);
void launch_chanmap_server( );

int GetInputString(FILE *f, char *s)
{
	fgets(s, 255, f);
	return strlen(s);
}


int main(int argc, char *argv[])
{
	streamer_t pb;
	char s[256];
	int i,err, indexer=0;

	pb.live_mode = 0;
	pb.fd = -1;
	pb.ch =  0;
	pb.use_mmap = 0;
	pb.kernel_mode = 1;
	pb.port = 5000;
	pb.protocol = 0;
	sprintf(pb.dest,"225.0.0.%d",pb.ch+7);
	if(argc == 1)
		goto usage;

	for (i = 1; i < argc; ++i)
	{
		if(strcmp(argv[i],"-ch")==0){ /* channel specified */
			if (sscanf(argv[++i],"%d",&pb.ch) != 1)
				goto usage;
			sprintf(pb.dest,"225.0.0.%d",pb.ch+1);
			i++;
		}
		
		if(strcmp(argv[i],"-index")==0){ /* index specified */
			if (sscanf(argv[++i],"%d",&indexer) != 1)
				goto usage;
			i++;
		}

		if(strcmp(argv[i],"-live")==0){ /* index specified */
			if (sscanf(argv[++i],"%d",&pb.live_mode) != 1)
				goto usage;
			i++;
		}

		if(strcmp(argv[i],"-mmap")==0){ /* index specified */
			if (sscanf(argv[++i],"%d",&pb.use_mmap) != 1)
				goto usage;
			i++;
		}

		if(strcmp(argv[i],"-kmode")==0){ /* index specified */
			if (sscanf(argv[++i],"%d",&pb.kernel_mode) != 1)
				goto usage;
			i++;
		}

		if(strcmp(argv[i],"-proto")==0){ /* index specified */
			if (sscanf(argv[++i],"%d",&pb.protocol) != 1)
				goto usage;
			i++;
		}

		if(strcmp(argv[i],"-dest")==0){ /* destination specified */
			if (sscanf(argv[++i],"%s",pb.dest) != 1)
				goto usage;
			i++;
		}

		if(strcmp(argv[i],"-port")==0){ /* index specified */
			if (sscanf(argv[++i],"%d",&pb.port) != 1)
				goto usage;
			i++;
		}

		if(strcmp(argv[i],"-f")==0){ /* index specified */
			if (sscanf(argv[++i],"%s",pb.filename) != 1)
					goto usage;
			if (sscanf(argv[++i],"%lx",&pb.videoPid) != 1)
					goto usage;
		}
	}
	
	if(indexer == 1){
		stream_create_index(&pb);
		return 0;
	}
	else if(indexer==2)
		stream_create_pcr_offset(&pb);

	sleep(1);
	launch_chanmap_server( );
	if(!pb.live_mode){
		if (streamer_udp_start(&pb) != 0)
			return -1;
	}
	else{
		if (livestreamer_start(&pb) != 0)
			return -1;
	}
	//launch_all_streamers(2,0);
	while(1){
		sleep(1);
	}

usage:
	printf("USAGE: streamer -ch (0|1|2) -live (0|1) -index (0|1) -proto (0|1|2) -mmap (0|1) -kmode (0|1)\n"
		    "\t\t-dest X.X.X.X -port N filename 0xPID\n"
		    "Args must be specified in order, skipping default values\n"
			"\tindex = 1 recreates an index file; index = 0 uses the already created index file\n"
		    "\tproto = 0 for UDP; 1 for RTP; 2 for TCP (default)\n"
			"\tdest = X.X.X.X IPaddr or Mcast Group; 225.0.0.7 is default\n"
			"\tport = N UDP port number (5000 is default)\n");
	return 0;
}

