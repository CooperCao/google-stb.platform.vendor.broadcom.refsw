/******************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

/* base modules */
#include <stdio.h>
#include <inttypes.h>
#include "bstd.h"           /* standard types */
#include "berr.h"           /* error code */
#include "bdbg.h"           /* debug interface */
#include "bkni.h"           /* kernel interface */

#include "bafl.h"
#include "bchp_pwr.h"

#include "basp.h"
#include "basp_priv.h"

#include "basp_image.h"
#include "bchp_asp_arcss_ctrl.h" /*TODO: This will be removed once reg structure is restructured. */
#include "bchp_xpt_mcpb_host_intr_aggregator.h" /* This is reuired to populate init message with XPT_MCPB_HOST_INTR_AGGREGATOR_INTR_W0_STATUS.*/
#include "bchp_xpt_memdma_mcpb_host_intr_aggregator.h" /* This is required to populate init message with  BCHP_XPT_MEMDMA_MCPB_HOST_INTR_AGGREGATOR_INTR_W0_STATUS */
#include "bchp_asp_core_host_intr_aggregator.h"
#include "bchp_int_id_asp_arcss_host_fw2h_l2.h"
#include "bchp_xpt_rave.h"  /* For Debug only. */

/* For accessing the ASP's DCCM (data memory) for debug. */
#include "bchp_asp_arcss_dccm.h"

BDBG_MODULE(BASP);
BDBG_OBJECT_ID(BASP_P_Device);           /* BASP_Handle */


#define BDBG_NUL(x)

BASP_Pi2Fw_Message gsPi2FwMessage;

/* Define an INT_ID at bit zero to represent the non-standard ASP Watchdog timer #1. */
#define BCHP_INT_ID_ASP_ARCSS_CTRL_WD_TIMER1_INTR     BCHP_INT_ID_CREATE(BCHP_ASP_ARCSS_CTRL_WD_TIMER1_CONTROL, 0)

/* Define some meaningful names for the FW2H INT_IDs. */
#define BASP_INT_ID_FW2H_ACK_BIT           BCHP_INT_ID_ASP_ARCSS_HOST_FW2H_L2_ARCSS_HOST00_INTR /* For acking host message */
#define BASP_INT_ID_FW2H_RESP_BIT          BCHP_INT_ID_ASP_ARCSS_HOST_FW2H_L2_ARCSS_HOST01_INTR /* For response to host message */
#define BASP_INT_ID_FW2H_FINNOTIFY_BIT     BCHP_INT_ID_ASP_ARCSS_HOST_FW2H_L2_ARCSS_HOST02_INTR /* For FIN notify message */
#define BASP_INT_ID_FW2H_RSTNOTIFY_BIT     BCHP_INT_ID_ASP_ARCSS_HOST_FW2H_L2_ARCSS_HOST03_INTR /* For RST notify message */
#define BASP_INT_ID_FW2H_FINCOMP_BIT       BCHP_INT_ID_ASP_ARCSS_HOST_FW2H_L2_ARCSS_HOST04_INTR /* For FIN completion message */
#define BASP_INT_ID_FW2H_PAYNOTIFY_BIT     BCHP_INT_ID_ASP_ARCSS_HOST_FW2H_L2_ARCSS_HOST05_INTR /* For payload notify message */
#define BASP_INT_ID_FW2H_FRAMEAVAIL_BIT    BCHP_INT_ID_ASP_ARCSS_HOST_FW2H_L2_ARCSS_HOST06_INTR /* For frame available message */
#define BASP_INT_ID_FW2H_SGFEEDCOMP_BIT    BCHP_INT_ID_ASP_ARCSS_HOST_FW2H_L2_ARCSS_HOST07_INTR /* For generic sg table feed completion message */
#define BASP_INT_ID_FW2H_RTONOTIFY_BIT     BCHP_INT_ID_ASP_ARCSS_HOST_FW2H_L2_ARCSS_HOST08_INTR /* For RTO notify message */

/* Function to provide cached byte access to the ASP's DCCM (data memory). */
static char BASP_P_GetDccmByte_isr(BASP_Handle hAsp,  uint32_t addr)
{
    BREG_Handle hReg = hAsp->handles.hReg;
    unsigned    byteIndex;          /* Index to byte within word. */

    union    /* Cached data in cachedAddress. */
    {
        uint32_t   val32;       /* As 32-bit word. */
        uint8_t    val8[4];     /* As 4 bytes. */
    } cachedValue;

    BKNI_ASSERT_ISR_CONTEXT();
    /* Passing address of zero invalidates the cached word. */
    if (addr == 0)
    {
        hAsp->DBG_HostPrintf.cachedAddress = 0;
        return 0;
    }

    /* Computer word-aligned address, and offset to desired byte. */
    byteIndex = addr & 0x3;
    addr = addr & ~0x3;

    /* If word isn't in cache, read it. */
    if (hAsp->DBG_HostPrintf.cachedAddress != addr)
    {
        hAsp->DBG_HostPrintf.cachedValue = BREG_Read32(hReg,  addr);
        hAsp->DBG_HostPrintf.cachedAddress = addr;
    }

    /* Return the desired byte from the cache. */
    cachedValue.val32 = hAsp->DBG_HostPrintf.cachedValue;
    return cachedValue.val8[byteIndex];
}


#define BASP_DBG_HOSTPRINTFCHECK_INTERVAL (10 * 1000)   /* How long (in microseconds) between checks for firmware debug messages. */

