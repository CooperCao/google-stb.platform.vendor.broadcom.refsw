#include	"unp.h"

#include <rpc/rpc.h>

#include "dsgcc_client_api.h"
#include "dsgccClientRegistration_rpcClient.h"
#include "dsgccClientCallback_rpcServer.h"

char* ReturnCodeTypeName( unsigned int value );
extern char* TunnelStatusTypeName( unsigned int value );
void dsgcc_ClientNotification(struct dsgccClientNotification *clientNotification);
static void DumpMessage(unsigned char *pBuffer, unsigned short bufferSize);

#if defined(BCM_DSGCC_CLIENTS_USE_SHARED_MEMORY_IPC)
#include "BcmSharedMemory.h"
#endif

#define MAXAPPNAMELEN 31
#define CLIENTPORTKEEPALIVE 50

struct dsgClientRegInfo regInfoData;

void printUsage (void) {
          printf("usage: dsgClient <port> <ID type (1-6)> [ID]\n\n");
          printf("\tdsgClient <port> 1 <Broadcast ID: 1,2,5 (SCTE65,SCTE18,XAIT)>\n");
          printf("\tdsgClient <port> 2 <mac address>\n");
          printf("\tdsgClient <port> 3 <CAS ID>\n");
          printf("\tdsgClient <port> 4 <Application ID>\n");
          printf("\tdsgClient <port> 5 <Application Name(max length %d)>\n", MAXAPPNAMELEN);
          printf("\tdsgClient <port> 6 <Vendor ID>\n\n");
          printf("Examples: \n");
          printf("\tdsgClient 2881 1 1\n");
          printf("\tdsgClient 2888 2 00:10:18:43:54:01\n");
          printf("\tdsgClient 1540 3 10\n");
          printf("\tdsgClient 2889 4 15\n");
          printf("\tdsgClient 2886 5 DEASsystem \n");
          printf("\tdsgClient 2848 6 210\n");
}

