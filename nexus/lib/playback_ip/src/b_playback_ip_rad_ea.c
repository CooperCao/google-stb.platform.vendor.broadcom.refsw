/***************************************************************************
*  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*
*  This program is the proprietary software of Broadcom and/or its licensors,
*  and may only be used, duplicated, modified or distributed pursuant to the terms and
*  conditions of a separate, written license agreement executed between you and Broadcom
*  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
*  no license (express or implied), right to use, or waiver of any kind with respect to the
*  Software, and Broadcom expressly reserves all rights in and to the Software and all
*  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
*  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
*  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
*  Except as expressly set forth in the Authorized License,
*
*  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
*  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
*  and to use this information only in connection with your use of Broadcom integrated circuit products.
*
*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
*  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
*  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
*  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
*  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
*  USE OR PERFORMANCE OF THE SOFTWARE.
*
*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
*  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
*  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
*  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
*  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
*  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
*  ANY LIMITED REMEDY.
*
* Description: RAD/EA module
*
***************************************************************************/
#if defined(LINUX) || defined(__vxworks)

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdlib.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/sockios.h>

#include "b_playback_ip_lib.h"
#include "b_playback_ip_priv.h"
#include "b_playback_ip_utils.h"
#include "b_playback_ip_ssl.h"
#include "bfile_buffered.h"
#include "fnmatch.h"
#include "b_playback_ip_rad_ea.h"

//#define DEBUG_RAD_EA
//#define WRITE_FILE 1

#ifdef DEBUG_RAD_EA
#define RADEA_PRINTF printf
#else
#define RADEA_PRINTF(...)
#endif

BDBG_MODULE(b_playback_ip_rad_ea);

/* TODO: The current code has the following assumptions
 * 1. EA HTTP header processing is extremely kludgy...we allocate a 15000 byte
 *    array and expect that the entire HTTP header + EA data wil fit in the buffer.
 * 2. The code assumes that the extra rad data that left after the HTTP header is
 *    processed is going to be less than 1 RAD packet (3072)
 * 3. The code assumes that the EA and RAD header sizes are as per the current
 *    spec (v3)...the code does NOT handle changes to the RAD/EA header sizes.
 * 4. The code assumes that any single read request will not exceed "MAX_EA_PACKETS"
 *    EA (and RAD) packets
 */
#define TMP_STRING_SIZE     256
#define HTTP_HEADER_LENGTH  1400

#define RAD_PACKET_SIZE     3072
#define EA_PACKET_SIZE      24
#define RADEA_PACKET_SIZE   (RAD_PACKET_SIZE + EA_PACKET_SIZE)
#define RAD_HEADER_SIZE     36
#define EA_HEADER_SIZE      36

#define MAX_EA_PACKETS      50
#define EA_TEXT_PACKET_SIZE EA_PACKET_SIZE*5 /* biggest number is -nnn followed by a comma */

#define DLNA_MAX_HTTP_RESP_SIZE (128*1024)  /* DLNA 1.5 guideline: 7.4.47 */

//#define MAX_TEMP_BUFFER     15000

static int gRadEaInitRefCnt = 0;

typedef struct B_PlaybackIpRadEaCtx
{
    int sd;        /* socket descriptor for RAD */
    int ea_sd;     /* socket descriptor for EA */

    B_PlaybackIpSessionOpenSettings radSessionOpenSettings;
    B_PlaybackIpSessionOpenSettings eaSessionOpenSettings;
    B_PlaybackIpNetIo sslNetIo;
    void *sslSecurityHandle;

    char *initialPayload;
    int  initialPayloadLength;

    unsigned int  eaPosition;
    unsigned int  radToComplete;
    char eaValidationData[4];
    unsigned int  radFileSize;
    unsigned int  eaFileSize;

    char decryptedAudio[RADEA_PACKET_SIZE];
    int  decryptedAudioLength;

    char radBuffer[RAD_PACKET_SIZE];
    char eaBuffer[EA_PACKET_SIZE*MAX_EA_PACKETS];
    char *tempBuffer;//[MAX_TEMP_BUFFER];
    unsigned int tempBufferSize;
} B_PlaybackIpRadEaCtx;


/************************************************************************/
/* Verifies RAD and EA header                                                                   */
/* hdr - pointer to RAD or EA header                                                          */
/* extraCharsAtEndOfHeader - number of characters after header and      */
/*                           before audio data                          */
/* isRad - identifies if header to be verified is RAD or EA                         */
/* validationData - validation data inserted in the first RAD frame     */
/************************************************************************/
static bool verifyHeaders(B_PlaybackIpRadEaCtx *radEaCtx, char *radHeader, char *eaHeader )
{
  bool rc = false;
  char temp[5];
  char *tmpPtr;
  unsigned int tempValue;

  /* all the numbers in the header are little endian */

  /* rad header */
  tmpPtr = radHeader;

  memcpy( temp, tmpPtr, 4 );
  temp[4] = 0;
  tmpPtr += 4;

  if( strcmp( temp, "TULI" ) != 0 )
  {
        BDBG_ERR(("%s: did not find TULI", BSTD_FUNCTION));
        goto verifyHeaders_done;
  }

  memcpy( temp, tmpPtr, 4 );
  temp[4] = 0;
  tmpPtr += 4;

  if( strcmp( temp, "RAD_" ) != 0 )
  {
        BDBG_ERR(("%s: did not find RAD_", BSTD_FUNCTION));
        goto verifyHeaders_done;
  }

  tempValue = tmpPtr[3];
  tempValue <<= 8;
  tempValue |= tmpPtr[2];
  tempValue <<= 8;
  tempValue |= tmpPtr[1];
  tempValue <<= 8;
  tempValue |= tmpPtr[0];
  tmpPtr += 4;

  /* assuming rad header size is as per v3 */
  BDBG_ASSERT( tempValue == 24 );

  tempValue = tmpPtr[1];
  tempValue <<= 8;
  tempValue |= tmpPtr[0];
  tmpPtr += 2;

  /* assuming rad major version is 3 */
  BDBG_ASSERT( tempValue == 3 );

  tempValue = tmpPtr[1];
  tempValue <<= 8;
  tempValue |= tmpPtr[0];
  tmpPtr += 2;

  /* assuming rad minor version is 0 */
  BDBG_ASSERT( tempValue == 0 );

  /* currently ignore the track id, codec id and format info */
  tmpPtr += 12;

  /* num rad padding bytes */
  tempValue = tmpPtr[3];
  tempValue <<= 8;
  tempValue |= tmpPtr[2];
  tempValue <<= 8;
  tempValue |= tmpPtr[1];
  tempValue <<= 8;
  tempValue |= tmpPtr[0];
  tmpPtr += 4;

  radEaCtx->radToComplete = tempValue;

  tempValue = tmpPtr[3];
  tempValue <<= 8;
  tempValue |= tmpPtr[2];
  tempValue <<= 8;
  tempValue |= tmpPtr[1];
  tempValue <<= 8;
  tempValue |= tmpPtr[0];
  tmpPtr += 4;

  /* rad file size */
  radEaCtx->radFileSize = tempValue;
  RADEA_PRINTF("Rad file size = %u\n", radEaCtx->radFileSize );


  /* ea header */
  tmpPtr = eaHeader;

  memcpy( temp, tmpPtr, 4 );
  temp[4] = 0;
  tmpPtr += 4;

  if( strcmp( temp, "TULI" ) != 0 )
  {
        BDBG_ERR(("%s: did not find TULI", BSTD_FUNCTION));
        goto verifyHeaders_done;
  }

  memcpy( temp, tmpPtr, 4 );
  temp[4] = 0;
  tmpPtr += 4;

  if( strcmp( temp, "EA__" ) != 0 )
  {
        BDBG_ERR(("%s: did not find EA__", BSTD_FUNCTION));
        goto verifyHeaders_done;
  }

  tempValue = tmpPtr[3];
  tempValue <<= 8;
  tempValue |= tmpPtr[2];
  tempValue <<= 8;
  tempValue |= tmpPtr[1];
  tempValue <<= 8;
  tempValue |= tmpPtr[0];
  tmpPtr += 4;

  /* assuming rad header size is as per v3 */
  BDBG_ASSERT( tempValue == 24 );

  tempValue = tmpPtr[1];
  tempValue <<= 8;
  tempValue |= tmpPtr[0];
  tmpPtr += 2;

  /* assuming rad major version is 3 */
  BDBG_ASSERT( tempValue == 3 );

  tempValue = tmpPtr[1];
  tempValue <<= 8;
  tempValue |= tmpPtr[0];
  tmpPtr += 2;

  /* assuming rad minor version is 0 */
  BDBG_ASSERT( tempValue == 0 );

  /* currently ignore the track id, codec id and format info */
  tmpPtr += 12;

  /* keep the validation data */
  memcpy( radEaCtx->eaValidationData, tmpPtr, 4 );
  tmpPtr += 4;

  tempValue = tmpPtr[3];
  tempValue <<= 8;
  tempValue |= tmpPtr[2];
  tempValue <<= 8;
  tempValue |= tmpPtr[1];
  tempValue <<= 8;
  tempValue |= tmpPtr[0];
  tmpPtr += 4;

  /* ea file size */
  radEaCtx->eaFileSize = tempValue;
  RADEA_PRINTF("EA file size = %u\n", radEaCtx->eaFileSize );

  rc = true;

verifyHeaders_done:
  return rc;
}

