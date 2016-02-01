/***************************************************************
**
**  Broadcom Corp. Confidential
**  Copyright 1998-2000 Broadcom Corp.  All Rights Reserved.
**
**  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED 
**  SOFTWARE LICENSE AGREEMENT  BETWEEN THE USER AND BROADCOM.  
**  YOU HAVE NO RIGHT TO USE OR EXPLOIT THIS MATERIAL EXCEPT 
**  SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
**
**  File:         playback.c
**  Description:  70xx playback utilities
**  Created:      
**
**  REVISION:
**
**	$Log: playback.c,v $
**	Revision 1.1.1.1  2007/03/09 22:20:02  yrajakar
**	Cabledemo Checkin
**	
**	Revision 1.1.1.1  2007/03/08 18:32:51  yrajakar
**	CAbledemo code
**	
**	Revision 1.2  2006/11/20 16:24:36  yrajakar
**	mediabolic changes
**	
**	Revision 1.1  2006/11/17 02:11:59  yrajakar
**	update bclplayer
**	
**	Revision 1.1.1.1  2003/11/21 18:44:37  root
**	Initial DP922 Checkin
**	
**
**
****************************************************************/
#include "playback.h"

/* This define option uses the playback test app to directly test the miscellaneous 
 * menu options and bypassing all playback functionality
 *
 */

/*! FUNCTION PROTOTYPES */
/****************************************************************
* GetInputString
*
* INPUTS:		buffer at least 256 bytes long.
* OUTPUTS:		None
* RETURNS:		Number of characters in the string.
* FUNCTION:		Utility function which simply gets the input string.
*
****************************************************************/

int GetInputString(FILE *f, char *s)
{
	fgets(s, 255, f);
	return strlen(s);
}



/****************************************************************
* DVRMenu
*
* INPUTS:		playback_t reference
* OUTPUTS:		none
* RETURNS:		none
* FUNCTION		Provide console control of the pvr application
*
****************************************************************/

void DVRMenu(playback_t *pb)
{
	char s[256];
	while (1)
	{
		printf("\nDVR Menu:\n\n");

 		printf("\t1)  Normal...\n");
		
		if(pb->ptype != ePlayer_None){
			printf("\t2)  SlowMotion Forward\n");
			printf("\t3)  SlowMotion Backward\n");
			printf("\t4)  Frame Advance Forward\n");
			printf("\t5)  Frame Advance Backward\n");
			printf("\t6)  FastForward I and P Frames\n");
			printf("\t7)  FastForward I Frames\n");
			printf("\t8)  Rewind\n");
			printf("\tb)  BRCM Rewind\n");
	 	}	
		printf("\tp)  Pause\n");
		printf("\to)  Misc Control\n");
		printf("\ts)  SPDIF\n");
		printf("\tq)  Exit\n");
		printf("\nEnter Option:");
		GetInputString(stdin,s);
		printf("\n");
		switch (s[0])
		{
		case '1': /* normal playback */
			play_play(pb);
			break;

		case '2':  /* Slow Motion */
			play_slowmotion(pb,2);
			break;

		case '3':  /* Slow Motion Backward */
			play_slowmotion(pb,-2);
			break;

		case '4':  /* Frame Advance Forward */
			play_frameadvance(pb,1);
			break;

		case '5':  /* Frame Advance Backward */
			play_frameadvance(pb,0);
			break;
			
		case '6': /* Fast Forward 1 */ 
			play_ff(pb, 0);
			break;
		
		case '7': /* Fast Forward 2 */ 
			play_ff(pb, 1);
			break;
		
		case '8': /* REWIND */ 
			play_rw(pb);
			break;

		case 'b': /* REWIND */ 
			play_brcm_rw(pb);
			break;
			
		case 'p':
			play_pause(pb);
			break;
		
		case 'q':
		case 'Q':
			play_stop(pb);
			return;
		}
	}
}


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
int main(int argc, char *argv[])
{
	int i,no_menu;
	playback_t pb;
	char s[256];
    int err;
	
	play_init(&pb);
	
	
	for (i = 1; i < argc; ++i)
	{
		if ((strcmp(argv[i],"-f") == 0) || (strcmp(argv[i],"-F") == 0))
        	{
			if (sscanf(argv[++i],"%s",pb.filename) != 1)
				goto usage;
			printf("Filename=%s\n",pb.filename);
		}
		else if ((strcmp(argv[i],"-i") == 0) || (strcmp(argv[i],"-I") == 0))
		{
			if (sscanf(argv[++i],"%s",pb.bindexfile) != 1)
				goto usage;
			printf("NavINdex=%s\n",pb.bindexfile);
		}
	}

	printf("Arguments Parsed\n");

	if (play_start(&pb, 0) != 0)
		return -1;
	
	DVRMenu(&pb);
		
	return play_done(&pb);

usage:
	printf("USAGE: playback\n");
	printf("                [-f <filename> -i <indexname>]\n");
	return 0;
}


