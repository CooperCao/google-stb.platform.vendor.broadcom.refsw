/******************************************************************************
 *    (c)2007-2015 Broadcom Corporation
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
 *****************************************************************************/


#include "bchp_bsp_control_intr2.h"
#include "bchp_bsp_glb_control.h"

#include "bhsm.h"
#include "bhsm_bsp_interface_legacy.h"


/* special support for  HSM ISR  vs. HSM Polling  */
#if BHSM_DEBUG_POLLING
#include "bchp_timer.h"
#endif


BDBG_MODULE(BHSM);


#define BHSM_CTRL_SET_ISR       (0)
#define BHSM_CTRL_SET_POLLING   (1)

/* To support new BSP command header format */
/* starting with BCM97405B0                 */
#define CMODE_PADDING_MSK     0x000000FF
#define CMODE_PADDING         0xABCDEF00
#define CTAGID_PADDING_MSK    0x000000FF
#define CTAGID_PADDING        0x0055AA00
#define CTAGID_SHFT                   24
#define CPLEN_PADDING_MSK     0x0000FFFF
#define CPLEN_PADDING         0x789A0000

#define BSP_S_DRV_VERSION           0x10


#define BHSM_DEBUG_BSP_INTERRUPT 0


void BHSM_P_IntHandler_isr (
    void *pParam1,    /* Device  handle */
    int pParam2       /* Unused */
)
{
    BHSM_Handle  hHsm;
    uint32_t interruptStatus = 0;
    uint32_t controlStatus = 0;
    uint32_t oLoadRegister = 0;

    BSTD_UNUSED( pParam2 );

    if( pParam1 == NULL )
    {
        BERR_TRACE( BHSM_STATUS_INPUT_PARM_ERR );
        return;
    }
    hHsm = (BHSM_Handle)pParam1;

    #if BHSM_SAGE_BSP_PI_SUPPORT /* if  we're on SAGE */
     #define LOCAL_BCHP_BSP_GLB_CONTROL_GLB_HOST_INTR_STATUS_OLOADx_INT_MASK BCHP_BSP_GLB_CONTROL_GLB_HOST_INTR_STATUS_OLOAD1_INT_MASK
     #define LOCAL_BCHP_BSP_GLB_CONTROL_GLB_OLOADx                           BCHP_BSP_GLB_CONTROL_GLB_OLOAD1
     #define LOCAL_BCHP_BSP_GLB_CONTROL_GLB_OLOADx_CMD_OLOADx_MASK           BCHP_BSP_GLB_CONTROL_GLB_OLOAD1_CMD_OLOAD1_MASK
    #else
     #define LOCAL_BCHP_BSP_GLB_CONTROL_GLB_HOST_INTR_STATUS_OLOADx_INT_MASK BCHP_BSP_GLB_CONTROL_GLB_HOST_INTR_STATUS_OLOAD2_INT_MASK
     #define LOCAL_BCHP_BSP_GLB_CONTROL_GLB_OLOADx                           BCHP_BSP_GLB_CONTROL_GLB_OLOAD2
     #define LOCAL_BCHP_BSP_GLB_CONTROL_GLB_OLOADx_CMD_OLOADx_MASK           BCHP_BSP_GLB_CONTROL_GLB_OLOAD2_CMD_OLOAD2_MASK
    #endif

    interruptStatus = BREG_Read32( hHsm->regHandle, BCHP_BSP_GLB_CONTROL_GLB_HOST_INTR_STATUS );

    if( (interruptStatus & LOCAL_BCHP_BSP_GLB_CONTROL_GLB_HOST_INTR_STATUS_OLOADx_INT_MASK) )
    {
        oLoadRegister = BREG_Read32( hHsm->regHandle, LOCAL_BCHP_BSP_GLB_CONTROL_GLB_OLOADx );

        /* Clear OLOADx bit */
        BREG_Write32( hHsm->regHandle,
                      LOCAL_BCHP_BSP_GLB_CONTROL_GLB_OLOADx,
                      ( oLoadRegister & (~LOCAL_BCHP_BSP_GLB_CONTROL_GLB_OLOADx_CMD_OLOADx_MASK) ) );

        /* Clear OLOADx Interrupt. */
        BREG_Write32( hHsm->regHandle,
                      BCHP_BSP_GLB_CONTROL_GLB_HOST_INTR_STATUS,
                      (~LOCAL_BCHP_BSP_GLB_CONTROL_GLB_HOST_INTR_STATUS_OLOADx_INT_MASK) );

        if( hHsm->channelHandles[BSP_CmdInterface]->bIsOpen == true )
        {
            hHsm->channelHandles[BSP_CmdInterface]->oLoadSet = 1;
            BKNI_SetEvent( hHsm->channelHandles[BSP_CmdInterface]->oLoadWait );
        }
    }

    controlStatus = BREG_Read32( hHsm->regHandle, BCHP_BSP_CONTROL_INTR2_CPU_STATUS );

    #if BHSM_DEBUG_BSP_INTERRUPT
    {
        uint32_t oLoadRegisterCheck;
        uint32_t cpuIntrMask;

        oLoadRegisterCheck = BREG_Read32( hHsm->regHandle, LOCAL_BCHP_BSP_GLB_CONTROL_GLB_OLOADx );
        cpuIntrMask = BREG_Read32( hHsm->regHandle, BCHP_BSP_GLB_CONTROL_GLB_HOST_INTR_EN );

        BDBG_LOG(("BSP interupt STATUS[%08x] oLoad[%08x][%08x] cpuInt[%08x] control[%08x]", interruptStatus, oLoadRegister, oLoadRegisterCheck, cpuIntrMask, controlStatus ));
    }
    #endif

    if( controlStatus & BCHP_BSP_CONTROL_INTR2_CPU_STATUS_BSP_TO_HOST_INTR_MASK )
    {
        if( hHsm->bIsOpen == true )
        {
            BDBG_WRN(( "BSP Exception [%X]", controlStatus ));
        }
    }

    return;
}

#if BHSM_SAGE_BSP_PI_SUPPORT

BERR_Code BHSM_P_SubmitCommand (
        BHSM_ChannelHandle        in_channelHandle,
        uint32_t    *inCmdBuf,
        uint32_t    *outCmdBuf
)
{
    BERR_Code errCode = BERR_SUCCESS;
    uint32_t    unVersionNum, unOwnerId, unContMode, unTagId, i, unIReady, j=0;
    uint32_t    inVal;
#if BHSM_DEBUG_POLLING
     uint32_t  startValue, endValue=0;
#endif
    uint32_t    cmdLength;

    BDBG_ENTER(BHSM_P_SubmitCommand);
    BDBG_ASSERT( in_channelHandle );
    BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_FAILED,
        (in_channelHandle->ulMagicNumber != BHSM_P_CHANNEL_HANDLE_MAGIC_NUMBER ) );

    /* Check IReady */
    unIReady = BREG_Read32( in_channelHandle->moduleHandle->regHandle,
                            in_channelHandle->ulIReadyRegAddr);

    /* Check IReady , ulTimeOutInMilSecs_max = 120 000, multiplied by 500 is till less than FFFFFFFF, no overflow yet*/
    for (i=0; i< in_channelHandle->moduleHandle->currentSettings.ulTimeOutInMilSecs * 500 ;i++)
    {
        unIReady = BREG_Read32( in_channelHandle->moduleHandle->regHandle, in_channelHandle->ulIReadyRegAddr);
        if (unIReady & in_channelHandle->ulIReadyVal)
        {
            break;
        }
        BKNI_Delay(2);   /*  2us per loop, still blocking so far. May give CPU chance to others if it's non-blocking on other platforms*/
    }
    if ( i ==  in_channelHandle->moduleHandle->currentSettings.ulTimeOutInMilSecs * 500)
    {
         BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_IRDY_ERR, 1 );
    }


    /* Clear output command buffer */
    for(i = 0; i<BCMD_BUFFER_BYTE_SIZE/4; i++) {
        BREG_Write32( in_channelHandle->moduleHandle->regHandle,
            in_channelHandle->ulOutCmdBufAddr+ i*4, 0);
    }


    /* Check Version Number */
    unVersionNum = *(inCmdBuf + BCMD_CommonBufferFields_eVerNum/4) & 0xFF;
    BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_VERSION_ERR,
        (  unVersionNum != (BSP_S_DRV_VERSION) ) );

    /* Fill in specific input command parameters ( entire cmd including header section words in RawCmd), loop in words */
    cmdLength = BCMD_HEADER_BYTE_SIZE + *(inCmdBuf+ BCMD_CommonBufferFields_eParamLenPlus1/4) ;

    /* Check Parameter length */
    BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_PARM_LEN_ERR,
        ( cmdLength  > BCMD_BUFFER_BYTE_SIZE ) );


    for(i = 0; i< BCMD_BUFFER_BYTE_SIZE/4 ; i++) {  /* this length including header or not? try 'yes' */
        switch (i)
        {
#if 0
            case 2 : /* Continual Mode */
                inVal = ((inoutp_commandData->unInputParamsBuf[i]) & CMODE_PADDING_MSK) | CMODE_PADDING;
                break;
            case 3 : /* Command Tag ID */
                inVal = ((inoutp_commandData->unInputParamsBuf[i]) & CTAGID_PADDING_MSK) | ((~(inoutp_commandData->unInputParamsBuf[i])) << CTAGID_SHFT) |CTAGID_PADDING;
                break;
            case 4 : /* Command Parameter Length */
                inVal = ((inoutp_commandData->unInputParamsBuf[i]) & CPLEN_PADDING_MSK) | CPLEN_PADDING;
                break;
#endif
            case (BCMD_CommonBufferFields_eOwnerId/4) :
                inVal = in_channelHandle->ulSequenceNum;
                break;

            default:
                if ( i < cmdLength )
                    inVal = *(inCmdBuf + i);
                else
                    inVal = 0 ;

                break;
        }

        BREG_Write32( in_channelHandle->moduleHandle->regHandle,in_channelHandle->ulInCmdBufAddr+ i*4,inVal);
    }

    /* Dump out the input commands */
    {
        unsigned int i;
        BDBG_MSG(("MIPS -----------------------------------> BSP:"));
        for(i = 0; i< (cmdLength + 3)/4 ; i++) {
            /* Data */
            BDBG_MSG(("\t\t\t%.3d - %08X ", i*4, BREG_Read32( in_channelHandle->moduleHandle->regHandle, in_channelHandle->ulInCmdBufAddr+ i*4)));
        }
    }
