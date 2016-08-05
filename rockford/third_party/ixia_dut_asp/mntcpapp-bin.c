/*
  mntcpapp.c: TCP Stub Server Application for use with ANVL TCP test suite

  Copyright (c) Ixia 2002-2011
  All rights reserved.

*/

/*

  MNTCPAPP -- a Brief
  ===================

  To perform TCP tests sometimes it is required to run a stub application
  on the DUT. The job of this stub application is to generate the TCP user
  calls as per the test requirements. It generates the following user
  calls(as described in RFC 793) Open(Active/Passive), Send, Receive, Close
  and Abort. To generate these calls by the stub application, ANVL needs to
  send certain commands to the stub. This control  communication between ANVL
  and stub is done through a UDP channel. At this point the stub application
  behaves as a UDP server and ANVL acts as its client. On receiving different
  control commands the from ANVL via the control UDP channel, the stub
  application generates the above mentioned TCP calls. As a TCP application
  the stub sometimes acts as a tcp client and some times as a TCP server,
  depending on the type of Open call (active or passive) issued by it.

*/

//#define __DEBUG__

#include "mntcpos.h"
#include "mntcpapp.h"

/* #define  __TCP_STDURG__ if this socket option is suported */

#define HELP_STR_IPV6_VERSION  "usage: %s [-help|--h] [-v|--ver] [-p listen-port] [-ipv4/6] [-delay delay-time]\n"
#define HELP_STR_IPV4_VERSION  "usage: %s [-help|--h] [-v|--ver] [-p listen-port] [-delay delay-time]\n"

static void UDPRequestsLoop(void);
static void AppVersionGet(void);

/* Functions to generate TCP calls as mentioned in RFC 793 */
static int TCPListen(void *response);  /* Passive open */
static int TCPConnect(void *param);  /* active open */
static int TCPSend(TCPTestAppCommand_t cmd, char *data, int dataLen, boolean noDelay, boolean urg,
				   boolean noPush);
static int TCPReceive(int expectLen);
static int TCPReceiveLimSz(int expectLen);
static int TCPReceiveOnce(int expectLen);
static int TCPClose(int statSend);
static int TCPAbort(void);

static int OSSockAbort(void);

static int UDPSend(TCPTestAppCommand_t cmd, char *data, int dataLen, MNTCPAppSocketInfo_t sockInfo);

/* TCPSocketCreate is not a call described by RFC 793, it actually creates
   a stream socket. */
static int TCPSocketCreate(void);

static int StateSet(unsigned int state);

static void PendingErrorGet(ApplicationState_t *theAppState);
static int MSSSet(int mssVal);
static int DFBitSet(int dfBit);
static int TTLSet(long int ttlVal);
#ifdef LIB_IPV6
static int HopLimitSet(int hopLimitVal);
#endif /* LIB_IPV6 */
static int SockRecvBuffSet(long int rcvbuflen);
static void IssueAccept(void);
static int OptUrgSet(void);
static int OptUrgInlineSet(void);




/* initialization/logging method */
static void NotifyTest(boolean start, StartEndTestReqParamForm_t *form);
static void SendStatusToAnvl(char *msg);

static unsigned short IPChecksum(unsigned char *data,
								 unsigned long int len);
static unsigned long int IPChecksumIncr(unsigned char *data,
										unsigned long int len,
										unsigned long int total);

#ifdef __DEBUG__
static void DEBUGStateStrGet(MNTCPAppState_t stateIndx, char *stateStr);
static void DEBUGMsgDecode(char *cmdMsg, char *extra);
#endif

static int ShutDnConn(TCPTestAppCommand_t cmd, ShutDownSock_t type);


/* Global data */
static ApplicationState_t appState;

static char tempRecv[MAX_MESG_SIZE + 1];  /* +1 to add NULL at the end */
#ifdef LIB_IPV6
int gIPVersionForApp; /* IPv4 or IPv6 */
#endif /* LIB_IPV6 */
/*sankarshan start*/
int gConnSock = -1;

/*sankarshan end*/
/*>

  int main(int argc, char* argv[]);

  DESCRIPTION:

  This function parses command line arguments and opens a UDP socket
  through which the TCP Stub Application will listen for requests to
  perform TCP functions.

  ARGS:
  argc         - the number of arguments
  argv[]       - the list of arguments

  RETURNS:

  int          - C return value of main

<*/
int
main(int argc, char* argv[])
{
  char           temp[2 * SMALL_BUF_LEN];
  int            status = 0;
  int            count = 0;
  int            versionMajor = MNTCPAPP_VERSION_MAJOR;
  int            versionMinor = MNTCPAPP_VERSION_MINOR;
  unsigned short idx = 0;
  int            portOK = FALSE;
  int            delayOK = FALSE;
  int            modeTCPOK = FALSE;
  char           portStr[MAX_STR];
  char           delayStr[MAX_STR];
  char           helpStr[MAX_STR];

  MemSet((void *)temp, '\0', sizeof(temp));
  MemSet((void *)helpStr, '\0', sizeof(helpStr));
  MemSet((void *)portStr, '\0', sizeof(portStr));
  MemSet((void *)delayStr, '\0', sizeof(delayStr));

#ifdef LIB_IPV6
			StrCpy(helpStr, HELP_STR_IPV6_VERSION);
#else /* LIB_IPV6 */
			StrCpy(helpStr, HELP_STR_IPV4_VERSION);
#endif /* LIB_IPV6 */

  /* Check for number of arguments passed */
#ifdef LIB_IPV6
  if (argc > 6) {
    FPrintf(stdout, HELP_STR_IPV6_VERSION,   argv[0]);
    return -1;
  }
#else /* LIB_IPV6 */
  if (argc > 5) {
    FPrintf(stdout, HELP_STR_IPV4_VERSION, argv[0]);
    return -1;
  }
#endif /* LIB_IPV6 */

#ifdef LIB_IPV6
  gIPVersionForApp = MODE_MNTCPAPP_IPV4; /* default is IPv4 mode */
#endif /* LIB_IPV6 */

  if (argc > 1) {

	/* -help option */
	if ((StrCmp(argv[1] , "-help") == 0) || (StrCmp(argv[1] , "--h") == 0) ) {
    FPrintf(stdout, helpStr, argv[0]);
	  return MNTCPAPP_CMDLINE_ERR;
	}

	/* -v option, for version */
	if ((StrCmp(argv[1] , "-v") == 0) || (StrCmp(argv[1], "--ver") == 0)) {
	  FPrintf(stdout, "%s: Version %d.%d\n", argv[0], versionMajor,
			  versionMinor);
	  return MNTCPAPP_CMDLINE_ERR;
	}

	if (argc >= 2) {
	  idx = 1; /* 1 for the 1st option */
	  while (idx < argc) {
		if (!StrCmp(argv[idx],"-p") && portOK != TRUE) {
		  /* port option ( -p )given
			 and also checking if any
			 repeatation occurs
		  */

		  if ((idx + 1) > (argc - 1)) { /* checking wheather the next
										   argument is present or not
										*/
			FPrintf(stdout, helpStr, argv[0]);
			return MNTCPAPP_CMDLINE_ERR;
		  }
		  else {  /* check if a correct port number is given or not */
			StrCpy(temp, argv[idx + 1]);
			for (count = 0; count < (StrLen(temp)); count++) {
			  if (!IsDigit(temp[count])) {
        FPrintf(stdout, helpStr, argv[0]);
				return MNTCPAPP_CMDLINE_ERR;
			  }
			}
			StrCpy(portStr, temp);
			portOK = TRUE;
			idx+=2;
		  }
		}
    else if (!StrCmp(argv[idx],"-delay") && delayOK != TRUE) {
      /* checking wheather the next argument is present or not */
      if ((idx + 1) > (argc - 1)) {
        FPrintf(stdout, helpStr, argv[0]);
        return MNTCPAPP_CMDLINE_ERR;
      }
      else {
        /* check if a correct delay time is given or not */
        StrCpy(temp, argv[idx + 1]);
        for (count = 0; count < (StrLen(temp)); count++) {
          if (!IsDigit(temp[count])) {
            FPrintf(stdout, helpStr, argv[0]);
            return MNTCPAPP_CMDLINE_ERR;
          }
        }
        StrCpy(delayStr, temp);
        delayOK = TRUE;
        idx+=2;
      }
    }
#ifdef LIB_IPV6
		else if (!StrCmp(argv[idx], "-ipv6")) {
		  gIPVersionForApp = MODE_MNTCPAPP_IPV6;
		  idx+=1;
		  modeTCPOK = TRUE;
		}
		else if (!StrCmp(argv[idx], "-ipv4")) {
		  gIPVersionForApp = MODE_MNTCPAPP_IPV4;
		  idx+=1;
		  modeTCPOK = TRUE;
		}
#endif /* LIB_IPV6 */
		else { /* there are some error */
			FPrintf(stdout, helpStr, argv[0]);
		  return MNTCPAPP_CMDLINE_ERR;
		}
	  } /* while loop ends */
	}
  }

  OSInit();

  MemSet(&appState, '\0', sizeof(appState));

  /* initialize sockets */
  appState.tcpSockInfo.connSock = -1;
  appState.tcpListenSock = -1;
  appState.udpSockInfo.connSock = -1;

  /* Open a UDP socket (an Internet datagram socket) */

  if ((appState.udpSockInfo.connSock =
	   OSSockSocket(MNTCPAPP_SOCKTYPE_DGRAM)) == MNTCPAPP_API_RETCODE_NOTOK) {
	FPrintf(stderr, "! Unable to create a UDP socket\n");
	PError("! reason");
	return MNTCPAPP_APICALL_ERR;
  }

  /* Bind a local address so that the client can send requests */
  if (portOK != TRUE) {
    appState.udpSockInfo.localPort = MNTCPAPP_UDP_PORT;
  }
  else {
    appState.udpSockInfo.localPort = ASCIIToInt(portStr);
  }

  /* delay before sending packets on UDP socket */
  if (delayOK != TRUE) {
    appState.udpSockInfo.delay = 0;
  }
  else {
    appState.udpSockInfo.delay = ASCIIToInt(delayStr);
  }

#ifdef LIB_IPV6
  if (gIPVersionForApp == MODE_MNTCPAPP_IPV6) {
    stubIPVersion = STUB_IPV6;
	status = OSSockBind6(appState.udpSockInfo.connSock, 0,
						 appState.udpSockInfo.localPort);
  }
  else {
#endif /* LIB_IPV6 */
    stubIPVersion = STUB_IPV4;
	status = OSSockBind(appState.udpSockInfo.connSock, 0,
						appState.udpSockInfo.localPort);
#ifdef LIB_IPV6
  }
#endif /* LIB_IPV6 */

  if (status == MNTCPAPP_API_RETCODE_NOTOK) {
	OSSockClose(appState.udpSockInfo.connSock);
	FPrintf(stdout, "! Unable to bind the UDP socket to any local address\n");
	FPrintf(stdout, "! Probably Stub is already running or another application is bound to the same local address.\n");
/*	PError("! reason");*/
	return MNTCPAPP_APICALL_ERR;
  }

  /* Wait for requests from ANVL and take action(s) accordingly */
  UDPRequestsLoop();
  OSSockClose(appState.udpSockInfo.connSock);
  return 0;
}

/*-----------------------------------------------------------------------*/

/*
  NOTE:
  #1 After receiving a command through the UDP channel this function sends
  an ack. This ack is an ASCII string containing the length of the
  received command
*/



