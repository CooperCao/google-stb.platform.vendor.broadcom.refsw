/***************************************************************************
 *     Copyright (c) 1998-2013, Broadcom Corporation
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
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ****************************************************************/
#include "bstd.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ts_utils.h"

#define NULL_PID 0x1fff

static int b_get_pcr(const unsigned char *buf, uint64_t *p_pcr)
{
    uint64_t pcr;
    uint32_t pcr_ext;

    /* read 33 bits of pts info */
    pcr = (buf[6] );
    pcr <<= 8;
    pcr |= buf[7]; /* 8 bits */
    pcr <<= 8;
    pcr |= buf[8]; /* 8 bits */
    pcr <<= 8;
    pcr |= buf[9]; /* 8 bits */
    pcr <<= 1;
    pcr |= ((buf[10] >> 7) & 0x01 ); /* 1 bit */
    /* Reserved = (buf[10] & 0x7E ) >> 1; */
    pcr_ext = buf[10] & 0x01;
    pcr_ext <<= 8;
    pcr_ext |= buf[11];


    pcr = (pcr * 300) + pcr_ext;
    /* printf( "pcr = %llu (0x%8llx) \n" , pcr , pcr ); */
    *p_pcr = pcr;
    return 0;
}

static void printPcr(const unsigned char *buf, const char *header)
{
    unsigned char pcr_flag,opcr_flag;
    uint64_t pcr;

    pcr_flag  = (buf[5] & 0x10) >> 4;
    opcr_flag = (buf[5] & 0x20) >> 5;

    /* printf( ", pcr=%d , opcr=%d" , pcr_flag, opcr_flag ); */

    if ( pcr_flag ) {
        b_get_pcr( buf, &pcr);
        printf( "%spcr %s%x%08x %s" , header?header:NULL, header?header:NULL,
            (unsigned)(pcr >> 32),
            (unsigned)pcr,
            header?header:NULL);
    }
}

static void printPts(const unsigned char *buf, unsigned bufsize, const char *header)
{
    unsigned i, sccount = 0;;
    for (i=0; i<bufsize; i++) {
        if (sccount == 3) {
            if (b_is_pes_stream_id(buf[i])) {
                b_pes_header pes_header;
                b_get_pes_header(&buf[i], &pes_header);
                printf("%spts %#x", header?header:NULL, pes_header.pts);
            }
            sccount = 0;
        }
        sccount = b_check_for_start_code(buf[i], sccount);
    }    
}

static void printPacket(const unsigned char *pkt, int packetsize, const uint32_t *timestamp_value, int print_pts, int print_pcr)
{
    static int cnt = 0;

    BSTD_UNUSED(packetsize);
    printf("packet %d: pid 0x%x, pusi, %d", cnt++, b_get_pid(pkt), (pkt[1]&0x40)>>6);
    if (timestamp_value) {
        printf(", tts, %u", *timestamp_value);
    }
    
    if (print_pts) {
        printPts(pkt, packetsize, ", ");
    }

    if (print_pcr) {
        int adaptation_field_control = (pkt[3] & 0x30) >> 4;

        if (adaptation_field_control == 0x2) {
            printPcr(pkt, ", ");
        }
    }
    
    printf("\n");

    if (b_is_btp(pkt))
    {
        uint32_t *pktdata = (uint32_t *)&pkt[12];
        int mode = b_get_btp_word(pkt, 0);

        if (!(pkt[5] & 0x80))
            printf("  discontinuity_indicator is cleared\n");
        printf("  BTP packet: %s, skip %u, display %u, discard head=%u, tail=%u, PTS %#lx\n",
            b_btp_mode_str(mode), be(pktdata[1]), be(pktdata[2]), be(pktdata[7]), be(pktdata[8]), (unsigned long)be(pktdata[9]));
    }
}

void printTransportHeader(const unsigned char *buf)
{
/* TODO: fill this out */
    printf("  transport_error: %d\n", (buf[2] & 0x80)?1:0);
    printf("  transport_scrambling_control: %d\n", (buf[3] & 0xb0)>>6);
    printf("  continuity_counter: %d\n", buf[3] & 0x0F);
    printf("  adaptation_field: %s\n",
        (buf[3] & 0x30) == 0x00 ? "reserved" :
        (buf[3] & 0x30) == 0x10 ? "none" :
        (buf[3] & 0x30) == 0x20 ? "w/o payload" :
        "w/ payload");
    if (buf[3] & 0x20) {
        /* print adaptation field */
        printf("    length %d\n", buf[4]);
        printf("    discontinuity %d\n", buf[5] & 0x80?1:0);
        printf("    random_access %d\n", buf[5] & 0x40?1:0);
    }
}

void printRawPacket(const unsigned char *buf, int packetsize)
{
    int i;
    for (i=0;i<packetsize;i++)
        printf("%02x ", buf[i]);
    printf("\n");
}

