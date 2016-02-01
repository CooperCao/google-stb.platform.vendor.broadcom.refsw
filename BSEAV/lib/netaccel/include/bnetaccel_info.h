/***************************************************************************
 *     Copyright (c) 2006-2008, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * File: strminfo.h: streaming out sub-module
 * Revision History:
 *    
 * $brcm_Log: $
 * 
 *************************************************************/ 

#ifndef _STRMINFO_H_
#define _STRMINFO_H_

/* structure for controlling ip streaming */
#define AV_PKT_LEN      (188*7)     /* default AV PKT LEN, but can be overwriten by apps */
#define DEFAULT_PULL_DESCS     12
#define DEFAULT_PUSH_DESCS     12

#define ETH_HDR_LEN         14
#define IP_HDR_LEN          20
#define TCP_HDR_LEN         8
#define UDP_HDR_LEN         8
#define RTP_HDR_LEN         20
#define ETH_IP_HDR_LEN    (ETH_HDR_LEN + IP_HDR_LEN)
#define TCP_ACK_LEN (ETH_IP_HDR_LEN + 4*TCP_HDR_LEN)
#define IP_TCP_HDR_LEN  (20 + 4*TCP_HDR_LEN)
#define IP_UDP_HDR_LEN  (IP_HDR_LEN + UDP_HDR_LEN)
#define IP_RTP_HDR_LEN  (IP_HDR_LEN + UDP_HDR_LEN + RTP_HDR_LEN)
#define ETH_IP_TCP_HDR_LEN  (ETH_HDR_LEN + IP_HDR_LEN + 4*TCP_HDR_LEN)
#define ETH_IP_UDP_HDR_LEN  (ETH_HDR_LEN + IP_HDR_LEN + UDP_HDR_LEN)
#define ETH_IP_RTP_HDR_LEN  (ETH_HDR_LEN + IP_HDR_LEN + UDP_HDR_LEN + RTP_HDR_LEN)
#define MAX_HEADER_LEN  96
#define ETH_MTU         1500
#define DEFAULT_STREAMER_PORT   5000

typedef enum eChannelType {
    eLiveStream,
    eUserUDPStream,
    eUserTCPStream,
    eNetRecord,
    eUnused
} eChannelType;

typedef enum BufferMode {
    eBufferModeNormal,
    eBufferModePacing
} BufferMode;

typedef enum StreamSource {
    ePVR,       /*Feeding From Streamer*/
    eRecord0,   /*Feeding From Rave Engine*/
    eRecord1
} StreamSource ;

typedef enum StreamProtocol {
    eStream_UDP,
    eStream_RTP,
    eStream_TCP
} StreamProtocol;

/* this is for gcc little endian */
typedef struct rtp_bits_t 
{
    unsigned int cc:4;              /* CSRC identifiers */
    unsigned int x:1;               /* extension headers */
    unsigned int p:1;               /* padding */
    unsigned int v:2;               /* version */
    unsigned int pt:7;              /* payload type, see RFC 1890 */
    unsigned int m:1;               /* marker */
    unsigned int seq:16;            /* sequence number */
}rtp_bits_t;

typedef struct rtp_header_t
{
  rtp_bits_t    b;
  int           timestamp;
  int           csrc;
}rtp_header_t;

#ifdef RECORD_FROM_RAVE
typedef struct rec_desc {
    unsigned long w0;
    unsigned long w1;
    unsigned long w2;
    unsigned long w3;
} rec_desc;

typedef struct record_info {
    unsigned long count;
    unsigned long prev_count;
    unsigned long wp;
    unsigned long rp;
    unsigned long prev_wp;
    unsigned long state;
    unsigned long bufaddr;
    unsigned long firstDesc;
    unsigned long rlen;
    unsigned long rbuf;
    unsigned long buffered;
    unsigned long rd_wrap;
    unsigned long wp_wrap;
    unsigned long itb_base;
    unsigned long itb_size;
    unsigned long itb_valid_ptr;
    unsigned long itb_state;
    unsigned long avail_pkts_to_send;
    unsigned long partial_packet_send;
    unsigned long sync_found;
    unsigned long max_desc;
    unsigned long initial_packet;
    struct rec_desc *desc;
} record_info;