/* Function to read and print debug log messages from the ASP. */
void BASP_DBG_HostPrintfCheck_isr(
        void *pParm1,
        int parm2
        )
{
    BASP_Handle hAsp = pParm1;
    BREG_Handle hReg = hAsp->handles.hReg;
    uint32_t regVal = 0;

    uint32_t bufferBaseDccmAddr = 0;
    uint32_t readPtr = 0;
    uint32_t writePtr = 0;

    Fw2HostPrintBuffer_t *pF2HDebugBlock;
    BERR_Code rc = BERR_SUCCESS;

    BSTD_UNUSED (parm2);

    BKNI_ASSERT_ISR_CONTEXT();

    BDBG_NUL(("%s: Entry...", BSTD_FUNCTION));

    /* See if the address of the firmware's debug buffer control block has been put
     * into the SYNC_LOCAL_RBUS register... If it isn't there yet, that's all for now. */
    regVal = BREG_Read32_isr(hReg, BCHP_ASP_ARCSS_CTRL_SYNC_LOCAL_RBUS);
    if (regVal == 0) {goto done; }

    /* The control block's address is there, now convert it from a DCCM address to an
     * address in our virtual address space. */
    pF2HDebugBlock = (Fw2HostPrintBuffer_t *) (unsigned long) (BCHP_ASP_ARCSS_DCCM_DATAi_ARRAY_BASE + (regVal & 0x1ffff));

    /* Find the address of the base of the ring buffer. */
    bufferBaseDccmAddr = (unsigned long)(&pF2HDebugBlock->ui8CharBuffer[0]);

    /* Get the current values of the read and write pointers from the control block. */
    readPtr = BREG_Read32_isr(hReg, (unsigned long) &pF2HDebugBlock->ui32ReadPtr);
    writePtr = BREG_Read32_isr(hReg, (unsigned long) &pF2HDebugBlock->ui32WritePtr);

    BDBG_NUL(("%s: baseaddr=0x%" PRIx32 " size=%" PRIu32 " ui32ReadPtr=%" PRId32 " ui32WritePtr=%" PRId32,
               BSTD_FUNCTION, bufferBaseDccmAddr, BASP_FW2H_PRINT_BUFFER_SIZE, readPtr, writePtr));

    {
        char        lineBuf[128];   /* Buffer to hold current message for printing. */
        unsigned    lineIdx = 0;    /* Current index into lineBuf. */
        char        thisChar;       /* Byte value at current read index. */

        BASP_P_GetDccmByte_isr(hAsp, 0);    /* Invalidate the cached word because it might be stale. */

        /* Just loop through the circular buffer until the read index catches up to the write index. */
        for (;;)
        {
            /* If the read index has caught up to the write index, take a fresh sample of the write index in case
             * it has advanced while we were printing  */
            if (readPtr == writePtr)
            {
                writePtr = BREG_Read32_isr(hReg, (unsigned long) &pF2HDebugBlock->ui32WritePtr);
                /* If read matches write, the buffer is empty. Exit the loop. */
                if (readPtr == writePtr) { break; }

                /* But if the write pointer advanced, invalidate the cached word. */
                BASP_P_GetDccmByte_isr(hAsp, 0);
            }

            /* Get the character at the current read index. */
            thisChar =  BASP_P_GetDccmByte_isr(hAsp, BCHP_ASP_ARCSS_DCCM_DATAi_ARRAY_BASE + (bufferBaseDccmAddr & 0x1ffff) + readPtr);
            BDBG_NUL(("ui32ReadPtr=%u, thisChar=0x%x (%c)", readPtr, thisChar, thisChar));

            /* Ignore any newline chars... BDBG_LOG prints each message on a new line. */
            if (thisChar != '\n')
            {
                lineBuf[lineIdx++] = thisChar;
            }

            /* Advance the read index, wrapping around if necessary. */
            readPtr++;
            if (readPtr >= BASP_FW2H_PRINT_BUFFER_SIZE)
            {
                readPtr = 0;
            }

            /* If this character is a NULL, we've collected an entire line, so log it. */
            if (thisChar == '\0')
            {
                lineBuf[lineIdx++] = '\0';
                BDBG_MSG(("<ASPFW>:\"%s\"", lineBuf));
                lineIdx = 0;

                /* Save the read index now to free up the space in the buffer. */
                BREG_Write32_isr(hReg, (unsigned long) &pF2HDebugBlock->ui32ReadPtr, readPtr);
            }
        }

        /* The buffer should be empty now, make sure the read index is saved. */
        BREG_Write32_isr(hReg, (unsigned long) &pF2HDebugBlock->ui32ReadPtr, readPtr);
    }

done:
    rc = BTMR_StartTimer_isr(hAsp->DBG_HostPrintf.hTimer, BASP_DBG_HOSTPRINTFCHECK_INTERVAL);
    if (rc != BERR_SUCCESS) {BERR_TRACE(rc);}

    BDBG_NUL(("%s: Exit.", BSTD_FUNCTION));
    return;
}

BERR_Code BASP_DBG_HostPrintfInit(
        BASP_Handle hAsp
        )
{
    BERR_Code rc = BERR_SUCCESS;
    BTMR_TimerHandle hTimer = NULL;

    BTMR_Settings stSettings = { BTMR_Type_eCountDown,
                                 NULL,
                                 NULL,
                                 0,
                                 false };

    stSettings.cb_isr = &BASP_DBG_HostPrintfCheck_isr;
    stSettings.pParm1 = hAsp;

    BREG_Write32(hAsp->handles.hReg, BCHP_ASP_ARCSS_CTRL_SYNC_LOCAL_RBUS, 0);

    rc = BTMR_CreateTimer( hAsp->handles.hTmr, &hTimer, &stSettings );
    if (rc != BERR_SUCCESS) {return BERR_TRACE(rc); }

    hAsp->DBG_HostPrintf.hTimer = hTimer;

    rc = BTMR_StartTimer(hAsp->DBG_HostPrintf.hTimer, BASP_DBG_HOSTPRINTFCHECK_INTERVAL);
    if (rc != BERR_SUCCESS) {return BERR_TRACE(rc); }

    return BERR_SUCCESS;
}