int
_https_socket_select(B_PlaybackIpState *playbackIpState, int fd)
{
    int rc = -1;
    fd_set rfds;
    struct timeval tv;
    while (true) {
        if ((*playbackIpState == B_PlaybackIpState_eStopping) || (*playbackIpState == B_PlaybackIpState_eStopped)) {
            /* user changed the channel, so return */
            BDBG_WRN(("%s: breaking out of select loop due to state (%d) change\n", BSTD_FUNCTION, *playbackIpState));
            return -1;
        }
        FD_ZERO(&rfds);
        FD_SET(fd, &rfds);
        tv.tv_sec = HTTP_SELECT_TIMEOUT;
        tv.tv_usec = 0;

        rc = select(fd +1, &rfds, NULL, NULL, &tv);
        if (rc < 0) {
            if (errno == EINTR) {
                BDBG_WRN(("%s: select System Call interrupted, retrying\n", BSTD_FUNCTION));
                continue;
            }
            BDBG_ERR(("%s: ERROR: select(): errno = %d", BSTD_FUNCTION, errno));
            return -1;
        }

        if (rc == 0 || !FD_ISSET(fd, &rfds)) {
            /* select timeout or some select event but our FD not set: No more data - wait */
            BDBG_ERR(("%s: ERROR: select timed out after %d sec\n", BSTD_FUNCTION, HTTP_SELECT_TIMEOUT));
            return -1;
        }
        /* ready ready event on socket, return succcess */
        return 0;
    }
}

#if 0
static bool process_http_header( char *tempBuf, int tempBufLen, int *bytesConsumed )
{
  bool rc = false;
  int i;


  *bytesConsumed = tempBufLen;

    for( i = 0; i < tempBufLen; i++)
    {
        if(tempBuf[i] == '[')
        {
          RADEA_PRINTF("Found opening brace\n");
            break;
    }
    }

  if( i < tempBufLen )
  {
    *bytesConsumed = i;
    rc = true;
  }

    return rc;
}
#endif

static int read_data( B_PlaybackIpRadEaCtx *radEaCtx, B_PlaybackIpState* playbackIpState, char *rbuf, int rbuf_len )
{
    int rc;

    while (1)
    {
        if ((*playbackIpState == B_PlaybackIpState_eStopping) || (*playbackIpState == B_PlaybackIpState_eStopped))
        {
            /* user changed the channel, so return */
            BDBG_WRN(("%s: breaking out of read loop due to state (%d) change\n", BSTD_FUNCTION, *playbackIpState));
            return -1;
        }

      rc = read(radEaCtx->sd, rbuf, rbuf_len);
      if (rc < 0)
      {
        if (errno == EINTR || errno == EAGAIN)
        {
          BDBG_ERR(("%s: Read System Call interrupted or timed out, retrying (errno %d)\n", BSTD_FUNCTION, errno));
          continue;
        }
        BDBG_ERR(("%s: read ERROR:%d", BSTD_FUNCTION, errno));
        return rc;
      }
      else if (rc == 0)
      {
        BDBG_ERR(("%s: WWW Reached EOF, server closed the connection!\n", BSTD_FUNCTION));
        return rc;
      }

      BDBG_MSG_FLOW(("%s: bytes read %d\n", BSTD_FUNCTION, rc));
      return rc;
    }

}

static char* radea_url_encode( char *inputStr, char *outputStr, int outputStrLength )
{
  char *tempPtr = outputStr;
  int i = 0;
  int j = 0;
  int inputStrLength = strlen(inputStr);
  char *unRestrictedChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.~";
  int tempChr;

  RADEA_PRINTF("->%s: input = %s\n", BSTD_FUNCTION, inputStr);
  /* account for the 0 at the end */
  outputStrLength--;

  for( i = 0; i < inputStrLength; i++ )
  {
    tempChr = ((int)inputStr[i]) & 0xff;
    if( strchr( unRestrictedChars, tempChr ) == NULL )
    {
      /* character is special */
      if( j+2 >= outputStrLength )
      {
        outputStr[j] = 0;
        RADEA_PRINTF("url_encode0: input[%s] -> output[%s]\n", inputStr, outputStr);
        return outputStr;
      }

      sprintf(&outputStr[j], "%%%02X", tempChr);
      j+=3;
    }
    else
    {
      if( j >= outputStrLength )
      {
        outputStr[j] = 0;
        RADEA_PRINTF("url_encode1: input[%s] -> output[%s]\n", inputStr, outputStr);
        return outputStr;
      }

      outputStr[j] = inputStr[i];
      j++;
    }
  }

  outputStr[j] = 0;

  RADEA_PRINTF("url_encode: input[%s] -> output[%s]\n", inputStr, outputStr);

  return outputStr;
}

#define TEMP_STR_LENGTH 256

