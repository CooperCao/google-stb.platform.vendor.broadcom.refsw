//****************************************************************************
//
// Copyright (c) 2009-2012 Broadcom Corporation
//
// This program is the proprietary software of Broadcom Corporation and/or
// its licensors, and may only be used, duplicated, modified or distributed
// pursuant to the terms and conditions of a separate, written license
// agreement executed between you and Broadcom (an "Authorized License").
// Except as set forth in an Authorized License, Broadcom grants no license
// (express or implied), right to use, or waiver of any kind with respect to
// the Software, and Broadcom expressly reserves all rights in and to the
// Software and all intellectual property rights therein.  IF YOU HAVE NO
// AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY,
// AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE
// SOFTWARE.
//
// Except as expressly set forth in the Authorized License,
//
// 1.     This program, including its structure, sequence and organization,
// constitutes the valuable trade secrets of Broadcom, and you shall use all
// reasonable efforts to protect the confidentiality thereof, and to use this
// information only in connection with your use of Broadcom integrated circuit
// products.
//
// 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
// "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
// OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
// RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
// IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
// A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
// ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
// THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
//
// 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
// OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
// INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
// RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
// HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
// EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
// WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
// FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
//
//****************************************************************************
//
//  Filename:
//  Author:
//  Creation Date:
//
//****************************************************************************
//
//****************************************************************************
//
//    Description: low-level hardware timer control
//
//****************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/ethernet.h>	/* the L2 protocols */
#include <netinet/in.h>
#include <netpacket/packet.h>
#include <string.h>
#include <syslog.h>
#include <stdarg.h>
#include <signal.h>

#include "../bcmvenet.h"

#define VERSION "1.1"

enum {
    FNAME_BOOTLOADER = 0,
    FNAME_DOCSIS0,
    FNAME_DOCSIS1,
    FNAME_PERM_NONVOL,
    FNAME_DYN_NONVOL,
    NUM_FILES
};

static char *fileNames[NUM_FILES];

#define BUFFER_SIZE (4 * 1024 * 1024)
static unsigned int buffer[BUFFER_SIZE / sizeof(unsigned int)];

static int pciDriver;
struct ifreq ifr;
static int firstIoctl = 1;

static void errorSyslog(char *s) {
    syslog(LOG_ERR, s);
}

static void infoSyslog(char *s) {
    syslog(LOG_INFO, s);
}

static void vErrorSyslog(char *s, ...) {
    va_list args;
    va_start(args, s);
    vsyslog(LOG_ERR, s, args);
}

static void vInfoSyslog(char *s, ...) {
    va_list args;
    va_start(args, s);
    vsyslog(LOG_INFO, s, args);
}

static void sighandler(int signum) {
    vErrorSyslog("Got signal %d - Terminating", signum);
    exit(1);
}

static unsigned int GetMailbox(void) {
	unsigned int mailbox;
	struct timespec delay;
	delay.tv_sec = 0;
	delay.tv_nsec = 976562;
	do {
		ifr.ifr_data = (void *)&mailbox;
		if (ioctl(pciDriver, SIOCGETMBOX, &ifr) != 0) {
			errorSyslog("IOCTL error get mailbox from BCMPCIENET driver");
			if (firstIoctl) {
				exit(1);
			} else {
				firstIoctl = 0;
			}
		} else {
			if (mailbox != 0) {
				break;
			}
		}
		nanosleep(&delay, NULL);
		if (delay.tv_nsec < 500000000) {
			delay.tv_nsec *= 2;
		}
	} while (1);
	return mailbox;
}

static error(char *msg, int ex, ...) {
    va_list args;
    va_start(args, ex);
    errorSyslog(msg);
	vsyslog(LOG_ERR, "Usage: %s bootloader docsis0 docsis1 permanentNonVol DynamicNonVol", args);
	if (ex) {
		exit(0);
	}
}

typedef struct {

    // The application signature.  It may match the chip ID or a customer product ID.
    unsigned short usSignature;
    // Major and minor software revisions no longer mean anything in particular.
    // Customers may use them for their own purposes.
    unsigned short usMajorRevision;
    unsigned short usMinorRevision;
    // Time when image was ProgramStore'd, expressed as seconds since Jan 1 1970.
    unsigned long ulcalendarTime;
    // Length of the file after compression, not including this header.
    unsigned long ulfilelength;
    // Address where the program should be loaded.
    unsigned long ulProgramLoadAddress;
    // Null-terminated file name, and padding reserved for future use.
    char cFilename[48];
    char pad[8];
    // When doing a dual binary, these are the lengths of the two parts.  The
    // sum of these should equal ulfilelength.
    unsigned long ulCompressedLength1;
    unsigned long ulCompressedLength2;
    // 16-bit crc Header checksum, not including the remaining fields.
    unsigned short usHcs;
    // Reserved for future use.
    unsigned short reserved;
    // CRC-32 of Program portion of file, not including this header.
    unsigned long ulcrc;

} BcmProgramHeader;