BERR_Code BASP_DBG_HostPrintfUninit(
        BASP_Handle hAsp
        )
{
    BERR_Code rc = BERR_SUCCESS;

    BTMR_StopTimer(hAsp->DBG_HostPrintf.hTimer);
    BERR_TRACE(rc);

    BTMR_DestroyTimer(hAsp->DBG_HostPrintf.hTimer);
    hAsp->DBG_HostPrintf.hTimer = NULL;

    return BERR_TRACE(rc);
}

#define ASP_SAGE_FIFO_SETUP 1
#ifdef ASP_SAGE_FIFO_SETUP
void
BASP_InitFwToSageFifo(
   BASP_Handle hAsp
   )
{
    /* Note: This is a temporary hack!!!! */
    /* Once DTCP SAGE ASP Module is coded & ready, it will be responsible for initializing */
    /* this message FIFOs for exchanging messages between SAGE & ASP. */
    /* The FIFO space (950bytes) is in the ASP SRAM and starts right after the Msg FIFO pointers. */
    /* We will split this space for the two FIFOs: 128 bytes for ASP -> SAGE, & 512 bytes for SAGE -> ASP. */
    /* SAGE -> ASP FIFO size is bigger as the message size is larger in this direction. */
#define ASP_FW_TO_SAGE_FIFO_RDB_BASE    0x18f9000
#define ASP_FW_TO_SAGE_FIFO_START       0xf98f9040
#define ASP_FW_TO_SAGE_FIFO_SIZE        256

#define SAGE_TO_ASP_FW_FIFO_RDB_BASE    0x18f9020
#define SAGE_TO_ASP_FW_FIFO_SIZE        512
#if 1
#define SAGE_TO_ASP_FW_FIFO_START       (ASP_FW_TO_SAGE_FIFO_START + ASP_FW_TO_SAGE_FIFO_SIZE ) /* Starts immediately after the ASP->SAGE FIFO. */
#else
#define SAGE_TO_ASP_FW_FIFO_START       (ASP_FW_TO_SAGE_FIFO_START)
#endif

    /* Setup ASP -> SAGE FIFOs: */
    /* ASP_ARCSS_SAGE_MSG_XL_FW_TO_SAGE_BASE_ADDR */
    uint64_t ui64Addr;
    ui64Addr = ASP_FW_TO_SAGE_FIFO_RDB_BASE + 0x00;
    BREG_Write64_isrsafe(hAsp->handles.hReg, ui64Addr, ASP_FW_TO_SAGE_FIFO_START);
    /* ASP_ARCSS_SAGE_MSG_XL_FW_TO_SAGE_END_ADDR */
    ui64Addr = ASP_FW_TO_SAGE_FIFO_RDB_BASE + 0x08;
    BREG_Write64_isrsafe(hAsp->handles.hReg, ui64Addr, ASP_FW_TO_SAGE_FIFO_START + ASP_FW_TO_SAGE_FIFO_SIZE - 4);
    /* ASP_ARCSS_SAGE_MSG_XL_FW_TO_SAGE_READ_ADDR */
    ui64Addr = ASP_FW_TO_SAGE_FIFO_RDB_BASE + 0x10;
    BREG_Write64_isrsafe(hAsp->handles.hReg, ui64Addr, ASP_FW_TO_SAGE_FIFO_START);
    /* ASP_ARCSS_SAGE_MSG_XL_FW_TO_SAGE_WRITE_ADDR */
    ui64Addr = ASP_FW_TO_SAGE_FIFO_RDB_BASE + 0x18;
    BREG_Write64_isrsafe(hAsp->handles.hReg, ui64Addr, ASP_FW_TO_SAGE_FIFO_START);

    /* Setup SAGE -> ASP FIFOs: */
    /* ASP_ARCSS_SAGE_MSG_XL_FW_TO_SAGE_BASE_ADDR */
    ui64Addr = SAGE_TO_ASP_FW_FIFO_RDB_BASE + 0x00;
    BREG_Write64_isrsafe(hAsp->handles.hReg, ui64Addr, SAGE_TO_ASP_FW_FIFO_START);
    /* ASP_ARCSS_SAGE_MSG_XL_FW_TO_SAGE_END_ADDR */
    ui64Addr = SAGE_TO_ASP_FW_FIFO_RDB_BASE + 0x08;
    BREG_Write64_isrsafe(hAsp->handles.hReg, ui64Addr, SAGE_TO_ASP_FW_FIFO_START + SAGE_TO_ASP_FW_FIFO_SIZE - 4);
    /* ASP_ARCSS_SAGE_MSG_XL_FW_TO_SAGE_READ_ADDR */
    ui64Addr = SAGE_TO_ASP_FW_FIFO_RDB_BASE + 0x10;
    BREG_Write64_isrsafe(hAsp->handles.hReg, ui64Addr, SAGE_TO_ASP_FW_FIFO_START);
    /* ASP_ARCSS_SAGE_MSG_XL_FW_TO_SAGE_WRITE_ADDR */
    ui64Addr = SAGE_TO_ASP_FW_FIFO_RDB_BASE + 0x18;
    BREG_Write64_isrsafe(hAsp->handles.hReg, ui64Addr, SAGE_TO_ASP_FW_FIFO_START);

    BDBG_LOG(("%s: temporary code to initialize Msg FIFOs between SAGE & ASP FW!!!!!!", BSTD_FUNCTION));
}