static int read_ea_data(B_PlaybackIpRadEaCtx *radEaCtx, B_PlaybackIpState *playbackIpState, unsigned int offset, char *rbuf, int rbuf_len  )
{
  int rc;
  char request[4096];
  char tempStr[TEMP_STR_LENGTH];
  char url[2048];
  bool headerProcessed = false;

  char* readPtr = rbuf;
  int bytesRead = 0;

  char* bufferPtr;
  int bufferSize = 0;
  int tempBufferSize = 0;
  int tempBufferIndex = 0;
  int allocatedSize = 0;
  int eaDataLength = 0;
  int i;

  B_PlaybackIpHttpMsgFields httpFields;
  B_PlaybackIpSecurityOpenOutputParams sslOpenOutputParams;

  if ((rc = B_PlaybackIp_UtilsTcpSocketConnect(radEaCtx->eaSessionOpenSettings.socketOpenSettings.ipAddr, radEaCtx->eaSessionOpenSettings.socketOpenSettings.port, true, &(radEaCtx->ea_sd))) != B_ERROR_SUCCESS)
  {
    BDBG_ERR(("%s: ERROR: failed to Setup Socket Connection to Server: %s:%d\n", BSTD_FUNCTION, radEaCtx->eaSessionOpenSettings.socketOpenSettings.ipAddr, radEaCtx->eaSessionOpenSettings.socketOpenSettings.port));
    goto read_ea_data_error1;
  }

  sslOpenOutputParams.netIoPtr = &radEaCtx->sslNetIo;
  sslOpenOutputParams.securityHandle = &radEaCtx->sslSecurityHandle;
  /* sslOpenOutputParams.byteRangeOffset = 0; - not needed */
  if ((rc = B_PlaybackIp_SslSessionOpen(&radEaCtx->eaSessionOpenSettings, radEaCtx->ea_sd, &sslOpenOutputParams) ) != B_ERROR_SUCCESS)
  {
    BDBG_ERR(("%s: ERROR: B_PlaybackIp_SslSessionOpen failed \n", BSTD_FUNCTION));
    goto read_ea_data_error2;
  }

  RADEA_PRINTF("%s composing url with offset = %d\n", BSTD_FUNCTION, offset);

  if( radEaCtx->eaSessionOpenSettings.security.settings.radEa.radioChannel == true )
  {
    strcpy(url, "/playback/data/getEAForChannel.js?");

    strcat(url, "logon=");
    //strcat(url, radea_url_encode(radEaCtx->eaSessionOpenSettings.security.settings.radEa.logon, tempStr, TEMP_STR_LENGTH));
    strcat(url, radEaCtx->eaSessionOpenSettings.security.settings.radEa.logon);

    strcat(url, "&cobrandId=");
    //strcat(url, radea_url_encode(radEaCtx->eaSessionOpenSettings.security.settings.radEa.cobrandId, tempStr, TEMP_STR_LENGTH));
    strcat(url, radEaCtx->eaSessionOpenSettings.security.settings.radEa.cobrandId);

    strcat(url, "&channelTrackId=");
    strcat(url, radea_url_encode(radEaCtx->eaSessionOpenSettings.security.settings.radEa.trackId, tempStr, TEMP_STR_LENGTH));
    //strcat(url, radEaCtx->eaSessionOpenSettings.security.settings.radEa.trackId);

    strcat(url, "&channelId=");
    //strcat(url, radea_url_encode(radEaCtx->eaSessionOpenSettings.security.settings.radEa.channelId, tempStr, TEMP_STR_LENGTH));
    strcat(url, radEaCtx->eaSessionOpenSettings.security.settings.radEa.channelId);

    strcat(url, "&playbackSessionId=");
    //strcat(url, radea_url_encode(radEaCtx->eaSessionOpenSettings.security.settings.radEa.playbackSessionId, tempStr, TEMP_STR_LENGTH));
    strcat(url, radEaCtx->eaSessionOpenSettings.security.settings.radEa.playbackSessionId);

    sprintf(tempStr, "&position=%u&count=%d", offset, rbuf_len);
    strcat(url, tempStr);

    strcat(url, "&format=");
    //strcat(url, radea_url_encode(radEaCtx->eaSessionOpenSettings.security.settings.radEa.format, tempStr, TEMP_STR_LENGTH));
    strcat(url, radEaCtx->eaSessionOpenSettings.security.settings.radEa.format);

    sprintf(tempStr, "&bitrate=%d", radEaCtx->eaSessionOpenSettings.security.settings.radEa.bitrate);
    strcat(url, tempStr);

    strcat(url, "&developerKey=");
    //strcat(url, radea_url_encode(radEaCtx->eaSessionOpenSettings.security.settings.radEa.developerKey, tempStr, TEMP_STR_LENGTH));
    strcat(url, radEaCtx->eaSessionOpenSettings.security.settings.radEa.developerKey);
  }
  else
  {
    strcpy(url, "/playback/data/getEA.js?");

    strcat(url, "trackId=");
    strcat(url, radea_url_encode(radEaCtx->eaSessionOpenSettings.security.settings.radEa.trackId, tempStr, TEMP_STR_LENGTH));
    //strcat(url, radEaCtx->eaSessionOpenSettings.security.settings.radEa.trackId);

    strcat(url, "&logon=");
    //strcat(url, radea_url_encode(radEaCtx->eaSessionOpenSettings.security.settings.radEa.logon, tempStr, TEMP_STR_LENGTH));
    strcat(url, radEaCtx->eaSessionOpenSettings.security.settings.radEa.logon);

    strcat(url, "&cobrandId=");
    //strcat(url, radea_url_encode(radEaCtx->eaSessionOpenSettings.security.settings.radEa.cobrandId, tempStr, TEMP_STR_LENGTH));
    strcat(url, radEaCtx->eaSessionOpenSettings.security.settings.radEa.cobrandId);

    strcat(url, "&playbackSessionId=");
    //strcat(url, radea_url_encode(radEaCtx->eaSessionOpenSettings.security.settings.radEa.playbackSessionId, tempStr, TEMP_STR_LENGTH));
    strcat(url, radEaCtx->eaSessionOpenSettings.security.settings.radEa.playbackSessionId);

    sprintf(tempStr, "&position=%u&count=%d", offset, rbuf_len);
    strcat(url, tempStr);

    strcat(url, "&format=");
    //strcat(url, radea_url_encode(radEaCtx->eaSessionOpenSettings.security.settings.radEa.format, tempStr, TEMP_STR_LENGTH));
    strcat(url, radEaCtx->eaSessionOpenSettings.security.settings.radEa.format);

    sprintf(tempStr, "&bitrate=%d&radversion=%d",
            radEaCtx->eaSessionOpenSettings.security.settings.radEa.bitrate,
            radEaCtx->eaSessionOpenSettings.security.settings.radEa.radversion);
    strcat(url, tempStr);

    strcat(url, "&developerKey=");
    //strcat(url, radea_url_encode(radEaCtx->eaSessionOpenSettings.security.settings.radEa.developerKey, tempStr, TEMP_STR_LENGTH));
    strcat(url, radEaCtx->eaSessionOpenSettings.security.settings.radEa.developerKey);
  }

  RADEA_PRINTF("EA URL is: %s\n", url);
  sprintf(request,
          "GET %s HTTP/1.1\r\n"
          "Host: %s:%d\r\n"
          "transferMode.dlna.org: Streaming\r\n"
          "getcontentFeatures.dlna.org: 1\r\n"
          "User-Agent: BRCM 9354x\r\n"
          "Connection: Close\r\n"
          "\r\n",
          url, radEaCtx->eaSessionOpenSettings.socketOpenSettings.ipAddr, radEaCtx->eaSessionOpenSettings.socketOpenSettings.port);


    /* Write  to the EA socket using SSL write */
  rc = radEaCtx->sslNetIo.write(radEaCtx->sslSecurityHandle, playbackIpState, radEaCtx->ea_sd, request, strlen(request));
  if (rc < 0 )
  {
    BDBG_ERR(("%s: write System Call interrupted or timed out, retrying (rc %d, errno %d)\n", BSTD_FUNCTION, rc, errno));
    goto read_ea_data_error;
  }
  RADEA_PRINTF("%s request written\n", BSTD_FUNCTION);

  /* Allocate buffer for ea data. HTTP_HEADER_LENGTH + 2  for the brackets, rbuf_len * 5 is for the ea data */
  eaDataLength = 2 + (rbuf_len * 5);
  allocatedSize = HTTP_HEADER_LENGTH + eaDataLength;
  if( radEaCtx->tempBufferSize < allocatedSize )
  {
    if( radEaCtx->tempBuffer != NULL )
      BKNI_Free(radEaCtx->tempBuffer);

    radEaCtx->tempBuffer = NULL;
    radEaCtx->tempBufferSize = 0;
  }

  if( radEaCtx->tempBuffer == NULL )
  {
    radEaCtx->tempBuffer = (char*)BKNI_Malloc(allocatedSize);
    if (!radEaCtx->tempBuffer)
    {
      BDBG_ERR(("%s: memory allocation failure\n", BSTD_FUNCTION));
      rc = -1;
      goto read_ea_data_error;
    }
  }

  memset(radEaCtx->tempBuffer, allocatedSize, 0 );
  tempBufferSize = allocatedSize;
  bufferPtr = radEaCtx->tempBuffer;

  /* Wait for data to be available */
  while (true)
  {
    if(_https_socket_select(playbackIpState, radEaCtx->ea_sd))
    {
      BDBG_ERR(("%s: select ERROR:%d\n", BSTD_FUNCTION, errno));
      rc = -1;
      goto read_ea_data_error;
    }

    RADEA_PRINTF("%s select succeeded\n", BSTD_FUNCTION);
    /* Read EA data */
    if ((*playbackIpState == B_PlaybackIpState_eStopping) || (*playbackIpState == B_PlaybackIpState_eStopped))
    {
      /* user changed the channel, so return */
      BDBG_WRN(("%s: breaking out of select loop due to state (%d) change\n", BSTD_FUNCTION, *playbackIpState));
      rc = -1;
      goto read_ea_data_error;
    }

    /* read tempBufferSize bytes at a time, until we run out of data */
    rc = radEaCtx->sslNetIo.read(radEaCtx->sslSecurityHandle, playbackIpState, radEaCtx->ea_sd, radEaCtx->tempBuffer + tempBufferIndex, tempBufferSize);
    if (rc < 0)
    {
      if (errno == EINTR || errno == EAGAIN)
        continue;
      BDBG_ERR(("%s: read ERROR:%d", BSTD_FUNCTION, errno));
      goto read_ea_data_error;
    }
    else if (rc == 0)
    {
      BDBG_ERR(("%s: SSSSS  Reached EOF, server closed the connection! \n", BSTD_FUNCTION));
      break;
    }

    /* we read some data */
    BDBG_MSG_FLOW(("%s: bytes read %d\n", BSTD_FUNCTION, rc));
    RADEA_PRINTF("bytes read = %d, headerProcessed = %d\n", rc, headerProcessed);

    bufferSize += rc;

    if( !headerProcessed )
    {
      if (B_PlaybackIp_UtilsHttpResponseParse(radEaCtx->tempBuffer, bufferSize, &httpFields) != B_ERROR_SUCCESS)
      {
        RADEA_PRINTF("B_PlaybackIp_UtilsHttpResponseParse returned parsing result = %d\n", httpFields.parsingResult);
        /* Error in HTTP parsing, check for parsing results */
        switch (httpFields.parsingResult) {
        case B_PlaybackIpHttpHdrParsingResult_eIncompleteHdr:
            BDBG_WRN(("%s: Haven't yet received complete HTTP message header (bytesRead %d), reading more data into buffer of size %d\n",
                        BSTD_FUNCTION, bufferSize, tempBufferSize));
            if (bufferSize >= DLNA_MAX_HTTP_RESP_SIZE + eaDataLength)
            {
              BDBG_ERR(("%s: ERROR: Did not receive complete HTTP Header Response in max allowed size (bytes read %d)\n", BSTD_FUNCTION, bytesRead));
              rc = -1;
              goto read_ea_data_error;
            }

            allocatedSize = (allocatedSize - eaDataLength)*2 + eaDataLength;
            BDBG_WRN(("%s: current buffer size (%d) not big enough to read complete HTTP response message, reallocing it to total of %d bytes\n",
                        BSTD_FUNCTION, tempBufferSize, allocatedSize));
            if ( (radEaCtx->tempBuffer = realloc(radEaCtx->tempBuffer, allocatedSize)) == NULL)
            {
              BDBG_ERR(("%s: Failed to reallocate memory by %d amount, errno = %d\n", BSTD_FUNCTION, tempBufferSize, errno));
              radEaCtx->tempBufferSize = 0;
              rc = -1;
              goto read_ea_data_error;
            }

            radEaCtx->tempBufferSize = allocatedSize;

            tempBufferIndex += bufferSize;
            tempBufferSize = allocatedSize - bufferSize;
            continue;

        case B_PlaybackIpHttpHdrParsingResult_eStatusNotSupported:
            BDBG_ERR(("%s: ERROR: Unsupported HTTP Message status code %d in HTTP Response!!\n", BSTD_FUNCTION, httpFields.statusCode));
            rc = -1;
            goto read_ea_data_error;

        case B_PlaybackIpHttpHdrParsingResult_eReadNextHdr:
            BDBG_ERR(("%s: Received HTTP 1xx status code (%d), going back to receiving next response\n", BSTD_FUNCTION, httpFields.statusCode));
            if(tempBufferSize < DLNA_MAX_HTTP_RESP_SIZE + eaDataLength)
            {
              memset(radEaCtx->tempBuffer, 0, tempBufferSize);
              tempBufferIndex = 0;
              tempBufferSize = allocatedSize;
              continue;
            }
            else
            {
              rc = -1;
              goto read_ea_data_error;
            }
        case B_PlaybackIpHttpHdrParsingResult_eIncorrectHdr:
        default:
            BDBG_ERR(("%s: received Incorrect/Invalid Header\n", BSTD_FUNCTION));
            rc = -1;
            goto read_ea_data_error;
        }
      }
      else
      {
        headerProcessed = true;
        RADEA_PRINTF("B_PlaybackIp_UtilsHttpResponseParse returned success\n");
        RADEA_PRINTF("HTTP Payload is %p, size = %d\n", httpFields.httpPayload, httpFields.httpPayloadLength);
      }
    }

    // continue reading until the end of file
    tempBufferIndex += bufferSize;
    tempBufferSize -= bufferSize;
  }

  RADEA_PRINTF("Buffer read completed - size = %d\n", bufferSize);

  /* header has been process. Find the opening bracket */
    for( i = 0; i < bufferSize; i++)
    {
        if(radEaCtx->tempBuffer[i] == '[')
        {
          RADEA_PRINTF("Found opening brace\n");
            break;
    }
    }

  if(i >= bufferSize)
  {
    rc = -1;
    goto read_ea_data_error;
  }

  bufferPtr += i;//bytesConsumed;
  bufferSize -= i;//bytesConsumed;


  /* Http header parsed successfully.  EA data needed is stored in the buffer so just parse EA data and store them in rbuf */

  RADEA_PRINTF("EA data = %s\n", bufferPtr);

  BDBG_ASSERT(bufferPtr[0] == '[');
  bufferPtr++;
  bufferSize--;

  bytesRead = 0;
  rc = 0;

  char *payload = bufferPtr-1;
  int payloadSize = bufferSize+1;
  char value[5];
  i = 0;
  int digitValue;

  memset(value, 0, 5);

  while( bufferSize > 0 )
  {
    if( bufferPtr[0] == ']' )
    {
      if( i != 0 )
      {
        digitValue = atoi(value);
        memset(value, 0, 5);
        i = 0;
        *readPtr = digitValue & 0xff;
        bytesRead++;
        readPtr++;
        bufferPtr++;
        bufferSize--;
      }

      rc = bytesRead;
      RADEA_PRINTF("%s: Returning %d bytes\n", BSTD_FUNCTION, bytesRead );
      if( bytesRead == 0 )
      {
        RADEA_PRINTF("returned HTTP data = %s\n", radEaCtx->tempBuffer);
      }

      goto read_ea_data_error;
    }
    else if( (bufferPtr[0] == '-') || (isdigit(bufferPtr[0])) )
    {
      if( i >= 4 )
      {
        RADEA_PRINTF(" extra digit found - i = %d\n", i);
        RADEA_PRINTF(" current offset = %d\n", (int)(bufferPtr - payload));
        payload[payloadSize-1] = 0;
        RADEA_PRINTF("Buffer is %sDONE\n", payload);
      }

      BDBG_ASSERT(i < 4);
      value[i] = bufferPtr[0];
      i++;
      bufferPtr++;
      bufferSize--;
    }
    else if( (bufferPtr[0] == ',') )
    {
      BDBG_ASSERT(i != 0);

      digitValue = atoi(value);
      memset(value, 0, 5);
      i = 0;
      *readPtr = digitValue & 0xff;
      bytesRead++;
      readPtr++;
      bufferPtr++;
      bufferSize--;
    }
    else if(bufferPtr[0] == 0x0d)
    {
      BDBG_WRN(("%s: Unexpected character in input %02x\n", BSTD_FUNCTION, ((int)bufferPtr[0])&0xff));
      bufferPtr++;
      bufferSize--;
    }
    else if(bufferPtr[0] == 0x0a)
    {
      BDBG_WRN(("%s: Unexpected character in input %02x\n", BSTD_FUNCTION, ((int)bufferPtr[0])&0xff));
      /* ignore the data from 0x0a to 0x0a */
      bufferPtr++;
      bufferSize--;
      while( *bufferPtr != 0x0a )
      {
        BDBG_WRN(("%s: Ignoring characters in input %02x\n", BSTD_FUNCTION, ((int)bufferPtr[0])&0xff));
        bufferPtr++;
        bufferSize--;
      }

      bufferPtr++;
      bufferSize--;
    }
    else
    {
      BDBG_ERR(("%s: Unexpected character in input %02x\n", BSTD_FUNCTION, ((int)bufferPtr[0])&0xff));
      payload[payloadSize-1] = 0;
      RADEA_PRINTF("Buffer is %sDONE\n", payload);
      BDBG_ASSERT(0);
    }
  }

read_ea_data_error:
  B_PlaybackIp_SslSessionClose(radEaCtx->sslSecurityHandle);

read_ea_data_error2:
  close(radEaCtx->ea_sd);

read_ea_data_error1:
  return rc;
}

