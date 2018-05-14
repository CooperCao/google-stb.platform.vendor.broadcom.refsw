/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
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
 ******************************************************************************/

#ifndef DSGCCCABLECARDIF_H
#define DSGCCCABLECARDIF_H

//********************** Include Files ***************************************

#include "Thread.h"

//********************** Global Types ****************************************
// From OC-SP-CCIF2.0-IO8-061031
enum
{
	SCTE55_MODE,
	BASIC_DSG_MODE,
	BASIC_DSG_ONEWAY_MODE,
	ADVANCED_DSG_MODE,
	ADVANCED_DSG_ONEWAY_MODE,
    DSG_INVALID_MODE
};

typedef struct _CCARD_CONFIGURE_SOCKET_FLOW_APDU
{
    uint8   protocol_flag;
#define     SF_UDP     0x00
#define     SF_TCP     0x01
    uint16  local_port_number;
    uint16  remote_port_number;
    uint8   remote_address_type;
#define     SF_DOMAIN_NAME  0x00
#define     SF_IPV4         0x01
#define     SF_IPV6         0x02
    union {
    uint8 *pNameOrIpv6addr;
    uint32 ipv4_address;
    } Var;
    uint8 connection_timeout;
} CCARD_CONFIGURE_SOCKET_FLOW_APDU;

#define     MAX_SOCKET_FLOW     0x08
typedef struct _CCARD_SOCKET_FLOW
{
    unsigned long SocketFlowId;
    int UdpTcpSocket;
    struct sockaddr_in  Us_sockaddr;
    struct sockaddr_in6 Us_sock6addr;
    uint8 remote_address_type;
} CCARD_SOCKET_FLOW;

typedef struct _CCARD_SOCKET_FLOW_CONNECTION
{
    uint32  gSocketFlowId;
    int     Ds_Socket_Id;
    sockaddr_in DsSockParam;
    sockaddr_in UsSockParam;
} CCARD_SOCKET_FLOW_CONNECTION;

//********************** Global Constants ************************************

#define kClientPort	2233
  
#if ( BCM_DSG_DUAL_PROCESSOR_INTERFACE ) 
#else
  // This name needs to match what is enabled in the eCM non-vol for CC IP_U stack
  // for now, just assume is Ip stack 3 (default)
  #define IPU_STACK_NAME	"bcm2"
#endif
  
#define LOSTFLOW_UNKNOWN        0x00
#define LOSTFLOW_IP_EXPIRE      0x01
#define LOSTFLOW_NET_BUSY       0x02
#define LOSTFLOW_NO_AUTHORIZE   0x03
#define LOSTFLOW_RTCP_CLOSED    0x04
#define LOSTFLOW_SOCKET_RD_ERR  0x05
#define LOSTFLOW_SOCKET_WR_ERR  0x06

#define INVALID_FLOW_ID     0xFF000000      // 24 bit valid flow ID. Use MSB to indicate non-zero invalid flow ID
//********************** Global Variables ************************************

//********************** Forward Declarations ********************************
static void BcmPassDcdToCableCard ( uint8 *pDcdTlv, unsigned int DcdLen );

//********************** Class Declaration ***********************************
// Forward reference...
class BcmMutexSemaphore;

class BcmCableCardIfThread : public BcmThread
{
public:

    // Constructor.  Initializes and starts the thread.
    //
    // Parameters:
    //      initialPriority - the thread priority that should be used.
    //
    // Returns:  N/A
    //
    BcmCableCardIfThread(BcmOperatingSystem::ThreadPriority initialPriority = BcmOperatingSystem::kTimeCriticalPriority);

    // Destructor.  If the thread is still running, it will be stopped, allowing
    // OS objects to be destroyed (note - this is actually done by the derived
    // class when it calls WaitForThread()).
    //
    // Parameters:  N/A
    //
    // Returns:  N/A
    //
    virtual ~BcmCableCardIfThread();

