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

#include <stdio.h>
#include <stdlib.h>

/*
 * Ts2pic is used to convert MPEG-TS stream file into a sequence of I-frame, P-frame files.
 * Each file composed of 1 frame. If the files are concatenated into one  file, it will be
 * a complete MPEG element stream.
 */

#define PSC_PICTURE_START        0x00
#define PSC_SEQUENCE_HEADER      0xb3
#define PSC_SEQUENCE_END         0xb7
#define PSC_PES_START            0xe0

#define TS_LEN 188      /* TS packet */
#define PAYLOAD_LEN 184  /* element stream payload len */
#define ESBUF_LEN 0x200000
#define MPEG_TS  0
#define MPEG_ES  1

unsigned char rbuf[TS_LEN];
long file_offset = 0;
unsigned int psc = PSC_SEQUENCE_END;
FILE * in;
FILE * out = NULL;
int img_index =0;
unsigned long total_frames= 1000000;
unsigned long current_frame = 0;
int stream_type = MPEG_TS;

void usage (char *cmd) {
	printf("Usage: %s {mpeg_ts|mpeg_es} <transport file> <picture files without extension> <video pid> [<num of frames>]\n"
	       "       Convert MPEG-2  transport stream into a serial of I, P pictures.\n", cmd);
}

/* return offset of the transport header, -1 if fail */
long search_ts_header(unsigned char * buf) {
	int n, i;
	long offset = ftell(in);

	while ( (n = fread(buf, TS_LEN, 1, in)) ) {
		i = 0;

		for ( i =0; i < n; i++) {
			if (buf[i] == 0x47) {
				offset += i;
				printf("Stream start at %ld\n", offset);
				return offset;
			}
		}
	}
	return -1;
}

/* return ES offset in buf if succeed, -1 if fail */
int find_pid(unsigned char * buf, unsigned long vpid) {
	unsigned long pid;
	int offset = -1;

	pid = buf[2] + ((buf[1]&0x1f) << 8);
	if ((vpid == pid)  && (buf[0] == 0x47)) {
		if (buf[1] & 0x40) {
			/* printf("\nnew pes"); */
		} else {
			/* printf("o"); */
		}
		offset = 4; /* skip ts header */
		if ((buf[3] & 0x30) == 0x20 || (buf[3] & 0x30) == 0x30 ) {
			printf("Adaptation field, len 0x%x\n", buf[4]);
			offset += (buf[4] + 1);
		}
	} else {
		/* printf("."); */
	}
	/* printf("%d ", offset); */
	return offset;
}
/* return 1 if found next start code, 0 if not.  The offset of start code  is returned in off */
int next_sc(unsigned char * buf, int len, int * off ) {
	int i = 0;
	int state = 0;

	while ( i < (len - 1)) {
		if ( buf[i] == 1 && state >= 2) {
			*off = i - 2;
			printf("%02x ", buf[i+1]);
			return 1;
		} else if (buf[i] == 0) {
			state++;
		} else {
			state = 0;
		}
		i++;
	}
	return 0;
}


void finish_current_picture (unsigned char * buf, int len) {
	/* unsigned char seq_end[4] = {0,0,1,0xb7}; */
	/* finish the picture */
	if (out) {
		fwrite(buf, 1,  len, out);
		/* fwrite(seq_end, 1, 4, out); */
		fclose(out);
		current_frame++;
		printf("== %ld frames\n", current_frame);

		if (current_frame >= total_frames)
			exit(1);
		out = NULL;
		/* exit(1);  */
	}
}

void open_new_picture( unsigned char * buf, int len, char * fn) {
	char tmp[256];
	if (out == NULL) {
		sprintf(tmp, "%s.%03d", fn, img_index++);
		if (( out = fopen(tmp, "wb")) == NULL) {
			printf("fail to open file %s\n", tmp);
			exit(1);
		}
		fwrite(buf, 1,  len, out);
	} else {
		printf("current file not closed\n");
		exit(1);
	}
	printf("\n== 0x%lx ", file_offset);
}

