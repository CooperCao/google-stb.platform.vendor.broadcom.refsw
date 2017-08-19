// mfgtestLGE.cpp : Console application for SVT
//

#include "stdafx.h"
#include "../../../include/lgeMfgTestApi.h"
#include "stdlib.h"
#include "stdio.h"

#define LGE_MFGTEST_API_DLL "MfgTestLibLGE.dll"

HINSTANCE LoadDLL();

typedef bool (*LGE_OpenDUT)();
typedef bool (*LGE_CloseDUT)();
typedef bool (*LGE_TxDataRate)(int TxDataRate);
typedef bool (*LGE_SetPreamble)(PreambleType preamble);
typedef bool (*LGE_Channel)(int ChannelNo);
typedef bool (*LGE_TxGain)(int TxGain);

typedef bool (*LGE_SetMacAddress)(EtherAddr Addr);

typedef bool (*LGE_TxBurstFrames)(int Frames);	/* Frames in a Tx burst */
typedef bool (*LGE_TxDestAddress)(EtherAddr DestAddr);
typedef bool (*LGE_TxBurstInterval)(int SIFS);
typedef bool (*LGE_TxPayloadLength)(int TxPayLength);
typedef bool (*LGE_TxStart)();
typedef bool (*LGE_TxStop)();

typedef bool (*LGE_UseAcks)(bool UseAcks);
typedef bool (*LGE_RxStart)();
typedef bool (*LGE_RxStop)();
typedef bool (*LGE_FRError)(int* FError);
typedef bool (*LGE_FRGood)( int* FRGood);
typedef bool (*LGE_PktengRxRSSI)(int* RSSI);

typedef bool (*LGE_RemoteWl)(char *command, char *output);

char* commands[] = {
	"",				/* Intentionally left blank */
	"LGE_OpenDUT",
	"LGE_CloseDUT",
	"LGE_TxDataRate",
	"LGE_SetPreamble",
	"LGE_Channel",
	"LGE_TxGain",
	"LGE_SetMacAddress",
	
	"LGE_TxBurstFrames",
	"LGE_TxDestAddress",
	"LGE_TxBurstInterval",
	"LGE_TxPayloadLength",
	"LGE_TxStart",
	"LGE_TxStop",
	
	"LGE_UseAcks",
	"LGE_RxStart",
	"LGE_RxStop",
	"LGE_FRError",
	"LGE_FRGood",
	"LGE_PktengRxRSSI",

	"LGE_RemoteWl",
	"Exit",
	"Continuous PER Test (Note : Not an API. For use by SVT)"
};

/* Currently ordering is very important between commands[] and this enum */
enum {
	enum_LGE_OpenDUT = 1,
	enum_LGE_CloseDUT,
	enum_LGE_TxDataRate,
	enum_LGE_SetPreamble,
	enum_LGE_Channel,
	enum_LGE_TxGain,
	enum_LGE_SetMacAddress,
	
	enum_LGE_TxBurstFrames,
	enum_LGE_TxDestAddress,
	enum_LGE_TxBurstInterval,
	enum_LGE_TxPayloadLength,
	enum_LGE_TxStart,
	enum_LGE_TxStop,
	
	enum_LGE_UseAcks,
	enum_LGE_RxStart,
	enum_LGE_RxStop,
	enum_LGE_FRError,
	enum_LGE_FRGood,
	enum_LGE_PktengRxRSSI,

	enum_LGE_RemoteWl,
	enum_Exit,
	enum_PER_TEST,
	enum_End
};

LGE_OpenDUT  lgeOpenDUT = 0;
LGE_CloseDUT lgeCloseDUT = 0; 
LGE_TxDataRate lgeTxDataRate = 0;
LGE_SetPreamble lgeSetPreamble = 0;
LGE_Channel  lgeChannel = 0;
LGE_TxGain   lgeTxGain = 0;
LGE_TxBurstInterval lgeTxBurstInterval = 0;
LGE_TxPayloadLength lgeTxPayloadLength = 0;
LGE_TxStart  lgeTxStart = 0;
LGE_TxStop   lgeTxStop = 0;
LGE_RxStart  lgeRxStart = 0;
LGE_RxStop   lgeRxStop = 0;
LGE_FRError  lgeFRError = 0;
LGE_FRGood   lgeFRGood = 0;
LGE_PktengRxRSSI lgePktengRxRSSI = 0;

