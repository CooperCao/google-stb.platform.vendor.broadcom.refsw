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

#include <stdlib.h>

#include "./include/mntcpos.h"
#include "./include/mntcpapp.h"

/* #define  __TCP_STDURG__ if this socket option is suported */

#define HELP_STR_VERSION  "usage: %s [-p listen-port] [-proxy proxy-server_ip]\n"

static void UDPRequestsLoop(void);
static void AppVersionGet(void);

/* Functions to generate TCP calls as mentioned in RFC 793 */
static int TCPListen(char *localPort);  /* Passive open */
static int TCPConnect(char *anvlAddr, char *anvlTCPPort);  /* active open */
static int TCPSend(char *data, int dataLen, boolean noDelay, boolean urg,
				   boolean noPush);
static int TCPReceive(int expectLen);
static int TCPReceiveLimSz(int expectLen);
static int TCPReceiveOnce(int expectLen);
static int TCPClose(int statSend);
static int TCPAbort(void);

static int OSSockAbort(void);

static int UDPSend(char *data, int dataLen, MNTCPAppSocketInfo_t sockInfo);

/* TCPSocketCreate is not a call described by RFC 793, it actually creates
   a stream socket. */
static int TCPSocketCreate(void);

static int StateSet(long int state);

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
static void NotifyStart(char *testNum);
static void NotifyEnd(char *testNum);

static void SendStatusToAnvl(char *msg);

static unsigned int GetCmdId(char *type, char *command);

static char *IntToASCII(long int num, char *numStr);

static unsigned short IPChecksum(unsigned char *data,
								 unsigned long int len);
static unsigned long int IPChecksumIncr(unsigned char *data,
										unsigned long int len,
										unsigned long int total);

static unsigned int MsgPaddingAdd(char *msg);
static unsigned int MsgPaddingRemove(char *msg);

#ifdef __DEBUG__
static void DEBUGStateStrGet(MNTCPAppState_t stateIndx, char *stateStr);
static void DEBUGMsgDecode(char *cmdMsg, char *extra);
#endif

static int ShutDnConn(ShutDownSock_t type);


/* Global data */
static ApplicationState_t appState;

static char tempRecv[MAX_MESG_SIZE + 1];  /* +1 to add NULL at the end */
#ifdef LIB_IPV6
int gIPVersionForApp; /* IPv4 or IPv6 */
#endif /* LIB_IPV6 */
/*sankarshan start*/
int gConnSock = -1;

int gUseAspSimulator = 0;
#define PROXY_SERVER_IP_BUFFER_SIZE 256
char gAspProxyServerIp[PROXY_SERVER_IP_BUFFER_SIZE] = "\0";

int
getEnvVariableValue(char *pName, unsigned long defaultValue)
{
    int value = defaultValue;
    char *pValue;
    char invalid = '\0';
    char *pInvalid = &invalid;

    value = defaultValue;
    pValue = getenv(pName);
    if (pValue != NULL) {
        value = strtoul(pValue, &pInvalid, 0);
        if (*pInvalid != '\0')
        {
            value = defaultValue;
        }
    }
    FPrintf(stdout,"\n%s: %s = %d\n", __FUNCTION__, pName, value);
    return value;
}

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
  char           portStr[MAX_STR] = "10000"; /* set default udp port to 10000*/
#ifdef LIB_IPV6
   int            modeTCPOK = FALSE;
#endif /* LIB_IPV6 */

  MemSet((void *)temp, '\0', sizeof(temp));

  /* Check for number of arguments passed */

#ifdef LIB_IPV6
  gIPVersionForApp = MODE_MNTCPAPP_IPV4; /* default is IPv4 mode */
