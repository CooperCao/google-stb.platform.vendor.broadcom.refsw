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
 **************************************************************************/
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "sap_unit_test.h"

#define SAP_PORT 9875
//#define SAP_IPV4_GLOBAL_SCOPE_MULTICAST_ADDR "224.2.127.254"
#define SAP_IPV4_GLOBAL_SCOPE_MULTICAST_ADDR "10.15.5.211"
//#define SAP_IPV4_GLOBAL_SCOPE_MULTICAST_ADDR "10.10.160.60"

#define AUTH_HDR_LEN 16

#define BLD_SAP_HDR(authHdrLen) \
      memcpy(sapPtr, (char *)&sap, sizeof(SAPHeader)); \
      sapLen = sizeof(SAPHeader) + (authHdrLen); \
      hdrPtr = sapPtr + sapLen;\
      //now add the AUTH hdr: just skip the actual auth field for now \

      //now add the payload type \
      sprintf(hdrPtr, "application/sdp"); \
      sapLen += strlen(hdrPtr) + 1; \
      hdrPtr = sapPtr + sapLen;

#define BLD_SDP_HDR(sdpRecords) \
      sprintf(hdrPtr, (sdpRecords)); \
      sapLen += strlen(hdrPtr) + 1;\
      hdrPtr = sapPtr + sapLen;

#define BLD_SDP_W_PORT_HDR(sdpRecords, port) \
      sprintf(hdrPtr, (sdpRecords), (port)); \
      sapLen += strlen(hdrPtr) + 1;\
      hdrPtr = sapPtr + sapLen;