extern char *getenv(const char *name);
extern unsigned long int strtoul(const char *nptr, char **endptr, int base);
uint32_t myGetEnv(const char *pName)
{
    char *pTmp;
    uint32_t value = 0;

    pTmp = getenv(pName);
    if (pTmp)
    {
        value = strtoul(pTmp, NULL, 16);
    }
    else
    {
        BDBG_WRN(("!!!!!!!!!! %s is NOT set, needed to enable DTCP/IP in ASP. !!!!!!!!!!!", pName));
    }
    return value;
}

void BASP_SendDrmConstants(
   BASP_Handle hAsp
   )
{
    unsigned i;
    uint64_t ui64WritePtr;
    uint32_t ui32WriteAddr;
    BASP_Sage2Fw_ConstInfo msg;
    uint32_t *pMsg;
    BASP_SageConstInfo sageConstants;

    sageConstants.ui32ConstKeyCa0[0] = myGetEnv("Ca0_0");
    sageConstants.ui32ConstKeyCa0[1] = myGetEnv("Ca0_1");
    sageConstants.ui32ConstKeyCa0[2] = myGetEnv("Ca0_2");
    sageConstants.ui32ConstKeyCb1[0] = myGetEnv("Cb1_0");
    sageConstants.ui32ConstKeyCb1[1] = myGetEnv("Cb1_1");
    sageConstants.ui32ConstKeyCb1[2] = myGetEnv("Cb1_2");
    sageConstants.ui32ConstKeyCb0[0] = myGetEnv("Cb0_0");
    sageConstants.ui32ConstKeyCb0[1] = myGetEnv("Cb0_1");
    sageConstants.ui32ConstKeyCb0[2] = myGetEnv("Cb0_2");
    sageConstants.ui32ConstKeyCc1[0] = myGetEnv("Cc1_0");
    sageConstants.ui32ConstKeyCc1[1] = myGetEnv("Cc1_1");
    sageConstants.ui32ConstKeyCc1[2] = myGetEnv("Cc1_2");
    sageConstants.ui32ConstKeyCc0[0] = myGetEnv("Cc0_0");
    sageConstants.ui32ConstKeyCc0[1] = myGetEnv("Cc0_1");
    sageConstants.ui32ConstKeyCc0[2] = myGetEnv("Cc0_2");
    sageConstants.ui32ConstKeyCd0[0] = myGetEnv("Cd0_0");
    sageConstants.ui32ConstKeyCd0[1] = myGetEnv("Cd0_1");
    sageConstants.ui32ConstKeyCd0[2] = myGetEnv("Cd0_2");
    sageConstants.ui32IVc[0] = myGetEnv("Iv0");
    sageConstants.ui32IVc[1] = myGetEnv("Iv1");
    sageConstants.ui32StreamLabel = 0;

    /* ASP_ARCSS_SAGE_MSG_XL_FW_TO_SAGE_WRITE_ADDR */
    ui64WritePtr = SAGE_TO_ASP_FW_FIFO_RDB_BASE + 0x18;
#if 0
    ui32WriteAddr = BREG_Read64_isrsafe( hAsp->handles.hReg, ui64WritePtr );
#else
    ui32WriteAddr = 0x18f9140;
#endif

    BKNI_Memset((char *)&msg, 0, sizeof(msg));
    msg.MessageHeader.MessageType = BASP_MessageType_eSage2FwConstInfo;
    msg.MessageHeader.ui32MessageCounter = 1;
    msg.MessageHeader.ResponseType = BASP_ResponseType_eAckRespRequired;
    msg.MessageHeader.ui32ChannelIndex = 0;     /* channel index is ignored for init message.*/
    msg.MessagePayload.SageToFWConstInfo = sageConstants;
    pMsg = (uint32_t *)&msg;

    for (i=0; i<sizeof(msg)/4; i++)
    {
        BREG_Write32( hAsp->handles.hReg, ui32WriteAddr+4*i, *(pMsg+i) );
    }
    BDBG_LOG(("%s: copied DRM constants, msgSize=%u wordsCopied=%u", BSTD_FUNCTION, (unsigned) sizeof(msg), i));
    ui32WriteAddr = BREG_Read64_isrsafe( hAsp->handles.hReg, ui64WritePtr );
    BREG_Write64_isrsafe( hAsp->handles.hReg, ui64WritePtr, ui32WriteAddr+sizeof(msg) );

    /* Set an interrupt bit to indicate SAGE To ASP FW msg. */
    BREG_Write32( hAsp->handles.hReg, 0x18f8004, 1);
}

BERR_Code
BASP_CreateDrmInitMessage(
   BASP_Handle hAsp
   )
{
    BSTD_UNUSED(hAsp);
    /* Any time a message is created can use this global variable and memset it before populating the values.*/
    BKNI_Memset(&gsPi2FwMessage, 0, sizeof(BASP_Pi2Fw_Message));

    gsPi2FwMessage.MessageHeader.MessageType = BASP_MessageType_ePi2FwGetDrmConst;
    gsPi2FwMessage.MessageHeader.ResponseType = BASP_ResponseType_eAckRespRequired;
    gsPi2FwMessage.MessageHeader.ui32ChannelIndex = 0;     /* channel index is ignored for init message.*/

   return BERR_TRACE( BERR_SUCCESS );
}
#endif

