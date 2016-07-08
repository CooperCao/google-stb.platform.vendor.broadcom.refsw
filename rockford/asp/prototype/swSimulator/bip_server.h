/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *************************************************************/
/**/
#define ASP_MSG_DEBUG 1
#ifdef ASP_MSG_DEBUG
#define DBG(x, y) printf(x, y)
#define DBG2(x, y, z) printf(x, y, z)
#define DBG3(w, x, y, z) printf(w, x, y, z)
#else
#define DBG(x, y)
#define DBG2(x, y, z)
#define DBG3(w, x, y, z)
#endif
/* check  value != 0, print errString, goto errorLable */
#define CHECK_ERR_NZ_GOTO(errString, rc, errorLabel)        \
    do {                                                                                                \
        if ((rc) != 0) {                                                            \
            printf("%s: %s at %s:%d\n", __FUNCTION__, errString, __FILE__, __LINE__);   \
          goto errorLabel;  \
        }                                           \
    } while(0)

/* check  value < 0, print errString, goto errorLable */
#define CHECK_ERR_LZ_GOTO(errString, errString2, rc, errorLabel)        \
    do {                                                                                                \
        if ((rc) < 0) {                                                         \
            printf("%s: %s %s at %s:%d\n", __FUNCTION__, errString, errString2, __FILE__, __LINE__);    \
          goto errorLabel;  \
        }                                           \
    } while(0)

/* check  value <= 0, print errString, goto errorLable */
#define CHECK_ERR_LEZ_GOTO(errString, errString2, rc, errorLabel)       \
    do {                                                                                                \
        if ((rc) <= 0) {                                                            \
            printf("%s: %s %s at %s:%d\n", __FUNCTION__, errString, errString2, __FILE__, __LINE__);    \
          goto errorLabel;  \
        }                                           \
    } while(0)

/* check ptr == NULL, print errString, goto errorLable */
#define CHECK_PTR_GOTO(errString, ptr, errorLabel)      \
    do {                                                                                                \
        if (ptr == NULL) {                                                          \
            printf("%s: %s at %s:%d\n", __FUNCTION__, errString, __FILE__, __LINE__);   \
          goto errorLabel;  \
        }                                           \
    } while(0)

/*
   BIP Server APIs
 */
typedef struct BIP_ServerCtx *BipServerHandle;
typedef struct SocketState *SocketStateHandle;
typedef struct ListenerState *ListenerStateHandle;


/* initialize global state */
int BIP_Server_Init(void);

typedef enum BIP_ServerProtocol
{
    BIP_ServerProtocol_eUdp,          /* Plain UDP */
    BIP_ServerProtocol_eRtp,          /* RTP */
    BIP_ServerProtocol_eRtsp,         /* RTSP */
    BIP_ServerProtocol_eHttp,         /* HTTP over TCP */
    BIP_ServerProtocol_eMax
} BIP_ServerProtocol;
/* new connection callback prototype: invoked when listener thread receives a new client connection */
typedef void (*NewConnectionCallback)(BipServerHandle serverCtx);
typedef struct BIP_ServerListenerSettings
{
    int port;
    char *ipAddr; /* bind to this specific address and then listen on it */
    BIP_ServerProtocol protocol;
    NewConnectionCallback newConnectionCallback;
} BIP_ServerListenerSettings;
void BIP_Server_GetDefaultListenerSettings(BIP_ServerListenerSettings *settings);

/* API to create, bind, and listen on the listening socket port for TCP streaming session */
/* app calls this function during the system init time and specifies a listening port & new connection callback function */
ListenerStateHandle BIP_Server_CreateTcpListener(BIP_ServerListenerSettings *settings);
/* Destroy TCP Listener */
void BIP_Server_DestroyTcpListener(ListenerStateHandle listener);

/* API to create server context for UDP streaming session */
BipServerHandle BIP_Server_CreateUdp(BIP_ServerListenerSettings *settings);
/* API to Destroy UDP streaming session */
BipServerHandle BIP_Server_DestroyUdp(BIP_ServerListenerSettings *settings);

/* App can call this call to receive the initial HTTP Get Request */
int BIP_Server_RecvHttpRequest(BipServerHandle bipServerHandle, char **initialReqData, size_t *initialReqDataLen);
/* App can call this call to send the initial HTTP Response */
/* Response is either sent out from the ASP if connection is offloaded to ASP during BIP_Server_Start or when connection callback function returns (true for error cases) */
int BIP_Server_SendHttpResponse(BipServerHandle bipServerHandle, char *initialRespData, size_t initialRespDataLen);

/* get & set settings for the streaming session */
int BIP_Server_GetSettings(BipServerHandle bipServerHandle /*, settings */);
int BIP_Server_SetSettings(BipServerHandle bipServerHandle /*, settings */);

/* app can use BIP helper functions to parse and process HTTP request */
void BIP_HttpGetHeader(void);

/* start streaming */
int BIP_Server_Start(BipServerHandle serverCtx, char *fileName);
/* stop streaming */
int BIP_Server_Stop(BipServerHandle serverCtx);