static int read_radea_headers(B_PlaybackIpRadEaCtx *radEaCtx, B_PlaybackIpState *playbackIpState)
{
  int rc = 0;
  char eaHeader[EA_HEADER_SIZE];
  char radHeader[RAD_HEADER_SIZE];

  /* read the EA header */
  if( (rc = read_ea_data(radEaCtx, playbackIpState, 0, eaHeader, EA_HEADER_SIZE )) <= 0 )
  {
    BDBG_ERR(("%s: read_ea_data failed with error code %d\n", BSTD_FUNCTION, rc));
    goto read_radea_headers_done;
  }

  if( rc < EA_HEADER_SIZE )
  {
    BDBG_ERR(("%s: Failed to read entire EA header\n", BSTD_FUNCTION));
    rc = -1;
    goto read_radea_headers_done;
  }

  radEaCtx->eaPosition = EA_HEADER_SIZE;

  /* read the RAD header */
  if( radEaCtx->initialPayloadLength < RAD_HEADER_SIZE )
  {
    char* radHeaderPtr = radHeader;
    int headerSize = RAD_HEADER_SIZE - radEaCtx->initialPayloadLength;

    memcpy( radHeaderPtr, radEaCtx->initialPayload, radEaCtx->initialPayloadLength );
    radHeaderPtr += radEaCtx->initialPayloadLength;

    if( radEaCtx->initialPayloadLength > 0 )
    {
      BKNI_Free(radEaCtx->initialPayload);
      radEaCtx->initialPayload = NULL;
      radEaCtx->initialPayloadLength = 0;
    }

    /* let's read the RAD header */
    if( (rc = read_data( radEaCtx, playbackIpState, radHeaderPtr, headerSize )) <= 0 )
    {
      BDBG_ERR(("%s: read_data failed with error code %d\n", BSTD_FUNCTION, rc));
      goto read_radea_headers_done;
    }

    if( rc < RAD_HEADER_SIZE )
    {
      BDBG_ERR(("%s: Failed to read entire EA header\n", BSTD_FUNCTION));
      rc = -1;
      goto read_radea_headers_done;
    }
  }
  else
  {
    int i;
    char *src;
    char *dest;

    memcpy( radHeader, radEaCtx->initialPayload, RAD_HEADER_SIZE );

    src = &radEaCtx->initialPayload[RAD_HEADER_SIZE];
    dest = radEaCtx->initialPayload;
    radEaCtx->initialPayloadLength -= RAD_HEADER_SIZE;
    i = radEaCtx->initialPayloadLength;
    while( i )
    {
      *dest = *src;
      dest++;
      src++;
      i--;
    }
  }

  verifyHeaders( radEaCtx, radHeader, eaHeader );
  return 1;

read_radea_headers_done:
  return rc;
}