/* Caller should call BASP_Close if this function returns error.*/
BERR_Code
BASP_CreateInitMessage(
   BASP_Handle hAsp
   )
{
   /* Any time a message is created can use this global variable and memset it before populating the values.*/
   BKNI_Memset(&gsPi2FwMessage, 0, sizeof(BASP_Pi2Fw_Message));

   /********************************************************************/
   /*********************** Populate Init Message **********************/
   /********************************************************************/

    gsPi2FwMessage.MessageHeader.MessageType = BASP_MessageType_ePi2FwInit;
    gsPi2FwMessage.MessageHeader.ResponseType = BASP_ResponseType_eAckRespRequired;
    gsPi2FwMessage.MessageHeader.ui32ChannelIndex = 0;     /* channel index is ignored for init message.*/

   gsPi2FwMessage.MessagePayload.Init.ui32NumMaxChannels = BASP_MAX_NUMBER_OF_CHANNEL; /* TODO: See if this can be derived from RDB.*/

   {
      uint64_t ui64RegisterAddress = BASP_CVT_TO_ASP_REG_ADDR( BCHP_PHYSICAL_OFFSET + BCHP_XPT_MEMDMA_MCPB_HOST_INTR_AGGREGATOR_INTR_W0_STATUS);

      gsPi2FwMessage.MessagePayload.Init.ui32MemDmaMcpbBaseAddressLo = (ui64RegisterAddress & 0xFFFFFFFF);
      gsPi2FwMessage.MessagePayload.Init.ui32MemDmaMcpbBaseAddressHi = ((ui64RegisterAddress >> 32) & 0xFFFFFFFF);
      /* Fw team confirmed that ui32MemDmaMcpbSize doesn't matter and they will remove it later.*/
   }

   {
      uint64_t ui64RegisterAddress = BASP_CVT_TO_ASP_REG_ADDR( BCHP_PHYSICAL_OFFSET + BCHP_XPT_MCPB_HOST_INTR_AGGREGATOR_INTR_W0_STATUS);

      gsPi2FwMessage.MessagePayload.Init.ui32XptMcpbBaseAddressLo = (ui64RegisterAddress & 0xFFFFFFFF);
      gsPi2FwMessage.MessagePayload.Init.ui32XptMcpbBaseAddressHi = ((ui64RegisterAddress >> 32) & 0xFFFFFFFF);
      /* Fw team confirmed that ui32XptMcpbSize doesn't matter and they will remove it later.*/
   }

   gsPi2FwMessage.MessagePayload.Init.ui32McpbStreamInDescType = BASP_DEFAULT_STREAM_IN_DESCRIPTOR_TYPE;

   /* Allocate memory for EDPKT Header buffer */
   {
     uint64_t ui64EdpktHeaderBufferDeviceOffset = BASP_P_Buffer_GetDeviceOffset_isrsafe( hAsp->aspSystemMemory.hEdpktHeaderBuffer ) ;

     gsPi2FwMessage.MessagePayload.Init.EdPktHeaderBuffer.ui32BaseAddrLo = (ui64EdpktHeaderBufferDeviceOffset & 0xFFFFFFFF);
     gsPi2FwMessage.MessagePayload.Init.EdPktHeaderBuffer.ui32BaseAddrHi = ((ui64EdpktHeaderBufferDeviceOffset >> 32) & 0xFFFFFFFF);
     gsPi2FwMessage.MessagePayload.Init.EdPktHeaderBuffer.ui32Size = BASP_EDPKT_HEADER_BUFFER_SIZE;
   }

   /* Allocate memory for Status buffer */
   {
     uint64_t ui64StatusBufferDeviceOffset = hAsp->sOpenSettings.ui64StatusBufferDeviceOffset;

     gsPi2FwMessage.MessagePayload.Init.StatusBuffer.ui32BaseAddrLo = (ui64StatusBufferDeviceOffset & 0xFFFFFFFF);
     gsPi2FwMessage.MessagePayload.Init.StatusBuffer.ui32BaseAddrHi = ((ui64StatusBufferDeviceOffset >> 32) & 0xFFFFFFFF);
     gsPi2FwMessage.MessagePayload.Init.StatusBuffer.ui32Size = hAsp->sOpenSettings.ui32StatusBufferSize;
   }

    /* Load the hardware init information, the put its DRAM address into the message.*/
    {
        uint64_t ui64DeviceOffset = BASP_P_Buffer_GetDeviceOffset_isrsafe(hAsp->aspCommMemory.hHwInitInfoBuffer);

        gsPi2FwMessage.MessagePayload.Init.HwInitInfo.ui32BaseAddrLo = (ui64DeviceOffset & 0xFFFFFFFF);
        gsPi2FwMessage.MessagePayload.Init.HwInitInfo.ui32BaseAddrHi = ((ui64DeviceOffset >> 32) & 0xFFFFFFFF);
        gsPi2FwMessage.MessagePayload.Init.HwInitInfo.ui32Size = BASP_P_Buffer_GetSize( hAsp->aspCommMemory.hHwInitInfoBuffer );
    }

   return BERR_TRACE( BERR_SUCCESS );
}

void
BASP_P_Watchdog_isr(
         void *pContext,
         int iParam
         )
{
   BASP_Handle hAsp = (BASP_Handle) pContext;
   BREG_Handle hReg;

   BSTD_UNUSED(iParam);
   BKNI_ASSERT_ISR_CONTEXT();
   BDBG_OBJECT_ASSERT(hAsp, BASP_P_Device);

   hReg = hAsp->handles.hReg;

   BDBG_ERR(("%s : %d ASP Watchdog Interrupt!!! iParam=%u count=%#x limit=%#x, ctl=%#x\n", BSTD_FUNCTION, __LINE__, iParam,
                   BREG_Read32_isr(hReg, BCHP_ASP_ARCSS_CTRL_WD_TIMER1_COUNTER),
                   BREG_Read32_isr(hReg, BCHP_ASP_ARCSS_CTRL_WD_TIMER1_PERIOD),
                   BREG_Read32_isr(hReg, BCHP_ASP_ARCSS_CTRL_WD_TIMER1_CONTROL) ));

   /* Clear all of the bits in the WD timer's control register to stop it from interrupting...
    * at least until firmware sets those bits again.    */
   BREG_Write32_isr(hReg, BCHP_ASP_ARCSS_CTRL_WD_TIMER1_CONTROL, 0);

   /* TODO: Need initiate a full reset of the ASP and ARC, the re-download the firmware and restart them.  */

   BDBG_ERR(("%s : %d Finished watchdog interrupt handking.", BSTD_FUNCTION, __LINE__ ));
}


