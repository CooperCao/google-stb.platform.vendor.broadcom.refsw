/***************************************************************************
 *     Copyright (c) 2005-2009, Broadcom Corporation
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
 * Module Description: Transport Stream Index Player
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***********************************************************/
#include "bstd.h"
#include "bkni.h"
#include "bcmplayer.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

static struct {
	eBpPlayModeParam playMode;
	const char *name;
} g_playModes[] = {
	{eBpPlayNormal, "normal"},
	{eBpPlayNormalByFrames, "normalbyframes"},
	{eBpPlayI, "i"},
	{eBpPlaySkipB, "skipb"},
	{eBpPlayIP, "ip"},
	{eBpPlayBrcm, "brcm"},
	{eBpPlayDecoderGOPTrick, "gop"},
};

static eBpPlayModeParam g_convert_playModeStr(const char *name) 
{
	unsigned i = 0;
	for (i=0;i<sizeof(g_playModes)/sizeof(g_playModes[0]);i++) {
		if (!strcasecmp(name, g_playModes[i].name)) {
			return g_playModes[i].playMode;
		}
	}
	printf("invalid mode: %s\n", name);
	return eBpPlayNormal;
}

static const char *g_convert_playMode(eBpPlayModeParam playMode) 
{
	unsigned i = 0;
	for (i=0;i<sizeof(g_playModes)/sizeof(g_playModes[0]);i++) {
		if (playMode == g_playModes[i].playMode) {
			return g_playModes[i].name;
		}
	}
	return "unknown";
}

int main(int argc, char **argv)
{
	BNAV_Player_Handle player;
	BNAV_Player_Settings cfg;
	BNAV_Player_PlayMode playMode;
 	BNAV_Player_PlayEntry bcmEntry;
	FILE *mpegfile, *indexfile, *outfile;
	int rc;
	unsigned char packetBuffer[188];
	unsigned short pid;
	char buf[256];
	int speed = -1, startindex = 200;
	
	BKNI_Init();
	BDBG_Init();
	BDBG_SetModuleLevel("bcmplayer", BDBG_eMsg);

	if (argc < 5) {
		printf("usage: mkbtpfile datafile indexfile pid outputfile\n");
		return 1;
	}
		 
	mpegfile = fopen(argv[1], "r");
	indexfile = fopen(argv[2], "r");
	assert(mpegfile);
	assert(indexfile);
	
	pid = strtoul(argv[3], NULL, 0);
	outfile = fopen(argv[4], "w+");
	
	printf("Select play mode ([Brcm], I, IP, SkipB, GOP, Normal, NormalByFrames):\n");
	fgets(buf, 256, stdin);
	playMode.playMode = eBpPlayBrcm;
	if (*buf && *buf != '\n') {
		buf[strlen(buf)-1] = 0;
		playMode.playMode = g_convert_playModeStr(buf);
	}
		
	printf("Enter mode modifier (default: -1):\n");
	fgets(buf, 256, stdin);
	if (*buf && *buf != '\n')
		speed = atoi(buf);
	
	printf("Enter start index (default: 200):\n");
	fgets(buf, 256, stdin);
	if (*buf && *buf != '\n')
		startindex = atoi(buf);
		
	printf("Trick mode: %s, mode %d, start %d\n",
		g_convert_playMode(playMode.playMode), speed, startindex);
	
	BNAV_Player_GetDefaultSettings(&cfg);
	cfg.videoPid = pid;
	cfg.filePointer = indexfile;
	cfg.readCb = (BP_READ_CB)fread;
	cfg.tellCb = (BP_TELL_CB)ftell;
	cfg.seekCb = (BP_SEEK_CB)fseek;
	
	rc = BNAV_Player_Open(&player, &cfg);
	assert(!rc);

	playMode.playModeModifier = speed;
	playMode.loopMode = eBpSinglePlay;
	playMode.disableExtraBOptimization = 0;
	BNAV_Player_SetPlayMode(player, &playMode);

	BNAV_Player_SetCurrentIndex(player, startindex);
		
	while (!BNAV_Player_GetNextPlayEntry(player, &bcmEntry, packetBuffer)) {
		if (bcmEntry.isInsertedPacket) {
			fwrite(packetBuffer, 1, 188, outfile);
		}
		else {
#define BUFSIZE (64*1024)
			char buf[BUFSIZE];
			int read = 0;
			int zero_at = bcmEntry.byteCount - bcmEntry.zeroByteCountEnd; /* index
				at which we start zeroing */
				
			if (bcmEntry.zeroByteCountEnd)
				printf("zero_at %d\n", zero_at);
			
			fseek(mpegfile, bcmEntry.startOffset, SEEK_SET);
			while ((unsigned long)read < bcmEntry.byteCount) {
				int n = bcmEntry.byteCount - read; /* amount we can/should read */
				if (n > BUFSIZE)
					n = BUFSIZE;
				if (fread(buf, 1, n, mpegfile) != (size_t)n)
					return -1;
					
				/* zero out unwanted data */
				if (read+n >= (int)zero_at && bcmEntry.zeroByteCountEnd) {
					if (read > zero_at) {
						printf("zeroing %d, %d\n", read, n);
						memset(&buf[read], 0, n); 
					}
					else {
						printf("zeroing %d, %d\n", zero_at-read, n-(zero_at-read));
						memset(&buf[zero_at-read], 0, n - (zero_at-read)); 
					}
				}
				
				fwrite(buf, 1, n, outfile);
				read += n;
			}
		}
	}

	fclose(mpegfile);
	fclose(indexfile);
	fclose(outfile);
	return 0;
}
