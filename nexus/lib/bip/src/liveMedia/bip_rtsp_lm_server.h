/******************************************************************************
 * (c) 2014 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *****************************************************************************/
/*
 * Extension of RTSP Server Class to provide a custom RTSP Server
 */
#ifndef BIP_LM_RTSP_SERVER_HH
#define BIP_LM_RTSP_SERVER_HH

#include "bip_types.h"
#include "bip_rtsp_server.h"
#include "bip_rtsp_lm_session.h"
#include "bip_igmp_listener.h"
#include "bip_rtsp_lm_sink.h"
#include "RTSPServer.hh"
#include "RTSPCommon.hh"
#include "RTCP.hh"

typedef enum {
    BIP_RtspLmSessionStreamType_Sat,
    BIP_RtspLmSessionStreamType_Qam,
    BIP_RtspLmSessionStreamType_File
} BIP_RtspLmSessionStreamType;

typedef enum {
    BIP_RtspLmSessionStreamState_Idle = 0,
    BIP_RtspLmSessionStreamState_UnicastSetupPending,
    BIP_RtspLmSessionStreamState_UnicastSetup,
    BIP_RtspLmSessionStreamState_UnicastPlayPending,
    BIP_RtspLmSessionStreamState_UnicastPlay,
    BIP_RtspLmSessionStreamState_MulticastSetupPending,
    BIP_RtspLmSessionStreamState_MulticastSetup,
    BIP_RtspLmSessionStreamState_MulticastPlayPending,
    BIP_RtspLmSessionStreamState_MulticastPlay,
    BIP_RtspLmSessionStreamState_UnicastChannelChangePending, /* SETUP came in while in PLAY */
    BIP_RtspLmSessionStreamState_MulticastChannelChangePending, /* SETUP came in while in PLAY */
    BIP_RtspLmSessionStreamState_IgmpJoinPending,
    BIP_RtspLmSessionStreamState_IgmpJoin,
    BIP_RtspLmSessionStreamState_IgmpJoinStreaming,
    BIP_RtspLmSessionStreamState_IgmpJoinChannelChangePending, /* case for SETUP,JOIN, SETUP(channelchange)*/
    BIP_RtspLmSessionStreamState_IgmpLeavePending, /* goes from this state to MulticastSetup */
    BIP_RtspLmSessionStreamState_TeardownPending, /* goes from this state to Idle */
    BIP_RtspLmSessionStreamState_TimeoutPending, /* goes from this state to Idle */
    BIP_RtspLmSessionStreamState_Max
} BIP_RtspLmSessionStreamState;

typedef enum {
    BIP_RtspSessionReturnCode_Continue=100,
    BIP_RtspSessionReturnCode_Ok=200,
    BIP_RtspSessionReturnCode_BadRequest=400,
    BIP_RtspSessionReturnCode_Forbidden=403,
    BIP_RtspSessionReturnCode_NotFound=404,
    BIP_RtspSessionReturnCode_MethodNotAllowed=405,
    BIP_RtspSessionReturnCode_NotAcceptable=406,
    BIP_RtspSessionReturnCode_RequestTimedOut=408,
    BIP_RtspSessionReturnCode_RequestUriTooLong=414,
    BIP_RtspSessionReturnCode_NotEnoughBandwidth=453,
    BIP_RtspSessionReturnCode_SessionNotFound=454,
    BIP_RtspSessionReturnCode_MethodNotValidInThisState=455,
    BIP_RtspSessionReturnCode_UnsupportedTransport=461,
    BIP_RtspSessionReturnCode_InternalServerError=500,
    BIP_RtspSessionReturnCode_NotImplemented=501,
    BIP_RtspSessionReturnCode_ServiceUnavailable=503,
    BIP_RtspSessionReturnCode_RtspVersionNotSupported=505,
    BIP_RtspSessionReturnCode_OptionNotSupported=551
} BIP_RtspSessionRecordCode;

char * extractUrl( const char *const fullRequestStr );
void parsePidsFromCsvList( char *pidListBuffer,
                           int *pidList,
                           int *pidListCount );