static struct
{
    BASP_P_CallbackIndex     cbIdx;
    BINT_Id                  intId;
    BINT_CallbackFunc        cbFunc;
} intCfg[]=
{
    {BASP_P_CallbackIndex_eWatchdog,            BCHP_INT_ID_ASP_ARCSS_CTRL_WD_TIMER1_INTR  , BASP_P_Watchdog_isr                  },
    {BASP_P_CallbackIndex_eFw2hRespBit,         BASP_INT_ID_FW2H_RESP_BIT                  , BASP_P_Msgqueue_FW2H_Receive_isr     },
    {BASP_P_CallbackIndex_eFw2hFinnotifyBit,    BASP_INT_ID_FW2H_FINNOTIFY_BIT             , BASP_P_Msgqueue_FW2H_Receive_isr     },
    {BASP_P_CallbackIndex_eFw2hRstnotifyBit,    BASP_INT_ID_FW2H_RSTNOTIFY_BIT             , BASP_P_Msgqueue_FW2H_Receive_isr     },
    {BASP_P_CallbackIndex_eFw2hFincompBit,      BASP_INT_ID_FW2H_FINCOMP_BIT               , BASP_P_Msgqueue_FW2H_Receive_isr     },
    {BASP_P_CallbackIndex_eFw2hPaynotifyBit,    BASP_INT_ID_FW2H_PAYNOTIFY_BIT             , BASP_P_Msgqueue_FW2H_Receive_isr     },
    {BASP_P_CallbackIndex_eFw2hFrameavailBit,   BASP_INT_ID_FW2H_FRAMEAVAIL_BIT            , BASP_P_Msgqueue_FW2H_Receive_isr     },
    {BASP_P_CallbackIndex_eFw2hSgfeedcompBit,   BASP_INT_ID_FW2H_SGFEEDCOMP_BIT            , BASP_P_Msgqueue_FW2H_Receive_isr     },
    {BASP_P_CallbackIndex_eFw2hRtonotifyBit,    BASP_INT_ID_FW2H_RTONOTIFY_BIT             , BASP_P_Msgqueue_FW2H_Receive_isr     },
};


static
BERR_Code
BASP_P_SetupInterrupts(BASP_Handle hAsp)
{
    BERR_Code rc = BERR_SUCCESS;
    uint32_t mask;
    unsigned int i;

   BDBG_OBJECT_ASSERT(hAsp, BASP_P_Device);

   /* Install BINT ISR Handlers */
   /* FW2Host Msgqueue Interrupt  */

    /* Clear the mask bits in the interrupt aggregator so it will pass the interrupts from
     * the BCHP_ASP_ARCSS_HOST_FW2H_L2_HOST_STATUS L2 interrupt register and the
     * ASP_ARCSS_CTRL_WD_TIMER1_CONTROL L2 (non-standard) interrupt register to the
     * ASP_0_CPU_INTR bit of the BCHP_HIF_CPU_INTR1_INTR_W1_STATUS L1 interrupt register. */
    mask = BCHP_ASP_CORE_HOST_INTR_AGGREGATOR_INTR_W0_MASK_CLEAR_ARC2HOST0_INTR_MASK  |
           BCHP_ASP_CORE_HOST_INTR_AGGREGATOR_INTR_W0_MASK_CLEAR_ARC_WDOG_INTR_MASK;
    BREG_Write32( hAsp->handles.hReg, BCHP_ASP_CORE_HOST_INTR_AGGREGATOR_INTR_W0_MASK_CLEAR, mask );

    for (i=0 ; i<BASP_N_ELEMENTS(intCfg) ; i++ )
    {
        if (intCfg[i].cbFunc == NULL) continue;     /* Skip over any unused entries. */

        /* Create a callback for each of the interrupt bits, even though they all call the same function. */
        rc = BINT_CreateCallback(
                &hAsp->callbacks.ahCallbacks[intCfg[i].cbIdx],
                hAsp->handles.hInt,
                intCfg[i].intId,
                intCfg[i].cbFunc,
                hAsp,
                intCfg[i].cbIdx );
        if (rc != BERR_SUCCESS) { return BERR_TRACE(rc); }

        rc = BINT_EnableCallback( hAsp->callbacks.ahCallbacks[intCfg[i].cbIdx] );
        if (rc != BERR_SUCCESS ) { return BERR_TRACE(rc); }
    }

   return BERR_TRACE( BERR_SUCCESS );
}


static
BERR_Code
BASP_P_TeardownInterrupts(
         BASP_Handle hAsp
         )
{
    BERR_Code rc;
    unsigned int i;

    /* Uninstall BINT ISR Handlers */
    /* Mailbox Interrupt  */

    for (i=0 ; i<BASP_N_ELEMENTS(intCfg) ; i++ )
    {
        if (intCfg[i].cbFunc == NULL) continue;     /* Skip over any unused entries. */

        if (hAsp->callbacks.ahCallbacks[intCfg[i].cbIdx] == NULL) continue;

        rc = BINT_DestroyCallback( hAsp->callbacks.ahCallbacks[intCfg[i].cbIdx] );
        hAsp->callbacks.ahCallbacks[intCfg[i].cbIdx] = NULL;
        if (rc != BERR_SUCCESS ) { return BERR_TRACE(rc); }
    }

    return BERR_TRACE( BERR_SUCCESS );
}