void continue_current_picture(unsigned char * buf, int len) {

	if (out) {
		/* continuous write */
		fwrite(buf, 1, len, out);
	}
}

int main(int argc, char * argv[]) {
	long start;
	int off;
	unsigned long vpid;
	int n;
	unsigned char * esbuf;
	int left;
	unsigned int new_psc;

	if (argc < 5) {
		printf("invalid arguments \n%d", argc);
		goto err;
	}
	if (!strncasecmp(argv[1],"mpeg_es", 128)) {
		stream_type = MPEG_ES;
		printf("MPEG_ES stream\n");
	} else 	if (!strncasecmp(argv[1],"mpeg_ts", 128)) {
		stream_type = MPEG_TS;
		printf("MPEG_TS stream\n");
	} else {
		printf("unknown stream type %s\n", argv[1]);
		goto err;
	}
	vpid = strtoul(argv[4], NULL, 0);
	printf("video pid 0x%lx\n", vpid);

	total_frames = strtoul(argv[5], NULL, 0);
	printf("total frames 0x%lx\n", total_frames);

	if  ( ( in = fopen(argv[2], "rb")) == NULL) {
		printf("fail to open file %s\n", argv[1]);
		goto err;
	}

	if (stream_type == MPEG_TS) {
		if ((start = search_ts_header(rbuf)) < 0) {
			printf("cannot find mpeg transport sync byte\n");
			goto err;
		}
	} else if (stream_type == MPEG_ES) {
		start = 0;
	}
	file_offset = start;
	fseek(in, start, SEEK_SET);
	while ( (n = fread(rbuf, 1, TS_LEN, in)) ) {
		file_offset += n;
		/* filter pid */
		if (stream_type == MPEG_TS) {
			if ((off = find_pid(rbuf, vpid)) <0 ) {
				continue;
			}
		} else if (stream_type == MPEG_ES) {
			off = 0;
		}
		esbuf = &rbuf[off];
		left = TS_LEN - off;
		/* seek start code */
		off = 0;
		while ((next_sc(esbuf, left, &off)) && left >= 3)  {
			new_psc = esbuf[off + 3];
			if ( (new_psc == PSC_SEQUENCE_HEADER) ) {
				finish_current_picture(esbuf, off);
				open_new_picture(&esbuf[off], 3, argv[3]);
				left -= (off + 3);
				esbuf += (off + 3);
				psc = new_psc;
				continue;
			} else if (new_psc == PSC_PICTURE_START) {
				if (psc == PSC_SEQUENCE_HEADER) {
					continue_current_picture(esbuf, off + 3);
					left -= (off + 3);
					esbuf+= (off + 3);
					psc = new_psc;
					continue;
				} else {
					finish_current_picture(esbuf, off);
					open_new_picture(&esbuf[off], 3, argv[3]);
					left -= (off + 3);
					esbuf+= (off + 3);
					psc = new_psc;
					continue;
				}
			} else if (new_psc == PSC_SEQUENCE_END) {
				if (psc != PSC_SEQUENCE_END) {
					finish_current_picture(esbuf, off);
					left -= (off + 3);
					esbuf += (off + 3);
					psc = new_psc;
					continue;
				} else {
					left -= (off + 3);
					esbuf += (off + 3);
					psc = new_psc;
					continue;
				}
			} else if (new_psc == PSC_PES_START) {
				finish_current_picture(esbuf, off);
				left -= (off + 3);
				esbuf += (off + 3);
				psc = new_psc;
				continue;
			} else {
				if (psc == PSC_PES_START) {
					left -= (off + 3);
					esbuf+= (off + 3);
					continue;
				} else {
					continue_current_picture(esbuf, off + 3);
					left -= (off + 3);
					esbuf+= (off + 3);
					continue;
				}
			}
		}

		/* continuous write */
		continue_current_picture(esbuf, left);

	}
	fclose(in);
	return 0;
 err:
	usage(argv[0]);
	return 1;

}