    // Returns the pointer to the singleton instance for this class.  Most
    // objects in the system will use this method rather than being passed a
    // pointer to it.  The singleton pointer will be set up in the base-class
    // constructor.
    //
    // NOTES:  This can return NULL if a singleton has not been set up for the
    //         system, so you must check for this condition.
    //
    //         You must not delete this object!
    //
    //         You should not store a pointer to the object that is returned,
    //         since it may be deleted and replaced with a new one.
    //
    // Parameters:  None.
    //
    // Returns:
    //      A pointer to the instance that should be used by the system.
    //
    static BcmCableCardIfThread *GetSingletonInstance(void);

    //simple accessors
    void SetDsgFlowId( unsigned long new_val ) {  gDsgFlowId = new_val; }
    unsigned long GetDsgFlowId(void) { return gDsgFlowId; }
    unsigned long GetIpUFlowId(void) { return gIpUFlowId; }
    uint32 GetCableCardIpAddress(void ) { return CableCardIpAddress; }
    void SetCableCardIpAddress( unsigned long ccip ) { CableCardIpAddress = ccip; }

	int UdpTcpSocket(unsigned long flowid)
    {
        int i ;
        for (i=0;i<MAX_SOCKET_FLOW;i++)
        {
            if (gSocketFlow[i].SocketFlowId == flowid)
            {
                return gSocketFlow[i].UdpTcpSocket;
            }
        }
        if (i == MAX_SOCKET_FLOW) return 0;
    }

	int  UdpTcpSocketUsIpV6Addr(unsigned long flowid, struct sockaddr_in6 *pSockaddr)
    {
        int i ;
        for (i=0;i<MAX_SOCKET_FLOW;i++)
        {
            if (gSocketFlow[i].SocketFlowId == flowid)
            {
                pSockaddr->sin6_addr = gSocketFlow[i].Us_sock6addr.sin6_addr;
                pSockaddr->sin6_port = gSocketFlow[i].Us_sock6addr.sin6_port;
                pSockaddr->sin6_family = gSocketFlow[i].Us_sock6addr.sin6_family;
                return 1;
            }
        }
        if (i == MAX_SOCKET_FLOW) return 0;
	}

    int UdpTcpSocketUsIpV4Addr(unsigned long flowid,struct sockaddr_in *pSockaddr)
    {
        int i ;
        for (i=0;i<MAX_SOCKET_FLOW;i++)
        {
            if (gSocketFlow[i].SocketFlowId == flowid)
            {
                pSockaddr->sin_addr = gSocketFlow[i].Us_sockaddr.sin_addr;
                pSockaddr->sin_port = gSocketFlow[i].Us_sockaddr.sin_port;
                pSockaddr->sin_family = gSocketFlow[i].Us_sockaddr.sin_family;
                return 1;
            }
        }
        if (i == MAX_SOCKET_FLOW) return 0;
    }

    unsigned char NumOpenTunnels(void) { return NumOpenTunnel; }

	uint8 SocketFlowIpType(unsigned long flowid)
    {
        int i ;
        for (i=0;i<MAX_SOCKET_FLOW;i++)
        {
            if (gSocketFlow[i].SocketFlowId == flowid)
            {
                return gSocketFlow[i].remote_address_type;
            }
        }
        if (i == MAX_SOCKET_FLOW) return 0;
    }

    void UdpTcpSocketFlowInit(int index)
    {
        if(index > MAX_SOCKET_FLOW) return;
	    gSocketFlow[index].SocketFlowId=INVALID_FLOW_ID;
	    gSocketFlow[index].UdpTcpSocket=0;
	    memset(&gSocketFlow[index].Us_sockaddr, 0, sizeof(gSocketFlow[index].Us_sockaddr));
	    memset(&gSocketFlow[index].Us_sock6addr, 0, sizeof(gSocketFlow[index].Us_sock6addr));
	    gSocketFlow[index].remote_address_type=SF_DOMAIN_NAME;
        return;
	}

	// Handle various DSG mode
    //
    void HandleSetDsgMode( unsigned char oper_mode, unsigned char * pmac_struct );

    //
    // Handle DSG AdvanceMode Tunnels configuration
    //
    void HandleConfigureAdvanceDsg(unsigned char *pkt_obj_ptr, unsigned short len );

