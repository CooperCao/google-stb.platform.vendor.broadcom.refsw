
#ifndef __BCM_MFG_API_H__
#define __BCM_MFG_API_H__

#include "bcmMfgCommon.h"

typedef char * string;
typedef UINT8 MACADDRESS[6];
typedef UINT32 CHANNEL;

typedef enum {
	BAND_2G = 2,
	Band_5G = 5,
} BAND ;

typedef enum {
	TEST_RATE_1M = 0, 	/* 1Mbits */
	TEST_RATE_2M, 		/* 2Mbits */
	TEST_RATE_5M, 		/* 5.5Mbits */
	TEST_RATE_6M,	 	/* 6Mbits */
	TEST_RATE_9M,  		/* 9Mbits */
	TEST_RATE_11M,  	/* 11Mbits */
	TEST_RATE_12M, 	 	/* 12Mbits */
	TEST_RATE_18M, 	 	/* 18Mbits */
	TEST_RATE_22M, 	 	/* 22Mbits */
	TEST_RATE_24M, 	 	/* 24Mbits */
	TEST_RATE_33M, 	 	/* 33Mbits */
	TEST_RATE_36M, 		/* 36Mbits */
	TEST_RATE_48M, 	 	/* 48Mbits */
	TEST_RATE_54M, 	 	/* 54Mbits */
} TEST_RATE;

typedef enum {
	REAMBLE_LONG = 0,
	PREAMBLE_SHORT = 1,
	PREAMBLE_AUTO = 2,
} PREAMBLE;

typedef enum  {
	FRAMETYPE_PKTENG = 0,
	FRAMETYPE_NORMAL_UDP = 1,
	FRAMETYPE_NORMAL_TCP = 2,
} FRAMETYPE;

typedef enum {
      CONNECTION_LOCAL = 0,
	  CONNECTION_REMOTE = 1,
} CONNECTION;

typedef enum  {
      IF_SDIO = 0,
      IF_UART = 1,
      IF_ETHER = 2,
      IF_UNKNOWN = 3,
} ADAPTER_INTERFACE_TYPE;


typedef enum  {
	CWMODE_CARRIER_TONE = 0,
	CWMODE_MODULATE_TONE = 1,
} CWMODE;

typedef enum  {
	TEST_PROGRAMOTP = 0, /*program either one word or a chunk of memeory */
	TEST_GET_OTPDUMP = 1,   /* show OTP content for verification */
	TEST_PROGRAMSROM = 2, /*program either one word or a chunk of memeory */
	TEST_GET_SROMDUMP = 3, /* show SROM content for verification */
	TEST_DUTINIT = 4, /*initializing DUT for Testing*/
	TEST_TXPERSTART = 5, /*set DUT to transmit at desired state*/
	TEST_TXPERSTOP = 6,  /*stop DUT transmit */
	TEST_RXPERSTART = 7, /*set DUT to transmit at desired state*/
	TEST_RXPERSTOP = 8,  /*stop DUT transmit */
	TEST_CARRIER_TONE_ON = 9, /*turn on carrier signal CW tone*/
	TEST_CARRIER_TONE_OFF = 10, /*turn off carrier signal CW tone*/ 
	TEST_EVM_ON = 11, /*turn on modulated CW tone for CCK/DSSS rates*/
	TEST_EVM_OFF = 12, /*turn off modulated CW tone for CCK/DSSS rates*/
	TEST_PD_GET_TSSI = 13, /*function required by Power detector test.  */
	TEST_PD_GET_ESTPOWER = 14, /*function required by Power detector test.  */
	TEST_PD_GET_PAPARAS = 15, /*function required by Power detector test.  */
	TEST_PD_SET_PAPARAS = 16, /*function required by Power detector test.  */
	TEST_PD_COMPUTE_PAPARA = 17, /*function required by Power detector test. */
} TEST_NAME;

typedef enum {
	PKTENG_RXWITHACK = 0,
	PKTENG_RXWITHOUTACK = 1,
} PKTENG_RXACKMODE;

typedef enum {
	RESULT_PASS = 0,
	RESULT_FAIL = 1,
} RESULTFLAG;

typedef struct {
	UINT32 nframes;
	UINT32 ifs_Delay;
	UINT32 frame_Length;
	MACADDRESS dest_Mac;
	MACADDRESS src_Mac;
} Pkteng_Tx_t;


typedef struct {
	PKTENG_RXACKMODE rx_Mode;
	MACADDRESS src_Mac;
} Pkteng_Rx_t;

typedef struct  {
	FRAMETYPE frame_Type;
	CWMODE cwMode_Type;
	BAND band;
	CHANNEL channel;
	TEST_RATE rate;
	PREAMBLE preamble;
	FLOAT32 power_Level;
	UINT32 length;
	UINT32 nframes;
	UINT32 delay;
	MACADDRESS dest_Mac;
	MACADDRESS src_Mac;
	Pkteng_Tx_t tx_Start;
	Pkteng_Rx_t rx_Start;
} Test_Packet_t;