int
main(int argc, char **argv)
{
	int					sockfd;
	struct sockaddr_in	cliaddr, servaddr;
        char idString[MAXAPPNAMELEN + 1];
        struct dsgClientRegInfo *regInfo = &regInfoData;
        int retVal;
        int n;
        socklen_t       len;
        pid_t pid;
        struct timeval  tv;
        char            mesg[MAXLINE];
        #if defined(BCM_DSGCC_CLIENTS_USE_SHARED_MEMORY_IPC)
        int sharedMemoryId;
        #endif

		CLIENT *handle = NULL;

        bzero((char *) regInfo, sizeof(struct dsgClientRegInfo));

	if ((argc < 3) || (argc > 4)) { 
	  printUsage();
          exit(1);   
        }

        regInfo->clientPort = atoi(argv[1]);
        // printf("client Port is %d\n", regInfo->clientPort);
        if (regInfo->clientPort == 0) {
	  printUsage();
          exit(1);   
        }

        regInfo->idType = atoi(argv[2]);
        // printf("id type is %d\n", regInfo->idType);
        if (regInfo->idType < 1 || regInfo->idType > 6) {
	  printUsage();
          exit(1);   
        }

	#if !defined(BCM_DSGCC_CLIENTS_USE_SHARED_MEMORY_IPC)
        /* We are a DSG Client, but we are acting more like as
           server in the classic client-server model. Actually
           we are just a sink */
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);

        bzero(&servaddr, sizeof(servaddr));
        servaddr.sin_family      = AF_INET;
        servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        servaddr.sin_port        = htons(regInfo->clientPort);

        if (CLIENTPORTKEEPALIVE) {
          if (getsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, &len) == -1) {
            printf("SO_RCVTIMEO not supported, can't do keep-alive!\n");
          } else {
            bzero((char *) &tv, sizeof(struct timeval));
            tv.tv_sec = CLIENTPORTKEEPALIVE;
            setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
          }
        }

        if ((bind(sockfd, (SA *) &servaddr, sizeof(servaddr))) < 0) {
          printf("bind to %d failed, exiting\n", regInfo->clientPort);
          exit (0);
        }
	#endif

        // Do RPC Callback setup
        regInfo->clientProgNum = dsgClientCallbackInit();

        switch (regInfo->idType) {
   
        case 1:
	  if (argc != 4) { 
	    printUsage();
            exit(1);   
          }

          if (sscanf(argv[3], "%d", &regInfo->clientId) != 1) {
	    printUsage();
            exit(1);   
          }

          if (regInfo->clientId == 0) {
            printf("Broadcast ID of 0 is prohibited!\n");
            printUsage();
            exit(1);
          } else if (regInfo->clientId == 1) {
            printf("This DSG Client is registering with DSGCC for SCTE-65 Broadcast traffic\n");
          } else if (regInfo->clientId == 2) {
            printf("This DSG Client is registering with DSGCC for SCTE-18 Broadcast traffic\n");
          } else if (regInfo->clientId == 3) {
            printf("This DSG Client is registering with DSGCC for OCAP Carousel Broadcast traffic(*deprecated)\n");
          } else if (regInfo->clientId == 4) {
            printf("This DSG Client is registering with DSGCC for OpenCable Common Download Broadcast traffic(*deprecated)\n");
          } else {
	    printUsage();
            exit(1);   
          }
          printf("and will listen on UDP Port %d.\n", regInfo->clientPort);
          break;

        case 2:
	  if (argc != 4) { 
	    printUsage();
            exit(1);   
          }

          if (sscanf(argv[3], "%hx:%hx:%hx:%hx:%hx:%hx",
               (short int *) &idString[0], 
               (short int *) &idString[1],
               (short int *) &idString[2], 
               (short int *) &idString[3], 
               (short int *) &idString[4], 
               (short int *) &idString[5]) != 6) {
	    printUsage();
            exit(1);   
          }

          memcpy(regInfo->clientName, idString, 6);

          printf("This DSG Client is registering with DSGCC using well-known MAC address,\n");
          printf("and will listen on UDP Port %d.\n", regInfo->clientPort);
          break;

        case 3:
	  if (argc != 4) { 
	    printUsage();
            exit(1);   
          }

          if (sscanf(argv[3], "%d", &regInfo->clientId) != 1) {
	    printUsage();
            exit(1);   
          }
          printf("This DSG Client is registering with DSGCC using CAS ID %d, \n", regInfo->clientId);
          printf("and will listen on UDP Port %d.\n", regInfo->clientPort);
          break;

        case 4:
	  if (argc != 4) { 
	    printUsage();
            exit(1);   
          }

          if (sscanf(argv[3], "%d", &regInfo->clientId) != 1) {
	    printUsage();
            exit(1);   
          }
          printf("This DSG Client is registering with DSGCC using Application ID %d\n", regInfo->clientId);
          printf("and will listen on UDP Port %d.\n", regInfo->clientPort);
          break;

        case 5:
	  if (argc != 4) { 
	    printUsage();
            exit(1);   
          }

          if (strlen(argv[3]) > MAXAPPNAMELEN) {
	    printUsage();
            exit(1);   
          }
          if (sscanf(argv[3], "%s", regInfo->clientName) != 1) {
	    printUsage();
            exit(1);   
          }
          printf("This DSG Client is registering with DSGCC using Application name %s\n", regInfo->clientName);
          printf("and will listen on UDP Port %d.\n", regInfo->clientPort);
          break;

        case 6:
	  if (argc != 4) { 
	    printUsage();
            exit(1);   
          }

          if (sscanf(argv[3], "%d", &regInfo->clientId) != 1) {
	    printUsage();
            exit(1);   
          }
          printf("This DSG Client is registering with DSGCC using Vendor ID %d\n", regInfo->clientId);
          printf("and will listen on UDP Port %d.\n", regInfo->clientPort);
          break;

        }

  // fork off the callback service
  pid = fork();

  if (pid == 0) {
    dsgClientCallbackSvcRun();
    exit(1); // should never return!
  } else if (pid == -1) {
    printf("fork error!\n");
    exit(1);
  } else {

    #if defined(BCM_DSGCC_CLIENTS_USE_SHARED_MEMORY_IPC)
        sharedMemoryId = BcmSharedMemoryCreate(regInfo->clientPort, 1);
    #endif

    // printf("Client is pid %d\n", pid);

    handle = clnt_create("127.0.0.1", DSGREGISTRAR, DSGREGISTRARVERS, "udp");
    if (handle == 0) {
      printf("Could not contact DSG Registrar, exiting.\n");
      exit(1);
    }

	regInfo->handle = (int)handle;
    //printf("Calling registerdsgclient() \n");

	// Tell DSG-CC to remove the IP/UDP headers of tunnel packets.
	regInfo->idType |= 0x00010000;


#if 1
	retVal = registerdsgclient(regInfo);
#else

	dsg_classifier clientClassifier;

	memset(&clientClassifier, 0 , sizeof(dsg_classifier));
	clientClassifier.source_ip_used = 1;
	clientClassifier.source_ip_adr = htonl(0x0B18046B);
	clientClassifier.dest_ip_used = 1;
	clientClassifier.dest_ip_adr = htonl(0xE6000109);
	clientClassifier.port_used = 1;
	clientClassifier.port_start = htons(0x1234);
	clientClassifier.port_end = htons(0x5678);

	retVal = dsgcc_RegisterClientSetClassifier(regInfo, &clientClassifier);
#endif
    printf("dsgcc_RegisterClientSetClassifier() returned %s\n", ReturnCodeTypeName(retVal));
	if( retVal == DSGCC_SUCCESS )
	{
		printf("Tunnel request is registered successfully and tunnel is pending. \n"
			   "To find out the tunnel status either wait for DSG-CC notification or poll DSG-CC by calling dsgcc_KeepAliveClient()\n");
	}

	// Check status
	retVal = dsgcc_KeepAliveClient(&regInfoData);
	printf("dsgcc_KeepAliveClient() returned status %s\n", ReturnCodeTypeName(retVal & 0xffff));
	if(retVal & DSGCC_CLIENT_REGISTERED)
	{
		// Get tunnel status = MSB 16 bits
		retVal >>= 16;
		printf("tunnel status %s\n", TunnelStatusTypeName(retVal));
	}

    for ( ; ; ) 
	{
		#if defined(BCM_DSGCC_CLIENTS_USE_SHARED_MEMORY_IPC)
		 n = BcmSharedMemoryRead(sharedMemoryId, mesg, 0);
		#else
		len = sizeof(cliaddr);
		n = recvfrom(sockfd, mesg, MAXLINE, 0, (SA *) &cliaddr, &len);
		// printf("DSG Client recvfrom returned %d.\n", n);
		#endif
  
		if (n > 0) 
		{
			printf("DSG Client received %d bytes.\n", n);

			//DumpMessage(mesg, n);
		}

		if (n <= 0)
		{
			if (CLIENTPORTKEEPALIVE) 
			{
			  if (errno == EWOULDBLOCK) {
				// Timeout
				printf("Calling keepalivedsgclient() \n");
				retVal = dsgcc_KeepAliveClient(&regInfoData);
				printf("dsgcc_KeepAliveClient() returned status %s\n", ReturnCodeTypeName(retVal & 0xffff));
				if(retVal & DSGCC_CLIENT_REGISTERED)
				{
					// Get tunnel status = MSB 16 bits
					retVal >>= 16;
					printf("tunnel status %s\n", TunnelStatusTypeName(retVal));
				}
			  }
			}
		}
    }

    printf("Calling unregisterdsgclient() \n");
    retVal = dsgcc_UnregisterClient(regInfo);
    printf("dsgcc_UnregisterClient() returned %d\n", retVal);
    #if defined(BCM_DSGCC_CLIENTS_USE_SHARED_MEMORY_IPC)
    BcmSharedMemoryDelete(sharedMemoryId);
    #endif

    exit(0);
  }
}

