//****************************************************************************
//
// Copyright (c) 2008-2013 Broadcom Corporation
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
// ****************************************************************************
//
// Filename:       rProgramStoreHost.c
// Author:         Pinar Taskiran, Karthik Balasubramanian
				    
// Creation Date:  Nov 29, 2006
  
// *****************************************************************************
// Description:
//      The purpose of this driver is to handle storing a new image to one of
//      two slots. This driver processes commands from eCM and handles the 
//		storage of single eCM images and/or monolithic settop images.  

//		Monolithic image header and image CRC validations are customer-specific.
 
//*****************************************************************************/

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

// For the portable types (uint32, etc).
#include "bcmtypes.h"
#include "crc.h"
#include "rProgramStore.h"
#include "rProgramStoreCustomerApi.h"
/* 
 * DEBUG=0 --> No debug output 
 * DEBUG=1 --> Some diagnostic messages
 * DEBUG=2 --> Verbose debug, prints packet buffers recv'd
 */
#define DEBUG 0

typedef struct
{
   uint16 usSignature;     // the unique signature may be specified as a command
                           // line option:  The default is: 0x3350

   uint16 usControl;       // Control flags; the 8 lsbs indicate the compression
                           // type that is used.  Other bits are reserved for
                           // feature specifications.
	#define  PGM_CONTROL_COMPRESSION_BITS                   0x0007
	#define  PGM_CONTROL_FEATURE_BITS                       0xfff8
   
	#define  PGM_CONTROL_COMPRESSED_IMAGE_NO_COMPRESSION    0x0000
	#define  PGM_CONTROL_COMPRESSED_IMAGE_LZRW1_KH          0x0001
	#define  PGM_CONTROL_COMPRESSED_IMAGE_MINILZO           0x0002
	#define  PGM_CONTROL_COMPRESSED_IMAGE_RESERVED          0x0003  // Skipped
	#define  PGM_CONTROL_COMPRESSED_IMAGE_NRV2D99           0x0004
	#define  PGM_CONTROL_COMPRESSED_IMAGE_LZMA              0x0005

   uint16 usMajorRevision; // Major SW Program Revision
   uint16 usMinorRevision; // Minor SW Program Revision
                           // From a command line option this is specified as xxx.xxx
                           // for the Major.Minor revision (note: Minor Revision is 3 digits)

   uint32 ulcalendarTime;  // calendar time of this build (expressed as seconds since Jan 1 1970)


   uint32 ulfilelength; // length of Program portion of file (after compressed, if compressed)

   uint32 ulProgramLoadAddress; // Address where program should be loaded (virtual, uncached)

   #define PROGRAM_STORE_MAX_FILENAME_LENGTH 64
   
   char cFilename[PROGRAM_STORE_MAX_FILENAME_LENGTH];     // NULL terminated filename only (not pathname)

   uint16 usHcs;     // 16-bit crc Header checksum (CRC_CCITT) over the header [usSignature through cFilename]
   uint16 reserved;  // reserved
   uint32 ulcrc;     // CRC-32 of Program portion of file (following the header)

} BcmProgramHeader;

/* Various static defines */
static int rspSocket;
static 	struct sockaddr_in sockAddrIn;

/*********************************************************/
/* static defines used for program storage optimization. */
/*********************************************************/
static int bytesUsed = 0;
static bool writeToStorage = false;
char *segmentBuffer = NULL;
int bufferSize = SEGMENT_SIZE*2;

#define bufferOffsetAdjustment  (bufferSize - SEGMENT_SIZE);

/* Globals */
char *monolithic_image = NULL;

char *image1 = NULL;
char *image1_temp = NULL;

char interfaceName[32] = "eth0:0";
char srcIpAddressStr[32] = "192.168.17.10";
char dstIpAddressStr[32] = "192.168.17.1";
unsigned int srcIpAddress = 0;
unsigned int dstIpAddress = 0;
unsigned char ethDeviceNeedsSetup = 0;

static int ProcessEcmMessages(char* buffer);
static void ProgramStoreDriverPrintProgramHeader( BcmProgramHeader * pProgramHeader );
static unsigned int IsImageValid(int fd, uint32 offset, uint32 print );

static bool SwUpgradeIsStarting( int command, int segment );
static bool UpdateProgramStoreRegion( int section );
static char * ComputeTempProgramStoreRegion( int section );

static void ClearTemporaryEcmProgramStorage(void);
static int OpenOrCreateTemporaryEcmProgramStorage(void);
static void UpgradeEcmImage(void);

static void ResetBufferSettings(void);
static char* BufferSegments(char *bufPtr, int length);

/*
 * Function to pass I.P address from the command
 * line in the x.x.x.x format and convert it into
 * a format that can be used in socket function calls
 */
static unsigned int ParseIP (unsigned char *s)
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

static unsigned int min( unsigned int x, unsigned int y )
{
    if ( x < y )
        return x;
    else
        return y;
}