int  parseToken( const char *input,
                 char *output,
                 int output_len,
                 const char *begin,
                 const char *end );
int parseTransportHeader(
    char const                     *buf,
    BIP_RtspLiveMediaStreamingMode &streamingMode,
    char *                         &clientAddressStr,
    u_int8_t                       &destinationTTL,
    portNumBits                    &clientRTPPortNum,  // if UDP
    portNumBits                    &clientRTCPPortNum, // if UDP
    Boolean                        &isMulticast,
    Boolean                        &isUnicast
    );
char * BIP_Rtsp_ConcatinateSdp( char *sdpDescriptionCombined,
    BIP_RtspLiveMediaSessionSatelliteSettings *satelliteSettings,
    BIP_RtspTransportStatus *transportStatus);
char * BIP_Rtsp_GenerateSdpDescription( BIP_RtspLiveMediaSessionSatelliteSettings *pSatelliteSettings,
       BIP_RtspTransportStatus *pTransportStatus);
char * BIP_Rtsp_CreateSessionIdStr( unsigned long int sessionId );


/********************************************************************************************************************************
 *
 *
 ********************************************************************************************************************************/
class BIP_RtspServer : public RTSPServer
{
public:
static BIP_RtspServer *createNew( UsageEnvironment&env, Port rtspPort = 554,
    UserAuthenticationDatabase *authDatabase = NULL,
    unsigned reclamationTestSeconds = 60, char *multicastAddressTemplate = NULL );
class BIP_RtspClientSession;    //forward declaration
class BIP_RtspClientConnection; //forward declaration

void  setIgmpListenerHandle(
    BIP_IgmpListener *igmpListener
    )
{fIgmpListener = igmpListener; }
void setConnectedCallback(
    BIP_CallbackDesc callback
    ) {fConnectedCallback = callback; }
void setGetRtpStatisticsCallback(
    BIP_CallbackDesc callback
    ) {fGetRtpStatisticsCallback = callback; }
void setIgmpCallbacks();

void                                      BIP_Rtsp_ProcessIgmpMembershipReport( void *context );

BIP_RtspServer::BIP_RtspClientConnection *createNewClientConnectionFull( int clientSocket );
protected:
BIP_RtspServer(
    UsageEnvironment           &env,
    int                         ourSocket,
    Port                        ourPort,
    UserAuthenticationDatabase *authDatabase,
    unsigned                    reclamationTestSeconds,
    char                       *multicastAddressTemplate
    );
public:
// called only by createNew();
virtual ~BIP_RtspServer();

protected: // redefined virtual functions
virtual BIP_RtspClientConnection *createNewClientConnection( int clientSocket, struct sockaddr_in clientAddr );

        public: // should be protected, but some old compilers complain otherwise
        /*******************************************************************************************************************
         *
         *
         ********************************************************************************************************************/
        class BIP_RtspClientConnection : public RTSPServer::RTSPClientConnection
        {
        public:
        BIP_RtspClientConnection(
            BIP_RtspServer    &ourServer,
            int                clientSocket,
            struct sockaddr_in clientAddr
            );
        virtual ~BIP_RtspClientConnection();
        void setMessageReceivedCallback(
            BIP_CallbackDesc callback
            ) {fMessageReceivedCallback = callback; }
        void setErrorCallback(
            BIP_CallbackDesc callback
            ) {fErrorCallback = callback; }
        void copyReceivedMessageConnection( char *dstBuffer );
        BIP_RtspClientSession *createNewClientSession( BIP_RtspClientConnection *clientConnection, char *requestStr );
        void                   sendResponse( BIP_RtspResponseStatus responseStatus );
        int setResponseSetup( BIP_RtspClientSession       *pClientSession,
                              char *                       responseBuffer,
                              int                          responseBufferLen );
        Boolean requestIsFromNonOwner( const char * sessionIdStr );
        Boolean requestIsValid( const char * sessionIdStr );

