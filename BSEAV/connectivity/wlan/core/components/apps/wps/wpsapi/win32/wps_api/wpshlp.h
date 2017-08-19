#pragma once

#define SESHLPAPI 

#define WPS_SDK 1
#include "..\..\..\wl\cpl\preflib\preflib.h"

//#ifdef _DEBUG
//#define TRACE DebugOutput
//#else
#define TRACE 1 ? (void)0 : DebugOutput
//#endif

#define BRCM_ADAPTER_TEXT 256

typedef enum {
	BRCM_AT_UNKNOWN = -1,
	BRCM_AT_802_3 = 0x0,
	BRCM_AT_ILINE,
	BRCM_AT_ENET,
	BRCM_AT_CODEC,
	BRCM_AT_WIRELESS,
	BRCM_AT_GPRS,
	BRCM_AT_USB
} BRCM_ADAPTER_TYPE, *PBRCM_ADAPTER_TYPE;

typedef struct _BRCM_ADAPTER {
	CHAR shortname[10];
	CHAR displayname[BRCM_ADAPTER_TEXT];
	CHAR name[BRCM_ADAPTER_TEXT];
	CHAR description[BRCM_ADAPTER_TEXT];
	BYTE macaddr[6];
	BRCM_ADAPTER_TYPE type;
	BOOL valid;
} BRCM_ADAPTER;

#define RADIOSTATE_SW_DISABLE	(1<<0)
#define RADIOSTATE_HW_DISABLE	(1<<1)

#define MGMTSTATE_RUN_BCM		(1<<0)
#define MGMTSTATE_RUN_WZC		(1<<1)
#define MGMTSTATE_RUN_ACM		(1<<2)

#define MGMTSTATE_MANAGE_NONE	0
#define MGMTSTATE_MANAGE_BCM	1
#define MGMTSTATE_MANAGE_WZC	2
#define MGMTSTATE_MANAGE_ACM	3

#ifdef __cplusplus
extern "C" {
#endif

SESHLPAPI int brcm_get_adapter_list(BRCM_ADAPTER devlist[], PDWORD pdwSize);

SESHLPAPI PREF_LIST_ST* GetPreferredNetworksList(LPCSTR adapter, LPCSTR sid);
SESHLPAPI PN_STATUS AddPreferredNetwork(LPCSTR adapter, LPCSTR sid, LPPREF_ST pNewPref, int nIndex);
SESHLPAPI PN_STATUS RemovePreferredNetwork(LPCSTR adapter, LPCSTR sid, DWORD nIndex);
SESHLPAPI BOOL FindPreferredNetwork(LPCSTR adapter, LPCSTR sid, LPCSTR ssid, DWORD *nIndex);

SESHLPAPI int GetRadioState(LPCTSTR szIfName, PDWORD pdwState);
SESHLPAPI int SetRadioState(LPCTSTR szIfName, DWORD dwState);
SESHLPAPI int GetManagementState(LPCTSTR szIfName, PDWORD pdwState);
SESHLPAPI int SetManagementState(LPCTSTR szIfName, DWORD dwState);
SESHLPAPI int IsUserPN(BOOL* pbResult);
void InitializeBCM();
DWORD GetPrefStructureSizeByVer(const DWORD dwClientVer);

#ifdef __cplusplus
}
#endif