#ifdef WRITE_FILE
static void write_file( char* fileName, char* data, int dataSize )
{
  FILE* fptr = fopen( fileName, "ab");

  RADEA_PRINTF("writing data size %d\n", dataSize);
  fwrite( data, 1, dataSize, fptr);

  fclose(fptr);
}
#endif

/*
 * This function tries to read the requested amount from the socket and returns any errors or bytes read.
 * Since data is encrypted with RAD/EA Encryption, this function is
 * responsible for performing the RAD/EA Decryption and proving the clear data
 * back to the caller.
 * It returns:
 *  =-1: for errors other than EINTR & EAGAIN during read call or when channel change occurs
 *  = 0: for EOF where server closed the TCP connection
 *  > 0: for success indicating number of bytes read from the socket
 *
 * Note1: this function is invoked by the HTTP module of the IP Applib in the
 * context of the Nexus Playback Thread.
 *
 * Note2: if this function needs to loop, it must periodically check the state
 * of Playback IP (B_PlaybackIpState) to see if it has changed to Closing or
 * CLosed state. This can happen when user changes channel and this function
 * can be busy reading data from the socket.
 */
int
_http_rad_ea_socket_read(void *voidHandle, B_PlaybackIpHandle playback_ip, int sd, char *rbuf, int rbuf_len)
{
  int rc;
  B_PlaybackIpRadEaCtx *radEaCtx = (B_PlaybackIpRadEaCtx *)voidHandle;
  int   numRadEaPackets = 0;
  int   readLength = rbuf_len;
  int   bytesRead = 0;
  char* readBufferPtr = rbuf;
  int   i;
  char  *radBuffer;
  int   radBufferSize;
  int   availableEaPackets;
  int   skipValidation = 0;
  char  validationData[4];
  B_PlaybackIpState *playbackIpState = &playback_ip->playback_state;

#ifdef WRITE_FILE
read_start:
#endif
  numRadEaPackets = 0;
  readLength = rbuf_len;
  bytesRead = 0;
  readBufferPtr = rbuf;
  skipValidation = 0;

  RADEA_PRINTF("%s called context = %p, buffer size = %d\n", BSTD_FUNCTION, radEaCtx, rbuf_len);

  if ( (rbuf == NULL) || (rbuf_len <= 0) || (radEaCtx == NULL) )
  {
    BDBG_ERR(("%s: invalid parameters to read, rbuf %p, rbuf_len %d, radEaCtx %p\n", BSTD_FUNCTION, rbuf, rbuf_len, radEaCtx));
    return -1;
  }

  BDBG_ASSERT( sd == radEaCtx->sd );

  RADEA_PRINTF("%s enableDecryption = %d\n", BSTD_FUNCTION, radEaCtx->radSessionOpenSettings.security.enableDecryption);

  /* The http header for the RAD data is in the clear and parsed by the IP playback library.
     The mechanism is as follows:
     Initially decypryption is disabled (as the header is in the clear)
     This read function is called until the entire header has been parsed.
     The extra data (payload) is then sent back to the RAD/EA module as part of the call
     enabling decryption
  */
  if(!radEaCtx->radSessionOpenSettings.security.enableDecryption)
  {
    if( (rc = read_data( radEaCtx, playbackIpState, rbuf, rbuf_len )) <= 0 )
    {
      BDBG_ERR(("%s: read_data failed with error code %d\n", BSTD_FUNCTION, rc));
    }

    return rc;
  }

  RADEA_PRINTF("%s decryption is enabled, buf len = %d, overflow = %d\n", BSTD_FUNCTION, rbuf_len, radEaCtx->initialPayloadLength);

  RADEA_PRINTF("%s decrypted audio length = %d\n", BSTD_FUNCTION, radEaCtx->decryptedAudioLength);
  /* there could be decrypted data from the last read call that still needs to be sent */
  if( rbuf_len < radEaCtx->decryptedAudioLength )
  {
    char tempBuffer[RADEA_PACKET_SIZE];

    RADEA_PRINTF("buffer to read is less than overflow data\n");
    memcpy( readBufferPtr, radEaCtx->decryptedAudio, rbuf_len );
    radEaCtx->decryptedAudioLength -= rbuf_len;
    memcpy( tempBuffer, &radEaCtx->decryptedAudio[rbuf_len], radEaCtx->decryptedAudioLength );
    memset( radEaCtx->decryptedAudio, 0, RADEA_PACKET_SIZE );
    memcpy( radEaCtx->decryptedAudio, tempBuffer, radEaCtx->decryptedAudioLength );

#ifdef WRITE_FILE
    write_file( "music.mp3", rbuf, rbuf_len );
    goto read_start;
#else
    RADEA_PRINTF("%s:%d returning %d\n", BSTD_FUNCTION, __LINE__, rbuf_len);
    return rbuf_len;
#endif
  }
  else
  {
    RADEA_PRINTF("buffer to read is greater than overflow data (size = %d)...copying it over\n", radEaCtx->decryptedAudioLength);
    /* copy over the extra decrypted audio from the last time */
    memcpy( readBufferPtr, radEaCtx->decryptedAudio, radEaCtx->decryptedAudioLength );
    readBufferPtr += radEaCtx->decryptedAudioLength;
    readLength = rbuf_len - radEaCtx->decryptedAudioLength;
    bytesRead = radEaCtx->decryptedAudioLength;
    memset( radEaCtx->decryptedAudio, 0, radEaCtx->decryptedAudioLength );
    radEaCtx->decryptedAudioLength = 0;
  }

  /* calculate how many RAD/EA packets we need to read in order to fulfil this request */
  numRadEaPackets = readLength/RADEA_PACKET_SIZE;
  if( readLength % RADEA_PACKET_SIZE )
    numRadEaPackets++;

  BDBG_ASSERT( numRadEaPackets <= MAX_EA_PACKETS );

  RADEA_PRINTF("%s radea packets required = %d, position = %d\n", BSTD_FUNCTION, numRadEaPackets, radEaCtx->eaPosition);

  /* read the headers */
  if( radEaCtx->eaPosition == 0 )
  {
    skipValidation = 4; /* skip the validation data */
    if( (rc = read_radea_headers(radEaCtx, playbackIpState)) <= 0 )
    {
      BDBG_ERR(("%s: read_radea_headers failed with error code %d\n", BSTD_FUNCTION, rc));
      return rc;
    }
  }

  RADEA_PRINTF("%s finished reading radea headers\n", BSTD_FUNCTION);

  /* read the EA data */
  if( (rc = read_ea_data(radEaCtx, playbackIpState, radEaCtx->eaPosition, radEaCtx->eaBuffer, numRadEaPackets*EA_PACKET_SIZE)) <= 0)
  {
    BDBG_ERR(("%s: read_ea_data failed with error code %d\n", BSTD_FUNCTION, rc));
    return rc;
  }

  availableEaPackets = rc/EA_PACKET_SIZE;
  RADEA_PRINTF("%s: available EA packets = %d\n", BSTD_FUNCTION, availableEaPackets);
  BDBG_ASSERT(availableEaPackets > 0);

  if( availableEaPackets < numRadEaPackets )
  {
    BDBG_WRN(("%s: Detected end of file\n", BSTD_FUNCTION));
    numRadEaPackets = availableEaPackets;
  }

  /* decode the 1st buffer */
  radBuffer = radEaCtx->radBuffer;
  radBufferSize = RAD_PACKET_SIZE - radEaCtx->initialPayloadLength;

  if( radEaCtx->initialPayloadLength > 0 )
  {
    memcpy( radBuffer, radEaCtx->initialPayload, radEaCtx->initialPayloadLength);
    radBuffer += radEaCtx->initialPayloadLength;

    BKNI_Free(radEaCtx->initialPayload);
    radEaCtx->initialPayload = NULL;
    radEaCtx->initialPayloadLength = 0;
  }

  RADEA_PRINTF("%s: reading RAD packet\n", BSTD_FUNCTION);

  if( (rc = read_data( radEaCtx, playbackIpState, radBuffer, radBufferSize )) <= 0 )
  {
    BDBG_ERR(("%s: read_data failed with error code %d\n", BSTD_FUNCTION, rc));
    return rc;
  }

  if( rc < radBufferSize )
  {
    /* we read less than the complete packet */
    radBufferSize = rc + RAD_PACKET_SIZE - radBufferSize;
  }
  else
  {
    radBufferSize = RAD_PACKET_SIZE;
  }

  RADEA_PRINTF("%s: RAD buffer size = %d, eaPosition = %u\n", BSTD_FUNCTION, radBufferSize, radEaCtx->eaPosition);


  /* decrypt into a temporary buffer */
  skipValidation = 0;

  radea_decrypt_frame((unsigned char*)radEaCtx->radBuffer, (unsigned char*)radEaCtx->eaBuffer, radBufferSize, radEaCtx->decryptedAudio);
  radBufferSize += EA_PACKET_SIZE;
  if( radEaCtx->eaPosition == EA_HEADER_SIZE )
  {
    skipValidation = 4;
    memcpy(validationData, radEaCtx->decryptedAudio, 4 );
    radBufferSize -= skipValidation;

    /* validate the data */
    if( memcmp( validationData, radEaCtx->eaValidationData, 4 ) != 0 )
    {
      BDBG_ERR(("%s: validation data did not match\n", BSTD_FUNCTION));
      RADEA_PRINTF("EA data = %02x %02x %02x %02x\n", radEaCtx->eaValidationData[0], radEaCtx->eaValidationData[1], radEaCtx->eaValidationData[2], radEaCtx->eaValidationData[3]);
      RADEA_PRINTF("RAD data = %02x %02x %02x %02x\n", validationData[0], validationData[1], validationData[2], validationData[3]);
      BDBG_ASSERT(NULL);
    }

    BDBG_WRN(("%s: validation data matches\n", BSTD_FUNCTION));
  }

  radEaCtx->eaPosition += EA_PACKET_SIZE;

  if( readLength > radBufferSize )
  {
    /* the data required is more than the 1st buffer - so we have more data to read */
    memcpy( readBufferPtr, &radEaCtx->decryptedAudio[skipValidation], radBufferSize );
    readBufferPtr += radBufferSize;
    readLength -= radBufferSize;
    bytesRead += radBufferSize;
  }
  else
  {
    int j;
    char *src;
    char *dest;

    memcpy( readBufferPtr, &radEaCtx->decryptedAudio[skipValidation], readLength );
    bytesRead += readLength;

    /* move the rest of the data to the beginning to hang on to for the next time */
    radEaCtx->decryptedAudioLength = radBufferSize - (readLength+skipValidation);
    RADEA_PRINTF("Setting overflow decrypted audio length = %d\n", radEaCtx->decryptedAudioLength);
    src = &radEaCtx->decryptedAudio[skipValidation+readLength];
    dest = radEaCtx->decryptedAudio;
    j = radEaCtx->decryptedAudioLength;
    while( j )
    {
      *dest = *src;
      dest++;
      src++;
      j--;
    }

#ifdef WRITE_FILE
    write_file( "music.mp3", rbuf, bytesRead );
    goto read_start;
#else
    RADEA_PRINTF("%s:%d returning %d\n", BSTD_FUNCTION, __LINE__, bytesRead);
    return bytesRead;
#endif
  }


  if( radBufferSize < (RADEA_PACKET_SIZE-skipValidation) )
  {
    BDBG_WRN(("%s: reached end of file?\n", BSTD_FUNCTION));
#ifdef WRITE_FILE
    write_file( "music.mp3", rbuf, bytesRead );
    return bytesRead;
#else
    RADEA_PRINTF("%s:%d returning %d\n", BSTD_FUNCTION, __LINE__, bytesRead);
    return bytesRead;
#endif
  }

  RADEA_PRINTF("first RAD/EA packet done...packets left = %d\n", numRadEaPackets-1);
  for( i = 1; i < numRadEaPackets; i++ )
  {
    if( (rc = read_data( radEaCtx, playbackIpState, radEaCtx->radBuffer, RAD_PACKET_SIZE )) <= 0 )
    {
      BDBG_ERR(("%s: read_data failed with error code %d\n", BSTD_FUNCTION, rc));
      return rc;
    }

    radBufferSize = rc;
    RADEA_PRINTF("Read RAD packet %d, size = %d\n", i, radBufferSize);

    if( readLength > radBufferSize+EA_PACKET_SIZE )
    {
      RADEA_PRINTF("Available buffer (%d) greater than rad data\n", readLength);

      radea_decrypt_frame((unsigned char*)radEaCtx->radBuffer, (unsigned char*)&radEaCtx->eaBuffer[i*EA_PACKET_SIZE], radBufferSize, readBufferPtr);
      radBufferSize += EA_PACKET_SIZE;
      radEaCtx->eaPosition += EA_PACKET_SIZE;
      readBufferPtr += radBufferSize;
      readLength -= radBufferSize;
      bytesRead += radBufferSize;

      if( radBufferSize < RADEA_PACKET_SIZE )
      {
        /* we are out of data */
        BDBG_WRN(("%s: reached end of file?\n", BSTD_FUNCTION));
#ifdef WRITE_FILE
        write_file( "music.mp3", rbuf, bytesRead );
        return bytesRead;
#else
        RADEA_PRINTF("%s:%d returning %d\n", BSTD_FUNCTION, __LINE__, bytesRead);
        return bytesRead;
#endif
      }
    }
    else
    {
      int j;
      char *src;
      char *dest;

      RADEA_PRINTF("Available buffer (%d) less than rad data\n", readLength);

      /* decrypt into a temporary buffer */
      radea_decrypt_frame((unsigned char*)radEaCtx->radBuffer, (unsigned char*)&radEaCtx->eaBuffer[i*EA_PACKET_SIZE], radBufferSize, radEaCtx->decryptedAudio);
      radBufferSize += EA_PACKET_SIZE;
      radEaCtx->eaPosition += EA_PACKET_SIZE;

      memcpy( readBufferPtr, radEaCtx->decryptedAudio, readLength );
      bytesRead += readLength;

      /* move the rest of the data to the beginning to hang on to for the next time */
      radEaCtx->decryptedAudioLength = radBufferSize - readLength;
      src = &radEaCtx->decryptedAudio[readLength];
      dest = radEaCtx->decryptedAudio;
      j = radEaCtx->decryptedAudioLength;
      while( j )
      {
        *dest = *src;
        dest++;
        src++;
        j--;
      }

#ifdef WRITE_FILE
      write_file( "music.mp3", rbuf, bytesRead );
      goto read_start;
#else
      RADEA_PRINTF("%s:%d returning %d\n", BSTD_FUNCTION, __LINE__, bytesRead);
      return bytesRead;
#endif
    }
  }

  BDBG_WRN(("%s: we should not have got here\n", BSTD_FUNCTION));
  BDBG_ASSERT(0);
  return -1;
}