LGE_TxBurstFrames  lgeTxBurstFrames = 0;
LGE_TxDestAddress  lgeTxDestAddress = 0;
LGE_SetMacAddress   lgeSetMacAddress = 0;
LGE_UseAcks 	   lgeUseAcks = 0;
LGE_RemoteWl   lgeRemoteWl = 0;

int 
readInt(const char *prompt)
{
	int i;
	printf("Enter the setting for %s : ", prompt);
	scanf("%d", &i);
	return i;
}
char* 
readString(const char *prompt, char *str)
{
	printf("Enter the setting for %s : ", prompt);
	scanf("%s", str);
	return str;
}
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
int main(int argc, char **argv)
{
	HINSTANCE dllHndl;
	ether_addr eth_addr = {0x0,0x15,0xc5,0x71,0xb6,0xb4};
	char eth_addr_str[32];

	if ((dllHndl = LoadDLL()) == NULL) {
		printf("Failed to load %s.\n", LGE_MFGTEST_API_DLL);
		return -1;
	}

	printf("Manufacturing Test API (For LG) - Demo App\n");
		
	while (1) {
	printf("-------------------------------------------\n");
		printf("Options :\n");
		for (int i = 1; i < enum_End; i++) {
			printf("%d.)%s\n", i, commands[i]); 
		}
		fflush(stdin);
		printf("Select an option :");
		if(scanf("%d", &i) != 1) {
			printf("Invalid input\n");
			continue;
		}
	
		printf("\n");
		switch (i) {
			case enum_LGE_OpenDUT :
				if(lgeOpenDUT()) {
					printf("Opening DUT \t\t\t: Done\n");		
				} else {
					printf("LGE_RFT_OpenDUT Failed\n");
					lgeCloseDUT();
					return -1;
				}
				break;

			case enum_LGE_CloseDUT :
				printf("\nClosing DUT \t\t\t: %s", (lgeCloseDUT()? "Done\n" : "Failed\n"));
				break;

			case enum_LGE_TxDataRate :
				printf("\nSetting Tx Data rate\t\t: %s", (lgeTxDataRate(readInt("TxDataRate")) ? "Done\n" : "Failed\n"));
				break;

			case enum_LGE_SetPreamble :
	printf("\nSetting Preamble\t: %s", (lgeSetPreamble((PreambleType)readInt("Preamble"))? "Done\n" : "Failed\n"));
				break;

			case enum_LGE_Channel :
	printf("\nSetting Channel\t\t: %s", (lgeChannel(readInt("Channel")) ? "Done\n" : "Failed\n"));
				break;

			case enum_LGE_TxGain :
				printf("\nSetting TxGain\t\t: %s", (lgeTxGain(readInt("TxGain"))? "Done\n" : "Failed\n"));
				break;

			case enum_LGE_SetMacAddress :
				readString("DUT's MAC address", eth_addr_str);
				if(ether_atoe(eth_addr_str, &eth_addr)) {
					lgeSetMacAddress(eth_addr);
					printf("\nSetting DUT's MAC address\t\t: Done\n");
				} else
					printf("\nSetting DUT's MAC address\t\t: Failed\n");
				break;
			
			case enum_LGE_TxBurstFrames :
				printf("\nSetting lgeTxBurstFrames\t: %s", (lgeTxBurstFrames(readInt("lgeTxBurstFrames"))? "Done\n" : "Failed\n"));
				break;
	
			case enum_LGE_TxDestAddress :
				readString("Destination ethernet address", eth_addr_str);
				if(ether_atoe(eth_addr_str, &eth_addr)) {
					printf("\nSetting TxDestAddress \t\t: %s", (lgeTxDestAddress(eth_addr)? "Done\n" : "Failed\n"));
				} else
					printf("\nSetting TxDestAddress\t\t: Failed\n");
				break;

			case enum_LGE_TxBurstInterval :
				printf("\nSetting TxBurstInterval : %s", (lgeTxBurstInterval(readInt("TxBurstInterval"))? "Done\n" : "Failed\n"));
				break;

			case enum_LGE_TxPayloadLength :
				printf("\nSetting TxPayloadLength : %s", (lgeTxPayloadLength(readInt("TxPayloadLength"))? "Done\n" : "Failed\n"));
				break;

			case enum_LGE_TxStart :
				printf("\nSetting TxStart \t\t: %s", (lgeTxStart()? "Done\n" : "Failed\n"));
				break;

			case enum_LGE_TxStop :
				printf("\nSetting TxStop \t\t\t: %s", (lgeTxStop()? "Done\n" : "Failed\n"));
				break;
			
			case enum_LGE_UseAcks :
				printf("\nSetting UseAcks : %s", (lgeUseAcks(readInt("UseAcks"))? "Done\n" : "Failed\n"));
				break;

			case enum_LGE_RxStart :
				printf("\nSetting RxStart \t\t: %s", (lgeRxStart()? "Done\n" : "Failed\n"));
				break;

			case enum_LGE_RxStop :
				printf("\nSetting RxStop \t\t\t: %s", (lgeRxStop()? "Done\n" : "Failed\n"));
				break;

			case enum_LGE_FRError : 
				{
					int badFrames;
					printf("\nGetting FRError \t\t: %s", (lgeFRError(&badFrames)? "Done\n" : "Failed\n"));
					printf("\nNumber of bad frames = %d\n", badFrames);
				}
				break;

			case enum_LGE_FRGood :
				int goodFrames;
				printf("\nGetting FRGood \t\t\t: %s", (lgeFRGood(&goodFrames)? "Done\n" : "Failed\n"));
				printf("\nNumber of good frames = %d\n", goodFrames);
				break;

			case enum_LGE_PktengRxRSSI :
				{
					int rssi;
					printf("\nGetting RSSI \t\t\t: %s", (lgePktengRxRSSI(&rssi)? "Done\n" : "Failed\n"));
					printf("\nRSSI = %d\n", rssi);
				}
				break;

			case enum_LGE_RemoteWl :
				{
					char command[8048], output[8048];
					int index = 0;
					fflush(stdin);
					printf("Enter the command (without wl prefix) : ");
					/* Read the entire line */
					while('\n' !=  (command[index++] = getchar())) {
					}
					command[--index] = '\0';
					if(strcmp(command, "exit") == 0 || strcmp(command, "quit") == 0)
						break;
					if(lgeRemoteWl(command, output))
						printf("%s\n", output);
					else
						printf("Command execution failed\n");
					
				}
				break;

			case enum_Exit :
				return 0;
				break;

			case enum_PER_TEST:
				{
					for(int i = 0; i < 100; i++) {
						int badFrames;
						int goodFrames;
						lgeRxStart();
						Sleep(15000);
						lgeRxStop();
						lgeFRError(&badFrames);
						lgeFRGood(&goodFrames);
						printf("Minutes elapsed = %d, Good Frames = %d\t Bad Frames = %d, PER = %f\n", i/4, goodFrames, badFrames, ((float)badFrames / (badFrames + goodFrames + 1)) * 100);
					}
				}
				break;

			default :
				printf("Invalid choice\n");
				break;
		}
	}
	return 0;
}