if ((in_channelHandle->moduleHandle->currentSettings.uSpecialControl & BHSM_CTRLS_POLLINGORISR) ==  BHSM_CTRL_SET_POLLING)
{
    BDBG_MSG(("////////// HSM in Polling,  ISR2 is disabled, %x, channel num=%d",
                    in_channelHandle->moduleHandle->currentSettings.uSpecialControl,
                    in_channelHandle->ucChannelNumber));
}else
{

        BKNI_ResetEvent(in_channelHandle->oLoadWait);        /* CTRL-C or any system level signal*/
    BDBG_MSG(("//////////HSM in ISR,  ISR2 is enabled, %x, channel num=%d",
                    in_channelHandle->moduleHandle->currentSettings.uSpecialControl,
                    in_channelHandle->ucChannelNumber));
}


    /* Set ILOAD to signal BSP there's a command ready. */
    BDBG_MSG(("BHSM_P_CommonSubmitRawCommand: ulILoadRegAddr = 0x%08lx, ulILoadVal = %ld",
        in_channelHandle->ulILoadRegAddr, in_channelHandle->ulILoadVal));
    BREG_Write32( in_channelHandle->moduleHandle->regHandle,
            in_channelHandle->ulILoadRegAddr,
            in_channelHandle->ulILoadVal);



/*--------------------start of experiement  12/19/06: ly----------------------------------------------
    as Bryant request for a test for Jeff Wong
       replace the ISR based processing with polling, to see if there is some performance improvement
 */


if ( (in_channelHandle->moduleHandle->currentSettings.uSpecialControl & BHSM_CTRLS_POLLINGORISR) == BHSM_CTRL_SET_POLLING)
{

#if BHSM_DEBUG_POLLING
     BREG_Write32(in_channelHandle->moduleHandle->regHandle, BCHP_TIMER_TIMER1_CTRL , 0x9FFFFFFF); /*start */
     startValue = BREG_Read32( in_channelHandle->moduleHandle->regHandle, BCHP_TIMER_TIMER1_STAT ) & 0x3FFFFFFF;
#endif

        /*for(;;),   about 20us per attemp, 2x500 = 1000 = 1ms */
    for (i=0; i< in_channelHandle->moduleHandle->currentSettings.ulTimeOutInMilSecs * 500 ;i++)
    {
        j = BREG_Read32(  in_channelHandle->moduleHandle->regHandle,     BCHP_BSP_GLB_CONTROL_GLB_OLOAD2);

#if BHSM_DEBUG_POLLING     /* as close as to the OLOAD2 reading moment*/
        endValue = BREG_Read32( in_channelHandle->moduleHandle->regHandle, BCHP_TIMER_TIMER1_STAT ) & 0x3FFFFFFF;
#endif
        BDBG_MSG(( "////BREG_Read32(BCHP_BSP_GLB_CONTROL_GLB_OLOAD2) = %x ", j));
        if (    ( j & BCHP_BSP_GLB_CONTROL_GLB_OLOAD2_CMD_OLOAD2_MASK) == 1  )
        {
            break;
        }
        BKNI_Delay(2);   /* to 2 from 19 plus ~1*/


    }
#if BHSM_DEBUG_POLLING   /* buffer it to improve timing performance*/
    /* convert to units of us, slighter faster than the actual timing due to a timer1 late start*/
    in_channelHandle->pollingIntervalBuf[in_channelHandle->pollingBufIndex] = (uint16_t)((endValue - startValue )/27);
    in_channelHandle->pollingBufIndex = (in_channelHandle->pollingBufIndex+1)%BHSM_DEBUG_POLLINGBUF_MAX;
    BREG_Write32(in_channelHandle->moduleHandle->regHandle, BCHP_TIMER_TIMER1_CTRL , 0x1FFFFFFF); /*stop */

#endif

        if (j==0)
        {
        BDBG_ERR(("BHSM_P_CommonSubmitRawCommand: Polling timeout error %d", in_channelHandle->moduleHandle->currentSettings.ulTimeOutInMilSecs));
        errCode = BHSM_STATUS_TIME_OUT;
        goto BHSM_P_DONE_LABEL;
        }
      /* crash the system, no direct operation. *(volatile uint32_t *) 0x1030b020 =0;  */ /* clear the Output ready flag*/
       BREG_Write32(  in_channelHandle->moduleHandle->regHandle,
                        BCHP_BSP_GLB_CONTROL_GLB_OLOAD2,
                        BREG_Read32(  in_channelHandle->moduleHandle->regHandle,
                                        BCHP_BSP_GLB_CONTROL_GLB_OLOAD2) &
                                        (~BCHP_BSP_GLB_CONTROL_GLB_OLOAD2_CMD_OLOAD2_MASK));


}
else
{

    if ( (errCode = BERR_TRACE(BKNI_WaitForEvent( in_channelHandle->oLoadWait,
                in_channelHandle->moduleHandle->currentSettings.ulTimeOutInMilSecs))) != BERR_SUCCESS ) {

        /* CTRL-C issue: according to Min requirement, App will loop to free the key slot, one slot each call ,
            App wants to continue anyway, assume CTRL-C caused an extra Event.
            Side effect: the timeout duration is doubled at worst case. Timeout normally won't happen. 2/26/07
        */

        if (errCode == BERR_OS_ERROR)
        {
            /*  1) use BKNI_Delay(), polling for a non-OS return, with a max of tolerated time of ulTimeOutInMilSecs
                    blocking with 1us (seems ok), 1us-pe-waiting which will be asynced
                   2) use BKNI_Sleep(),returns OS_ERROR also; then we have to mark the OS event in ISR handler
            */
            for (i=0; i<in_channelHandle->moduleHandle->currentSettings.ulTimeOutInMilSecs*1000; i++)
            {
                BKNI_Delay(1);

                if (in_channelHandle->oLoadSet)  /*   ISR action must be checked before waitforevent */
                {
                    errCode = BERR_SUCCESS;         /*  assume it's good if ISR has set OLoad*/
                    break;
                }
#if 0
                errCode = BERR_TRACE(BKNI_WaitForEvent( in_channelHandle->oLoadWait,
                                        in_channelHandle->moduleHandle->currentSettings.ulTimeOutInMilSecs));
                if (errCode != BERR_OS_ERROR)
                {
                    break;    /*  stop the loop to continue the cmd parsing, for TIMEOUT or SUCCESS */
                }
#endif
            }
        }

        if(i== in_channelHandle->moduleHandle->currentSettings.ulTimeOutInMilSecs*1000)
        {
            errCode = BHSM_STATUS_TIME_OUT;   /* CTRL-C, OS_ERROR stays all the time in current app cycle */
        }

        /* by here, should not be an OS event*/
        if (errCode != BERR_SUCCESS)
        {
            BDBG_ERR(("BHSM_P_CommonSubmitRawCommand:  Error Code= %x returned from BKNI_WaitForEvent", errCode));
            /*errCode = BHSM_STATUS_TIME_OUT;     no enforement */
            goto BHSM_P_DONE_LABEL;
        }
    }
 }
    /*BKNI_Delay(100); */ /* 1) if no such delay, the following first Buffer2_Read is failed with a 'zero' reading ;
                                       2) 50us is not long enough to solve this issue
                                       3) 500us or 600us more is good to get the right data from Buffer2*/
    /* Read output buffer */
    /* Version Number */
    unVersionNum = BREG_Read32(  in_channelHandle->moduleHandle->regHandle,
            in_channelHandle->ulOutCmdBufAddr + (BCMD_CommonBufferFields_eVerNum / 4) * 4) & 0xFF;

    /* Owner ID */
    unOwnerId = BREG_Read32(  in_channelHandle->moduleHandle->regHandle,
            in_channelHandle->ulOutCmdBufAddr +  (BCMD_CommonBufferFields_eOwnerId / 4) * 4) & 0xFF;

    /* Continuous Mode */
    unContMode = BREG_Read32(  in_channelHandle->moduleHandle->regHandle,
            in_channelHandle->ulOutCmdBufAddr +  (BCMD_CommonBufferFields_eContMode / 4) * 4) & 0xFF;

    /* Tag ID */
    unTagId = BREG_Read32(  in_channelHandle->moduleHandle->regHandle,
            in_channelHandle->ulOutCmdBufAddr +  (BCMD_CommonBufferFields_eTagId / 4) * 4);
    unTagId &= CTAGID_PADDING_MSK;

    /* Check Tag ID, not for no-os */
    /*    BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_PARM_LEN_ERR,
        (unTagId !=  inoutp_commandData->cmdId ) );
    */
    /* Parameter Length */
    cmdLength = BCMD_HEADER_BYTE_SIZE + (BREG_Read32(  in_channelHandle->moduleHandle->regHandle,
            in_channelHandle->ulOutCmdBufAddr +   (BCMD_CommonBufferFields_eParamLenPlus1 / 4) * 4) & 0xFFFF);

    /* Check Parameter length */
    BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_PARM_LEN_ERR,
        (cmdLength > BCMD_BUFFER_BYTE_SIZE ) );

    /* Check if owner id still the same */
    /*    BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_OWNER_ID_ERR,
        (unOwnerId != in_channelHandle->ulSequenceNum ) );
    */

    /* Fill in specific output command parameters , +3 to resolve DH return in bits, e.g. 2018 bits 252bytes+2 bits*/
    for(i = 0; i< (cmdLength + 3)/4; i++) {
        *(outCmdBuf + i) =     BREG_Read32( in_channelHandle->moduleHandle->regHandle, in_channelHandle->ulOutCmdBufAddr+ i*4);
        /*  strong debug to see what is actually copied from ISR buffer into HSM command buffer
        BDBG_MSG( ("i =%d, inoutp_commandData->unOutputParamsBuf[i]=%.8x",i, inoutp_commandData->unOutputParamsBuf[i]));
        */
    }

    /* Dump out the output commands, +3 to dump the rest bits in the last word32 */
    {
        unsigned int i;
        BDBG_MSG(("MIPS <----------------------------------- BSP:"));
        for(i = 0; i < (cmdLength + 3)/4; i++ ) {
            /* Data */
            BDBG_MSG(("\t\t\t%.3d - %08X",  i*4,     *(outCmdBuf + i)));
            /* BDBG_MSG(("\t\t\t%.3d - %08X",  i*4,
                        BREG_Read32( in_channelHandle->moduleHandle->regHandle, in_channelHandle->ulOutCmdBufAddr+ i*4)));  */
        }
    }

    BDBG_MSG(("BHSM_P_CommonSubmitRawCommand output: unVersionNum = %08X, unOwnerId = %08X, unContMode = %x, unTagId = %08X, outp_unOutputParamLen = %08X",
        unVersionNum, unOwnerId, unContMode, unTagId, cmdLength));

    /* Increment the sequence number by 1 */
    if ((++in_channelHandle->ulSequenceNum) >= 256)
        in_channelHandle->ulSequenceNum = 0;

    /* Clear Input command buffer */
    for(i = 0; i<BCMD_BUFFER_BYTE_SIZE/4; i++) {
        BREG_Write32( in_channelHandle->moduleHandle->regHandle,
            in_channelHandle->ulInCmdBufAddr+ i*4, 0);
    }