int B_PlaybackIp_RadEaDecryptionEnable( void *securityHandle, char *initialPayload, int initialPayloadLength )
{
  B_PlaybackIpRadEaCtx *radEaCtx = (B_PlaybackIpRadEaCtx *)securityHandle;

  if (!radEaCtx)
  {
    BDBG_ERR(("%s: invalid radEaCtx %p\n", BSTD_FUNCTION, radEaCtx));
    return -1;
  }

  /* set flag to enable decryption */
  radEaCtx->radSessionOpenSettings.security.enableDecryption = true;
  BDBG_WRN(("%s: initialPayloadLength = %d\n", BSTD_FUNCTION, initialPayloadLength));
  if( initialPayloadLength > 0 )
  {
      /* for now assume that the extra data is going to be less than one RAD packet */
      BDBG_ASSERT( initialPayloadLength < RAD_PACKET_SIZE );

      radEaCtx->initialPayloadLength = initialPayloadLength;
      radEaCtx->initialPayload = BKNI_Malloc(initialPayloadLength*sizeof(char));
      if (!radEaCtx->initialPayload)
      {
        BDBG_ERR(("%s: memory allocation failure\n", BSTD_FUNCTION));
        return -1;
      }

      memcpy(radEaCtx->initialPayload, initialPayload, initialPayloadLength);
  }

  return 0;
}