char* ReturnCodeTypeName( unsigned int value ) 
{
	char* names[] =
	{
		"DSGCC_SUCCESS",			// = 0
		"DSGCC_ERROR",              // = 1
		"DSGCC_INVALID_PARAM",		// = 2
		"DSGCC_MAX_CLIENT_REACHED"  // = 3
	};

	return names[ value ];
}

/*char* TunnelStatusTypeName( unsigned int value ) 
{
	char* names[] =
	{
		"DSGCC_TUNNEL_REQUEST_PENDING", 	//= 1,
		"DSGCC_TUNNEL_OPEN",				//= 2,
		"DSGCC_TUNNEL_NOT_AVAILABLE",		//= 3,
		"DSGCC_TUNNEL_PENDING	"			//= 4
	};

	return names[ value ];
}*/
	
void dsgcc_ClientNotification(struct dsgccClientNotification *clientNotification) 
{
	printf("DSG CLIENT: NOTIFICATION from DSG-CC -> %s \n", TunnelStatusTypeName(clientNotification->eventType)); 	
}


// This function just spews the octets out to the console in hex format.
//
// Parameters:
//      pBuffer - the buffer to dump.
//      bufferSize - the number of bytes to dump.
//
// Returns:  Nothing.
//
static void DumpMessage(unsigned char *pBuffer, unsigned short bufferSize)
{
    unsigned short index, bytesPerLine, maxBytesPerLine;

    //if (gDebugLevel >= kDynSvcDebugAll)
    {
        printf("Message contents (%d bytes):\n\n", bufferSize);

        if ((pBuffer == NULL) || (bufferSize == 0))
        {
            printf("{Empty or NULL buffer!}\n");

            return;
        }

        // Split the buffer up into multiple lines of the number of bytes specified
        // below.
        maxBytesPerLine = 16;
        while (bufferSize > 0)
        {
            // Do the max bytes per line, or the number of bytes remaining, which
            // ever is less.
            bytesPerLine = maxBytesPerLine;
            if (bytesPerLine > bufferSize)
            {
                bytesPerLine = bufferSize;
            }

            // Show each octet of data in hex, with a space between.
            for (index = 0; index < bytesPerLine; index++)
            {
                if ((index & 0x03) == 0)
                {
                    printf("  ");
                }

                printf("%02x ", (int) pBuffer[index]);
            }
            printf("\n");

            // Show the ASCII representation, or a '.' if non-printable.
            for (index = 0; index < bytesPerLine; index++)
            {
                if ((index & 0x03) == 0)
                {
                    printf("  ");
                }

                //if (isprint(pBuffer[index]))
                if ((pBuffer[index] >= 32) && (pBuffer[index] <= 126))
                {
                    printf("%c  ", pBuffer[index]);
                }
                else
                {
                    printf(".  ");
                }
            }
            printf("\n");

            // Set up for the next line.
            pBuffer += bytesPerLine;
            bufferSize -= bytesPerLine;
        }
    }
}