BHSM_P_DONE_LABEL:

    in_channelHandle->oLoadSet=0;        /* avoid the leftover from a good HSM comand into a CTRL-C interrupted HSM cmd*/

    BDBG_LEAVE(BHSM_P_SubmitCommand);
    BDBG_MSG(("Exit BHSM_P_SubmitCommand"));
    return( errCode );
}


#endif


BERR_Code BHSM_P_CommonSubmitCommand (
        BHSM_ChannelHandle        in_channelHandle,
        BHSM_P_CommandData_t    *inoutp_commandData
)
{
    BERR_Code errCode = BERR_SUCCESS;
    uint32_t    unVersionNum, unOwnerId, unContMode, unTagId, i, unIReady, j=0;
    uint32_t  inVal;

#if BHSM_DEBUG_POLLING
     uint32_t  startValue, endValue=0;
#endif
    BDBG_ENTER(BHSM_P_CommonSubmitCommand);

    BDBG_ASSERT( in_channelHandle );

    BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_FAILED,
        (in_channelHandle->ulMagicNumber != BHSM_P_CHANNEL_HANDLE_MAGIC_NUMBER ) );

    /* Check IReady , ulTimeOutInMilSecs_max = 120 000, multiplied by 500 is till less than FFFFFFFF, no overflow yet*/
    for (i=0; i< in_channelHandle->moduleHandle->currentSettings.ulTimeOutInMilSecs * 500 ;i++)
    {
        unIReady = BREG_Read32( in_channelHandle->moduleHandle->regHandle,
                            in_channelHandle->ulIReadyRegAddr);
        if (unIReady & in_channelHandle->ulIReadyVal) {
            /*
            errCode = BERR_TRACE(errCode);
            goto BHSM_P_DONE_LABEL;
            */
            break;
        }
        BKNI_Delay(2);   /*  2us per loop, still blocking so far. May give CPU chance to others if it's non-blocking on other platforms*/
    }
    if ( i ==  in_channelHandle->moduleHandle->currentSettings.ulTimeOutInMilSecs * 500)
    {
         BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_IRDY_ERR, 1 );
    }

    /* There is no need to clear command response buffer, since BSP FW maintains its own */
#if 0
    /* Clear output command buffer */
    for(i = 0; i<BCMD_BUFFER_BYTE_SIZE/4; i++) {
        BREG_Write32( in_channelHandle->moduleHandle->regHandle,
            in_channelHandle->ulOutCmdBufAddr+ i*4, 0);
    }
#endif

    /* Check Parameter length */
    BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_PARM_LEN_ERR,
        ( inoutp_commandData->unInputParamLen > BCMD_NON_HEADER_BUFFER_BYTE_SIZE ) );

    /* Fill input buffer. */
    /* Version Number */
    BREG_Write32(  in_channelHandle->moduleHandle->regHandle,
        in_channelHandle->ulInCmdBufAddr + (BCMD_CommonBufferFields_eVerNum / 4) * 4,
        BSP_S_DRV_VERSION);

    /* Need a seq num: Owner ID */
    BREG_Write32(  in_channelHandle->moduleHandle->regHandle,
        in_channelHandle->ulInCmdBufAddr + (BCMD_CommonBufferFields_eOwnerId / 4) * 4,
        in_channelHandle->ulSequenceNum);

    /* Continuous Mode */
    inVal = (inoutp_commandData->unContMode & CMODE_PADDING_MSK) | CMODE_PADDING;
    BREG_Write32(  in_channelHandle->moduleHandle->regHandle,
        in_channelHandle->ulInCmdBufAddr + (BCMD_CommonBufferFields_eContMode / 4) * 4,
        inVal);

    /* Fill in Tag ID */
    inVal = (inoutp_commandData->cmdId & CTAGID_PADDING_MSK) |
            ((~(inoutp_commandData->cmdId)) << CTAGID_SHFT) |
            CTAGID_PADDING;
    BREG_Write32(  in_channelHandle->moduleHandle->regHandle,
        in_channelHandle->ulInCmdBufAddr + (BCMD_CommonBufferFields_eTagId / 4) * 4,
        inVal );

    /* Parameter Length */
    inVal = (inoutp_commandData->unInputParamLen & CPLEN_PADDING_MSK) |
            CPLEN_PADDING;
    BREG_Write32(  in_channelHandle->moduleHandle->regHandle,
        in_channelHandle->ulInCmdBufAddr + (BCMD_CommonBufferFields_eParamLenPlus1 / 4) * 4,
        inVal);

    /* Fill in specific input command parameters */
    for(i = 0; i<(inoutp_commandData->unInputParamLen+3)/4; i++) {
        BREG_Write32( in_channelHandle->moduleHandle->regHandle,
            in_channelHandle->ulInCmdBufAddr+ ((BCMD_CommonBufferFields_eStatus / 4) * 4) + i*4,
            inoutp_commandData->unInputParamsBuf[i]);
    }

    /* Dump out the input commands */
    {
        unsigned int i;
        BDBG_MSG(("MIPS -----------------------------------> BSP:"));
        for(i = 0; i<(inoutp_commandData->unInputParamLen+3+BCMD_HEADER_BYTE_SIZE)/4; i++) {
            /* Data */
            BDBG_MSG(("\t\t\t%.3d - %08X ", i*4, BREG_Read32( in_channelHandle->moduleHandle->regHandle, in_channelHandle->ulInCmdBufAddr+ i*4)));
        }
    }
    if ((in_channelHandle->moduleHandle->currentSettings.uSpecialControl & BHSM_CTRLS_POLLINGORISR) ==  BHSM_CTRL_SET_POLLING)
    {
        /* now to disable HSM ISR,  the following polling should dominate;
              Note: this is only disable the channel2 HSM ISR.  channel1 HSM ISR  (high priority for Cancel cmd) is not existing for 3563
           */

        /* moved to PI, once per setting change as Bryant suggested, which may reduce robustness
        BREG_Write32(in_channelHandle->moduleHandle->regHandle, BCHP_BSP_GLB_CONTROL_GLB_HOST_INTR_EN,
                                BREG_Read32(  in_channelHandle->moduleHandle->regHandle,
                                            BCHP_BSP_GLB_CONTROL_GLB_HOST_INTR_EN) &
                                            (~BCHP_BSP_GLB_CONTROL_GLB_HOST_INTR_STATUS_OLOAD2_INT_MASK));

           BREG_Write32(  in_channelHandle->moduleHandle->regHandle,
                            BCHP_BSP_GLB_CONTROL_GLB_OLOAD2,
                            BREG_Read32(  in_channelHandle->moduleHandle->regHandle,
                                            BCHP_BSP_GLB_CONTROL_GLB_OLOAD2) &
                                            (~BCHP_BSP_GLB_CONTROL_GLB_OLOAD2_CMD_OLOAD2_MASK));
        */

        BDBG_MSG(("////////// HSM in Polling,  ISR2 is disabled, %x, channel num=%d",
                        in_channelHandle->moduleHandle->currentSettings.uSpecialControl,
                        in_channelHandle->ucChannelNumber));
    }
    else
    {
        /*
        BREG_Write32(in_channelHandle->moduleHandle->regHandle, BCHP_BSP_GLB_CONTROL_GLB_HOST_INTR_EN,
                                BREG_Read32(  in_channelHandle->moduleHandle->regHandle,     BCHP_BSP_GLB_CONTROL_GLB_HOST_INTR_EN) |
                                            BCHP_BSP_GLB_CONTROL_GLB_HOST_INTR_STATUS_OLOAD2_INT_MASK);     */

        BKNI_ResetEvent(in_channelHandle->oLoadWait);        /* CTRL-C or any system level signal*/
        BDBG_MSG(("//////////HSM in ISR,  ISR2 is enabled, %x, channel num=%d",
                        in_channelHandle->moduleHandle->currentSettings.uSpecialControl,
                        in_channelHandle->ucChannelNumber));
    }


    /* Set ILOAD to signal BSP there's a command ready. */
    BDBG_MSG(("BHSM_P_SubmitCommand: ulILoadRegAddr = 0x%08lx, ulILoadVal = %ld",
        in_channelHandle->ulILoadRegAddr, in_channelHandle->ulILoadVal));
    BREG_Write32( in_channelHandle->moduleHandle->regHandle,
            in_channelHandle->ulILoadRegAddr,
            in_channelHandle->ulILoadVal);



/*--------------------start of experiement  12/19/06: ly----------------------------------------------
    as Bryant request for a test for Jeff Wong
       replace the ISR based processing with polling, to see if there is some performance improvement
 */


