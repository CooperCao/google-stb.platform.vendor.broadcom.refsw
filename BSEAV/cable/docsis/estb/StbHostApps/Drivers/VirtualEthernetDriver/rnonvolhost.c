/*
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
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>

/* 
 * DEBUG=0 --> No debug output 
 * DEBUG=1 --> Some diagnostic messages
 * DEBUG=2 --> Verbose debug, prints packet buffers recv'd
 */
#define DEBUG 0

/* Various static defines */
#define RNONVOL_COMMAND_PORT    0x7777
#define RNONVOL_RESPONSE_PORT   0x9999
#define PERM_NONVOL_SECTION	    0x01
#define DYN_NONVOL_SECTION      0x00
#define NVOL_READ		        0x20000000
#define NVOL_WRITE		        0x30000000
#define NVOL_WRITE_LAST		    0x31000000

#define MAX_MESSAGE_SIZE        1458

/* Globals */
char *permNonVolFileName;
char *dynNonVolFileName;
char *interfaceName;
unsigned int srcIpAddress;
unsigned int dstIpAddress;
unsigned char ethDeviceNeedsSetup = 0;
int fd_dyn = -1;
int fd_perm = -1;

/*
 * Function to pass I.P address from the command
 * line in the x.x.x.x format and convert it into
 * a format that can be used in socket function calls
 */
static unsigned int ParseIP(unsigned char *s)
{
	unsigned int ip;
	unsigned char octet;
	ip = 0;
	while ((*s != 0) && (*s != '.'))
	{
		octet = 0;
		while ((*s != 0) && (*s != '.'))
		{
			octet = (octet * 10) + (*s++ - '0');
		}
		if (*s == '.')
			s++;
		ip = (ip * 256) + octet;
	}
	return ip;
}

/*
 * This function checks to see if the specificed ethernet devie
 * is up.  We use this to check non-persisent ethernet devices
 * such as USB, where the ethernet device (ethx) isn't really
 * created until the device establishes a connection to this host.
 * If secsToWait is set to 0, this function will continue to loop
 * until the Ethernet interface is alive.  If secsToWait is set to
 * a non-zero value, the function will wait for that many secs.  If
 * the interface is still not up after secsToWait seconds elapse, it
 * will return with an error status.
 */
int ethInterfaceUp(int secsToWait)
{
	struct ifreq ifr;
	struct sockaddr_in *sin = (struct sockaddr_in *)&(ifr.ifr_addr);
	int sleepSecsCount = 0;
	int socketDescriptor;

	/* Set the socket options */
	memset(&ifr, 0, sizeof (struct ifreq));
	strcpy(ifr.ifr_name, interfaceName);
	sin->sin_family = AF_INET;
	sin->sin_port = 0;

	/* Create the socket. Bail if we're not able to create one */
	socketDescriptor = socket(AF_INET, SOCK_DGRAM, 0);
	if (!socketDescriptor)
	{
		/* We weren't able to create a socket - something must be
		 * seriously messed up.  Print error message and bail */
		perror("rnonvolhost");
		return -1;
	}

	while (1)
	{
		/* Now check to see if the device is up and return status */
		if (ioctl(socketDescriptor, SIOCGIFFLAGS, &ifr))
		{
			/* 
			 * If we get an ENODEV error, the device is not yet available -
			 * we'll sleep a few seconds and come back and check until:
			 * 1.  The device becomes available or
			 * 2.  We were passed a non-zero timeout, which expires.
			 */
			if (errno == ENODEV)
			{
				sleep(1);
				sleepSecsCount++;
				/* 
				 * Obviously, we went down, so when we come back up,
				 * the interface will have to be re-configured.
				 */
				if (ethDeviceNeedsSetup == 0)
					ethDeviceNeedsSetup = 1;
			}
				/*  Some other error occurred */
			else
			{
				perror("rnonvolhost: ethInterfaceUp error:");
				return -1;
			}
		}
			/* Device is there, so don't do anything. Return success */
		else
		{
			close(socketDescriptor);
			return 0;
		}

		/* Print periodic message */
		if ((sleepSecsCount % 5) == 0)
		{
			printf("rnonvolhost: Waiting for eth interface to come up.\n");
			printf("             %d seconds elapsed\n", sleepSecsCount);
		}

		if (secsToWait && (sleepSecsCount > secsToWait))
		{
			printf("rnonvolhost:  Timeout expired and eth interface still unavailable...exiting\n");
			close(socketDescriptor);
			return errno;
		}
	}
}

