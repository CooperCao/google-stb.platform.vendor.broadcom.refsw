//****************************************************************************
//
// Copyright (c) 2008-2011 Broadcom Corporation
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
//*****************************************************************************
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

#include "if_net.h"
      
#define MBOX_BULKDMA_P_NVOL     0xF0000000
#define MBOX_BULKDMA_D_NVOL     0xF1000000
#define MBOX_BULKDMA_IMG_HDR    0xF2000000
#define MBOX_BULKDMA_IMG        0xF3000000
#define MBOX_BULKDMA_IMG_FULL   0xF4000000

#define RESERVED_FOR_REBOOT 	0x01000000

#define BULK_WIN_SIZE           (128 * 1024)

typedef struct BcmEnet_bootparms {
    unsigned int    offset;
    unsigned int    length;
} BcmEnet_bootparms;

typedef struct BcmEnet_ioctlparms {
    unsigned int    from;
    unsigned int    to;
    unsigned int    length;
} BcmEnet_ioctlparms;

static int pciDriver;
struct ifreq ifr;

static unsigned int GetMailbox(void)
{
    unsigned int mailbox;
    struct timespec delay;
    delay.tv_sec = 0;
    delay.tv_nsec = 976562;
    do {
        ifr.ifr_data = (void *)&mailbox;
        if (ioctl(pciDriver, SIOCGETMBOX, &ifr) < 0)
        {
            perror("IOCTL error get mailbox from BCMPCIENET driver");
            break;
        }
        if (mailbox != 0)
            break;
        nanosleep(&delay, NULL);
        if (delay.tv_nsec < 500000000)
            delay.tv_nsec *= 2;
    } while (1);
    return mailbox;
}

int determineFileSize(int f, int is_nonvol_file)
{
    int fileSize = 0;

    if (is_nonvol_file) {
	    // Determine size of file
	    if ((fileSize = lseek(f, 0, SEEK_END)) == -1)
	    {
		    printf("Can't seek to end of file\n");
	    }
    } else { 
	    char *ptr;
	    int i, len, ff_len;

	    /* skip the trailing 0xff */
	    if (lseek(f, 0, SEEK_SET) == -1)
	    {
		    printf("Can't seek to start of file\n");
		    fileSize = -1;
		    return fileSize;
	    }

	    if ((ptr = malloc(0x10000)) ==  NULL) {
		    printf("out of memory \n");
		    fileSize = -1;
		    return fileSize;
	    }
	    fileSize = 0;
	    ff_len = 0;
	    while ((len = read(f, ptr, 0x10000)) > 0) {
		    for (i = 0; i < len; i++) {
			    if (ptr[i] == (char) 0xff) {
				    ff_len++;
			    } else {
				    ff_len = 0;
			    }
		    }
		    fileSize += len;
	    }
	    if (len < 0) {
		    printf("read error\n");
		    fileSize = -1;
		    return fileSize;
	    }
	    fileSize -= ff_len;
	    free(ptr);
    }
    if (lseek(f, 0, SEEK_SET) == -1)
    {
        printf("Can't seek to start of file\n");
        fileSize = -1;
    }
    return fileSize;
}

static char *appFileName;
static int is7118 = 0;