if ( (in_channelHandle->moduleHandle->currentSettings.uSpecialControl & BHSM_CTRLS_POLLINGORISR) == BHSM_CTRL_SET_POLLING)
{

#if BHSM_DEBUG_POLLING
     BREG_Write32(in_channelHandle->moduleHandle->regHandle, BCHP_TIMER_TIMER1_CTRL , 0x9FFFFFFF); /*start */
     startValue = BREG_Read32( in_channelHandle->moduleHandle->regHandle, BCHP_TIMER_TIMER1_STAT ) & 0x3FFFFFFF;
#endif

        /*for(;;),   about 20us per attemp, 2x500 = 1000 = 1ms */
    for (i=0; i< in_channelHandle->moduleHandle->currentSettings.ulTimeOutInMilSecs * 500 ;i++)
    {
        j = BREG_Read32(  in_channelHandle->moduleHandle->regHandle,     BCHP_BSP_GLB_CONTROL_GLB_OLOAD2);

#if BHSM_DEBUG_POLLING     /* as close as to the OLOAD2 reading moment*/
        endValue = BREG_Read32( in_channelHandle->moduleHandle->regHandle, BCHP_TIMER_TIMER1_STAT ) & 0x3FFFFFFF;
#endif
        BDBG_MSG(( "////BREG_Read32(BCHP_BSP_GLB_CONTROL_GLB_OLOAD2) = %x ", j));
        if (    ( j & BCHP_BSP_GLB_CONTROL_GLB_OLOAD2_CMD_OLOAD2_MASK) == 1  )
        {
            break;
        }
        BKNI_Delay(2);   /* to 2 from 19 plus ~1*/


    }
#if BHSM_DEBUG_POLLING   /* buffer it to improve timing performance*/
    /* convert to units of us, slighter faster than the actual timing due to a timer1 late start*/
    in_channelHandle->pollingIntervalBuf[in_channelHandle->pollingBufIndex] = (uint16_t)((endValue - startValue )/27);
    in_channelHandle->pollingBufIndex = (in_channelHandle->pollingBufIndex+1)%BHSM_DEBUG_POLLINGBUF_MAX;
    BREG_Write32(in_channelHandle->moduleHandle->regHandle, BCHP_TIMER_TIMER1_CTRL , 0x1FFFFFFF); /*stop */

#endif

        if (j==0)
        {
        BDBG_ERR(("BHSM_P_CommonSubmitCommand: Polling timeout error %d", in_channelHandle->moduleHandle->currentSettings.ulTimeOutInMilSecs));
        errCode = BHSM_STATUS_TIME_OUT;
        goto BHSM_P_DONE_LABEL;
        }
      /* crash the system, no direct operation. *(volatile uint32_t *) 0x1030b020 =0;  */ /* clear the Output ready flag*/
       BREG_Write32(  in_channelHandle->moduleHandle->regHandle,
                        BCHP_BSP_GLB_CONTROL_GLB_OLOAD2,
                        BREG_Read32(  in_channelHandle->moduleHandle->regHandle,
                                        BCHP_BSP_GLB_CONTROL_GLB_OLOAD2) &
                                        (~BCHP_BSP_GLB_CONTROL_GLB_OLOAD2_CMD_OLOAD2_MASK));


}
else
{

    if ( (errCode = BERR_TRACE(BKNI_WaitForEvent( in_channelHandle->oLoadWait,
                in_channelHandle->moduleHandle->currentSettings.ulTimeOutInMilSecs))) != BERR_SUCCESS ) {

        /* CTRL-C issue: according to Min requirement, App will loop to free the key slot, one slot each call ,
            App wants to continue anyway, assume CTRL-C caused an extra Event.
            Side effect: the timeout duration is doubled at worst case. Timeout normally won't happen. 2/26/07
        */

        if (errCode == BERR_OS_ERROR)
        {
            /*  1) use BKNI_Delay(), polling for a non-OS return, with a max of tolerated time of ulTimeOutInMilSecs
                    blocking with 1us (seems ok), 1us-pe-waiting which will be asynced
                   2) use BKNI_Sleep(),returns OS_ERROR also; then we have to mark the OS event in ISR handler
            */
            for (i=0; i<in_channelHandle->moduleHandle->currentSettings.ulTimeOutInMilSecs*1000; i++)
            {
                BKNI_Delay(1);

                if (in_channelHandle->oLoadSet)  /*   ISR action must be checked before waitforevent */
                {
                    errCode = BERR_SUCCESS;         /*  assume it's good if ISR has set OLoad*/
                    break;
                }
#if 0
                errCode = BERR_TRACE(BKNI_WaitForEvent( in_channelHandle->oLoadWait,
                                        in_channelHandle->moduleHandle->currentSettings.ulTimeOutInMilSecs));
                if (errCode != BERR_OS_ERROR)
                {
                    break;    /*  stop the loop to continue the cmd parsing, for TIMEOUT or SUCCESS */
                }
#endif
            }
        }

        if(i== in_channelHandle->moduleHandle->currentSettings.ulTimeOutInMilSecs*1000)
        {
            errCode = BHSM_STATUS_TIME_OUT;   /* CTRL-C, OS_ERROR stays all the time in current app cycle */
        }

        /* by here, should not be an OS event*/
        if (errCode != BERR_SUCCESS)
        {
            BDBG_ERR(("BHSM-SubmitCommand:  Error Code= %x returned from BKNI_WaitForEvent", errCode));
            /*errCode = BHSM_STATUS_TIME_OUT;     no enforement */
            goto BHSM_P_DONE_LABEL;
        }
    }
 }
    /*BKNI_Delay(100); */ /* 1) if no such delay, the following first Buffer2_Read is failed with a 'zero' reading ;
                                       2) 50us is not long enough to solve this issue
                                       3) 500us or 600us more is good to get the right data from Buffer2*/
    /* Read output buffer */
    /* Version Number */
    unVersionNum = BREG_Read32(  in_channelHandle->moduleHandle->regHandle,
            in_channelHandle->ulOutCmdBufAddr + (BCMD_CommonBufferFields_eVerNum / 4) * 4) & 0xFF;

    /* Owner ID */
    unOwnerId = BREG_Read32(  in_channelHandle->moduleHandle->regHandle,
            in_channelHandle->ulOutCmdBufAddr +  (BCMD_CommonBufferFields_eOwnerId / 4) * 4) & 0xFF;

    /* Continuous Mode */
    unContMode = BREG_Read32(  in_channelHandle->moduleHandle->regHandle,
            in_channelHandle->ulOutCmdBufAddr +  (BCMD_CommonBufferFields_eContMode / 4) * 4) & 0xFF;

    /* Tag ID */
    unTagId = BREG_Read32(  in_channelHandle->moduleHandle->regHandle,
            in_channelHandle->ulOutCmdBufAddr +  (BCMD_CommonBufferFields_eTagId / 4) * 4) & 0xFF;

    unTagId &= CTAGID_PADDING_MSK;
    /* Check Tag ID */
    BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_PARM_LEN_ERR,
        (unTagId !=  inoutp_commandData->cmdId ) );

    /* Parameter Length */
    inoutp_commandData->unOutputParamLen = BREG_Read32(  in_channelHandle->moduleHandle->regHandle,
            in_channelHandle->ulOutCmdBufAddr +   (BCMD_CommonBufferFields_eParamLenPlus1 / 4) * 4) & 0xFFFF;

    /* Check Parameter length */
    BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_PARM_LEN_ERR,
        (inoutp_commandData->unOutputParamLen > BCMD_NON_HEADER_BUFFER_BYTE_SIZE ) );

    /* Check if owner id still the same */
    BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_OWNER_ID_ERR,
        (unOwnerId != in_channelHandle->ulSequenceNum ) );

    /* Fill in specific output command parameters , +3 to resolve DH return in bits, e.g. 2018 bits 252bytes+2 bits*/
    for(i = 0; i< (inoutp_commandData->unOutputParamLen + 3)/4; i++) {

        inoutp_commandData->unOutputParamsBuf[i] =
            BREG_Read32( in_channelHandle->moduleHandle->regHandle,
            in_channelHandle->ulOutCmdBufAddr+ ((BCMD_CommonBufferFields_eStatus / 4) * 4)  + i*4);
        /*  strong debug to see what is actually copied from ISR buffer into HSM command buffer
        BDBG_MSG( ("i =%d, inoutp_commandData->unOutputParamsBuf[i]=%.8x",i, inoutp_commandData->unOutputParamsBuf[i]));
        */
    }

    /* unOutputParamsBuf[0] is an 8 bit status */
    inoutp_commandData->unOutputParamsBuf[0] &= 0xFF;

    /* Dump out the output commands, +3 to dump the rest bits in the last word32 */
    {
        unsigned int i;
        BDBG_MSG(("MIPS <----------------------------------- BSP:"));
        for(i = 0; i<(inoutp_commandData->unOutputParamLen+3+BCMD_HEADER_BYTE_SIZE)/4; i++) {
            /* Data */
            BDBG_MSG(("\t\t\t%.3d - %08X",  i*4,
                BREG_Read32( in_channelHandle->moduleHandle->regHandle,
                    in_channelHandle->ulOutCmdBufAddr+ i*4)));
        }
    }

    BDBG_MSG(("BHSM_P_CommonSubmitCommand output: unVersionNum = %08X, unOwnerId = %08X, unContMode = %x, unTagId = %08X, outp_unOutputParamLen = %08X",
        unVersionNum, unOwnerId, unContMode, unTagId, inoutp_commandData->unOutputParamLen));

    /* Increment the sequence number by 1 */
    if ((++in_channelHandle->ulSequenceNum) >= 256)
        in_channelHandle->ulSequenceNum = 0;

    /* Clear Input command buffer */
    for(i = 0; i<BCMD_BUFFER_BYTE_SIZE/4; i++) {
        BREG_Write32( in_channelHandle->moduleHandle->regHandle,
            in_channelHandle->ulInCmdBufAddr+ i*4, 0);
    }

BHSM_P_DONE_LABEL:

    in_channelHandle->oLoadSet=0;        /* avoid the leftover from a good HSM comand into a CTRL-C interrupted HSM cmd*/

    BDBG_LEAVE(BHSM_P_CommonSubmitCommand);
    return( errCode );

}

#ifdef BHSM_AUTO_TEST

BERR_Code BHSM_P_CommonSubmitRawCommand (
        BHSM_ChannelHandle        in_channelHandle,
        BHSM_P_CommandData_t    *inoutp_commandData
)
{
    BERR_Code errCode = BERR_SUCCESS;
    uint32_t    unVersionNum, unOwnerId, unContMode, unTagId, i, unIReady, j=0;
    uint32_t    inVal;
#if BHSM_DEBUG_POLLING
     uint32_t  startValue, endValue=0;
#endif
    BDBG_ENTER(BHSM_P_CommonSubmitRawCommand);

    BDBG_ASSERT( in_channelHandle );
    BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_FAILED,
        (in_channelHandle->ulMagicNumber != BHSM_P_CHANNEL_HANDLE_MAGIC_NUMBER ) );

    /* Check IReady */
    unIReady = BREG_Read32( in_channelHandle->moduleHandle->regHandle,
                            in_channelHandle->ulIReadyRegAddr);

    /* Check IReady , ulTimeOutInMilSecs_max = 120 000, multiplied by 500 is till less than FFFFFFFF, no overflow yet*/
    for (i=0; i< in_channelHandle->moduleHandle->currentSettings.ulTimeOutInMilSecs * 500 ;i++)
    {
        unIReady = BREG_Read32( in_channelHandle->moduleHandle->regHandle, in_channelHandle->ulIReadyRegAddr);
        if (unIReady & in_channelHandle->ulIReadyVal)
        {
            break;
        }
        BKNI_Delay(2);   /*  2us per loop, still blocking so far. May give CPU chance to others if it's non-blocking on other platforms*/
    }
    if ( i ==  in_channelHandle->moduleHandle->currentSettings.ulTimeOutInMilSecs * 500)
    {
         BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_IRDY_ERR, 1 );
    }

    /* Clear output command buffer */
    for(i = 0; i<BCMD_BUFFER_BYTE_SIZE/4; i++) {
        BREG_Write32( in_channelHandle->moduleHandle->regHandle,
            in_channelHandle->ulOutCmdBufAddr+ i*4, 0);
    }

    /* Fill in specific input command parameters ( entire cmd including header section words in RawCmd), loop in words */
    for(i = 0; i< (inoutp_commandData->unInputParamLen/4); i++) {  /* this length including header or not? try 'yes' */
        switch (i)
        {
            case 2 : /* Continual Mode */
                inVal = ((inoutp_commandData->unInputParamsBuf[i]) & CMODE_PADDING_MSK) | CMODE_PADDING;
                break;
            case 3 : /* Command Tag ID */
                inVal = ((inoutp_commandData->unInputParamsBuf[i]) & CTAGID_PADDING_MSK) | ((~(inoutp_commandData->unInputParamsBuf[i])) << CTAGID_SHFT) |CTAGID_PADDING;
                break;
            case 4 : /* Command Parameter Length */
                inVal = ((inoutp_commandData->unInputParamsBuf[i]) & CPLEN_PADDING_MSK) | CPLEN_PADDING;
                break;
            default:
                inVal = inoutp_commandData->unInputParamsBuf[i];


        }
        BREG_Write32( in_channelHandle->moduleHandle->regHandle,
            in_channelHandle->ulInCmdBufAddr+ i*4,
            inVal);  /*  unInputParamsBuf[]  has 5 header-section-words, in RawCmd*/
    }

    /* Dump out the input commands */
    {
        unsigned int i;
        BDBG_MSG(("MIPS -----------------------------------> BSP:"));
        for(i = 0; i<(inoutp_commandData->unInputParamLen+3)/4; i++) {
            /* Data */
            BDBG_MSG(("\t\t\t%.3d - %08X ", i*4, BREG_Read32( in_channelHandle->moduleHandle->regHandle, in_channelHandle->ulInCmdBufAddr+ i*4)));
        }
    }