typedef struct itb_info {
    unsigned long count;
    unsigned long prev_count;
    unsigned long wp;
    unsigned long rp;
    unsigned long prev_wp;
    unsigned long state;
    unsigned long bufaddr;
    unsigned long firstDesc;
    unsigned long rlen;
    unsigned long rbuf;
    unsigned long buffered;
    unsigned long rd_wrap;
    unsigned long wp_wrap;
    unsigned long itb_base;
    unsigned long itb_size;
    unsigned long itb_valid_ptr;
    unsigned long itb_state;
    unsigned long avail_pkts_to_send;
    unsigned long partial_packet_send;
    unsigned long sync_found;
    unsigned long max_desc;
    unsigned long initial_packet;
    struct rec_desc *desc;
} itb_info;

typedef struct mem_info_t {
    unsigned long phys_addr;
    int size;
} mem_info_t;

#endif

typedef struct buf_info_t {
    unsigned long buf;
    unsigned long len;
    unsigned long pcr;

    unsigned long use_pcr;
} buf_info_t;

/* Descriptor to hold per user write buffer info */
typedef struct desc_t {
    unsigned long len;
    unsigned long orig_len;
    unsigned long pad_nulls;
    unsigned long base;
    unsigned long offset;
    unsigned long initial_offset;
    unsigned long index;
    unsigned long npg;
    unsigned long pcr;
    unsigned long use_pcr;
    struct page *pages[256];    /*limits each write to 1MB, make it tunable*/
    struct desc_t   *next;
    struct desc_t   *prev;
    int active;
    struct  timeval t0;
} desc_t;

/* Aggregate structure for holding user written buffers */
typedef struct user_buf_info_t {
    int     num_desc;
    unsigned long sd;
    struct sockaddr_in local;
    struct sockaddr_in remote;
    int    pkt_len;
    unsigned long pcr0;
    unsigned long pkts;
    int      buf_count;
    struct  timeval t0;
    int     buffer_depth;
    int     buffer_locked_depth;
    int     max_map_len;
    int     user_pages;
    int     protocol;
    int     trick_mode;
    int     resync;
    desc_t  *ds;        /* array of desc */
    desc_t  *retran_ds;
    desc_t  *avail_ds;
    desc_t  *desc;
} user_buf_info_t;

#define STRM_DRIVER_TYPE    'S'

/* Depricated: Configuration params for Streamer Session */
typedef struct STRM_Config_t {
    StreamProtocol protocol;
    unsigned long multicast;
    unsigned long address;
    unsigned long src_addr;
    unsigned long port;
    unsigned long route_local;
    unsigned char dest_mac[6];
    unsigned char src_mac[6];
} STRM_Config_t;

#define STRM_CONFIG _IOWR(STRM_DRIVER_TYPE,0,STRM_Config_t)

/* Enable / Disable Streaming */
typedef struct STRM_Ctrl_t {
    unsigned long enable;   /* Depricated: */
    unsigned long resync;   /* Depricated: */
    unsigned long flush;    /* Depricated: */
    unsigned long disable_checksum; /* Dont compute checksum on outgoing packets */
    unsigned long route_local; /* Decpricated: */
    BufferMode buffer_mode; /* Inform driver to Pace outgoing packets using PCRs */
} STRM_Ctrl_t;

#define STRM_CTRL _IOWR(STRM_DRIVER_TYPE,1,STRM_Ctrl_t)

/* Information about the streamer status: used for checking write FIFO depth */
typedef struct STRM_Status_t {
    unsigned long buffer_depth;
    unsigned long buffer_locked_depth;  /* how much data is written but not yet sent out */
    unsigned int free_desc_count;       /* how many free sw descriptors are currently available */
    unsigned long total_desc_count;     /* total sw descriptors */
    unsigned long retrans_depth;        /* not yet implemented: */
    /* rest of the fields are all depricated */
    unsigned long rp;                   
    unsigned long wp;
    unsigned long connection_state;
    int retransmit;
    unsigned long client_addr;
    unsigned long client_port;
    unsigned long trick_mode;
    unsigned long driver_stc;
    unsigned long driver_pcr;
    unsigned long tcp_seq;
    unsigned long tcp_window;
} STRM_Status_t;

#define STRM_STATUS _IOWR(STRM_DRIVER_TYPE,2,STRM_Status_t)

/* Statistics about the streamer */
typedef struct STRM_Stats_t {
    unsigned long protocol;
    unsigned long receive_count;
    unsigned long dropped_count;
    unsigned long retransmit_count;
    unsigned long send_count;
} STRM_Stats_t;

#define STRM_STATS  _IOWR(STRM_DRIVER_TYPE,3,STRM_Stats_t)

/* Streamer Resync */
typedef struct STRM_Resync_t {
    unsigned long resync;       /* allows apps to flush the queued data & resync the PCR related timing logic (e.g. used during file wrap arounds) */
    unsigned long trick_mode;   /* Depricated: */
    unsigned long cache_flush;  /* Depricated: */
    int speed;                  /* Depricated: */
} STRM_Resync_t;

