/******************************************************************************
 *    (c)2008-2012 Broadcom Corporation
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
#if NEXUS_HAS_SMARTCARD
#include "nexus_smartcard.h"
#endif
#include "bstd.h"
#include "bkni.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>


#if NEXUS_HAS_SMARTCARD
unsigned char g_resetdata[] = {
    0x3F,  0xFA,  0x11,  0x25,  0x02,  0x00,  0x21,
    0xB0,  0xA1,  0x04,  0x05,  0x06,  0x07,  0x08,  0x09,  0x0A};

#define TOTAL_COMMANDS 4
#define BUFSIZE 16

struct commands {
    int num_send;
    unsigned char send[264];
    int num_receive;
    unsigned char receive[264];
} g_commands[4] = {
    {5, {0x00, 0x42, 0x00, 0x00, 0x01}, 1, {0x42}}, /* Sending NDS Baud Rate */
    {1, {0x00}, 2, {0x90,0x00}},                /* Set to 9600 baud */
    {5, {0x00, 0x84,  0x00,  0x84,  0x00}, 3, {0x84,  0x90,  0x00}}, /* Sending NDS Procedure: */
    {5, {0x00, 0x84,  0x90,  0x00,  0x00}, 2, {0x90,  0x00}}    /* Sending NDS Procedure: */
    };

struct atr {
    char buf[BUFSIZE];
    unsigned char size;
    unsigned char protocol[16];
    NEXUS_SmartcardSettings settings;
} atr;


#define FAIL() \
    do {printf("Failed on line %d\n", __LINE__); goto error; } while (0)


static void ResetInterface(NEXUS_SmartcardHandle smartcard);
static int ResetCard(NEXUS_SmartcardHandle smartcard);

static void card_callback(void *context, int param)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_SmartcardHandle smartcard = (NEXUS_SmartcardHandle)context;
    NEXUS_SmartcardStatus status;

    BSTD_UNUSED(param);
    rc = NEXUS_Smartcard_GetStatus(smartcard, &status);
    if(rc){rc = BERR_TRACE(rc);}
    
    printf("status.cardPresent = %d.\n", status.cardPresent);
}

#endif /* NEXUS_HAS_SMARTCARD */

int main(int argc, char **argv)
{
#if NEXUS_HAS_SMARTCARD
    /* Open will block until the card is inserted. */
    NEXUS_SmartcardHandle smartcard;
    unsigned char buf[BUFSIZE];
    int n, counter = 0;
    int loops;
    int i;
    NEXUS_Error rc ;

    argc=argc;
    argv=argv;

    if(NEXUS_Platform_Init(NULL)){
        printf("Init failed");
        FAIL();     
    }
    BKNI_Memset(&atr, 0, sizeof(atr));
    
    NEXUS_Smartcard_GetDefaultSettings(&atr.settings);
    atr.settings.standard = NEXUS_SmartcardStandard_eNds;
    smartcard = NEXUS_Smartcard_Open(0, &atr.settings);
    
    if ( NULL == smartcard )
    {
        fprintf(stderr, "Unable to open smartcard");
        return -1;
    }


    NEXUS_Smartcard_Reset(smartcard, false); /* This may error if no card is present, the error may be safely ignored.  It is required to finish the interface initialization. */
    NEXUS_Smartcard_DetectCard(smartcard);  /* This should block until a smartcard is inserted */
    
    /* You must reset the interface after opening. */
    ResetInterface(smartcard);
    
    atr.settings.cardCallback.callback = card_callback;
    atr.settings.cardCallback.context = smartcard;    
    NEXUS_Smartcard_SetSettings(smartcard, &atr.settings);

    loops = 0;
    while (counter < 30) {
        if (loops == 0)
            i = 0;
        else
            i = 2; /* don't repeat the 1st two commands */
        for ( ; i<TOTAL_COMMANDS; i++) {
            /*printf("bsmartcard_write\n"); */
            rc = NEXUS_Smartcard_Write(smartcard, g_commands[i].send, g_commands[i].num_send, (size_t *)&n);
            if (rc) {
                ResetInterface(smartcard);
                goto startover;
            }

            if (n != g_commands[i].num_send)
                FAIL();

            /*printf("bsmartcard_read\n"); */
            rc = NEXUS_Smartcard_Read(smartcard, buf, g_commands[i].num_receive, (size_t *)&n);
            if (rc) {
                ResetInterface(smartcard);
                goto startover;
            }
            if (n != g_commands[i].num_receive || memcmp(g_commands[i].receive, buf, n))
            {
                ResetInterface(smartcard);
                goto startover;
            }
        }
        printf("Successful transaction.\n");
        counter++;
        loops++;
        continue;
startover:
        i = -1;
        loops = 0;
    }

    printf("Press enter to exit.\n");
    getchar();
    NEXUS_Smartcard_Deactivate(smartcard);

error:
    NEXUS_Smartcard_Close(smartcard);
#else
    BSTD_UNUSED(argc);
    BSTD_UNUSED(argv);
#endif /* NEXUS_HAS_SMARTCARD */
    return 0;
}