if ((in_channelHandle->moduleHandle->currentSettings.uSpecialControl & BHSM_CTRLS_POLLINGORISR) ==  BHSM_CTRL_SET_POLLING)
{
    BDBG_MSG(("////////// HSM in Polling,  ISR2 is disabled, %x, channel num=%d",
                    in_channelHandle->moduleHandle->currentSettings.uSpecialControl,
                    in_channelHandle->ucChannelNumber));
}else
{

        BKNI_ResetEvent(in_channelHandle->oLoadWait);        /* CTRL-C or any system level signal*/
    BDBG_MSG(("//////////HSM in ISR,  ISR2 is enabled, %x, channel num=%d",
                    in_channelHandle->moduleHandle->currentSettings.uSpecialControl,
                    in_channelHandle->ucChannelNumber));
}


    /* Set ILOAD to signal BSP there's a command ready. */
    BDBG_MSG(("BHSM_P_CommonSubmitRawCommand: ulILoadRegAddr = 0x%08lx, ulILoadVal = %ld",
        in_channelHandle->ulILoadRegAddr, in_channelHandle->ulILoadVal));
    BREG_Write32( in_channelHandle->moduleHandle->regHandle,
            in_channelHandle->ulILoadRegAddr,
            in_channelHandle->ulILoadVal);



/*--------------------start of experiement  12/19/06: ly----------------------------------------------
    as Bryant request for a test for Jeff Wong
       replace the ISR based processing with polling, to see if there is some performance improvement
 */


if ( (in_channelHandle->moduleHandle->currentSettings.uSpecialControl & BHSM_CTRLS_POLLINGORISR) == BHSM_CTRL_SET_POLLING)
{

#if BHSM_DEBUG_POLLING
     BREG_Write32(in_channelHandle->moduleHandle->regHandle, BCHP_TIMER_TIMER1_CTRL , 0x9FFFFFFF); /*start */
     startValue = BREG_Read32( in_channelHandle->moduleHandle->regHandle, BCHP_TIMER_TIMER1_STAT ) & 0x3FFFFFFF;
#endif

        /*for(;;),   about 20us per attemp, 2x500 = 1000 = 1ms */
    for (i=0; i< in_channelHandle->moduleHandle->currentSettings.ulTimeOutInMilSecs * 500 ;i++)
    {
        j = BREG_Read32(  in_channelHandle->moduleHandle->regHandle,     BCHP_BSP_GLB_CONTROL_GLB_OLOAD2);

#if BHSM_DEBUG_POLLING     /* as close as to the OLOAD2 reading moment*/
        endValue = BREG_Read32( in_channelHandle->moduleHandle->regHandle, BCHP_TIMER_TIMER1_STAT ) & 0x3FFFFFFF;
#endif
        BDBG_MSG(( "////BREG_Read32(BCHP_BSP_GLB_CONTROL_GLB_OLOAD2) = %x ", j));
        if (    ( j & BCHP_BSP_GLB_CONTROL_GLB_OLOAD2_CMD_OLOAD2_MASK) == 1  )
        {
            break;
        }
        BKNI_Delay(2);   /* to 2 from 19 plus ~1*/


    }
#if BHSM_DEBUG_POLLING   /* buffer it to improve timing performance*/
    /* convert to units of us, slighter faster than the actual timing due to a timer1 late start*/
    in_channelHandle->pollingIntervalBuf[in_channelHandle->pollingBufIndex] = (uint16_t)((endValue - startValue )/27);
    in_channelHandle->pollingBufIndex = (in_channelHandle->pollingBufIndex+1)%BHSM_DEBUG_POLLINGBUF_MAX;
    BREG_Write32(in_channelHandle->moduleHandle->regHandle, BCHP_TIMER_TIMER1_CTRL , 0x1FFFFFFF); /*stop */

#endif

        if (j==0)
        {
        BDBG_ERR(("BHSM_P_CommonSubmitRawCommand: Polling timeout error %d", in_channelHandle->moduleHandle->currentSettings.ulTimeOutInMilSecs));
        errCode = BHSM_STATUS_TIME_OUT;
        goto BHSM_P_DONE_LABEL;
        }
      /* crash the system, no direct operation. *(volatile uint32_t *) 0x1030b020 =0;  */ /* clear the Output ready flag*/
       BREG_Write32(  in_channelHandle->moduleHandle->regHandle,
                        BCHP_BSP_GLB_CONTROL_GLB_OLOAD2,
                        BREG_Read32(  in_channelHandle->moduleHandle->regHandle,
                                        BCHP_BSP_GLB_CONTROL_GLB_OLOAD2) &
                                        (~BCHP_BSP_GLB_CONTROL_GLB_OLOAD2_CMD_OLOAD2_MASK));


}
else
{

    if ( (errCode = BERR_TRACE(BKNI_WaitForEvent( in_channelHandle->oLoadWait,
                in_channelHandle->moduleHandle->currentSettings.ulTimeOutInMilSecs))) != BERR_SUCCESS ) {

        /* CTRL-C issue: according to Min requirement, App will loop to free the key slot, one slot each call ,
            App wants to continue anyway, assume CTRL-C caused an extra Event.
            Side effect: the timeout duration is doubled at worst case. Timeout normally won't happen. 2/26/07
        */

        if (errCode == BERR_OS_ERROR)
        {
            /*  1) use BKNI_Delay(), polling for a non-OS return, with a max of tolerated time of ulTimeOutInMilSecs
                    blocking with 1us (seems ok), 1us-pe-waiting which will be asynced
                   2) use BKNI_Sleep(),returns OS_ERROR also; then we have to mark the OS event in ISR handler
            */
            for (i=0; i<in_channelHandle->moduleHandle->currentSettings.ulTimeOutInMilSecs*1000; i++)
            {
                BKNI_Delay(1);

                if (in_channelHandle->oLoadSet)  /*   ISR action must be checked before waitforevent */
                {
                    errCode = BERR_SUCCESS;         /*  assume it's good if ISR has set OLoad*/
                    break;
                }
#if 0
                errCode = BERR_TRACE(BKNI_WaitForEvent( in_channelHandle->oLoadWait,
                                        in_channelHandle->moduleHandle->currentSettings.ulTimeOutInMilSecs));
                if (errCode != BERR_OS_ERROR)
                {
                    break;    /*  stop the loop to continue the cmd parsing, for TIMEOUT or SUCCESS */
                }
#endif
            }
        }

        if(i== in_channelHandle->moduleHandle->currentSettings.ulTimeOutInMilSecs*1000)
        {
            errCode = BHSM_STATUS_TIME_OUT;   /* CTRL-C, OS_ERROR stays all the time in current app cycle */
        }

        /* by here, should not be an OS event*/
        if (errCode != BERR_SUCCESS)
        {
            BDBG_ERR(("BHSM_P_CommonSubmitRawCommand:  Error Code= %x returned from BKNI_WaitForEvent", errCode));
            /*errCode = BHSM_STATUS_TIME_OUT;     no enforement */
            goto BHSM_P_DONE_LABEL;
        }
    }
 }
    /*BKNI_Delay(100); */ /* 1) if no such delay, the following first Buffer2_Read is failed with a 'zero' reading ;
                                       2) 50us is not long enough to solve this issue
                                       3) 500us or 600us more is good to get the right data from Buffer2*/
    /* Read output buffer */
    /* Version Number */
    unVersionNum = BREG_Read32(  in_channelHandle->moduleHandle->regHandle,
            in_channelHandle->ulOutCmdBufAddr + (BCMD_CommonBufferFields_eVerNum / 4) * 4);

    /* Owner ID */
    unOwnerId = BREG_Read32(  in_channelHandle->moduleHandle->regHandle,
            in_channelHandle->ulOutCmdBufAddr +  (BCMD_CommonBufferFields_eOwnerId / 4) * 4);

    /* Continuous Mode */
    unContMode = BREG_Read32(  in_channelHandle->moduleHandle->regHandle,
            in_channelHandle->ulOutCmdBufAddr +  (BCMD_CommonBufferFields_eContMode / 4) * 4);

    /* Tag ID */
    unTagId = BREG_Read32(  in_channelHandle->moduleHandle->regHandle,
            in_channelHandle->ulOutCmdBufAddr +  (BCMD_CommonBufferFields_eTagId / 4) * 4);
    unTagId &= CTAGID_PADDING_MSK;

    /* Check Tag ID, not for no-os */
    /*    BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_PARM_LEN_ERR,
        (unTagId !=  inoutp_commandData->cmdId ) );
    */
    /* Parameter Length */
    inoutp_commandData->unOutputParamLen = BCMD_HEADER_BYTE_SIZE + (BREG_Read32(  in_channelHandle->moduleHandle->regHandle,
            in_channelHandle->ulOutCmdBufAddr +   (BCMD_CommonBufferFields_eParamLenPlus1 / 4) * 4) & 0xFF);

    /* Check Parameter length */
    BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_PARM_LEN_ERR,
        (inoutp_commandData->unOutputParamLen > BCMD_BUFFER_BYTE_SIZE ) );

    /* Check if owner id still the same */
    /*    BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_OWNER_ID_ERR,
        (unOwnerId != in_channelHandle->ulSequenceNum ) );
    */

    /* Fill in specific output command parameters , +3 to resolve DH return in bits, e.g. 2018 bits 252bytes+2 bits*/
    for(i = 0; i< (inoutp_commandData->unOutputParamLen + 3)/4; i++) {

        inoutp_commandData->unOutputParamsBuf[i] =
            BREG_Read32( in_channelHandle->moduleHandle->regHandle,
            in_channelHandle->ulOutCmdBufAddr+  i*4);
        /*  strong debug to see what is actually copied from ISR buffer into HSM command buffer
        BDBG_MSG( ("i =%d, inoutp_commandData->unOutputParamsBuf[i]=%.8x",i, inoutp_commandData->unOutputParamsBuf[i]));
        */
    }

    /* Dump out the output commands, +3 to dump the rest bits in the last word32 */
    {
        unsigned int i;
        BDBG_MSG(("MIPS <----------------------------------- BSP:"));
        for(i = 0; i<(inoutp_commandData->unOutputParamLen+3)/4; i++) {
            /* Data */
            BDBG_MSG(("\t\t\t%.3d - %08X",  i*4,
                        inoutp_commandData->unOutputParamsBuf[i]));
            /*
                BREG_Read32( in_channelHandle->moduleHandle->regHandle,
                    in_channelHandle->ulOutCmdBufAddr+ i*4)));  */
        }
    }

    BDBG_MSG(("BHSM_P_CommonSubmitRawCommand output: unVersionNum = %08X, unOwnerId = %08X, unContMode = %x, unTagId = %08X, outp_unOutputParamLen = %08X",
        unVersionNum, unOwnerId, unContMode, unTagId, inoutp_commandData->unOutputParamLen));

    /* Increment the sequence number by 1 */
    if ((++in_channelHandle->ulSequenceNum) >= 256)
        in_channelHandle->ulSequenceNum = 0;

    /* Clear Input command buffer */
    for(i = 0; i<BCMD_BUFFER_BYTE_SIZE/4; i++) {
        BREG_Write32( in_channelHandle->moduleHandle->regHandle,
            in_channelHandle->ulInCmdBufAddr+ i*4, 0);
    }

