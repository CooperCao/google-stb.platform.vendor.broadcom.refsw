
#ifndef _BTIMED_PKT_H__
#define _BTIMED_PKT_H__


typedef enum b_pkt_type {
	b_pkt_type_data=0,
	b_pkt_type_rtcp_sr=1,
	b_pkt_type_rtp_data=2,
	b_pkt_type_rtsp_stream_desc=3
} b_pkt_type;

struct b_pkt_header {
	uint8_t type; /* packet type */
	uint8_t reserved;
	uint16_t len;
	uint32_t offset; /* time offset in msec */
};

struct b_pkt_rtcp_sr {
	uint32_t ntp_msw;
	uint32_t ntp_lsw;
	uint32_t timestamp;
	uint32_t clock_rate;
};


#endif /* _BTIMED_PKT_H__ */