        protected: // redefined virtual functions
        virtual void handleCmd_OPTIONS();
        virtual void handleCmd_DESCRIBE( char const *urlPreSuffix, char const *urlSuffix, char const *fullRequestStr );
        virtual void handleRequestBytes( int newBytesRead );
        static void  incomingRequestHandler( void *, int /*mask*/ );
        void         resetRequestBuffer();
        void         setRequestBuffer(const char * newContents );
        void         noteLiveness( const char * sessionIdStr ); /* called if OPTIONS command comes in with a specific sessionId */
        void         noteLiveness( void ); /* called every time any request comes in for a connection */

        public:
        void         livenessTimeoutTask( BIP_RtspClientConnection *clientConnection );

        protected:
        friend class BIP_RtspClientSession;
        friend class BIP_RtspServer;
        u_int32_t          fClientSessionId;
        u_int32_t          fClientInputSocket;
        struct sockaddr_in fClientInputAddr;
        BIP_RtspServer    *fOurServer;
        BIP_CallbackDesc   fMessageReceivedCallback;
        BIP_CallbackDesc   fErrorCallback;
        TaskToken          fLivenessCheckTask;
        Boolean            fIsTimingOut;          // set when we have a liveness timeout

        }; /* end class BIP_RtspClientConnection */

protected: // redefined virtual functions
virtual BIP_RtspClientSession *createNewClientSession( u_int32_t sessionId );

        public: // should be protected, but some old compilers complain otherwise

        BIP_RtspClientSession *BIP_Rtsp_FindSessionBySessionId( u_int32_t sessionId );





/********************************************************************************************************************************
 *
 *
 ********************************************************************************************************************************/
        class BIP_RtspClientSession : public RTSPServer::RTSPClientSession
        {
        public:
        BIP_RtspClientSession(
            BIP_RtspServer&ourServer,
            u_int32_t      sessionId
            );
        virtual ~BIP_RtspClientSession();
        void setMessageReceivedCallback(
            BIP_CallbackDesc callback
            ) {fMessageReceivedCallback = callback; }
         void setIgmpMembershipReportCallback(
            BIP_CallbackDesc callback
            ) {fIgmpMembershipReportCallback = callback; }
         BIP_CallbackDesc getIgmpMembershipReportCallback(){return (fIgmpMembershipReportCallback); }
        u_int32_t getSessionId(
            void
            ) {return( fSessionId ); }
        void sendResponseUsingRequest( BIP_RtspResponseStatus responseStatus, char *fullRequestStr );
        void sendResponse( BIP_RtspResponseStatus responseStatus );
        void copyReceivedMessageSession( char *dstBuffer );
        u_int32_t getClientSessionId() {return( fClientConnection->fClientSessionId ); }
        u_int32_t getClientSessionId2() {return( fSessionId ); }
        BIP_RtspClientConnection *getClientConnection() {return( fClientConnection ); }
        void reportLockStatus( bool bLockStatus );

        virtual void handleCmd_PLAY( BIP_RtspClientConnection *ourClientConnection,
            ServerMediaSubsession *subsession, char const *fullRequestStr );
        virtual void handleCmd_TEARDOWN( BIP_RtspClientConnection *ourClientConnection,
            ServerMediaSubsession *subsession, char const *fullRequestStr );
        void handleCmd_TEARDOWN_Callback( void * context );
        int setState(BIP_RtspLmSessionStreamState fState);
        BIP_RtspLmSessionStreamState getState(void);
        void   livenessTimeoutTask(BIP_RtspClientSession *clientSession);
        int    updateSettings( int                                        bitmaskOptionsFound,
                            BIP_RtspLiveMediaSessionSatelliteSettings *pSatelliteSettings,
                            BIP_RtspTransportStatus                   *pTransportStatus );
        void   scheduleGetRtpStatistics( void );
        void   unscheduleGetRtpStatistics( void );
        void   getRtpStatistics( void *context );
        void   createRtpRtcpSockets(   Port& serverRTPPort, Port& serverRTCPPort );

        protected: // redefined virtual functions
        BIP_Status handleCmd_SETUP( BIP_RtspClientConnection *ourClientConnection,
            char const *urlPreSuffix, char const *urlSuffix, char const *fullRequestStr );

