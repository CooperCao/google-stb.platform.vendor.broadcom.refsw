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
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***********************************************/
#include "bstd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bcmindexer.h"
#include "bcmindexerpriv.h" /* for BNAV_frameTypeStr */
#include "bcmindexer_nav.h"
#include "tsindexer.h"
#include "tsplayer.h"
#include "mpeg2types.h"
#include "avstypes.h"
#include "bcmindexer_svc.h"

static void printUsage(void)
{
}

typedef enum {
    b_indextype_sct,
    b_indextype_sct6,
    b_indextype_bcm,
    b_indextype_avcbcm,
    b_indextype_vc1bcm,
    b_indextype_avsbcm,
    b_indextype_timestamp_only,
    b_indextype_avc_extended,
    b_indextype_unknown
} b_indextype;

static b_indextype indextype = b_indextype_unknown;

int main(int argc, char **argv) {
    int i;
    int offset = 0;
    int total = -1;
    const char *filename = NULL;
    const char *outfilename = NULL;
    FILE *file, *outfile;
    unsigned entrysize = 0;
    unsigned add_timestamp = 0;
    BNAV_Entry entry;
    bool silence = false;

    for (i=1;i<argc;i++) {
        if (!strcmp(argv[i], "--help")) {
            printUsage();
            exit(0);
        }
        else if (!strcmp(argv[i], "-add_ts")) {
            add_timestamp = strtoul(argv[++i], NULL, 0);
        }
        else if (!filename) {
            filename = argv[i];
        }
        else if (!outfilename) {
            outfilename = argv[i];
        }
        else {
            printf("Invalid parameters.\n");
            exit(1);
        }
    }

    if (!filename || !outfilename) {
        printUsage();
        return -1;
    }

    if (indextype == b_indextype_unknown) {
        indextype = b_indextype_sct; /* sct */
        if (filename) {
            int len = strlen(filename);
            if (len > 4 &&
                (!strcmp(&filename[len-4], ".nav") ||
                 !strcmp(&filename[len-4], ".bcm")))
                indextype = b_indextype_bcm;
        }
        if (!silence)
            printf("Default index type: %s\n", indextype==1?"SCT":"BCM");
    }

    file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Cannot open %s\n", filename);
        exit(1);
    }

    outfile = fopen(outfilename, "wb");
    if (!outfile) {
        fprintf(stderr, "Cannot open %s\n", outfilename);
        exit(1);
    }

    fseek(file, 0, SEEK_SET);
    if (fread(&entry, sizeof(entry), 1, file) == 1) {
        BNAV_Version ver = BNAV_get_version(&entry);
        if (!silence)
            printf("Version: %d\n", ver);
        if (ver == BNAV_Version_AVC) {
            indextype = b_indextype_avcbcm;
        }
        else if (ver == BNAV_Version_VC1_PES) {
            indextype = b_indextype_vc1bcm;
        }
        else if (ver == BNAV_Version_AVS) {
            indextype = b_indextype_avsbcm;
        }
        else if (ver == BNAV_Version_TimestampOnly) {
            indextype = b_indextype_timestamp_only;
        }
        else if (ver == BNAV_Version_AVC_Extended) {
            indextype = b_indextype_avc_extended;
        }
        else if (ver >= BNAV_VersionUnknown) {
            printf("Unknown version indicates corrupt index.\n");
            exit(1);
        }
    }

    switch (indextype) {
    case b_indextype_sct:
        entrysize = sizeof(sIndexEntry); break;
    case b_indextype_sct6:
        entrysize = sizeof(sSixWordIndexEntry); break;
    case b_indextype_bcm:
    case b_indextype_avsbcm:
    case b_indextype_timestamp_only:
        entrysize = sizeof(BNAV_Entry); break;
    case b_indextype_avcbcm:
    case b_indextype_vc1bcm:
        entrysize = sizeof(BNAV_AVC_Entry); break;
    case b_indextype_avc_extended:
        entrysize = sizeof(BNAV_Entry); break;
    default: break;
    }

    fseek(file, offset * entrysize, SEEK_SET);

    for (i=0;(total==-1 || i<total) && !feof(file);i++) {
        switch (indextype) {
        case b_indextype_bcm:
        case b_indextype_avsbcm:
        case b_indextype_timestamp_only:
            {
            /* TODO: the -o option should also determine the order! */
            BNAV_Entry entry;
            if (fread(&entry, sizeof(entry), 1, file) != 1)
                break;
            if (add_timestamp) {
                BNAV_set_timestamp(&entry, BNAV_get_timestamp(&entry) + add_timestamp);
            }
            if (fwrite(&entry, sizeof(entry), 1, outfile) != 1)
                break;
            }
            break;
        case b_indextype_avcbcm:
        case b_indextype_vc1bcm:
            {
            BNAV_AVC_Entry entry;
            if (fread(&entry, sizeof(entry), 1, file) != 1)
                break;
            if (add_timestamp) {
                BNAV_set_timestamp(&entry, BNAV_get_timestamp(&entry) + add_timestamp);
            }
            if (fwrite(&entry, sizeof(entry), 1, outfile) != 1)
                break;
            }
            break;
        case b_indextype_avc_extended:
            /* TODO */
        default:
            printf("Unknown index format\n");
        }
    }
    if (filename)
        fclose(file);
    fclose(outfile);
    return 0;
}