/*>>
   static void UDPRequestsLoop(void)

  DESCRIPTION:

  This function waits for UDP messages on the UDP socket, parses them, and then
  calls the appropriate TCP subroutine.  This function continues in this
  manner indefinitely.

<<*/
static void
UDPRequestsLoop(void)
{
  boolean       noDelay = FALSE;
  boolean       urg = FALSE;
  boolean       noPush = FALSE;
  boolean       repeatDataSent = TRUE;
  boolean       dataSent = FALSE;
  unsigned char attempt = 0;
  unsigned int  anvlIPAddr = 0;
  unsigned char flags;
  unsigned char *data = 0;
  char          *repeatDataBuff = 0;
  ubyte         mesg[MAX_MESG_SIZE];
#ifdef __DEBUG__
  char          copyMesg[MAX_MESG_SIZE];
#endif
  unsigned char          mesgAck[MAX_MESG_SIZE];
  unsigned char          buf[MAX_STR];
  char          tmpBuf[SMALL_BUF_LEN];
  int           numBytes = 0;
  unsigned int           ackLen = 0;
  unsigned int           expectLen = 0;
  int           dataLen = 0;
  int           tmpLen = 0;
  unsigned int  tempState = 0;
  int           mssVal = 0;
  int           ttlVal = 0;
  int           dfBit  = 0;
#ifdef LIB_IPV6
  int           hopLimitVal = 0;
#endif /* LIB_IPV6 */
  unsigned short int   cmdId = 0;
  int         errorCode = 0;
  int ret = 0;
  unsigned long rcvBufVal = 0;
  unsigned short checksum = 0;
  unsigned short int          anvlUDPPort = 0;
  unsigned           	      dutUDPPort = 0;
  unsigned char sendAttempts = 5; /* Try sending the packet this times before
                                      giving up */
  MNTCPAppSocketInfo_t udpDataSockInfo;
  STUBForm_t *rcvdCmdForm = 0;
  STUBForm_t *ackForm = 0;
  STUBForm_t *rspForm = 0;
  SetTTLParamForm_t *ttlForm = 0;
  StartEndTestReqParamForm_t *startEndTest = 0;
#ifdef LIB_IPV6
  SetHopLimitParamForm_t *hopLimitForm = 0;
#endif
  unsigned char anvlIPv6Addr[16];
  unsigned short int dutPort = 0;
  unsigned short int anvlPort = 0;
  void *param = 0;
  unsigned int paddingLen = 0, paramLen = 0, j = 0;
  unsigned char padding[MAX_PAD_LEN];
  unsigned char *paddingPtr = 0;
  unsigned char sampleData[30];
  byte log[MAX_STR];
  byte suiteName[MAX_STR];

  MemSet((void *)&udpDataSockInfo, '\0', sizeof(udpDataSockInfo));
  MemSet((void *)mesg, '\0', sizeof(mesg));
#ifdef __DEBUG__
  MemSet((void *)copyMesg, '\0', sizeof(copyMesg));
#endif
  MemSet((void *)mesgAck, '\0', sizeof(mesgAck));
  MemSet((void *)buf, '\0', sizeof(buf));
  MemSet((void *)tmpBuf, '\0', sizeof(tmpBuf));
  MemSet((void *)padding, '\0', sizeof(padding));
  MemSet((void *)sampleData, '\0', sizeof(sampleData));
  MemSet((void *)anvlIPv6Addr, '\0', sizeof(anvlIPv6Addr));
  MemSet((void *)log, '\0', sizeof(log));
  for ( ; ; ) {  /* infinite for */

    FPrintf(stdout, "Ready to receive UDP (binary) request on port %d\n",
        appState.udpSockInfo.localPort);
#ifdef __DEBUG__
	FPrintf(stdout, "DEBUG --> Blocked to receive command from ANVL\n");
#endif
	numBytes = OSSockRecvFrom(appState.udpSockInfo.connSock, mesg,
							  MAX_MESG_SIZE, &(appState.udpSockInfo));

	if (numBytes == MNTCPAPP_API_RETCODE_NOTOK) {
	  FPrintf(stderr, "! recvfrom error occurred\n");
	  PError("! UDPRequestsLoop::reason");
	  continue;
	}
	mesg[numBytes] = '\0'; /* NULL terminate the received UDP message */

#ifdef __DEBUG__
	FPrintf(stdout, "DEBUG --> Received command [%s] from ANVL, numbytes = %d\n", mesg, numBytes);
#endif

	/*
	   the integer (numBytes => number of received) will be sent via
           the UDP channel as an ACK of the received UDP command
	*/
#ifdef __DEBUG__
	StrCpy(copyMesg, mesg);
	DEBUGMsgDecode(copyMesg, 0);
#else
#endif
        rcvdCmdForm = STUBFormCreate();
        ackForm = STUBFormCreate();
        STUBPacketToForm(mesg, rcvdCmdForm);

        cmdId = rcvdCmdForm->cmdID;
        ackForm->msgType = ACK;
        ackForm->cmdID = cmdId;
        ackForm->paramsLen = 4;
        ackLen = STUBBuild(ackForm, mesgAck);
        ackForm->params = &numBytes;
        ackLen += Pack(mesgAck+8, "L", (unsigned int *)ackForm->params);
        mesgAck[ackLen] = '\0';
        STUBFormDestroy(ackForm);

	/* send acknowledgement */

#ifdef __DEBUG__
	FPrintf(stdout, "\n--->Start UDP Control Message ACK\n");
#endif

  STUBTypeToString(FALSE, cmdId, log);
  FPrintf(stdout, "Sending %s acknowledgement\n", log);

	ret = OSSockSendTo(appState.udpSockInfo.connSock, mesgAck,
					 ackLen, appState.udpSockInfo);
	if(ret == MNTCPAPP_API_RETCODE_NOTOK) {
	  FPrintf(stderr, "! sendto error occurred\n");
	  PError("! UDPRequestsLoop::reason");
	  continue;
	}

#ifdef __DEBUG__
  FPrintf(stdout, "--->End UDP Control Message ACK\n\n");
#endif

  FPrintf(stdout, "Sending %s response\n", log);

	switch (cmdId) {
	case TCP_CMD_GET_VERSION:
	  AppVersionGet();
	  break;

	case TCP_CMD_CONNECT:
          param = ConnectReqParamFormCreate();
	  ConnectReqParamToForm((unsigned char *)rcvdCmdForm->params, (ConnectReqParamForm_t *)param);
          dutPort = ((ConnectReqParamForm_t *)param)->portNum;
	  appState.tcpSockInfo.connSock = TCPConnect(param);
	  break;

	case TCP_CMD_SEND:
	  if(rcvdCmdForm->params) {
        param = SendDataReqParamFormCreate();
	    SendDataReqParamToForm((unsigned char *)rcvdCmdForm->params, (SendDataReqParamForm_t *)param);

        if (stubIPVersion == STUB_IPV6) {
            MemMove(anvlIPv6Addr, ((SendDataReqParamForm_t *)param)->anvlIPv6Addr, 16);
        }
        else {
            anvlIPAddr = ((SendDataReqParamForm_t *)param)->anvlIPAddr;
        }
		anvlPort = ((SendDataReqParamForm_t *)param)->portNum;
		flags = ((SendDataReqParamForm_t *)param)->flags;
		dataLen = ((SendDataReqParamForm_t *)param)->dataLen;

		urg = OS_BIT_GET(flags, 0);
		noDelay = OS_BIT_GET(flags, 1);
		noPush = !OS_BIT_GET(flags, 2);
	 }
	  else {
		  FPrintf(stderr, "! Did not receive data\n");
	  }

      if (appState.dutState == TCP_STATE_LISTEN) {
          /* In LISTEN we won't do a Connect so we have to supply the
             other side's address info directly */
#ifdef LIB_IPV6
          if (gIPVersionForApp == MODE_MNTCPAPP_IPV6){
              MemMove(appState.tcpSockInfo.anvlIPv6Addr, anvlIPv6Addr, 16);
          }
          else {
#endif /* LIB_IPV6 */
              appState.tcpSockInfo.anvlIPAddr = anvlIPAddr;
#ifdef LIB_IPV6
          }
#endif /* LIB_IPV6 */
          appState.tcpSockInfo.anvlPort = anvlPort;
          if (OSSockFcntl(appState.tcpSockInfo.connSock,
                      FCNTL_APPEND_NONBLOCKING) ==
                  MNTCPAPP_API_RETCODE_NOTOK) {
              FPrintf(stderr, "! Flag setting error before connect\n");
              PError("! TCPConnect::reason");
          }
      }
	  if (!TCPSend(rcvdCmdForm->cmdID, ((SendDataReqParamForm_t *)param)->data, dataLen, noDelay, urg, noPush)) {
		FPrintf(stderr, "! Error Data not sent\n", ret);
                dataSent = FALSE;
	  }
	  Free(((SendDataReqParamForm_t *)param)->data);
	  ((SendDataReqParamForm_t *)param)->data = 0;
	  if(param) {
	    SendDataReqParamFormDestroy(param);
	  }
	  break;

	case TCP_CMD_SEND_REPEAT_DATA:
	  if(rcvdCmdForm->params) {
        param = SendRptDataReqParamFormCreate();
	    SendRptDataReqParamToForm((unsigned char *)rcvdCmdForm->params, (SendRptDataReqParamForm_t *)param);
        if (stubIPVersion == STUB_IPV6) {
            MemMove(anvlIPv6Addr, ((SendRptDataReqParamForm_t *)param)->anvlIPv6Addr, 16);
        }
        else {
            anvlIPAddr = ((SendRptDataReqParamForm_t *)param)->anvlIPAddr;
        }
		anvlPort = ((SendRptDataReqParamForm_t *)param)->portNum;
		flags = ((SendRptDataReqParamForm_t *)param)->flags;
	    data = (((SendRptDataReqParamForm_t *)param)->data);
		dataLen = ((SendRptDataReqParamForm_t *)param)->dataLen;
		repeatDataBuff = (char *)Malloc((unsigned int)(dataLen + 1));
		MemSet(repeatDataBuff, *data, (size_t)dataLen);
		repeatDataBuff[dataLen] = '\0';

		urg = OS_BIT_GET(flags, 0);
		noDelay = OS_BIT_GET(flags, 1);
		noPush = !OS_BIT_GET(flags, 2);

		for (attempt = 0; (attempt < sendAttempts); attempt++) {
		/* Try sending the packet this many times till send is successful and
         between each send attempt wait for 1 sec */
		  if (TCPSend(rcvdCmdForm->cmdID, repeatDataBuff, dataLen, noDelay, urg, noPush) >= 0) {
		  FPrintf(stdout, "Data sent\n");
                  repeatDataSent = TRUE;
        }

        if (repeatDataSent) {
          break;
        }
        else {
          OSSleep(1); /* Wait for 1 sec before attempting to send again */
        }
       }
	   Free(repeatDataBuff);
	   repeatDataBuff = 0;
	   if(param) {
	     SendRptDataReqParamFormDestroy(param);
	   }

	 }
	  else {
		  FPrintf(stderr, "! Did not receive repeat data\n");
	  }

	  break;

	case TCP_CMD_RECV:
	  Unpack((unsigned char *)rcvdCmdForm->params, "L", &expectLen);
	  if(!expectLen) {
		expectLen = MNTCPAPP_DEF_RECV_DATALEN;
	  }
	  if (TCPReceive(expectLen) > 0) {
		FPrintf(stdout, "Data received\n");
	  }
	  else{
		FPrintf(stderr, "! Did not received data\n");
	  }
	  break;

	case TCP_CMD_RECV_LIMSZ:
	  Unpack((unsigned char *)rcvdCmdForm->params, "L", &expectLen);
	  if(!expectLen) {
		expectLen = MNTCPAPP_DEF_RECV_DATALEN;
	  }
	  if (TCPReceiveLimSz(expectLen) > 0) {
		FPrintf(stdout, "Data received\n");
	  }
	  else {
		FPrintf(stdout, "! Did not receive data\n");
	  }
	  break;

	case TCP_CMD_RECV_ONCE:
	  Unpack((unsigned char *)rcvdCmdForm->params, "L", &expectLen);
	  if(!expectLen) {
		expectLen = MNTCPAPP_DEF_RECV_DATALEN;
	  }

	  if (TCPReceiveOnce(expectLen) > 0) {
		FPrintf(stdout, "Data received\n");
	  }
	  else {
		FPrintf(stdout, "! Did not receive data\n");
	  }
	  break;

	case TCP_CMD_LISTEN:
          if(rcvdCmdForm) {
            param = (ListenReqParamForm_t *)ListenReqParamFormCreate();
			ListenReqParamToForm((unsigned char *)rcvdCmdForm->params, (ListenReqParamForm_t *)param);
            dutPort = ((ListenReqParamForm_t *)param)->portNum;
	        TCPListen(param);
          }
          else {
            FPrintf(stderr, "! Did not receive the port to listen to\n");
          }
	  break;

	case TCP_CMD_CLOSE:
	  /* TCP's CLOSE Call corresponds to OS Shutdown Write on socket */
	  ShutDnConn(TCP_CMD_CLOSE, SHUT_WRITE);
	  break;

	case TCP_CMD_SET_STATE:
	  Unpack((unsigned char *)rcvdCmdForm->params, "L", &tempState);
	  StateSet(tempState);
	  break;

	case TCP_CMD_ASYNC_RECV:
	  appState.recvMode = 1;
	  break;

	case TCP_CMD_GET_SAMPLE_DATA:

	  /* Command to retrieve fixed data (through the UDP channel) which
		 is already received through the TCP channel */

	  rspForm = STUBFormCreate();
	  rspForm->msgType = RESPONSE;
	  rspForm->cmdID = TCP_CMD_GET_SAMPLE_DATA;
	  dataLen = StrLen(appState.rcvdBuff);
	  if(StrLen(appState.rcvdBuff)) {
	    StrNCpy(sampleData, appState.rcvdBuff, StrLen(appState.rcvdBuff));
          }
	    paramLen = sizeof(unsigned int) + dataLen;
	    if(paramLen % 4) {
              paddingLen = (((paramLen-1)|3)+1) - paramLen;
		  paramLen += paddingLen;
		  paddingPtr = padding;
		  for(j=0; j < paddingLen; j++) {
		    *(paddingPtr++) = '~';
		  }
	    }
	    rspForm->paramsLen = paramLen;
	    MemSet((void *)buf, '\0', sizeof(buf));
	    tmpLen = STUBBuild(rspForm, buf);
	    rspForm->params = GetSampleDataRespParamFormCreate();
	    ((GetSampleDataRespParamForm_t*)rspForm->params)->dataLen = StrLen(appState.rcvdBuff);
            if(dataLen) {
	      ((GetSampleDataRespParamForm_t*)rspForm->params)->data = sampleData;
            }

            if(paddingLen) {
	      ((GetSampleDataRespParamForm_t*)rspForm->params)->padding = padding;
            }
	    tmpLen += GetSampleDataRespParamBuild(rspForm->params, buf+8, paddingLen);

	    if (OSSockSendTo(appState.udpSockInfo.connSock, buf,
					   tmpLen, appState.udpSockInfo) ==
		  MNTCPAPP_API_RETCODE_NOTOK ) {
		FPrintf(stderr, "! Sendto failed\n");
		PError("! UDPRequestsLoop::reason");
	    }
	  if(rspForm->params) {
	    GetSampleDataRespParamFormDestroy(rspForm->params);
	  }
	  if(rspForm) {
	    STUBFormDestroy(rspForm);
	  }
	  break;

	case TCP_CMD_GET_DATA:
	  /* Command to retrieve the data (through the UDP channel) which
		 is already received through the TCP channel */

	  tmpLen = (unsigned int)StrLen(appState.rcvdBuff);

	  checksum = IPChecksum((unsigned char *)appState.rcvdBuff,
							(unsigned long int)tmpLen);
#ifdef __DEBUG__
	  FPrintf(stdout, "Returning data length: %u checksum: %u\n",
			  tmpLen, checksum);
#endif

	  /* Add padding, if necessary */
	  rspForm = STUBFormCreate();
	  rspForm->msgType = RESPONSE;
	  rspForm->cmdID = TCP_CMD_GET_DATA;
	  rspForm->params = GetDataLenRespParamFormCreate();
	  rspForm->paramsLen = 4;
	  ((GetDataLenRespParamForm_t*)rspForm->params)->dataLen = (unsigned short)tmpLen;
	  ((GetDataLenRespParamForm_t*)rspForm->params)->checksum = checksum;
	  MemSet((void *)buf, '\0', sizeof(buf));
	  tmpLen = STUBBuild(rspForm, buf);
	  tmpLen += GetDataLenRespParamBuild(rspForm->params, buf+8);

	  if (OSSockSendTo(appState.udpSockInfo.connSock, buf,
					   tmpLen, appState.udpSockInfo)  ==
		  MNTCPAPP_API_RETCODE_NOTOK ) {
		FPrintf(stderr, "! Sendto failed\n");
		PError("! UDPRequestsLoop::reason");
	  }
	  GetDataLenRespParamFormDestroy(rspForm->params);
	  STUBFormDestroy(rspForm);
	  break;

	case TCP_CMD_GET_CODE:
	  /* Command to retrieve the last error code. */
	  errorCode = OSErrorCodeToRFCError(appState.lastErrCode[appState.topCode-1]);
	  rspForm = STUBFormCreate();
	  rspForm->msgType = RESPONSE;
	  rspForm->cmdID = TCP_CMD_GET_CODE;
	  rspForm->paramsLen = 4;
	  rspForm->params = &errorCode;
	  MemSet((void *)buf, '\0', sizeof(buf));
	  tmpLen = STUBBuild(rspForm, buf);
	  tmpLen += Pack(buf+8, "L", &errorCode);

	  if (OSSockSendTo(appState.udpSockInfo.connSock, buf,
					   tmpLen, appState.udpSockInfo)  ==
		  MNTCPAPP_API_RETCODE_NOTOK ) {
		FPrintf(stderr, "! Sendto failed\n");
		PError("! UDPRequestsLoop::reason");
	  }
	  else {
		if (appState.topCode > 0) {
		  --appState.topCode;
		}
	  }
	  STUBFormDestroy(rspForm);
	  break;

	case TCP_CMD_GET_RETCODE:
	  /* Command to retrieve the return code of the last function called. */
	  rspForm = STUBFormCreate();
	  rspForm->msgType = RESPONSE;
	  rspForm->cmdID = TCP_CMD_GET_RETCODE;
	  rspForm->paramsLen = 4;
	  rspForm->params = &appState.returnCode;
	  MemSet((void *)buf, '\0', sizeof(buf));
	  tmpLen = STUBBuild(rspForm, buf);
	  tmpLen += Pack(buf+8, "L", &appState.returnCode);

	  if (OSSockSendTo(appState.udpSockInfo.connSock, buf,
					   tmpLen, appState.udpSockInfo) ==
		  MNTCPAPP_API_RETCODE_NOTOK) {
		FPrintf(stderr, "! Sendto failed\n");
		PError("! UDPRequestsLoop::reason");
	  }
	  STUBFormDestroy(rspForm);
	  break;

	case TCP_CMD_DUT_REBOOT:
	  FPrintf(stderr, "! Reboot is not supported\n");
	  break;

	case TCP_CMD_SOCK_CREATE:
	  TCPSocketCreate();
	  break;

	case TCP_CMD_ABORT:
	  TCPAbort();
	  break;

	case TCP_CMD_GET_PERROR:
	  PendingErrorGet(&appState);
	  break;

	case TCP_CMD_SET_DF_BIT:
	  Unpack((unsigned char *)rcvdCmdForm->params, "L", &dfBit);
	  if (!dfBit) {
		dfBit = MNTCPAPP_DEF_DFBIT;
	  }

	  if (DFBitSet(dfBit) == 0) {
		FPrintf(stdout, "! Could not set DF Bit\n");
	  }
	  break;

	case TCP_CMD_SET_MSS:
	  Unpack((unsigned char *)rcvdCmdForm->params, "L", &mssVal);
	  if (!mssVal) {
		mssVal = MNTCPAPP_DEF_MSS;
	  }
	  if (MSSSet(mssVal) == 0) {
		FPrintf(stdout, "! Could not set MSS\n");
	  }
	  break;

	case TCP_CMD_SET_TTL:
	  ttlForm = SetTTLParamFormCreate();
	  SetTTLParamToForm((unsigned char *)rcvdCmdForm->params, ttlForm);
	  ttlVal = (int)ttlForm->ttl;
	  if (!ttlVal) {
		ttlVal = DEF_IP_TTL_VAL;
	  }
	  if (TTLSet(ttlVal) == 0) {
		FPrintf(stdout, "! Could not set TTL\n");
	  }
	  SetTTLParamFormDestroy(ttlForm);
	  break;

#ifdef LIB_IPV6
	case TCP_CMD_SET_HOPLIMIT:
	  hopLimitForm = SetHopLimitParamFormCreate();
	  SetHopLimitParamToForm((unsigned char *)rcvdCmdForm->params, hopLimitForm);
	  hopLimitVal = (int)hopLimitForm->hopLimit;
	  if (!hopLimitVal) {
		hopLimitVal = DEF_IPV6_HOPLIMIT_VAL;
	  }
	  if (HopLimitSet(hopLimitVal) == 0) {
		FPrintf(stdout, "! Could not set HOP LIMIT\n");
	  }
	  SetHopLimitParamFormDestroy(hopLimitForm);
	  break;
#endif /* LIB_IPV6 */

	case TCP_CMD_SET_RCVBUF_LEN:
	  Unpack((unsigned char *)rcvdCmdForm->params, "L", &rcvBufVal);
	  if(!rcvBufVal) {
		rcvBufVal = DEF_SOCK_RCVBUFF_LEN;
	  }

	  if (SockRecvBuffSet((long int)rcvBufVal) == 0) {
		FPrintf(stdout, "! Could not set sock recv buffer\n");
	  }
	  break;

	case TCP_CMD_NOTIFY_TEST_START:
	  startEndTest = StartEndTestReqParamFormCreate();
	  StartEndTestReqParamToForm(rcvdCmdForm->params, startEndTest);
	  NotifyTest(TRUE, startEndTest);
      StartEndTestReqParamFormDestroy(startEndTest);
	  break;

	case TCP_CMD_NOTIFY_TEST_END:
	  startEndTest = StartEndTestReqParamFormCreate();
	  StartEndTestReqParamToForm(rcvdCmdForm->params, startEndTest);
	  NotifyTest(FALSE, startEndTest);
      StartEndTestReqParamFormDestroy(startEndTest);
	  break;

	case TCP_CMD_API_ACCEPT:
	  IssueAccept();
	  break;

	case TCP_CMD_OPT_STDURG:
	  if (OptUrgSet() == 0) {
		FPrintf(stdout,"! Could not set socket option TCP_STDURG\n");
	  }
	  break;

	case TCP_CMD_OPT_URGINLINE:
	  if (OptUrgInlineSet() == 0) {
		FPrintf(stdout,"! Could not set socket option SO_OOBINLINE\n");
	  }
	  break;

	case TCP_CMD_SHUTDOWN_READ:
	  ShutDnConn(TCP_CMD_SHUTDOWN_READ, SHUT_READ);
	  break;

	case TCP_CMD_SHUTDOWN_WRITE:
	  ShutDnConn(TCP_CMD_SHUTDOWN_WRITE, SHUT_WRITE);
	  break;

	case TCP_CMD_SHUTDOWN_RD_WT:
	  ShutDnConn(TCP_CMD_SHUTDOWN_RD_WT, SHUT_RD_WT);
	  break;

	case UDP_CMD_SEND:
	  param = UDPSendDataReqParamFormCreate();
	  UDPSendDataReqParamToForm((unsigned char *)rcvdCmdForm->params, (UDPSendDataReqParamForm_t *)param);
      if (stubIPVersion == STUB_IPV6) {
          MemMove(anvlIPv6Addr, ((UDPSendDataReqParamForm_t *)param)->anvlIPv6Addr, 16);
      }
      else {
          anvlIPAddr = ((UDPSendDataReqParamForm_t *)param)->anvlIPAddr;
      }
	  anvlUDPPort = ((UDPSendDataReqParamForm_t *)param)->dstPort;
	  dutUDPPort = ((UDPSendDataReqParamForm_t *)param)->srcPort;
	  dataLen = ((UDPSendDataReqParamForm_t *)param)->dataLen;
	  data = (((UDPSendDataReqParamForm_t *)param)->data);

	  MemSet(&udpDataSockInfo, '\0', sizeof(udpDataSockInfo));

	  /* initialize socket */
	  udpDataSockInfo.connSock = -1;

#ifdef LIB_IPV6
	  if (gIPVersionForApp == MODE_MNTCPAPP_IPV6){
		MemMove((char *)udpDataSockInfo.anvlIPv6Addr, anvlIPv6Addr, 16);
	  }
	  else {
#endif /* LIB_IPV6 */
		udpDataSockInfo.anvlIPAddr = anvlIPAddr;
#ifdef LIB_IPV6
	  }
#endif /* LIB_IPV6 */

	  udpDataSockInfo.anvlPort = anvlUDPPort;
	  udpDataSockInfo.localPort = dutUDPPort;

	  if (!UDPSend(rcvdCmdForm->cmdID, data, dataLen, udpDataSockInfo)) {
		FPrintf(stdout, "UDP Data sent\n");
                dataSent = FALSE;

	  }
          else {
            dataSent = TRUE;
          }

	  if(param) {
	    UDPSendDataReqParamFormDestroy(param);
	  }
	  break;

	case UDP_CMD_SEND_REPEAT_DATA:
          param = UDPSendRptDataReqParamFormCreate();
	  UDPSendRptDataReqParamToForm((unsigned char *)rcvdCmdForm->params, (UDPSendRptDataReqParamForm_t *)param);
      if (stubIPVersion == STUB_IPV6) {
          MemMove(anvlIPv6Addr, ((UDPSendRptDataReqParamForm_t *)param)->anvlIPv6Addr, 16);
      }
      else {
          anvlIPAddr = ((UDPSendRptDataReqParamForm_t *)param)->anvlIPAddr;
      }
	  anvlUDPPort = ((UDPSendRptDataReqParamForm_t *)param)->dstPort;
	  dutUDPPort = ((UDPSendRptDataReqParamForm_t *)param)->srcPort;
	  dataLen = ((SendRptDataReqParamForm_t *)param)->dataLen;
	  data = (((SendRptDataReqParamForm_t *)param)->data);
	  repeatDataBuff = (char *)Malloc((unsigned int)(dataLen + 1));
	  MemSet(repeatDataBuff, *data, (size_t)dataLen);
	  repeatDataBuff[dataLen] = '\0';
	  MemSet(&udpDataSockInfo, '\0', sizeof(udpDataSockInfo));

	  /* initialize socket */
	  udpDataSockInfo.connSock = -1;

#ifdef LIB_IPV6
	  if (gIPVersionForApp == MODE_MNTCPAPP_IPV6){
		MemMove((char *)udpDataSockInfo.anvlIPv6Addr, anvlIPv6Addr, 16);
	  }
	  else {
#endif /* LIB_IPV6 */
		udpDataSockInfo.anvlIPAddr = anvlIPAddr;
#ifdef LIB_IPV6
    }
#endif /* LIB_IPV6 */

	  udpDataSockInfo.anvlPort = anvlUDPPort;
	  udpDataSockInfo.localPort = dutUDPPort;

	  if (!UDPSend(rcvdCmdForm->cmdID, repeatDataBuff, dataLen, udpDataSockInfo)) {
		  FPrintf(stdout, "UDP Repeat Data sent\n");
                  repeatDataSent = FALSE;

	  }
          else {
            repeatDataSent = TRUE;
          }
	  /* Free allocated buffer */
	  Free(repeatDataBuff);
	  if(param) {
	    UDPSendRptDataReqParamFormDestroy(param);
	  }
	  break;

	default:
	  FPrintf(stdout, "Unknown command :: %d\n", cmdId);
	  break;
	} /* end switch */
        STUBFormDestroy(rcvdCmdForm);

  } /* infinite for */

  return;
} /* UDPRequestsLoop */