/*
 * This function sets up the specified Ethernet interface that we're
 * going to use for implementing remote non-vol operations.  The
 * ethInterfaceUp() function checks to see if the interface is up and
 * once it's up, this function will set it up with values read from a 
 * configuration file.  The values specified include I.P address, netmask
 * and gateway (if any).
 */
int configureEthInterface()
{
	/*
	 * We first need to bring the interface up.  Once it's up, we can set
	 * up the aliased interface with the static I.P address for remote
	 * non-vol.  The unaliased interface will run DHCP to obtain an address
	 * from the headend when the modem registers.
	 */
	struct ifreq ifr;
	struct sockaddr_in *sin = (struct sockaddr_in *)&(ifr.ifr_addr);
	int fSocketDescriptor;
	char *pBuffer = 0;
	unsigned int retVal = 0;

	/* Set the socket values */
	memset(&ifr, 0, sizeof (struct ifreq));
	strcpy(ifr.ifr_name, interfaceName);
	sin->sin_family = AF_INET;
	sin->sin_port = 0;

	ifr.ifr_flags = IFF_UP | IFF_RUNNING;

	fSocketDescriptor = socket(AF_INET, SOCK_DGRAM, 0);
	if (ioctl(fSocketDescriptor, SIOCSIFFLAGS, &ifr))
	{
		perror("rnonvolhost");
		retVal = errno;
	}

	/* Okay, the interface is now up.  Let's set it up:
	 * The interace we'll setup with a static IP will be
	 * the alias (see comments above).
	 * Allocate space for a string that can hold the new
	 * interface name, which includes the alias.
	 */
	pBuffer = (char *)malloc(strlen(interfaceName) + 3);
	strcpy(pBuffer, interfaceName);
	pBuffer = strcat(pBuffer, ":0");
	strcpy(ifr.ifr_name, pBuffer);
	sin->sin_family = AF_INET;
	sin->sin_port = 0;
	sin->sin_addr.s_addr = htonl(srcIpAddress);

	fSocketDescriptor = socket(AF_INET, SOCK_DGRAM, 0);
	/* Set the IP address. */
	if (ioctl(fSocketDescriptor, SIOCSIFADDR, &ifr) == -1)
	{
		perror("rnonvolhost");
		return errno;
	}

	free(pBuffer);
	close(fSocketDescriptor);
	return retVal;
}

static int configureNonVol(int *fd, char *cfg)
{
    *fd = open(cfg, O_RDWR);
	if (*fd < 0)
	{
        char buf[100];
		printf("Could not open %s\n", cfg);
        if (memcmp(cfg, "/dev/", 5) == 0)
            return 1;
        printf("Creating %s\n", cfg);
        *fd = creat(cfg, O_RDWR);
        if (*fd < 0)
        {
            printf("Could not create %s\n", cfg);
            return 1;
        }
        memset(buf, 0xff, sizeof(buf));
        write(*fd, buf, sizeof(buf));
	}

	return 0;
}

/*
 * This function handles non-vol reads and writes.  For non-vol reads,
 * this function will read the appropriate data from the non-vol file
 * and return the data.  For non-vol writes, this function will write 
 * data provided by the requestor to the appropriate section and return
 * the section written to and size written to the requestor.  The 
 * requestor can use the response to verify the success of the operation.
 */
