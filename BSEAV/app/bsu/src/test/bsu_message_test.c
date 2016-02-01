/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
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
 ***************************************************************************/
#include <stdio.h>
#include "nexus_platform.h"
#include "nexus_pid_channel.h"
#include "nexus_parser_band.h"
#include "nexus_message.h"
#include "bstd.h"
#include "bkni.h"
#include "bsu-api.h"
#include "bsu-api2.h"

BDBG_MODULE(message);

void message_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

void bsu_message_test(void) 
{
    NEXUS_ParserBand parserBand = NEXUS_ParserBand_e0;
    NEXUS_PidChannelHandle pidChannel;
    NEXUS_MessageHandle msg;
    NEXUS_MessageSettings settings;
    NEXUS_MessageStartSettings startSettings;
    NEXUS_PidChannelSettings pidChannelSettings;
    BKNI_EventHandle event;
    NEXUS_Error rc;
    unsigned count = 20;

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

    return;
}