/*>>

  (int) socket = TCPConnect(char *anvlAddr, char *anvlTCPPort);

  DESCRIPTION:

  This function issues a TCP active open call. If socket is not
  created then it creates the socket first and then issues a
  nonblocking connect call on this socket. It tries to connect
  to the address passed to this function as function arguments.

  ARGS:

  anvlAddr     - IP address of ANVL.
  anvlTCPPort  - TCP port number of ANVL.

  RETURNS:

  int          - the socket used for the connect call

<<*/
static int
TCPConnect(void *param)
{
  int              connResult = 0;
  unsigned int status = 1, msgLen = 0;
  unsigned char             buf[MAX_STR];
  ConnectReqParamForm_t *form = 0;
  STUBForm_t *rspForm = STUBFormCreate();
  ubyte16 zeroIPv6Addr;

#ifdef __DEBUG__
  char             state[MAX_STR];
  MemSet((void *)state, '\0', sizeof(state));

  FPrintf(stdout, "\n--->In TCPConnect\n");
#endif
  form = (ConnectReqParamForm_t *)param;
  rspForm->msgType = RESPONSE;
  rspForm->cmdID = TCP_CMD_CONNECT;
  rspForm->paramsLen = 4;
  MemSet((void *)buf, '\0', sizeof(buf));
  MemSet((void *)zeroIPv6Addr, 0, 16);

  TCPSocketCreate();
  rspForm->params = &status;

  /* Connect to the server */
  if (stubIPVersion == STUB_IPV6) {
    if (!MemCmp(form->anvlIPv6Addr, zeroIPv6Addr, 16)) {
      MemMove(appState.tcpSockInfo.anvlIPv6Addr, zeroIPv6Addr, 16);
      appState.tcpSockInfo.anvlPort = 0;
    }
    else {
      MemMove(appState.tcpSockInfo.anvlIPv6Addr, form->anvlIPv6Addr, 16);
      appState.tcpSockInfo.anvlPort = (unsigned short int)form->portNum;
      if (OSSockFcntl(appState.tcpSockInfo.connSock,
            FCNTL_APPEND_NONBLOCKING) == MNTCPAPP_API_RETCODE_NOTOK) {
        FPrintf(stderr, "! Flag setting error before connect\n");
        PError("! TCPConnect::reason");
      }
    }
  }
  else {
    if (!form->anvlIPAddr) {
      appState.tcpSockInfo.anvlIPAddr = 0;
      appState.tcpSockInfo.anvlPort = 0;
    }
    else {
      appState.tcpSockInfo.anvlIPAddr = form->anvlIPAddr;
      appState.tcpSockInfo.anvlPort = (unsigned short int)form->portNum;
      if (OSSockFcntl(appState.tcpSockInfo.connSock,
            FCNTL_APPEND_NONBLOCKING) == MNTCPAPP_API_RETCODE_NOTOK) {
        FPrintf(stderr, "! Flag setting error before connect\n");
        PError("! TCPConnect::reason");
      }
    }
  }

#ifdef LIB_IPV6
  if (gIPVersionForApp == MODE_MNTCPAPP_IPV6){
	connResult = OSSockConnect6(appState.tcpSockInfo.connSock,
								appState.tcpSockInfo.anvlIPv6Addr,
								appState.tcpSockInfo.anvlPort);

  }
  else {
#endif /* LIB_IPV6 */
	connResult = OSSockConnect(appState.tcpSockInfo.connSock,
							   appState.tcpSockInfo.anvlIPAddr,
							   appState.tcpSockInfo.anvlPort);
#ifdef LIB_IPV6
  }
#endif /* LIB_IPV6 */

  if ((connResult == MNTCPAPP_API_RETCODE_NOTOK) && ConnInProgressCheck()) {
	appState.dutState = TCP_STATE_SYNSENT;
	FPrintf(stdout, "TCP Connection is in progress\n");
#ifdef __DEBUG__
	DEBUGStateStrGet(appState.dutState, state);
	FPrintf(stdout, "TCPConnect:DUT reached %s state\n", state);
#endif
  }
  else if (connResult == MNTCPAPP_API_RETCODE_NOTOK) {
	appState.lastErrCode[appState.topCode++] = ErrNo();
#ifdef __DEBUG__
	FPrintf(stdout, "Set last error code to %d\n", ErrNo());
#endif
	FPrintf(stderr, "! Connection error occurred\n");
	PError("! TCPConnect::reason");
	OSSockClose(appState.tcpSockInfo.connSock);
	appState.tcpSockInfo.connSock = -1;
	appState.dutState = TCP_STATE_CLOSED;
	if (appState.tcpListenSock >= 0) {
	  OSSockClose(appState.tcpListenSock);
	  appState.tcpListenSock = -1;
#ifdef __DEBUG__
	  FPrintf(stdout, "Closed TCP listening socket\n");
#endif
	}
    /* RETURN error inspite of any state in listening socket */
	status = 0;
  }
  else {
	FPrintf(stdout, "TCP Connection has been established\n");
	appState.dutState = TCP_STATE_ESTABLISHED;
#ifdef __DEBUG__
	DEBUGStateStrGet(appState.dutState, state);
	FPrintf(stdout, "TCPConnect:DUT reached %s state\n", state);
	FPrintf(stdout, "Set last error code to %d\n", ErrNo());
#endif
  }
  msgLen = STUBBuild(rspForm, buf);
  msgLen += Pack(buf+8, "L", &status);
  if (OSSockSendTo(appState.udpSockInfo.connSock, buf,
				   msgLen, appState.udpSockInfo) ==
	  MNTCPAPP_API_RETCODE_NOTOK ) {
	FPrintf(stderr, "! Sendto failed\n");
	PError("! SendStatusToAnvl::reason");
  }
  if(form) {
    ConnectReqParamFormDestroy(form);
  }
  if(rspForm) {
    STUBFormDestroy(rspForm);
  }

#ifdef __DEBUG__
  FPrintf(stdout, "--->Moving out of TCPConnect\n\n");
#endif
  return(appState.tcpSockInfo.connSock);
}

