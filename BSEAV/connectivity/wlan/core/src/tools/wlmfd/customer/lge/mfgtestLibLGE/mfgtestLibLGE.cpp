// mfgtestLibLGE.cpp : Defines the entry point for the DLL application.
//
/* Broadcom Proprietary and Confidential. Copyright (C) 2017,
/* All Rights Reserved.
/* 
/* This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
/* the contents of this file may not be disclosed to third parties, copied
/* or duplicated in any form, in whole or in part, without the prior
/* written permission of Broadcom.
 *
 * $Id: mfgtestLibLGE.cpp,v 1.6 2008-11-10 08:34:21 $
 */

#include "stdafx.h"
#include "../../../include/lgeMfgTestApi.h"

#define MFGTEST_API_DLL "MfgTestApi.dll"

#define BUFF_LEN 2048

HINSTANCE hDllInstance = NULL;

pkteng_data pkteng_txdata_cache;
pkteng_data pkteng_rxdata_cache;

static int pktengrxducast;
static int rxbadfcs;
static int rxbadplcp;

static int pktengrxducast_delta;
static int rxbadfcs_delta;
static int rxbadplcp_delta;
static int mpc_state;

#define BAD_FCS_CNT "rxbadfcs"
#define BAD_PLCP_CNT "rxbadplcp"
#define GOOD_FRAMES_CNT "pktengrxducast"

#define GLACIAL_TIMER "glacial_timer"
#define SLOW_TIMER "slow_timer"
#define FAST_TIMER "fast_timer"

#define GLACIAL_TIMER_VAL "120"
#define SLOW_TIMER_VAL "60"
#define FAST_TIMER_VAL "15"
#define DISABLE_TIMER_VAL "600000"

#ifdef _MANAGED
#pragma managed(push, off)
#endif