/*
 * Un-Initialize session specific context of the security module
 * Called during B_PlaybackIp_SocketClose().
 */
void B_PlaybackIp_RadEaSessionClose(void *voidHandle)
{

    B_PlaybackIpRadEaCtx *radEaCtx = (B_PlaybackIpRadEaCtx *)voidHandle;

    /* Free up RadEa Handle & any other saved contexts */
    if (radEaCtx) {
        //radEaCtx->sslNetIo.close(radEaCtx->sslSecurityHandle);
        if( radEaCtx->tempBuffer != NULL ) {
            BKNI_Free(radEaCtx->tempBuffer);
        }
        if (radEaCtx->eaSessionOpenSettings.socketOpenSettings.url)
            BKNI_Free(radEaCtx->eaSessionOpenSettings.socketOpenSettings.url);

        BKNI_Free(radEaCtx);
    }
}


/*
 * Initialize session specific context of the security module
 * Called during B_PlaybackIp_SocketOpen().
 */
int B_PlaybackIp_RadEaSessionOpen(
    B_PlaybackIpSessionOpenSettings *openSettings,  /* input: provides server ip, port info */
    int sd,                                              /* input: socket descriptor */
    B_PlaybackIpSecurityOpenOutputParams *securityOpenOutputParams) /* output: security settings return params */
{

    B_PlaybackIpRadEaCtx *radEaCtx = NULL;
    B_PlaybackIpSecurityOpenSettings *securityOpenSettings;  /* provides initial security context & other info */

    if (openSettings == NULL) {
        BDBG_ERR(("%s: Invalid parameters, Open Settings: %p\n", BSTD_FUNCTION, openSettings));
        goto sessionOpen_error;
    }

#if 0
    /* SSOOD: 04/23/2010: commenting out the code below as seekOffsetTime is anyway not being used. */
    /* SSOOD: this value is now residing in a different structure SessionSetupSettings, so we will need to add a RadEaSessionSetup i/f to get these values */

    /* TODO - probably need to moved to down in the function */
    /* This value is calculated from seekOffsetTime */
    if (openSettings->u.http.seekOffsetTime) {
        /* Do some magic to calculate the byte offset */
        securityOpenOutputParams->byteRangeOffset = 0;  /* change value */
    }else
        securityOpenOutputParams->byteRangeOffset = 0;
#endif

    securityOpenSettings = &openSettings->security;
    if (sd <= 0) {
        BDBG_ERR(("%s: invalid socket, sd = %d", BSTD_FUNCTION, sd));
        goto sessionOpen_error;
    }

    if (securityOpenSettings->securityProtocol != B_PlaybackIpSecurityProtocol_RadEa) {
        BDBG_ERR(("%s: invoking RAD/EA module with incorrect security protocol %d", BSTD_FUNCTION, securityOpenSettings->securityProtocol));
        goto sessionOpen_error;
    }

    if (securityOpenSettings->initialSecurityContext == NULL) {
        BDBG_ERR(("%s: SSL Handle is not provided by app\n", BSTD_FUNCTION));
        goto sessionOpen_error;
    }

    radEaCtx = BKNI_Malloc(sizeof(B_PlaybackIpRadEaCtx));
    if (!radEaCtx) {
        BDBG_ERR(("%s: memory allocation failure\n", BSTD_FUNCTION));
        goto sessionOpen_error;
    }
    memset(radEaCtx, 0, sizeof(B_PlaybackIpRadEaCtx));
    radEaCtx->sd = sd;
    memcpy(&radEaCtx->radSessionOpenSettings, openSettings, sizeof(B_PlaybackIpSessionOpenSettings));
    radEaCtx->eaPosition = 0;
    radEaCtx->decryptedAudioLength = 0;
    radEaCtx->initialPayloadLength = 0;
    radEaCtx->initialPayload = NULL;

    memcpy(&radEaCtx->eaSessionOpenSettings, openSettings, sizeof(B_PlaybackIpSessionOpenSettings));

    RADEA_PRINTF("trackId = %s\n", radEaCtx->eaSessionOpenSettings.security.settings.radEa.trackId);
    RADEA_PRINTF("logon = %s\n", radEaCtx->eaSessionOpenSettings.security.settings.radEa.logon);
    RADEA_PRINTF("cobrandid = %s\n", radEaCtx->eaSessionOpenSettings.security.settings.radEa.cobrandId);
    RADEA_PRINTF("playbackSessionId = %s\n", radEaCtx->eaSessionOpenSettings.security.settings.radEa.playbackSessionId);
    RADEA_PRINTF("developerKey = %s\n", radEaCtx->eaSessionOpenSettings.security.settings.radEa.developerKey);
    RADEA_PRINTF("format = %s\n", radEaCtx->eaSessionOpenSettings.security.settings.radEa.format);
    RADEA_PRINTF("bitrate = %d\n", radEaCtx->eaSessionOpenSettings.security.settings.radEa.bitrate);
    RADEA_PRINTF("radversion = %d\n", radEaCtx->eaSessionOpenSettings.security.settings.radEa.radversion);

    radEaCtx->eaSessionOpenSettings.security.enableDecryption = false;
    radEaCtx->eaSessionOpenSettings.security.securityProtocol = B_PlaybackIpSecurityProtocol_Ssl;
    radEaCtx->eaSessionOpenSettings.security.initialSecurityContext = openSettings->security.initialSecurityContext;
    strcpy(radEaCtx->eaSessionOpenSettings.socketOpenSettings.ipAddr, "secure-direct.rhapsody.com");
    if ((radEaCtx->eaSessionOpenSettings.socketOpenSettings.url = BKNI_Malloc(512)) == NULL) {
        BDBG_ERR(("%s: Failed to allocate memory", BSTD_FUNCTION));
        goto sessionOpen_error;
    }
    strcpy(radEaCtx->eaSessionOpenSettings.socketOpenSettings.url, "");
    radEaCtx->eaSessionOpenSettings.socketOpenSettings.port = 443;

#if 0
    if ((rc = B_PlaybackIp_UtilsTcpSocketConnect(radEaCtx->eaSessionOpenSettings.ipAddr, radEaCtx->eaSessionOpenSettings.port, &(radEaCtx->ea_sd))) != B_ERROR_SUCCESS)
    {
      BDBG_ERR(("%s: ERROR: failed to Setup Socket Connection to Server: %s:%d\n", BSTD_FUNCTION, radEaCtx->eaSessionOpenSettings.ipAddr, radEaCtx->openSettings.port));
      goto sessionOpen_error;
    }

    if ((rc = B_PlaybackIp_SslSessionOpen(&radEaCtx->eaSessionOpenSettings, radEaCtx->ea_sd, &radEaCtx->sslNetIo, &radEaCtx->sslSecurityHandle) ) != B_ERROR_SUCCESS)
    {
      BDBG_ERR(("%s: ERROR: B_PlaybackIp_SslSessionOpen failed \n", BSTD_FUNCTION));
      goto sessionOpen_error;
    }
#endif

    BDBG_WRN(("%s: setting up the netIo interface for socket read \n", BSTD_FUNCTION));
    securityOpenOutputParams->netIoPtr->read = _http_rad_ea_socket_read;
    securityOpenOutputParams->netIoPtr->close = B_PlaybackIp_RadEaSessionClose;
    securityOpenOutputParams->netIoPtr->suspend = NULL;
    BDBG_WRN(("%s: returning context %p \n", BSTD_FUNCTION, radEaCtx));

    *securityOpenOutputParams->securityHandle = (void *)radEaCtx;
    return 0;

sessionOpen_error:
    if (radEaCtx && radEaCtx->eaSessionOpenSettings.socketOpenSettings.url)
        BKNI_Free(radEaCtx->eaSessionOpenSettings.socketOpenSettings.url);
    if (radEaCtx)
        BKNI_Free(radEaCtx);
    return -1;
}