/*>>

  (int) socket = TCPListen(char* localPort);

  DESCRIPTION:

  This function issues a TCP passive open call. If socket is not
  created then it creates the socket first and then issues a
  nonblocking bind and listen call on this socket. It listens
  on the port number passed as argument to this function.

  ARGS:

  localPort  - Port on which to listen.

  RETURNS:

  int        -  the socket used for the connect call

<<*/
static int
TCPListen(void* response)
{
  ubyte buf[MAX_STR];
  ListenReqParamForm_t *form = (ListenReqParamForm_t *)response;
  STUBForm_t *rspForm = 0;
#ifdef  LIB_IPV6
  int             status = 0;
#endif /* LIB_IPV6 */
  int tmpConnSock = 0; /* Used for case where we need to do listen more than
                          once on the same port */
  unsigned short int localPort = 0;
  unsigned int rspStatus = 1;
  unsigned int msgLen = 0;

  MemSet((void *)buf, '\0', sizeof(buf));
  rspForm = STUBFormCreate();
  rspForm->msgType = RESPONSE;
  rspForm->cmdID = TCP_CMD_LISTEN;
  rspForm->paramsLen = 4;
  localPort = form->portNum;

#ifdef __DEBUG__
  FPrintf(stdout, "---> In TCPListen\n\n");
#endif
/*sankarshan start:179 is TCP port for BGP:TCP MD5 option related:TCP ADVANCED suite*/
  if(localPort == 179 && gConnSock >=0)
  {
	  OSSockClose(gConnSock);
	  appState.tcpSockInfo.connSock = -1;
  }
/*sankarshan end*/

  if (appState.tcpSockInfo.connSock < 0) {
    if ((appState.tcpSockInfo.connSock =
       OSSockSocket(MNTCPAPP_SOCKTYPE_STREAM)) ==
      MNTCPAPP_API_RETCODE_NOTOK) {
      FPrintf(stderr, "! Could not create TCP socket\n");
      PError("! reason");
      return MNTCPAPP_APICALL_ERR;
    }
  }
  else {
    /* Here we listen on a port on which we are already listening.
       Purpose of such listen would be to check whether the DUT
       allows multiple binds to the same socket or throws error when
       attempted. Due to this nature of the call the behaviour of this
       case will be just the opposite of normal case i.e., we shall
       signal SUCCESS if bind fails and FAILURE otherwise
    */
    if ((tmpConnSock =
       OSSockSocket(MNTCPAPP_SOCKTYPE_STREAM)) ==
      MNTCPAPP_API_RETCODE_NOTOK) {
      FPrintf(stderr, "! Could not create second TCP socket\n");
      PError("! reason");
      return MNTCPAPP_APICALL_ERR;
    }

    if (OSSockFcntl(tmpConnSock, FCNTL_APPEND_NONBLOCKING) ==
        MNTCPAPP_API_RETCODE_NOTOK) {
      FPrintf(stderr, "! Flag setting error before listen\n");
      PError("! TCPListen::reason");
      rspStatus = 0; //FAIL
      rspForm->params = &rspStatus;
      //StrCpy(buf, MNTCPAPP_STR_FAIL);
    }

#ifdef LIB_IPV6
    /* Bind it to the local address */
    if (gIPVersionForApp == MODE_MNTCPAPP_IPV6) {
      status = OSSockBind6(tmpConnSock, 0, localPort);
    }
    else {
      status = OSSockBind(tmpConnSock, 0, localPort);
    }

    if (status == MNTCPAPP_API_RETCODE_NOTOK ) {
#else /* LIB_IPV6 */
      if (OSSockBind(tmpConnSock, 0, localPort) ==
          MNTCPAPP_API_RETCODE_NOTOK ) {
#endif /* LIB_IPV6 */
        FPrintf(stderr, "Correctly could not bind the second tcp socket\n"
                        "to the same local address\n");
        PError("reason");
        appState.lastErrCode[appState.topCode++] = ErrNo();
        OSSockClose(tmpConnSock);
        rspStatus = 1; //FAIL
        rspForm->params = &rspStatus;
        //StrCpy(buf, MNTCPAPP_STR_SUCCESS);
#ifndef LIB_IPV6
      }
      else {
        FPrintf(stderr, "! Incorrectly bound the second tcp socket to the\n"
                        "! same local address\n");
        rspStatus = 0; //FAIL
        rspForm->params = &rspStatus;
        //StrCpy(buf, MNTCPAPP_STR_FAIL);
        OSSockClose(tmpConnSock);
      }
#else
    }
    else {
      FPrintf(stderr, "! Incorrectly bound the second tcp socket to the\n"
                      "! same local address\n");
      rspStatus = 0; //FAIL
      rspForm->params = &rspStatus;
      //StrCpy(buf, MNTCPAPP_STR_FAIL);
      OSSockClose(tmpConnSock);
    }
#endif /* !LIB_IPV6 */
    msgLen = STUBBuild(rspForm, buf);
    msgLen += Pack(buf+8, "L", &rspStatus);
    if (OSSockSendTo(appState.udpSockInfo.connSock, buf,
				   msgLen, appState.udpSockInfo) ==
	  MNTCPAPP_API_RETCODE_NOTOK ) {
	FPrintf(stderr, "! Sendto failed\n");
	PError("! SendStatusToAnvl::reason");
    }
    //SendStatusToAnvl(buf);

#ifdef __DEBUG__
    FPrintf(stdout, "---> Moving out of TCPListen\n\n");
#endif
    return appState.tcpSockInfo.connSock;
  }
/*sankarshan start*/
  if(localPort == 179)
  {
	gConnSock =  appState.tcpSockInfo.connSock;
  }
/*sanakarshan end*/
  if (OSSockFcntl(appState.tcpSockInfo.connSock,
				  FCNTL_APPEND_NONBLOCKING) == MNTCPAPP_API_RETCODE_NOTOK) {
    FPrintf(stderr, "! Flag setting error before listen\n");
    PError("! TCPListen::reason");
    rspStatus = 0; //FAIL
    rspForm->params = &rspStatus;
    //StrCpy(buf, MNTCPAPP_STR_FAIL);
  }

  /* Bind it to the local address */
  appState.tcpSockInfo.localPort = localPort;

#ifdef LIB_IPV6
  if (gIPVersionForApp == MODE_MNTCPAPP_IPV6) {
    status = OSSockBind6(appState.tcpSockInfo.connSock, 0,
						 appState.tcpSockInfo.localPort);
  }
  else {
    status = OSSockBind(appState.tcpSockInfo.connSock, 0,
						appState.tcpSockInfo.localPort);
  }
  if (status == MNTCPAPP_API_RETCODE_NOTOK ) {
#else /* LIB_IPV6 */

  if (OSSockBind(appState.tcpSockInfo.connSock, 0,
				 appState.tcpSockInfo.localPort ) ==
	  MNTCPAPP_API_RETCODE_NOTOK ) {
#endif /* LIB_IPV6 */
	FPrintf(stderr, "! Could not bind the tcp socket to the local address\n");
	PError("! reason");
	appState.lastErrCode[appState.topCode++] = ErrNo();
#ifdef __DEBUG__
	FPrintf(stdout, "Set last error code to %d\n", ErrNo());
	FPrintf(stdout, "TCPListen:DUT reached TCP_STATE_CLOSED state\n");
#endif
	OSSockClose(appState.tcpSockInfo.connSock);
	appState.tcpSockInfo.connSock = -1;
	appState.dutState = TCP_STATE_CLOSED;
        rspStatus = 0; //FAIL
        rspForm->params = &rspStatus;
	//StrCpy(buf, MNTCPAPP_STR_FAIL);
  }
  else if (OSSockListen(appState.tcpSockInfo.connSock,
						MNTCPAPP_NO_OF_CLIENT_TO_LISTEN) ==
		   MNTCPAPP_API_RETCODE_NOTOK) {
	appState.lastErrCode[appState.topCode++] = ErrNo();
#ifdef __DEBUG__
	FPrintf(stdout, "Set last error code to %d\n", ErrNo());
#endif
	FPrintf(stderr, "! Listen error occurred\n");
	PError("! reason");
	OSSockClose(appState.tcpSockInfo.connSock);
	appState.tcpSockInfo.connSock = -1;
        rspStatus = 0; //FAIL
        rspForm->params = &rspStatus;
	//StrCpy(buf, MNTCPAPP_STR_FAIL);
  }
  else {
	appState.dutState = TCP_STATE_LISTEN;
        rspStatus = 1; //SUCCESS
        rspForm->params = &rspStatus;
	//StrCpy(buf, MNTCPAPP_STR_SUCCESS);
  }

  msgLen = STUBBuild(rspForm, buf);
  msgLen += Pack(buf+8, "L", &rspStatus);
  if (OSSockSendTo(appState.udpSockInfo.connSock, buf,
				 msgLen, appState.udpSockInfo) ==
	MNTCPAPP_API_RETCODE_NOTOK ) {
    FPrintf(stderr, "! Sendto failed\n");
    PError("! SendStatusToAnvl::reason");
  }
  //SendStatusToAnvl(buf);

#ifdef __DEBUG__
  FPrintf(stdout, "---> Moving out of TCPListen\n\n");
#endif
  if(form) {
    ListenReqParamFormDestroy(form);
  }
  if(rspForm) {
    STUBFormDestroy(rspForm);
  }
  return appState.tcpSockInfo.connSock;
}  /*  TCPListen */

/*>

  (int) socket = TCPSocketCreate();

  DESCRIPTION:

  This function creates a TCP socket (AN INTERNET STREAM SOCKET)

  RETURNS:

  int  -  the created socket

<*/
int
TCPSocketCreate(void)
{
#ifdef __DEBUG__
  char state[MAX_STR];

  MemSet((void *)state, '\0', sizeof(state));
  FPrintf(stdout, "\n--->In TCPSocketCreate\n");
#endif

  if (appState.tcpSockInfo.connSock < 0) {
	if ((appState.tcpSockInfo.connSock =
		 OSSockSocket(MNTCPAPP_SOCKTYPE_STREAM)) ==
		MNTCPAPP_API_RETCODE_NOTOK) {
	  FPrintf(stderr, "! Could not create TCP socket\n");
	  PError("! reason");
	  return MNTCPAPP_APICALL_ERR;
	}
  }
  appState.dutState = SOCKET_CREATED;
#ifdef __DEBUG__
  DEBUGStateStrGet(appState.dutState, state);
  FPrintf(stdout, "TCPSocketCreate:DUT reached %s state\n", state);
  FPrintf(stdout, "--->Moving out of TCPSocketCreate\n\n");
#endif

  return appState.tcpSockInfo.connSock;
}

/*>>

  (int) no_of_byte_sent = TCPSend(char *data, int dataLen, boolean noDelay,
                                  boolean urg, boolean noPush);

   DESCRIPTION:

   This function is used to send data to anvl. The data to be send and the
   length of the data is passed to this function. This function also
   enable / disable Nagle Algo and also can set urgent pointer.

   ARGS:

   data        -  Data to send.
   dataLen     -  Length of the data.
   noDelay     -  Flag to enable disable Nagle Algo.
   urg         -  Flag to mark this data as urgent.
   noPush      -  Flag to disable PUSH.

   RETURNS:

   int         -  no of data bytes sent. In case of error this is
                0 or negative.

<<*/
static int
TCPSend(TCPTestAppCommand_t cmd, char *data, int dataLen, boolean noDelay, boolean urg,
		boolean noPush)
{
  int  val = 0;
  unsigned char buf[MAX_STR];
  unsigned int status = 1, msgLen = 0;
  STUBForm_t *rspForm = 0;

  MemSet((void *)buf, '\0', sizeof(buf));
  rspForm = STUBFormCreate();
  rspForm->msgType = RESPONSE;
  rspForm->cmdID = cmd;
  rspForm->paramsLen = 4;
  rspForm->params = &status;

#ifdef __DEBUG__
  FPrintf(stdout, "\n--->In TCPSend\n");
#endif

  if (noPush) {
	/* we should only set NOPUSH if explicitly asked to do so */
	val = 1;
	if (OSSockSetSockOpt(appState.tcpSockInfo.connSock,
						 SOCKOPT_SET_NOPUSH, val, 0) ==
		MNTCPAPP_API_RETCODE_NOTOK) {
	  FPrintf(stderr, "! setsockopt error occurred\n");
	  PError("! TCPSend::reason");
	}
  }

  if (noDelay) {
	val = 1;
  }
  else {
	val = 0;
  }

  if (!noPush) {
	/* socket error occurs if NOPUSH and NODELAY both set on socket */
	if (OSSockSetSockOpt(appState.tcpSockInfo.connSock,
						 SOCKOPT_SET_NAGGLE_ALGO, val, 0) ==
		MNTCPAPP_API_RETCODE_NOTOK) {
	  FPrintf(stderr, "! setsockopt error occurred\n");
	  PError("! TCPSend::reason");
	}
  }

  if (urg) {

    if (OSSockFcntl(appState.tcpSockInfo.connSock,
					FCNTL_APPEND_NONBLOCKING) == MNTCPAPP_API_RETCODE_NOTOK ) {

	  FPrintf(stderr, "! Flag setting error before send\n");
	  PError("! TCPSend::reason");
	}
	FPrintf(stderr, "Sending urgent data\n");

	if (appState.dutState != TCP_STATE_LISTEN) {
	  val = OSSockSend(appState.tcpSockInfo.connSock, data,
					   (unsigned int)dataLen, TRUE);
	}
	else {
	  val = OSSockSendTo(appState.tcpSockInfo.connSock, data, dataLen,
						 appState.tcpSockInfo);
	}
  }
  else {
	if (OSSockFcntl(appState.tcpSockInfo.connSock,
					FCNTL_APPEND_NONBLOCKING) == MNTCPAPP_API_RETCODE_NOTOK) {
	  FPrintf(stderr, "! Flag setting error before send\n");
	  PError("! TCPSend::reason");
	}
	FPrintf(stderr, "Sending normal data\n");

	if (appState.dutState != TCP_STATE_LISTEN) {
	  val = OSSockSend(appState.tcpSockInfo.connSock, data,
					   (unsigned int)dataLen, FALSE);
	}
	else {
	  val = OSSockSendTo(appState.tcpSockInfo.connSock, data, dataLen,
						 appState.tcpSockInfo);
	}
  }

  if (val == MNTCPAPP_API_RETCODE_NOTOK) {
#ifdef __DEBUG__
	FPrintf(stdout, "Set last error code to %d\n", ErrNo());
	PError("! TCPSend::reason");
#endif
	appState.lastErrCode[appState.topCode++] = ErrNo();
	//StrCpy(buf, MNTCPAPP_STR_FAIL);
	status = 0;
  }
  //else {
	//StrCpy(buf, MNTCPAPP_STR_SUCCESS);
  //}

  msgLen = STUBBuild(rspForm, buf);
  msgLen += Pack(buf+8, "L", (ubyte4 *)rspForm->params);
  if (OSSockSendTo(appState.udpSockInfo.connSock, buf,
				   msgLen, appState.udpSockInfo) ==
	  MNTCPAPP_API_RETCODE_NOTOK ) {
	FPrintf(stderr, "! Sendto failed\n");
	PError("! SendStatusToAnvl::reason");
  }
  if(rspForm) {
    STUBFormDestroy(rspForm);
  }
  //SendStatusToAnvl(buf);

#ifdef __DEBUG__
  FPrintf(stdout, "--->Moving out of TCPSend\n\n");
#endif

  return val;
}

/*>>

  (int) num_of_byte_received = TCPReceive(int expectLen);

  DESCRIPTION:

  This function is used to receive data from anvl. The number of
  bytes to receive is passed as argument to this function. This
  function calls recv (blocking) multiple time each time try to recv
  MAX_MESG_SIZE bytes, till the accumulated received data size is
  greater or equal to expectLen. After receiving the received data
  is kept in the global structure appState.


  ARGS:

  expectLen        -  Num of bytes to read.


  RETURNS:

  int              -  num of data bytes received. In case of error this is
                      0 or negative.

<<*/

int
TCPReceive(int expectLen)
{
  int  ret = 0, numBytes = 0;

#ifdef __DEBUG__
  FPrintf(stdout, "\n--->In TCPReceive\n");
#endif

  if (expectLen > MAX_MESG_SIZE -1 ) {
	FPrintf(stderr, "! Expected data len exceeds buffer size.\n");
	return -1;
  }

  if (OSSockFcntl(appState.tcpSockInfo.connSock,
          FCNTL_APPEND_BLOCKING) == MNTCPAPP_API_RETCODE_NOTOK) {
	FPrintf(stderr, "! Flag setting error before receive\n");
	PError("! TCPSend::reason");
	/* even in case of error continue further */
  }

  tempRecv[0] = '\0';

  /* Receive bytes from ANVL upto expectLen or return -1 */
  do {
#ifdef __DEBUG__
	FPrintf(stdout, "Before a recv call to read %d bytes of data.\n",
			MAX_MESG_SIZE);
#endif
	ret = OSSockRecv(appState.tcpSockInfo.connSock,
					 appState.rcvdBuff, MAX_MESG_SIZE);

	if (ret == MNTCPAPP_API_RETCODE_NOTOK) {
	  FPrintf(stderr, "! Error %d encountered in receive\n", ErrNo());
	  PError("! reason");
	  appState.rcvdBuff[0] = '\0';
	  appState.lastErrCode[appState.topCode++] = ErrNo();
#ifdef __DEBUG__
	  FPrintf(stdout, "Set last error code to %d\n", ErrNo());
	  FPrintf(stdout, "--->Moving out of TCPReceive\n\n");
#endif
	  return -1;
	}
	else if (ret == 0) {
	  FPrintf(stdout, "! No data received\n");
	  return -1;
	}
#ifdef __DEBUG__
	FPrintf(stdout, "After the recv call. Read %d bytes of data.\n", ret);
#endif
	numBytes += ret;
	appState.rcvdBuff[ret] = '\0';
	if (numBytes < MAX_MESG_SIZE) {
	  StrCat(tempRecv, appState.rcvdBuff);
	}
  } while (numBytes < expectLen);

  if (numBytes > expectLen) {
	FPrintf(stderr, "! Data received is more than expected(%d bytes)\n",
			expectLen);

	if (numBytes > MAX_MESG_SIZE) {
	  FPrintf(stderr, "! Data received(%d bytes) is more than ", numBytes);
	  FPrintf(stderr, "available buffer(%d bytes)\n", MAX_MESG_SIZE);
	  FPrintf(stderr, "! ANVL will not be able to get correct data from "
			  "DUT\n");
	}
  }

  if (numBytes <= MAX_MESG_SIZE) {
	StrCpy(appState.rcvdBuff, tempRecv);
  }

#ifdef __DEBUG__
  FPrintf(stdout, "--->Moving out of TCPReceive\n\n");
#endif

  return numBytes;
}

/*>>

  int TCPReceiveLimSz(int expectLen)

  DESCRIPTION:
  This function is used to read data of the exect size given by expectLen.
  It give recv calls repetedly till it reads expectLen bytes of data.


  The difference between TCPReceive and TCPReceiveLimSz is TCPReceive
  tries to read data of size MAX_MESG_SIZE from the TCP receive buffer
  till it reads data greater than or equal to expectLen. But this
  routine reads data of size expectLen only from the TCP receive
  buffer. Thus tcp input buffer gets freed by expectLen only.

  PARAMETERS:
  expectLen   -   Length of data to read.

  RETURNS:
  Size of the data read.

  NOTES:
  expectLen should be less than MAX_MESG_SIZE

<<*/

int
TCPReceiveLimSz(int expectLen)
{
  int ret = 0, numBytes = 0;
  int bytesToRead = 0;

#ifdef __DEBUG__
  FPrintf(stdout, "\n--->In TCPReceiveLimSz\n");
#endif

  if (expectLen > MAX_MESG_SIZE-1) {
	FPrintf(stderr, "! Expected data len exceeds buffer size.\n");
	return -1;
  }

  if (OSSockFcntl(appState.tcpSockInfo.connSock,
          FCNTL_APPEND_BLOCKING) == MNTCPAPP_API_RETCODE_NOTOK) {
	FPrintf(stderr, "! Flag setting error before receive limit size\n");
	PError("! TCPSend::reason");
  }

  tempRecv[0] = '\0';
  bytesToRead = expectLen;

  /* Receive bytes from ANVL up to expectLen or return -1 */
  do {
#ifdef __DEBUG__
	FPrintf(stdout, "Before a recv call to read %d bytes of data.\n",
			bytesToRead);
#endif
	ret = OSSockRecv(appState.tcpSockInfo.connSock, appState.rcvdBuff,
					 (unsigned int)bytesToRead);

	if (ret == MNTCPAPP_API_RETCODE_NOTOK) {
	  FPrintf(stderr, "! Error %d encountered in receive\n", ErrNo());
	  PError("! reason");
	  appState.rcvdBuff[0] = '\0';
	  appState.lastErrCode[appState.topCode++] = ErrNo();
#ifdef __DEBUG__
	  FPrintf(stdout, "Set last error code to %d\n", ErrNo());
#endif
	  return -1;
	}
	else if (ret == 0) {
	  FPrintf(stdout, "! No data received\n");
	  return -1;
	}

#ifdef __DEBUG__
	FPrintf(stdout, "After the recv call. Read %d bytes of data.\n", ret);
#endif
	numBytes += ret;
	appState.rcvdBuff[ret] = '\0';
	StrCat(tempRecv, appState.rcvdBuff);

	if (numBytes < expectLen) {
	  bytesToRead = expectLen - numBytes;
	}
  } while (numBytes < expectLen);

  if (numBytes != expectLen) {
	/* expectLen is always greater than numBytes as we have come out
	   of the while loop */
	FPrintf(stderr, "! Data received is more than expected\n");
	StrCpy(appState.rcvdBuff, tempRecv);
	return -1;
  }

  StrCpy(appState.rcvdBuff, tempRecv);

#ifdef __DEBUG__
  FPrintf(stdout, "--->Moving out of TCPReceiveLimSz\n\n");
#endif

  return numBytes;
}

/*>>

  int TCPReceiveOnce(int expectLen)

  DESCRIPTION:
  It gives recv calls only once to read expectLen bytes of data.

  PARAMETERS:
  expectLen   - Length of data to read.

  RETURNS:
  1           - On success
  <=0         - On error

  NOTES:
  expectLen should be less than MAX_MESG_SIZE

<<*/
int
TCPReceiveOnce(int expectLen)
{
  int ret = 0;
  int retCode = 1;

#ifdef __DEBUG__
  FPrintf(stdout, "\n--->In TCPReceiveOnce\n");
#endif

  if (expectLen > MAX_MESG_SIZE-1) {
	FPrintf(stderr, "! TCPReceiveOnce:Expected data len exceeds buffer "
			"size.\n");
	retCode = -1;
  }
  else if (appState.tcpSockInfo.connSock < 0) {
	FPrintf(stderr, "! TCPReceiveOnce:TCP connected socket not available.\n");
	retCode = -1;
  }

  if (OSSockFcntl(appState.tcpSockInfo.connSock,
				  FCNTL_APPEND_BLOCKING) == MNTCPAPP_API_RETCODE_NOTOK) {
	FPrintf(stderr, "! Flag setting error before receive once\n");
	PError("! TCPReceiveOnce::reason");
	/* continue further if if there is error */
  }

  tempRecv[0] = '\0';

  /* Receive bytes from ANVL up to expectLen or return -1 */
#ifdef __DEBUG__
  FPrintf(stdout, "Before a recv call to read %d bytes of data.\n",
		  expectLen);
#endif
  ret = OSSockRecv(appState.tcpSockInfo.connSock,
				   appState.rcvdBuff, (unsigned int)expectLen);
  if (ret == MNTCPAPP_API_RETCODE_NOTOK) {
	FPrintf(stderr, "! Error %d encountered in receive\n", ErrNo());
	PError("! reason");
	appState.rcvdBuff[0] = '\0';
	appState.lastErrCode[appState.topCode++] = ErrNo();
#ifdef __DEBUG__
    FPrintf(stdout, "Set last error code to %d\n", ErrNo());
#endif
    retCode = MNTCPAPP_APICALL_ERR;
  }
  else if (ret == 0) {
    FPrintf(stdout, "! No data received\n");
    retCode = MNTCPAPP_APICALL_ERR;
  }
  else {
	appState.rcvdBuff[ret] = '\0';
    StrCat(tempRecv, appState.rcvdBuff);
    StrCpy(appState.rcvdBuff, tempRecv);
    if (ret != expectLen) {
	  FPrintf(stderr, "! Data received (%d bytes) is more or less ", ret);
	  FPrintf(stderr, "than expected(%d bytes)\n", expectLen);
	  StrCpy(appState.rcvdBuff, tempRecv);
	  retCode = ret;
    }
  }

#ifdef __DEBUG__
  FPrintf(stdout, "After the recv call. Read %d bytes of data.\n",
		  ret);
  FPrintf(stdout, "--->Moving out of TCPReceiveOnce\n\n");
#endif
  return retCode;
}  /* TCPReceiveOnce */

/*>

  int TCPClose(int StatSend)

  DESCRIPTION:
  This function issues the close call on the connection socket.

  ARGS:
  statSend

  RETURNS:
  1           - On success
  >=0         - On error

<*/
int
TCPClose(int statSend)
{
  int  ret = 0;
  unsigned char buf[MAX_STR];
  STUBForm_t *rspForm = 0;
  unsigned int status = 1;
  unsigned int msgLen = 0;

  MemSet((void *)buf, '\0', sizeof(buf));

#ifdef __DEBUG__
  FPrintf(stdout, "\n--->In TCPClose\n");
#endif
  rspForm = STUBFormCreate();
  rspForm->msgType = RESPONSE;
  rspForm->cmdID = TCP_CMD_SET_STATE;
  rspForm->paramsLen = 4;
  if (appState.tcpSockInfo.connSock >= 0) {
	FPrintf(stdout, "TCP connection is closing\n");

	/*
	  we have only one reference to this connected socket.
	  So calling close will send the FIN immediately
	*/
	ret = OSSockClose(appState.tcpSockInfo.connSock);
	if (ret == MNTCPAPP_API_RETCODE_NOTOK) {
	  appState.lastErrCode[appState.topCode++] = ErrNo();
#ifdef __DEBUG__
	  FPrintf(stdout, "Set last error code to %d\n", ErrNo());
	  PError("! Reason :");
#endif
	}
	appState.tcpSockInfo.connSock = -1;
  }
  else {
	FPrintf(stdout, "TCP connection has already closed down\n");
  }

  if (appState.tcpListenSock >= 0) {
	OSSockClose(appState.tcpListenSock);
	appState.tcpListenSock = -1;
#ifdef __DEBUG__
	FPrintf(stdout, "Closed TCP listening socket\n");
#endif
  }

  /* send a reply to ANVL */
  if (ret == MNTCPAPP_API_RETCODE_NOTOK) {
	status = 0;
	rspForm->params = &status;
	//StrCpy(buf, MNTCPAPP_STR_FAIL);
  }
  else {
	rspForm->params = &status;
	//StrCpy(buf, MNTCPAPP_STR_SUCCESS);
  }

  if (statSend) {
	msgLen = STUBBuild(rspForm, buf);
    msgLen += Pack(buf+8, "L", &status);
	if (OSSockSendTo(appState.udpSockInfo.connSock, buf,
				   msgLen, appState.udpSockInfo) ==
	  MNTCPAPP_API_RETCODE_NOTOK ) {
	FPrintf(stderr, "! Sendto failed\n");
	PError("! SendStatusToAnvl::reason");
  }
	//SendStatusToAnvl(buf);
  }
  else {
    FPrintf(stdout, "No status to send\n");
  }

#ifdef __DEBUG__
  FPrintf(stdout, "--->Moving out of TCPClose\n\n");
#endif
  STUBFormDestroy(rspForm);
  return(appState.lastErrCode[appState.topCode - 1]);
}   /* TCPClose */


/*>

  int OSSockAbort(void);

  DESCRIPTION:
  This function issues the abort call ONLY IF it is implemented in the
  TCP stack being used by the stub.

  RETURNS:
  1           - On success
  >=0         - On error

<*/
int
OSSockAbort(void)
{
  boolean status = TRUE;

#ifdef COMMENT
  /* This function is intentionally kept blank because as of now we do not
     have support in the TCP stack to implement this except for the
     round about way (using SO_LINGER) which is done for some of the
     TCP FSM states in TCPAbort() function.

     When the implementation of TCP abort is available according to the
     RFC rfc793, this function needs to be changed to have the new code
  */
#endif /* COMMENT */

  return status;
}

/*>

  int TCPAbort(void);

  DESCRIPTION:
  This function issues the abort call on the connection socket.

  RETURNS:
  1           - On success
  >=0         - On error

<*/
int
TCPAbort(void)
{
    long int ret = 0;

#ifdef __DEBUG__
  char state[MAX_STR];
  FPrintf(stdout, "\n--->In TCPAbort\n");
#endif

  /* NOTE :
     --------------------------------------------------------------
     We assume that for most of the cases the method of aborting
     TCP connection as cited in    */
  switch (appState.dutState) {
    case TCP_STATE_CLOSING:
    case TCP_STATE_CLOSED:
    case TCP_STATE_FINWAIT1:
    case TCP_STATE_FINWAIT2:
    case TCP_STATE_TIMEWAIT:
      OSSockAbort(); /* Use system specified abort call and this can
                        replace the cases below as well provided support
                        is there to send RST segment, release the TCP and
                        transit to the CLOSED state */
      break;
    case TCP_STATE_LISTEN:
    case TCP_STATE_SYNSENT:
    case TCP_STATE_SYNRCVD:
    case TCP_STATE_ESTABLISHED:
    case TCP_STATE_CLOSEWAIT:
    case TCP_STATE_LASTACK:
    case SOCKET_CREATED:
      if (!(appState.tcpSockInfo.connSock)) {
      FPrintf(stderr, "! Socket is not present. Abort call not issued\n");
      if (appState.tcpListenSock >= 0) {
        OSSockClose(appState.tcpListenSock);
        appState.tcpListenSock = -1;
#ifdef __DEBUG__
        FPrintf(stdout, "Closed TCP listening socket\n");
        FPrintf(stdout, "\n--->Moving out of TCPAbort\n");
#endif
      }
      return -1;
      }

      if (OSSockSetSockOpt(appState.tcpSockInfo.connSock,
                 SOCKOPT_SET_LINGER_TIME, LINGER_ON, 0) ==
        MNTCPAPP_API_RETCODE_NOTOK) {
      FPrintf(stderr, "! setsockopt error occurred\n");
      PError("! reason");
      }

      if ((ret = OSSockClose(appState.tcpSockInfo.connSock)) ==
        MNTCPAPP_API_RETCODE_NOTOK) {
      appState.returnCode = MNTCPAPP_API_RETCODE_NOTOK;
      appState.lastErrCode[appState.topCode++] = ErrNo();
#ifdef __DEBUG__
      FPrintf(stdout, "Set last error code to %d\n", ErrNo());
      DEBUGStateStrGet(appState.dutState, state);
      FPrintf(stdout, "DUT reached %s state\n", state);
#endif
      FPrintf(stderr, "! Aborting error occurred\n");
      PError("! reason");
      return MNTCPAPP_APICALL_ERR;
      }
      appState.returnCode = MNTCPAPP_API_RETCODE_OK;

      appState.tcpSockInfo.connSock = -1;
      if (appState.tcpListenSock >= 0) {
      OSSockClose(appState.tcpListenSock);
      appState.tcpListenSock = -1;
#ifdef __DEBUG__
      FPrintf(stdout, "Closed TCP listening socket\n");
#endif
      }

      appState.dutState = TCP_STATE_CLOSED;
      break;

    default:
      FPrintf(stderr,
            "\n! Operation TCPAbort() not permitted in current state\n");
      break;
  }

#ifdef __DEBUG__
  DEBUGStateStrGet(appState.dutState, state);
  FPrintf(stdout, "DUT reached %s state\n", state);
  FPrintf(stdout, "--->Moving out of TCPAbort\n\n");
#endif

  return 0;
}

/*>>

  (int) no_of_byte_sent = UDPSend(TCPTestAppCommandSend_t cmd, char *data, int dataLen,
                                  MNTCPAppSocketInfo_t sockInfo);

   DESCRIPTION:

   This function is used to send udp data to anvl. The data to be send and the
   length of the data is passed to this function.

   ARGS:

   data        -  Data to send.
   dataLen     -  Length of the data.
   sockInfo    -  MNTCPAppSocketInfo_t structure

   RETURNS:

   int         -  no of data bytes sent. In case of error this is
                0 or negative.

<<*/
static int
UDPSend(TCPTestAppCommand_t cmd, char *data, int dataLen, MNTCPAppSocketInfo_t sockInfo)
{
  int  val = 0;
  int  status = 1, msgLen = 0;
  unsigned char buf[MAX_STR];
  STUBForm_t *rspForm = 0;
  rspForm = STUBFormCreate();
  rspForm->msgType = RESPONSE;
  rspForm->cmdID = cmd;
  rspForm->paramsLen = 4;
  rspForm->params = &status;

  MemSet((void *)buf, '\0', sizeof(buf));

#ifdef __DEBUG__
  FPrintf(stdout, "\n--->In UDPSend\n");
#endif

  /* Close the socket if there is already a valid socket descriptor */
  if (sockInfo.connSock >= 0){
    OSSockClose(sockInfo.connSock);
  }

  if ((sockInfo.connSock = OSSockSocket(MNTCPAPP_SOCKTYPE_DGRAM))
	  == MNTCPAPP_API_RETCODE_NOTOK) {
    FPrintf(stderr, "! Unable to create a UDP socket\n");
    PError("! reason");
    return MNTCPAPP_APICALL_ERR;
  }

#ifdef LIB_IPV6
  if (gIPVersionForApp == MODE_MNTCPAPP_IPV6) {
	status = OSSockBind6(sockInfo.connSock, 0, sockInfo.localPort);
  }
  else {
#endif /* LIB_IPV6 */
	status = OSSockBind(sockInfo.connSock, 0, sockInfo.localPort);
#ifdef LIB_IPV6
  }
#endif /* LIB_IPV6 */

  if (status == MNTCPAPP_API_RETCODE_NOTOK) {
    FPrintf(stdout, "! Unable to bind the UDP socket to any local port\n");
    PError("! reason");
  }
  /* Send UDP data */
  if ((val = OSSockSendTo(sockInfo.connSock, data,
						  dataLen, sockInfo)) ==
	  MNTCPAPP_API_RETCODE_NOTOK) {
    FPrintf(stderr, "! sendto error occurred\n");
    PError("! UDPSend::reason");
    status = 0;
  }

  appState.lastErrCode[appState.topCode++] = ErrNo();

#ifdef __DEBUG__
  if (val == MNTCPAPP_API_RETCODE_NOTOK) {
	FPrintf(stdout, "Set last error code to %d\n", ErrNo());
	PError("! UDPSend::reason");
  }
#endif
  msgLen = STUBBuild(rspForm, buf);
  msgLen += Pack(buf+8, "L", &status);
  if (OSSockSendTo(appState.udpSockInfo.connSock, buf,
				   msgLen, appState.udpSockInfo) ==
	  MNTCPAPP_API_RETCODE_NOTOK ) {
	FPrintf(stderr, "! Sendto failed\n");
	PError("! SendStatusToAnvl::reason");
  }
  if(rspForm) {
    STUBFormDestroy(rspForm);
  }

#ifdef __DEBUG__
  FPrintf(stdout, "--->Moving out of UDPSend\n\n");
#endif

  OSSockClose(sockInfo.connSock);
  return val;
}

/*>

  (ubyte2)sum = IPChecksum(unsigned char *data, unsigned long int len);

  REQUIRES:

  DESCRIPTION:
  Computes the IP checksum for a buffer of data.

  ARGS:
  data pointer to the data to compute the checksum on
  len  how many bytes to compute it for

  RETURNS:
  sum the 16 bit checksum

  SIDE EFFECTS:

<*/
static unsigned short
IPChecksum(unsigned char *data, unsigned long int len)
{
  unsigned long int total;

  total = IPChecksumIncr(data, len, 0);

  /* One's complement */
  total = (~total) & 0xffff;

  return (unsigned short)total;
}

/*>

  (unsigned long int)sum = IPChecksumIncr(unsigned char *data,
                                          unsigned long int len,
                                          unsigned long int total);

  REQUIRES:

  DESCRIPTION:
  Increments the IP checksum for a buffer of data.

  ARGS:
  data  pointer to the data to compute the checksum on
  len   how many bytes to compute it for
  total current checksum total -- will be incremented

  RETURNS:
  total incremented checksum total

  SIDE EFFECTS:

<*/
static unsigned long int
IPChecksumIncr(unsigned char *data, unsigned long int len,
			   unsigned long int total)
{
  unsigned short sum2;
  unsigned char  remainder[2] = {0, 0};
  unsigned long int sum = 0;
  boolean swap = FALSE;

  /*
	The algorithm has a few subtleties:

	(a) we add aligned short words, handling leading or trailing bytes
	    specially.  In particular, if there is a leading unaligned byte
	    then this introduces a byte swap that we have to undo at the end.

	(b) summing is done in host byte order, and the resulting sum is then
	    converted to network byte order.

	See RFC1071 for details.
  */

  /* Align to the first short boundary, remember to swap if we do */
  if ((len > 0) && ((long int)data & 1)) {
	remainder[1] = *data++;
	sum += *((unsigned short*)remainder);
	len--;
	swap = TRUE;
  }

  /* Add up all the full short words. */
  for (; len > 1; len -= 2, data += 2) {
	sum += *(unsigned short*)data;
  }

  /* Add any trailing byte */
  if (len > 0) {
	remainder[0] = *data;
	remainder[1] = 0;
	sum += *((unsigned short*)remainder);
  }

  /* Wrap the overflow around */
  while(sum > 0xffff){
    sum = (sum & 0xffff) + ((sum >> 16) & 0xffff);
  }

  /* swap bytes if we had an odd byte at the front */
  if (swap) {
	sum = ((sum << 8) & 0xff00) | ((sum >> 8) & 0x00ff);
  }

  /* convert to network byte order and add into total */
  sum2 = (unsigned short)sum;
  NetworkByteOrder((unsigned char*)&sum2, (unsigned int)(sizeof(unsigned short)));

  total += sum2;
  while (total > 0xffff) {
	total = (total & 0xffff) + ((total >> 16) & 0xffff);
  }

  return total;
}

void
NetworkByteOrder(unsigned char *data, unsigned int count)
{
  unsigned char tmp;
  unsigned long int i;

  /* Swap the first half of the bytes with the last half */
  for(i = 0; i < count/2; i++){
    tmp = *(data + i);
    *(data + i) = *(data + count - i - 1);
    *(data + count - i - 1) = tmp;
  }
}

/*>>

  void PendingErrorGet(ApplicationState_t* appState )

  REQUIRES:
  Tcp connected socket must be available in ApplicationState. It generates
  a FATAL error message if this socket is not existing.
  It also generate a fatal error message if getsockopt fails.

  DESCRIPTION:
  This function converts an integer to a string.

  ARGS:
  appState        -   The application state data structure

  RETURNS:

<<*/
static void
PendingErrorGet(ApplicationState_t *theAppState)
{
  int                val = 0;
  unsigned char               buf[SMALL_BUF_LEN];
  unsigned int status = 1, msgLen = 0;
  STUBForm_t *rspForm = 0;

#ifdef __DEBUG__
  FPrintf(stdout, "\n--->In PendingErrorGet\n");
#endif
  rspForm = STUBFormCreate();
  rspForm->msgType = RESPONSE;
  rspForm->cmdID = TCP_CMD_ABORT;
  rspForm->paramsLen = 4;
  rspForm->params = &status;

  if (theAppState->tcpSockInfo.connSock == 0) {
	FPrintf(stdout, "!FATAL ERROR: Try to retrive pending error without "
			"connected socket\n");
  }

  if (OSSockGetSockOpt(theAppState->tcpSockInfo.connSock,
					   SOCKOPT_GET_ERROR, &val) ==
	  MNTCPAPP_API_RETCODE_NOTOK) {
	FPrintf(stderr, "! FATAL ERROR: getsockopt error occurred\n");
	PError("! reason");
	status = 0;
  }


#ifdef __DEBUG__
  FPrintf(stdout, "Received pending error = %s\n", buf);
#endif
  msgLen = STUBBuild(rspForm, buf);
  msgLen += Pack(buf+8, "L", &status);
  if (OSSockSendTo(appState.udpSockInfo.connSock, buf,
				 msgLen, appState.udpSockInfo) ==
	    MNTCPAPP_API_RETCODE_NOTOK ) {
	    FPrintf(stderr, "! Sendto failed\n");
	    PError("! SendStatusToAnvl::reason");
  }

   if(rspForm) {
     STUBFormDestroy(rspForm);
   }

#ifdef __DEBUG__
  FPrintf(stdout, "--->Moving out of PendingErrorGet\n\n");
#endif
}


/*>>

  static int DFBitSet(dfBit);

  DESCRIPTION:
  This function instruct tcp/ip stack to set DF bit while sending data packet.

  RETURNS:
  Upon failure return 0
  otherwise value of MSS

<<*/
static int
DFBitSet(int dfBit)
{
  unsigned char     buf[MAX_STR];
  STUBForm_t *rspForm = 0;
  unsigned int msgLen = 0;

  MemSet((void *)buf, '\0', sizeof(buf));
  rspForm = STUBFormCreate();
  rspForm->msgType = RESPONSE;
  rspForm->paramsLen = 4;
  rspForm->params = &dfBit;
  rspForm->cmdID = TCP_CMD_SET_DF_BIT;

#ifdef __DEBUG__
  FPrintf(stdout, "\n--->In DFBitSet\n");
#endif

  if (appState.tcpSockInfo.connSock < 0) {
	FPrintf(stderr, "! TCP socket not created\n");
	return 0;
  }

  if (OSSockSetSockOpt(appState.tcpSockInfo.connSock, SOCKOPT_SET_DFBIT,
                       dfBit, 0) == MNTCPAPP_API_RETCODE_NOTOK) {
	FPrintf(stderr, "! setsockopt error occurred\n");
	PError("! reason");
  }
  /* Send the value of MSS to ANVL */

#ifdef __DEBUG__
  FPrintf(stdout, "Set DF Bit = %s\n", buf);
#endif
  msgLen = STUBBuild(rspForm, buf);
  msgLen += Pack(buf+8, "L", &dfBit);
  if (OSSockSendTo(appState.udpSockInfo.connSock, buf,
				 msgLen, appState.udpSockInfo) ==
	    MNTCPAPP_API_RETCODE_NOTOK ) {
	    FPrintf(stderr, "! Sendto failed\n");
	    PError("! SendStatusToAnvl::reason");
  }

   if(rspForm) {
     STUBFormDestroy(rspForm);
   }

#ifdef __DEBUG__
  FPrintf(stdout, "--->Moving out of DFBitSet\n\n");
#endif

  return dfBit;
}


/*>>

  static int MSSSet(mssVal);

  DESCRIPTION:
  This function sets the Maximum Segment Size on the TCP connection socket.

  RETURNS:
  Upon failure return 0
  otherwise value of MSS

<<*/
static int
MSSSet(int mssVal)
{
  unsigned char     buf[MAX_STR];
  STUBForm_t *rspForm = 0;
  unsigned int msgLen = 0;

  MemSet((void *)buf, '\0', sizeof(buf));
  rspForm = STUBFormCreate();
  rspForm->msgType = RESPONSE;
  rspForm->paramsLen = 4;
  rspForm->params = &mssVal;
  rspForm->cmdID = TCP_CMD_SET_MSS;

#ifdef __DEBUG__
  FPrintf(stdout, "\n--->In MSSSet\n");
#endif

  if (mssVal == 0) {
	FPrintf(stderr, "! MSS value given is 0\n");
	return 0;
  }

  if (appState.tcpSockInfo.connSock < 0) {
	FPrintf(stderr, "! TCP socket not created\n");
	return 0;
  }

  if (OSSockSetSockOpt(appState.tcpSockInfo.connSock, SOCKOPT_SET_MSS,
					   mssVal, 0) == MNTCPAPP_API_RETCODE_NOTOK) {
	FPrintf(stderr, "! setsockopt error occurred\n");
	PError("! reason");
  }
  /* Send the value of MSS to ANVL */

#ifdef __DEBUG__
  FPrintf(stdout, "Set socket MSS = %s\n", buf);
#endif
  msgLen = STUBBuild(rspForm, buf);
  msgLen += Pack(buf+8, "L", &mssVal);
  if (OSSockSendTo(appState.udpSockInfo.connSock, buf,
				 msgLen, appState.udpSockInfo) ==
	    MNTCPAPP_API_RETCODE_NOTOK ) {
	    FPrintf(stderr, "! Sendto failed\n");
	    PError("! SendStatusToAnvl::reason");
  }

   if(rspForm) {
     STUBFormDestroy(rspForm);
   }

#ifdef __DEBUG__
  FPrintf(stdout, "--->Moving out of MSSSet\n\n");
#endif

  return mssVal;
}

/*>>

  static int TTLSet(long int ttlVal);

  DESCRIPTION:
#ifdef LIB_IPV6
  This function sets the Time To Live value on the TCP connection socket
  in IPv4 mode and sets the Hor Limit value on the TCP connection socket
  in IPv6 mode.
#else
  This function sets the Time To Live value on the TCP connection socket.
#endif
  RETURNS:
  Upon failure return 0
  otherwise value of TTL

<<*/
static int
TTLSet(long int ttlVal)
{
  unsigned char  val = (unsigned char)ttlVal;
  unsigned char buf[MAX_STR];
  STUBForm_t *rspForm = 0;
  unsigned int msgLen = 0;

  MemSet((void *)buf, '\0', sizeof(buf));
  rspForm = STUBFormCreate();
  rspForm->msgType = RESPONSE;
  rspForm->paramsLen = 4;
  rspForm->params = SetTTLParamFormCreate();
  ((SetTTLParamForm_t *)rspForm->params)->ttl = val;
  rspForm->cmdID = TCP_CMD_SET_TTL;

#ifdef __DEBUG__
  FPrintf(stdout, "\n--->In TTLSet\n");
#endif

  if (ttlVal == 0) {
	FPrintf(stderr, "! TTLSet:TTL value given is 0\n");
	return 0;
  }

  if (appState.tcpSockInfo.connSock < 0) {
	FPrintf(stderr, "! TTLSet: TCP socket not created\n");
	return 0;
  }

  if (OSSockSetSockOpt(appState.tcpSockInfo.connSock, SOCKOPT_SET_TTL,
					   val, 0) == MNTCPAPP_API_RETCODE_NOTOK) {
	FPrintf(stderr, "! setsockopt error occurred\n");
	PError("! reason");
	FPrintf(stdout, "! TTL value = %d\n", val);
	((SetTTLParamForm_t *)rspForm->params)->ttl = val;
  }

  /* Send the value of TTL to ANVL */
#ifdef __DEBUG__
  FPrintf(stdout, "Set socket TTL = %s\n", buf);
#endif
  msgLen = STUBBuild(rspForm, buf);
  msgLen += SetTTLParamBuild(rspForm->params, buf+8);
  if (OSSockSendTo(appState.udpSockInfo.connSock, buf,
				 msgLen, appState.udpSockInfo) ==
	    MNTCPAPP_API_RETCODE_NOTOK ) {
	    FPrintf(stderr, "! Sendto failed\n");
	    PError("! SendStatusToAnvl::reason");
  }
   if(rspForm->params) {
     SetTTLParamFormDestroy(rspForm->params);
   }
   if(rspForm) {
     STUBFormDestroy(rspForm);
   }

#ifdef __DEBUG__
  FPrintf(stdout, "--->Moving out of TTLSet\n\n");
#endif

  return (int)ttlVal;
}


#ifdef LIB_IPV6
/*>>

  static int HopLimitSet(int hopLimitVal);

  DESCRIPTION:
  This function sets the Hop Limit value on the TCP connection socket
  in IPv6 mode
  RETURNS:
  Upon failure return 0
  otherwise value of HOP LIMIT

<<*/
static int
HopLimitSet(int hopLimitVal)
{
  unsigned char buf[MAX_STR];
  STUBForm_t *rspForm = 0;
  unsigned int msgLen = 0;

  MemSet((void *)buf, '\0', sizeof(buf));
  rspForm = STUBFormCreate();
  rspForm->msgType = RESPONSE;
  rspForm->paramsLen = 4;
  rspForm->params = SetHopLimitParamFormCreate();
  ((SetHopLimitParamForm_t *)rspForm->params)->hopLimit = (unsigned char)hopLimitVal;
  rspForm->cmdID = TCP_CMD_SET_HOPLIMIT;

#ifdef __DEBUG__
  FPrintf(stdout, "\n--->In HopLimitSet\n");
#endif

  if (hopLimitVal == 0) {
	FPrintf(stderr, "! HopLimitSet: HOP LIMIT value given is 0\n");
	return 0;
  }

  if (appState.tcpSockInfo.connSock < 0) {
	FPrintf(stderr, "! HopLimitSet: TCP socket not created\n");
	return 0;
  }

  if (OSSockSetSockOpt(appState.tcpSockInfo.connSock,
					   SOCKOPT_SET_HOPLIMIT, hopLimitVal, 0)
	  == MNTCPAPP_API_RETCODE_NOTOK) {
	FPrintf(stderr, "! setsockopt error occurred\n");
	PError("! reason");
	((SetHopLimitParamForm_t *)rspForm->params)->hopLimit = (unsigned char)hopLimitVal;
  }

  /* Send the value of HOP LIMIT to ANVL */
#ifdef __DEBUG__
  FPrintf(stdout, "Set socket HOP LIMIT = %s\n", buf);
#endif
  msgLen = STUBBuild(rspForm, buf);
  msgLen += SetHopLimitParamBuild(rspForm->params, buf+8);
  if (OSSockSendTo(appState.udpSockInfo.connSock, buf,
				 msgLen, appState.udpSockInfo) ==
	    MNTCPAPP_API_RETCODE_NOTOK ) {
	    FPrintf(stderr, "! Sendto failed\n");
	    PError("! SendStatusToAnvl::reason");
  }
   if(rspForm->params) {
     SetHopLimitParamFormDestroy(rspForm->params);
   }
   if(rspForm) {
     STUBFormDestroy(rspForm);
   }

#ifdef __DEBUG__
  FPrintf(stdout, "--->Moving out of HopLimitSet\n\n");
#endif
  return hopLimitVal;
}
#endif /* LIB_IPV6 */

#ifdef __DEBUG__

/*>>

  static void DEBUGMsgDecode(char* cmdMsg, char* extra);

  DESCRIPTION:
  This function is used for debugging only. It prints the command received
  from ANVL is descriptive format.

  ARGS:
  cmdMsg
  extra

<<*/
static void
DEBUGMsgDecode(char* cmdMsg, char* extra)
{
  boolean       noDelay = FALSE;
  boolean       urg = FALSE;
  boolean       noPush = FALSE;
  char          *anvlAddr = 0;
  char          *anvlTCPPort = 0;
  char          *dutTCPPort = 0;
  char          *data = 0;
  int           dataLen = 0;
  char          *command = 0;
  int           tempState = 0;
  char          *type = 0;
  char          state[MAX_STR];
  int           expectLen = 0;
  int           mssVal = 0;
  int           ttlVal = 0;
#ifdef LIB_IPV6
  int           hopLimitVal;
#endif /* LIB_IPV6 */
  long int      rcvBufLen;

  state[MAX_STR] ='\0';
  FPrintf(stdout, "\n--->Start UDP Control Message decode\n");

  if (extra != NULL) {
	FPrintf(stdout, "%s\n", extra);
  }

  if (cmdMsg == NULL) {
	FPrintf(stdout, "No message to decode\n");
	FPrintf(stdout, "End decode message\n");
	return;
  }


  type = StrTok(cmdMsg, MNTCPAPP_CMD_DELIM);
  if (StrCmp(type, CMD_GET_VERSION_STR) == 0) {
	FPrintf(stdout, "Received GET VERSION command\n");
  }
  else if (StrCmp(type, CMD_CONNECT_STR) == 0) {
	anvlAddr = StrTok(NULL, MNTCPAPP_CMD_DELIM);
	anvlTCPPort = StrTok(NULL, MNTCPAPP_CMD_DELIM);
	FPrintf(stdout, "Received CONNECT command\n");
	FPrintf(stdout, "To connect to %s::%s\n",
			anvlAddr, anvlTCPPort);
  }
  else if (StrCmp(type, CMD_SEND_STR) == 0) {
	FPrintf(stdout, "Received SEND command\n");
	anvlAddr = StrTok(NULL, MNTCPAPP_CMD_DELIM);
	anvlTCPPort = StrTok(NULL, MNTCPAPP_CMD_DELIM);
	FPrintf(stdout, "To send data to %s::%s\n",
			anvlAddr, anvlTCPPort);
	data = StrTok(NULL, MNTCPAPP_CMD_DELIM);

	dataLen = ASCIIToInt(StrTok(NULL, MNTCPAPP_CMD_DELIM));
	FPrintf(stdout, "Data= %s, DataLen=%d\n", data, dataLen);

	noDelay = (boolean)(StrCmp(StrTok(NULL, MNTCPAPP_CMD_DELIM),
							   "NODELAY") ? 0 : 1);
	if (noDelay) {
	  FPrintf(stdout, "With Nagle Algorithm disabled\n");
	}
	else {
	  FPrintf(stdout, "With Nagle Algorithm enabled\n");
	}

	urg = (boolean)(StrCmp(StrTok(NULL, MNTCPAPP_CMD_DELIM), "URG") ? 0 : 1);
	if (urg) {
	  FPrintf(stdout, "With urgent flag on\n\n");
	}
	else {
	  FPrintf(stdout, "With urgent flag off\n");
	}

	noPush = (boolean)(StrCmp(StrTok(NULL, MNTCPAPP_CMD_DELIM),
							  "NOPUSH") ? 0 : 1);
	if (noDelay) {
	  FPrintf(stdout, "With PUSH disabled\n");
	}
	else {
	  FPrintf(stdout, "With PUSH enabled\n");
	}
  }
  else if (StrCmp(type, CMD_SEND_REPEAT_DATA_STR) == 0) {
	FPrintf(stdout, "Received SEND Repeating Data command\n");
	anvlAddr = StrTok(NULL, MNTCPAPP_CMD_DELIM);
	anvlTCPPort = StrTok(NULL, MNTCPAPP_CMD_DELIM);
	FPrintf(stdout, "To send data to %s::%s\n", anvlAddr, anvlTCPPort);
	data = StrTok(NULL, MNTCPAPP_CMD_DELIM);

	dataLen = ASCIIToInt(StrTok(NULL, MNTCPAPP_CMD_DELIM));
	FPrintf(stdout, "Data= %s, DataLen=%d\n", data, dataLen);

	noDelay = (boolean)(StrCmp(StrTok(NULL, MNTCPAPP_CMD_DELIM),
							   "NODELAY") ? 0 : 1);
	if (noDelay) {
	  FPrintf(stdout, "With Nagle Algorithm disabled\n");
	}
	else {
	  FPrintf(stdout, "With Nagle Algorithm enbaled\n");
	}

	urg = (boolean)(StrCmp(StrTok(NULL, MNTCPAPP_CMD_DELIM), "URG") ? 0 : 1);
	if (urg) {
	  FPrintf(stdout, "With urgent flag on\n\n");
	}
	else {
	  FPrintf(stdout, "With urgent flag off\n");
	}
  }
  else if (StrCmp(type, CMD_SEND_UDP_STR) == 0) {
    FPrintf(stdout, "Received SEND UDP Data command\n");
    anvlAddr = StrTok(NULL, MNTCPAPP_CMD_DELIM);
    anvlTCPPort = StrTok(NULL, MNTCPAPP_CMD_DELIM);
    dutTCPPort = StrTok(NULL, MNTCPAPP_CMD_DELIM);
    FPrintf(stdout, "To send data to %s::%s\n", anvlAddr, anvlTCPPort);
    FPrintf(stdout, "from local port :%s\n", dutTCPPort);
    data = StrTok(NULL, MNTCPAPP_CMD_DELIM);

    dataLen = ASCIIToInt(StrTok(NULL, MNTCPAPP_CMD_DELIM));
    FPrintf(stdout, "Data= %s, DataLen=%d\n", data, dataLen);
  }
  else if (StrCmp(type, CMD_SEND_UDP_REPEAT_DATA_STR) == 0) {
    FPrintf(stdout, "Received SEND UDP Repeat Data command\n");
    anvlAddr = StrTok(NULL, MNTCPAPP_CMD_DELIM);
    anvlTCPPort = StrTok(NULL, MNTCPAPP_CMD_DELIM);
    dutTCPPort = StrTok(NULL, MNTCPAPP_CMD_DELIM);
    FPrintf(stdout, "To send data to %s::%s\n", anvlAddr, anvlTCPPort);
    FPrintf(stdout, "from local port :%s\n", dutTCPPort);
    data = StrTok(NULL, MNTCPAPP_CMD_DELIM);

    dataLen = ASCIIToInt(StrTok(NULL, MNTCPAPP_CMD_DELIM));
    FPrintf(stdout, "Data= %s, DataLen=%d\n", data, dataLen);
  }
  else if (StrCmp(type, CMD_RECEIVE_STR) == 0) {

	FPrintf(stdout, "Received RECEIVE command\n");

	command = StrTok(NULL, MNTCPAPP_CMD_DELIM);
	if (command) {
	  expectLen = ASCIIToInt(command);
	}
	else {
	  expectLen = 10;
	}
	FPrintf(stdout, "For %d bytes of data\n", expectLen);
  }
  else if (StrCmp(type, CMD_RECEIVE_LIMSZ_STR) == 0) {
	/* Command to give a limited size receive call */
	FPrintf(stdout, "Received RECEIVE (limited size) command\n");
	command = StrTok(NULL, MNTCPAPP_CMD_DELIM);
	if (command) {
	  expectLen = ASCIIToInt(command);
	}
	else {
	  expectLen = 10;
	}
	FPrintf(stdout, "For %d bytes of data\n", expectLen);
  }
  else if (StrCmp(type, CMD_RECEIVE_ONCE_STR) == 0) {
	/* Command to give recv call only once */
	FPrintf(stdout, "Received RECEIVE (once) command\n");
	command = StrTok(NULL, MNTCPAPP_CMD_DELIM);
	if (command) {
	  expectLen = ASCIIToInt(command);
	}
	else {
	  expectLen = 10;
	}
	FPrintf(stdout, "For %d bytes of data\n", expectLen);
  }
  else if (StrCmp(type, CMD_LISTEN_STR) == 0) {
	FPrintf(stdout, "Received LISTEN command\n");
	dutTCPPort = StrTok(NULL, MNTCPAPP_CMD_DELIM);
	FPrintf(stdout, "To issue a TCP LISTEN call on port %s\n", dutTCPPort);
  }
  else if (StrCmp(type, CMD_CLOSE_STR) == 0) {
	FPrintf(stdout, "Received CLOSE command\n");
  }
  else if (StrCmp(type, CMD_INFORMATION_STR) == 0) {
	FPrintf(stdout, "Received INFORMATION command\n");

	command = StrTok(NULL, MNTCPAPP_CMD_DELIM);


	if (StrCmp(command, "STATE") == 0) {
	  /* Command to set TCP-STATE of this app */
	  tempState = ASCIIToInt(StrTok(NULL, MNTCPAPP_CMD_DELIM));

	  DEBUGStateStrGet(tempState, state);
	  FPrintf(stdout, "To set the state of the app to %s state\n", state);
	}
	else if (StrCmp(command, "ASYNC_RECV") == 0) {
	  /* appState.recvMode = 1; */
	  FPrintf(stdout, "To set the the recv mode to ASYNC_RECV\n");
	}
	else if (StrCmp(command, "NO_ASYNC_RECV") == 0) {
	  /* appState.recvMode = 0; */
	  FPrintf(stdout, "To set the the recv mode to NO_ASYNC_RECV\n");
	}
	else if (StrCmp(command, "GET_SAMPLE_DATA") == 0) {
	  /* Command to retrieve fixed data (through the UDP channel) which
		 is already received through the TCP channel */
	  FPrintf(stdout, "To get the received data\n");
	}
	else if (StrCmp(command, "GETDATA") == 0) {
	  /* Command to retrieve the data (through the UDP channel) which
		 is already received through the TCP channel */
	  FPrintf(stdout, "To get the received data\n");
	}
	else if (StrCmp(command, "GETCODE") == 0) {
	  /* Command to retrieve the last error code. */
	  FPrintf(stdout, "To get the last error code\n");
	}
	else if (StrCmp(command, "GETRETCODE") == 0) {
	  /* Command to retrieve the return code of the last function called. */
	  FPrintf(stdout, "To get the last return code\n");
	}
  }
  else if (StrCmp(type, CMD_REBOOT_STR) == 0) {
	FPrintf(stdout, "Received REBOOT command\n");
  }
  else if (StrCmp(type, CMD_SOCK_CREATE_STR) == 0) {
	FPrintf(stdout, "Received CREATE command\n");
	FPrintf(stdout, "To create a TCP socket\n");
  }
  else if (StrCmp(type, CMD_ABORT_STR) == 0) {
	FPrintf(stdout, "Received ABORT command\n");
  }
  else if (StrCmp(type, CMD_PERROR_STR) == 0) {
	/* Command to send pending error */
	FPrintf(stdout, "Received PENDING ERROR command\n");
	FPrintf(stdout, "To get the pending error\n");
  }
  else if (StrCmp(type, CMD_SETMSS_STR) == 0) {
	/* Command to give a limited size of MSS */
	FPrintf(stdout, "Received SET MSS command\n");
	command = StrTok(NULL, MNTCPAPP_CMD_DELIM);
	if (command) {
	  mssVal = ASCIIToInt(command);
	}
	else {
	  mssVal = MNTCPAPP_DEF_MSS;
	}
	FPrintf(stdout, "To set the value of MSS to %d\n", mssVal);
  }
  else if (StrCmp(type, CMD_SETTTL_STR) == 0) {
	/* Command to set ip TTL on a connected socket */
	FPrintf(stdout, "Received SET TTL command\n");
	command = StrTok(NULL, MNTCPAPP_CMD_DELIM);
	if (command) {
	  ttlVal = ASCIIToInt(command);
	}
	else {
	  ttlVal = DEF_IP_TTL_VAL;
	}
	FPrintf(stdout, "To set the value of TTL to %d\n", ttlVal);
  }
#ifdef LIB_IPV6
  else if (StrCmp(type, CMD_SETHOPLIMIT_STR) == 0) {
	/* Command to set IPv6 HOP LIMIT on a connected socket */
	FPrintf(stdout, "Received SET HOP LIMIT command\n");
	command = StrTok(NULL, MNTCPAPP_CMD_DELIM);
	if (command) {
	  hopLimitVal = ASCIIToInt(command);
	}
	else {
	  hopLimitVal = DEF_IPV6_HOPLIMIT_VAL;
	}
	FPrintf(stdout, "To set the value of HOP LIMIT to %d\n", hopLimitVal);
  }
#endif /* LIB_IPV6 */
  else if (StrCmp(type, CMD_SET_SOCK_RCVBUF_LEN_STR) == 0) {
	/* Command to set socket receive buffer len on a connected socket */
	FPrintf(stdout, "Received SET SOCKET RCEIVE BUFFER LENGTH command\n");
	command = StrTok(NULL, MNTCPAPP_CMD_DELIM);
	if (command) {
	  rcvBufLen = ASCIIToInt(command);
	}
	else {
	  rcvBufLen = DEF_SOCK_RCVBUFF_LEN;
	}
	FPrintf(stdout, "To set the length of socket receive buffer to %ld\n",
			rcvBufLen);
  }
  else if (StrCmp(type, CMD_NOTIFY_TESTSTART_STR) == 0) {
	/* Command to test start notification */
	command = StrTok(NULL, MNTCPAPP_CMD_DELIM);
	FPrintf(stdout, "Received test start notification (%s)\n", cmdMsg);
  }
  else if (StrCmp(type, CMD_NOTIFY_TESTEND_STR) == 0) {
	/* Command to test end notification */
	command = StrTok(NULL, MNTCPAPP_CMD_DELIM);
	FPrintf(stdout, "Received test end notification (%s)\n", cmdMsg);
  }
  else if (StrCmp(type, CMD_API_ACCEPT_STR) == 0) {
	/* Command to issue socket api call accept*/
	command = StrTok(NULL, MNTCPAPP_CMD_DELIM);
	FPrintf(stdout, "Received command to issue socket api call accept\n");
	command = StrTok(NULL, MNTCPAPP_CMD_DELIM);
  }
  else if (StrCmp(type, CMD_URG_CORRECT_STR) == 0) {
	/* Command to set socket option TCP_STDURG */
	command = StrTok(NULL, MNTCPAPP_CMD_DELIM);
	FPrintf(stdout, "Received command to set socket option TCP_STDURG\n");
	FPrintf(stdout, "On setting this option urgent pointer points \n");
	FPrintf(stdout, "to the last byte of urgent data\n");
	command = StrTok(NULL, MNTCPAPP_CMD_DELIM);
  }
  else if (StrCmp(type, CMD_URG_INLINE_STR) == 0) {
	/* Command to set socket option SO_OOBINLINE */
	command = StrTok(NULL, MNTCPAPP_CMD_DELIM);
	FPrintf(stdout, "Received command to set socket option SO_OOBINLINE\n");
	FPrintf(stdout, "On setting this option urgent data byte \n");
	FPrintf(stdout, "will be placed inline with the normal data\n");
	command = StrTok(NULL, MNTCPAPP_CMD_DELIM);
  }
  else if (StrCmp(type, CMD_SHUTDOWN_READ_STR) == 0) {
	/* Command to close the read end of the socket */
	command = StrTok(NULL, MNTCPAPP_CMD_DELIM);
	FPrintf(stdout, "Received command to close read end of the socket\n");
  }
  else if (StrCmp(type, CMD_SHUTDOWN_WRITE_STR) == 0) {
	/* Command to close the write end of the socket */
	command = StrTok(NULL, MNTCPAPP_CMD_DELIM);
	FPrintf(stdout, "Received command to close write end of the socket\n");
  }
  else if (StrCmp(type, CMD_SHUTDOWN_RD_WT_STR) == 0) {
	/* Command to set socket option SO_OOBINLINE */
	command = StrTok(NULL, MNTCPAPP_CMD_DELIM);
	FPrintf(stdout, "Received command to close read and write "
			"end of the socket\n");
  }
  else {
	FPrintf(stdout, "No message decode for this message\n");
	FPrintf(stdout, "type=%s  command =%s\n", type, cmdMsg);
	FPrintf(stdout, "Probably this is a new message for which decode \n");
	FPrintf(stdout, "has not been coded\n");
	FPrintf(stdout, "Modify MessageDecode\n");
  }

  FPrintf(stdout, "--->End UDP Control Message decode\n\n");
  return;
}
#endif

#ifdef __DEBUG__

/*>>

  static void DEBUGStateStrGet(MNTCPAppState_t stateIndx, char *stateStr)

  DESCRIPTION:
  This function is used for debugging only. It translates a TCP state
  into the appropriate string.

  ARGS:
  stateIndx
  stateStr

<<*/
static void
DEBUGStateStrGet(MNTCPAppState_t stateIndx, char *stateStr)
{

  switch (stateIndx){
  case TCP_STATE_CLOSED:
  StrCpy(stateStr, "TCP_STATE_CLOSED");
    break;

  case TCP_STATE_LISTEN:
  StrCpy(stateStr, "TCP_STATE_LISTEN");
    break;

  case TCP_STATE_SYNSENT:
  StrCpy(stateStr, "TCP_STATE_SYNSENT");
    break;

  case TCP_STATE_SYNRCVD:
  StrCpy(stateStr, "TCP_STATE_SYNRCVD");
    break;

  case TCP_STATE_ESTABLISHED:
  StrCpy(stateStr, "TCP_STATE_ESTABLISHED");
    break;

  case TCP_STATE_FINWAIT1:
  StrCpy(stateStr, "TCP_STATE_FINWAIT1");
    break;

  case TCP_STATE_FINWAIT2:
  StrCpy(stateStr, "TCP_STATE_FINWAIT2");
    break;

  case TCP_STATE_CLOSING:
  StrCpy(stateStr, "TCP_STATE_CLOSING");
    break;

  case TCP_STATE_CLOSEWAIT:
  StrCpy(stateStr, "TCP_STATE_CLOSEWAIT");
    break;

  case TCP_STATE_LASTACK:
  StrCpy(stateStr, "TCP_STATE_LASTACK");
    break;

  case TCP_STATE_TIMEWAIT:
  StrCpy(stateStr, "TCP_STATE_TIMEWAIT");
    break;

  case SOCKET_CREATED:
  StrCpy(stateStr, "SOCKET_CREATED");
    break;

  default:
  StrCpy(stateStr, "INVALID");
    break;

  }
  return;
}
#endif

/*>>

  static void
  NotifyTest(boolean start, StartEndTestReqParamForm_t *form)

  DESCRIPTION:
  This function prints out a string to stdout indicating the beginning
  of a TCP test.

  ARGS:
  testNum      NULL terminated string containing the number of the current
               test being run.

<<*/
static void
NotifyTest(boolean start, StartEndTestReqParamForm_t *form)
{
  unsigned char suiteName[MAX_STR];

  MemMove(suiteName, form->suiteName, form->suiteNameLen);
  suiteName[form->suiteNameLen] = '\0';

  FPrintf(stdout, "=========================================\n");
  FPrintf(stdout, "             %s TEST %s %u.%u\n", start ? "START" : "END",
      suiteName, form->majorNum, form->minorNum);
  FPrintf(stdout, "=========================================\n");

  MemSet(appState.lastErrCode, 0, MAX_ERR_COUNT * sizeof(int));
  appState.topCode = 0;
}

/*>>

  static void IssueAccept(void);

  DESCRIPTION:
  This function issues an accept call on the TCP connection socket.

<<*/
static void
IssueAccept(void)
{
  int  temp = 0;
  unsigned char buf[MAX_STR];
  STUBForm_t *rspForm = 0;
  unsigned int status = 1;
  unsigned int tmpLen = 0;
  MemSet((void *)buf, '\0', sizeof(buf));

#ifdef __DEBUG__
  FPrintf(stdout, "--->In IssueAccept\n\n");
#endif

  rspForm = STUBFormCreate();
  rspForm->msgType = RESPONSE;
  rspForm->cmdID = TCP_CMD_API_ACCEPT;
  rspForm->paramsLen = 4;
  MemSet((void *)buf, '\0', sizeof(buf));
  if (OSSockFcntl(appState.tcpSockInfo.connSock,
          FCNTL_APPEND_BLOCKING) == MNTCPAPP_API_RETCODE_NOTOK) {
	FPrintf(stderr, "! Flag setting error before accept\n");
	PError("! IssueAccept::reason");
  }

  temp = OSSockAccept(appState.tcpSockInfo.connSock, &appState.tcpSockInfo);

  if (temp == MNTCPAPP_API_RETCODE_NOTOK) {
	FPrintf(stderr, "! Could not issue accept\n");
	PError("! reason");
	status = 0;
  }
  else {
	FPrintf(stdout, "Issued accept call successfully\n");

#ifdef __DEBUG__
	FPrintf(stdout, "Created TCP connected socket\n");
#endif
  /* Also transit to Established state */
  appState.dutState = TCP_STATE_ESTABLISHED;
	appState.tcpListenSock = appState.tcpSockInfo.connSock;
	appState.tcpSockInfo.connSock = temp;
  }

  /* send status to ANVL */
  rspForm->params = &status;
  tmpLen = STUBBuild(rspForm, buf);
  tmpLen += Pack(buf+8, "L", &status);
  if (OSSockSendTo(appState.udpSockInfo.connSock, buf,
				   tmpLen, appState.udpSockInfo) ==
	  MNTCPAPP_API_RETCODE_NOTOK ) {
	FPrintf(stderr, "! Sendto failed\n");
	PError("! SendStatusToAnvl::reason");
  }
  if(rspForm) {
    STUBFormDestroy(rspForm);
  }

#ifdef __DEBUG__
  FPrintf(stdout, "--->Moving out of IssueAccept\n\n");
#endif

  return;
}

/*>>

  static int OptUrgSet(void);

  DESCRIPTION:
  This function is used to set the TCP socket up to use the Urgent Pointer
  value correction defined in RFC 1122

  RETURNS
  0 to indicate error

<<*/
static int
OptUrgSet(void)
{
  int setVal = 0;
  unsigned char     buf[MAX_STR];
  unsigned int msgLen = 0;
  STUBForm_t *rspForm = 0;

#ifdef __DEBUG__
  FPrintf(stdout, "\n--->In OptUrgSet\n");
#endif
  rspForm = STUBFormCreate();
  rspForm->msgType = RESPONSE;
  rspForm->cmdID = TCP_CMD_OPT_STDURG;
  rspForm->paramsLen = 4;
  rspForm->params = &setVal;
  if (appState.tcpSockInfo.connSock < 0){
	FPrintf(stderr, "! TCP socket not created\n");

#ifdef __DEBUG__
	FPrintf(stdout, "--->Moving out of OptUrgSet\n\n");
#endif

	return 0;
  }

#ifdef __TCP_STDURG__
  if (OSSockSetSockOpt(appState.tcpSockInfo.connSock, SOCKOPT_SET_STDURG,
					   val, 0) == MNTCPAPP_API_RETCODE_NOTOK) {
	FPrintf(stderr, "! setsockopt error occurred\n");
	PError("! reason");
  }
  else if (OSSockGetSockOpt(appState.tcpSockInfo.connSock,
							SOCKOPT_GET_STDURG, &setVal) ==
		   MNTCPAPP_API_RETCODE_NOTOK) {
	FPrintf(stderr, "! FATAL ERROR: getsockopt error occurred\n");
	PError("! reason");
  }
#else
  setVal = 0;
  FPrintf(stderr, "! OptUrgSet: socket option TCP_STDURG not ");
  FPrintf(stderr, "suported by DUT\n");
#endif

  /* get the socket option and send a reply to ANVL */
  if (setVal == 0) {
#ifdef __TCP_STDURG__
	FPrintf(stderr, "! OptUrgSet: could not set TCP_STDURG\n");
#endif
  }
  else {
	FPrintf(stdout, "Set TCP_STDURG\n");
  }
  msgLen = STUBBuild(rspForm, buf);
  msgLen += Pack(buf+8, "L", &setVal);
  if (OSSockSendTo(appState.udpSockInfo.connSock, buf,
				 msgLen, appState.udpSockInfo) ==
	    MNTCPAPP_API_RETCODE_NOTOK ) {
	    FPrintf(stderr, "! Sendto failed\n");
	    PError("! SendStatusToAnvl::reason");
  }

  if(rspForm) {
    STUBFormDestroy(rspForm);
  }


#ifdef __DEBUG__
  FPrintf(stdout, "--->Moving out of OptUrgSet\n\n");
#endif

  return setVal;
}

/*>>

  static int OptUrgInlineSet(void);

  DESCRIPTION:

  RETURNS:
  0 to indicate error

<<*/
static int
OptUrgInlineSet(void)
{
  int val = 1;
  int setVal = 0;
  int setValSize = 0;
  char buf[MAX_STR];

  MemSet((void *)buf, '\0', sizeof(buf));

  setValSize = (int)sizeof(setVal);
#ifdef __DEBUG__
  FPrintf(stdout, "\n--->In OptUrgInlineSet\n");
#endif

  if (appState.tcpSockInfo.connSock < 0){
	FPrintf(stderr, "! TCP socket not created\n");

#ifdef __DEBUG__
	FPrintf(stdout, "--->Moving out of OptUrgInlineSet\n\n");
#endif

	return 0;
  }

  if (OSSockSetSockOpt(appState.tcpSockInfo.connSock,
					   SOCKOPT_SET_OOB_DATA_INLINE, val, 0) ==
	  MNTCPAPP_API_RETCODE_NOTOK) {
	FPrintf(stderr, "! setsockopt error occurred\n");
	PError("! reason");
  }
  else if (OSSockGetSockOpt(appState.tcpSockInfo.connSock,
							SOCKOPT_GET_OOB_DATA_INLINE, &setVal) ==
		   MNTCPAPP_API_RETCODE_NOTOK) {
	FPrintf(stderr, "! FATAL ERROR: getsockopt error occurred\n");
	PError("! reason");
  }
  else {
	/* get the socket option and send a reply to ANVL */

	if (setVal == 0){
	  FPrintf(stderr, "! Could not set SO_OOBINLINE\n");
	  StrCpy(buf, MNTCPAPP_STR_FAIL);
	}
	else {
	  FPrintf(stdout, "Set SO_OOBINLINE\n");
	  StrCpy(buf, MNTCPAPP_STR_SUCCESS);
	}
  }

  SendStatusToAnvl(buf);

#ifdef __DEBUG__
  FPrintf(stdout, "--->Moving out of OptUrgInlineSet\n\n");
#endif

  return setVal;
}

/*>>

  static int SockRecvBuffSet(long int rcvbuflen);

  DESCRIPTION:
  This function sets the receive buffer length of the TCP connection
  socket.

  RETURNS:
  0 to indicate error

<<*/
static int
SockRecvBuffSet(long int rcvbuflen)
{
  unsigned int  val = 0;
  unsigned char buf[MAX_STR];
  unsigned int msgLen = 0;
  STUBForm_t *rspForm = 0;

  MemSet((void *)buf, '\0', sizeof(buf));
  rspForm = STUBFormCreate();
  rspForm->msgType = RESPONSE;
  rspForm->cmdID = TCP_CMD_SET_RCVBUF_LEN;
  rspForm->paramsLen = 4;
  rspForm->params = &val;

#ifdef __DEBUG__
  FPrintf(stdout, "\n--->In SockRecvBuffSet\n");
#endif

  if (rcvbuflen == 0) {
	FPrintf(stderr, "! SockRecvBuffSet: buffer value given is 0\n");
	return 0;
  }

  if (appState.tcpSockInfo.connSock < 0) {
	FPrintf(stderr, "! SockRecvBuffSet: TCP socket not created\n");
	return 0;
  }

  val = (unsigned int)rcvbuflen;

  if (OSSockSetSockOpt(appState.tcpSockInfo.connSock,
					   SOCKOPT_SET_RECV_BUF_SIZE, val, 0) ==
	  MNTCPAPP_API_RETCODE_NOTOK) {
	FPrintf(stderr, "! setsockopt error occurred\n");
	PError("! reason");
  }

  /* Get the value of buffer to check if has been set with proper value */
  if (OSSockGetSockOpt(appState.tcpSockInfo.connSock,
					   SOCKOPT_GET_RECV_BUF, (int *)&val) ==
	  MNTCPAPP_API_RETCODE_NOTOK) {
    FPrintf(stderr, "! FATAL ERROR: getsockopt error occurred\n");
    PError("! reason");
  }
  if (val != (unsigned int)rcvbuflen) {
	FPrintf(stderr, "! SockRecvBuffSet: Unable to set socket recv buffer "
			"of %ld bytes\n", rcvbuflen);
  }

  /* Send the value of buffer to ANVL */

  msgLen = STUBBuild(rspForm, buf);
  msgLen += Pack(buf+8, "L", &val);
  if (OSSockSendTo(appState.udpSockInfo.connSock, buf,
				 msgLen, appState.udpSockInfo) ==
	    MNTCPAPP_API_RETCODE_NOTOK ) {
	    FPrintf(stderr, "! Sendto failed\n");
	    PError("! SendStatusToAnvl::reason");
  }

   if(rspForm) {
     STUBFormDestroy(rspForm);
   }
#ifdef __DEBUG__
  FPrintf(stdout, "--->Moving out of SockRecvBuffSet\n\n");
#endif

  return val;
}

/*>>

  static void SendStatusToAnvl(char *msg);

  DESCRIPTION:
  This function sends a status message over the UDP socket.

  ARGS:
  msg      should be null terminated string

<<*/
static void
SendStatusToAnvl(char *msg)
{
  unsigned char msgCopy[MAX_STR];
  int msgLen = 0;

#ifdef __DEBUG__
  FPrintf(stdout, "Sending [%s] status to ANVL\n", msg);
#endif

  MemSet((void *)msgCopy, '\0', sizeof(msgCopy));

  msgLen = StrLen(msgCopy);
  if (OSSockSendTo(appState.udpSockInfo.connSock, msg,
				   msgLen, appState.udpSockInfo) ==
	  MNTCPAPP_API_RETCODE_NOTOK ) {
	FPrintf(stderr, "! Sendto failed\n");
	PError("! SendStatusToAnvl::reason");
  }
  return;
}

/*>>

  static int StateSet(unsigned int state);

  DESCRIPTION:
  This function prints out the current TCP state to stdout.

  ARGS:
  state      the current TCP state

<<*/
static int
StateSet(unsigned int state)
{
  char stateStr[MAX_STR];
  unsigned char buf[MAX_STR];
  STUBForm_t *rspForm = 0;
  unsigned int status = 1;
  unsigned int msgLen = 0;

  MemSet((void *)stateStr, '\0', sizeof(stateStr));
  rspForm = STUBFormCreate();
  rspForm->msgType = RESPONSE;
  rspForm->cmdID = TCP_CMD_SET_STATE;
  rspForm->paramsLen = 4;
  rspForm->params = &status;
#ifdef COMMENT
  /* +++king add some range checking on the value of state */
#endif /* COMMENT */

#ifdef __DEBUG__
  FPrintf(stdout, "--->In Set State\n\n");
#endif

  if (state == TCP_STATE_CLOSED) {
#ifdef __DEBUG__
	FPrintf(stdout, "Calling TCPClose\n");
#endif
	TCPClose(TRUE);
  }
  else {
    msgLen = STUBBuild(rspForm, buf);
    msgLen += Pack(buf+8, "L", &status);
    if (OSSockSendTo(appState.udpSockInfo.connSock, buf,
				   msgLen, appState.udpSockInfo) ==
	  MNTCPAPP_API_RETCODE_NOTOK ) {
	FPrintf(stderr, "! Sendto failed\n");
	PError("! SendStatusToAnvl::reason");
  }
  //SendStatusToAnvl(MNTCPAPP_STR_SUCCESS);
  }
  appState.dutState = state;

#ifdef __DEBUG__
  DEBUGStateStrGet(appState.dutState, stateStr);
  FPrintf(stdout, "DUT reached %s state\n", stateStr);
#endif


#ifdef __DEBUG__
  FPrintf(stdout, "--->Moving out of Set State\n\n");
#endif
  if(rspForm) {
    STUBFormDestroy(rspForm);
  }
  return 1;
}

/*>

  int ShutDnConn(TCPTestAppCommand_t cmd, ShutDownSock_t type)

  DESCRIPTION:
  This function issues the shutdown call on the connection socket.
  It can shutdown read/write od both end

  ARGS:
  commandID
  statSend

  RETURNS:
  MNTCPAPP_API_RETCODE_OK            - On success
  MNTCPAPP_API_RETCODE_NOTOK         - If API fail
  ERR_INVALID_ARG                    - Invalide type

<*/
static int
ShutDnConn(TCPTestAppCommand_t cmd, ShutDownSock_t type)
{
  int  ret = 0;
  unsigned char buf[MAX_STR];
  STUBForm_t *rspForm = 0;
  unsigned int status = 1, msgLen = 0;

  MemSet((void *)buf, '\0', sizeof(buf));
  rspForm = STUBFormCreate();
  rspForm->msgType = RESPONSE;
  rspForm->paramsLen = 4;
  rspForm->params = &status;
  rspForm->cmdID = cmd;

#ifdef __DEBUG__
  FPrintf(stdout, "\n--->In ShutDnConn\n");
#endif

  if (appState.tcpSockInfo.connSock >= 0) {
	switch (type) {

	case   SHUT_READ:
	  FPrintf(stdout, "Closing read end of the connection\n");
	  break;

	case SHUT_WRITE:
	  FPrintf(stdout, "Closing write end of the connection\n");
	  break;

	case SHUT_RD_WT:
	  FPrintf(stdout, "Closing read and write end of the connection\n");
	  break;

	default :
	  FPrintf(stdout, "! Invalid shutdown type %d\n", type);
	  FPrintf(stdout, "! Shutdown call not issued\n");
	  STUBFormDestroy(rspForm);
	  return ERR_INVALID_ARG;
	  break;

	}

	appState.returnCode =
	  OSSockShutDn(appState.tcpSockInfo.connSock, type);

	ret = appState.returnCode;

	if (appState.returnCode == MNTCPAPP_API_RETCODE_NOTOK) {
	  appState.lastErrCode[appState.topCode++] = ErrNo();
#ifdef __DEBUG__
	  FPrintf(stdout, "Set last error code to %d\n", ErrNo());
#endif
	  FPrintf(stdout, "! Shutdown failed\n");
	}
	else {
	  FPrintf(stdout, "Shutdown successfully\n");
	}
  }
  else {
	FPrintf(stdout, "TCP connection has already closed down\n");
  }

  /* send a reply to ANVL */
  if (ret == MNTCPAPP_API_RETCODE_NOTOK) {
	  status = 0;
  }
  msgLen = STUBBuild(rspForm, buf);
  msgLen += Pack(buf+8, "L", &status);
  if (OSSockSendTo(appState.udpSockInfo.connSock, buf,
				   msgLen, appState.udpSockInfo) ==
	  MNTCPAPP_API_RETCODE_NOTOK ) {
	FPrintf(stderr, "! Sendto failed\n");
	PError("! SendStatusToAnvl::reason");
  }

#ifdef __DEBUG__
  FPrintf(stdout, "--->Moving out of ShutDnConn\n\n");
#endif

  if(rspForm) {
    STUBFormDestroy(rspForm);
  }
  return appState.returnCode;
}   /* ShutDnConn */

/*>>

  static void AppVersionGet(void);

  DESCRIPTION:
  This function notifies ANVL of Stub Application version.

<<*/
static void
AppVersionGet(void)
{
  unsigned char          buf[MAX_STR];
  STUBForm_t	*form = 0;
  unsigned int msgLen = 0;

  MemSet((void *)buf, '\0', sizeof(buf));

#ifdef __DEBUG__
  FPrintf(stdout, "--->In AppVersionGet\n\n");
#endif

  form = STUBFormCreate();
  form->msgType = RESPONSE;
  form->cmdID = TCP_CMD_GET_VERSION;
  form->paramsLen = 4;
  form->params = GetVerRespParamFormCreate();
  ((GetVerRespParamForm_t *)form->params)->majorVer = MNTCPAPP_VERSION_MAJOR;
  ((GetVerRespParamForm_t *)form->params)->minorVer = MNTCPAPP_VERSION_MINOR;
  msgLen = STUBBuild(form, buf);
  msgLen += GetVerRespParamBuild((GetVerRespParamForm_t *)form->params, buf+8);

  /* Send version stamp to ANVL */
  if (OSSockSendTo(appState.udpSockInfo.connSock, buf,
				   msgLen, appState.udpSockInfo) ==
	  MNTCPAPP_API_RETCODE_NOTOK ) {
	FPrintf(stderr, "! Sendto failed\n");
	PError("! SendStatusToAnvl::reason");
  }
  GetVerRespParamFormDestroy(form->params);
  STUBFormDestroy(form);

#ifdef __DEBUG__
  FPrintf(stdout, "--->Moving out of AppVersionGet\n\n");
#endif

  return;
}