main(int argc, char *argv[])
{
  int fd;
  int sent;
  struct sockaddr_in remoteAddr;
  SAPHeader sap = {0, 0, 0, 0, 0, 0x1, 0, 0x8f4c, 0x6f5f7373};
  char sapPtr[1024];
  char *hdrPtr = sapPtr;
  int testNum = 0;
  int sapLen = 0;
  int ttl = 16;
  int count;
  int i, j;
  int port;

  if (argc < 2)
  {
    printf("missing argument: sap <unit_test_num>\n");
    exit(1);
  }

  if ( (fd = socket(PF_INET, SOCK_DGRAM, 0)) == -1){
    perror("socket");
    return (-1);
  }

  // initialize remote address
  memset(&remoteAddr, 0, sizeof(remoteAddr));
  remoteAddr.sin_family = AF_INET;
  remoteAddr.sin_port = htons(SAP_PORT);
  if (inet_aton(SAP_IPV4_GLOBAL_SCOPE_MULTICAST_ADDR, &remoteAddr.sin_addr) == 0)
  {
    perror("inet_aton");
    return (-1);
  }

  //set the TTL
  if (setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(int)) < 0)
  {
    perror("failed to set the TTL\n");
    return (-1);
  }

  //test the unit test number
  printf("unit test number is testNum = %d\n", (testNum = atoi(argv[1])));
  switch (testNum)
  {
    case 0:
      printf("case: incorrect SAP header version\n");
      printf("result: listener drops this pkt\n");

      sap.version = 2;
      memcpy(sapPtr, (char *)&sap, sizeof(SAPHeader));
      sapLen = sizeof(SAPHeader);
      break;
    case 1:
      printf("case: correct minimum SAP header but incorrect SDP header\n");
      printf("result: listener drops this pkt\n");

      memcpy(sapPtr, (char *)&sap, sizeof(SAPHeader));
      sapLen = 128;     //larger length to include junk in SDP header fields
      hdrPtr = sapPtr + sapLen;
      break;
    case 2:
      printf("case: correct SAP header but no SDP header\n");
      printf("result: listener drops this pkt\n");

      memcpy(sapPtr, (char *)&sap, sizeof(SAPHeader));
      sapLen = sizeof(SAPHeader);
      break;
    case 3:
      printf("case: payload type is present, but no SDP payload\n");
      printf("result: SAP listener should drop such pkt\n");

      memcpy(sapPtr, (char *)&sap, sizeof(SAPHeader));
      sapLen = sizeof(SAPHeader);
      hdrPtr = sapPtr + sapLen;

      //add the payload type field
      sprintf(hdrPtr, "application/sdp");
      sapLen += strlen(hdrPtr) + 1;
      hdrPtr = sapPtr + sapLen;
      break;
    case 4:
      printf("case: no SAP payload field, but SDP version is present\n");
      printf("result: listener accepts pkt, prints connx fields\n");
      memcpy(sapPtr, (char *)&sap, sizeof(SAPHeader));
      sapLen = sizeof(SAPHeader);
      hdrPtr = sapPtr + sapLen;

      //add the SDP fields
      sprintf(hdrPtr, "v=0\nc=IN IP4 10.10.160.60/127");
      sapLen += strlen(hdrPtr) + 1;
      hdrPtr = sapPtr + sapLen;
      break;
    case 5:
      printf("case: no SAP payload field, but a NULL char & SDP version is present\n");
      printf("result: listener accepts pkt, prints connx fields\n");
      memcpy(sapPtr, (char *)&sap, sizeof(SAPHeader));
      sapLen = sizeof(SAPHeader);
      hdrPtr = sapPtr + sapLen;

      //add just a NULL char instead of full payload type string
      *hdrPtr++ = '\0';
      sapLen++;

      //add the SDP fields
      sprintf(hdrPtr, "v=0\nc=IN IP4 10.10.160.60/127");
      sapLen += strlen(hdrPtr) + 1;
      hdrPtr = sapPtr + sapLen;
      break;
    case 6:
      printf("case: Send a Session Deletion Pkt \n");
      printf("result: if session already exists, listener will delete that session; else ignore this messages\n");
      sap.messageType = 1;

    case 7:
      printf("case: Auth Hdr present\n");
      printf("result: listener accepts pkt, prints connx fields\n");

      sap.authLen = AUTH_HDR_LEN;
      BLD_SAP_HDR(AUTH_HDR_LEN);

      //add the SDP fields
      BLD_SDP_HDR("v=0\ns=Test Session\nc=IN IP4 10.15.5.211/127\nm=video 1234/2 RTP/AVP 33");
      break;

    case 8:
      printf("case: Basic SAP/SDP Announcement Message\n");
      printf("result: listener accepts pkt and adds the channel\n");

      //add SAP header
      BLD_SAP_HDR(0);

      //add the SDP fields
      BLD_SDP_HDR("v=0\ns=Test Session\nc=IN IP4 10.15.5.211/127\nm=video 1234/2 RTP/AVP 33");
      break;

    case 9:
      printf("case: 'c=' connection record is only present at sub-session level\n");
      printf("result: listener accepts pkt and adds the channel\n");

      //add SAP header
      BLD_SAP_HDR(0);

      //add the SDP fields
      BLD_SDP_HDR("v=0\ns=Test Session\nm=video 1234/2 RTP/AVP 33\nc=IN IP4 10.15.5.211/127\n");
      break;

    case 10:
      printf("case: 'c=' connection record is missing at both session & sub-session level\n");
      printf("result: listener should reject SAP Message\n");

      //add SAP header
      BLD_SAP_HDR(0);

      //add the SDP fields
      BLD_SDP_HDR("v=0\ns=Test Session\nm=video 1234/2 RTP/AVP 33\n");
      break;

    case 11:
      printf("case: 'c=' connection record contains host name instead of IP address\n");
      printf("result: listener should reject SAP Message as name resolution is not yet supported\n");

      //add SAP header
      BLD_SAP_HDR(0);

      //add the SDP fields
      BLD_SDP_HDR("v=0\ns=Test Session\nm=video 1234/2 RTP/AVP 33\nc=IN IP4 stb-irva-01");
      break;

    case 12:
      printf("case: multiple media records\n");
      printf("result: listener should add one channel per media record\n");

      //add SAP header
      BLD_SAP_HDR(0);

      //add the SDP fields
      BLD_SDP_HDR("v=0\ns=Test Session\nc=IN IP4 10.15.5.211/127\nm=video 1234/2 RTP/AVP 33\nm=video 5678/2 RTP/AVP 33\n");
      break;

    case 13:
      if (argc == 3)
        count = atoi(argv[2]);
      else
        count = 10;
      printf("case: Sending Multiple SAP/SDP Announcement Messages (cnt = %d)\n", count);
      printf("result: listener accepts pkt and adds the channel\n");

      port = 2001;
      for (i=0; i<count; i++)
      {
        sap.msgidHash = i;

        //add SAP header
        BLD_SAP_HDR(0);

        //add the SDP fields
        BLD_SDP_W_PORT_HDR("v=0\ns=Test Session\nc=IN IP4 10.15.5.211/127\nm=video %d/2 RTP/AVP 33", (port+i));

        if ( (sent = sendto(fd, sapPtr, sapLen, 0, (struct sockaddr *)&remoteAddr, sizeof(remoteAddr))) == -1)
        {
          perror("sendto");
          exit(-1);
        }
      }
      printf("test complete\n");
      return;

    case 14:
      if (argc == 3)
        count = atoi(argv[2]);
      else
        count = 10;
      printf("case: Sending Multiple SAP/SDP Deletion Messages (cnt = %d)\n", count);
      printf("result: listener should delete the corresponding SAP sessions\n");

      //set message type to delete message
      sap.messageType = 1;

      port = 2001;
      for (i=0; i<count; i++)
      {
        sap.msgidHash = i;

        //add SAP header
        BLD_SAP_HDR(0);

        //add the SDP fields
        BLD_SDP_W_PORT_HDR("v=0\ns=Test Session\nc=IN IP4 10.15.5.211/127\nm=video %d/2 RTP/AVP 33", (port+i));

        if ( (sent = sendto(fd, sapPtr, sapLen, 0, (struct sockaddr *)&remoteAddr, sizeof(remoteAddr))) == -1)
        {
          perror("sendto");
          exit(-1);
        }
      }
      printf("test complete\n");
      return;

    case 15:
      if (argc == 3)
        count = atoi(argv[2]);
      else
        count = 10;
      printf("case: Sending Multiple Periodic SAP/SDP Announcement Messages (cnt = %d)\n", count);
      printf("result: listener accepts pkt and adds the channel\n");

      port = 2001;
      for (j=0; j<count; j++)
      {
        for (i=0; i<count; i++)
        {
          sap.msgidHash = i;

          //add SAP header
          BLD_SAP_HDR(0);

          //add the SDP fields
          BLD_SDP_W_PORT_HDR("v=0\ns=Test Session\nc=IN IP4 10.15.5.211/127\nm=video %d/2 RTP/AVP 33", (port+i));

          if ( (sent = sendto(fd, sapPtr, sapLen, 0, (struct sockaddr *)&remoteAddr, sizeof(remoteAddr))) == -1)
          {
            perror("sendto");
            exit(-1);
          }
        }
        printf("test complete\n");
        sleep(5);
      }
      return;

    default:
      break;
  }


  if ( (sent = sendto(fd, sapPtr, sapLen, 0, (struct sockaddr *)&remoteAddr, sizeof(remoteAddr))) == -1)
  {
    perror("sendto");
  }

  printf("sent = %d\n", sent);
}
