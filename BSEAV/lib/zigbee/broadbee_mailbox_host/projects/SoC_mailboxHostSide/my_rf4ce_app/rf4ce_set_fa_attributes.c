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
******************************************************************************/
/* Simple RF4CE app */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <termios.h>
#include "bbMailAPI.h"
#include "zigbee_api.h"
#include "zigbee.h"
#include "bbSysPayload.h"
#include "zigbee_rf4ce_registration.h"
#include "zigbee_dbg.h"

# pragma GCC optimize "short-enums"     /* Implement short enums. */
typedef enum _TEST_enum_t
{
    TEST_ENUM_1 = 0x00,
} TEST_enum_t;

SYS_DbgAssertStatic(sizeof(TEST_enum_t) == 1)

#define SYS_DBG_LOG_BUFFER_SIZE     256

#  define HAL_DbgLogStr(message)                                TEST_DbgLogStr(message)

void sysDbgHalt(const uint32_t errorUid /* , const char *const fileName, const uint32_t fileLine ) */
# if defined(_DEBUG_FILELINE_)
                                           , const char *const fileName, const uint32_t fileLine
# endif
                                                                                                 )
{
    HAL_IRQ_DISABLE();
# if defined(_DEBUG_FILELINE_)
#  if (_DEBUG_CONSOLELOG_ >= 1)
    sysDbgLogStr("HALT: %s(%d) - 0x%08X", fileName, fileLine, errorUid);
#  endif
#  if (_DEBUG_HARNESSLOG_ >= 1)
    HAL_DbgHalt(errorUid, fileName, fileLine);
#  endif

# else /* ! _DEBUG_FILELINE_ */
#  if (_DEBUG_CONSOLELOG_ >= 1)
    sysDbgLogStr("HALT: 0x%08X", errorUid);
#  endif
#  if (_DEBUG_HARNESSLOG_ >= 1)
    HAL_DbgHalt(errorUid);
#  endif

# endif /* ! _DEBUG_FILELINE_ */

    while(1);
}



#if defined(_DEBUG_LOG_)
/*
 * Logs warning and proceeds with program execution.
 */
void sysDbgLogId(const uint32_t warningUid /* , const char *const fileName, const uint32_t fileLine ) */
# if defined(_DEBUG_FILELINE_)
                                              , const char *const fileName, const uint32_t fileLine
# endif
                                                                                                    )
{
# if defined(_DEBUG_FILELINE_)
#  if (_DEBUG_CONSOLELOG_ >= 2)
    sysDbgLogStr("WARN: %s(%d) - 0x%08X", fileName, fileLine, warningUid);
#  endif
#  if (_DEBUG_HARNESSLOG_ >= 2)
    HAL_DbgLogId(warningUid, fileName, fileLine);
#  endif

# else /* ! _DEBUG_FILELINE_ */
#  if (_DEBUG_CONSOLELOG_ >= 2)
    sysDbgLogStr("WARN: 0x%08X", warningUid);
#  endif
#  if (_DEBUG_HARNESSLOG_ >= 2)
    HAL_DbgLogId(warningUid);
#  endif

# endif /* ! _DEBUG_FILELINE_ */
}
#endif /* _DEBUG_LOG_ */


#if defined(_DEBUG_LOG_) || (defined(_DEBUG_) && (_DEBUG_CONSOLELOG_ >= 1))
/*
 * Logs custom formatted debugging string with auxiliary parameters.
 */
void sysDbgLogStr(const char *const format, ...)
{
    char     message[SYS_DBG_LOG_BUFFER_SIZE];      /* String buffer for the message to be logged. */
    va_list  args;                                  /* Pointer to the variable arguments list of this function. */

    va_start(args, format);
    vsnprintf(message, SYS_DBG_LOG_BUFFER_SIZE, format, args);     /* TODO: Implement custom tiny formatted print. */
    va_end(args);

    HAL_DbgLogStr(message);
}
#endif

