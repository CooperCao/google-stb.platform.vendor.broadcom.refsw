// WFITool.h : header file
//

#if !defined(WFITOOL_H)
#define WFITOOL_H


#define MAXCHARLEN			            1024


/*******************************************
*  WFI Tool Version Definition:
*******************************************/
#define WFI_TOOL_VERSION                0x01

/*******************************************
*  WFI Tool Server/Daemon Port:
*******************************************/
#define WFI_TOOL_PORT                   9966

/*******************************************
*  Message types:
*******************************************/
#define WFI_TOOL_LIST_REQUEST			0x00
#define WFI_TOOL_LIST_RESPONSE			0x01

#define WFI_TOOL_INVITE_REQUEST			0x02
#define WFI_TOOL_INVITE_RESPONSE		0x03

#define WFI_TOOL_CHECK_REQUEST			0x04
#define WFI_TOOL_CHECK_RESPONSE			0x05

#define WFI_TOOL_CANCEL_REQUEST			0x06
#define WFI_TOOL_CANCEL_RESPONSE		0x07

#define WFI_TOOL_ENABLE_REQUEST			0x08
#define WFI_TOOL_ENABLE_RESPONSE		0x09

#define WFI_TOOL_DISABLE_REQUEST		0x0a
#define WFI_TOOL_DISABLE_RESPONSE		0x0b

#define MAXDATALEN						1600
#define ETHER_ADDR_LEN					6

#define MAXPHYUNIT						2
#define DEFAULTUNIT						0
#define MAXWIFIINVTESTA                                         20

/* 
 * Invite status 
 */
#define INVITING						10
#define SUCCESSFUL 						0
#define TIMEOUT 						1
#define NORESPONSE 						2
#define	STAREJECT						3
#define INVITEERROR 					20

/*
 * timers
 */

#define TIMER_REFRESH_INTERVAL			15000
#define TIMER_CHECK_INTERVAL			5000

#define TIMERREFRESH					1
#define TIMERCHECK						2

#define CHECKTIMEOUT					120
#define TIMETOTRY						3

/*
 * For display messages, 0 and 1 are reserved.
 */

#define	FOUNDNOSTA						2
#define	INVITEINPROGRESS				3
#define	CANCELLING						4
#define	CANCELLED						5
#define	INVITINGERROR					6
#define	INVITESUCCESSFUL				7
#define FAILEDTIMEOUT					8
#define	FAILEDREJECT					9
#define	FAILEDNORESPONSE				10
#define	SOCKETDLLERR					11
#define	SOCKETCREATEERR					12
#define	SOCKEOPTERR						13
#define	SOCKETADDRERR					14
#define	SOCKETBINDRERR					15
#define	APNORESPOND						16
#define	ADAPTERERR						17
#define FAILEDRECEIVE					18
#define NETWORKUNREACH					19
#define FAILEDSEND						20
#define ADAPTERERROR					21
#define APRESPOND						22
#define	FOUNDSTAS						23
#define SCANNING						24

struct AdapterInfos { 
	char 	name[10];				// display name
	char 	IP[20];				// IP address
	char  	gateway[20];			// Gateway IP address
	char 	subnet[20];				// subnet mask
};


struct wfi_sta_info {
	char 	name[33];					//friendly name
	unsigned char ea[ETHER_ADDR_LEN];	//mac address
};

struct wfi_sta_info_list {
	char unit;						//physical interface unit #
	unsigned char num;					//number of STAs
	struct wfi_sta_info wfi_list[MAXWIFIINVTESTA];		//STA List
};

struct wfi_invited_sta_info {
	char name[33];						//friendly name
	unsigned char ea[ETHER_ADDR_LEN];	//mac address
	char unit[8];						//wl_unit
};
#endif // !defined(WFITOOL_H)