void
BASP_GetDefaultOpenSettings(
   BASP_OpenSettings           *pOpenSettings /*!< out: default asp settings.*/
   )
{
   BDBG_ENTER( BASP_GetDefaultOpenSettings );

   BDBG_ASSERT( pOpenSettings );

   BKNI_Memset(pOpenSettings, 0, sizeof(BASP_OpenSettings));

   if ( NULL != pOpenSettings )
   {
      /* Initialize the image interface and image context.*/
      pOpenSettings->pImgInterface = &BASP_IMAGE_Interface;
      pOpenSettings->pImgContext = BASP_IMAGE_Context;
   }

   BDBG_LEAVE( BASP_GetDefaultOpenSettings );
}

BERR_Code
BASP_Open(
         BASP_Handle *phAsp, /* [out] ASP Device handle returned */
         BCHP_Handle hChp,   /* [in] Chip handle */
         BREG_Handle hReg,   /* [in] Register handle */
         BMMA_Heap_Handle hMem,   /* [in] System Memory handle */
         BINT_Handle hInt,   /* [in] Interrupt handle */
         BTMR_Handle hTmr,   /* [in] Timer handle */
         const BASP_OpenSettings *pOpenSettings /* [in] ASP Device Open settings */
         )
{
   BERR_Code rc;
   BASP_Handle hAsp = NULL;

   BDBG_ENTER( BASP_Open );

   BDBG_ASSERT( phAsp );
   BDBG_ASSERT( hChp );
   BDBG_ASSERT( hReg );
   BDBG_ASSERT( hMem );
   BDBG_ASSERT( hInt );
   BDBG_ASSERT( pOpenSettings );

   /* TODO: Later we need to check here signature before we proceed further.*/

   BDBG_MSG(("%s: Starting BASP_Open ================== Executed  ", BSTD_FUNCTION));
   *phAsp = NULL;

   hAsp = (BASP_Handle) BKNI_Malloc(sizeof(BASP_P_Device));
   if ( NULL == hAsp )  {rc = BERR_OUT_OF_SYSTEM_MEMORY; BERR_TRACE(rc); goto error;}

   BKNI_Memset(hAsp, 0, sizeof(BASP_P_Device));

   BDBG_OBJECT_SET( hAsp, BASP_P_Device);

   hAsp->sOpenSettings = *pOpenSettings;

   /* Setup Device Handles */
   hAsp->handles.hChp = hChp;
   hAsp->handles.hReg = hReg;
   hAsp->handles.hInt = hInt;
   hAsp->handles.hTmr = hTmr;
   hAsp->handles.hMem = hMem;

   BLST_S_INIT(&hAsp->contextList);
   BLST_S_INIT(&hAsp->msgqueueList);

   /* Setup Allocators */
   BDBG_MSG(("%s: Calling BASP_P_SetupAllocators", BSTD_FUNCTION));
   rc = BASP_P_SetupAllocators( hAsp );
   if ( BERR_SUCCESS != rc ) {BERR_TRACE(rc) ; goto error;}

   {
       BCHP_PWR_AcquireResource(hChp, BCHP_PWR_RESOURCE_ASP);
       BCHP_PWR_AcquireResource(hChp, BCHP_PWR_RESOURCE_ASP_SRAM);
   }

   /* Reset asp */
   BDBG_MSG(("%s: Calling BASP_P_Reset", BSTD_FUNCTION));
   rc = BASP_P_Reset(hAsp);
   if ( BERR_SUCCESS != rc ) {BERR_TRACE(rc) ; goto error;}

   /* Create the ASP message queues. */
   BDBG_MSG(("%s: Creating message queues", BSTD_FUNCTION));
   rc = BASP_Msgqueue_Create( hAsp, BASP_MsgqueueType_eFwToHost, 1024, &hAsp->hMsgqueueFwToHost);
   if ( BERR_SUCCESS != rc ) {BERR_TRACE(rc) ; goto error;}

   rc = BASP_Msgqueue_Create( hAsp, BASP_MsgqueueType_eHostToFw, 1024, &hAsp->hMsgqueueHostToFw);
   if ( BERR_SUCCESS != rc ) {BERR_TRACE(rc) ; goto error;}

   rc = BASP_Msgqueue_Create( hAsp, BASP_MsgqueueType_eRaToFw,   1024, &hAsp->hMsgqueueRaToFw);
   if ( BERR_SUCCESS != rc ) {BERR_TRACE(rc) ; goto error;}

   rc = BASP_Msgqueue_Create( hAsp, BASP_MsgqueueType_eFwToRa,   1024, &hAsp->hMsgqueueFwToRa);
   if ( BERR_SUCCESS != rc ) {BERR_TRACE(rc) ; goto error;}

    rc =BASP_DBG_HostPrintfInit(hAsp);
    if ( BERR_SUCCESS != rc ) {BERR_TRACE(rc) ; goto error;}

    /* Temporary workaround until FW initializes all the MCPB registers via hw init info. */
    while (true)
    {
        unsigned value;

        BREG_Write32( hAsp->handles.hReg,0x1870008 /* BCHP_ASP_CORE_CTRL_ASP_SW_INIT_DO_MEM_INIT*/, 1 );
        BREG_Write32( hAsp->handles.hReg, 0x1870000 /*BCHP_ASP_CORE_CTRL_ASP_SW_INIT*/, 0x4 );
        value = BREG_Read32( hAsp->handles.hReg, 0x1870004 /*BCHP_ASP_CORE_CTRL_ASP_SW_INIT_STATUS*/);
        if (value != 0x4)
        {
            BDBG_WRN(("MCPB SoftInit is not yet complete, wait & retry!"));
            BKNI_Sleep(100);
            continue;
        }
        else
        {
            BDBG_WRN(("MCPB SoftInit is complete!"));
            BREG_Write32( hAsp->handles.hReg, 0x1870000 /* BCHP_ASP_CORE_CTRL_ASP_SW_INIT*/, 0 );
            break;
        }
    }

    /* Load the ASP firmware */
    rc = BASP_P_LoadFirmware( hAsp );
    if ( BERR_SUCCESS != rc ) {BERR_TRACE(rc) ; goto error;}

    rc = BASP_P_SetupInterrupts(hAsp);
    if ( BERR_SUCCESS != rc ) {BERR_TRACE(rc) ; goto error;}

    rc = BASP_P_AllocateDeviceMemory(hAsp->aspSystemMemory.hAllocator,           /* Use this allocator */
                                     &hAsp->aspSystemMemory.hEdpktHeaderBuffer,  /* Put allocated buffer handle here */
                                     BASP_EDPKT_HEADER_BUFFER_SIZE,              /* Allocate this many bytes */
                                     BASP_EDPKT_HEADER_BUFFER_ALLIGNMENT);       /* Aligned like this */
    if ( BERR_SUCCESS != rc ) {BERR_TRACE(rc) ; goto error;}

    rc = BASP_P_LoadHwInitInfo(hAsp);
    if ( BERR_SUCCESS != rc ) {BERR_TRACE(rc) ; goto error;}

    /* Create and send the Init message. */
    BASP_CreateInitMessage( hAsp );
    rc = BASP_Msgqueue_Write(hAsp->hMsgqueueHostToFw, &gsPi2FwMessage);
    if ( BERR_SUCCESS != rc ) {BERR_TRACE(rc) ; goto error;}

#ifdef ASP_SAGE_FIFO_SETUP
    /* This is temporary code until SAGE DTCP/IP module is brought up. */
    BKNI_Sleep(100); /* wait for FW to process the Init msg. */
    BASP_InitFwToSageFifo( hAsp );
    BASP_SendDrmConstants( hAsp );
    BASP_CreateDrmInitMessage( hAsp );
    rc = BASP_Msgqueue_Write(hAsp->hMsgqueueHostToFw, &gsPi2FwMessage);
    if ( BERR_SUCCESS != rc ) {BERR_TRACE(rc) ; goto error;}
    BKNI_Sleep(100);
#endif

    *phAsp = hAsp;

    BDBG_LEAVE( BASP_Open );
    return BERR_TRACE( BERR_SUCCESS );

error:
    if (hAsp)
    {
        BASP_Close( hAsp );
    }
    return (rc);
}