static int NonVolCommandHandler(int command, int segment, int section, int size, void *buffer)
{
	int fd;
	int bytesWrittenOrRead = 0;
	int offset;

	/* Convert segment number into file offset */
	offset = segment * (MAX_MESSAGE_SIZE - 8);
	/*
	 * Check the section requested and open the appropriate file. Regardless of 
	 * the operation requested, we open the file in  append+read mode (I guess I
	 *  was being lazy, but this shouldn't hurt anything).
	 */
	if (section == PERM_NONVOL_SECTION)
	{
		fd = fd_perm;
	}
	else if (section == DYN_NONVOL_SECTION)
	{
		fd = fd_dyn;
	}

	if ((fd < 0) || ((command != NVOL_WRITE) && (command != NVOL_READ)))
	{
		/*
		 * Something bad happened above. Skip the file write
		 * and setup return value to signal error.
		 */
		bytesWrittenOrRead = 0;

#if (DEBUG > 0)
		if (fd < 0)
		{
			printf("**DEBUG**> Could not open nonvol file\n");
		}
		else
		{
			printf("**DEBUG**> Invalid nonvol command!\n");
		}
#endif
	}
	else
	{
		/* Seek to the offset specified */
		if (lseek(fd, offset, SEEK_SET) < 0)
		{
			/* Couldn't seek to offset, maybe a bad offset?? */
			perror("rnonvolhost");
			bytesWrittenOrRead = 0;
		}
		else
		{
			/* Phew! After all that, finally get to the real operation! */
			if (command == NVOL_WRITE)
			{
				/* Write data to file and check status of the write */
				bytesWrittenOrRead = write(fd, (const void *)buffer, size);
				if (bytesWrittenOrRead == -EIO)
				{
#if (DEBUG > 0)
					printf("**DEBUG**> Retrying write due to bad block\n");
#endif
					bytesWrittenOrRead = write(fd, (const void *)buffer, size);

				}
#if (DEBUG > 0)
				printf("**DEBUG**> Wrote %d bytes at offset %d\n", bytesWrittenOrRead, offset);
#endif
			}
			else
			{
				/* 
				 * Skip first 8 bytes of buffer for reads.  This is where the
				 * header will be written.
				 */
				buffer += 8;
				/* Read data from file and check status of the read */
				bytesWrittenOrRead = read(fd, buffer, size);
#if (DEBUG > 0)
				printf("**DEBUG**> Read %d bytes from offset %d\n", bytesWrittenOrRead, offset);
#endif
			}

			if (bytesWrittenOrRead != size)
			{
				/* 
				 * Print message that we didn't read/write all data.
				 * The requestor will know since the response will
				 * show that we haven't read/written what was requested.
				 */
				printf("rnonvolhost:  error: incomplete nvol operation\n");
				printf("              requested: %d bytes, actual: %d bytes", size, bytesWrittenOrRead);
			}
		}
	}

	/* 
	 * Return bytes read/written.  This will tell the caller whether
	 * the operation succeeded or not.
	 */
	return bytesWrittenOrRead;
}