static void rf4ce_Test_Set_Fa_Attr(uint8_t attrId, int8_t value)
{
    uint8_t statusSetFaAttr = 0;

    RF4CE_NWK_SetReqDescr_t req = {0};
    void rf4ce_Test_Set_Fa_Attr_Callback(RF4CE_NWK_SetReqDescr_t *request, RF4CE_NWK_SetConfParams_t *conf)
    {
        if(0 == conf->status)
            statusSetFaAttr = 1;
        else
            printf("Set FA Attributes failed\r\n");
    }

    req.params.attrId.attrId = attrId;
    req.callback = rf4ce_Test_Set_Fa_Attr_Callback;
    switch (attrId)
    {
        case RF4CE_NWK_FA_SCAN_THRESHOLD:
            req.params.data.nwkFaScanThreshold = value;
            break;
         case RF4CE_NWK_FA_COUNT_THRESHOLD:
            req.params.data.nwkFaCountThreshold = value;
            break;
         case RF4CE_NWK_FA_DECREMENT:
            req.params.data.nwkFaDecrement = value;
            break;
         default:
            printf("Default\r\n");
    }

    RF4CE_NWK_SetReq(&req);
    while(!statusSetFaAttr);
}

static int rf4ce_Test_Get_Fa_Attr(uint8_t attrId)
{
    uint8_t statusGetFaAttr = 0;
    int value;

    RF4CE_NWK_GetReqDescr_t req = {0};

    void rf4ce_Test_Get_Fa_Attr_Callback(RF4CE_NWK_GetReqDescr_t *request, RF4CE_NWK_GetConfParams_t *conf)
    {
        if(0 == conf->status)
        {
            switch (attrId)
            {
                case RF4CE_NWK_FA_SCAN_THRESHOLD:
                     value= conf->data.nwkFaScanThreshold;
                    break;
                 case RF4CE_NWK_FA_COUNT_THRESHOLD:
                     value=  conf->data.nwkFaCountThreshold;
                    break;
                 case RF4CE_NWK_FA_DECREMENT:
                    value=  conf->data.nwkFaDecrement;
                    break;
            }
            statusGetFaAttr = 1;
        }
        else
            printf("Get FA Attributes failed\r\n");
    }

    req.params.attrId.attrId = attrId;
    req.callback = rf4ce_Test_Get_Fa_Attr_Callback;

    RF4CE_NWK_GetReq(&req);
    while(!statusGetFaAttr);

    return value;
}

const char usage[] = "\nrf4ce_set_fa_attributes - Set rf4ce Frequency Agility Attributes\n\n"
                     "    usage: nrf4ce_set_fa_attributes [threshold] [count threshold] [decrement]\n"
                     "    If any field missed ('-' is used for any other value followed), current value is used.\n";

int main(int argc, char *argv[])
{
    struct zigbeeCallback zcb;

    if (argc <2)
    {
        printf(usage);
        return -1;
    }

    RF4CE_StartReqDescr_t request;
    static struct termios oldt, newt;

#ifdef BYPASS_RPC
    extern int zigbee_init(int argc, char *argv[]);
    zigbee_init(argc, argv);
#endif

    /* Register the callback functions you are interested in.  Ones that are not filled out, won't be called back. */
    /* Calling Zigbee_GetDefaultSettings will initialize the callback structure */
    Zigbee_GetDefaultSettings(&zcb);
    Zigbee_Open(&zcb, "127.0.0.1");

    int threshold0 = rf4ce_Test_Get_Fa_Attr(RF4CE_NWK_FA_SCAN_THRESHOLD);
    int cntThreshold0 = rf4ce_Test_Get_Fa_Attr(RF4CE_NWK_FA_COUNT_THRESHOLD);
    int decrement0 = rf4ce_Test_Get_Fa_Attr(RF4CE_NWK_FA_DECREMENT);

    int threshold = 0;
    int cntThreshold = 0;
    int decrement = 0;

    if (argc > 1)
        sscanf(argv[1], "%d", &threshold);
    if (argc > 2)
        sscanf(argv[2], "%d", &cntThreshold);
    if (argc > 3)
        sscanf(argv[3], "%d", &decrement);

    if (0 == threshold) threshold = threshold0;
    if (0 == cntThreshold) cntThreshold = cntThreshold0;
    if (0 == decrement) decrement = decrement0;

    printf("threshold    = %3d => %3d\n",threshold0,threshold);
    printf("cntThreshold = %3d => %3d\n",cntThreshold0,cntThreshold);
    printf("decrement    = %3d => %3d\n",decrement0, decrement);

    rf4ce_Test_Set_Fa_Attr(RF4CE_NWK_FA_SCAN_THRESHOLD,(int8_t)threshold);
    rf4ce_Test_Set_Fa_Attr(RF4CE_NWK_FA_COUNT_THRESHOLD,(int8_t)cntThreshold);
    rf4ce_Test_Set_Fa_Attr(RF4CE_NWK_FA_DECREMENT,(int8_t)decrement);

    Zigbee_Close();

    return 0;
}
