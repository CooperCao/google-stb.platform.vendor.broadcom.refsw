/***************************************************************************
 *     (c)2012-2013 Broadcom Corporation
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
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 **************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

int g_timestampOffset = 0;
unsigned g_packetSize = 188;
unsigned short g_pid = 0;
unsigned char g_pkt[192];
unsigned long g_params[10];

#if defined BCHP_7411_VER
#define TT_MODE_PLAY				0x09
#else
#define TT_MODE_PLAY				0x00
#endif
#define TT_MODE_DISPLAY				0x01
#define TT_MODE_BUILD_REFERENCE		0x02
#define TT_MODE_PROCESS				0x03
#define TT_MODE_DISCARD_TILL_BTP	0x04
#define TT_MODE_DISPLAY_FORWARD		0x05
#define TT_MODE_DISPLAY_REWIND		0x06
#define TT_MODE_DISPLAY_PAST_BUFFER	0x07

void BuildBTPParams(unsigned long *params, int mode, int skip, int display, int discard_headend, int discard_tailend, unsigned long pts)
{
	params[0] = mode;
	params[1] = skip;		/* SKIP_COUNT */
	params[2] = display;		/* DISPLAY_COUNT */
	params[7] = discard_headend;
	params[8] = discard_tailend;
	params[9] = pts;
}

void GenerateBTP(const unsigned long *params, unsigned char *pkt, bool discarding_tailend)
{
	unsigned i;

	pkt[g_timestampOffset+0] = 0x47;   /* SYNC BYTE */
	pkt[g_timestampOffset+1] = (g_pid >> 8) & 0x1f;
	pkt[g_timestampOffset+2] = g_pid & 0xff;  /* PID */
	pkt[g_timestampOffset+3] = 0x20; /* not scrambled, adaptation_field and no payload, 0 continuity counter */
	pkt[g_timestampOffset+4] = 0xb7; /* adaptation field length is 183 - the remainder of the packet */
	if (discarding_tailend)
		pkt[g_timestampOffset+5] = 0x02; /* don't set discontinuity_indicator in this condition */
	else
		pkt[g_timestampOffset+5] = 0x82;
	pkt[g_timestampOffset+6] = 45; /* Number of relevant bytes */
	pkt[g_timestampOffset+7] = 0x00;  /* Align byte */
	pkt[g_timestampOffset+8] = 0x42;	/* B */
	pkt[g_timestampOffset+9] = 0x52;	/* R */
	pkt[g_timestampOffset+10] = 0x43;	/* C */
	pkt[g_timestampOffset+11] = 0x4d;	/* M */
	for(i=0; i<10; ++i)
	{
		int base = g_timestampOffset + 12 + i*4;
		pkt[base] = (unsigned char) ((params[i] & 0xff000000) >> 24);
		pkt[base+1] = (unsigned char) ((params[i] & 0x00ff0000) >> 16);
		pkt[base+2] = (unsigned char) ((params[i] & 0x0000ff00) >> 8);
		pkt[base+3] = (unsigned char) (params[i] & 0x000000ff);
	}
	for(i=g_timestampOffset+52; i<g_packetSize; ++i)
	{
		pkt[i] = 0x00;
	}
}

int ccounter = 0;
void GenerateData(unsigned char *pkt, int startnumber)
{
	unsigned i;

	pkt[g_timestampOffset+0] = 0x47;   /* SYNC BYTE */
	pkt[g_timestampOffset+1] = (g_pid >> 8) & 0x1f;
	pkt[g_timestampOffset+2] = g_pid & 0xff;  /* PID */
	pkt[g_timestampOffset+3] = 0x10 | ccounter; /* not scrambled, no adaptation_field, ccounter */
	if (++ccounter > 0xF)
		ccounter = 0;
	for(i=0; i<g_packetSize-4; ++i)
	{
		if (i == 0)
			pkt[i+4+g_timestampOffset] = startnumber;
		else
			pkt[i+4+g_timestampOffset] = i;
	}
}

void printUsage(void)
{
	printf(
	"Usage: btp_generator PID CMD >btpfile.dat\n"
	" CMD is a number. See code for details.\n"
	" output to stdout.\n"
	);
}

