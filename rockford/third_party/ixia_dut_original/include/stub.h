/*
  stub.c : This file contains utility functions that will be used to exchange
  messages between ANVL and the STUB application running on DUT

  Copyright (c) Ixia 2014
  All rights reserved.

*/
#ifndef __STUB_H__
#define __STUB_H__

typedef unsigned char           ubyte;
typedef char                byte;
typedef unsigned short          ubyte2;
typedef unsigned int        ubyte4;
typedef float               real4;
typedef double              real8;
typedef unsigned long long          long8;
typedef unsigned int                Size_t;/* largest contiguous piece of memory */
typedef unsigned char ubyte16 [16];

/* stub version v4/v6, default v4 */
#define STUB_IPV4 4
#define STUB_IPV6 6
ubyte stubIPVersion;

/* macros used in Stub Protocol Specification */
#define COMMAND  1
#define ACK      2
#define RESPONSE 3

#define CMD_BUFF_LEN_WITHOUT_PRM 8
#define CMD_BUFF_LEN1 12
#define CMD_BUFF_LEN2 32
#define MAX_PAD_LEN 8
#define CLEAR_DATA(dPtr) MemSet((void *)(dPtr), '\0', sizeof(*(dPtr)))
#define UBYTE4_MAXVAL           0xFFFFFFFF

#define OS_BIT_GET(x, n) ((((ubyte4)((x) & ((1) << (n)))) > 0 ) ? 1 : 0)

#define STUB_GET_TEST_NUMBER(test, major, minor)    \
{                                                     \
    ubyte4 index = 0, stringLen = 0;                  \
    byte *string = 0, *strPtr = 0, *majorStr = 0, *minorStr = 0;   \
    string = StrDup(test);                            \
    stringLen = StrLen(string);                       \
    strPtr = string + stringLen-1;                    \
    for(index = 0; index < stringLen; index++) {      \
        strPtr--;                                     \
        if((*strPtr) == '-') {                        \
            break;                                    \
        }                                             \
    }                                                 \
    strPtr++;                                         \
    majorStr = StrTok(strPtr, ".");                   \
    minorStr = StrTok(NULL, ".");                     \
    major = StrToL(majorStr,0,10);                    \
    minor = StrToL(minorStr,0,10);                    \
    free(string);                                     \
}

/* STUB command packet that will be exchanged between ANVL and STUB running on
 * DUT*/
struct STUBForm_s {
  ubyte2 msgType;     /* type of message (can be either command or response or acknowledgement */
  ubyte2 cmdID;   /* commandID for a particular STUB command e.g.*/
  ubyte2 reserved;
  ubyte2 paramsLen;    /* length of parameters */
  void *params;      /* field containing 0 or many subfields depending on the command */
};
typedef struct STUBForm_s STUBForm_t;

struct GetVerRespParamForm_s {
  ubyte2 majorVer;   /* major version of STUB application e.g. X.Y, X=major version*/
  ubyte2 minorVer;   /* minor version of STUB application e.g. X.Y Y=minor version*/
};
typedef struct GetVerRespParamForm_s GetVerRespParamForm_t;

struct StartEndTestReqParamForm_s {
  ubyte2 majorNum;   /* major number of test case number e.g. X.Y, X=major number*/
  ubyte2 minorNum;   /* minor number of test case number e.g. X.Y, Y=minor version*/
  ubyte4 suiteNameLen;
  ubyte *suiteName;
  ubyte *padding;
};
typedef struct StartEndTestReqParamForm_s StartEndTestReqParamForm_t;

struct ListenReqParamForm_s {
  ubyte2 portNum;   /* port number on which STUB application will listen to*/
  ubyte2 reserved;
};
typedef struct ListenReqParamForm_s ListenReqParamForm_t;

struct GetSampleDataRespParamForm_s {
  ubyte4 dataLen;   /* port number on which STUB application will listen to*/
  ubyte* data;
  ubyte* padding;
};
typedef struct GetSampleDataRespParamForm_s GetSampleDataRespParamForm_t;

struct GetDataLenRespParamForm_s {
  ubyte2 dataLen;   /* port number on which STUB application will listen to*/
  ubyte2 checksum;
};
typedef struct GetDataLenRespParamForm_s GetDataLenRespParamForm_t;