/*
 * Clone session.
 * A duplicate session is opened using a separate file descriptor
 * Provided for separate video and audio seeks.
 */
int B_PlaybackIp_RadEaCloneSessionOpen(
    int sd,                                                     /* input: socket descriptor */
    void *sourceSecurityHandle,                                 /* intput: original security handle */
    void **targetSecurityHandle)                                /* output: new security handle */
{

    B_PlaybackIpRadEaCtx *newSecurityCtx = NULL;
    B_PlaybackIpRadEaCtx *radEaCtx = (B_PlaybackIpRadEaCtx *)sourceSecurityHandle;

    if (!radEaCtx || sd <=0 || !targetSecurityHandle ) {
        BDBG_ERR(("%s: invalid radEaCtx %p or sd %d\n", BSTD_FUNCTION, radEaCtx, sd));
        return -1;
    }

    newSecurityCtx = BKNI_Malloc(sizeof(B_PlaybackIpRadEaCtx));
    if (!newSecurityCtx) {
        BDBG_ERR(("%s: memory allocation failure\n", BSTD_FUNCTION));
        return -1;
    }

    memcpy(newSecurityCtx, radEaCtx, sizeof(B_PlaybackIpRadEaCtx));
    /* disable encryption to begin with as initial HTTP response is in clear */
    newSecurityCtx->radSessionOpenSettings.security.enableDecryption = false;
    newSecurityCtx->sd = sd;

    *targetSecurityHandle = (void *)newSecurityCtx;

    return 0;
}

/*
 * Initialize global context of the security module
 * Called during B_PlaybackIp_Open().
 */
void *B_PlaybackIp_RadEaInit(B_PlaybackIpSslInitSettings *initSettings)
{
    SSL_CTX *gRadEaInitCtx = NULL;

    printf("refcount = %d\n", gRadEaInitRefCnt);

    if (!gRadEaInitRefCnt) {
        gRadEaInitRefCnt++;
        /* Global system initialization for RadEa library */
        if ( (gRadEaInitCtx = B_PlaybackIp_SslInit(initSettings)) == NULL ) {
            BDBG_ERR(("%s: SslInit Failed for RAD/EA sessions\n", BSTD_FUNCTION));
            goto radEaInit_error;
        }

        BDBG_WRN(("%s: RAD/EA CTX for RAD/EA session = %p \n", BSTD_FUNCTION, gRadEaInitCtx));
        return gRadEaInitCtx;
    }

radEaInit_error:
    return NULL;
}

/*
 * Un-Initialize global context of the security module
 * Called during B_PlaybackIp_Close().
 */
void B_PlaybackIp_RadEaUnInit(void *ctx)
{
    SSL_CTX *gRadEaInitCtx = (SSL_CTX *)ctx;

    if( gRadEaInitRefCnt == 0 )
      return;

    if( gRadEaInitRefCnt > 0 )
      gRadEaInitRefCnt--;

    if (!gRadEaInitRefCnt) {

          /* Global system un-initialization for RadEa library */
          BDBG_ASSERT(gRadEaInitCtx != NULL);
          B_PlaybackIp_SslUnInit(gRadEaInitCtx);
          BDBG_WRN(("%s: done\n", BSTD_FUNCTION));
    }
}

#endif /* LINUX || VxWorks */