static void ComputeCRC8(const void *in2_str, int Length, unsigned long *crc_word) {

    int hex_index = 0;
    int old_hex_index = 0;
    int stop = 0;
    unsigned long value = 0;
    int digits = 0;
    int shift_count = 0;
    int max_shift_count = 0;
    unsigned long data_word = 0;
    unsigned long polynomial_value = 0x00000000;
    unsigned long xor_mask = 0x00000000;
    const unsigned char *DataPtr;
    int Counter;

    DataPtr = (const unsigned char *)in2_str;
    *crc_word = 0xffffffff;
    digits = 4;
    polynomial_value = 0x00001021;
    max_shift_count = digits * 4;

    Counter = 0;
    /* Read in hex digits : */
    while ((Counter < Length) || !stop) {
        if (Counter < Length) {
            value = *DataPtr;
        } else {
            stop = 1;
            value = 0;
            data_word = data_word << ((digits - (hex_index % digits)) * 4 - 8);
            max_shift_count = (hex_index % digits) * 4;
            old_hex_index = hex_index;
            hex_index = digits - 2;
        }

        data_word  = data_word << 8;
        data_word += value;

        if (hex_index % digits == (digits - 2)) {
            /* Last hex digit has been reached - process CRC : */
            for (shift_count = 0; shift_count < max_shift_count; shift_count++) {
                xor_mask =
                    ((data_word & (1L << (digits * 4 - 1))) ^ (*crc_word & (1L << (digits * 4 - 1)))) ? polynomial_value : 0L;
                *crc_word = *crc_word << 1;
                *crc_word = *crc_word ^ xor_mask;
                data_word = data_word << 1;
            }
        }

        if (!stop) {
            hex_index += 2;
            DataPtr++;
            Counter++;
        } else {
            hex_index = old_hex_index;
        }
    }
    *crc_word = ~(*crc_word);

}

static unsigned int SetupForNextDocsisFile(unsigned int fileId, char **fileName) {

    int file;
    unsigned long ulCrc;

    *fileName = fileNames[fileId];
    file = open(*fileName, O_RDONLY);
    if (file < 0) {
        return 0;
    }
    read(file, buffer, sizeof(BcmProgramHeader));
    close(file);
    ComputeCRC8(buffer, sizeof(BcmProgramHeader) - 8, &ulCrc);
    ulCrc &= 0xffff;
    if ((unsigned short)ulCrc != ntohs(((BcmProgramHeader *)buffer)->usHcs)) {
        return 0;
    }
    return ntohl(((BcmProgramHeader *)buffer)->ulfilelength) + sizeof(BcmProgramHeader);
}

static unsigned int SetupForNextFile(unsigned int fileId) {

    int file;
    unsigned int temp;
    char *fileName;
    unsigned int fileSize;
    ssize_t readLen;

	fileSize = 0;
    switch (fileId) {
    case FID_BOOTLOADER:
        fileSize = 64 * 1024;
        fileName = fileNames[FNAME_BOOTLOADER];
        break;
    case FID_DOCSIS:
        fileSize = SetupForNextDocsisFile(FNAME_DOCSIS1, &fileName);
        if (fileSize == 0) {
            fileSize = SetupForNextDocsisFile(FNAME_DOCSIS0, &fileName);
        }
        break;
    case FID_PERM_NONVOL:
    case FID_DYN_NONVOL:
        if (fileId == FID_PERM_NONVOL) {
            fileName = fileNames[FNAME_PERM_NONVOL];
        }
        else {
            fileName = fileNames[FNAME_DYN_NONVOL];
        }
        file = open(fileName, O_RDONLY);
        fileSize = 0;
        if (file >= 0) {
            if (read(file, &temp, sizeof(temp)) == sizeof(temp)) {
                int maxSize = ((fileId == FID_PERM_NONVOL) ? 64 : 128) * 1024;
                fileSize = ntohl(temp);
                if (fileSize > maxSize) {
                    fileSize = 0;
                }
            }
        }
        if (fileSize == 0) {
            infoSyslog("Unformatted nonvol");
            buffer[0] = 0xffffffff;
            close(file);
            return sizeof(buffer[0]);
        }
        close(file);
        break;
    }
    file = open(fileName, O_RDONLY);
	read(file, buffer, (fileSize + 3) & ~3);
    close(file);
    return fileSize;
}