BERR_Code
BASP_Close(
   BASP_Handle hAsp
   )
{
    BERR_Code rc = BERR_SUCCESS;

    BDBG_ENTER( BASP_Close );

   BDBG_OBJECT_ASSERT(hAsp, BASP_P_Device);


   /* Build and send an UnInit message to the ASP firmware. */
   {
       static  BASP_Pi2Fw_Message pi2FwUnInitMessage;

       pi2FwUnInitMessage.MessageHeader.MessageType = BASP_MessageType_ePi2FwUnInit;
       pi2FwUnInitMessage.MessageHeader.ResponseType = BASP_ResponseType_eAckRequired;
       pi2FwUnInitMessage.MessageHeader. ui32ChannelIndex = 0;
       pi2FwUnInitMessage.MessagePayload.UnInit.ui32Unused = 0;
       BASP_Msgqueue_Write(hAsp->hMsgqueueHostToFw, &pi2FwUnInitMessage);
   }

    /* TODO: We need to reset/stop the ASP before freeing the DRAM that it is using. */

   rc = BASP_P_TeardownInterrupts(hAsp);
   BERR_TRACE(rc);

   rc = BASP_P_FreeHwInitInfo(hAsp);        /* TODO: We can probably free this after we get the response for the Init message. */
   BERR_TRACE(rc);

   /* Free some DRAM that that was allocated for the ASP. */
   if (hAsp->aspSystemMemory.hEdpktHeaderBuffer)
   {
       BASP_P_FreeDeviceMemory(hAsp->aspSystemMemory.hEdpktHeaderBuffer);
       hAsp->aspSystemMemory.hEdpktHeaderBuffer = NULL;
   }

   rc = BASP_DBG_HostPrintfUninit( hAsp);
   BERR_TRACE(rc);

   if (hAsp->hMsgqueueFwToRa)
   {
       BASP_Msgqueue_Destroy(hAsp->hMsgqueueFwToRa);
       hAsp->hMsgqueueFwToRa = NULL;
   }

   if (hAsp->hMsgqueueRaToFw)
   {
       BASP_Msgqueue_Destroy(hAsp->hMsgqueueRaToFw);
       hAsp->hMsgqueueRaToFw = NULL;
   }

   if (hAsp->hMsgqueueHostToFw)
   {
       BASP_Msgqueue_Destroy(hAsp->hMsgqueueHostToFw);
       hAsp->hMsgqueueHostToFw = NULL;
   }

   if (hAsp->hMsgqueueFwToHost)
   {
       BASP_Msgqueue_Destroy(hAsp->hMsgqueueFwToHost);
       hAsp->hMsgqueueFwToHost = NULL;
   }

    BASP_P_DestroyAllocators(hAsp);

    {
        BCHP_PWR_ReleaseResource(hAsp->handles.hChp, BCHP_PWR_RESOURCE_ASP);
        BCHP_PWR_ReleaseResource(hAsp->handles.hChp, BCHP_PWR_RESOURCE_ASP_SRAM);
    }

    BDBG_OBJECT_DESTROY(hAsp, BASP_P_Device);

    BKNI_Free(hAsp);

   BDBG_LEAVE( BASP_Close );
   return BERR_TRACE( BERR_SUCCESS );
}
