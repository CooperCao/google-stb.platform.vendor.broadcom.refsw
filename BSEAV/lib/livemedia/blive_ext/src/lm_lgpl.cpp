/**********
 This library is free software; you can redistribute it and/or modify it under
 the terms of the GNU Lesser General Public License as published by the
 Free Software Foundation; either version 2.1 of the License, or (at your
 option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

 This library is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 more details.

 You should have received a copy of the GNU Lesser General Public License
 along with this library; if not, write to the Free Software Foundation, Inc.,
 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/
/* this file has pieces of code derived from the LiveMedia RTP library, and
 * therefore falls under LGPL license
 */

#ifdef SPF_SUPPORT
#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#include "GroupsockHelper.hh"

#include "lm_lgpl.h"

int
lm_stream_initiate(lm_session_t lm_session, lm_stream_t stream, LM_RTPFactory &rtpFactory)
{
    MediaSubsession *subsession = stream->subsession;
    MediaSession *session = lm_session->session;
    const struct in_addr &fSourceFilterAddr = session->sourceFilterAddr();
    unsigned short fClientPortNum = subsession->clientPortNum();
    unsigned char fRTPPayloadFormat = subsession->rtpPayloadFormat();
    unsigned fRTPTimestampFrequency = subsession->rtpTimestampFrequency();
    UsageEnvironment* env = lm_session->lm->env;
    RTCPInstance* fRTCPInstance = NULL;
    RTPSource* fRTPSource = NULL;
    Groupsock* fRTPSocket = NULL;
    Groupsock* fRTCPSocket = NULL;
    Groupsock* oldGroupsock = NULL;
    struct in_addr tempAddr;
    tempAddr.s_addr = subsession->connectionEndpointAddress();
    Boolean success = False;

    do {
        while (1) {
            unsigned short rtpPortNum = fClientPortNum&~1;
            if (subsession->isSSM()) {
                fRTPSocket = new Groupsock(*env, tempAddr, fSourceFilterAddr, rtpPortNum);
            } else {
                fRTPSocket = new Groupsock(*env, tempAddr, rtpPortNum, 255);
            }
            if (fRTPSocket == NULL) {
                env->setResultMsg("Failed to create RTP socket");
                break;
            }

            // Get the client port number, to make sure that it's even (for RTP):
              Port clientPort(0);
              if (!getSourcePort(*env, fRTPSocket->socketNum(), clientPort)) {
                break;
            }
            fClientPortNum = ntohs(clientPort.num());
            // If the port number's not even, try again:
              if ((fClientPortNum&1) == 0) {
                success = True;
                break;
              }
            delete oldGroupsock;
            oldGroupsock = fRTPSocket;
            fClientPortNum = 0;
        }
        delete oldGroupsock;
        if (!success) break;
        // Set our RTCP port to be the RTP port +1
        unsigned short const rtcpPortNum = fClientPortNum|1;
        if (subsession->isSSM()) {
             fRTCPSocket = new Groupsock(*env, tempAddr, fSourceFilterAddr,
                        rtcpPortNum);
            // Also, send RTCP packets back to the source via unicast:
            if (fRTCPSocket != NULL) {
                fRTCPSocket->changeDestinationParameters(fSourceFilterAddr,0,~0);
            }
        } else {
            fRTCPSocket = new Groupsock(*env, tempAddr, rtcpPortNum, 255);
        }
        if (fRTCPSocket == NULL) {
            char tmpBuf[100];
            sprintf(tmpBuf, "Failed to create RTCP socket (port %d)",
                rtcpPortNum);
            env->setResultMsg(tmpBuf);
               break;
        }
        fRTPSource = rtpFactory.createSource(*env, fRTPSocket, fRTPPayloadFormat, fRTPTimestampFrequency);
        // Finally, create our RTCP instance. (It starts running automatically)
        if (fRTPSource != NULL) {
            unsigned totSessionBandwidth = 500; // HACK - later get from SDP#####
            fRTCPInstance = RTCPInstance::createNew(*env, fRTCPSocket,
                    totSessionBandwidth,
                    (unsigned char const*)session->CNAME(),
                    NULL /* we're a client */,
                    fRTPSource);
            if (fRTCPInstance == NULL) {
                env->setResultMsg("Failed to create RTCP instance");
                break;
            }
        }
        subsession->setClientPortNum(fClientPortNum);
        stream->rtcpInstance = fRTCPInstance;
          stream->rtpSource = fRTPSource;
          stream->rtpSocket = fRTPSocket;
        stream->rtcpSocket = fRTCPSocket;
        return 0;
    } while (0);
    Medium::close(fRTCPInstance);
    Medium::close(fRTPSource);
    if(fRTPSocket) {
        delete fRTPSocket;
    }
    if(fRTCPSocket) {
        delete fRTCPSocket;
    }
    return -1;
}

int
lm_stream_set_session(lm_stream_t stream, lm_session_t session, bool usingTcp, unsigned bufferSize)
{
    if (usingTcp) {
        int fInputSocketNum = session->client->socketNum();
        // Tell the subsession to receive RTP (and send/receive RTCP)
        // over the RTSP stream:
        if (stream->rtpSource != NULL) {
            stream->rtpSource->setStreamSocket(fInputSocketNum, stream->subsession->rtpChannelId);
        }
        if (stream->rtcpInstance != NULL) {
            stream->rtcpInstance->setStreamSocket(fInputSocketNum, stream->subsession->rtcpChannelId);
        }
    } else {
        // Set the RTP and RTCP sockets' destination address and port
        // from the information in the SETUP response:
        unsigned destAddress = stream->subsession->connectionEndpointAddress();
        struct in_addr destAddr; destAddr.s_addr = destAddress;
        int destTTL = ~0; // means: don't change

        if (stream->rtpSocket) {
            Port destPort(stream->subsession->serverPortNum);
            stream->rtpSocket->changeDestinationParameters(destAddr, destPort, destTTL);
            if (bufferSize) {
                int socketNum = stream->rtpSource->RTPgs()->socketNum();
                setReceiveBufferTo(*session->lm->env, socketNum, bufferSize);
            }
        }
        if (stream->rtcpSocket && !stream->subsession->isSSM()) {
            Port destPort(stream->subsession->serverPortNum+1);
            stream->rtcpSocket->changeDestinationParameters(destAddr, destPort, destTTL);
        }
    }
    return 0;
}

#endif /* SPF_SUPPORT */