typedef struct {
	UINT32 board_Rev;
	UINT32 chip_Rev;
	UINT32 srom_Rev;
	UINT8 *firmware_Rev;
	UINT32 device_Id;
	UINT32 vendor_Id;
	UINT32 subSystem_Id;
	MACADDRESS cur_EtherAddr;
} Device_Info_t;   /* hardware related device information */


typedef struct {
	Device_Info_t dev_Info;
	BOOL assoicated;  /* when linkup with REF is required */
	UINT8 *bssid;	/* when linkup with REF is required */
	UINT32 beacon_Rate;   /* when linkup with REF is required */
	UINT32 beacon_Interval; /* when linkup with REF is required */
	CONNECTION connection;	/* can be local or remote (serial/ethernet) */
	ADAPTER_INTERFACE_TYPE if_Type;
	UINT8 *port_Name; /*in case of serial port communication, COM1, COM2 */
	/*IP address */
} Adapter_Info_t ;


typedef struct {
	UINT8 *test_Name;
	Test_Packet_t test_Packet;
	RESULTFLAG flag;
	FLOAT32 test_Result;
	FLOAT32 low_Pass_Limit;
	FLOAT32 high_Pass_Limit;
} Test_Result_t;

typedef struct  {
	UINT32 nFrames;
	UINT32 nlostFrames;
	UINT32 nFCSError;
	UINT32 nPLCPError;
	UINT32 SNR;
	UINT32 RSSI;
} Per_Test_Result;

#ifndef BUILD_MFGAPI_DLL
typedef  BOOL (*selectInterface_t) (ADAPTER_INTERFACE_TYPE iftype, char *interfaceName);
typedef void (*setComPort_t) (char *Com_Port_No);
typedef void (*setRemoteIPAddress_t) (char *remoteIPAddr);
typedef BOOL (*openAdapter_t) (void);
typedef void (*closeAdapter_t) (void);
typedef BOOL (*runTest_t)(TEST_NAME testName, void *testParameters, Test_Result_t *testResult);
typedef Per_Test_Result* (*getTxPERResult_t) (void); /*query txper result */
typedef void (*setTxPERResult_t)(Per_Test_Result *perResult); /*set txper result by caller */
typedef Per_Test_Result* (*getRxPERResult_t) (void); /*query rxper result */
typedef void (*setRxPERResult_t)(Per_Test_Result *perResult); /*set rxper result by caller*/
typedef UINT32 (*setWlIovar_t) (const char *iovar,void *param);
typedef UINT32 (*getWlIovar_t) (const char *iovar, void *param, int param_len, void *bufptr);
typedef UINT32 (*setWlIoctl_t) (const char *ioctl,void *param);
typedef UINT32 (*getWlIoctl_t) (const char *ioctl, void *param, int param_len, void *bufptr);
typedef void (*cmdSeqStart_t)();
typedef void (*cmdSeqStop_t)();
typedef void (*cmdSeqDelay_t) (int delay_ms);
typedef void (*appendResult_t)(Test_Result_t *result);

selectInterface_t selectInterface;
setComPort_t setComPort;
setRemoteIPAddress_t setRemoteIPAddress;
openAdapter_t openAdapter;
closeAdapter_t closeAdapter;
runTest_t runTest;
getTxPERResult_t getTxPERResult;
setTxPERResult_t  setTxPERResult;
getRxPERResult_t getRxPERResult;
setRxPERResult_t setRxPERResult;
setWlIovar_t setWlIovar;
getWlIovar_t getWlIovar;
setWlIoctl_t setWlIoctl;
getWlIoctl_t getWlIoctl;
cmdSeqStart_t cmdSeqStart;
cmdSeqStop_t cmdSeqStop;
cmdSeqDelay_t cmdSeqDelay;
appendResult_t appendResult;

#else 
extern "C" {
	DLLExport bool selectInterface(ADAPTER_INTERFACE_TYPE iftype, char *interfaceName);
	DLLExport BOOL openAdapter();
	DLLExport void closeAdapter();
	DLLExport BOOL runTest(TEST_NAME testName, void *testParameters, Test_Result_t *testResult);
	DLLExport Per_Test_Result *getTxPERResult(); /*query txper result */
	DLLExport void setTxPERResult(Per_Test_Result *perResult); /*set txper result by caller */
	DLLExport Per_Test_Result *getRxPERResult(); /*query rxper result */
	DLLExport void setRxPERResult(Per_Test_Result *perResult); /*set rxper result by caller*/
	DLLExport UINT32 setWlIovar (const char *iovar,void *param);
	DLLExport UINT32 getWlIovar(const char *iovar, void *param, int param_len, void *bufptr);
	DLLExport UINT32 setWlIoctl (const char *ioctl,void *param);
	DLLExport UINT32 getWlIoctl(const char *ioctl, void *param, int param_len, void *bufptr);
	DLLExport void cmdSeqStart();
	DLLExport void cmdSeqStop();
	DLLExport void cmdSeqDelay(int delay_ms);
	DLLExport void appendResult (Test_Result_t *result);
}
#endif

#endif /* __BCM_MFG_API_H__ */