BHSM_P_DONE_LABEL:

    in_channelHandle->oLoadSet=0;        /* avoid the leftover from a good HSM comand into a CTRL-C interrupted HSM cmd*/

    BDBG_LEAVE(BHSM_P_CommonSubmitRawCommand);
    return( errCode );
}
#endif

#if BHSM_PATCH_MEMC_DMA            /* added to reduce footprint for other chip/versions without patch*/

BERR_Code BHSM_P_CommonSubmitCommand_DMA (
        BHSM_ChannelHandle        in_channelHandle,
        BHSM_P_CommandData_t    *inoutp_commandData
)
{
    BERR_Code errCode = BERR_SUCCESS;
    uint32_t    unVersionNum, unOwnerId, unContMode, unTagId, i, unIReady, j=0;
#if BHSM_DEBUG_POLLING
     uint32_t  startValue, endValue=0;
#endif

    uint32_t  inVal;

#if BHSM_PATCH_MEMC_DMA
    uint32_t            memcBackup=0;
#endif
    BDBG_ENTER(BHSM_P_CommonSubmitCommand_DMA);

    BDBG_ASSERT( in_channelHandle );
    BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_FAILED,
        (in_channelHandle->ulMagicNumber != BHSM_P_CHANNEL_HANDLE_MAGIC_NUMBER ) );

#if BHSM_PATCH_MEMC_DMA
        memcBackup = BREG_Read32( in_channelHandle->moduleHandle->regHandle,     BHSM_PATCH_MEMC_DMA_REG );
    BREG_Write32( in_channelHandle->moduleHandle->regHandle,BHSM_PATCH_MEMC_DMA_REG,BHSM_PATCH_MEMC_DMA_BOMASK);

#if BKNI_ACQUIREMUTEX_LOCK_TRUST
    i = (BHSM_PATCH_MEMC_DMA_WAITING*2+500)/1000;   /* 1.5ms will be treated as 2ms */
    BKNI_Sleep(i);
#else
    for (i=0; i<  BHSM_PATCH_MEMC_DMA_WAITING ;i++)   /* pls adjust the macro value to change your actual timing*/
    {
        BKNI_Delay(2);
    }
#endif

#endif


    /* Check IReady , ulTimeOutInMilSecs_max = 120 000, multiplied by 500 is till less than FFFFFFFF, no overflow yet*/
    for (i=0; i< in_channelHandle->moduleHandle->currentSettings.ulTimeOutInMilSecs * 500 ;i++)
    {
        unIReady = BREG_Read32( in_channelHandle->moduleHandle->regHandle,
                            in_channelHandle->ulIReadyRegAddr);
        if (unIReady & in_channelHandle->ulIReadyVal) {
            /*
            errCode = BERR_TRACE(errCode);
            goto BHSM_P_DONE_LABEL;
            */
            break;
        }
        BKNI_Delay(2);   /*  2us per loop, still blocking so far. May give CPU chance to others if it's non-blocking on other platforms*/
    }
    if ( i ==  in_channelHandle->moduleHandle->currentSettings.ulTimeOutInMilSecs * 500)
    {
         BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_IRDY_ERR, 1 );
    }

    /* Clear output command buffer */
    for(i = 0; i<BCMD_BUFFER_BYTE_SIZE/4; i++) {
        BREG_Write32( in_channelHandle->moduleHandle->regHandle,
            in_channelHandle->ulOutCmdBufAddr+ i*4, 0);
    }

    /* Check Parameter length */
    BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_PARM_LEN_ERR,
        ( inoutp_commandData->unInputParamLen > BCMD_NON_HEADER_BUFFER_BYTE_SIZE ) );

    /* Fill input buffer. */
    /* Version Number */
    BREG_Write32(  in_channelHandle->moduleHandle->regHandle,
        in_channelHandle->ulInCmdBufAddr + (BCMD_CommonBufferFields_eVerNum / 4) * 4,
        BSP_S_DRV_VERSION);

    /* Need a seq num: Owner ID */
    BREG_Write32(  in_channelHandle->moduleHandle->regHandle,
        in_channelHandle->ulInCmdBufAddr + (BCMD_CommonBufferFields_eOwnerId / 4) * 4,
        in_channelHandle->ulSequenceNum);

    /* Continuous Mode */
    inVal = (inoutp_commandData->unContMode & CMODE_PADDING_MSK) | CMODE_PADDING;
    BREG_Write32(  in_channelHandle->moduleHandle->regHandle,
        in_channelHandle->ulInCmdBufAddr + (BCMD_CommonBufferFields_eContMode / 4) * 4,
        inVal);

    /* Fill in Tag ID */
    inVal = (inoutp_commandData->cmdId & CTAGID_PADDING_MSK) |
            ((~(inoutp_commandData->cmdId)) << CTAGID_SHFT) |
            CTAGID_PADDING;
    BREG_Write32(  in_channelHandle->moduleHandle->regHandle,
        in_channelHandle->ulInCmdBufAddr + (BCMD_CommonBufferFields_eTagId / 4) * 4,
        inVal );

    /* Parameter Length */
    inVal = (inoutp_commandData->unInputParamLen & CPLEN_PADDING_MSK) |
            CPLEN_PADDING;
    BREG_Write32(  in_channelHandle->moduleHandle->regHandle,
        in_channelHandle->ulInCmdBufAddr + (BCMD_CommonBufferFields_eParamLenPlus1 / 4) * 4,
        inVal);

    /* Fill in specific input command parameters */
    for(i = 0; i<(inoutp_commandData->unInputParamLen+3)/4; i++) {
        BREG_Write32( in_channelHandle->moduleHandle->regHandle,
            in_channelHandle->ulInCmdBufAddr+ ((BCMD_CommonBufferFields_eStatus / 4) * 4) + i*4,
            inoutp_commandData->unInputParamsBuf[i]);
    }

    /* Dump out the input commands */
    {
        unsigned int i;
        BDBG_MSG(("MIPS -----------------------------------> BSP:"));
        for(i = 0; i<(inoutp_commandData->unInputParamLen+3+BCMD_HEADER_BYTE_SIZE)/4; i++) {
            /* Data */
            BDBG_MSG(("\t\t\t%.3d - %08X ", i*4, BREG_Read32( in_channelHandle->moduleHandle->regHandle, in_channelHandle->ulInCmdBufAddr+ i*4)));
        }
    }
if ((in_channelHandle->moduleHandle->currentSettings.uSpecialControl & BHSM_CTRLS_POLLINGORISR) ==  BHSM_CTRL_SET_POLLING)
{
    BDBG_MSG(("////////// HSM in Polling,  ISR2 is disabled, %x, channel num=%d",
                    in_channelHandle->moduleHandle->currentSettings.uSpecialControl,
                    in_channelHandle->ucChannelNumber));
}else
{

        BKNI_ResetEvent(in_channelHandle->oLoadWait);        /* CTRL-C or any system level signal*/
    BDBG_MSG(("//////////HSM in ISR,  ISR2 is enabled, %x, channel num=%d",
                    in_channelHandle->moduleHandle->currentSettings.uSpecialControl,
                    in_channelHandle->ucChannelNumber));
}


    /* Set ILOAD to signal BSP there's a command ready. */
    BDBG_MSG(("BHSM_P_SubmitCommand: ulILoadRegAddr = 0x%08lx, ulILoadVal = %ld",
        in_channelHandle->ulILoadRegAddr, in_channelHandle->ulILoadVal));
    BREG_Write32( in_channelHandle->moduleHandle->regHandle,
            in_channelHandle->ulILoadRegAddr,
            in_channelHandle->ulILoadVal);



/*--------------------start of experiement  12/19/06: ly----------------------------------------------
    as Bryant request for a test for Jeff Wong
       replace the ISR based processing with polling, to see if there is some performance improvement
 */