int main(int argc, char *argv[])
{
	int rspSocket, cmdSocket;
	struct sockaddr_in sinStruct;
	struct sockaddr *fromSockAddr;
	int sockLen;
	char *buffer;
	unsigned int *bufPtr;
	FILE *nvolFile;
	int retCode;

	/* Sanity check for right number of args */
	if (argc != 6)
	{
		printf("Missing argument!\n");
		printf("Usage: %s interface, myipaddress, destipaddress, permnonvolfile, dynnonvolfile\n", argv[0]);
		return -1;
	}

	/*  Get all the args from command line */
	interfaceName = argv[1];
	srcIpAddress = ParseIP(argv[2]);
	dstIpAddress = ParseIP(argv[3]);
	permNonVolFileName = argv[4];
	dynNonVolFileName = argv[5];

	if (configureNonVol(&fd_perm, permNonVolFileName) || configureNonVol(&fd_dyn, dynNonVolFileName))
	{
		printf("rnonvolhost:  error: could not configure NonVol %s\n");
		return -1;
	}
	/* 
	 * Check if our eth interface is up.  This function will wait until the interace
	 * comes up if we pass it 0 as a parameter.  If the parameter is non-zero, it will
	 * wait for that many seconds before bailing.
	 */
	if (ethInterfaceUp(0))
	{
		printf("rnonvolhost:  error: Eth interface never came up...exiting\n");
		return -1;
	}

	/* Ethernet interface is up; configure it */
	if (configureEthInterface())
	{
		/* 
		 * Something bad happened and we couldn't setup
		 * the interface...print diag message and bail
		 */
		printf("rnonvolhost:  error: could not configure eth interace %s\n", interfaceName);
		return -1;
	}
	ethDeviceNeedsSetup = 0;

	/*
	 * Everything's good at this point, so lets open up the sockets on which we'll
	 * listen for non-vol requests. 
	 */

	/* Create the sockets for receiving commands and sending responses */
	rspSocket = socket(AF_INET, SOCK_DGRAM, 0);
	cmdSocket = socket(AF_INET, SOCK_DGRAM, 0);

	bzero((char *)&sinStruct, sizeof (sinStruct));
	sinStruct.sin_family = AF_INET;
	sinStruct.sin_addr.s_addr = htonl(srcIpAddress);

	/* Setup cmd socket and bind it to the designated port */
	sinStruct.sin_port = htons(RNONVOL_COMMAND_PORT);
	if (bind(cmdSocket, (struct sockaddr *)&sinStruct, sizeof (sinStruct)) < 0)
	{
		/* Handle error */
		printf("rnonvolhost:  Couldn't bind command socket!\n");
		perror("rnonvolhost");
		return -1;
	}

	/* Allocate space for receiving commands */
	buffer = (char *)malloc(MAX_MESSAGE_SIZE);

	/* Enter main loop where we'll wait on the cmd port and respond to requests */
	while (1)
	{
		int numBytesRead;
		int retVal = 0;
		fd_set rfds;
		struct timeval tv;

		/*
		 * Initialize the temporary pointer that we'll use
		 * to increment and extract command from buffer
		 */
		bufPtr = (unsigned int *)buffer;

		/*
		 * Check if the ethernet device we're using is still up.  It could
		 * have gone away if the other side reset.  
		 */
		retCode = ethInterfaceUp(0);
		if (retCode)
		{
			/* Something else is wrong, so we should probably bail */
			perror("rnonvolhost");
			printf(" Bailing...\n");
			return -1;
		}
		else if (ethDeviceNeedsSetup)
		{
			if (configureEthInterface())
			{
				printf("rnonvolhost:  Could not configure ethernet interface %s\n", interfaceName);
				perror("rnonvolhost");
			}
			ethDeviceNeedsSetup = 0;
		}

		numBytesRead = 0;
		/* Clear out the receive buffer */
		memset((void *)buffer, 0, MAX_MESSAGE_SIZE);

		/* Set up the timeval struct */
		tv.tv_sec = 2;
		tv.tv_usec = 0;

		/*
		 * This is inefficient, but I can't see a way around it:
		 * We use a select call to check for messages every 2 seconds instead of using a 
		 * blocking recvfrom.  The reason we do this is to make sure we can detect that 
		 * the eth interface we're using is unavailable (when the other side resets). If we
		 * use a blocking recvfrom, the call will simply block forever waiting for a message on
		 * the socket that will never arrive.
		 */
		FD_ZERO(&rfds);
		FD_SET(cmdSocket, &rfds);
		retVal = select(cmdSocket + 1, &rfds, (fd_set *)0, (fd_set *)0, &tv);
		if (retVal == -1)
		{
			/* We errored.  Print error message only for now */
			perror("rnonvolhost");
		}
		else if (retVal == 0)
		{
			// printf("rnonvolhost: No data received!\n");
		}
		else
		{
			sockLen = sizeof (fromSockAddr);

			/* Receive data from the cmd socket.  This will block until data is received */
			numBytesRead = recvfrom(cmdSocket, buffer, MAX_MESSAGE_SIZE,
									0, (struct sockaddr *)&fromSockAddr, &sockLen);
			/* Check if socket receive was successul */
			if (numBytesRead <= 0)
			{
				printf("rnonvolhost:  Socket Receive Error\n");
				perror("error");
			}
			else
			{
				int cmd, segment, section, length, cmdSegment, secLength;
				int bytesDone = 0;
				int count;
				char *cmdData;
				struct sockaddr_in sockAddrIn;

				/* Set up the socket response struct */
				bzero((char *)&sockAddrIn, sizeof (sockAddrIn));
				sockAddrIn.sin_family = AF_INET;
				sockAddrIn.sin_port = htons(RNONVOL_RESPONSE_PORT);
				sockAddrIn.sin_addr.s_addr = htonl(dstIpAddress);

#if (DEBUG > 0)
				printf("**DEBUG**> Received %d bytes on nvol cmd socket\n", numBytesRead);
#endif
				/* We received data.  Let's parse it and try to respond to it */
#if (DEBUG > 1)
				{
					int i;
					for (i = 0; i < numBytesRead; i++)
					{
						printf("%x\n", (unsigned char)buffer[i]);
						if ((i > 0) && ((i % 16) == 0))
							printf("\n");
					}
					printf("\n");
				}
#endif

				bufPtr = (unsigned int *)buffer;
				/* The first word contains the command and segment to read/write */
				cmdSegment = ntohl(*bufPtr);
				bufPtr++;

				/* The second word contains the nonvol section and length of data */
				secLength = ntohl(*bufPtr);
				bufPtr++;

				/* Parse out all the parameters */
				cmd = cmdSegment & 0xff000000;
				segment = cmdSegment & 0x00ffffff;
				section = (secLength & 0xffff0000) >> 16;
				length = secLength & 0x0000ffff;

#if (DEBUG > 0)
				printf("**DEBUG**>  Cmd: %02x  Segment: %02x\n", cmd, segment);
				printf("            Sec: %02x  Length: %02x\n", section, length);
#endif
				/* 
				 * Since the same function handles both read & write, we'll have to
				 * allocate buffer space for the read before we call the function.
				 * Check the command we received and do it now.
				 */
				if (cmd == NVOL_WRITE || (cmd == NVOL_WRITE_LAST))
				{
					cmd = NVOL_WRITE;
					cmdData = (char *)bufPtr;
				}
				else if (cmd == NVOL_READ)
				{
					cmdData = (char *)malloc(length + 8);
				}
				else
				{
					printf("rnonvolhost:  Did not understand command!\n");
				}

				/* Finally, perform the non-vol operation */
				bytesDone = NonVolCommandHandler(cmd, segment, section, length, (void *)cmdData);
				if (bytesDone != length)
				{
					printf("rnonvolhost:  Nonvol operation return size did not match specified size\n");
					printf("              specified size: %d  command returned: %d\n", length, bytesDone);
				}

				/*
				 * Let's send the response to the requester.  We'll simply use the command buffer and 
				 * only modify the length field.  The requestor can verify if the operation was successul 
				 * by comparing the size specified in the command to the actual size we operated on, 
				 * returned in this message.
				 */
				secLength = htonl(((section << 16) | bytesDone));
				memcpy((void *)&buffer[4], (const void *)&secLength, sizeof (secLength));

				/* For non-vol write commands, we'll just send back the first 8 bytes */
				if (cmd == NVOL_WRITE)
				{
					bytesDone = 8;
				}

				/*
				 *  Copy the header into the buffer.  We wont do this for writes, as we'll
				 *  simply re-use what was sent to us.
				 */
				if (cmd == NVOL_READ)
				{
					memcpy((void *)cmdData, (const void *)buffer, sizeof(unsigned int) * 2);
					/* Add the size of the header to the size of data we're going to send */
					bytesDone += 8;
				}

#if (DEBUG > 0)
				printf("**DEBUG**> Sending response of %d bytes\n", bytesDone);
#endif
				/* Send response back to caller */
				if (cmd == NVOL_READ)
				{
					count =
						sendto(rspSocket, (const void *)cmdData, bytesDone,
							   0, (struct sockaddr *)&sockAddrIn, sizeof (sockAddrIn));	
				}
				else
				{
					count =
						sendto(rspSocket, (const void *)buffer, bytesDone,
							   0, (struct sockaddr *)&sockAddrIn, sizeof (sockAddrIn));
				}
				if (count < 0)
				{
					printf("rnonvolhost:  Could not send message via response socket: %d!\n", count);
					perror("error");
				}

				if (cmd == NVOL_READ)
				{
					/* Free the buffer we allocated for reading data */
					free(cmdData);
				}
			}
		}
	}
}