#define STRM_RESYNC  _IOWR(STRM_DRIVER_TYPE,4,STRM_Resync_t)

/* Depricated: */
typedef struct STRM_MapBuffer_t {
    unsigned long buf;
    unsigned long size;
} STRM_MapBuffer_t;

#define STRM_SETSOURCE  _IOWR(STRM_DRIVER_TYPE,5,STRM_SetSource_t)

/* Depricated: */
typedef struct STRM_SetSource_t {
    StreamSource  source;
} STRM_SetSource_t;

/* Depricated: Streamer Test */
typedef struct STRM_Test_t {
    unsigned long fd;
} STRM_Test_t;

#define STRM_TEST  _IOWR(STRM_DRIVER_TYPE,8,STRM_Test_t)

/* Depricated: */
typedef struct STRM_Sleep_t {
    unsigned long interval;
} STRM_Sleep_t;

#define STRM_SLEEP _IOWR(STRM_DRIVER_TYPE,7,STRM_Sleep_t)

/* modify the number of software descritors of the driver: one descriptor per user buffer is used */
typedef struct STRM_SetBuffer_t {
    unsigned long num_desc; /* set to number of buffers that user allocates for maintaining the write FIFO */
} STRM_SetBuffer_t;

#define STRM_SETDESC _IOWR(STRM_DRIVER_TYPE,9,STRM_SetBuffer_t)

/* notify driver about using plain UDP or RTP over UDP protocol */
typedef struct STRM_Protocol_t {
    StreamProtocol protocol;
    int fec;
    int simulate_pkt_loss;
    int transportPacketSize;    /* Specifies the trasnport packet size: 188/192/130/134 */
} STRM_Protocol_t;

#define STRM_PROTOCOL _IOWR(STRM_DRIVER_TYPE,10,STRM_Protocol_t)

/* new socket type for receiving UDP based data via Net DMA path */
#define SOCK_BRCM_DGRAM 7
/* new socket type for receiving TCP based data via Net DMA path */
#define SOCK_BRCM_STREAM 8

/* socket options for tuning how much data to receive */
typedef struct STRM_SockRecvParams_t {
    int pktsPerRecv;    /* how many pkts/datagrams to receive in a read/recvfrom socket call */
                        /* defaults to 1 pkt per recv/recvfrom call */
    int pktOffset;      /* offset at which each UDP datagram should be copied */
                        /* 0 value means no fixed offset, packets are copied back to back in rcv buf */
    int hdrOffset;      /* how many bytes to skip after IP hdr, each data pkt begin after this offset */
                        /* Driver ignores it for TCP, adjusts payload start to this offset for UDP */
                        /* Allows Apps using UDP to either get data starting at UDP header or past UDP header */
	int recvTimeout;	/* Timeout value in millisec for read/recvfrom: returns currently accumulated data a/f this timeout */
    int useCpuCopy;     /* 1: Use CPU to copy the received data (using copy_to_user) instead of M2M DMA */
                        /* 0: use M2M for copy (default */
} STRM_SockRecvParams_t;
#define STRM_SOCK_RECV_PARAMS _IOWR(STRM_DRIVER_TYPE,11,STRM_SockRecvParams_t)

#define STRM_SOCK_RECV_PARAMS_UDP_DEFAULT  {1,       0, UDP_HDR_LEN, 300, 0}
#define STRM_SOCK_RECV_PARAMS_RTP_DEFAULT  {1, ETH_MTU,           0, 300, 0}
#define STRM_SOCK_RECV_PARAMS_HTTP_DEFAULT {1,       0,           0,  100, 0}

/* socket options for SOCK_BRCM_DGRAM socket type */
typedef struct STRM_SockSecurityParams_t {
    int slotNum;        /* slot number that contains the encryption/decryption key */
    int do_enc_dec;     /* encrypt/decrypt the payloads */
} STRM_SockSecurityParams_t;
#define STRM_SOCK_SECURITY_PARAMS _IOWR(STRM_DRIVER_TYPE,12,STRM_SockSecurityParams_t)


#define STRM_SOCK_SET_FILTER_STATE  _IOWR(STRM_DRIVER_TYPE,13, STRM_SockSetFilterState_t)

typedef struct STRM_SockSetFilterState_t {
    int filterEnable;    /* true: enable filter, false: disable filter */
} STRM_SockSetFilterState_t;

#endif /* _STRMINFO_H_ */