struct ConnectReqParamForm_s {
  ubyte4 anvlIPAddr;
  ubyte16 anvlIPv6Addr;
  ubyte4 portNum;
};
typedef struct ConnectReqParamForm_s ConnectReqParamForm_t;

struct SendRptDataReqParamForm_s {
  ubyte4 anvlIPAddr;
  ubyte16 anvlIPv6Addr;
  ubyte2 portNum;
  ubyte flags;
  ubyte2 dataLen;
  ubyte *data;
  ubyte2 reserved;
};
typedef struct SendRptDataReqParamForm_s SendRptDataReqParamForm_t;

struct SendDataReqParamForm_s {
  ubyte4 anvlIPAddr;
  ubyte16 anvlIPv6Addr;
  ubyte2 portNum;
  ubyte flags;
  ubyte reserved;
  ubyte2 dataLen;
  ubyte *data;
  ubyte *padding;
};
typedef struct SendDataReqParamForm_s SendDataReqParamForm_t;

struct SetTTLParamForm_s {
  ubyte ttl;
  ubyte reserved[3];
};
typedef struct SetTTLParamForm_s SetTTLParamForm_t;

#ifdef LIB_IPV6
struct SetHopLimitParamForm_s {
  ubyte hopLimit;
  ubyte reserved[3];
};
typedef struct SetHopLimitParamForm_s SetHopLimitParamForm_t;
#endif

struct UDPSendDataReqParamForm_s {
  ubyte4 anvlIPAddr;
  ubyte16 anvlIPv6Addr;
  ubyte2 dstPort;
  ubyte2 srcPort;
  ubyte4 dataLen;
  ubyte *data;
};
typedef struct UDPSendDataReqParamForm_s UDPSendDataReqParamForm_t;

struct UDPSendRptDataReqParamForm_s {
  ubyte4 anvlIPAddr;
  ubyte16 anvlIPv6Addr;
  ubyte2 dstPort;
  ubyte2 srcPort;
  ubyte4 dataLen;
  ubyte  *data;
  ubyte reserved[3];
};
typedef struct UDPSendRptDataReqParamForm_s UDPSendRptDataReqParamForm_t;

/* Protocol functions for STUB message header */
extern STUBForm_t *STUBFormCreate(void);
extern void STUBFormDestroy(STUBForm_t *form);
extern void STUBPacketToForm(ubyte *pktdata, STUBForm_t *form);
extern ubyte4 STUBBuild(STUBForm_t *form, ubyte *buffer);

/* Protocol functions for parameters list of Listen command message */
extern ListenReqParamForm_t *ListenReqParamFormCreate(void);
extern void ListenReqParamFormDestroy(ListenReqParamForm_t *form);
extern void ListenReqParamToForm(ubyte *pktdata, ListenReqParamForm_t *form);
extern ubyte4 ListenReqParamBuild(ListenReqParamForm_t *form, ubyte *buffer);

/* Protocol functions for parameters list of Set_State command message */
extern StartEndTestReqParamForm_t *StartEndTestReqParamFormCreate(void);
extern void StartEndTestReqParamFormDestroy(StartEndTestReqParamForm_t *form);
extern void StartEndTestReqParamToForm(ubyte *pktdata, StartEndTestReqParamForm_t *form);
extern ubyte4 StartEndTestReqParamBuild(StartEndTestReqParamForm_t *form, ubyte *buffer, ubyte2 paddingLen);

/* Protocol functions for parameters list of GET_VERSION response message */
extern GetVerRespParamForm_t *GetVerRespParamFormCreate(void);
extern void GetVerRespParamFormDestroy(GetVerRespParamForm_t *form);
extern void GetVerRespParamToForm(ubyte *pktdata, GetVerRespParamForm_t *form);
extern ubyte4 GetVerRespParamBuild(GetVerRespParamForm_t *form, ubyte *buffer);

/* Protocol functions for parameters list of GET_SAMPLE_DATA response message */
extern GetSampleDataRespParamForm_t *GetSampleDataRespParamFormCreate(void);
extern void GetSampleDataRespParamFormDestroy(GetSampleDataRespParamForm_t *form);
extern void GetSampleDataRespParamToForm(ubyte *pktdata, GetSampleDataRespParamForm_t *form);
extern ubyte4 GetSampleDataRespParamBuild(GetSampleDataRespParamForm_t *form, ubyte *buffer, ubyte2 paddingLen);

