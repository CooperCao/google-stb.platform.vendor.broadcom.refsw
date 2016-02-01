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

int GetInputString(FILE *f, char *s)
{
	fgets(s, 255, f);
	return strlen(s);
}


int main(int argc, char *argv[])
{
	record_t r;
	char s[256];
	int i,err, indexer=0;

	r.fd = -1;
	r.ch =  0;
	r.port = 5000;
	r.protocol = 0;
	sprintf(r.addr,"225.0.0.%d",r.ch+1);
	if(argc == 1)
		goto usage;

	for (i = 1; i < argc; ++i)
	{
		if(strcmp(argv[i],"-ch")==0){ /* channel specified */
			if (sscanf(argv[++i],"%d",&r.ch) != 1)
				goto usage;
			i++;
		}

		if(strcmp(argv[i],"-proto")==0){ /* index specified */
			if (sscanf(argv[++i],"%d",&r.protocol) != 1)
				goto usage;
			i++;
		}

		if(strcmp(argv[i],"-dest")==0){ /* destination specified */
			if (sscanf(argv[++i],"%s",r.addr) != 1)
				goto usage;
			i++;
		}

		if(strcmp(argv[i],"-port")==0){ /* index specified */
			if (sscanf(argv[++i],"%d",&r.port) != 1)
				goto usage;
			i++;
		}

		if(strcmp(argv[i],"-uri")==0){ /* uri specified */
			if (sscanf(argv[++i],"%s",r.uri) != 1)
				goto usage;
			i++;
		}

		if(strcmp(argv[i],"-pcr")==0){ /* uri specified */
			if (sscanf(argv[++i],"%x",&r.pcrpid) != 1)
				goto usage;
			i++;
		}

		if(strcmp(argv[i],"-f")==0){ /* index specified */
			if (sscanf(argv[++i],"%s",r.filename) != 1)
					goto usage;
		}
	}

	printf("ch=%d file=%s proto=%d addr=%s:%d%s pcr=%x\n",r.ch,r.filename,r.protocol,r.addr,r.port,r.uri,r.pcrpid);
	if (record_start(&r) != 0)
		return -1;

	while(1){
		GetInputString(stdin, s);
		if(s[0] == 'q')
			return 0;
	}

usage:
	printf("USAGE: netrecord -ch (0|1|2) -dest X.X.X.X -port N -uri URI -pcr 0xPCRPID -f filename\n"
		    "Args must be specified in order, skipping default values\n"
		    "\tproto = 0 for UDP; 1 for RTP; 2 for TCP (UDP default)\n"
			"\tdest = X.X.X.X IPaddr or Mcast Group; 225.0.0.7 is default\n"
			"\tport = N UDP port number (5000 is default)\n"
		    "\turi  = URI e.g./stream.mpg on server\n"
		    "\tpcr  = pcrpid on hex\n"
		   );
	return 0;
}