HINSTANCE LoadDLL() 
{
	HINSTANCE dllHndl = LoadLibrary(TEXT(LGE_MFGTEST_API_DLL));
	if (dllHndl == NULL) {
		return NULL;
	}

	lgeOpenDUT  = (LGE_OpenDUT)GetProcAddress(dllHndl, WIDE_CHAR("LGE_RFT_OpenDUT"));
	lgeCloseDUT = (LGE_CloseDUT)GetProcAddress(dllHndl, WIDE_CHAR("LGE_RFT_CloseDUT"));
	lgeTxDataRate = (LGE_TxDataRate)GetProcAddress(dllHndl, WIDE_CHAR("LGE_RFT_TxDataRate"));
	lgeSetPreamble = (LGE_SetPreamble)GetProcAddress(dllHndl, WIDE_CHAR("LGE_RFT_SetPreamble"));
	lgeChannel = (LGE_Channel)GetProcAddress(dllHndl, WIDE_CHAR("LGE_RFT_Channel"));
	lgeTxGain = (LGE_TxGain)GetProcAddress(dllHndl, WIDE_CHAR("LGE_RFT_TxGain"));
	lgeTxBurstInterval = (LGE_TxBurstInterval)GetProcAddress(dllHndl, WIDE_CHAR("LGE_RFT_TxBurstInterval"));
	lgeTxPayloadLength = (LGE_TxPayloadLength)GetProcAddress(dllHndl, WIDE_CHAR("LGE_RFT_TxPayloadLength"));
	lgeTxStart = (LGE_TxStart)GetProcAddress(dllHndl, WIDE_CHAR("LGE_RFT_TxStart"));
	lgeTxStop = (LGE_TxStop)GetProcAddress(dllHndl, WIDE_CHAR("LGE_RFT_TxStop"));
	lgeRxStart = (LGE_RxStart)GetProcAddress(dllHndl, WIDE_CHAR("LGE_RFT_RxStart"));
	lgeRxStop = (LGE_RxStop)GetProcAddress(dllHndl, WIDE_CHAR("LGE_RFT_RxStop"));
	lgeFRError = (LGE_FRError)GetProcAddress(dllHndl, WIDE_CHAR("LGE_RFT_FRError"));
	lgeFRGood = (LGE_FRGood)GetProcAddress(dllHndl, WIDE_CHAR("LGE_RFT_FRGood"));
	lgePktengRxRSSI = (LGE_PktengRxRSSI)GetProcAddress(dllHndl, WIDE_CHAR("LGE_RFT_PktengRxRSSI"));
	lgeTxBurstFrames =  (LGE_TxBurstFrames)GetProcAddress(dllHndl, WIDE_CHAR("LGE_RFT_TxBurstFrames"));
	lgeTxDestAddress = (LGE_TxDestAddress)GetProcAddress(dllHndl, WIDE_CHAR("LGE_RFT_TxDestAddress"));
	lgeSetMacAddress  = (LGE_SetMacAddress)GetProcAddress(dllHndl, WIDE_CHAR("LGE_RFT_SetMacAddress"));
	lgeUseAcks  = (LGE_UseAcks)GetProcAddress(dllHndl, WIDE_CHAR("LGE_RFT_UseAcks"));
	lgeRemoteWl = (LGE_RemoteWl)GetProcAddress(dllHndl, WIDE_CHAR("LGE_RFT_RemoteWl"));

	if ((lgeOpenDUT == NULL)   ||
		(lgeCloseDUT == NULL)  ||
		(lgeSetPreamble == NULL)  ||
		(lgeChannel == NULL)  ||
		(lgeTxGain == NULL)  ||
		(lgeTxBurstInterval == NULL)  ||
		(lgeTxPayloadLength == NULL)  ||
		(lgeTxStart == NULL)  ||
		(lgeTxStop == NULL)  ||
		(lgeRxStart == NULL)  ||
		(lgeRxStop == NULL)  ||
		(lgeFRError == NULL)  ||
		(lgeFRGood == NULL)  ||
		(lgePktengRxRSSI == NULL) ||
		(lgeTxBurstFrames == NULL)  ||
		(lgeTxDestAddress == NULL)  ||
		(lgeSetMacAddress == NULL) ||
		(lgeUseAcks == NULL) ||
		(lgeRemoteWl == NULL)) {
			FreeLibrary(dllHndl);
			return 0;
		}

	return dllHndl;
}