if ( (in_channelHandle->moduleHandle->currentSettings.uSpecialControl & BHSM_CTRLS_POLLINGORISR) == BHSM_CTRL_SET_POLLING)
{

#if BHSM_DEBUG_POLLING
     BREG_Write32(in_channelHandle->moduleHandle->regHandle, BCHP_TIMER_TIMER1_CTRL , 0x9FFFFFFF); /*start */
     startValue = BREG_Read32( in_channelHandle->moduleHandle->regHandle, BCHP_TIMER_TIMER1_STAT ) & 0x3FFFFFFF;
#endif

        /*for(;;),   about 20us per attemp, 2x500 = 1000 = 1ms */
    for (i=0; i< in_channelHandle->moduleHandle->currentSettings.ulTimeOutInMilSecs * 500 ;i++)
    {
        j = BREG_Read32(  in_channelHandle->moduleHandle->regHandle,     BCHP_BSP_GLB_CONTROL_GLB_OLOAD2);

#if BHSM_DEBUG_POLLING     /* as close as to the OLOAD2 reading moment*/
        endValue = BREG_Read32( in_channelHandle->moduleHandle->regHandle, BCHP_TIMER_TIMER1_STAT ) & 0x3FFFFFFF;
#endif
        BDBG_MSG(( "////BREG_Read32(BCHP_BSP_GLB_CONTROL_GLB_OLOAD2) = %x ", j));
        if (    ( j & BCHP_BSP_GLB_CONTROL_GLB_OLOAD2_CMD_OLOAD2_MASK) == 1  )
        {
            break;
        }
        BKNI_Delay(2);   /* to 2 from 19 plus ~1*/


    }
#if BHSM_DEBUG_POLLING   /* buffer it to improve timing performance*/
    /* convert to units of us, slighter faster than the actual timing due to a timer1 late start*/
    in_channelHandle->pollingIntervalBuf[in_channelHandle->pollingBufIndex] = (uint16_t)((endValue - startValue )/27);
    in_channelHandle->pollingBufIndex = (in_channelHandle->pollingBufIndex+1)%BHSM_DEBUG_POLLINGBUF_MAX;
    BREG_Write32(in_channelHandle->moduleHandle->regHandle, BCHP_TIMER_TIMER1_CTRL , 0x1FFFFFFF); /*stop */

#endif

        if (j==0)
        {
        BDBG_ERR(("BHSM_P_CommonSubmitCommand: Polling timeout error %d", in_channelHandle->moduleHandle->currentSettings.ulTimeOutInMilSecs));
        errCode = BHSM_STATUS_TIME_OUT;
        goto BHSM_P_DONE_LABEL;
        }
      /* crash the system, no direct operation. *(volatile uint32_t *) 0x1030b020 =0;  */ /* clear the Output ready flag*/
       BREG_Write32(  in_channelHandle->moduleHandle->regHandle,
                        BCHP_BSP_GLB_CONTROL_GLB_OLOAD2,
                        BREG_Read32(  in_channelHandle->moduleHandle->regHandle,
                                        BCHP_BSP_GLB_CONTROL_GLB_OLOAD2) &
                                        (~BCHP_BSP_GLB_CONTROL_GLB_OLOAD2_CMD_OLOAD2_MASK));


}
else
{

    if ( (errCode = BERR_TRACE(BKNI_WaitForEvent( in_channelHandle->oLoadWait,
                in_channelHandle->moduleHandle->currentSettings.ulTimeOutInMilSecs))) != BERR_SUCCESS ) {

        /* CTRL-C issue: according to Min requirement, App will loop to free the key slot, one slot each call ,
            App wants to continue anyway, assume CTRL-C caused an extra Event.
            Side effect: the timeout duration is doubled at worst case. Timeout normally won't happen. 2/26/07
        */

        if (errCode == BERR_OS_ERROR)
        {
            /*  1) use BKNI_Delay(), polling for a non-OS return, with a max of tolerated time of ulTimeOutInMilSecs
                    blocking with 1us (seems ok), 1us-pe-waiting which will be asynced
                   2) use BKNI_Sleep(),returns OS_ERROR also; then we have to mark the OS event in ISR handler
            */
            for (i=0; i<in_channelHandle->moduleHandle->currentSettings.ulTimeOutInMilSecs*1000; i++)
            {
                BKNI_Delay(1);

                if (in_channelHandle->oLoadSet)  /*   ISR action must be checked before waitforevent */
                {
                    errCode = BERR_SUCCESS;         /*  assume it's good if ISR has set OLoad*/
                    break;
                }
#if 0
                errCode = BERR_TRACE(BKNI_WaitForEvent( in_channelHandle->oLoadWait,
                                        in_channelHandle->moduleHandle->currentSettings.ulTimeOutInMilSecs));
                if (errCode != BERR_OS_ERROR)
                {
                    break;    /*  stop the loop to continue the cmd parsing, for TIMEOUT or SUCCESS */
                }
#endif
            }
        }

        if(i== in_channelHandle->moduleHandle->currentSettings.ulTimeOutInMilSecs*1000)
        {
            errCode = BHSM_STATUS_TIME_OUT;   /* CTRL-C, OS_ERROR stays all the time in current app cycle */
        }

        /* by here, should not be an OS event*/
        if (errCode != BERR_SUCCESS)
        {
            BDBG_ERR(("BHSM-SubmitCommand:  Error Code= %x returned from BKNI_WaitForEvent", errCode));
            /*errCode = BHSM_STATUS_TIME_OUT;     no enforement */
            goto BHSM_P_DONE_LABEL;
        }
    }
 }
    /*BKNI_Delay(100); */ /* 1) if no such delay, the following first Buffer2_Read is failed with a 'zero' reading ;
                                       2) 50us is not long enough to solve this issue
                                       3) 500us or 600us more is good to get the right data from Buffer2*/
    /* Read output buffer */
    /* Version Number */
    unVersionNum = BREG_Read32(  in_channelHandle->moduleHandle->regHandle,
            in_channelHandle->ulOutCmdBufAddr + (BCMD_CommonBufferFields_eVerNum / 4) * 4) & 0xFF;

    /* Owner ID */
    unOwnerId = BREG_Read32(  in_channelHandle->moduleHandle->regHandle,
            in_channelHandle->ulOutCmdBufAddr +  (BCMD_CommonBufferFields_eOwnerId / 4) * 4) & 0xFF;

    /* Continuous Mode */
    unContMode = BREG_Read32(  in_channelHandle->moduleHandle->regHandle,
            in_channelHandle->ulOutCmdBufAddr +  (BCMD_CommonBufferFields_eContMode / 4) * 4) & 0xFF;

    /* Tag ID */
    unTagId = BREG_Read32(  in_channelHandle->moduleHandle->regHandle,
            in_channelHandle->ulOutCmdBufAddr +  (BCMD_CommonBufferFields_eTagId / 4) * 4);

    unTagId &= CTAGID_PADDING_MSK;
    /* Check Tag ID */
    BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_PARM_LEN_ERR,
        (unTagId !=  inoutp_commandData->cmdId ) );

    /* Parameter Length */
    inoutp_commandData->unOutputParamLen = BREG_Read32(  in_channelHandle->moduleHandle->regHandle,
            in_channelHandle->ulOutCmdBufAddr +   (BCMD_CommonBufferFields_eParamLenPlus1 / 4) * 4) & 0xFFFF;

    /* Check Parameter length */
    BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_PARM_LEN_ERR,
        (inoutp_commandData->unOutputParamLen > BCMD_NON_HEADER_BUFFER_BYTE_SIZE ) );

    /* Check if owner id still the same */
    BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_OWNER_ID_ERR,
        (unOwnerId != in_channelHandle->ulSequenceNum ) );

    /* Fill in specific output command parameters , +3 to resolve DH return in bits, e.g. 2018 bits 252bytes+2 bits*/
    for(i = 0; i< (inoutp_commandData->unOutputParamLen + 3)/4; i++) {

        inoutp_commandData->unOutputParamsBuf[i] =
            BREG_Read32( in_channelHandle->moduleHandle->regHandle,
            in_channelHandle->ulOutCmdBufAddr+ ((BCMD_CommonBufferFields_eStatus / 4) * 4)  + i*4);
        /*  strong debug to see what is actually copied from ISR buffer into HSM command buffer
        BDBG_MSG( ("i =%d, inoutp_commandData->unOutputParamsBuf[i]=%.8x",i, inoutp_commandData->unOutputParamsBuf[i]));
        */
    }

    /* Dump out the output commands, +3 to dump the rest bits in the last word32 */
    {
        unsigned int i;
        BDBG_MSG(("MIPS <----------------------------------- BSP:"));
        for(i = 0; i<(inoutp_commandData->unOutputParamLen+3+BCMD_HEADER_BYTE_SIZE)/4; i++) {
            /* Data */
            BDBG_MSG(("\t\t\t%.3d - %08X ",  i*4,
                BREG_Read32( in_channelHandle->moduleHandle->regHandle,
                    in_channelHandle->ulOutCmdBufAddr+ i*4)));
        }
    }

    BDBG_MSG(("BHSM_P_CommonSubmitCommand output: unVersionNum = %08X, unOwnerId = %08X, unContMode = %x, unTagId = %08X, outp_unOutputParamLen = %08X",
        unVersionNum, unOwnerId, unContMode, unTagId, inoutp_commandData->unOutputParamLen));

    /* Increment the sequence number by 1 */
    if ((++in_channelHandle->ulSequenceNum) >= 256)
        in_channelHandle->ulSequenceNum = 0;

    /* Clear Input command buffer */
    for(i = 0; i<BCMD_BUFFER_BYTE_SIZE/4; i++) {
        BREG_Write32( in_channelHandle->moduleHandle->regHandle,
            in_channelHandle->ulInCmdBufAddr+ i*4, 0);
    }

BHSM_P_DONE_LABEL:

#if BHSM_PATCH_MEMC_DMA

#if BKNI_ACQUIREMUTEX_LOCK_TRUST
    i = (BHSM_PATCH_MEMC_DMA_WAITING*2+500)/1000;   /* 1.5ms will be treated as 2ms */
    BKNI_Sleep(i);
#else
    for (i=0; i<  BHSM_PATCH_MEMC_DMA_WAITING ;i++)   /* pls adjust the macro value to change your actual timing*/
    {
        BKNI_Delay(2);
    }
#endif

    BREG_Write32( in_channelHandle->moduleHandle->regHandle,     BHSM_PATCH_MEMC_DMA_REG, memcBackup );
#endif

    in_channelHandle->oLoadSet=0;        /* avoid the leftover from a good HSM comand into a CTRL-C interrupted HSM cmd*/

    BDBG_LEAVE(BHSM_P_CommonSubmitCommand_DMA);
    return( errCode );
}

#endif

#ifdef BHSM_AUTO_TEST