#if defined (STANDALONE_APP)
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
int ethInterfaceUp (int secsToWait)
{
  struct ifreq ifr;
  struct sockaddr_in *sin = (struct sockaddr_in *) &(ifr.ifr_addr);
  int sleepSecsCount = 0;
  int socketDescriptor;

  /* Set the socket options */
  memset (&ifr, 0, sizeof (struct ifreq));
  strcpy (ifr.ifr_name, interfaceName);
  sin->sin_family = AF_INET;
  sin->sin_port = 0;

  /* Create the socket. Bail if we're not able to create one */
  socketDescriptor = socket (AF_INET, SOCK_DGRAM, 0);
  if (!socketDescriptor)
  {
    /* We weren't able to create a socket - something must be
     * seriously messed up.  Print error message and bail */
    perror ("rProgramStore Host");
    return -1;
  }

  while (1)
  {
    /* Now check to see if the device is up and return status */
    if (ioctl (socketDescriptor, SIOCGIFFLAGS, &ifr))
    {
      /* 
       * If we get an ENODEV error, the device is not yet available -
       * we'll sleep a few seconds and come back and check until:
       * 1.  The device becomes available or
       * 2.  We were passed a non-zero timeout, which expires.
       */
      if (errno == ENODEV)
      {
        sleep (1);
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
        perror ("rProgramStore Host: ethInterfaceUp error:");
        return -1;
      }
    }
    /* Device is there, so don't do anything. Return success */
    else
    {
      close (socketDescriptor);
      return 0;
    }

    /* Print periodic message */
    if ((sleepSecsCount % 5) == 0)
    {
      printf ("rProgramStore Host: Waiting for eth interface to come up.\n");
      printf ("             %d seconds elapsed\n", sleepSecsCount);
    }

    if (secsToWait && (sleepSecsCount > secsToWait))
    {
      printf ("rnonvolhost:  Timeout expired and eth interface still unavailable...exiting\n");
      close (socketDescriptor);
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
int configureEthInterface ()
{
  /*
   * We first need to bring the interface up.  Once it's up, we can set
   * up the aliased interface with the static I.P address for remote
   * non-vol.  The unaliased interface will run DHCP to obtain an address
   * from the headend when the modem registers.
   */
  struct ifreq ifr;
  struct sockaddr_in *sin = (struct sockaddr_in *) &(ifr.ifr_addr);
  int fSocketDescriptor;
  char *pBuffer = 0;
  unsigned int retVal = 0;


  /* Set the socket values */
  memset (&ifr, 0, sizeof (struct ifreq));
  strcpy (ifr.ifr_name, interfaceName);
  sin->sin_family = AF_INET;
  sin->sin_port = 0;

  ifr.ifr_flags = IFF_UP | IFF_RUNNING;

  fSocketDescriptor = socket (AF_INET, SOCK_DGRAM, 0);
  if (ioctl (fSocketDescriptor, SIOCSIFFLAGS, &ifr))
  {
    perror ("rProgramStore Host");
    retVal = errno;
  }

  /* Okay, the interface is now up.  Let's set it up:
   * The interace we'll setup with a static IP will be
   * the alias (see comments above).
   * Allocate space for a string that can hold the new
   * interface name, which includes the alias.
   */
  pBuffer = (char *) malloc (strlen (interfaceName) + 3);
  strcpy (pBuffer, interfaceName);
  pBuffer = strcat (pBuffer, ":0");
  strcpy (ifr.ifr_name, pBuffer);
  sin->sin_family = AF_INET;
  sin->sin_port = 0;
  sin->sin_addr.s_addr = htonl (srcIpAddress);

  fSocketDescriptor = socket (AF_INET, SOCK_DGRAM, 0);
  /* Set the IP address. */
  if (ioctl (fSocketDescriptor, SIOCSIFADDR, &ifr) == -1)
  {
    perror ("rProgramStore Host");
    return errno;
  }

  free (pBuffer);
  close (fSocketDescriptor);
  return retVal;
}
#endif

/*
 * This function handles non-vol reads and writes.  For non-vol reads,
 * this function will read the appropriate data from the non-vol file
 * and return the data.  For non-vol writes, this function will write 
 * data provided by the requestor to the appropriate section and return
 * the section written to and size written to the requestor.  The 
 * requestor can use the response to verify the success of the operation.
 */

/* for monolithic image download, open the file only once, assuming write is in sequence,
 * as DLSS does not support lseek.
 */
#define OPEN_ONCE
static int NonVolCommandHandler (int command, int segment, int section, int size, void *buffer)
{
#ifdef OPEN_ONCE
  static int fd = -1;
#else
  int fd = -1;
#endif

  int bytesWrittenOrRead = 0;
  int offset;
  char *programStoreRegionName;

  /* Convert segment number into file offset */
  offset = segment; 

  /*
   * Check the section requested and open the appropriate file. Regardless of 
   * the operation requested, we open the file in  append+read mode (I guess I
   *  was being lazy, but this shouldn't hurt anything).
   */
  programStoreRegionName = ComputeTempProgramStoreRegion( section );
 
  if( programStoreRegionName == NULL )
  {
	  printf ("****** programStoreRegionName == NULL *******\n");
	  return bytesWrittenOrRead;
  }

  if( SwUpgradeIsStarting( command, segment) )
  {
		/* DOCSIS-only case */
		if( !(section & kMonolithic) )
		{
			printf ("\n**********************************************************\n"
				    " Starting software upgrade..Using symbolic link [ %s ]    \n"
				    " %s points to [ %s ] \n"
					"**********************************************************\n"
					, programStoreRegionName, programStoreRegionName, 
						kDefaultNewEcmImageStoragePath? kDefaultNewEcmImageStoragePath: "NULL");
			fd = OpenOrCreateTemporaryEcmProgramStorage();
		}
		else
		{
			printf ("\n**********************************************************\n"
				    " Starting software upgrade..Using symbolic link [ %s ]    \n"
				    " %s points to [ %s ] \n"
					"**********************************************************\n"
					, programStoreRegionName, programStoreRegionName, 
						kDefaultNewMonolithStoragePath? kDefaultNewMonolithStoragePath: "NULL");
			fd = open (programStoreRegionName, O_CREAT | O_WRONLY | O_TRUNC);
		}
  }
  else
  {
	#ifndef OPEN_ONCE
	fd = open (programStoreRegionName, O_RDWR);
	#else
	// Handle the specific case for validating CRC after eCM runs SHA-digest on the image with secure header stripped off.
	if((command == PROG_STORE_COMPUTE_CRC) || (command == PROG_STORE_COMPUTE_CRC_AND_LOCKIN))
	{
		if(fd < 0)
			fd = open (programStoreRegionName, O_RDWR);
	}
	#endif
  }

  if ((fd < 0) || ((command != PROG_STORE_WRITE)		&& 
					  (command != PROG_STORE_COMPUTE_CRC)	&& 
					  (command != PROG_STORE_COMPUTE_CRC_AND_LOCKIN)	&& 
					  (command != PROG_STORE_VALIDATE_IMGHDR)     ))
  {
    /*
     * Something bad happened above. Skip the file write
     * and setup return value to signal error.
     */
    bytesWrittenOrRead = 0;

	#if (DEBUG > 0)
    if (fd < 0)
    {
      printf ("**DEBUG**> Could not open nonvol file %s \n", programStoreRegionName);
    }
    else 
    {
      printf ("**DEBUG**> Invalid nonvol command!\n");
    }
	#endif
  }
  else
  {
#ifndef OPEN_ONCE
    /* Seek to the offset specified */
    if (lseek (fd, offset, SEEK_SET) < 0)
    {
      /* Couldn't seek to offset, maybe a bad offset?? */
      perror ("rProgramStore Host - Couldn't seek to offset");
      bytesWrittenOrRead = 0;
    }
    else
#endif
    {
		/* Phew! After all that, finally get to the real operation! */
		if (command == PROG_STORE_WRITE)
		{
			/* Write data to file and check status of the write */
			bytesWrittenOrRead = write (fd, (const void *) buffer, size);
			#if (DEBUG > 0)
			printf ("**DEBUG**> Wrote %d bytes at offset %d\n", bytesWrittenOrRead, offset);
			#endif

			if (bytesWrittenOrRead != size)
			{
				/* 
				 * Print message that we didn't read/write all data.
				 * The requestor will know since the response will
				 * show that we haven't read/written what was requested.
				 */
				printf ("rProgramStore Host:  error: incomplete nvol operation\n");
				printf ("              requested: %d bytes, actual: %d bytes", size, bytesWrittenOrRead);
			}
		}
		else if((command == PROG_STORE_COMPUTE_CRC) || (command == PROG_STORE_COMPUTE_CRC_AND_LOCKIN))
		{
			printf ("File download is complete. Validating image...\n");
      
			/* 
			 * Is image valid?
			 * ( Should change the name of variable "bytesWrittenOrRead" to
			 * something more meaningful. )
			 */
			if(section & kMonolithic)
			{
                printf("closing file handler\n");
                if (fd >= 0 ) {
                    close(fd);
                    fd  = -1;
                }
				bytesWrittenOrRead = EstbSwUpgrade_IsMonolithicImageValid();
			}
			else
			{
                /* Move the file pointer o to the end of the file */
                size = lseek(fd, 0, SEEK_END);
				printf ("Calling IsImageValid - offset is %d..\n", offset);
				bytesWrittenOrRead = IsImageValid(fd, offset, 1);
			}
		}
		else if(command == PROG_STORE_VALIDATE_IMGHDR)
		{/*
			#if (DEBUG > 0)
				printf ("**DEBUG**> Received PROG_STORE_VALIDATE_IMGHDR image = %d\n", section);
			#endif

			 Is image header valid?
			bytesWrittenOrRead = IsMonolithicImageHeaderValid((const void *) buffer, size, section);
			*/

		}
		else
		{
			#if (DEBUG > 0)
					printf ("**DEBUG**> NonVolCommandHandler did not recognize command %d\n", command);
			#endif
		}
    }
  }
#ifndef OPEN_ONCE
  if( fd >= 0 )
	close (fd);
#endif
  /* 
   * Return bytes read/written.  This will tell the caller whether
   * the operation succeeded or not.
   */
  return bytesWrittenOrRead;
}
void print_usage() {
	printf ("Missing argument!\n");
	printf ("Usage: <interface>\n"
				"optional: <myipaddress>\n"
				"optional: <destipaddress>\n"
				"optional: <default_symlink_to_new_monolith>\n"
				"optional: <default_symlink_to_new_ecm_image>\n");

}


int SetupStandAloneApp(int argc, char *argv[])
{
	/* Sanity check for right number of args */
	if (argc < 2)
	{
		print_usage();
		return -1;
	}

	/*  Get all the args from command line */
	strcpy(interfaceName, argv[1]);

	if( argv[2] )
		srcIpAddress = (uint32) ParseIP ((unsigned char*)argv[2]);

	if( argv[3] )
		dstIpAddress = ParseIP ((unsigned char*)argv[3]);

	if( (argc>4) && (argv[4] != NULL))
		monolithic_image = argv[4];

	if( (argc>5) && (argv[5] != NULL))
	{
	  if( strlen(argv[5]) > 32 )
	  {		  
		  printf("File name %s is too long\n", argv[5]);
		  return -1;		  
	  }

	  image1 = argv[5];
	}

	if( (argc>6) && (argv[6] != NULL))
	{
	  if( strlen(argv[6]) > 32 )
	  {		  
		  printf("File name %s is too long\n", argv[6]);
		  return -1; 
	  }

	  image1_temp = argv[6];	  
	}

	return 0;
}

#if defined (STANDALONE_APP)
#warning "BUILDING STANDALONE_APP"
int main (int argc, char *argv[])
#else
static int gRemoteProgramStoreHost_initialized = 0;
int InitializeRemoteProgramStoreHost(char* pInterfaceName, uint32 localRxIpAddress, uint32 remoteEcmIpAddress)
#endif
{
	int cmdSocket;
	struct sockaddr_in sinStruct;
	struct sockaddr *fromSockAddr;

	int sockLen;
	char *buffer;
	int fd = -1;
	unsigned int optVal = SEGMENT_SIZE + HDR_SIZE;
  
#if defined (STANDALONE_APP)
	if( SetupStandAloneApp(argc, argv) <0 )
	{
		exit(1);
	}
#else
    if (gRemoteProgramStoreHost_initialized) 
	{
        printf("RemoteProgramStoreHost already initialized\n");
        return 0;
    };

	strncpy(interfaceName, pInterfaceName, strlen(pInterfaceName));
	srcIpAddress = localRxIpAddress;
	dstIpAddress = remoteEcmIpAddress;
#endif	
	printf("%s\n", __FUNCTION__);
	if(srcIpAddress == 0)
		srcIpAddress = (uint32) ParseIP ((unsigned char*)srcIpAddressStr);

	if(dstIpAddress == 0)
		dstIpAddress = (uint32) ParseIP ((unsigned char*)dstIpAddressStr);

	if(!monolithic_image)
	{
		monolithic_image = kDefaultSymLinkToNewMonolith;
	}

	/* This will be needed to upgrade software for an eCM in DOCSIS only mode. */
	if(!image1)
	{
		image1 = kDefaultSymLinkToOperationalEcmImage;
	}

	if(!image1_temp)
	{
		image1_temp = kDefaultSymLinkToNewEcmImage;
	}

	/*
	* First off, let's check if the symbolic link and storage path exist.  If this is the first time we're running,
	* chances are the files don't exist.  We'll print a diagnostic message for the user and then
	* create the symbolic link and storage path.
	*/
	if ((monolithic_image) && ((fd = open (monolithic_image, O_RDWR)) <0))
	{
        if (kDefaultNewMonolithStoragePath) {
		    printf ("Could not find %s.  Setting up symbolic link to %s...\n", monolithic_image, 
					kDefaultNewMonolithStoragePath? kDefaultNewMonolithStoragePath: "NULL");

		    if ((fd = open (kDefaultNewMonolithStoragePath, O_RDWR)) <0)
		    {
			    printf ("Could not find %s.  Creating...\n", kDefaultNewMonolithStoragePath?kDefaultNewMonolithStoragePath: "NULL");
			    fd = open (kDefaultNewMonolithStoragePath, O_CREAT | O_WRONLY | O_TRUNC);
			    if (fd < 0)
			    {
			      printf ("rProgramStore Host:  error: Could not create temporary program storage:  %s!\n", 
				  				kDefaultNewMonolithStoragePath? kDefaultNewMonolithStoragePath: "NULL");
			      perror ("rProgramStore Host");
			      exit (1);
			    }
		    }
    		
		    /* Setting up symbolic link*/
            symlink(kDefaultNewMonolithStoragePath, monolithic_image);
        }
	}
    if (fd >= 0) {
	    close (fd);
        fd = -1;
    }

	/* Setup eCM storage. This will only be used in DOCSIS-only mode. */
	if ((image1) && ((fd = open (image1, O_RDWR)) <0))
	{
		printf ("Could not find %s.  Setting up symbolic link to operational eCM Image Storage Path %s...\n", image1, 
						kDefaultOperationalEcmImageStoragePath? kDefaultOperationalEcmImageStoragePath: "NULL");

		if ((kDefaultOperationalEcmImageStoragePath) && ((fd = open (kDefaultOperationalEcmImageStoragePath, O_RDWR)) <0))
		{
			printf ("Could not find %s.  Creating...\n", kDefaultOperationalEcmImageStoragePath);
			fd = open (kDefaultOperationalEcmImageStoragePath, O_CREAT | O_WRONLY | O_TRUNC);
			if (fd < 0)
			{
			  printf ("rProgramStore Host:  error: Could not create operational eCM Image storage:  %s!\n", 
			  			kDefaultOperationalEcmImageStoragePath? kDefaultOperationalEcmImageStoragePath: "NULL" );
			  perror ("rProgramStore Host");
			  exit (1);
			}
		}
		
		/* Setting up symbolic link*/
		symlink(kDefaultOperationalEcmImageStoragePath, image1);
	}
    if (fd >= 0)
	    close (fd);

	/* Setup temp eCM storage. */
	fd = OpenOrCreateTemporaryEcmProgramStorage();
	if( fd >= 0 )
	{
		close (fd);
	}

	#if defined (STANDALONE_APP)
	/* 
	* Check if our eth interface is up.  This function will wait until the interace
	* comes up if we pass it 0 as a parameter.  If the parameter is non-zero, it will
	* wait for that many seconds before bailing.
	*/
	if (ethInterfaceUp (0))
	{
		printf ("rProgramStore Host:  error: Eth interface never came up...exiting\n");
		return -1;
	}

	/* Ethernet interface is up; configure it */
	if (configureEthInterface ())
	{
		/* 
		 * Something bad happened and we couldn't setup
		 * the interface...print diag message and bail
		 */
		printf ("rProgramStore Host:  error: could not configure eth interace %s\n", interfaceName);
		return -1;
	}
	ethDeviceNeedsSetup = 0;
	#endif

	/*
	* Everything's good at this point, so lets open up the sockets on which we'll
	* listen for non-vol requests. 
	*/

	/* Create the sockets for receiving commands and sending responses */
	rspSocket = socket (AF_INET, SOCK_DGRAM, 0);
	cmdSocket = socket (AF_INET, SOCK_DGRAM, 0);

	bzero ((char *) &sinStruct, sizeof (sinStruct));
	sinStruct.sin_family = AF_INET;
	sinStruct.sin_addr.s_addr = htonl (srcIpAddress);

	/* Setup cmd socket and bind it to the designated port */
	sinStruct.sin_port = htons (ESTB_PROGSTORE_WRITE_PORT);
	if (bind (cmdSocket, (struct sockaddr *) &sinStruct, sizeof (sinStruct)) < 0)
	{
		/* Handle error */
		printf ("rProgramStore Host:  Couldn't bind command socket 0x%x!\n", srcIpAddress);
		perror ("rProgramStore Host");
		return -1;
	}

	/* BIND the response socket to the local interface */
	bzero ((char *) &sockAddrIn, sizeof (sockAddrIn));
	sockAddrIn.sin_family = AF_INET;
	sockAddrIn.sin_port = htons (0);
	sockAddrIn.sin_addr.s_addr = htonl (srcIpAddress);

	if (bind (rspSocket, (struct sockaddr *) &sockAddrIn, sizeof (sockAddrIn)) < 0)
	{
		/* Handle error */
		printf ("rProgramStore Host:  Couldn't bind response socket 0x%x!\n", srcIpAddress);
		perror ("rProgramStore Host");
		return -1;
	}

	/* Set up the socket response struct */
	bzero ((char *) &sockAddrIn, sizeof (sockAddrIn));
	sockAddrIn.sin_family = AF_INET;
	sockAddrIn.sin_port = htons (ESTB_SWDLOAD_REPLY_PORT);
	sockAddrIn.sin_addr.s_addr = htonl (dstIpAddress);

	/* Allocate space for receiving commands */
	buffer = (char *) malloc (SEGMENT_SIZE + HDR_SIZE);
	if( buffer == NULL )
	{
		printf ("Failed to allocate memory for socket receive buffer\n");  
		return -1;
	}

	
	if (setsockopt(cmdSocket, SOL_SOCKET, SO_RCVBUF, (char *) &optVal, sizeof(optVal)) != 0)
	{
		printf ("setsockopt SO_RCVBUF error:  %s \n", strerror(errno));  
	}

#if !defined (STANDALONE_APP)
    gRemoteProgramStoreHost_initialized = 1;
#endif	

	/* Enter main loop where we'll wait on the cmd port and respond to requests */
	while (1)
	{
		int numBytesRead;
		int retVal = 0;
		fd_set rfds;
		struct timeval tv;

		#if defined (STANDALONE_APP)
		/*
		 * Check if the ethernet device we're using is still up.  It could
		 * have gone away if the other side reset.  
		 */
		if (ethInterfaceUp (0))
		{
			/* Something else is wrong, so we should probably bail */
			perror ("rProgramStore Host");
			printf (" Bailing...\n");
#if !defined (STANDALONE_APP)
            gRemoteProgramStoreHost_initialized = 0;
#endif	
			return -1;
		}
		else if (ethDeviceNeedsSetup)
		{
			if (configureEthInterface ())
			{
				printf ("rProgramStore Host:  Could not configure ethernet interface %s\n", interfaceName);
				perror ("rProgramStore Host");
			}

			ethDeviceNeedsSetup = 0;
		}
		#endif

		numBytesRead = 0;

		/* Clear out the receive buffer */
		memset ((void *) buffer, 0, (SEGMENT_SIZE + HDR_SIZE));

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
		FD_ZERO (&rfds);
		FD_SET (cmdSocket, &rfds);
		retVal = select (cmdSocket + 1, &rfds, (fd_set *) 0, (fd_set *) 0, &tv);
		if (retVal == -1)
		{
		  /* We errored.  Print error message only for now */
		  perror ("rProgramStore Host");
		}
		else if (retVal == 0)
		{
			//printf("rProgramStore Host: No data received!\n");
		}
		else
		{
			sockLen = sizeof (fromSockAddr);
         
			/* Receive data from the cmd socket.  This will block until data is received */
			numBytesRead = recvfrom (cmdSocket, buffer, (SEGMENT_SIZE + HDR_SIZE), 0, (struct sockaddr *) &fromSockAddr, (unsigned int*) &sockLen);

			/* Check if socket receive was successul */
			if (numBytesRead <= 0)
			{
				printf ("rProgramStore Host:  Socket Receive Error\n");
				perror ("error");
			}
			else
			{
				int bytesDone = 0;
				int count;

				#if (DEBUG > 0)
				printf ("**DEBUG**> Received %d bytes on nvol cmd socket\n", numBytesRead);
				#endif

				/* We received data.  Let's parse it and try to respond to it */
				bytesDone = ProcessEcmMessages( buffer ); 

				if( bytesDone > 0 )
				{
					#if (DEBUG > 0)
						printf ("**DEBUG**> Sending response of %d bytes\n", bytesDone);
					#endif

					/* Send response back to caller */
					count = sendto (rspSocket, (const void *) buffer, bytesDone, 0, (struct sockaddr *) &sockAddrIn, sizeof (sockAddrIn));

					if (count < 0)
					{
						printf ("rProgramStore Host:  Could not send message via response socket: %d!\n", count);
						perror ("error");
					}
				}
			}
		}
	}
#if !defined (STANDALONE_APP)
            gRemoteProgramStoreHost_initialized = 0;
#endif	

}

int ProcessEcmMessages(char* buffer)
{
	unsigned int *bufPtr;
	int cmd, segment, section, length, cmdSegment, secLength;
	int bytesDone = 0;
	char *cmdData = NULL; 
    static int write_error = 0;
    /*
     * Initialize the temporary pointer that we'll use
     * to increment and extract command from buffer
     */
    bufPtr = (unsigned int *) buffer;

    /* The first word contains the command and segment to read/write */
    cmdSegment = ntohl (*bufPtr);
    bufPtr++;

    /* The second word contains the nonvol section and length of data */
    secLength = ntohl (*bufPtr);
    bufPtr++;

    /* Parse out all the parameters */
    cmd = cmdSegment & 0xf0000000;
    segment = cmdSegment & 0x0Fffffff;
    section = (secLength & 0xff000000) >> 24;
    length = secLength & 0x00ffffff;

	#if (DEBUG > 0)
        printf ("**DEBUG**>  Cmd: %02x  Segment: %02x\n", cmd, segment);
        printf ("            Sec: %02x  Length: %02x\n", section, length);
	#endif

    /* 
     * Since the same function handles both read & write, we'll have to
     * allocate buffer space for the read before we call the function.
     * Check the command we received and do it now.
     */
	switch(cmd)
	{
		case PROG_STORE_WRITE:
			{
				if( SwUpgradeIsStarting( cmd, segment) )
				{
					ResetBufferSettings();
                    write_error = 0;
				}

				cmdData = BufferSegments((char*) bufPtr, length);


				if( writeToStorage )
				{
					if( segment )
					{
						/* Adjust the buffer offset only if the length of the buffered data
						 * is equal or bigger than SEGMENT_SIZE and if the buffer can hold more than one 
						 * SEGMENT_SIZE segment.
						 */
						if( (bufferSize > SEGMENT_SIZE) && (bytesUsed >= SEGMENT_SIZE) )
						{	
							segment -= bufferOffsetAdjustment; /* Trick: segment is actually the offset and offset starts from 0)*/
						}						
					}

					/* Hand the data to program store device */
					bytesDone = NonVolCommandHandler (cmd, segment, section, /*length*/ bytesUsed, (void *) cmdData);
					if (bytesDone != bytesUsed) /*length */
					{
						printf ("rProgramStore Host:  WRITE operation return size did not match specified size\n");
						printf ("              specified size: %d  command returned: %d\n", bytesUsed, bytesDone);
                        write_error = 1;
					}

					ResetBufferSettings();
				}
                if (write_error) {
                        /* Send eCM response to indicate write error */
                        secLength = htonl (((section << 24) | bytesDone));
                        memcpy ((void *) &buffer[4], (const void *) &secLength, sizeof (secLength));
						printf ("              send eCM response to indicate write error\n");

                        if( sendto (rspSocket, (const void *) buffer, HDR_SIZE, 0, (struct sockaddr *) &sockAddrIn, sizeof (sockAddrIn)) < 0 )
                        {
                            printf ("rProgramStore Host:  Could not send message via response socket\n");
                            perror ("error");
                        }
                }
                else  {
                    /* Send response back to caller */
                    if( sendto (rspSocket, (const void *) buffer, HDR_SIZE, 0, (struct sockaddr *) &sockAddrIn, sizeof (sockAddrIn)) < 0 )
                    {
                        printf ("rProgramStore Host:  Could not send message via response socket\n");
                        perror ("error");
                    }
                }
				/* Now return zero so that calling routine does send the response to eCM twice.*/
				return 0;
			}
			break;

		case PROG_STORE_VALIDATE_IMGHDR:
			{
				printf ("Sw upgrade available. \n");

				#if (DEBUG > 0)
				printf ("**DEBUG**> Received PROG_STORE_VALIDATE_IMGHDR image = %d\n", section);
				#endif

				cmdData = (char *) bufPtr;

				/*
				 * eCM requests image header validation which means an upgrade is available.
				 * Ask customer application whether it wants to continue with the upgrade.
				 * At this point eCM already downloaded first chunk of the image and sent it
				 * to eSTB for header validation.
				 * If eSTB does not want to upgrade now then we will return o and eCM will abort the upgrade.
				 */
				if(EstbSwUpgrade_IsUpgradePermittedNow())
				{
					printf ("Sw upgrade permitted. \n");
					/* fixme!, how to differentiate SNMP, ecm and cvt triggered tftp download ? */
					EstbSwUpgrade_InProgress(kTftpDownloadTriggerEcmCfgFile);

					/* Is image header valid? */
					bytesDone = EstbSwUpgrade_IsMonolithicImageHeaderValid();
				}
				else
				{
					bytesDone = 0;
				}
			}
			break;

		case PROG_STORE_COMPUTE_CRC:
		case PROG_STORE_COMPUTE_CRC_AND_LOCKIN:
			{
				#if (DEBUG > 0)
				printf ("**DEBUG**>  Cmd: %02x  Segment: %02x\n", cmd, segment);
				printf ("            Sec: %02x  Length: %02x\n", section, length);
				#endif

				/* Ask program store device to compute image crc. */
				bytesDone = NonVolCommandHandler (cmd, segment, section, length, (void *) cmdData);

				/* Send response back to caller */
				/*
				 * bytesDone is the return value of NonVolCommandHandler and in this case
				 * it is either 0 or 1.(0:CRC invalid   1:CRC valid)
				 */
				buffer[7] = bytesDone;
				if( sendto (rspSocket, (const void *) buffer, HDR_SIZE, 0, (struct sockaddr *) &sockAddrIn, sizeof (sockAddrIn)) < 0 )
				{
					printf ("rProgramStore Host:  Could not send message via response socket\n");
					perror ("error");
				}

				if( (bytesDone) && (cmd == PROG_STORE_COMPUTE_CRC_AND_LOCKIN))
				{
					/* Update program storage. */
					UpdateProgramStoreRegion( section );
				}

				/* Now return zero so that calling routine does send the response to eCM twice.*/
				return 0;
			}
			break;

		case DLOAD_SUCCESSFUL:
			{
				if(section & kMonolithic)
				{
					printf ("**DEBUG**>  MONOLITHIC IMG DLOAD SUCCESSFUL\n");
					EstbSwUpgrade_Notification(kMonolithicDownloadOk);
				}
				else
				{
					printf ("**DEBUG**>  ECM IMG DLOAD SUCCESSFUL\n");
					EstbSwUpgrade_Notification(kEcmDownloadOk);
				}
			}
			break;

		case DLOAD_FAILED:
			{				
				int fd = -1;

				if(section & kMonolithic)
				{
					printf ("**DEBUG**>  MONOLITHIC IMG DLOAD FAILED\n");
					EstbSwUpgrade_Notification(kMonolithicDownloadFail);
				}
				else
				{
					printf ("**DEBUG**>  ECM IMG DLOAD FAILED\n");
					EstbSwUpgrade_Notification(kEcmDownloadFail);

					/* clear temporary storage.*/
					ClearTemporaryEcmProgramStorage();

					/* Prepare for the next upgrade. */
					fd = OpenOrCreateTemporaryEcmProgramStorage();
					if( fd >= 0)
					{
						close(fd);
					}					
				}
			}
			break;

		default:
			{
				printf ("rProgramStore Host:  Did not understand command!\n");
			}
			break;
	}

    /*
     * Let's send the response to the requester.  We'll simply use the command buffer and 
     * only modify the length field.  The requestor can verify if the operation was successul 
     * by comparing the size specified in the command to the actual size we operated on, 
     * returned in this message.
     */
    secLength = htonl (((section << 24) | bytesDone));
    memcpy ((void *) &buffer[4], (const void *) &secLength, sizeof (secLength));

    /* For non-vol write commands, we'll just send back the first 8 bytes */
    if (cmd == PROG_STORE_WRITE) 
    {
      bytesDone = 8;
    }

	if((cmd == PROG_STORE_COMPUTE_CRC) || (cmd == PROG_STORE_COMPUTE_CRC_AND_LOCKIN))
	{
		#if (DEBUG > 0)
			printf ("**DEBUG**> Responding to PROG_STORE_COMPUTE_CRC/LOCKIN \n");
		#endif

			/*
			 * bytesDone is the return value of NonVolCommandHandler and in this case
			 * it is either 0 or 1.(0:CRC invalid   1:CRC valid)
			 */
		buffer[7] = bytesDone;			
			  
		bytesDone = 8;
	}

	if(cmd == PROG_STORE_VALIDATE_IMGHDR)
	{
		#if (DEBUG > 0)
			printf ("**DEBUG**> Rsponding to PROG_STORE_VALIDATE_IMGHDR \n");
		#endif

			/*
			 * bytesDone is the return value of NonVolCommandHandler and in this case
			 * it is either 0 or 1.(0:Image Header invalid   1:Image Header valid).
			 * eCM will read buffer[7].
			 */
		buffer[7] = bytesDone;			
		
		bytesDone = 8;
	}

	return bytesDone;
}

/*
 * This is where the derived class reads data from the device into the
 * buffer.
 *
 * Parameters:
 *      inputBuffer - the buffer into which the data should be read.
 *      filename    - the filename that should be read.
 *
 * Returns:
 *      true if successful.
 *      false if there was a problem (usually hardware failure).
 */
bool ReadFile(char *inputBuffer, uint32 fileSize,
              int fd, uint32 bufferOffset)
{
    bool result = true;
    size_t bytesRead;

    /* go to the offset in the file */
    if (lseek (fd, bufferOffset, SEEK_SET) < 0)
    {
      /* Couldn't seek to offset, maybe a bad offset?? */
      perror ("rProgramStoreHostDriver");
      return false;
    }	

    /* Read the buffer from the file. */
    bytesRead = read(fd, inputBuffer, fileSize);
    if(bytesRead != fileSize)
    {
	    printf("ReadFile - bytesRead != fileSize!\n");  
	    result = false;	
    }

    return result;
}

void SetMonolithicImageTempFileName(char * fn) 
{
	int fd = -1;

   /* Let's check if the symbolic link exists. If this is the first time we're running,
	* chances are the files don't exist. We'll print a diagnostic message for the user and
	* create the symbolic link. 
	*/
	if (monolithic_image == NULL) 
	{
		monolithic_image = kDefaultSymLinkToNewMonolith;
		if ((monolithic_image) && ((fd = open (monolithic_image, O_RDWR)) <0))
		{
			printf ("rProgramStore Host:SetMonolithicImageTempFileName: Could not find %s. \n", monolithic_image);
		}
		else 
		{
			close(fd);
			printf ("rProgramStore Host:SetMonolithicImageTempFileName: remove %s\n", monolithic_image);

			remove(monolithic_image);
		}
	}
	else
	{
		printf ("*** SetMonolithicImageTempFileName - Removing symbolic link %s \n", monolithic_image);
		remove(monolithic_image);
	}

	/* Setting up symbolic link*/
	printf ("*** SetMonolithicImageTempFileName - Setting up symbolic link: %s -> %s\n", monolithic_image, fn);
	symlink(fn, monolithic_image);
}

char * GetMonolithicImageTempFileName(void ) 
{
	return monolithic_image;
}

char * ComputeTempProgramStoreRegion( int section )
{
	if( section & kMonolithic )
	{
		return monolithic_image;
	}
	else
	{
		return image1_temp;
	}
}

bool UpdateProgramStoreRegion( int section )
{
	int fd = -1;

	if(section & kMonolithic)
	{
		/* Call customer API. */
		return true;
	}
	else
	{
        if (kDefaultNewEcmImageStoragePath == NULL) {
            return false;
        }
		/*if( docsis_only_mode )*/
		{
			UpgradeEcmImage();
			
			/* clear temporary storage.*/
			ClearTemporaryEcmProgramStorage();

			/* Prepare for the next upgrade. */
			fd = OpenOrCreateTemporaryEcmProgramStorage();
			if( fd < 0)
			{
				return false;
			}
			close(fd);
		}

		return true;
	}
}

bool SwUpgradeIsStarting( int command, int segment )
{
	/* Decide whether this is the beginning of an upgrade. */
	if( (command == PROG_STORE_WRITE) && ( segment == 0 ) )
	{
		return true;
	}

	return false;
}

static int CalcCrc(const void *in2_str, int Length, unsigned long *crc_word, int reverse) 
{
    int                 hex_index       = 0;
    int                 old_hex_index   = 0; 
    int                 stop            = 0; 
    unsigned long       value           = 0;  
    int                 digits          = 0; 
    int                 shift_count     = 0; 
    int                 max_shift_count = 0;  
    unsigned long       rvalue          = 0;  
    unsigned long       data_word       = 0;
    unsigned long       polynomial_value= 0x00000000;  
    unsigned long       xor_mask        = 0x00000000; 
    const unsigned char *DataPtr;
    int                 Counter;
    int                 i;

    DataPtr = (const unsigned char *) in2_str;
    *crc_word=0xffffffff; 

    digits=4; 
    polynomial_value=0x00001021; 

    max_shift_count = digits*4; 

    Counter = 0;
    /* Read in hex digits : */ 
    while ((Counter < Length) || !stop)
    {
        if (Counter < Length)
            value = *DataPtr;
        else
        {
            stop            = 1; 
            value           = 0; 
            data_word       = data_word << ((digits-(hex_index%digits))*4-8); 
            max_shift_count = (hex_index % digits) * 4;  
            old_hex_index   = hex_index; 
            hex_index       = digits - 2; 
        }

        if (reverse)
        {
            rvalue = 0;
            for ( i=0; i<8; i++ )
            {
                if (value & 0x01)
                    rvalue = rvalue | 0x01;
                value = value >> 1;
                rvalue = rvalue << 1;
            }
            rvalue = rvalue >> 1;
            value = rvalue;
        }

        data_word  = data_word<<8; 
        data_word += value; 

        if ( hex_index%digits == (digits-2) )
        {
            //if (polynomial_name == CRC_CCITT)
            //   Print("%04x ", (data_word & 0xffff));
            //else
            //   Print("%08lx ", data_word);

            /* Last hex digit has been reached - process CRC : */   

            for ( shift_count = 0; shift_count < max_shift_count; shift_count++ )
            {
                xor_mask = ((data_word&(1L<<(digits*4-1)))^
                    (*crc_word&(1L<<(digits*4-1))))? polynomial_value: 0L; 

                *crc_word = *crc_word << 1; 
                *crc_word = *crc_word ^ xor_mask; 

                data_word = data_word<<1; 
            } 
        }

        if (!stop)
        {
            hex_index += 2;
            DataPtr++;
            Counter++;
        }
        else
            hex_index = old_hex_index;  
    } 
    *crc_word = ~(*crc_word); 
    value = *crc_word;
    if (reverse)
    {
        rvalue = 0;
        for (i=0; i<32; i++)
        {
            if (value & 0x01)
                rvalue = rvalue | 0x01;
            value = value >> 1;
            if (i != 31)
                rvalue = rvalue << 1;
        }
        *crc_word = (unsigned long) ((rvalue & 0xFF000000)) >> 24 |
            (unsigned long) ((rvalue & 0x00FF0000) >> 8) |
            (unsigned long) ((rvalue & 0x0000FF00) << 8) |
            (unsigned long) ((rvalue & 0x000000FF) << 24);
    }
    return 0;
}

/*****************************************************************************/
/* The following functions are used when validating and upgrading CM images. */
/* BcmProgramHeader structure is assumed to be used for CM image headers.    */
/*****************************************************************************/
/*---------------------------------------------------------------------------
    Name: IsImageValid
 Purpose: Computes CRC over the image and checks it against the CRC value
		  stored in the image header.
		  Note that program header structure is BcmProgramHeader.

  Inputs:
        fileSize - psize of the file that contains the image.
		pFile - pointer to the file that contains the image.
 Returns:
        0 - image cannot be validated for some reason
		1 - image CRC is valid
   Notes:
---------------------------------------------------------------------------*/
unsigned int IsImageValid(int fd, uint32 bufferOffset, uint32 print )
{
    bool result = 0;
    char *testBuffer;
    unsigned int crcResult;
	BcmProgramHeader *pProgramStoreHeader;
	unsigned int bytesDone = 0;
	unsigned long imageRemaining = 0;
	unsigned long crcLength      = 0;
	unsigned long oldCrc         = 0;
    unsigned long ulCrc;

	/* Allocate buffer to read in the file.*/
    testBuffer = malloc(64*1024); 
	if( !testBuffer )
	{
		return 0;
	}

	/* Read the program header. */
	result = ReadFile(testBuffer, (64*1024), fd, bufferOffset);

	/* If the read failed, no sense even trying to back it up. */
    if (result)
    {
		bytesDone += (64*1024);

		/* Overlay the header structure. */
		pProgramStoreHeader = (BcmProgramHeader *) testBuffer;

		/* Display program header. */
        if (print)
		    ProgramStoreDriverPrintProgramHeader(pProgramStoreHeader);

        /* validate program header */
        CalcCrc(pProgramStoreHeader, sizeof(BcmProgramHeader) - 8, &ulCrc, 0);
        ulCrc = ntohs(ulCrc & 0x0000ffff);
        if (ulCrc != pProgramStoreHeader->usHcs)
        {
            free(testBuffer);
            return 0;
        }
				
		imageRemaining = ntohl(pProgramStoreHeader->ulfilelength);
		crcLength      = min( imageRemaining, 64*1024-sizeof(BcmProgramHeader) );
		oldCrc         = ntohl(pProgramStoreHeader->ulcrc);

		/*  Start a CRC on the image data.  Assume we can only read 64k at a time. */
		crcResult = FastCrc32First( testBuffer + sizeof(BcmProgramHeader),
									crcLength );
		/* Now continue the CRC on each 64k chunk until we are done. */
		while ( 1 )
		{
			imageRemaining -= crcLength;
			crcLength       = min( imageRemaining, 64*1024 );
			if ( crcLength == 0 )
			{
				break;
			}
			if (read(fd, testBuffer, crcLength) != crcLength)
			{
				/* Read of the next chunk failed!  Return error */
				printf("Error - Failed to read\n");
			}

			bytesDone += crcLength;
			crcResult = FastCrc32More( crcResult, testBuffer, crcLength );
		}
		
		/* Finalize the CRC. */
		crcResult = FastCrc32Finish( crcResult );

		if ( crcResult == oldCrc )
		{
			printf("Image CRC validated!\n");
			result = 1;				
		}
		else
		{
			printf("Error - Image CRC could not be validated!\n");
			printf("Computed CRC = %8x  does not match CRC value in the program header = %8x \n", crcResult, ntohl(pProgramStoreHeader->ulcrc) );

			result = 0;
		}         
    }

    /* No longer need the buffer. */
    free( testBuffer );
	return result;
}

/*---------------------------------------------------------------------------
    Name: ProgramStoreDriverPrintProgramHeader
 Purpose: Display the image header contents
  Inputs:
        BcmProgramHeader - pointer to a program header in memory
 Returns:
        Nothing
   Notes:
---------------------------------------------------------------------------*/
void ProgramStoreDriverPrintProgramHeader( BcmProgramHeader * pProgramHeader )
{
	printf("\n************************************\n"
            "ProgramStoreDriverPrintProgramHeader\n"
            "************************************\n"
            "      Signature: %x"      
            "\n      Control: %x"     
            "\n    Major Rev: %x"    
            "\n    Minor Rev: %x"    

	        "\n  File Length: %d"     
	        "\n Load Address: %08x"     
	        "\n     Filename: %s"    
	        "\n          HCS: %x"    
	        "\n          CRC: %x\n"
	        "\n************************************\n"
							, ntohl(pProgramHeader->usSignature)
							, ntohl(pProgramHeader->usControl)
							, ntohl(pProgramHeader->usMajorRevision)
							, ntohl(pProgramHeader->usMinorRevision)
							, ntohl(pProgramHeader->ulfilelength)
							, ntohl(pProgramHeader->ulProgramLoadAddress)
							, pProgramHeader->cFilename
							, ntohl(pProgramHeader->usHcs)
							, ntohl(pProgramHeader->ulcrc));

}

/*---------------------------------------------------------------------------
    Name: ClearTemporaryEcmProgramStorage
 Purpose: Erase the temporary storage that was used to download and validate the new eCM image.
		 
  Inputs:
        
 Returns:
        Nothing
   Notes:
---------------------------------------------------------------------------*/
void ClearTemporaryEcmProgramStorage(void)
{			
	if( image1_temp )
	{	
		/* Remove symbolic link	*/
		remove(image1_temp);
	}

	if( kDefaultNewEcmImageStoragePath )
	{
		/* clear temporary storage.*/
		/* ATTENTION: Do not use "remove" if kDefaultNewEcmImageStoragePath is a flash device. */
		printf ("WARNING: ClearTemporaryEcmProgramStorage -  Removing %s...\n", 
				kDefaultNewEcmImageStoragePath? kDefaultNewEcmImageStoragePath: "NULL" );
		remove(kDefaultNewEcmImageStoragePath);
	}
}

int OpenOrCreateTemporaryEcmProgramStorage(void)
{
	int fd = -1;

	if ((image1_temp) && ((fd = open (image1_temp, O_RDWR)) <0))
	{
        if (kDefaultNewEcmImageStoragePath) {
            printf ("Could not find %s.  Setting up symbolic link to new eCM Image Storage Path %s...\n", image1_temp, 
					kDefaultNewEcmImageStoragePath? kDefaultNewEcmImageStoragePath: "NULL" );

            if ((fd = open (kDefaultNewEcmImageStoragePath, O_RDWR)) <0)
            {
                printf ("Could not find %s.  Creating...\n", kDefaultNewEcmImageStoragePath);
                fd = open (kDefaultNewEcmImageStoragePath, O_CREAT | O_WRONLY | O_TRUNC);
                if (fd < 0)
                {
                    printf ("rProgramStore Host:  error: Could not create temporary program storage:  %s!\n", 
							kDefaultNewEcmImageStoragePath? kDefaultNewEcmImageStoragePath: "NULL");
                    perror ("rProgramStore Host");
                    return fd;
                }
                /* Setting up symbolic link */
                symlink(kDefaultNewEcmImageStoragePath, image1_temp);
            }
        }
	}

	return fd;
}

unsigned int IsImageValid(int fd, uint32 bufferOffset, uint32 print );

#define FILEBUFFERSIZE (128 * 1024)

#ifdef kDefaultOperationalEcmImageStoragePath2
// Dual image case
void UpgradeEcmImage(void)
{
    int activeFileDesc, inactiveFileDesc, readFileDesc, part1Valid;
    int readSize, writesize, err, retrycount = 0;
    char *fileReadBuff;

    /* delete the symbolic link to operational image*/
	printf("Removing the symbolic link (%s) to operational image\n", image1? image1: "NULL");
    remove(image1);

    /* determine active partition */
    if ((readFileDesc = open(kDefaultOperationalEcmImageStoragePath2, O_RDONLY)) < 0)
    {        
		printf("Couldn't open ECM storage %s\n", kDefaultOperationalEcmImageStoragePath2? kDefaultOperationalEcmImageStoragePath2: "NULL");
        return;
    }
    part1Valid = IsImageValid(readFileDesc, 0, 0);
    close(readFileDesc);

    if (part1Valid) {
        printf("Active = docsis1 (%s), inactive = docsis0 (%s)\n", kDefaultOperationalEcmImageStoragePath2, kDefaultOperationalEcmImageStoragePath);
        if ((activeFileDesc = open(kDefaultOperationalEcmImageStoragePath2, O_WRONLY)) < 0)
        {
			printf("Couldn't open ECM storage %s\n", kDefaultOperationalEcmImageStoragePath2? kDefaultOperationalEcmImageStoragePath2: "NULL");
            return;
        }
        if ((inactiveFileDesc = open(kDefaultOperationalEcmImageStoragePath, O_WRONLY)) < 0)
        {
			printf("Couldn't open ECM storage %s\n", kDefaultOperationalEcmImageStoragePath? kDefaultOperationalEcmImageStoragePath: "NULL");
            close(activeFileDesc);
            return;
        }
    }
    else {
        printf("Active = docsis0 (%s), inactive = docsis1 (%s)\n", kDefaultOperationalEcmImageStoragePath, kDefaultOperationalEcmImageStoragePath2);
        if ((inactiveFileDesc = open(kDefaultOperationalEcmImageStoragePath2, O_WRONLY)) < 0)
        {
			printf("Couldn't open ECM storage %s\n", kDefaultOperationalEcmImageStoragePath2? kDefaultOperationalEcmImageStoragePath2: "NULL");
            return;
        }
        activeFileDesc = -1;
    }


    /* Read from temp file in 64K chunks and write out to eCM flash partition */
    fileReadBuff = malloc (FILEBUFFERSIZE);
    if (!fileReadBuff) {
        if (activeFileDesc >= 0)
            close(activeFileDesc);
        close(inactiveFileDesc);
        return;
    }

    printf("Finalizing image...\n");
    if ((readFileDesc = open(kDefaultNewEcmImageStoragePath, O_RDWR)) < 0) {
        printf("Could not open %s\n", kDefaultNewEcmImageStoragePath? kDefaultNewEcmImageStoragePath: "NULL");
        if (activeFileDesc >= 0)
            close(activeFileDesc);
        close(inactiveFileDesc);
        free(fileReadBuff);
        return;
    }

    printf("Writing image...\n");
    retrycount = 0;
retry:
    err = 0;
    readSize = read(readFileDesc, fileReadBuff, FILEBUFFERSIZE);
    while (readSize)
    { 
        writesize = write(inactiveFileDesc, fileReadBuff, readSize);
        if ((writesize == -EIO) && (retrycount == 0)) {
            retrycount++;
            printf("Retrying write due to new bad block\n");
            lseek(inactiveFileDesc, 0, SEEK_SET);
            lseek(readFileDesc, 0, SEEK_SET);
            goto retry;
        }
        if (readSize != writesize) {
            err = 1;
            break;
        }
        readSize = read(readFileDesc, fileReadBuff, FILEBUFFERSIZE);
    }
    close(readFileDesc);
    if (!err)
        ioctl(inactiveFileDesc, SIOCDEVPRIVATE);
    close(inactiveFileDesc);
    if ((activeFileDesc >= 0) && (!err)) {
        printf("Invalidating active image...\n");
        memset(fileReadBuff, 0, sizeof(BcmProgramHeader));
        write(activeFileDesc, fileReadBuff, sizeof(BcmProgramHeader));
        close(activeFileDesc);
    }
    free(fileReadBuff);
    printf("done!\n");

    /* recreate the symbolic link.*/ 
    symlink(kDefaultOperationalEcmImageStoragePath, image1);
}
#else
// Single image case
void UpgradeEcmImage(void)
{
	int writeFileDesc, readFileDesc;
	int readSize, writesize, retrycount = 0;
	char *fileReadBuff;
	if( !image1 || !image1_temp)
		return;

	/* delete the symbolic link to operational image*/
	printf("Removing the symbolic link (%s) to operational image\n", image1? image1: "NULL");
	remove(image1);
	
	/* Read from temp file in 64K chunks and write out to eCM flash partition */
	fileReadBuff = malloc (FILEBUFFERSIZE);
	if (!fileReadBuff)
		return;

	if ((writeFileDesc = open(kDefaultOperationalEcmImageStoragePath, O_WRONLY)) < 0)
	{
	    printf("Couldn't open ECM storage %s\n", kDefaultNewEcmImageStoragePath? kDefaultNewEcmImageStoragePath: "NULL");
        free(fileReadBuff);
	    return;
	}

	if (lseek(writeFileDesc, 0, SEEK_SET) != 0)
	{
	    printf("Couldn't seek to start of ECM image file\n");
	    close(writeFileDesc);
        free(fileReadBuff);
	    return;
	}

	printf("Finalizing image...\n");
    if ((readFileDesc = open(kDefaultNewEcmImageStoragePath, O_RDWR)) < 0) {
        printf("Could not open %s\n", kDefaultNewEcmImageStoragePath? kDefaultNewEcmImageStoragePath: "NULL");
        close(writeFileDesc);
        free(fileReadBuff);
        return;
    }

retry:
	readSize = read(readFileDesc, fileReadBuff, FILEBUFFERSIZE);
	while (readSize)
	{
	    writesize = write(writeFileDesc, fileReadBuff, readSize);
        if ((writesize == -EIO) && (retrycount == 0)) {
            retrycount++;
            printf("Retrying write due to new bad block\n");
            lseek(writeFileDesc, 0, SEEK_SET);
            lseek(readFileDesc, 0, SEEK_SET);
            goto retry;
        }
        readSize = read(readFileDesc, fileReadBuff, FILEBUFFERSIZE);
	}

	close(readFileDesc);
    ioctl(writeFileDesc, SIOCDEVPRIVATE);
	close(writeFileDesc);
    free(fileReadBuff);
	printf("done! Image stored to %s\n", kDefaultOperationalEcmImageStoragePath);

	/* recreate the symbolic link.*/ 
	symlink(kDefaultOperationalEcmImageStoragePath, image1);
}
#endif

/*****************************************************************************/
/* The following functions are used for program storage optimization.		 */
/*****************************************************************************/

char* BufferSegments(char *bufPtr, int length)
{
	static bool firstTime = true;

	if ( firstTime )
	{
		/* Allocate space for buffer */
		segmentBuffer = (char *) malloc (bufferSize);
		if( segmentBuffer == NULL )
		{
			printf ("Failed to allocate memory for pPreStorageBuffer\n");  
			return NULL;
		}

		firstTime = false;
	}

	memcpy(segmentBuffer + bytesUsed, bufPtr, length);	
	bytesUsed += length;

	/* Note that each block eCM requests from TFTP server MUST be a multiple of the SEGMENT_SIZE.*/
	if( (bytesUsed == (bufferSize)) || (length < SEGMENT_SIZE) )
	{
		writeToStorage = true;
	}
	
	return segmentBuffer;
}

void ResetBufferSettings(void)
{
	bytesUsed = 0;
	writeToStorage = false;
}

