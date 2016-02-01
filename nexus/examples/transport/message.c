/******************************************************************************
 *    (c)2008-2011 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *****************************************************************************/
#include "nexus_platform.h"
#include "nexus_pid_channel.h"
#include "nexus_parser_band.h"
#include "nexus_message.h"

#include "bstd.h"
#include "bkni.h"

#include <stdio.h>

BDBG_MODULE(message);

void message_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

int main(void) {
    NEXUS_ParserBand parserBand = NEXUS_ParserBand_e0;
    NEXUS_PidChannelHandle pidChannel;
    NEXUS_MessageHandle msg;
    NEXUS_MessageSettings settings;
    NEXUS_MessageStartSettings startSettings;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_PidChannelSettings pidChannelSettings;
    BKNI_EventHandle event;
    NEXUS_Error rc;
    unsigned count = 100;

    NEXUS_Platform_Init(NULL);

    NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
    NEXUS_Platform_GetStreamerInputBand(0, &parserBandSettings.sourceTypeSettings.inputBand);
    NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);

    NEXUS_PidChannel_GetDefaultSettings(&pidChannelSettings);
    pidChannel = NEXUS_PidChannel_Open(parserBand, 0x0, &pidChannelSettings);

    BKNI_CreateEvent(&event);

    NEXUS_Message_GetDefaultSettings(&settings);
    settings.dataReady.callback = message_callback;
    settings.dataReady.context = event;
    /* use default settings.maxContiguousMessageSize */
    msg = NEXUS_Message_Open(&settings);
    BDBG_ASSERT(msg);

    NEXUS_Message_GetDefaultStartSettings(msg, &startSettings);
    startSettings.pidChannel = pidChannel;
    /* use the default filter for any data */

    rc = NEXUS_Message_Start(msg, &startSettings);
    BDBG_ASSERT(!rc);

    /* Read the PAT a few times */
    while (count--) {
        const uint8_t *buffer;
        size_t size;
        int programNum, message_length;

        rc = NEXUS_Message_GetBuffer(msg, (const void **)&buffer, &size);
        BDBG_ASSERT(!rc);
        if (!size) {
            BERR_Code rc = BKNI_WaitForEvent(event, 5 * 1000); /* wait 5 seconds */
            if (rc) {BDBG_ERR(("test failed")); rc = -1; break;}
            continue;
        }

#define TS_READ_16( BUF ) ((uint16_t)((BUF)[0]<<8|(BUF)[1]))
#define TS_PSI_GET_SECTION_LENGTH( BUF )    (uint16_t)(TS_READ_16( &(BUF)[1] ) & 0x0FFF)

        /* We should always get whole PAT's because maxContiguousMessageSize is 4K */
        message_length = TS_PSI_GET_SECTION_LENGTH(buffer) + 3;
        BDBG_ASSERT(size >= (size_t)message_length);

        printf("Found PAT: id=%d size=%d\n", buffer[0], message_length);
        for (programNum=0;programNum<(TS_PSI_GET_SECTION_LENGTH(buffer)-7)/4;programNum++) {
            unsigned byteOffset = 8 + programNum*4;
            printf("  program %d: pid 0x%x\n",
                TS_READ_16( &buffer[byteOffset] ),
                (uint16_t)(TS_READ_16( &buffer[byteOffset+2] ) & 0x1FFF));
        }

        /* XPT HW is configured to pad all messages to 4 bytes. If we are calling NEXUS_Message_ReadComplete
        based on message length and not the size returned by NEXUS_Message_GetBuffer, then we must add that pad.
        If we are wrong, NEXUS_Message_ReadComplete will fail. */
        if (message_length % 4) {
            message_length += 4 - (message_length % 4);
        }

        /* only complete one PAT */
        rc = NEXUS_Message_ReadComplete(msg, message_length);
        BDBG_ASSERT(!rc);
    }

    NEXUS_Message_Stop(msg);
    NEXUS_Message_Close(msg);
    NEXUS_PidChannel_Close(pidChannel);
    BKNI_DestroyEvent(event);
    NEXUS_Platform_Uninit();
    return 0;
}