BERR_Code BHSM_P_CommonSubmitRawCommand_DMA (
        BHSM_ChannelHandle        in_channelHandle,
        BHSM_P_CommandData_t    *inoutp_commandData
)
{
    BERR_Code errCode = BERR_SUCCESS;
    uint32_t    unVersionNum, unOwnerId, unContMode, unTagId, i, unIReady;
    uint32_t    inVal;

#if BHSM_PATCH_MEMC_DMA
    uint32_t            memcBackup=0;
#endif

    BDBG_ENTER(BHSM_P_CommonSubmitRawCommand_DMA);
    BDBG_ASSERT( in_channelHandle );
    BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_FAILED,
        (in_channelHandle->ulMagicNumber != BHSM_P_CHANNEL_HANDLE_MAGIC_NUMBER ) );


#if BHSM_PATCH_MEMC_DMA
    memcBackup = BREG_Read32( in_channelHandle->moduleHandle->regHandle,     BHSM_PATCH_MEMC_DMA_REG );
    BREG_Write32( in_channelHandle->moduleHandle->regHandle,BHSM_PATCH_MEMC_DMA_REG,BHSM_PATCH_MEMC_DMA_BOMASK);

#if BKNI_ACQUIREMUTEX_LOCK_TRUST
    i = (BHSM_PATCH_MEMC_DMA_WAITING*2+500)/1000;   /* 1.5ms will be treated as 2ms */
    BKNI_Sleep(i);
#else
    for (i=0; i<  BHSM_PATCH_MEMC_DMA_WAITING ;i++)   /* pls adjust the macro value to change your actual timing*/
    {
        BKNI_Delay(2);
    }
#endif

#endif

    /* Check IReady */

    /* Check IReady , ulTimeOutInMilSecs_max = 120 000, multiplied by 500 is till less than FFFFFFFF, no overflow yet*/
    for (i=0; i< in_channelHandle->moduleHandle->currentSettings.ulTimeOutInMilSecs * 500 ;i++)
    {
        unIReady = BREG_Read32( in_channelHandle->moduleHandle->regHandle, in_channelHandle->ulIReadyRegAddr);
        if (unIReady & in_channelHandle->ulIReadyVal)
        {
            break;
        }
        BKNI_Delay(2);   /*  2us per loop, still blocking so far. May give CPU chance to others if it's non-blocking on other platforms*/
    }
    if ( i ==  in_channelHandle->moduleHandle->currentSettings.ulTimeOutInMilSecs * 500)
    {
         BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_IRDY_ERR, 1 );
    }

    /* Clear output command buffer */
    for(i = 0; i<BCMD_BUFFER_BYTE_SIZE/4; i++) {
        BREG_Write32( in_channelHandle->moduleHandle->regHandle,
            in_channelHandle->ulOutCmdBufAddr+ i*4, 0);
    }

    /* Fill in specific input command parameters */
    for(i = 0; i<inoutp_commandData->unInputParamLen/4; i++) {
                switch (i)
                {
                        case 2 : /* Continual Mode */
                                inVal = ((inoutp_commandData->unInputParamsBuf[i]) & CMODE_PADDING_MSK) | CMODE_PADDING;
                                break;
                        case 3 : /* Command Tag ID */
                                inVal = ((inoutp_commandData->unInputParamsBuf[i]) & CTAGID_PADDING_MSK) | ((~(inoutp_commandData->unInputParamsBuf[i])) << CTAGID_SHFT) |CTAGID_PADDING;
                                break;
                        case 4 : /* Command Parameter Length */
                                inVal = ((inoutp_commandData->unInputParamsBuf[i]) & CPLEN_PADDING_MSK) | CPLEN_PADDING;
                                break;
                        default:
                                inVal = inoutp_commandData->unInputParamsBuf[i];


                }

        BREG_Write32( in_channelHandle->moduleHandle->regHandle,
            in_channelHandle->ulInCmdBufAddr+ i*4,
            inVal);
    }

    /* Dump out the input commands */
    {
        unsigned int i;
        for(i = 0; i<inoutp_commandData->unInputParamLen; i++) {
            /* Data */
            BDBG_MSG(("%08X",  BREG_Read32( in_channelHandle->moduleHandle->regHandle,
                in_channelHandle->ulInCmdBufAddr+ i*4)));
        }
    }

    /* Set ILOAD to signal BSP there's a command ready. */
    BDBG_MSG(("BHSM_P_SubmitRawCommand_DMA: ulILoadRegAddr = 0x%08lx, ulILoadVal = %ld",
        in_channelHandle->ulILoadRegAddr, in_channelHandle->ulILoadVal));
    BREG_Write32( in_channelHandle->moduleHandle->regHandle,
            in_channelHandle->ulILoadRegAddr,
            in_channelHandle->ulILoadVal);



#if 1
    if ( (errCode = BERR_TRACE(BKNI_WaitForEvent( in_channelHandle->oLoadWait,
                in_channelHandle->moduleHandle->currentSettings.ulTimeOutInMilSecs))) != BERR_SUCCESS ) {

        BDBG_ERR(("BHSM_P_CommonSubmitRawCommand_DMA: BKNI_WaitForEvent timeout error %d", in_channelHandle->moduleHandle->currentSettings.ulTimeOutInMilSecs));
        errCode = BHSM_STATUS_TIME_OUT;
        goto BHSM_P_DONE_LABEL;
    }
#else
    BKNI_WaitForEvent( in_channelHandle->oLoadWait,
                in_channelHandle->moduleHandle->currentSettings.ulTimeOutInMilSecs);
#endif

    /* Read output buffer */
    /* Version Number */
    unVersionNum = BREG_Read32(  in_channelHandle->moduleHandle->regHandle,
            in_channelHandle->ulOutCmdBufAddr + (BCMD_CommonBufferFields_eVerNum / 4) * 4) & 0xFF;


    /* Owner ID */
    unOwnerId = BREG_Read32(  in_channelHandle->moduleHandle->regHandle,
            in_channelHandle->ulOutCmdBufAddr +  (BCMD_CommonBufferFields_eOwnerId / 4) * 4) & 0xFF;

    /* Continuous Mode */
    unContMode = BREG_Read32(  in_channelHandle->moduleHandle->regHandle,
            in_channelHandle->ulOutCmdBufAddr +  (BCMD_CommonBufferFields_eContMode / 4) * 4) & 0xFF;

    /* Tag ID */
    unTagId = BREG_Read32(  in_channelHandle->moduleHandle->regHandle,
            in_channelHandle->ulOutCmdBufAddr +  (BCMD_CommonBufferFields_eTagId / 4) * 4) & 0xFF;

    /* Parameter Length, including header section, translated in bytes */
    inoutp_commandData->unOutputParamLen = BCMD_HEADER_BYTE_SIZE + (BREG_Read32(  in_channelHandle->moduleHandle->regHandle,
            in_channelHandle->ulOutCmdBufAddr +   (BCMD_CommonBufferFields_eParamLenPlus1 / 4) * 4) & 0xFFFF) ;

    /* Check Parameter length */
    BHSM_P_CHECK_ERR_CODE_CONDITION( errCode, BHSM_STATUS_PARM_LEN_ERR,
        (inoutp_commandData->unOutputParamLen > BCMD_BUFFER_BYTE_SIZE ) );


    /* Fill in specific output command parameters, loop in words */
    for(i = 0; i<(inoutp_commandData->unOutputParamLen + 3)/4; i++) {

        inoutp_commandData->unOutputParamsBuf[i] =
            BREG_Read32( in_channelHandle->moduleHandle->regHandle,     in_channelHandle->ulOutCmdBufAddr  + i*4);
    }

    /* Dump out the output commands */
    {
        unsigned int i;
                 BDBG_MSG(("MIPS <----------------------------------- BSP:"));
        for(i = 0; i<(inoutp_commandData->unOutputParamLen+3)/4; i++) {
            /* Data */
            BDBG_MSG(("\t\t\t%.3d - %08X ",  i*4,
                BREG_Read32( in_channelHandle->moduleHandle->regHandle,
                    in_channelHandle->ulOutCmdBufAddr+ i*4)));
        }
    }



    /* Clear Input command buffer */
    for(i = 0; i<BCMD_BUFFER_BYTE_SIZE/4; i++) {
        BREG_Write32( in_channelHandle->moduleHandle->regHandle,
            in_channelHandle->ulInCmdBufAddr+ i*4, 0);
    }

BHSM_P_DONE_LABEL:

#if BHSM_PATCH_MEMC_DMA
#if BKNI_ACQUIREMUTEX_LOCK_TRUST
    i = (BHSM_PATCH_MEMC_DMA_WAITING*2+500)/1000;   /* 1.5ms will be treated as 2ms */
    BKNI_Sleep(i);
#else
    for (i=0; i<  BHSM_PATCH_MEMC_DMA_WAITING ;i++)   /* pls adjust the macro value to change your actual timing*/
    {
        BKNI_Delay(2);
    }
#endif
    BREG_Write32( in_channelHandle->moduleHandle->regHandle,     BHSM_PATCH_MEMC_DMA_REG, memcBackup );

#endif

    BDBG_LEAVE(BHSM_P_CommonSubmitRawCommand_DMA);
    return( errCode );
}

#endif

#if BHSM_DEBUG_POLLING
BERR_Code BHSM_TEST_PollingDebugInfor( BHSM_P_Handle    *  hHsm )
{
      int     i;

      BDBG_WRN(("=======HSM Polling Interval Data (us)========"));
     for (i=0; i<BHSM_DEBUG_POLLINGBUF_MAX; i+=8 )
     {
              /*if ( ( i%16 ==0) && (i !=0)) BDBG_MSG(( ""));*/
               BDBG_WRN((" %4d %4d %4d %4d %4d %4d %4d %4d ",
                       hHsm->channelHandles[BSP_CmdInterface]->pollingIntervalBuf[i],
                       hHsm->channelHandles[BSP_CmdInterface]->pollingIntervalBuf[i+1],
                       hHsm->channelHandles[BSP_CmdInterface]->pollingIntervalBuf[i+2],
                       hHsm->channelHandles[BSP_CmdInterface]->pollingIntervalBuf[i+3],
                       hHsm->channelHandles[BSP_CmdInterface]->pollingIntervalBuf[i+4],
                       hHsm->channelHandles[BSP_CmdInterface]->pollingIntervalBuf[i+5],
                       hHsm->channelHandles[BSP_CmdInterface]->pollingIntervalBuf[i+6],
                       hHsm->channelHandles[BSP_CmdInterface]->pollingIntervalBuf[i+7]
                       ));
     }

      BDBG_WRN(("=======Polling Interval Location: %d ========",
             hHsm->channelHandles[BSP_CmdInterface]->pollingBufIndex));
      return (BERR_SUCCESS);

}


uint32_t BHSM_P_HDMIeToSerializerAddress(
        uint32_t        portEnum
)
{

    return (portEnum);

}


#endif