static void printUsage(void) {
    printf(
    "Usage: printts OPTIONS [FILENAME] [SKIP]\n"
    "Options:\n"
    "  -all         Don't stop when you hit a bad packet\n"
    "  -bad         Only print bad packets\n"
    "  -hdr         Print the contents of the transport header\n"
    "  -pid XXX     Only print pid # XXX\n"
    );
    printf(
    "  -pktsz XXX   Use packet size XXX (default 188)\n"
    "               Use 204 to ignore appended 16 byte FEC data\n"
    "  -timestamp   Transport stream contains 4 byte prepended timestamps.\n"
    "  -raw         Print raw contents of packet\n"
    "  -pidonly     Only print the number of the pid (useful for counting pids)\n"
    );
    printf(
    "  -cc          Perform CC check (only works on single pid streams. preprocess your multipid stream with filterts first.)\n"
    "  -q           Quiet. Only print data.\n"
    "  -pts         Seach for PES headers in payload and print PTS's\n"
    "  -pcr         print PCR's in stream\n"
    "\n"
    );
    printf(
    "Parameters:\n"
    "  FILENAME   name of MPEG2 Transport file (default is stdin)\n"
    "  SKIP       # of transport packets to skip (default is 0)\n"
    );
}

int main(int argc, char **argv) {
#define MAX_PACKET_SIZE (204+4)
    unsigned char buf[MAX_PACKET_SIZE];
    FILE *file;
    const char *filename = NULL;
    int i;
    int skip = 0;
    int all = 0;
    int quiet = 0;
    int pidonly = 0;
    int onlyPrintBad = 0;
    int timestamp_offset = 0;
    unsigned short pidFilter = NULL_PID;
    int do_printTransportHeader = 0;
    int packetsize = 188;
    int raw = 0;
    int cc_check = 0;
    struct {
        int next;
        bool discontinuity;
    } current_cc = {0, true};
    int print_pts = 0;
    int print_pcr = 0;

    for (i=1;i<argc;i++) {
        if (!strcmp(argv[i], "--help")) {
            printUsage();
            exit(0);
        }
        else if (!strcasecmp("-all", argv[i]))
            all = 1;
        else if (!strcasecmp("-q", argv[i]))
            quiet = 1;
        else if (!strcasecmp("-raw", argv[i]))
            raw = 1;
        else if (!strcasecmp("-bad", argv[i]))
            onlyPrintBad = 1;
        else if (!strcasecmp("-pidonly", argv[i]))
            pidonly = 1;
        else if (!strcasecmp("-cc", argv[i]))
            cc_check = 1;
        else if (!strcasecmp("-pts", argv[i]))
            print_pts = 1;
        else if (!strcasecmp("-pcr", argv[i]))
            print_pcr = 1;
        else if (!strcasecmp("-timestamp", argv[i])) {
            timestamp_offset = 4;
            packetsize += timestamp_offset;
        }
        else if (!strcasecmp("-pid", argv[i]) && i+1<argc)
            pidFilter = strtoul(argv[++i], NULL, 16);
        else if (!strcasecmp("-pktsz", argv[i]) && i+1<argc)
            packetsize = atoi(argv[++i]);
        else if (!strcasecmp("-hdr", argv[i]))
            do_printTransportHeader = 1;
        else if (!filename)
            filename = argv[i];
        else
            skip = atoi(argv[i]);
    }

    if (!quiet) {
        printf(
        "printts, (C)2002-2011 Broadcom, Corp.\n"
        "Prints MPEG2 Transport packets.\n");
    }
    if (!filename || !strcmp(filename, "-")) {
        filename = "stdin";
        file = stdin;
    }
    else
        file = fopen(filename, "rb");
    if (file) {
        if (!quiet) {
            printf("Reading from %s\n", filename);
        }
    }
    else {
        printf("Cannot open %s\n", filename);
    }

    if (file) {
        int cnt = 0;
        const unsigned char *pkt = &buf[timestamp_offset];

        fseek(file, packetsize * skip, SEEK_SET);
        cnt = skip;

        while (!feof(file)) {
            if (fread(buf, 1, packetsize, file) != (size_t)packetsize)
                break;
            /* after reading into buf, always access pkt which skips any timestamp_offset */

            if (pkt[0] != 0x47) {
                printf("packet %d: no header: %x\n", cnt, pkt[0]);
                if (!all)
                    break;
            }

            if (pidFilter != NULL_PID && pidFilter != b_get_pid(pkt))
                continue;

            if (cc_check) {
                int this_cc = pkt[3] & 0xF;
                int adaptation_field_control = (pkt[3] & 0x30) >> 4;

                if (adaptation_field_control != 0 && adaptation_field_control != 2) {
                    if (!current_cc.discontinuity && this_cc != current_cc.next) {
                        printf("Bad CC %d at packet %d\n", this_cc, cnt);
                    }
                    current_cc.next = this_cc + 1;
                    if (current_cc.next == 0x10) current_cc.next = 0x0;
                    current_cc.discontinuity = false;
                }
            }

            if (pidonly) {
                printf("pid 0x%x\n", b_get_pid(pkt));
            }
            else {
                if (!onlyPrintBad) {
                    uint32_t timestamp_value;
                    if (timestamp_offset) {
                        timestamp_value = buf[0]<<24|buf[1]<<16|buf[2]<<8|buf[3];
                    }
                    printPacket(pkt, packetsize-timestamp_offset, timestamp_offset?&timestamp_value:NULL, print_pts,print_pcr);
                }
                if (do_printTransportHeader) {
                    printTransportHeader(pkt);
                }
                if (raw) {
                    printRawPacket(pkt, packetsize-timestamp_offset);
                }
            }
            cnt++;
        }
        fclose(file);
    }
    return 0;
}