#endif /* LIB_IPV6 */

  if (argc > 1) {
	/* -help option */

	if ((StrCmp(argv[1] , "-help") == 0) || (StrCmp(argv[1] , "--h") == 0) ) {
	  FPrintf(stdout, HELP_STR_VERSION ,
			  argv[0]);
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
			FPrintf(stdout, HELP_STR_VERSION,
					argv[0]);
            return MNTCPAPP_CMDLINE_ERR;
		  }
		  else {  /* check if a correct port number is given or not */
			StrCpy(temp, argv[idx + 1]);
			for (count = 0; count < (StrLen(temp)); count++) {
			  if (!IsDigit(temp[count])) {
				FPrintf(stdout, HELP_STR_VERSION, argv[0]);
                return MNTCPAPP_CMDLINE_ERR;
			  }
			}
			StrCpy(portStr, temp);
			portOK = TRUE;
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
#endif
        else if(!StrCmp(argv[idx],"-proxy")) {
            if ((idx + 1) > (argc - 1)) { /* checking wheather the next
										   argument is present or not
										*/
                FPrintf(stdout, HELP_STR_VERSION,
					argv[0]);
                return MNTCPAPP_CMDLINE_ERR;
            }
            else {  /* check if a correct port number is given or not */
                if(StrLen(argv[idx + 1]) > (PROXY_SERVER_IP_BUFFER_SIZE-1))
                {
                    FPrintf(stdout, " gAspProxyServerIp buffer size is not big ennough PROXY_SERVER_IP_BUFFER_SIZE=%d\n", PROXY_SERVER_IP_BUFFER_SIZE);
                    return MNTCPAPP_CMDLINE_ERR;
                }
                else
                {
                    StrCpy(gAspProxyServerIp, argv[idx + 1]);
                    FPrintf(stdout," Proxy server ip is %s \n", gAspProxyServerIp);
                }
                idx+=2;
            }
        }
		else { /* there are some error */
		  FPrintf(stdout, HELP_STR_VERSION, argv[0]);
          return MNTCPAPP_CMDLINE_ERR;
		}
	  } /* while loop ends */
	}
  }

  if(StrLen(gAspProxyServerIp)==0)
  {
      FPrintf(stdout," Proxy server ip is not mentioned , default is 127.0.0.1 \n");
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

  if (portOK != TRUE) {
    appState.udpSockInfo.localPort = MNTCPAPP_UDP_PORT;
  }
  else {
    appState.udpSockInfo.localPort = ASCIIToInt(portStr);
  }


#ifdef LIB_IPV6
  if (gIPVersionForApp == MODE_MNTCPAPP_IPV6) {
	status = OSSockBind6(appState.udpSockInfo.connSock, 0,
						 appState.udpSockInfo.localPort);
  }
  else {
#endif /* LIB_IPV6 */
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
  gUseAspSimulator = getEnvVariableValue("useAspSimulator", 0);
  if(gUseAspSimulator) {
      FPrintf(stdout, "\n================================= ASP Simulator is enabled =================================\n");
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
  boolean       repeatDataSent = FALSE;
  unsigned char attempt = 0;
  char          *anvlAddr = 0;
  char          *anvlTCPPort = 0;
  char          *dutTCPPort = 0;
  char          *data = 0;
  char          *repeatDataBuff = 0;
  char          *command = 0;
  char          *type = 0;
  char          mesg[MAX_MESG_SIZE];
#ifdef __DEBUG__
  char          copyMesg[MAX_MESG_SIZE];
#endif
  char          mesgAck[MAX_MESG_SIZE];
  char          buf[MNTCPAPP_UDP_MIN_DATA_LEN];
  char          tmpBuf[SMALL_BUF_LEN];
  int           numBytes = 0;
  int           ackLen = 0;
  int           expectLen = 0;
  int           dataLen = 0;
  int           tmpLen = 0;
  int           tempState = 0;
  int           mssVal = 0;
  int           ttlVal = 0;
  int           dfBit  = 0;
#ifdef LIB_IPV6
  int           hopLimitVal = 0;
#endif /* LIB_IPV6 */
  int           cmdId = 0;
  int         errorCode = 0;
  unsigned long rcvBufVal = 0;
  unsigned short checksum = 0;
  char          *anvlUDPPort = 0;
  char          *dutUDPPort = 0;
  unsigned char sendAttempts = 5; /* Try sending the packet this times before
                                      giving up */
  MNTCPAppSocketInfo_t udpDataSockInfo;

  MemSet((void *)&udpDataSockInfo, '\0', sizeof(udpDataSockInfo));
  MemSet((void *)mesg, '\0', sizeof(mesg));
#ifdef __DEBUG__
  MemSet((void *)copyMesg, '\0', sizeof(copyMesg));
#endif
  MemSet((void *)mesgAck, '\0', sizeof(mesgAck));
  MemSet((void *)buf, '\0', sizeof(buf));
  MemSet((void *)tmpBuf, '\0', sizeof(tmpBuf));

  for ( ; ; ) {  /* infinite for */

    FPrintf(stdout, "Ready to receive UDP (text) request on port %d\n",
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

	/* remove any padding */
	numBytes -= MsgPaddingRemove(mesg);

#ifdef __DEBUG__
	FPrintf(stdout, "DEBUG --> Received command [%s] from ANVL\n", mesg);
#endif

	/*
	   convert the integer (numBytes => number of received) to a string. This
	   string will be sent via the UDP channel as an ACK of the received UDP
	   command
	*/
	IntToASCII(numBytes, mesgAck);
	ackLen = (int)StrLen(mesgAck);
#ifdef __DEBUG__
	StrCpy(copyMesg, mesg);
	DEBUGMsgDecode(copyMesg, 0);
#else
  FPrintf(stdout, "%s\n", mesg);
#endif
	type = StrTok(mesg, MNTCPAPP_CMD_DELIM);
	command = StrTok(NULL, MNTCPAPP_CMD_DELIM);
  cmdId = (int)GetCmdId(type, command);

	/* send acknowledgement */
	/* first pad the ack if necessary */
	ackLen += MsgPaddingAdd(mesgAck);

#ifdef __DEBUG__
	FPrintf(stdout, "\n--->Start UDP Control Message ACK\n");

	FPrintf(stdout, "Sending %s\n", mesgAck);
#endif
	if (OSSockSendTo(appState.udpSockInfo.connSock, mesgAck,
					 ackLen, appState.udpSockInfo) ==
		MNTCPAPP_API_RETCODE_NOTOK) {
	  FPrintf(stderr, "! sendto error occurred\n");
	  PError("! UDPRequestsLoop::reason");
	  continue;
	}
#ifdef __DEBUG__
  FPrintf(stdout, "--->End UDP Control Message ACK\n\n");
#endif

	switch (cmdId) {
	case TCP_CMD_GET_VERSION:
	  AppVersionGet();
	  break;

	case TCP_CMD_CONNECT:
	  anvlAddr = command;
	  anvlTCPPort = StrTok(NULL, MNTCPAPP_CMD_DELIM);
	  appState.tcpSockInfo.connSock = TCPConnect(anvlAddr, anvlTCPPort);
	  break;

	case TCP_CMD_SEND:
	  anvlAddr = command;
	  anvlTCPPort = StrTok(NULL, MNTCPAPP_CMD_DELIM);
	  data = StrTok(NULL, MNTCPAPP_CMD_DELIM);
	  dataLen = ASCIIToInt(StrTok(NULL, MNTCPAPP_CMD_DELIM));
	  noDelay = (boolean)(StrCmp(StrTok(NULL, MNTCPAPP_CMD_DELIM),
								 MNTCPAPP_STR_NODELAY) ? 0 : 1);
	  urg = (boolean)(StrCmp(StrTok(NULL, MNTCPAPP_CMD_DELIM),
							 MNTCPAPP_STR_URG) ? 0 : 1);
	  noPush = (boolean)(StrCmp(StrTok(NULL, MNTCPAPP_CMD_DELIM),
								MNTCPAPP_STR_NOPUSH) ? 0 : 1);

	  if (appState.dutState == TCP_STATE_LISTEN) {

		/* In LISTEN we won't do a Connect so we have to supply the
		   other side's address info directly */
		if (StrCmp(anvlAddr, "0") == 0) {
		  /*
			When anvl address is 0 we do not want to
			set this socket blocking.
		  */
#ifdef LIB_IPV6
		  if (gIPVersionForApp == MODE_MNTCPAPP_IPV6){
			MemSet(appState.tcpSockInfo.anvlIPv6Addr, 0, 16);
			/* 16 byte is the size of ipv6 address */
		  }
		  else {
#endif /* LIB_IPV6 */
			appState.tcpSockInfo.anvlIPAddr = 0;
#ifdef LIB_IPV6
		  }
#endif /* LIB_IPV6 */
		  appState.tcpSockInfo.anvlPort = 0;
		}
		else {
#ifdef LIB_IPV6
		  if (gIPVersionForApp == MODE_MNTCPAPP_IPV6){
			if (OSIPv6AddrStrToByteArray(anvlAddr,
									  (char *)appState.tcpSockInfo.anvlIPv6Addr)
				== MNTCPAPP_API_RETCODE_NOTOK) {
			  FPrintf(stderr, "! Wrong Address to Connect \n");
			  PError("! inet_pton::reason");
			}
		  }
		  else {
#endif /* LIB_IPV6 */
			appState.tcpSockInfo.anvlIPAddr = OSIPAddrStrToLong(anvlAddr);
#ifdef LIB_IPV6
		  }
#endif /* LIB_IPV6 */
		  appState.tcpSockInfo.anvlPort =
			(unsigned short int)ASCIIToInt(anvlTCPPort);
		  if (OSSockFcntl(appState.tcpSockInfo.connSock,
						  FCNTL_APPEND_NONBLOCKING) ==
			  MNTCPAPP_API_RETCODE_NOTOK) {
			FPrintf(stderr, "! Flag setting error before connect\n");
			PError("! TCPConnect::reason");
		  }
		}
	  }

	  if (!TCPSend(data, dataLen, noDelay, urg, noPush)) {
		FPrintf(stdout, "Data sent\n");
	  }
	  break;

	case TCP_CMD_SEND_REPEAT_DATA:
	  anvlAddr = command;
	  anvlTCPPort = StrTok(NULL, MNTCPAPP_CMD_DELIM);

	  data = StrTok(NULL, MNTCPAPP_CMD_DELIM);
	  dataLen = ASCIIToInt(StrTok(NULL, MNTCPAPP_CMD_DELIM));

		repeatDataBuff = (char *)Malloc((unsigned int)(dataLen + 1));
		MemSet(repeatDataBuff, *data, (size_t)dataLen);
		repeatDataBuff[dataLen] = '\0';

	  noDelay = (boolean)(StrCmp(StrTok(NULL, MNTCPAPP_CMD_DELIM),
								 MNTCPAPP_STR_NODELAY) ? 0 : 1);
	  urg = (boolean)(StrCmp(StrTok(NULL, MNTCPAPP_CMD_DELIM),
							 MNTCPAPP_STR_URG) ? 0 : 1);
	  noPush = (boolean)(StrCmp(StrTok(NULL, MNTCPAPP_CMD_DELIM),
								MNTCPAPP_STR_NOPUSH) ? 0 : 1);

		for (attempt = 0; (attempt < sendAttempts); attempt++) {
		/* Try sending the packet this many times till send is successful and
         between each send attempt wait for 1 sec */
      if (TCPSend(repeatDataBuff, dataLen, noDelay, urg, noPush) >= 0) {
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
	  break;

	case TCP_CMD_RECV:
	  if (command) {
		expectLen = ASCIIToInt(command);
	 }
	  else {
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
	  if (command) {
		expectLen = ASCIIToInt(command);
	  }
	  else {
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
	  if (command) {
		expectLen = ASCIIToInt(command);
	  }
	  else {
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
	  dutTCPPort = command;
	  TCPListen(dutTCPPort);
	  break;

	case TCP_CMD_CLOSE:
	  /* TCP's CLOSE Call corresponds to OS Shutdown Write on socket */
	  ShutDnConn(SHUT_WRITE);
	  break;

	case TCP_CMD_SET_STATE:
	  tempState = ASCIIToInt(StrTok(NULL, MNTCPAPP_CMD_DELIM));
	  StateSet(tempState);
	  break;

	case TCP_CMD_ASYNC_RECV:
	  appState.recvMode = 1;
	  break;

	case TCP_CMD_GET_SAMPLE_DATA:

	  /* Command to retrieve fixed data (through the UDP channel) which
		 is already received through the TCP channel */

	  /* Add padding, if necessary */
	  tmpLen = StrLen(appState.rcvdBuff) + MsgPaddingAdd(appState.rcvdBuff);

	  if (OSSockSendTo(appState.udpSockInfo.connSock, appState.rcvdBuff,
					   tmpLen, appState.udpSockInfo) ==
		  MNTCPAPP_API_RETCODE_NOTOK ) {
		FPrintf(stderr, "! Sendto failed\n");
		PError("! UDPRequestsLoop::reason");
	    }
	  break;

	case TCP_CMD_GET_DATA:
	  /* Command to retrieve the data (through the UDP channel) which
		 is already received through the TCP channel */
	  tmpLen = (int)StrLen(appState.rcvdBuff);

	  checksum = IPChecksum((unsigned char *)appState.rcvdBuff,
							(unsigned long int)tmpLen);

	  IntToASCII((long int)checksum, tmpBuf);

	  IntToASCII(tmpLen, buf);

	  StrCat(buf, MNTCPAPP_CMD_DELIM);

	  StrCat(buf, tmpBuf);

	  StrCat(buf, MNTCPAPP_CMD_DELIM);

#ifdef __DEBUG__
	  FPrintf(stdout, "Returning data length: %u checksum: %u\n",
			  tmpLen, checksum);
#endif

	  /* Add padding, if necessary */
	  tmpLen = StrLen(buf) + MsgPaddingAdd(buf);

	  if (OSSockSendTo(appState.udpSockInfo.connSock, buf,
					   tmpLen, appState.udpSockInfo)  ==
		  MNTCPAPP_API_RETCODE_NOTOK ) {
		FPrintf(stderr, "! Sendto failed\n");
		PError("! UDPRequestsLoop::reason");
	  }
	  break;

	case TCP_CMD_GET_CODE:
	  /* Command to retrieve the last error code. */
	  errorCode = OSErrorCodeToRFCError(appState.lastErrCode[appState.topCode-1]);

	  IntToASCII(errorCode, buf);

	  /* Add padding, if necessary */
	  tmpLen = StrLen(buf) + MsgPaddingAdd(buf);

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
	  break;

	case TCP_CMD_GET_RETCODE:
	  /* Command to retrieve the return code of the last function called. */
	  IntToASCII(appState.returnCode, buf);

	  /* Add padding, if necessary */
	  tmpLen = StrLen(buf) + MsgPaddingAdd(buf);

	  if (OSSockSendTo(appState.udpSockInfo.connSock, buf,
					   tmpLen, appState.udpSockInfo) ==
		  MNTCPAPP_API_RETCODE_NOTOK) {
		FPrintf(stderr, "! Sendto failed\n");
		PError("! UDPRequestsLoop::reason");
	  }
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
	  if (command) {
		dfBit = ASCIIToInt(command);
	  }
	  else {
		dfBit = MNTCPAPP_DEF_DFBIT;
	  }

	  if (DFBitSet(dfBit) == 0) {
		FPrintf(stdout, "! Could not set DF Bit\n");
	  }
	  break;

	case TCP_CMD_SET_MSS:
	  if (command) {
		mssVal = ASCIIToInt(command);
	  }
	  else {
		mssVal = MNTCPAPP_DEF_MSS;
	  }

	  if (MSSSet(mssVal) == 0) {
		FPrintf(stdout, "! Could not set MSS\n");
	  }
	  break;

	case TCP_CMD_SET_TTL:
	  if (command) {
		ttlVal = ASCIIToInt(command);
	  }
	  else {
		ttlVal = DEF_IP_TTL_VAL;
	  }

	  if (TTLSet(ttlVal) == 0) {
		FPrintf(stdout, "! Could not set TTL\n");
	  }
	  break;

#ifdef LIB_IPV6
	case TCP_CMD_SET_HOPLIMIT:
	  if (command) {
		hopLimitVal = ASCIIToInt(command);
	  }
	  else {
		hopLimitVal = DEF_IPV6_HOPLIMIT_VAL;
	  }

	  if (HopLimitSet(hopLimitVal) == 0) {
		FPrintf(stdout, "! Could not set HOP LIMIT\n");
	  }
	  break;
#endif /* LIB_IPV6 */

	case TCP_CMD_SET_RCVBUF_LEN:
	  if (command) {
		rcvBufVal = (unsigned long int)ASCIIToInt(command);
	  }
	  else {
		rcvBufVal = DEF_SOCK_RCVBUFF_LEN;
	  }

	  if (SockRecvBuffSet((long int)rcvBufVal) == 0) {
		FPrintf(stdout, "! Could not set sock recv buffer\n");
	  }
	  break;

	case TCP_CMD_NOTIFY_TEST_START:
	  NotifyStart(command);
	  break;

	case TCP_CMD_NOTIFY_TEST_END:
	  NotifyEnd(command);
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
	  ShutDnConn(SHUT_READ);
	  break;

	case TCP_CMD_SHUTDOWN_WRITE:
	  ShutDnConn(SHUT_WRITE);
	  break;

	case TCP_CMD_SHUTDOWN_RD_WT:
	  ShutDnConn(SHUT_RD_WT);
	  break;

	case UDP_CMD_SEND:
	  anvlAddr = command;
	  anvlUDPPort = StrTok(NULL, MNTCPAPP_CMD_DELIM);
	  dutUDPPort = StrTok(NULL, MNTCPAPP_CMD_DELIM);
	  data = StrTok(NULL, MNTCPAPP_CMD_DELIM);
	  dataLen = ASCIIToInt(StrTok(NULL, MNTCPAPP_CMD_DELIM));
	  MemSet(&udpDataSockInfo, '\0', sizeof(udpDataSockInfo));

	  /* initialize socket */
	  udpDataSockInfo.connSock = -1;

#ifdef LIB_IPV6
	  if (gIPVersionForApp == MODE_MNTCPAPP_IPV6){
		if (OSIPv6AddrStrToByteArray(anvlAddr,
									 (char *)udpDataSockInfo.anvlIPv6Addr)
			== MNTCPAPP_API_RETCODE_NOTOK) {
		  FPrintf(stderr, "! Wrong Address to Send \n");
		  PError("! inet_pton::reason");
		}
	  }
	  else {
#endif /* LIB_IPV6 */
		udpDataSockInfo.anvlIPAddr = OSIPAddrStrToLong(anvlAddr);
#ifdef LIB_IPV6
	  }
#endif /* LIB_IPV6 */

	  udpDataSockInfo.anvlPort = (unsigned short int)ASCIIToInt(anvlUDPPort);
	  udpDataSockInfo.localPort = (unsigned short int)ASCIIToInt(dutUDPPort);

	  if (!UDPSend(data, dataLen, udpDataSockInfo)) {
		FPrintf(stdout, "UDP Data sent\n");
	  }
	  break;

	case UDP_CMD_SEND_REPEAT_DATA:
	  anvlAddr = command;
	  anvlUDPPort = StrTok(NULL, MNTCPAPP_CMD_DELIM);
	  dutUDPPort = StrTok(NULL, MNTCPAPP_CMD_DELIM);
	  data = StrTok(NULL, MNTCPAPP_CMD_DELIM);
	  dataLen = ASCIIToInt(StrTok(NULL, MNTCPAPP_CMD_DELIM));

	  repeatDataBuff = (char *)Malloc((unsigned int)(dataLen + 1));
	  MemSet(repeatDataBuff, *data, (size_t)dataLen);
	  repeatDataBuff[dataLen] = '\0';
	  MemSet(&udpDataSockInfo, '\0', sizeof(udpDataSockInfo));

	  /* initialize socket */
	  udpDataSockInfo.connSock = -1;

#ifdef LIB_IPV6
	  if (gIPVersionForApp == MODE_MNTCPAPP_IPV6){
		if (OSIPv6AddrStrToByteArray(anvlAddr,
									 (char *)udpDataSockInfo.anvlIPv6Addr)
			== MNTCPAPP_API_RETCODE_NOTOK) {
		  FPrintf(stderr, "! Wrong Address to Send \n");
		  PError("! inet_pton::reason");
		}
	  }
	  else {
#endif /* LIB_IPV6 */
		udpDataSockInfo.anvlIPAddr = OSIPAddrStrToLong(anvlAddr);
#ifdef LIB_IPV6
    }
#endif /* LIB_IPV6 */

	  udpDataSockInfo.anvlPort = (unsigned short int)ASCIIToInt(anvlUDPPort);
	  udpDataSockInfo.localPort = (unsigned short int)ASCIIToInt(dutUDPPort);

	  if (!UDPSend(repeatDataBuff, dataLen, udpDataSockInfo)) {
		  FPrintf(stdout, "UDP Repeat Data sent\n");
	  }

	  /* Free allocated buffer */
	  Free(repeatDataBuff);
	  break;

	default:
	  FPrintf(stdout, "Unknown command :: %d\n", cmdId);
	  break;
	} /* end switch */

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
TCPConnect(char *anvlAddr, char *anvlTCPPort)
{
  int              connResult = 0;
  char             buf[MAX_STR];

#ifdef __DEBUG__
  char             state[MAX_STR];
  MemSet((void *)state, '\0', sizeof(state));

  FPrintf(stdout, "\n--->In TCPConnect\n");
#endif

  MemSet((void *)buf, '\0', sizeof(buf));

  TCPSocketCreate();

  StrCpy(buf, MNTCPAPP_STR_SUCCESS);

  /* Connect to the server */
  if (StrCmp(anvlAddr, "0") == 0) {
    /*
      When anvl address is 0 we do not want to
      set this socket blocking.
	*/
#ifdef LIB_IPV6
	if (gIPVersionForApp == MODE_MNTCPAPP_IPV6){
	  MemSet(appState.tcpSockInfo.anvlIPv6Addr, 0, 16);
	  /* 16 byte is the size of ipv6 address */
	}
	else {
#endif /* LIB_IPV6 */
	  appState.tcpSockInfo.anvlIPAddr = 0;
#ifdef LIB_IPV6
	}
#endif /* LIB_IPV6 */
	appState.tcpSockInfo.anvlPort = 0;
  }
  else {
#ifdef LIB_IPV6
	if (gIPVersionForApp == MODE_MNTCPAPP_IPV6){
	  if (OSIPv6AddrStrToByteArray(anvlAddr,
								   (char *)appState.tcpSockInfo.anvlIPv6Addr)
		  == MNTCPAPP_API_RETCODE_NOTOK) {
		FPrintf(stderr, "! Wrong Address to Connect \n");
		PError("! inet_pton::reason");
	  }
	}
	else {
#endif /* LIB_IPV6 */
	  appState.tcpSockInfo.anvlIPAddr = OSIPAddrStrToLong(anvlAddr);
#ifdef LIB_IPV6
	}
#endif /* LIB_IPV6 */
	appState.tcpSockInfo.anvlPort = (unsigned short int)ASCIIToInt(anvlTCPPort);
	if (OSSockFcntl(appState.tcpSockInfo.connSock,
					FCNTL_APPEND_NONBLOCKING) == MNTCPAPP_API_RETCODE_NOTOK) {
	  FPrintf(stderr, "! Flag setting error before connect\n");
	  PError("! TCPConnect::reason");
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
	  StrCpy(buf, MNTCPAPP_STR_FAIL);
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

  SendStatusToAnvl(buf);

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
TCPListen(char* localPort)
{
  char            buf[MAX_STR];
#ifdef  LIB_IPV6
  int             status = 0;
#endif /* LIB_IPV6 */
  int tmpConnSock = 0; /* Used for case where we need to do listen more than
                          once on the same port */

  MemSet((void *)buf, '\0', sizeof(buf));

#ifdef __DEBUG__
  FPrintf(stdout, "---> In TCPListen\n\n");
#endif
/*sankarshan start:179 is TCP port for BGP:TCP MD5 option related:TCP ADVANCED suite*/
  if(ASCIIToInt(localPort) == 179 && gConnSock >=0)
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
      StrCpy(buf, MNTCPAPP_STR_FAIL);
    }

#ifdef LIB_IPV6
    /* Bind it to the local address */
    if (gIPVersionForApp == MODE_MNTCPAPP_IPV6) {
      status = OSSockBind6(tmpConnSock, 0,
                           (unsigned short int)ASCIIToInt(localPort));
    }
    else {
      status = OSSockBind(tmpConnSock, 0,
                          (unsigned short int)ASCIIToInt(localPort));
    }

    if (status == MNTCPAPP_API_RETCODE_NOTOK ) {
#else /* LIB_IPV6 */
      if (OSSockBind(tmpConnSock, 0,
                     (unsigned short int)ASCIIToInt(localPort)) ==
          MNTCPAPP_API_RETCODE_NOTOK ) {
#endif /* LIB_IPV6 */
        FPrintf(stderr, "Correctly could not bind the second tcp socket\n"
                        "to the same local address\n");
        PError("reason");
        appState.lastErrCode[appState.topCode++] = ErrNo();
        OSSockClose(tmpConnSock);
        StrCpy(buf, MNTCPAPP_STR_SUCCESS);
#ifndef LIB_IPV6
      }
      else {
        FPrintf(stderr, "! Incorrectly bound the second tcp socket to the\n"
                        "! same local address\n");
        StrCpy(buf, MNTCPAPP_STR_FAIL);
        OSSockClose(tmpConnSock);
      }
#else
    }
    else {
      FPrintf(stderr, "! Incorrectly bound the second tcp socket to the\n"
                      "! same local address\n");
      StrCpy(buf, MNTCPAPP_STR_FAIL);
      OSSockClose(tmpConnSock);
    }
#endif /* !LIB_IPV6 */

    SendStatusToAnvl(buf);

#ifdef __DEBUG__
    FPrintf(stdout, "---> Moving out of TCPListen\n\n");
#endif
    return appState.tcpSockInfo.connSock;
  }
/*sankarshan start*/
  if(ASCIIToInt(localPort) == 179)
  {
	gConnSock =  appState.tcpSockInfo.connSock;
  }
/*sanakarshan end*/
  if (OSSockFcntl(appState.tcpSockInfo.connSock,
				  FCNTL_APPEND_NONBLOCKING) == MNTCPAPP_API_RETCODE_NOTOK) {
    FPrintf(stderr, "! Flag setting error before listen\n");
    PError("! TCPListen::reason");
    StrCpy(buf, MNTCPAPP_STR_FAIL);
  }

  /* Bind it to the local address */
  appState.tcpSockInfo.localPort = (unsigned short int)ASCIIToInt(localPort);

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
	StrCpy(buf, MNTCPAPP_STR_FAIL);
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
	StrCpy(buf, MNTCPAPP_STR_FAIL);
  }
  else {
	appState.dutState = TCP_STATE_LISTEN;
	StrCpy(buf, MNTCPAPP_STR_SUCCESS);
  }

  SendStatusToAnvl(buf);

#ifdef __DEBUG__
  FPrintf(stdout, "---> Moving out of TCPListen\n\n");
#endif
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
TCPSend(char *data, int dataLen, boolean noDelay, boolean urg,
		boolean noPush)
{
  int  val = 0;
  char buf[MAX_STR];

  MemSet((void *)buf, '\0', sizeof(buf));

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
    FPrintf(stdout, "%s: *****************************> Set last error code to %d\n",__FUNCTION__, ErrNo());
	PError("! TCPSend::reason");

	appState.lastErrCode[appState.topCode++] = ErrNo();
	StrCpy(buf, MNTCPAPP_STR_FAIL);
  }
  else {
	StrCpy(buf, MNTCPAPP_STR_SUCCESS);
  }

  SendStatusToAnvl(buf);

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
  char buf[MAX_STR];

  MemSet((void *)buf, '\0', sizeof(buf));

#ifdef __DEBUG__
  FPrintf(stdout, "\n--->In TCPClose\n");
#endif

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
	StrCpy(buf, MNTCPAPP_STR_FAIL);
  }
  else {
	StrCpy(buf, MNTCPAPP_STR_SUCCESS);
  }

  if (statSend) {
	SendStatusToAnvl(buf);
  }
  else {
    FPrintf(stdout, "No status to send\n");
  }

#ifdef __DEBUG__
  FPrintf(stdout, "--->Moving out of TCPClose\n\n");
#endif

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

  (int) no_of_byte_sent = UDPSend(char *data, int dataLen,
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
UDPSend(char *data, int dataLen, MNTCPAppSocketInfo_t sockInfo)
{
  int  val = 0;
  int  status = 0;
  char buf[MAX_STR];

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
  }

  appState.lastErrCode[appState.topCode++] = ErrNo();
  StrCpy(buf, MNTCPAPP_STR_SUCCESS);

#ifdef __DEBUG__
  if (val == MNTCPAPP_API_RETCODE_NOTOK) {
	FPrintf(stdout, "Set last error code to %d\n", ErrNo());
	PError("! UDPSend::reason");
  }
#endif
  SendStatusToAnvl(buf);

#ifdef __DEBUG__
  FPrintf(stdout, "--->Moving out of UDPSend\n\n");
#endif

  OSSockClose(sockInfo.connSock);
  return val;
}

/*>>

  char *IntToASCII(long int num, char *numStr)

  REQUIRES:

  DESCRIPTION:
  This function converts an integer to a string.

  ARGS:
  num       -   The integer to convert
  numStr    -   String to hold the number

  RETURNS:
  The converted string

<<*/
static char
*IntToASCII(long int num, char *numStr)
{
  char         tempStr[MAX_STR];
  long int     count = 0;

  if (num == 0) {
	numStr[count++] = '0';
	numStr[count] = '\0';

	return numStr;
  }

  while (num) {
	tempStr[count++] = (char)(num % 10 + '0');
	num = num / 10;
  }

  while (count) {
	numStr[num++] = tempStr[--count];
  }

  numStr[num] = '\0';

  return numStr;
}  /* IntToASCII */


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
  char               buf[SMALL_BUF_LEN];

#ifdef __DEBUG__
  FPrintf(stdout, "\n--->In PendingErrorGet\n");
#endif

  if (theAppState->tcpSockInfo.connSock == 0) {
	FPrintf(stdout, "!FATAL ERROR: Try to retrive pending error without "
			"connected socket\n");
  }

  if (OSSockGetSockOpt(theAppState->tcpSockInfo.connSock,
					   SOCKOPT_GET_ERROR, &val) ==
	  MNTCPAPP_API_RETCODE_NOTOK) {
	FPrintf(stderr, "! FATAL ERROR: getsockopt error occurred\n");
	PError("! reason");
  }

  IntToASCII(val, buf);

#ifdef __DEBUG__
  FPrintf(stdout, "Received pending error = %s\n", buf);
#endif

  SendStatusToAnvl(buf);

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
  char     buf[SMALL_BUF_LEN];

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

  IntToASCII(dfBit, buf);
#ifdef __DEBUG__
  FPrintf(stdout, "Set DF Bit = %s\n", buf);
#endif

  SendStatusToAnvl(buf);

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
  char     buf[SMALL_BUF_LEN];

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

  IntToASCII(mssVal, buf);
#ifdef __DEBUG__
  FPrintf(stdout, "Set socket MSS = %s\n", buf);
#endif

  SendStatusToAnvl(buf);

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
  int  val = (int)ttlVal;
  char buf[SMALL_BUF_LEN];

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
  }

  /* Send the value of TTL to ANVL */
  IntToASCII(val, buf);
#ifdef __DEBUG__
  FPrintf(stdout, "Set socket TTL = %s\n", buf);
#endif

  SendStatusToAnvl(buf);

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
  char buf[SMALL_BUF_LEN];

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
  }

  /* Send the value of HOP LIMIT to ANVL */
  IntToASCII(hopLimitVal, buf);
#ifdef __DEBUG__
  FPrintf(stdout, "Set socket HOP LIMIT = %s\n", buf);
#endif

  SendStatusToAnvl(buf);

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

  static void NotifyStart(char* testNum);

  DESCRIPTION:
  This function prints out a string to stdout indicating the beginning
  of a TCP test.

  ARGS:
  testNum      NULL terminated string containing the number of the current
               test being run.

<<*/
static void
NotifyStart(char* testNum)
{
  FPrintf(stdout, "=========================================\n");
  FPrintf(stdout, "             START TEST %s\n", testNum);
  FPrintf(stdout, "=========================================\n\n");
  MemSet(appState.lastErrCode, 0, MAX_ERR_COUNT * sizeof(int));
  appState.topCode = 0;
}

/*>>

  static void NotifyEnd(char* testNum);

  DESCRIPTION:
  This function prints out a string to stdout indicating the completion
  of a TCP test.

  ARGS:
  testNum      NULL terminated string containing the number of the current
               test being run.

<<*/
static void
NotifyEnd(char* testNum)
{
  appState.tcpSockInfo.connSock = -1;
  FPrintf(stdout, "\n\n=========================================\n");
  FPrintf(stdout, "              END TEST %s\n", testNum);
  FPrintf(stdout, "=========================================\n\n");
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
  char buf[MAX_STR];

  MemSet((void *)buf, '\0', sizeof(buf));

#ifdef __DEBUG__
  FPrintf(stdout, "--->In IssueAccept\n\n");
#endif

  if (OSSockFcntl(appState.tcpSockInfo.connSock,
          FCNTL_APPEND_BLOCKING) == MNTCPAPP_API_RETCODE_NOTOK) {
	FPrintf(stderr, "! Flag setting error before accept\n");
	PError("! IssueAccept::reason");
  }

  temp = OSSockAccept(appState.tcpSockInfo.connSock, &appState.tcpSockInfo);

  if (temp == MNTCPAPP_API_RETCODE_NOTOK) {
	FPrintf(stderr, "! Could not issue accept\n");
	PError("! reason");
	StrCpy(buf, MNTCPAPP_STR_FAIL);
  }
  else {
	StrCpy(buf, MNTCPAPP_STR_SUCCESS);
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
  SendStatusToAnvl(buf);


#ifdef __DEBUG__
  FPrintf(stdout, "--->Moving out of IssueAccept\n\n");
#endif

  return;
}

/*>>

  static unsigned int GetCmdId(char *type, char *command)

  DESCRIPTION:
  This function is used to get the command id from command string

  ARGS:
  type
  command

<<*/
static unsigned int
GetCmdId(char *type, char *command)
{
  unsigned int retVal = 0;

  if (!type) {
    return TCP_CMD_INVALID;
  }

  if (StrCmp(type, CMD_GET_VERSION_STR) == 0) {
	retVal = TCP_CMD_GET_VERSION;
  }
  else if (StrCmp(type, CMD_CONNECT_STR) == 0) {
	retVal = TCP_CMD_CONNECT;
  }
  else if (StrCmp(type, CMD_SEND_STR) == 0) {
	retVal = TCP_CMD_SEND;
  }
  else if (StrCmp(type, CMD_SEND_REPEAT_DATA_STR) == 0) {
	retVal = TCP_CMD_SEND_REPEAT_DATA;
  }
  else if (StrCmp(type, CMD_RECEIVE_STR) == 0) {
	retVal = TCP_CMD_RECV;
  }
  else if (StrCmp(type, CMD_RECEIVE_LIMSZ_STR) == 0) {
	retVal = TCP_CMD_RECV_LIMSZ;
  }
  else if (StrCmp(type, CMD_RECEIVE_ONCE_STR) == 0) {
	retVal = TCP_CMD_RECV_ONCE;
  }
  else if (StrCmp(type, CMD_LISTEN_STR) == 0) {
	retVal = TCP_CMD_LISTEN;
  }
  else if (StrCmp(type, CMD_CLOSE_STR) == 0) {
	retVal = TCP_CMD_CLOSE;
  }
  else if (StrCmp(type, CMD_INFORMATION_STR) == 0) {

	if (StrCmp(command, "STATE") == 0) {
	  retVal = TCP_CMD_SET_STATE;
	}
	else if (StrCmp(command, "ASYNC_RECV") == 0) {
	  retVal = TCP_CMD_ASYNC_RECV;
	}
	else if (StrCmp(command, "GET_SAMPLE_DATA") == 0) {
	  retVal = TCP_CMD_GET_SAMPLE_DATA;
	}
	else if (StrCmp(command, "GETDATA") == 0) {
	  retVal = TCP_CMD_GET_DATA;
	}
	else if (StrCmp(command, "GETCODE") == 0) {
	  retVal = TCP_CMD_GET_CODE;
	}
	else if (StrCmp(command, "GETRETCODE") == 0) {
	  retVal = TCP_CMD_GET_RETCODE;
	}
  }
  else if (StrCmp(type, CMD_REBOOT_STR) == 0) {
	retVal = TCP_CMD_DUT_REBOOT;
  }
  else if (StrCmp(type, CMD_SOCK_CREATE_STR) == 0) {
	retVal = TCP_CMD_SOCK_CREATE;
  }
  else if (StrCmp(type, CMD_ABORT_STR) == 0) {
	retVal = TCP_CMD_ABORT;
  }
  else if (StrCmp(type, CMD_PERROR_STR) == 0) {
	retVal = TCP_CMD_GET_PERROR;
  }
  else if (StrCmp(type, CMD_SETDFBIT_STR) == 0) {
	retVal = TCP_CMD_SET_DF_BIT;
  }
  else if (StrCmp(type, CMD_SETMSS_STR) == 0) {
	retVal = TCP_CMD_SET_MSS;
  }
  else if (StrCmp(type, CMD_SETTTL_STR) == 0) {
	retVal = TCP_CMD_SET_TTL;
  }
#ifdef LIB_IPV6
  else if (StrCmp(type, CMD_SETHOPLIMIT_STR) == 0) {
	retVal = TCP_CMD_SET_HOPLIMIT;
  }
#endif /* LIB_IPV6 */
  else if (StrCmp(type, CMD_SET_SOCK_RCVBUF_LEN_STR) == 0) {
	retVal = TCP_CMD_SET_RCVBUF_LEN;
  }
  else if (StrCmp(type, CMD_NOTIFY_TESTSTART_STR) == 0) {
	retVal = TCP_CMD_NOTIFY_TEST_START;
  }
  else if (StrCmp(type, CMD_NOTIFY_TESTEND_STR) == 0) {
	retVal = TCP_CMD_NOTIFY_TEST_END;
  }
  else if (StrCmp(type, CMD_API_ACCEPT_STR) == 0) {
	retVal = TCP_CMD_API_ACCEPT;
  }
  else if (StrCmp(type, CMD_URG_CORRECT_STR) == 0) {
	retVal = TCP_CMD_OPT_STDURG;
  }
  else if (StrCmp(type, CMD_URG_INLINE_STR) == 0) {
	retVal = TCP_CMD_OPT_URGINLINE;
  }
  else if (StrCmp(type, CMD_SHUTDOWN_READ_STR) == 0) {
	retVal = TCP_CMD_SHUTDOWN_READ;
  }
  else if (StrCmp(type, CMD_SHUTDOWN_WRITE_STR) == 0) {
	retVal = TCP_CMD_SHUTDOWN_WRITE;
  }
  else if (StrCmp(type, CMD_SHUTDOWN_RD_WT_STR) == 0) {
	retVal =   TCP_CMD_SHUTDOWN_RD_WT;
  }
  else if (StrCmp(type, CMD_SEND_UDP_STR) == 0) {
    retVal = UDP_CMD_SEND;
  }
  else if (StrCmp(type, CMD_SEND_UDP_REPEAT_DATA_STR) == 0) {
    retVal = UDP_CMD_SEND_REPEAT_DATA;
  }
  else {
	retVal = TCP_CMD_INVALID;
  }
  return retVal;
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
  char     buf[MAX_STR];

#ifdef __DEBUG__
  FPrintf(stdout, "\n--->In OptUrgSet\n");
#endif

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
	StrCpy(buf, MNTCPAPP_STR_FAIL);
  }
  else {
	FPrintf(stdout, "Set TCP_STDURG\n");
	StrCpy(buf, MNTCPAPP_STR_SUCCESS);
  }

  SendStatusToAnvl(buf);

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
  int  val = 0;
  char buf[SMALL_BUF_LEN];

  MemSet((void *)buf, '\0', sizeof(buf));

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

  val = (int)rcvbuflen;

  if (OSSockSetSockOpt(appState.tcpSockInfo.connSock,
					   SOCKOPT_SET_RECV_BUF_SIZE, val, 0) ==
	  MNTCPAPP_API_RETCODE_NOTOK) {
	FPrintf(stderr, "! setsockopt error occurred\n");
	PError("! reason");
  }

  /* Get the value of buffer to check if has been set with proper value */
  if (OSSockGetSockOpt(appState.tcpSockInfo.connSock,
					   SOCKOPT_GET_RECV_BUF, &val) ==
	  MNTCPAPP_API_RETCODE_NOTOK) {
    FPrintf(stderr, "! FATAL ERROR: getsockopt error occurred\n");
    PError("! reason");
  }
  if (val != (int)rcvbuflen) {
	FPrintf(stderr, "! SockRecvBuffSet: Unable to set socket recv buffer "
			"of %ld bytes\n", rcvbuflen);
  }

  /* Send the value of buffer to ANVL */
  IntToASCII(val, buf);


  SendStatusToAnvl(buf);
#ifdef __DEBUG__
  FPrintf(stdout, "Set socket receive buffer size = %s\n", buf);
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
  char msgCopy[MAX_STR];
  int msgLen = 0;

#ifdef __DEBUG__
  FPrintf(stdout, "Sending [%s] status to ANVL\n", msg);
#endif

  MemSet((void *)msgCopy, '\0', sizeof(msgCopy));

  StrCpy(msgCopy, msg);
  /* add padding, if necessary */
  msgLen = StrLen(msgCopy) + MsgPaddingAdd(msgCopy);
  if (OSSockSendTo(appState.udpSockInfo.connSock, msgCopy,
				   msgLen, appState.udpSockInfo) ==
	  MNTCPAPP_API_RETCODE_NOTOK ) {
	FPrintf(stderr, "! Sendto failed\n");
	PError("! SendStatusToAnvl::reason");
  }
  return;
}

/*>>

  static int StateSet(long int state);

  DESCRIPTION:
  This function prints out the current TCP state to stdout.

  ARGS:
  state      the current TCP state

<<*/
static int
StateSet(long int state)
{
  char stateStr[MAX_STR];

  MemSet((void *)stateStr, '\0', sizeof(stateStr));

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
	TCPClose(FALSE);
  }
  appState.dutState = state;

#ifdef __DEBUG__
  DEBUGStateStrGet(appState.dutState, stateStr);
  FPrintf(stdout, "DUT reached %s state\n", stateStr);
#endif

  SendStatusToAnvl(MNTCPAPP_STR_SUCCESS);

#ifdef __DEBUG__
  FPrintf(stdout, "--->Moving out of Set State\n\n");
#endif

  return 1;
}

/*>

  int ShutDnConn(ShutDownSock_t type)

  DESCRIPTION:
  This function issues the shutdown call on the connection socket.
  It can shutdown read/write od both end

  ARGS:
  statSend

  RETURNS:
  MNTCPAPP_API_RETCODE_OK            - On success
  MNTCPAPP_API_RETCODE_NOTOK         - If API fail
  ERR_INVALID_ARG                    - Invalide type

<*/
static int
ShutDnConn(ShutDownSock_t type)
{
  int  ret = 0;
  char buf[MAX_STR];

  MemSet((void *)buf, '\0', sizeof(buf));

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
	StrCpy(buf, MNTCPAPP_STR_FAIL);
  }
  else {
	StrCpy(buf, MNTCPAPP_STR_SUCCESS);
  }

  SendStatusToAnvl(buf);

#ifdef __DEBUG__
  FPrintf(stdout, "--->Moving out of ShutDnConn\n\n");
#endif

  return appState.returnCode;
}   /* ShutDnConn */

/*>>

  static unsigned int MsgPaddingAdd(char *msg);

  DESCRIPTION:
  This function adds padding to the msg so that the length of the data
  is big enough to prevent the message from getting stuck in buffers
  for being "too small". See note for MNTCPAPP_UDP_MIN_DATA_LEN. If
  the msg is bigger than the minimum size, then no padding is added.

  ARGS:
  msg      pointer to string buffer containing message to be padded

  RETURNS:
  paddingLen    number of bytes of padding added to end of msg

<<*/
static unsigned int
MsgPaddingAdd(char *msg)
{
  char paddingLen = 0, *paddingPtr = 0, i = 0;
  long int msgLen = 0;

  msgLen = (long int)StrLen(msg);
  if (msgLen < MNTCPAPP_UDP_MIN_DATA_LEN) {
	/* calculate number of bytes of padding to add */
	paddingLen = MNTCPAPP_UDP_MIN_DATA_LEN - (char) msgLen;
	paddingPtr = msg + msgLen;

	/* set length of message to minimum data length */
	msgLen = MNTCPAPP_UDP_MIN_DATA_LEN;
	for (i = 0; i < paddingLen; i++) {
	  *(paddingPtr++) = MNTCPAPP_PADDING_CHAR;
	}
	/* null-terminate new string */
	*paddingPtr = '\0';
  }
  else {
	/* data is already the correct size, so no need to pad */
  }
  return (unsigned int)paddingLen;
}

/*>>

  static unsigned int MsgPaddingRemove(char *msg);

  DESCRIPTION:
  This function removes any padding from a msg. Note that
  MNTCPAPP_PADDING_CHAR should not be used within a message.

  ARGS:
  msg      pointer to string containing message

  RETURNS:
  paddingLen    number of bytes of padding removed from message

<<*/
static unsigned int
MsgPaddingRemove(char *msg)
{
  char paddingLen = 0, *paddingPtr = 0;
  int msgLen = 0;

  msgLen = (int)StrLen(msg);
  paddingPtr = StrChr(msg, MNTCPAPP_PADDING_CHAR);
  if (paddingPtr) {
#ifdef COMMENT
	/* +++dhwong: or do I need a bigger variable? */
#endif /* COMMENT */
	paddingLen = msg + msgLen - paddingPtr;
	/* chop off padding by null terminating string before it */
	*paddingPtr = '\0';
  }
  else {
	/* no padding was found */
  }
  return (unsigned int)paddingLen;
}

/*>>

  static void AppVersionGet(void);

  DESCRIPTION:
  This function notifies ANVL of Stub Application version.

<<*/
static void
AppVersionGet(void)
{
  char          buf[MAX_STR];
  int           versionMajor = MNTCPAPP_VERSION_MAJOR;
  int           versionMinor = MNTCPAPP_VERSION_MINOR;

  MemSet((void *)buf, '\0', sizeof(buf));

#ifdef __DEBUG__
  FPrintf(stdout, "--->In AppVersionGet\n\n");
#endif

  /* Send version stamp to ANVL */
  SPrintf(buf, "%d#%d#", versionMajor, versionMinor);
  SendStatusToAnvl(buf);

#ifdef __DEBUG__
  FPrintf(stdout, "--->Moving out of AppVersionGet\n\n");
#endif

  return;
}