    // Handle cablecard Ip Unicast flow request
    // This is supposed to implement as a dhcp proxy for cablecard
    //
    // Parameters:  
    //	unsigned long flowid - IP unicast flow id to use when forwarding data to cablecard
    //	unsigned char *mac - CableCard Mac address to use for this flow
    //	unsigned char opt_len - dhcp request option length
    //	unsigned char *opt_data - actual dhcp option string
    //	unsigned char *ret_option - buffer to contain any dhcp reply option string
    //	int *ropt_len - dhcp reply option length
    //
    // Returns:  error code of 0 or ipaddress obtained.
    //
    uint32 HandleIpUnicastWithDhcpReq( unsigned long flowid, unsigned char *mac, unsigned char opt_len, unsigned char *opt_data, unsigned char *ret_option, int *ropt_len );

    // Handle cablecard removal clean up
    //
    void HandleCardRemovedCleanUp( void );

    // new DSG socket flow functions
    unsigned char HandleSocketFlowConfig( unsigned long flowid, unsigned char *pkt_obj_ptr, unsigned short len );
    unsigned char HandleDeleteFlowRequest( unsigned long flowid );
    void HandleConfigureDsgDirectory(unsigned char *pkt_obj_ptr, unsigned short len );

    // C++ to C calls
    int sendTunnelDataToHost( const unsigned char *pBufData, uint16 pktLen, uint32 clientType, uint32 clientId );
    int sendTunnelDataToCableCard( const unsigned char *pBufData, uint16 pktLen );
    void SendDsDataToCableCard( unsigned char *pBufData, uint16 pktLen );
    void sendFixedAppsTunnelRequestToCableCard( void );
    void sendUcidToCableCard( unsigned char ucid );
    void sendTwoWayModeToCableCard( int mode );
    void sendDsScanCompleteToCableCard( void );
    void sendDccDepartToCableCard( unsigned char init_type );
    void sendDisabledForwardingTypeToCableCard( unsigned char disabled_fwd_type );
    void sendEcmResetToCableCard( void );

    // Debug helper function
    void DumpBytes(const char *pname, unsigned char *buf, int length);

    // C code call to start thread
    void StartDsRxThread( void );

    // Remember Dhcp reply option byte
    void SetCableCardDhcpReplyOption( char * pDhcpResponse, unsigned char optionlen );

    // Helper function
    void CloseAllCableCardClients( BcmDsgClientCtlThread *pDsgClientCtrl, bool CloseEcmTunnels = true );

    // ipv6 support
    unsigned char ConfigureIpV6SocketFlow( int i );

    // helper function to clear private variable
    // a modulo 256 counter. Use 16bit 0x5500 as the reset value
    void ResetDsgDirVersion( void ) { dsg_directory_version = 0x5500; cout << "Reset DSG_DIR version!!!" << endl; }

protected:

    // Thread constructor - this is the first method called after the thread has
    // been spawned, and is where the thread should create all OS objects.  This
    // has to be done here, rather than in the object's constructor, because
    // some OS objects must be created in the context of the thread that will
    // use them.  The object's constructor is still running in the context of
    // the thread that created this object.
    //
    // The default implementation simply returns true, allowing derived classes
    // that don't have any initialization to use the default.
    //
    // Parameters:  None.
    //
    // Returns:
    //      true if successful and ThreadMain() should be called.
    //      false if there was a problem (couldn't create an OS object, etc.)
    //          and ThreadMain() should not be called.
    //
    virtual bool Initialize(void);

    // Thread destructor - this is the last method called when the thread is
    // exiting, and is where the thread should delete all of the OS objects that
    // it created.
    //
    // The default implementation does nothing, allowing derived classes that
    // don't have any deinitialization to use the default.
    //
    // Parameters:  None.
    //
    // Returns:  Nothing.
    //
    virtual void Deinitialize(void);

    // This is the main body of the thread's code.  This is the only method
    // that absolutely must be provided in the derived class (since there is no
    // reasonable default behavior).
    //
    // Parameters:  None.
    //
    // Returns:  Nothing.
    //
    virtual void ThreadMain(void);

public:
    // use in CableCardCallbackSendSocketFlowUsData()

private:

    unsigned long gDsgFlowId;
    unsigned long gIpUFlowId;
    CCARD_SOCKET_FLOW gSocketFlow[MAX_SOCKET_FLOW];
    BcmMacAddress CableCardMacAddress;
    unsigned short Remove_Header_Bytes;
    bool DocsisRangedRegistered;
    uint32 CableCardIpAddress;
	uint32 fLastCableCardIpAddress;
	uint8 fLastDsgMsgApdu;  

    unsigned char packet_buf[2048];
    unsigned char NumOpenTunnel;
    unsigned char DhcpOptionLen;
    unsigned char DhcpOptionByte[256];

    CCARD_CONFIGURE_SOCKET_FLOW_APDU SocFlowInfo;

    bool EcmResetRecoveryForIpu;
    unsigned char saved_mac[6];
    unsigned char saved_opt_len;
    unsigned char saved_opt_data[256];

    unsigned char dsg_oper_mode;

    bool need_to_rescan;
    uint16 vct_id;
    uint16 dsg_directory_version;

    BcmMutexSemaphore *fpMutex;

};


//********************** Inline Method Implementations ***********************



//********************** Export local procedures without name-mangling ***********************
extern "C" 
{
  // My functions for pod_dsg_api.c to call
  void CableCardCallbackSetDSGMode( unsigned char * pkt_obj_ptr, unsigned short len );
  void CableCardCallbackDSGFlowID( unsigned  long flow_id );
  void CableCardCallbackConfig_Advanced_DSG( unsigned char *pkt_obj_ptr, unsigned short len );
  void CableCardCallbackDSGPacketError( unsigned char err_status );
  unsigned long CableCardCallbackIPUDhcp( unsigned  long flowid, unsigned char *mac, unsigned char opt_len, unsigned char *opt_data, unsigned char *ret_option, int *ropt_len );
  void CableCardCallbackSendIPUData( unsigned char *pdata, unsigned long len );
  void CableCardRemovedCallbackCleanUp( void );
  unsigned char CableCardCallbackSocketFlowConfig( unsigned long flowid, unsigned char *pkt_obj_ptr, unsigned short len  );
  void CableCardCallbackSendSocketFlowUsData( unsigned long flow_id, unsigned char *pdata, unsigned long len );
  unsigned char CableCardCallbackDeleteFlowReq(unsigned long flowid);
  void CableCardCallbackDSG_Directory( unsigned char *pkt_obj_ptr, unsigned short len );

  // functions in pod_dsg_api.c for me to call
  unsigned char BcmSendDSGTunnelDataToHost( unsigned char *pBufData, unsigned int pktlen, unsigned long client_type, unsigned long client_id);
  unsigned char BcmSendDSGTunnelDataToPOD( unsigned char *pBufData, unsigned int pktlen, unsigned long flow_id );
  unsigned char BcmSendDataToPOD( unsigned char *pBufData, unsigned int pktlen, unsigned long flow_id );
  void POD_Api_Send_DCD_Info(void *dcd_ptr, unsigned short dcd_len);
  void POD_Api_Send_DSG_Message(void *dsg_message_ptr, unsigned short dsg_message_len);
  void POD_Api_Lost_Flow_Ind(unsigned long id,unsigned char status);

  #if (defined( BCM_DSG_DUAL_PROCESSOR_INTERFACE ) || defined( BCM_DSG_DUAL_PROCESS_INTERFACE ))
  // functions in EstbRemoteInterfaceStub.c for me to call
  int DsgStartProxyDhcpRequest( unsigned char opt_len, unsigned char *mac, unsigned char *opt_data, uint32 ipaddress); //unsigned char *ret_option )
  int DsgShutDownProxyIpuStack( void );
  int DsgSendCableCardIpUsData( unsigned char *pData, unsigned short len );
  // My functions for EstbRemoteInterfaceStub.c to call
  void BcmEstbIndicateCableCardProxyDhcpResponse( unsigned long ccIpAddr, char * pDhcpResponse, unsigned char optionlen );
  void BcmSendDsDataToCableCard( int data_len, unsigned char *pDataStart );
  void BcmIndicateEcmReset( void );     //This one called from dsgclient controller thread
  #endif
}

#endif
