/*
  mntcpapp.h: TCP Stub Server Application include file.

  Copyright (c) Ixia 2002-2009
  All rights reserved.
 
*/

#ifndef __MNTCPAPP_H__
#define __MNTCPAPP_H__

/* This file contains definitions than need to be shared between the
   TCP Stub Server Application and ANVL */
#define MNTCPAPP_VERSION_MAJOR        3
#define MNTCPAPP_VERSION_MINOR        8

#define CMD_GET_VERSION_STR          "GET_VERSION"
#define CMD_LISTEN_STR               "LISTEN"
#define CMD_CONNECT_STR              "CONNECT"
#define CMD_SEND_STR                 "SEND" 
#define CMD_SEND_REPEAT_DATA_STR     "SEND_REPEAT_DATA" 
#define CMD_RECEIVE_STR              "RECEIVE"
#define CMD_RECEIVE_LIMSZ_STR        "RECEIVE_LIMSZ"           
#define CMD_RECEIVE_ONCE_STR         "RECEIVE_ONCE"

#define CMD_SEND_UDP_STR             "UDP_SEND"			    
#define CMD_SEND_UDP_REPEAT_DATA_STR "UDP_SEND_REPEAT_DATA"			    

#define CMD_SHUTDOWN_READ_STR        "SHUT_READ"
#define CMD_SHUTDOWN_WRITE_STR       "SHUT_WRITE"
#define CMD_SHUTDOWN_RD_WT_STR       "SHUT_RD_WT"

#define CMD_CLOSE_STR                "CLOSE"           
#define CMD_INFORMATION_STR          "INFORMATION"
#define CMD_REBOOT_STR               "REBOOT"
#define CMD_SOCK_CREATE_STR          "CREATE"
#define CMD_ABORT_STR                "ABORT"
#define CMD_PERROR_STR               "PERROR"
#define CMD_SETDFBIT_STR             "SETDFBIT"
#define CMD_SETMSS_STR               "SETMSS"
#define CMD_SETTTL_STR               "SETTTL"
#ifdef LIB_IPV6
#define CMD_SETHOPLIMIT_STR          "SETHOPLIMIT"
#endif /* LIB_IPV6 */
#define CMD_SET_SOCK_RCVBUF_LEN_STR  "SETRCVBUFLEN"
#define CMD_NOTIFY_TESTSTART_STR     "START_TEST"
#define CMD_NOTIFY_TESTEND_STR       "END_TEST"
#define CMD_API_ACCEPT_STR           "API_ACCEPT"
#define CMD_URG_CORRECT_STR          "OPT_STDURG"
#define CMD_URG_INLINE_STR           "OPT_OBBINLINE"

#define MNTCPAPP_STR_NODELAY         "NODELAY"
#define MNTCPAPP_STR_URG             "URG"
#define MNTCPAPP_STR_FAIL            "FAIL"
#define MNTCPAPP_STR_SUCCESS         "SUCCESS"
#define MNTCPAPP_STR_URGENT          "URG"
#define MNTCPAPP_STR_NODELAY         "NODELAY"
#define MNTCPAPP_STR_NOPUSH          "NOPUSH"

#define UDP_SOCK_BIND_RETRIES_NUM    5
#define TCP_SOCK_BIND_RETRIES_NUM    20

#define MNTCPAPP_API_RETCODE_OK               1
#define MNTCPAPP_API_RETCODE_NOTOK           -1

#define ERR_INVALID_ARG               -2

#ifdef LIB_IPV6
#define MODE_MNTCPAPP_IPV4           0
#define MODE_MNTCPAPP_IPV6           1
extern int gIPVersionForApp;
#endif /* LIB_IPV6 */

enum MNTCPAppState_e {
  TCP_STATE_CLOSED = 0,
  TCP_STATE_LISTEN,
  TCP_STATE_SYNSENT,
  TCP_STATE_SYNRCVD,
  TCP_STATE_ESTABLISHED,
  TCP_STATE_FINWAIT1,
  TCP_STATE_FINWAIT2,
  TCP_STATE_CLOSING,
  TCP_STATE_CLOSEWAIT,
  TCP_STATE_LASTACK,
  TCP_STATE_TIMEWAIT,

  /* MNTCPAPP state */
  SOCKET_CREATED
};
typedef enum MNTCPAppState_e MNTCPAppState_t;

enum TCPTestAppCommand_e {
  TCP_CMD_INVALID = 0,
  TCP_CMD_ABORT = 401,
  TCP_CMD_SOCK_CREATE,
  TCP_CMD_NOTIFY_TEST_END,
  TCP_CMD_RECV,
  TCP_CMD_RECV_LIMSZ,
  TCP_CMD_RECV_ONCE,
  TCP_CMD_NOTIFY_TEST_START,

