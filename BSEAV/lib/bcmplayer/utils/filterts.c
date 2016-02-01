/***************************************************************************
 *     Copyright (c) 2002-2013, Broadcom Corporation
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
 * Module Description: Converts startcode index to bcmplayer index
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "ts_utils.h"

BDBG_MODULE(filterts);

static void print_usage(void)
{
    fprintf(stderr,
    "Usage: filterts [-s|-btp] [pid1 [pid2 ...]]\n"
    "  stdin is MPEG2TS input. stdout is either filtered MPEG2TS or PES.\n"
    "  -s          = strip transport headers (only one pid)\n"
    "  -btp        = strip transport headers and trim with embedded PROCESS BTP packets (only one pid)\n"
    "  -timestamp  = packets have 4 byte timestamp prepended to each packet\n"
    );
    fprintf(stderr,
    "  -strip_timestamp = strip prepended timestamp\n"
    "  -pktsz #    = specify size of the packets\n"
    "  -remap      = rewrite pid1 as pid2\n"
    "  -pts_offset # = add a PTS offset to each specified pid. pass through other pids unmodified.\n"
    );
}

int main(int argc, char **argv)
{
#define MAXPIDS 20
    unsigned char *buf;
    unsigned short pidlist[MAXPIDS];
    unsigned totalpids = 0;
    unsigned strip = 0;
    unsigned btp_strip = 0;
    int curarg = 1;
    unsigned timestamp_offset = 0;
    bool strip_timestamp = false;
    unsigned remap = 0;
    unsigned total_packets = 0;
    unsigned packetsize = 188;
    unsigned packetnum = 1;
    unsigned pts_offset = 0;
    struct {
        unsigned data_start_byte;
        unsigned data_end_byte;
    } btp;

    memset(&btp, 0, sizeof(btp));

    while (curarg < argc) {
        if (!strcmp(argv[curarg], "-s")) {
            strip = 1;
        }
        else if (!strcmp(argv[curarg], "-btp")) {
            btp_strip = 1;
        }
        else if (!strcmp(argv[curarg], "-remap")) {
            remap = 1;
        }
        else if (!strcmp(argv[curarg], "-pts_offset") && curarg+1 < argc) {
            pts_offset = strtoul(argv[++curarg], NULL, 0);
        }
        else if (!strcmp(argv[curarg], "-timestamp")) {
            timestamp_offset = 4;
            packetsize += timestamp_offset;
        }
        else if (!strcmp(argv[curarg], "-strip_timestamp")) {
            strip_timestamp = true;
        }
        else if (!strcmp(argv[curarg], "-pktsz") && curarg+1 < argc) {
            packetsize = atoi(argv[++curarg]);
        }
        else {
            if (totalpids == MAXPIDS) {
                print_usage();
                return -1;
            }
            pidlist[totalpids] = strtoul(argv[curarg], NULL, 0);
            if (!pidlist[totalpids]) {
                fprintf(stderr, "WARN: Did you intend pid '%s'?\n", argv[curarg]);
            }
            totalpids++;
        }
        curarg++;
    }
    if (btp_strip && strip) {
        fprintf(stderr, "ERROR: Cannot specific both -s and -btp.\n");
        print_usage();
        exit(1);
    }
    if (remap && strip) {
        fprintf(stderr, "ERROR: Cannot specific both -s and -remap.\n");
        print_usage();
        exit(1);
    }
    if (pts_offset && strip) {
        fprintf(stderr, "ERROR: Cannot specific both -s and -pts_offset.\n");
        print_usage();
        exit(1);
    }
    if ((btp_strip || strip) && totalpids > 1) {
        fprintf(stderr, "ERROR: Stripping transport headers is only valid for one pid.\n");
        print_usage();
        exit(1);
    }
    if (remap && totalpids != 2) {
        fprintf(stderr, "ERROR: -remap requires exactly two pids.\n");
        print_usage();
        exit(1);
    }

#define READ_SIZE (packetsize*1024)

    buf = malloc(READ_SIZE);
    assert(buf);

    while (!feof(stdin)) {
        unsigned i, n, pos;

        n = fread(buf, 1, READ_SIZE, stdin);
        if (n <= 0)
            break;

        for (pos=0;pos<n;pos+=packetsize, packetnum++) {
            unsigned offset = 0;
            unsigned payload_start = 0;
            unsigned char *pkt = &buf[pos];
            bool selected_pid = false;

            pkt += timestamp_offset;

            if (pkt[0] != 0x47) {
                fprintf(stderr, "ERROR: Sync byte missing at packet %d. Invalid TS stream.\n", packetnum);
                exit(1);
            }

            if (remap) {
                if (b_get_pid(pkt) == pidlist[0]) {
                    /* remove old pid */
                    pkt[1] &= ~0x1f;
                    pkt[2] = 0;
                    /* set new pid */
                    pkt[1] |= (pidlist[1]>>8) & 0x1f;
                    pkt[2] = pidlist[1] & 0xff;
                }
                fwrite(&pkt[offset], 1, packetsize-offset, stdout);
                total_packets++;
                continue;
            }

            if (totalpids) {
                for (i=0;i<totalpids;i++) {
                    if (b_get_pid(pkt) == pidlist[i]) {
                        selected_pid = true;
                        break;
                    }
                }
                if (!selected_pid && !pts_offset) {
                    total_packets++;
                    continue;
                }
            }

            payload_start = 4;
            if (pkt[3] & 0x20) {
                /* is adaptation field present? */
                payload_start += pkt[4] + 1; /* then add the adaptation_field_length and 1 for the length field itself */
            }

            /* strip transport header */
            if (btp_strip || strip) {
                offset = payload_start;
            }

            /* add a pts_offset to the PES header */
            if (selected_pid && pts_offset) {
                /* this code assumes the PES header does not span a transport packet */
                unsigned sccount = 0;
                for (i=payload_start; i<packetsize - 16; i++) {
                    if (sccount == 3) {
                        if (b_is_pes_stream_id(pkt[i])) {
                            b_pes_header pes_header;
                            b_get_pes_header(&pkt[i], &pes_header);
                            if (pes_header.pes_type == b_pes_packet_type_pes) {
                                pes_header.pts += pts_offset;
                                pes_header.dts += pts_offset;
                                b_set_pes_header(&pkt[i], &pes_header);
                            }
                            break;
                        }
                        sccount = 0;
                    }
                    sccount = b_check_for_start_code(pkt[i], sccount);
                }
            }

            if (btp_strip && b_is_btp(pkt)) {
                if (b_get_btp_word(pkt, 0) == TT_MODE_PROCESS) {
                    /* DISCARD_HEADEND indexes first byte of payload. DISCARD_HEADEND = amount discarded. */
                    btp.data_start_byte = b_get_btp_word(pkt, 7);
                    /* DISCARD_TAILEND indexes first byte after payload. 188-DISCARD_TAILEND = amount discarded. */
                    btp.data_end_byte = b_get_btp_word(pkt, 8);
                    /* only applies to the next packet */
                }
            }
            else {
                const unsigned char *buf = &pkt[offset];
                unsigned size = packetsize - offset;
                if (timestamp_offset && !strip_timestamp && !offset) {
                    /* timestamp is naturally stripped if extracting payload */
                    fwrite(buf - timestamp_offset, 1, timestamp_offset, stdout);
                }
                if (btp.data_start_byte || btp.data_end_byte) {
                    /* PROCESS BTP not relative to TS header, so reset buf and size */
                    buf = pkt;
                    size = packetsize;
                    if (btp.data_start_byte) {
                        buf += btp.data_start_byte;
                        size -= btp.data_start_byte;
                        btp.data_start_byte = 0;
                    }
                    if (btp.data_end_byte) {
                        size -= (188 - btp.data_end_byte);
                        btp.data_end_byte = 0;
                    }
                }
                fwrite(buf, 1, size, stdout);
            }
            total_packets++;
        }
    }
    return 0;
}