int main(int argc, char **argv)
{
	int i;
	
	if (argc < 3) {
		printUsage();
		return -1;
	}
	
	g_pid = strtoul(argv[1], NULL, 0);
	memset(g_params, 0, sizeof(g_params));
	
	switch (atoi(argv[2])) {
	case 0:
		BuildBTPParams(g_params, TT_MODE_PROCESS, 0, 0, 20, 100, 0x01234578);
		GenerateBTP(g_params, g_pkt, false);
		fwrite(g_pkt, 188, 1, stdout);
		
		GenerateData(g_pkt, 0);
		fwrite(g_pkt, 188, 1, stdout);
		break;
	case 1:
		BuildBTPParams(g_params, TT_MODE_PROCESS, 0, 0, 20, 188, 0x01234578);
		GenerateBTP(g_params, g_pkt, false);
		fwrite(g_pkt, 188, 1, stdout);
		
		GenerateData(g_pkt, 0);
		fwrite(g_pkt, 188, 1, stdout);
		
		BuildBTPParams(g_params, TT_MODE_PROCESS, 0, 0, 0, 20, 0x01234578);
		GenerateBTP(g_params, g_pkt, false);
		fwrite(g_pkt, 188, 1, stdout);
		
		GenerateData(g_pkt, 0);
		fwrite(g_pkt, 188, 1, stdout);
		break;
	case 2:
		BuildBTPParams(g_params, TT_MODE_PROCESS, 0, 0, 20, 188, 0x01234578);
		GenerateBTP(g_params, g_pkt, false);
		fwrite(g_pkt, 188, 1, stdout);
		
		for (i=0;i<10;i++) {
			GenerateData(g_pkt, 0);
			fwrite(g_pkt, 188, 1, stdout);
		}
		
		BuildBTPParams(g_params, TT_MODE_PROCESS, 0, 0, 0, 20, 0x01234578);
		GenerateBTP(g_params, g_pkt, false);
		fwrite(g_pkt, 188, 1, stdout);
		
		GenerateData(g_pkt, 0x10);
		fwrite(g_pkt, 188, 1, stdout);
		break;
	case 3:
		if (argc < 2 + 4) break;
		
		{
		int discard_headend = atoi(argv[3]);
		int num_packets_except_last = atoi(argv[4]);
		int discard_tailend = atoi(argv[5]);
		
		BuildBTPParams(g_params, TT_MODE_PROCESS, 0, 0, discard_headend, 188, 0x01234578);
		GenerateBTP(g_params, g_pkt, false);
		fwrite(g_pkt, 188, 1, stdout);
		
		for (i=0;i<num_packets_except_last;i++) {
			GenerateData(g_pkt, i);
			fwrite(g_pkt, 188, 1, stdout);
		}
		
		BuildBTPParams(g_params, TT_MODE_PROCESS, 0, 0, 0, discard_tailend, 0x01234578);
		GenerateBTP(g_params, g_pkt, false);
		fwrite(g_pkt, 188, 1, stdout);
		
		GenerateData(g_pkt, i);
		fwrite(g_pkt, 188, 1, stdout);
		}
		break;
	case 4:
		for (i=0;i<atoi(argv[3]);i++) {
			GenerateData(g_pkt, i);
			fwrite(g_pkt, 188, 1, stdout);
		}
		break;
	case 5:
		{
		int discard_headend = atoi(argv[3]);
		int num_packets_except_last = atoi(argv[4]);
		int discard_tailend = atoi(argv[5]);
		
		BuildBTPParams(g_params, TT_MODE_PROCESS, 0, 0, 0, discard_tailend, 0x01234578);
		GenerateBTP(g_params, g_pkt, false);
		fwrite(g_pkt, 188, 1, stdout);
		
		for (i=0;i<num_packets_except_last;i++) {
			GenerateData(g_pkt, i);
			fwrite(g_pkt, 188, 1, stdout);
		}
		
		BuildBTPParams(g_params, TT_MODE_PROCESS, 0, 0, discard_headend, 188, 0x01234578);
		GenerateBTP(g_params, g_pkt, false);
		fwrite(g_pkt, 188, 1, stdout);
		
		GenerateData(g_pkt, i);
		fwrite(g_pkt, 188, 1, stdout);
		}
		break;
	}
	return 0;
}