/* Protocol functions for parameters list of GET_DATA_LEN response message */
extern GetDataLenRespParamForm_t *GetDataLenRespParamFormCreate(void);
extern void GetDataLenRespParamFormDestroy(GetDataLenRespParamForm_t *form);
extern void GetDataLenRespParamToForm(ubyte *pktdata, GetDataLenRespParamForm_t *form);
extern ubyte4 GetDataLenRespParamBuild(GetDataLenRespParamForm_t *form, ubyte *buffer);

/* Protocol functions for parameters list of CONNECT request message */
extern ConnectReqParamForm_t *ConnectReqParamFormCreate(void);
extern void ConnectReqParamFormDestroy(ConnectReqParamForm_t *form);
extern void ConnectReqParamToForm(ubyte *pktdata, ConnectReqParamForm_t *form);
extern ubyte4 ConnectReqParamBuild(ConnectReqParamForm_t *form, ubyte *buffer);

/* Protocol functions for parameters list of SEND_REPEAT_DATA request message */
extern SendRptDataReqParamForm_t *SendRptDataReqParamFormCreate(void);
extern void SendRptDataReqParamFormDestroy(SendRptDataReqParamForm_t *form);
extern void SendRptDataReqParamToForm(ubyte *pktdata, SendRptDataReqParamForm_t *form);
extern ubyte4 SendRptDataReqParamBuild(SendRptDataReqParamForm_t *form, ubyte *buffer);

/* Protocol functions for parameters list of SEND request message */
extern SendDataReqParamForm_t *SendDataReqParamFormCreate(void);
extern void SendDataReqParamFormDestroy(SendDataReqParamForm_t *form);
extern void SendDataReqParamToForm(ubyte *pktdata, SendDataReqParamForm_t *form);
extern ubyte4 SendDataReqParamBuild(SendDataReqParamForm_t *form, ubyte *buffer, ubyte2 paddingLen);

/* Protocol functions for parameters list of SET_TTL request/response message */
extern SetTTLParamForm_t *SetTTLParamFormCreate(void);
extern void SetTTLParamFormDestroy(SetTTLParamForm_t *form);
extern void SetTTLParamToForm(ubyte *pktdata, SetTTLParamForm_t *form);
extern ubyte4 SetTTLParamBuild(SetTTLParamForm_t *form, ubyte *buffer);

#ifdef LIB_IPV6
/* Protocol functions for parameters list of SET_HOPLIMIT request/response message */
extern SetHopLimitParamForm_t *SetHopLimitParamFormCreate(void);
extern void SetHopLimitParamFormDestroy(SetHopLimitParamForm_t *form);
extern void SetHopLimitParamToForm(ubyte *pktdata, SetHopLimitParamForm_t *form);
extern ubyte4 SetHopLimitParamBuild(SetHopLimitParamForm_t *form, ubyte *buffer);
#endif

/* Protocol functions for parameters list of UDP_CMD_SEND_REPEAT_DATA request message */
extern UDPSendRptDataReqParamForm_t *UDPSendRptDataReqParamFormCreate(void);
extern void UDPSendRptDataReqParamFormDestroy(UDPSendRptDataReqParamForm_t *form);
extern void UDPSendRptDataReqParamToForm(ubyte *pktdata, UDPSendRptDataReqParamForm_t *form);
extern ubyte4 UDPSendRptDataReqParamBuild(UDPSendRptDataReqParamForm_t *form, ubyte *buffer);

/* Protocol functions for parameters list of UDP_CMD_SEND request message */
extern UDPSendDataReqParamForm_t *UDPSendDataReqParamFormCreate(void);
extern void UDPSendDataReqParamFormDestroy(UDPSendDataReqParamForm_t *form);
extern void UDPSendDataReqParamToForm(ubyte *pktdata, UDPSendDataReqParamForm_t *form);
extern ubyte4 UDPSendDataReqParamBuild(UDPSendDataReqParamForm_t *form, ubyte *buffer);

extern ubyte4 STUBCommandMsgConstruct(STUBForm_t *form, ubyte *buffer, void *dutSidecontext);
extern ubyte STUBMsgPaddingAdd(STUBForm_t *form);
extern byte *STUBTypeToString(ubyte type, ubyte2 value, byte *string);

extern ubyte4 Unpack(ubyte *data, byte *formatString, ...);
extern ubyte4 Pack(ubyte *data, byte *formatString, ...);

#endif /* __STUB_H__ */