int main(int argc, char *argv[])
{
    int bootloaderFile;
    long bootloaderSize, readsize;
    char *bootloaderImage;
    unsigned int mailbox;
    BcmEnet_bootparms bootParms;
    BcmEnet_ioctlparms ioctlParms;

    // Check for one parameters
	if (argc < 3)
	{
		printf("\nBcm7118 hostboot: 7118 BNM boot provider.\n");
		appFileName = "ecram_sto.bin.7118";
		is7118 = 1;
	}
	else
	{
		printf("\nBcm7xxx hostboot: 3255 BNM boot provider.\n");
                if (argc > 3)
                    appFileName = argv[3];
                else
		    appFileName = "ecram_sto.bin.3255";
                
		// Try to open the bootloader file
		bootloaderFile = open(argv[2], O_RDONLY);
		if (bootloaderFile <0)
		{
			printf("Can't find bootloader file %s\n", argv[2]);
			exit(1);
		}

		// Determine size of file
		bootloaderSize = determineFileSize(bootloaderFile, 0);
		printf("Bootloader file %s size is  %ld bytes\n", argv[2], bootloaderSize);

		// Allocate memory buffer for bootloader image
		bootloaderImage = malloc(bootloaderSize);
		if (bootloaderImage == NULL)
		{
			printf("Cannot allocate storage for the bootloader image\n");
			close(bootloaderFile);
			exit(1);
		}

		// Read the image into memory
		readsize = read(bootloaderFile, bootloaderImage, bootloaderSize);
		if (bootloaderSize != readsize)
		{
			printf("Can't read bootloader file %s\nBytes read = %ld\n", argv[2], readsize);
			close(bootloaderFile);
			exit(1);
		}

		// Done with the bootloader file
		close(bootloaderFile);
	}

    // Open the PCI driver
    pciDriver = socket(AF_INET, SOCK_DGRAM, 0);

    if (pciDriver < 0)
    {
        perror("Could not open the BCMPCIENET driver");
        exit(1);
    }

    strcpy(ifr.ifr_name, argv[1]);

    // Enter the infinite loop
    while (1)
    {
        // Read the mailbox to get the offset of the boot parameters
        // Note: This call should block till we get a mailbox value over the
        // PCI interface.
        mailbox = GetMailbox();

        switch (mailbox & 0xff000000)
        {
            case MBOX_BULKDMA_P_NVOL:
            case MBOX_BULKDMA_D_NVOL: 
            case MBOX_BULKDMA_IMG:
            case MBOX_BULKDMA_IMG_HDR:
            case MBOX_BULKDMA_IMG_FULL:
            {
                int errorFlag = 0;
		int f = 0;
                int fileSize;
                unsigned char *pBuffer;

                // Based on command received, open the file requested
                if ((mailbox & 0xFF000000) == MBOX_BULKDMA_P_NVOL)
                {
                    if (argc > 4)
                        f = open(argv[4], O_RDONLY);
                    else
                        f = open("PermanentNonVolSettings.bin", O_RDONLY);

		    fileSize = determineFileSize(f, 1);
		    printf("PermanentNonVolSettings.bin size is  %ld bytes\n", fileSize);

                }
                else if ((mailbox & 0xFF000000) == MBOX_BULKDMA_D_NVOL)
                {
                    if (argc > 5)
                        f = open(argv[5], O_RDONLY);
                    else
                        f = open("DynamicNonVolSettings.bin", O_RDONLY);

		    fileSize = determineFileSize(f, 1);
		    printf("DynamicNonVolSettings.bin size is  %ld bytes\n", fileSize);
                }
                else 
                {
                    f = open(appFileName, O_RDONLY);
		    fileSize = determineFileSize(f, 0);
		    printf("Docsis image %s size is  %ld bytes\n", appFileName, fileSize);

                }

                // If one of the files does not exist, or if the file size is 0,
                // set a flag accordingly
                if ((f == 0) || (fileSize == 0))
                {
                    errorFlag = 1;
                    printf("Hostboot error: File does not exist or file size is 0!\n");
                }

                // If the 3255 requested the image header, set the size accordingly.
                // If there was an error in the file, we'll still set the file size
                // to 92 (same as header).  We'll allocate the buffer for this size and
                // send out a buffer of 0's if there was an error so that the 3255 can
                // detect the error condition by reading a zero file size or an image
                //  header checksum failure
                if ((errorFlag) || ((mailbox & 0xFF000000) == MBOX_BULKDMA_IMG_HDR))
                {
                    fileSize = 92;
                }

                // Alloc the buffer and zero it out
                pBuffer = (char *) malloc (fileSize);
                memset(pBuffer, 0, fileSize);

                // If the file we opened was valid, go ahead and suck it
                // into our buffer and update the file size read.
                if (errorFlag == 0)
                {
                    if (read(f, pBuffer, fileSize) != fileSize)
                    {
                        // Declare error and return
                        printf("Couldn't read file!!\n");
                    }
                }

        	    // The 3255 sends this command if it read the header
        	    // and found an uncompressed image.  In this case, it
        	    // only needs the remaining part of the image (minus header)
        	    // as we'll copy the uncompressed image directly to the 3255's
        	    // load address.
                if ((mailbox & 0xFF000000) == MBOX_BULKDMA_IMG)
                {
        	        ioctlParms.length = fileSize-92;
	                ioctlParms.from = (unsigned int)pBuffer + 92;
                }
                else
                {
        	        ioctlParms.length = fileSize;
        	        ioctlParms.from = (unsigned int)pBuffer;
        	    }

                ioctlParms.to = (mailbox & 0x00FFFFFF) << 8;
                ifr.ifr_data = (void *)&ioctlParms;
        	    //mailbox &= 0x00000000;

                if (ioctl(pciDriver, SIOCBULKDMA, (void *)&ifr) < 0)
                {
                    perror("IOCTL error in BCMPCIENET driver");
                }

        	    if (f)
        	        close(f);

                if (pBuffer)
                    free (pBuffer);

                break;
            }

            case RESERVED_FOR_REBOOT:
            {
                //zero out command byte
                mailbox &= 0x00ffffff;		

                // Validate offset and length
                if (mailbox > (4096 - sizeof(bootParms)))
                {
                    printf("Segment requested. Offset = %08x\nError: Falls outside 4K window\n", mailbox);
                    goto skip;
                }

                // Read the boot parameters from the PCI window
                ioctlParms.length = sizeof(bootParms);
                ioctlParms.from = mailbox;
                ioctlParms.to = (unsigned int)&bootParms;
                ifr.ifr_data = (void *)&ioctlParms;
                if (ioctl(pciDriver, SIOCGETWINDOW, (void *)&ifr) < 0)
                {
                    perror("IOCTL error getting boot parameters from BCMPCIENET driver");
                    break;
                }

                bootParms.offset = htonl(bootParms.offset);
                bootParms.length = htonl(bootParms.length);
                printf("Hostboot sending secure bootloader: offset=0x%08lx len=%d\n", bootParms.offset, bootParms.length);

                // Validate offset and length
                if (bootParms.length > (4096 - 8))
                {
                    printf("Segment requested. Offset = %08x, length = %08x\nError: Falls outside 4K window\n",
                        bootParms.offset, bootParms.length);
                    goto skip;
                }

                // Write the bootloader segment to the PCI window
                ioctlParms.length = bootParms.length;
                ioctlParms.from = (unsigned int)bootloaderImage + bootParms.offset;
                ioctlParms.to = 0;
                ifr.ifr_data = (void *)&ioctlParms;
                if (ioctl(pciDriver, SIOCPUTWINDOW, (void *)&ifr) < 0)
                {
                    perror("IOCTL error get mailbox from BCMPCIENET driver");
                    break;
                }

skip:
                // Clear the boot parameters to tell the slave we are done moving this
                // bootloader segment.
                bootParms.length = 0;
                bootParms.offset = 0;
                ioctlParms.length = sizeof(bootParms);
                ioctlParms.to = mailbox;
                ioctlParms.from = (unsigned int)&bootParms;
                ifr.ifr_data = (void *)&ioctlParms;
                if (ioctl(pciDriver, SIOCPUTWINDOW, (void *)&ifr) < 0)
                {
                    perror("IOCTL error clearing boot parameters for BCMPCIENET driver");
                    break;
                }

                break;
            }
            defaults:
                printf("Hostboot: Unsupported mailbox command %08lx\n", mailbox);
                break;
        }   // end switch
    }   // end while(1)

    // In theory we should NEVER get here
    printf("Yikes...Shouldn't ever get here!!!\n");

    return 1;
}