BOOL APIENTRY DllMain( HANDLE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

HINSTANCE LoadDLL();

int                                                                                          
ether_atoe(const char *a, struct ether_addr *n)                                           
{                                                                                            
        char *c;                                                                             
        int i = 0;                                                                           
                                                                                             
        memset(n, 0, ETHER_ADDR_LEN);                                                        
        for (;;) {                                                                           
                n->octet[i++] = (uint8)strtoul(a, &c, 16);                                   
                if (!*c++ || i == ETHER_ADDR_LEN)                                            
                        break;                                                               
                a = c;                                                                       
        }                                                                                    
        return (i == ETHER_ADDR_LEN);                                                        
}                                                                                            
                                                                                             
char *                                                                                       
ether_etoa(const struct ether_addr *n)                                                    
{                                                                                            
        static char etoa_buf[ETHER_ADDR_LEN * 3]; //Not re-entrant. But OK for the time being                                           
        char *c = etoa_buf;                                                                  
        int i;                                                                               
                                                                                             
        for (i = 0; i < ETHER_ADDR_LEN; i++) {                                               
                if (i)                                                                       
                        *c++ = ':';                                                          
                c += sprintf(c, "%02X", n->octet[i] & 0xff);                                 
        }                                                                                    
        return etoa_buf;                                                                     
}

static bool
read_counter(char *counter, int *stats)
{
	char buff[BUFF_LEN];
	bool bRet = false;
	char iovar[] = "counters";
	char *counter_ptr;
	if (hDllInstance && counter) {
		if(getWlIovar(iovar, NULL, 0, buff)) {
			DBGMSG(("read_counter : Failure during getWIovar(counters).\n"));
			return false;
		}
		if(counter_ptr = strstr(buff, counter)) {
			bRet = sscanf(counter_ptr + strlen(counter), "%d", stats);
	        DBGMSG(("read_counter : %s = %d\n", counter, *stats));	
		} else {
			DBGMSG(("read_counter : Failed reading counters for %s.\n", counter));
		}
	}
	return bRet;
}

DLLExport bool LGE_RFT_OpenDUT()
{
	char buff[BUFF_LEN];
	char cur_etheraddr[] = "cur_etheraddr";
	char mpc[] = "mpc";
	bool bRet = false;
	
	if ((hDllInstance = LoadDLL()) == NULL) {
		DBGMSG(("Failed to load %s\n", MFGTEST_API_DLL));
		return false;
	}
	/* By default do not send acks */
	pkteng_rxdata_cache.UseAcks = 0;
	
	/*initialize adapter before test
	* Example, open a adapter from local machine
	*/
	ADAPTER_INTERFACE_TYPE iftype = IF_SDIO;
	selectInterface(iftype, "none");
	if (bRet = openAdapter()) {
		/* Initialize the MAC addresses */
		if (getWlIovar(cur_etheraddr, NULL, 0, buff))
			return false;
		if (strstr(buff, cur_etheraddr)) {
			ether_atoe(buff + strlen(cur_etheraddr) + 1, &pkteng_txdata_cache.SourceMacAddr);
			memcpy(&pkteng_rxdata_cache.DestinationMacAddr, &pkteng_txdata_cache.SourceMacAddr, ETHER_ADDR_LEN);
			DBGMSG(("Initializing Tx Source Mac Address %s\n", ether_etoa(&pkteng_txdata_cache.SourceMacAddr)));
			DBGMSG(("Initializing Rx Destination Mac Address %s\n", ether_etoa(&pkteng_rxdata_cache.DestinationMacAddr)));
		}

		/* Disable MPC during this test, as it can cause problems */
		if (getWlIovar(mpc, NULL, 0, buff) == 0) {
			if(buff[0] == '1')
				mpc_state = 1;
			else
				mpc_state = 0;
		} else {
			DBGMSG(("Warning : Failed to get MPC state. Assuming it to be 0.\n"));
			mpc_state = 0;
		}
		setWlIovar(mpc, "0");
		setWlIovar(GLACIAL_TIMER, DISABLE_TIMER_VAL);
		setWlIovar(SLOW_TIMER, DISABLE_TIMER_VAL);
		setWlIovar(FAST_TIMER, DISABLE_TIMER_VAL);
		
	}
	return bRet;
}

DLLExport bool LGE_RFT_CloseDUT()
{
	bool bRet =  false;
	char mpc[] = "mpc";
		
	if (hDllInstance) {
		bRet = (setWlIovar(mpc, mpc_state ? "1" : "0")? false : true);
		if (false == bRet) {
			DBGMSG(("Warning : LGE_RFT_CloseDUT failed to restore mpc_state."));
		}
		setWlIovar(GLACIAL_TIMER, GLACIAL_TIMER_VAL);
		setWlIovar(SLOW_TIMER, SLOW_TIMER_VAL);
		setWlIovar(FAST_TIMER, FAST_TIMER_VAL);
		
		closeAdapter();
		FreeLibrary(hDllInstance);
		bRet = true; //TODO: maybe closeAdapter should return a value
	}
	return bRet;
}

/* LGE_RFT_TxDataRate : Sets the Tx data rate.
 * Parameters :
 * TxDataRate - LG Mapped enum values of data rate
 * Note :
 * Use the mapping of data rates specified in lgeMfgtestApi.h
 */
DLLExport bool LGE_RFT_TxDataRate(int TxDataRate)
{
	char buff[BUFF_LEN];
	char* rate = "rate";
	bool bRet =  false;
	
	if (hDllInstance) {
		if ((TxDataRate < RATE_MIN) || (TxDataRate > RATE_MAX)) {
			DBGMSG(("Invalid input value for TxDataRate. Should be a value between from 1 to 14\n. Ref : MfgtestApi.h"));
			return false;
		}
		sprintf(buff,"%s",RATE_TABLE[TxDataRate]);
		bRet = (setWlIovar(rate, buff)? false : true);			
	}
	return bRet;
}

DLLExport bool LGE_RFT_SetPreamble(PreambleType preamble)
{
	char buff[BUFF_LEN];
	char *plcphdr = "plcphdr";
	bool bRet =  false;
	
	if (hDllInstance) {
		if ((preamble != Long) && (preamble != Short)) {
			DBGMSG(("Invalid input value for preamble\n"));
			return false;
		}
		sprintf(buff, "%s", PREAMBLE_TYPE[preamble]);
		bRet = (setWlIovar(plcphdr, buff) ? false : true);		
	}
	return bRet;	
}

DLLExport bool LGE_RFT_Channel(int ChannelNo)
{
	char buff[BUFF_LEN];
	char* channel = "channel";
	bool bRet =  false;

	if (hDllInstance) {
		if (ChannelNo >= 1 && ChannelNo <= 14){
			sprintf(buff,"%d",ChannelNo);
			bRet = (setWlIovar(channel, buff)? false : true);
		}
	}
	return bRet;
}

DLLExport bool LGE_RFT_TxGain(int TxGain)
{
	char buff[BUFF_LEN];
	char *txpwr = "txpwr1 -o -d";
	bool bRet =  false;
	
	if (hDllInstance) {
		if (TxGain >= 23 || TxGain <= 0) {
			DBGMSG(("Invalid input value for TxGain.\n"));
			return false;
		}
		sprintf(buff,"%d", TxGain);
		bRet = (setWlIovar(txpwr, buff)? false : true);
	}
	return bRet;		
}

DLLExport bool LGE_RFT_TxBurstInterval(int SIFS)
{
	/* BRCM specific restriction */
	if (SIFS < 20 || SIFS > 1000)
		return false;
	pkteng_txdata_cache.TxBurstInterval = SIFS;
	return true;
}

DLLExport bool LGE_RFT_TxPayloadLength(int TxPayLength)
{
	if (TxPayLength <= 0)
		return false;
	pkteng_txdata_cache.TxPayloadLength = TxPayLength;
	return true;
}

/* LGE_RFT_TxStart : Start the pktengine in Tx mode.
 * Note : 
 * Use LGE_RFT_SetMacAddress to set the source address in frames (Optional)
 * Use LGE_RFT_TxDestAddress to set the destination address in frames (Mandatory)
 * Use LGE_RFT_TxBurstInterval to set the burst interval (Mandatory)
 * Use LGE_RFT_TxPayloadLength to set the length of the frames to be sent (Mandatory)
 * Use LGE_RFT_TxFrames to set the number of frames to be sent. If set to 0, the pktengine 
 * 			continuously sends the packets, till it is interrupted by 
 * 			either LGE_RFT_TxStop or any other OIDs/IOCTLs/IOVARs (Mandatory)
 */
DLLExport bool LGE_RFT_TxStart()
{
	char buff[BUFF_LEN];
	char *pkteng = "pkteng_start";
	char dest_addr[ETHER_ADDR_LEN * 3];
	char src_addr[ETHER_ADDR_LEN * 3];
	bool bRet =  false;
	
	if (hDllInstance) {
		strcpy(dest_addr, ether_etoa(&pkteng_txdata_cache.DestinationMacAddr));
		strcpy(src_addr, ether_etoa(&pkteng_txdata_cache.SourceMacAddr));

		sprintf(buff,"%s tx %d %d %d %s", dest_addr, pkteng_txdata_cache.TxBurstInterval,
			pkteng_txdata_cache.TxPayloadLength, pkteng_txdata_cache.TxFrames,
			src_addr);
		DBGMSG(("Command = %s", buff));
		bRet = (setWlIovar(pkteng, buff)? false : true);		
	}
	return bRet;	
}

/* LGE_RFT_TxStop : Stop the Tx test using pktengine
 */
DLLExport bool LGE_RFT_TxStop()
{
	char buff[BUFF_LEN] = "tx";
	char *pkteng = "pkteng_stop";
	bool bRet =  false;
	if (hDllInstance) {
		bRet = (setWlIovar(pkteng, buff)? false : true);		
	}
	return bRet;
}

/* LGE_RFT_RxStart : Start the pktengine to receive the frames 
 * Note : 
 * Use LGE_RFT_UseAcks() to send acknowledgements.
 * Use LGE_RFT_SetMacAddress() to receive only packets destined for this MAC address. (Mandatory)
 *
 * Setup :
 * Configure the frame generator (such as Agilent) to send out 
 * a.) frames with sequence numbers in frames (this will be used to identify the missing frames)
 * b.) data frames
 * c.) with proper destination address set
 */
DLLExport bool LGE_RFT_RxStart()
{
	char buff[BUFF_LEN];
	char pkteng[] = "pkteng_start";
	char *src_addr;
	bool bRet =  false;
	
	pktengrxducast_delta = 0;
	rxbadfcs_delta = 0;
	rxbadplcp_delta = 0;

	if (hDllInstance) {
		bRet = read_counter(GOOD_FRAMES_CNT, &pktengrxducast);
		if (!bRet) {
			DBGMSG(("LGE_RFT_RxStart : Error occured while reading counters (pktengrxducast).\n"));
			return bRet;
		}
		bRet = read_counter(BAD_FCS_CNT, &rxbadfcs);
		if (!bRet) {
			DBGMSG(("LGE_RFT_RxStart : Error occured while reading counters (rxbadfcs).\n"));
			return bRet;
		}
		bRet = read_counter(BAD_PLCP_CNT, &rxbadplcp);
		if (!bRet) {
			DBGMSG(("LGE_RFT_RxStart : Error occured while reading counters (rxbadplcp).\n"));
			return bRet;
		}
		src_addr = ether_etoa(&pkteng_rxdata_cache.DestinationMacAddr);
		if (pkteng_rxdata_cache.UseAcks)
			sprintf(buff, "%s rxwithack", src_addr);
		else 
		sprintf(buff, "%s rx", src_addr);
		DBGMSG(("Command = %s", buff));
		bRet = (setWlIovar(pkteng, buff)? false : true);		
	}
	return bRet;
}

/* LGE_RFT_RxStop : Stop the Rx test using pktengine
 */
DLLExport bool LGE_RFT_RxStop()
{
	char buff[BUFF_LEN] = "rx";
	char pkteng_rxducast[] = "pktengrxducast";
	char *pkteng = "pkteng_stop";
	bool bRet =  false;
	int new_count;
	if (hDllInstance) {
		bRet = (setWlIovar(pkteng, buff)? false : true);
		if (!bRet)
			return bRet;

		bRet = read_counter(GOOD_FRAMES_CNT, &new_count);
		if (!bRet) {
			DBGMSG((" LGE_RFT_RxStop : Error occured while reading counters (pktengrxducast).\n"));
			return bRet;
		}
		pktengrxducast_delta = new_count - pktengrxducast;
		
		bRet = read_counter(BAD_FCS_CNT, &new_count);
		if (!bRet) {
			DBGMSG((" LGE_RFT_RxStop : Error occured while reading counters (rxbadfcs).\n"));
			return bRet;
		}
		rxbadfcs_delta = new_count - rxbadfcs;
		
		bRet = read_counter(BAD_PLCP_CNT, &new_count);
		if (!bRet) {
			DBGMSG((" LGE_RFT_RxStop : Error occured while reading counters (rxbadplcp).\n"));
			return bRet;
		}
		rxbadplcp_delta = new_count - rxbadplcp;
	}
	return bRet;	
}

/* LGE_RFT_FRError : Provides the number of lost frames in Rx test
 * Note :
 * This API needs to be called only after LGE_RFT_RxStart()/RxStop() sequence.
 */
DLLExport bool LGE_RFT_FRError(int* FError)
{
	bool bRet =  false;
	if (hDllInstance) {
		*FError = rxbadfcs_delta + rxbadplcp_delta;
		bRet = TRUE;
	}
	return bRet;
}

/* LGE_RFT_FRGood : Provides the number of directed good frames in Rx test
 * This API needs to be called only after LGE_RFT_RxStart()/RxStop() sequence.
 */
DLLExport bool LGE_RFT_FRGood( int* FRGood)
{
	*FRGood = pktengrxducast_delta;
	return true;
}

/* LGE_RFT_FRPktengRxRSSI: Provides the RSSI value of the recevied packets during a pkteng
 * test
 */
DLLExport bool LGE_RFT_PktengRxRSSI(int* RSSI)
{
	char buff[BUFF_LEN];
	bool bRet = false;
	char iovar[] = "pkteng_stats";
	char *rssi_ptr;
	const char *rssi_str = "RSSI ";
	if (hDllInstance) {
		if(getWlIovar(iovar, NULL, 0, buff)) {
			DBGMSG(("LGE_RFT_PktengRxRSSI : Failure during getWIovar(pkteng_stats).\n"));
			return false;
		}
		if(rssi_ptr = strstr(buff, rssi_str)) {
			bRet = sscanf(rssi_ptr + strlen(rssi_str), "%d", RSSI);
			if (*RSSI < 0)
				*RSSI = -(*RSSI);
			DBGMSG(("LGE_RFT_PktengRxRSSI : %s = %d\n", rssi_str, *RSSI));
		} else {
			DBGMSG(("LGE_RFT_PktengRxRSSI : Failed reading RSSI.\n"));
		}
	}
	return bRet;	
}

/* LGE_RFT_SetMacAddress : Overrides the STA's MAC address, during pktengine tests
 * Paramerters : 
 * Addr - MAC address to use
 */
DLLExport bool LGE_RFT_SetMacAddress(EtherAddr Addr)
{
	memcpy(&pkteng_txdata_cache.SourceMacAddr, &Addr, ETHER_ADDR_LEN);
	memcpy(&pkteng_rxdata_cache.DestinationMacAddr, &Addr, ETHER_ADDR_LEN);
	DBGMSG(("pkteng_txdata_cache.SourceMacAddr = %s\n", ether_etoa(&pkteng_txdata_cache.SourceMacAddr)));
	DBGMSG(("pkteng_rxdata_cache.DestinationMacAddr = %s\n", ether_etoa(&pkteng_rxdata_cache.DestinationMacAddr)));
	return true;
}

/* LGE_RFT_UseAcks : Use it to explicity enable or disable ACKing, when running Rx pktengine tests
 * 		     The default behavior is not to ack.
 * Parameters :
 * UseAcks - true : ACK during Rx tests. 
 * 	     false : Do not ACK during Rx tests.
 */
DLLExport bool LGE_RFT_UseAcks(bool UseAcks)
{
	pkteng_rxdata_cache.UseAcks = UseAcks;
	return true;
}

DLLExport bool LGE_RFT_TxBurstFrames(int Frames)
{
	if (Frames < 0)
		return false;
	pkteng_txdata_cache.TxFrames = Frames;
	return true;
}

DLLExport bool LGE_RFT_TxDestAddress(EtherAddr DestAddr)
{
	memcpy(&pkteng_txdata_cache.DestinationMacAddr, &DestAddr, ETHER_ADDR_LEN);
	return true;
}

DLLExport bool LGE_RFT_RemoteWl(char *command, char *output)
{
	bool bRet =  false;
	if (hDllInstance) {
		if(getWlIovar(command, NULL, 0, output) == 0)
			bRet = true;		
	}
	return bRet;
}

HINSTANCE LoadDLL() 
{
	HINSTANCE dllHndl = LoadLibrary(TEXT(MFGTEST_API_DLL));
	if (dllHndl == NULL) {
//		printf("Error loading %s\n", MFGTEST_API_DLL);
		return NULL;
	}

	if ((selectInterface = (selectInterface_t) GetProcAddress (dllHndl, WIDE_CHAR("selectInterface"))) == NULL) {
//		printf("Function %s address is invalid.\n", "selectInterface");
		FreeLibrary(dllHndl);
		return NULL;
	}
	
	if ((openAdapter = (openAdapter_t) GetProcAddress (dllHndl, WIDE_CHAR("openAdapter"))) == NULL) {
//		printf("Function %s address is invalid.\n", "openAdapter");
		FreeLibrary(dllHndl);
		return NULL;
	}

	if ((closeAdapter = (closeAdapter_t) GetProcAddress (dllHndl, WIDE_CHAR("closeAdapter"))) == NULL) {
//		printf("Function %s address is invalid.\n", "closeAdapter");
		FreeLibrary(dllHndl);
		return NULL;
	}

	if ((runTest = (runTest_t) GetProcAddress (dllHndl, WIDE_CHAR("runTest"))) == NULL) {
//		printf("Function %s address is invalid.\n", "runTest");
		FreeLibrary(dllHndl);
		return NULL;
	}

	if ((getTxPERResult = (getTxPERResult_t) GetProcAddress (dllHndl, WIDE_CHAR("getTxPERResult"))) == NULL) {
//		printf("Function %s address is invalid.\n", "getTxPERResult");
		FreeLibrary(dllHndl);
		return NULL;
	}

	if ((setTxPERResult = (setTxPERResult_t) GetProcAddress (dllHndl, WIDE_CHAR("setTxPERResult"))) == NULL) {
//		printf("Function %s address is invalid.\n", "setTxPERResult");
		FreeLibrary(dllHndl);
		return NULL;
	}

	
	if ((getRxPERResult = (getRxPERResult_t) GetProcAddress (dllHndl, WIDE_CHAR("getRxPERResult"))) == NULL) {
//		printf("Function %s address is invalid.\n", "getRxPERResult");
		FreeLibrary(dllHndl);
		return NULL;
	}

	if ((setRxPERResult = (setRxPERResult_t) GetProcAddress (dllHndl, WIDE_CHAR("setRxPERResult"))) == NULL) {
//		printf("Function %s address is invalid.\n", "setRxPERResult");
		FreeLibrary(dllHndl);
		return NULL;
	}

	if ((setWlIovar = (setWlIovar_t) GetProcAddress (dllHndl, WIDE_CHAR("setWlIovar"))) == NULL) {
//		printf("Function %s address is invalid.\n", "setWlIovar");
		FreeLibrary(dllHndl);
		return NULL;
	}

	if ((getWlIovar = (getWlIovar_t) GetProcAddress (dllHndl, WIDE_CHAR("getWlIovar"))) == NULL) {
//		printf("Function %s address is invalid.\n", "getWlIovar");
		FreeLibrary(dllHndl);
		return NULL;
	}

	if ((setWlIoctl = (setWlIoctl_t) GetProcAddress (dllHndl, WIDE_CHAR("setWlIoctl"))) == NULL) {
//		printf("Function %s address is invalid.\n", "setWlIoctl");
		FreeLibrary(dllHndl);
		return NULL;
	}

	if ((getWlIoctl = (getWlIoctl_t) GetProcAddress (dllHndl, WIDE_CHAR("getWlIoctl"))) == NULL) {
//		printf("Function %s address is invalid.\n", "getWlIoctl");
		FreeLibrary(dllHndl);
		return NULL;
	}

	
	if ((cmdSeqStart = (cmdSeqStart_t) GetProcAddress (dllHndl, WIDE_CHAR("cmdSeqStart"))) == NULL) {
//		printf("Function %s address is invalid.\n", "cmdSeqStart");
		FreeLibrary(dllHndl);
		return NULL;
	}

	/*
	if ((cmdSeqStop = (cmdSeqStop_t) GetProcAddress (dllHndl, WIDE_CHAR("cmdSeqStop"))) == NULL) {
//		printf("Function %s address is invalid.\n", "cmdSeqStop");
		FreeLibrary(dllHndl);
		return NULL;
	}

	if ((cmdSeqDelay = (cmdSeqDelay_t) GetProcAddress (dllHndl, WIDE_CHAR("cmdSeqDelay"))) == NULL) {
//		printf("Function %s address is invalid.\n", "cmdSeqDelay");
		FreeLibrary(dllHndl);
		return NULL;
	}
*/
	return dllHndl;
}