int main(int argc, char *argv[]) {

	int fileSize, readsize, f;
	static int lastFileId;
	unsigned int mailbox;
	pid_t pid;
	BcmVEnet_ioctlparms ioctlParms;
	BcmVEnet_blockparms blockParms;
	BcmVEnet_blockparms nonZeroBlockParms = {2, 2, 2, 2};
    static char *fileTypes[] = {
        "Bootloader", "DOCSIS", "Permanent nonvol", "Dynamic nonvol"
    };

    openlog("Hostboot", LOG_CONS, LOG_DAEMON);

	// Check for 4 parameters
	vInfoSyslog("3383 ECM boot provider - version %s", VERSION);
	// Try to open the bootloader file
	if (argc < NUM_FILES) {
		error("requires 5 parameters", 1, argv[0]);
	}
	for (lastFileId = 0; lastFileId < NUM_FILES; lastFileId++) {
		f = open(argv[lastFileId + 1], O_RDONLY);
		if (f < 0) {
			char msg[128];
			sprintf(msg, "Can't find file %s", argv[lastFileId + 1]);
			error(msg, 1, argv[0]);
		}
		close(f);
        fileNames[lastFileId] = argv[lastFileId + 1];
	}

	// parameters are ok, time to fork off
	pid = fork();
	if (pid > 0) {
		return 1;
	} else if (pid < 0) {
		infoSyslog("Fork failed, continuing as application");
	}

    for (fileSize = 1; fileSize < 16; ++fileSize )
        signal(fileSize, sighandler);

	// Open the PCI driver
	pciDriver = socket(AF_INET, SOCK_DGRAM, 0);

	if (pciDriver < 0) {
		errorSyslog("Could not open the BCMPCIETH driver");
		exit(1);
	}

	strcpy(ifr.ifr_name, "bcm0");

	// Enter the infinite loop
	while (1) {
		// Read the mailbox to get the offset of the boot parameters
		// Note: This call should block till we get a mailbox value over the
		// PCI interface.
		mailbox = GetMailbox();

		switch (mailbox & MBOX_MSG_MASK) {
		case MBOX_XFER_BLOCK: {
			//zero out command byte
			mailbox &= ~MBOX_MSG_MASK;
			if (mailbox == 0) {
				infoSyslog("Transfer complete");
				break;
			}

			// Read the block parameters from the PCIe window
			ioctlParms.length = sizeof(BcmVEnet_blockparms);
			ioctlParms.from = mailbox;
			ioctlParms.to = (unsigned int)&blockParms;
			ifr.ifr_data = (void *)&ioctlParms;
			if (ioctl(pciDriver, SIOCGETWINDOW, (void *)&ifr) != 0) {
				errorSyslog("IOCTL error getting block parameters from BCMPCIENET driver");
				break;
			}
			// Make sure that the offset is non 0 after we've read it in order to avoid
			// race condition in bootloader end of transfer test;
			ioctlParms.length = sizeof(BcmVEnet_blockparms);
			ioctlParms.to = mailbox;
			ioctlParms.from = (unsigned int)&nonZeroBlockParms;
			ifr.ifr_data = (void *)&ioctlParms;
			if (ioctl(pciDriver, SIOCPUTWINDOW, (void *)&ifr) != 0) {
				errorSyslog("IOCTL error getting block parameters from BCMPCIENET driver");
				break;
			}

			if (blockParms.length == 0) {
				infoSyslog("Transfer complete");
				goto skip;
			}
            if (blockParms.fileId >= NUM_FILES) {
                errorSyslog("Got bogus file id");
                exit(0);
            }

			if (blockParms.fileId != lastFileId) {
				lastFileId = blockParms.fileId;
				fileSize = SetupForNextFile(lastFileId);
                vInfoSyslog("Sending %s, %d bytes", fileTypes[blockParms.fileId], fileSize);
				if (fileSize == 0) {
					char msg[128];
					sprintf(msg, "%s, can't open, or length of 0\n", argv[lastFileId + 1]);
					error(msg, 0, argv[0]);
					goto skip;
				}
			}

			// Write the block to the PCI window
			if ((blockParms.offset + blockParms.length) <= fileSize) {
				ioctlParms.length = blockParms.length;
			} else {
				ioctlParms.length = fileSize - blockParms.offset;
			}
			if (ioctlParms.length <= 0) {
				goto skip;
			}
			ioctlParms.from = (unsigned int)((unsigned char *)buffer + blockParms.offset);
			ioctlParms.to = blockParms.to;
			ifr.ifr_data = (void *)&ioctlParms;
			if (ioctl(pciDriver, SIOCPUTWINDOW, (void *)&ifr) != 0) {
				errorSyslog("IOCTL error get mailbox from BCMPCIENET driver");
				break;
			}

		skip:
			// Clear the boot parameters to tell the slave we are done moving this
			// bootloader segment.
			blockParms.length = 0;
			blockParms.offset = 0;
			ioctlParms.length = sizeof(blockParms);
			ioctlParms.to = mailbox;
			ioctlParms.from = (unsigned int)&blockParms;
			ifr.ifr_data = (void *)&ioctlParms;
			if (ioctl(pciDriver, SIOCPUTWINDOW, (void *)&ifr) != 0) {
				errorSyslog("IOCTL error clearing boot parameters for BCMPCIENET driver");
			}
			break;
		}
		default:
            vInfoSyslog("Unsupported mailbox command %08lx", mailbox);
			break;
		}   // end switch
	}   // end while(1)

	// In theory we should NEVER get here
	errorSyslog("Yikes...Shouldn't ever get here!!!");

	return 1;
}