  TCP_CMD_API_ACCEPT = 501,
  TCP_CMD_ASYNC_RECV,
  TCP_CMD_CLOSE,
  TCP_CMD_CONNECT,
  TCP_CMD_GET_VERSION,
  TCP_CMD_GET_CODE,
  TCP_CMD_GET_RETCODE,
  TCP_CMD_LISTEN,
  TCP_CMD_OPT_STDURG,
  TCP_CMD_GET_PERROR,
  TCP_CMD_SET_STATE,
  TCP_CMD_SET_DF_BIT,
  TCP_CMD_SET_HOPLIMIT,
  TCP_CMD_SET_MSS,
  TCP_CMD_SET_RCVBUF_LEN,
  TCP_CMD_SET_TTL,
  TCP_CMD_SHUTDOWN_RD_WT,
  TCP_CMD_SHUTDOWN_READ,
  TCP_CMD_SHUTDOWN_WRITE,
  TCP_CMD_GET_SAMPLE_DATA,
  TCP_CMD_GET_DATA,
  TCP_CMD_SEND,
  TCP_CMD_SEND_REPEAT_DATA,
  UDP_CMD_SEND,
  UDP_CMD_SEND_REPEAT_DATA,

  TCP_CMD_NO_ASYNC_RECV = 800,
  TCP_CMD_DUT_REBOOT,
  TCP_CMD_OPT_URGINLINE,
};
typedef enum TCPTestAppCommand_e TCPTestAppCommand_t;

enum TCPErrorCode_e {
  TCP_ERROR_UNKNOWN = 0,
  TCP_ERROR_CONN_FOREIGN_UNSPECIFIED = 1,
  TCP_ERROR_CONN_ILLEGAL_FOR_PROCESS = 1 << 1,
  TCP_ERROR_CONN_ALREADY_EXISTS = 1 << 2,
  TCP_ERROR_CONN_CLOSING = 1 << 3,
  TCP_ERROR_CONN_DOES_NOT_EXIST = 1 << 4,
  TCP_ERROR_CONN_REFUSED = 1 << 5,
  TCP_ERROR_CONN_RESET = 1 << 6,
  TCP_ERROR_ANY = 0xFF
};
typedef enum TCPErrorCode_e TCPErrorCode_t;

#ifdef __MNTCPOS_H__
/* The following are used by mntcpapp only */

/* The default UDP port on which mntcpapp will run */
#define MNTCPAPP_UDP_PORT                   10000

/* Maximum size of the UDP messages */
#define MAX_MESG_SIZE                       5000

#define SMALL_BUF_LEN                       10

/* Maximum number of errors that can occur in a test */
#define MAX_ERR_COUNT                       100



#define DEF_IP_TTL_VAL                      60

#ifdef LIB_IPV6
#define DEF_IPV6_HOPLIMIT_VAL               60
#endif /* LIB_IPV6 */

#define DEF_MNTCPAPP_TTL_VAL                74

#define DEF_SOCK_RCVBUFF_LEN                32768


#define CMD_INVALID_STR 

#define MNTCPAPP_DEF_RECV_DATALEN           10

#define MNTCPAPP_NO_OF_CLIENT_TO_LISTEN     1


#define MNTCPAPP_DEF_EXPECT_DATALEN         10
#define MNTCPAPP_DEF_MSS                    536
#define MNTCPAPP_DEF_DFBIT                  1
#define MAX_STR                             256

#define MNTCPAPP_APICALL_ERR                -1
#define MNTCPAPP_CMDLINE_ERR                -2

#define MNTCPAPP_CMD_DELIM                  "#"

/* 
   For some interface types (such as those that utilize an Ethernet
   controller chip, if the frame is too small when received (less than
   64 bytes), it may get stuck in the card's buffer. To avoid this,
   ensure that the data supplied in the packet will be at least 18
   bytes long (64 bytes - 14 bytes Ethernet Header - 20 bytes IP
   header - 8 bytes UDP header - 4 bytes checksum).
*/

#define MNTCPAPP_UDP_MIN_DATA_LEN           18
#define MNTCPAPP_PADDING_CHAR               '~'

enum ShutDownSock_e {
  SHUT_INVALID = 0,
  SHUT_READ,
  SHUT_WRITE,
  SHUT_RD_WT
};
typedef enum ShutDownSock_e ShutDownSock_t;

struct ApplicationState_s {
  /*
	State of the underlying TCP stack as perceived by the application layer. 
	This is set depending on the API call return code and sometimes ANVL 
	sets this value . Example: when ANVL receives the final ACK of connection
	establishment it sets this to TCP_ESTABLISHED. This is only used to 
	generate log information. This may help to analyze the test results.
  */
  MNTCPAppState_t dutState; 

  int tcpListenSock;
  int lastErrCode[MAX_ERR_COUNT];
  int returnCode;
  int topCode;
  char rcvdBuff[MAX_MESG_SIZE + 1];  /* +1 to add NULL at the end */
  int recvMode;                      /* ANVL can set this but this is not 
										used yet */

  /* Store the socket information in host byte order */
  MNTCPAppSocketInfo_t udpSockInfo;
  MNTCPAppSocketInfo_t tcpSockInfo;
};
typedef struct ApplicationState_s ApplicationState_t;

#endif /* __MNTCPOS_H__ */

#endif /*! __MNTCPAPP_H__ */