#if NEXUS_HAS_SMARTCARD
static int DecodeTSByte(char ts){
    if (ts == 0x3f) {
        fprintf(stderr, "TS = %02x, Inverse Convention\n", ts); return BERR_SUCCESS;
    }
    else if (ts == 0x3b) {
        fprintf(stderr, "TS = %02x, Direct Convention\n", ts); return BERR_SUCCESS;
    }
    else {
        fprintf(stderr, "TS = %02x, Unknown Convention\n", ts);
    }
    return BERR_NOT_SUPPORTED;
}

static void DecodeT0Byte(char t0, unsigned int  *k){
    *k = (unsigned int)(t0 & 0x0f);
}

static void DecodeTA1Byte(char ta1){
    atr.settings.fFactor = (ta1 >> 4) & 0x0f;
    atr.settings.dFactor = ta1 & 0x0f;
}

static void DecodeTB1Byte(char tb1)
{
    BSTD_UNUSED(tb1);
}

static int DecodeTD1Byte(char td1){
    unsigned char t = td1 & 0x0f;

    atr.protocol[t] = 1;

    switch (t) {
        case 0:         
            atr.settings.protocol = NEXUS_SmartcardProtocol_eT0;
            break;
        case 1:
            atr.settings.protocol = NEXUS_SmartcardProtocol_eT1;
            break;
        case 14:
            atr.settings.protocol = NEXUS_SmartcardProtocol_eT14;
            break;
        default:
            fprintf(stderr, "Unsupported protocol %02x\n", t);
    }
    return BERR_SUCCESS;
}
static int DecodeAtr(void){ 
    int n=0;
    unsigned int k=0;
    int t0, td1;
    
    if(DecodeTSByte(atr.buf[n++])) return BERR_NOT_SUPPORTED;

    t0 = atr.buf[n++];
    DecodeT0Byte(t0, &k) ;
        
    if (t0  & 0x10 ) { 
        DecodeTA1Byte(atr.buf[n++]);
    } 
    else {
        atr.settings.fFactor = 1;
        atr.settings.dFactor = 1;   
    }
    
    if(t0 & 0x20) {
        DecodeTB1Byte(atr.buf[n++]);    
    }
    
    if (t0 & 0x40) {
        atr.settings.extraGuardTime.value = atr.buf[n++];
        atr.settings.extraGuardTime.unit = NEXUS_TimerUnit_eEtu;
    }
    else { 
        atr.settings.extraGuardTime.value = 0;
    }
    if (t0 & 0x80) {
        td1 = atr.buf[n++];
        DecodeTD1Byte(td1);
    }

    return 0;   
}

static int ResetCard(NEXUS_SmartcardHandle smartcard)
{
    if (NEXUS_Smartcard_ResetCard(smartcard, (void *)atr.buf, BUFSIZE, (size_t *)(&atr.size)))
        FAIL();

    if (DecodeAtr())
        FAIL();

    return 0;
error:
    return -1;
}


/*
Reset the interface.

If the card is not present, or if the bsmartcard_reset_card fails,
we must call bsmartcard_reset to reset the interface. i
The result is that this function blocks until a card is inserted.

The reason it must do this is that any error on any function might simply be
the removal of the card.
*/
static void ResetInterface(NEXUS_SmartcardHandle smartcard)
{
    while (1) {
        if (NEXUS_Smartcard_Reset(smartcard, false))
            continue;

        NEXUS_Smartcard_DetectCard(smartcard);

        if (!ResetCard(smartcard))
            break;
    }
}
#endif
