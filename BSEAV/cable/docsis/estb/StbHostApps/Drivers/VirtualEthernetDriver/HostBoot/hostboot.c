/****************************************************************************
*
*  Copyright (c) 2010-2011 Broadcom Corporation
*
*  This program is the proprietary software of Broadcom Corporation and/or
*  its licensors, and may only be used, duplicated, modified or distributed
*  pursuant to the terms and conditions of a separate, written license
*  agreement executed between you and Broadcom (an "Authorized License").
*  Except as set forth in an Authorized License, Broadcom grants no license
*  (express or implied), right to use, or waiver of any kind with respect to
*  the Software, and Broadcom expressly reserves all rights in and to the
*  Software and all intellectual property rights therein.  IF YOU HAVE NO
*  AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY,
*  AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE
*  SOFTWARE.
*
*  Except as expressly set forth in the Authorized License,
*
*  1.     This program, including its structure, sequence and organization,
*  constitutes the valuable trade secrets of Broadcom, and you shall use all
*  reasonable efforts to protect the confidentiality thereof, and to use this
*  information only in connection with your use of Broadcom integrated circuit
*  products.
*
*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
*  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
*  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
*  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
*  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
*  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
*  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
*  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
*  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
*  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
*  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*
****************************************************************************
*
*  Filename: hostboot.c
*
****************************************************************************
* Description: This is the network interface driver for the virtual Ethernet 
*              interface between 7125 BNM and HOST.
*               
* Updates    : 01-05-11  jcyr.  Created.
* 
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <bcmvenet.h>

static void Help(char *msg) {
    printf("Error: %s\n\n", msg);
    printf("Usage:\n\n"
        "hostboot physicalMemorySizeM bootSegmentSizeK docsisSegmentSizeM bootloaderFileName DocsisApplicationFileName dynamicNonVolFileName permanentNonVolFileName\n\n");
    exit(-1);
}

static void LoadFile(char *fileName, void **codePtr, unsigned int *codeSize, int createIfNotFound) {
    FILE *f;
    char msg[256];
    f = fopen(fileName, "r");
    if (f == NULL) {
        if (createIfNotFound) {
            f  = fopen(fileName, "w+");
            if (f == NULL) {
                sprintf(msg, "Could not create %s", fileName);
                Help(msg);
            }
            memset(msg, 0xff, sizeof(msg));
            fwrite(msg, sizeof(msg), 1, f);
        }
        else {
            sprintf(msg, "Could not find %s", fileName);
            Help(msg);
        }
    }
    if (fseek(f, 0, SEEK_END)) {
        sprintf(msg, "Could not determine size of %s", fileName);
        Help(msg);
    }
    *codeSize = ftell(f);
    if (*codeSize != 0) {
        fseek(f, 0, SEEK_SET);
        *codePtr = malloc(*codeSize);
        if (*codePtr == NULL) {
            sprintf(msg, "Could not allocate memory buffer for %s", fileName);
            Help(msg);
        }
        if (fread(*codePtr, *codeSize, 1, f) != 1) {
	        //Error in Reading ,Default to 32b 
	        if (*codePtr ){
		        free (*codePtr);
	        }
            *codePtr = malloc(32);
            *codeSize = 32;
            if (*codePtr == NULL) {
                sprintf(msg, "Could not allocate memory buffer for %s", fileName);
                Help(msg);
            }
            memset(*codePtr, 0xff, 32);
            printf("Warning file %s has length of 0, 32 bytes of dummy data substituted\n", fileName);
        }
    }
    else {
        *codePtr = malloc(32);
        *codeSize = 32;
        if (*codePtr == NULL) {
            sprintf(msg, "Could not allocate memory buffer for %s", fileName);
            Help(msg);
        }
        memset(*codePtr, 0xff, 32);
        printf("Warning file %s has length of 0, 32 bytes of dummy data substituted\n", fileName);
    }
    fclose(f);
    printf("File %s loaded, %d bytes\n", fileName, *codeSize);
}

char *strcpy(char *, const char *);

int main(int argc, char **argv) {
    struct venet_ioctl_parm parms;
    static int sock;
    struct ifreq ifr;

    if (argc != 8)
        Help("8 parameters required");
    parms.physicalMemorySize = atoi(argv[1]) * 1024 * 1024;
    parms.bootSegSize = atoi(argv[2]) * 1024;
    parms.docsisSegSize = atoi(argv[3]) * 1024 * 1024;
    LoadFile(argv[4], &parms.bootCodePtr, &parms.bootCodeSize, 0);
    LoadFile(argv[5], &parms.docsisCodePtr, &parms.docsisCodeSize, 0);
    LoadFile(argv[6], &parms.dynNonVolPtr, &parms.dynNonVolSize, 1);
    LoadFile(argv[7], &parms.permNonVolPtr, &parms.permNonVolSize, 1);
    // Open the PCI driver
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
        Help("Could not open the BCMVENET driver");
    printf("Rebooting ECM\n");
    sleep(1);
    strcpy(ifr.ifr_name, "bcm0");
    // Reboot the ECM
    ifr.ifr_data = (void *)&parms;
    if (ioctl(sock, SIOC_HOSTBOOT, (void *)&ifr) < 0)
        Help("IOCTL error");
    return 0;
}
