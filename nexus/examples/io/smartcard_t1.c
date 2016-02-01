/******************************************************************************
 *    (c)2010-2013 Broadcom Corporation
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
#include <stdlib.h>

#include "smartcard.h"

#if NEXUS_HAS_SMARTCARD
unsigned char g_resetdata[] = {
    0x3F,  0xFA,  0x11,  0x25,  0x02,  0x00,  0x21,
    0xB0,  0xA1,  0x04,  0x05,  0x06,  0x07,  0x08,  0x09,  0x0A};

#define TOTAL_COMMANDS 1
#define BUFSIZE 27

struct commands {
    int num_send;
    unsigned char send[264];
    int num_receive;
    unsigned char receive[264];
} g_commands[1] = {
    {5, {0x21, 0xc1, 0x01, 0x58, 0xb9}, 5, {0x12, 0xe1, 0x01, 0x58, 0xaa }}
    };

struct atr {
    unsigned char smartcardType;
    char buf[BUFSIZE];
    unsigned int size;
    unsigned char protocol[16];
    NEXUS_SmartcardSettings settings;
} atr;


#define FAIL() \
    do {printf("Failed on line %d\n", __LINE__); goto error; } while (0)


static void ResetInterface(NEXUS_SmartcardHandle smartcard);
static int ResetCard(NEXUS_SmartcardHandle smartcard);
#endif

int main(int argc, char **argv)
{
#if NEXUS_HAS_SMARTCARD
    /* Open will block until the card is inserted. */
    NEXUS_PlatformSettings platformSettings;
    NEXUS_SmartcardHandle smartcard;
    unsigned char buf[BUFSIZE];
    int n, counter = 0;
    int loops;
    int i;
    NEXUS_Error rc ;

    if ( argc == 2 )
    {
        atr.smartcardType = atoi(argv[1]);
        fprintf(stderr, "Smartcard Type = %d\n", atr.smartcardType);
        if ( atr.smartcardType > 2 )
        {
            fprintf(stderr, "usage: %s <filename> 0(for Default)/ 1 (for Dss Icam)/ 2(for Nagra card)\n", argv[0]);
            return -1;
        }
    }
    else {
        fprintf(stderr, "usage: %s <filename> 0(for Default)/ 1 (for Dss Icam)/ 2(for Nagra card)\n", argv[0]);
        return -1;
    }


    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    platformSettings.smartCardSettings.clockFrequency = 36000000;
    platformSettings.smartCardSettings.clockSource = NEXUS_SmartcardClockSource_eInternalClock;
    if(NEXUS_Platform_Init(&platformSettings)){
        printf("Init failed");
        FAIL();
    }

    NEXUS_Smartcard_GetDefaultSettings(&atr.settings);
    atr.settings.standard = NEXUS_SmartcardStandard_eIso;
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

    NEXUS_Smartcard_SetSettings(smartcard, &atr.settings);

    loops = 0;
    while (counter < 30) {
        if (loops == 0)
            i = 0;
        for ( ; i<TOTAL_COMMANDS; i++) {
            /*printf("bsmartcard_write\n"); */
            rc = NEXUS_Smartcard_Write(smartcard, g_commands[i].send, g_commands[i].num_send, (size_t *)&n);
            if (rc) {
                fprintf(stderr, "Write Failed.");
                ResetInterface(smartcard);
                goto startover;
            }

            if (n != g_commands[i].num_send)
                FAIL();

            /*printf("bsmartcard_read\n"); */
            rc = NEXUS_Smartcard_Read(smartcard, buf, g_commands[i].num_receive, (size_t *)&n);
            if (rc) {
                fprintf(stderr, "Read Failed.");
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

error:
    NEXUS_Smartcard_Close(smartcard);
    return 0;
#else
    BSTD_UNUSED(argc);
    BSTD_UNUSED(argv);
    return -1;
#endif
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

static void DecodeTB1Byte(char tb1){
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

static unsigned char CliGetPrescale(void)
{
    fprintf(stderr, "preScale =%d\n", CliDf[atr.smartcardType][atr.settings.dFactor][atr.settings.fFactor].preScale);
    return CliDf[atr.smartcardType][atr.settings.dFactor][atr.settings.fFactor].preScale;
}

static unsigned char CliGetBaudDiv(void)
{
   fprintf(stderr, "Baud Div =%d\n", CliDf[atr.smartcardType][atr.settings.dFactor][atr.settings.fFactor].baudDiv);
    return CliDf[atr.smartcardType][atr.settings.dFactor][atr.settings.fFactor].baudDiv;
}

static unsigned char CliGetClkDiv(void)
{
   fprintf(stderr, "Clk Div =%d\n", CliDf[atr.smartcardType][atr.settings.dFactor][atr.settings.fFactor].clkDiv);
   return CliDf[atr.smartcardType][atr.settings.dFactor][atr.settings.fFactor].clkDiv;
}

static void DecodeTB3Byte(char tb3){
    unsigned int guardTime, guardTimePlusOne, blockWaitTime, prescale;
    unsigned char cwiVal, bwiVal;
    unsigned char baudDiv, clkDiv;
    unsigned int cwiCheck;

    cwiVal = tb3 & 0x0f;
    bwiVal = (tb3 >> 4) & 0x0f;

    if (cwiVal == 0x00) {
        cwiCheck = 1;
    }
    else {
        cwiCheck = 2<<(cwiVal-1);
    }

    guardTime = atr.settings.extraGuardTime.value;

    if (guardTime == 0xff)
        guardTimePlusOne = 0;
    else
        guardTimePlusOne = guardTime + 1;


    atr.settings.characterWaitTime &= 0xf0;
    atr.settings.characterWaitTime |= cwiVal;

    prescale = CliGetPrescale() * atr.settings.extClockDivisor + (atr.settings.extClockDivisor - 1);
    baudDiv = CliGetBaudDiv();
    clkDiv = CliGetClkDiv();

    if (bwiVal == 0x00) {
        blockWaitTime = 960 * 372 * clkDiv /
        (prescale+1) / baudDiv + 11;
    }
    else {
        blockWaitTime = (2<<(bwiVal-1)) * 960 *  372 *
        clkDiv / (prescale+1) / baudDiv  + 11;
    }

    /* Change timer to equal calculated BWT */
    atr.settings.blockWaitTime.value = blockWaitTime ;
    atr.settings.blockWaitTime.unit = NEXUS_TimerUnit_eEtu;

}

static int DecodeAtr(void){
    int n=0;
    unsigned int k=0;
    int t0, td1, td2;

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
    if (td1  & 0x10 ) {
        n++;
    }
    if(td1 & 0x20) {
        n++;
    }

    if (td1 & 0x40) {
        n++;
    }
    if (td1 & 0x80) {
        td2 = atr.buf[n++];
    }

    if (td2  & 0x10 ) {
        n++;
    }
    if(td2 & 0x20) {
        DecodeTB3Byte(atr.buf[n++]);
    }

    if (td2 & 0x40) {
        n++;
    }
    if (td2 & 0x80) {
        /*td3 = atr.buf[n++];       */
        n++;
    }

    return 0;
}

static int ResetCard(NEXUS_SmartcardHandle smartcard)
{
    unsigned int j;

    if (NEXUS_Smartcard_ResetCard(smartcard, (void *)atr.buf, BUFSIZE, (size_t *)(&atr.size)))
        FAIL();

    printf("Bytes Read = %d\n", atr.size);

    for (j=0; j<atr.size; j++)
        printf("0x%02x ", (unsigned char)atr.buf[j]);

    printf("\n");


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