        private:
        /* Parsing Function added from  old SatSession */
        BIP_RtspClientSession *BIP_Rtsp_FindOwnerByStreamId( int streamId );
        BIP_RtspClientSession *BIP_Rtsp_FindSessionByMulticastOctet( unsigned char octet );
        char * BIP_Rtsp_GenerateSdpDescription( BIP_RtspClientSession *pSession );
        unsigned char  BIP_Rtsp_ComputeNextMulticastOctet( void );
        char * GetIpAddressFromSocket( long int clientSocket );
        int  BIP_Rtsp_ComputeNextStreamId( void );
        char * BIP_Rtsp_CreateMulticastAddressStr( const char   *addrTemplate, unsigned char lastOctet );
        char * BIP_Rtsp_ConcatinateSdp( char *sdpDescriptionCombined, BIP_RtspClientSession *pSession );
        void   noteLiveness( void );
        void   startRtcpReports( void );
        void   stopRtcpReports( void );

        protected:
        friend class BIP_RtspClientConnection;

        private:
        u_int16_t                      fOurStreamId;
        ServerMediaSession            *fOurMediaSession;
        ServerMediaSubsession         *fOurMediaSubsession;
        Boolean                        fIsPlaying;          // set when we have sent a successful response to a PLAY request
        netAddressBits                 fDestinationAddress; // Destination IP address (unicast or multicast) to which this session is streaming
        Boolean                        fIsMulticast;        // true if we are streaming to multicast address, false otherwise
        BIP_RtspClientConnection      *fClientConnection;
        BIP_RtspServer                *fOurServer;
        RTSPServer::RTSPClientSession *fRtspClientSession;
        BIP_CallbackDesc               fMessageReceivedCallback;
        BIP_CallbackDesc               fIgmpMembershipReportCallback;
        u_int32_t                      fSessionId;
        void                          *fTeardownTaskToken;
        TaskToken                      fLivenessCheckTask;
        BIP_RtspLmSessionStreamState   fState; /* 1:setup-uni, 2: play-uni, 3: setup-multi, 4:play-multi */
        public:
        BIP_RtspLiveMediaSessionSatelliteSettings satelliteSettings;
        BIP_RtspTransportStatus transportStatus;
        char *requestUrl;           /* needed to send response for SDP (strdup) */
        RTPSource                     *fRtpSource;
        BIP_RtpSink                   *fRtpSink;
        RTCPInstance                  *fRtcpInstance;
        Groupsock                     *fRtpGroupsock;
        Groupsock                     *fRtcpGroupsock;
        unsigned                       fTimestampFreq;
        Boolean                        fIsVlc;
        Boolean                        fIsIpClient;
        TaskToken                      fGetRtpStatisticsId;
        }; /* end class BIP_RtspClientSession */


protected:
friend class BIP_RtspClientConnection;
friend class BIP_RtspClientSession;
private:
char                     *fOurMulticastAddressTemplate;
BIP_RtspClientConnection *fClientConnection;
BIP_CallbackDesc          fConnectedCallback;
BIP_CallbackDesc          fGetRtpStatisticsCallback;
BIP_IgmpListenerHandle    fIgmpListener;
HashTable                *fOurStreamIds;    // maps fourStreamID
HashTable                *fClientSessions;  // maps 'session id' strings to "BIP_RtspClientSession" objects
HashTable                *fClientAddresses; // maps 'ip_address' strings to "BIP_RtspClientSession" objects
unsigned                  fReclamationTestSeconds;
public:
static EventTriggerId eventMembershipReportTriggerId;

};  /* end class BIP_RtspServer */

void handleCmd_TEARDOWN_Callback( void *context );

#ifdef __cplusplus
extern "C" {
#endif

int  BIP_Rtsp_ParseUrlOptions( BIP_RtspLiveMediaSessionSatelliteSettings *satSettings,
                               BIP_RtspTransportStatus *pTransportStatus,
                               const char *urlSuffix );

#ifdef __cplusplus
}
#endif

#endif /* BIP_LM_RTSP_SERVER_HH */
