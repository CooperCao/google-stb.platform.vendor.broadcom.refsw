
#ifndef __LGE_MFG_API_H__
#define __LGE_MFG_API_H__

#include "bcmMfgTestApi.h"

#define ETHER_ADDR_LEN 6
typedef unsigned char uint8;

#include <packed_section_start.h>
BWL_PRE_PACKED_STRUCT struct  ether_addr {                                                                         
        uint8 octet[ETHER_ADDR_LEN];                                                         
} BWL_POST_PACKED_STRUCT;
#include <packed_section_end.h>
typedef ether_addr EtherAddr;

enum PreambleType {Long, Short};

char *PREAMBLE_TYPE[] = {
	"long",
	"debug"
};

char *RATE_TABLE[] = {
	"-1",		/* Automatically set to the best rate. Not used by LGE */
	"1", 		/* 1Mbits */
	"2", 		/* 2Mbits */
	"5.5", 		/* 5.5Mbits */
	"6",	 	/* 6Mbits */
	"9",  		/* 9Mbits */
	"11",  		/* 11Mbits */
	"12", 	 	/* 12Mbits */
	"18", 	 	/* 18Mbits */
	"22", 	 	/* 22Mbits */
	"24", 	 	/* 24Mbits */
	"33", 	 	/* 33Mbits */
	"36", 		/* 36Mbits */
	"48", 	 	/* 48Mbits */
	"54", 	 	/* 54Mbits */
};
#define RATE_MIN	1	/* Min index in rate table */
#define RATE_MAX	14	/* Max index in rate table */

typedef struct {
	int TxBurstInterval;
	int TxPayloadLength;
	int TxFrames;
	int UseAcks;
	ether_addr DestinationMacAddr;
	ether_addr SourceMacAddr;
} pkteng_data;

extern pkteng_data pkteng_txdata_cache;
extern pkteng_data pkteng_rxdata_cache;

extern "C" {
	DLLExport bool LGE_RFT_OpenDUT();
	DLLExport bool LGE_RFT_CloseDUT();
	DLLExport bool LGE_RFT_TxDataRate(int TxDataRate);
	DLLExport bool LGE_RFT_SetPreamble(PreambleType preamble);
	DLLExport bool LGE_RFT_Channel(int ChannelNo);
	DLLExport bool LGE_RFT_TxGain(int TxGain);
	DLLExport bool LGE_RFT_TxBurstInterval(int SIFS);
	DLLExport bool LGE_RFT_TxPayloadLength(int TxPayLength);
	DLLExport bool LGE_RFT_TxStart();
	DLLExport bool LGE_RFT_TxStop();
	DLLExport bool LGE_RFT_RxStart();
	DLLExport bool LGE_RFT_RxStop();
	DLLExport bool LGE_RFT_FRError(int* FError);
	DLLExport bool LGE_RFT_FRGood( int* FRGood);
	DLLExport bool LGE_RFT_PktengRxRSSI(int* RSSI);

	DLLExport bool LGE_RFT_TxBurstFrames(int Frames);	/* Frames in a Tx burst */
	DLLExport bool LGE_RFT_TxDestAddress(EtherAddr DestAddr);

	DLLExport bool LGE_RFT_SetMacAddress(EtherAddr Addr);
	DLLExport bool LGE_RFT_UseAcks(bool UseAcks);
	
	DLLExport bool LGE_RFT_RemoteWl(char *command, char *output);
}

#endif /* __LGE_MFG_API_H__ */
