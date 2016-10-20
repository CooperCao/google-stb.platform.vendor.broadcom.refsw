/******************************************************************************
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
 *****************************************************************************/
#ifndef NEXUS_SAGE_MESSAGE_TYPES_H__
#define NEXUS_SAGE_MESSAGE_TYPES_H__

#ifdef __cplusplus
extern "C" {
#endif


/*
 * This header is shared between the Host and the SAGE side
 * (Nexus Sage on the Host; SAPM on the SAGE)
 */

/***************************************************************************
Summary:
Sage Channel Message Request structure

Description:
Used to send a request to the remote SAGE (Write on the Host; Read on the SAGE)
Exchanged through Host-SAGE Command Buffer (used for the forward communication)
***************************************************************************/
typedef struct NEXUS_SageChannelRequest
{
    uint64_t messageOffset; /* physical address of a NEXUS_SageMessage */
} NEXUS_SageChannelRequest;

/***************************************************************************
Summary:
Sage Channel Message Ack structure

Description:
Used to acknowledge a received request from Host (Write on the SAGE; Read on the Host)
Exchanged through Host-SAGE Response Buffer (used for the forward communication)
***************************************************************************/
typedef struct NEXUS_SageChannelAck
{
    uint32_t rc;     /* Request return Code */
} NEXUS_SageChannelAck;

/***************************************************************************
Summary:
Sage Channel Message Response structure

Description:
Used to send a response from the SAGE (Write on the SAGE; Read on the Host)
Exchanged through SAGE-Host Data Buffer (used for the reverse communication)
***************************************************************************/
typedef struct NEXUS_SageChannelResponse
{
    uint64_t messageOffset; /* Physical address of a NEXUS_SageMessage */
    uint32_t rc;            /* Request processing return Code */
} NEXUS_SageChannelResponse;



/* message protocol version */
#define NEXUS_SAGE_MESSAGE_VERSION_MINOR 0x0000
#define NEXUS_SAGE_MESSAGE_VERSION_MAJOR 0x0001
#define NEXUS_SAGE_MESSAGE_VERSION ((NEXUS_SAGE_MESSAGE_VERSION_MAJOR<< 16) | NEXUS_SAGE_MESSAGE_VERSION_MINOR)


/***************************************************************************
Summary:
Sage Channel Message definition

Description:
Message exchanged between the Host and SAGE
This message is shared and not written down on the wire
(only a reference to it is exchanged through NEXUS_SageChannelRequest/Ack/Response)
***************************************************************************/
typedef struct NEXUS_SageMessage
{
    uint32_t version;     /* Sage version : NEXUS_SAGE_MESSAGE_VERSION */
    uint32_t timestamp;   /* Time Stamp in micro seconds */
    uint32_t sequence;    /* Message Sequence number, unique system-wide */
    uint32_t instanceId;  /* Instance ID <-- --> Channel Handle
                             At NEXUS Sage point of view its a Channel reference,
                             at SAGE-side point of view its a instance identifier */
    uint32_t systemCommandId;   /* System Command ID, processed by SAPM */
    uint32_t platformId;        /* Platform ID (unique in the system) */
    uint32_t moduleId;          /* Module ID (unique per Platform) */
    uint32_t moduleCommandId;
    /* next has to be 64 bits aligned */
    uint64_t payloadOffset;
} NEXUS_SageMessage;

#ifdef NEXUS_SAGE_MESSAGE_DEBUG
#define NEXUS_SAGE_DUMP_MESSAGE(STR, MESSAGE, RAW)                \
    if (RAW) {                                                    \
        unsigned int i, j;                                        \
        uint8_t *b = (uint8_t *)(MESSAGE);                        \
        for (i = 0, j = 0; i < sizeof(*(MESSAGE)); i+=4, j++) {   \
            uint32_t *w = (uint32_t *)b;                          \
            BDBG_MSG(("DUMP #%02d\t0x%08x\t%02x %02x %02x %02x ", \
                      j, *w, *b, *(b+1), *(b+2), *(b+3)));        \
            b+=4;                                                 \
        }                                                         \
    }                                                             \
    BDBG_MSG(("DUMP message %s @ 0x%08x (%d bytes):", (STR), (void *)(MESSAGE), sizeof(*(MESSAGE)))); \
    BDBG_MSG(("        version=%08x",         (MESSAGE)->version));                                   \
    BDBG_MSG(("        timestamp=%08x",       (MESSAGE)->timestamp));                                 \
    BDBG_MSG(("        sequence=%08x",        (MESSAGE)->sequence));                                  \
    BDBG_MSG(("        instanceId=%08x",      (MESSAGE)->instanceId));                                \
    BDBG_MSG(("        systemCommandId=%08x", (MESSAGE)->systemCommandId));                           \
    BDBG_MSG(("        platformId=%08x",      (MESSAGE)->platformId));                                \
    BDBG_MSG(("        moduleId=%08x",        (MESSAGE)->moduleId));                                  \
    BDBG_MSG(("        moduleCommandId=%08x", (MESSAGE)->moduleCommandId));                           \
    BDBG_MSG(("        payloadOffset=0x%08x", (uint32_t)(MESSAGE)->payloadOffset));
#else
#define NEXUS_SAGE_DUMP_MESSAGE(STR, MESSAGE, RAW)
#endif

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_SAGE_MESSAGE_TYPES_H__ */
