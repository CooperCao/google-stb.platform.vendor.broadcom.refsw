/***************************************************************************
 *     Copyright (c) 2003-2012, Broadcom Corporation
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
 * Module Description: This file contains Broadcom smart card Porting 
 *                     Interface private data structures, enums, 
 *                     definitions and functions prototypes.           
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef BSCD_PRIV_H__
#define BSCD_PRIV_H__

#include "berr_ids.h"
#include "bscd.h"

#ifdef __cplusplus
extern "C" {
#endif


/* Definitions */

/**
  Smart card D and F Table according to SC_PRESCALE Table, where F is Clock Rate 
  Conversion Factor and D is the Baud Rate Adjustment Factor.
  This table contains the recommended set of values for the SC_PRESCALE and SC_CLK_CMD based 
  on the ATR TA1 byte. The resulting clock rate and baud rate will comply to ISO 7816-3.
**/
typedef struct
{
      unsigned char sc_clkdiv;         /* clk divider in SC_CLK_CMD register */
      unsigned char sc_prescale;    /* prescale in SC_PRESCALE register */
      unsigned char sc_bauddiv;     /* baud rate divider in SC_CLK_CMD register */
      unsigned char sc_etuclkdiv;   /* etu clk divider in SC_CLK_CMD register */
} BSCD_P_DFSmartCardStruct;



#define BSCD_P_UART_CMD_1		0x00 /* SMART CARD UART COMMAND REGISTER */
#define BSCD_P_UART_CMD_2		0x40 /* SMART CARD UART COMMAND REGISTER */
#define BSCD_P_PROTO_CMD		0x0c /* SMART CARD PROTOCOL COMMAND REGISTER */
#define BSCD_P_FLOW_CMD		0x28 /* SMART CARD FLOW CONTROL COMMAND REGISTER */
#define BSCD_P_IF_CMD_1			0x04 /* SMART CARD INTERFACE COMMAND REGISTER */
#define BSCD_P_IF_CMD_2            	0x4c /* SMART CARD INTERFACE COMMAND REGISTER */
#define BSCD_P_INTR_STAT_1		0x58 /* SMART CARD INTERRUPT STATUS REGISTER */
#define BSCD_P_INTR_STAT_2		0x5c /* SMART CARD INTERRUPT STATUS REGISTER */
#define BSCD_P_INTR_EN_1		0x50 /* SMART CARD INTERRUPT ENABLE REGISTER */
#define BSCD_P_INTR_EN_2		0x54 /* SMART CARD INTERRUPT ENABLE REGISTER */
#define BSCD_P_CLK_CMD			0x08 /* SMART CARD CLOCK COMMAND */
#if(BCHP_CHIP==7420) ||(BCHP_CHIP==7340) ||(BCHP_CHIP==7342)||(BCHP_CHIP==7125)
#define BSCD_P_CLK_CMD_2		0x3c /* SMART CARD CLOCK COMMAND */
#endif
#define BSCD_P_PRESCALE			0x10 /* SMART CARD CLOCK PRESCALE */
#define BSCD_P_TIMER_CMD		0x48 /* SMART CARD TIMER COMMAND REGISTER */
#define BSCD_P_BGT				0x44 /* SMART CARD BLOCK GUARD TIME REGISTER */
#define BSCD_P_TIMER_CNT_1		0x68 /* SMART CARD GENERAL PURPOSE TIMER COUNT REGISTER */
#define BSCD_P_TIMER_CNT_2		0x6c /* SMART CARD GENERAL PURPOSE TIMER COUNT REGISTER */
#define BSCD_P_TIMER_CMP_1		0x60 /* SMART CARD GENERAL PURPOSE TIMER COMPARE REGISTER */
#define BSCD_P_TIMER_CMP_2		0x64 /* SMART CARD GENERAL PURPOSE TIMER COMPARE REGISTER */
#define BSCD_P_WAIT_1			0x70 /* SMART CARD WAITING TIMER REGISTER */
#define BSCD_P_WAIT_2			0x74 /* SMART CARD WAITING TIMER REGISTER */
#define BSCD_P_WAIT_3			0x78 /* SMART CARD WAITING TIMER REGISTER */
#define BSCD_P_TGUARD			0x14 /* SMART CARD TRANSMIT GUARD TIME REGISTER */
#define BSCD_P_TRANSMIT			0x18 /* SMART CARD TRANSMIT REGISTER */
#define BSCD_P_RECEIVE			0x1c /* SMART CARD RECEIVE REGISTER */
#define BSCD_P_STATUS_1			0x34 /* SMART CARD STATUS 1 REGISTER */
#define BSCD_P_STATUS_2			0x38 /* SMART CARD STATUS 2 REGISTER */
#define BSCD_P_TLEN_2			0x20 /* SMART CARD TRANSMIT LENGTH REGISTER */
#define BSCD_P_TLEN_1			0x24 /* SMART CARD TRANSMIT LENGTH REGISTER */
#define BSCD_P_RLEN_2			0x2c /* SMART CARD RECEIVE LENGTH REGISTER */
#define BSCD_P_RLEN_1			0x30 /* SMART CARD RECEIVE LENGTH REGISTER */
#ifdef BSCD_EMV2000_CWT_PLUS_4_EVENT_INTR
#define BSCD_P_EVENT1_CNT		0x80 /* SMART CARD EVENT 1 COUNT REGISTER */
#define BSCD_P_EVENT1_CMP 		0x88 /* SMART CARD EVENT 1 COMPARE REGISTER  */
#define BSCD_P_EVENT1_CMD_1 	0x90 /* SMART CARD EVENT 1 COMMAND 1 REGISTER */ 
#define BSCD_P_EVENT1_CMD_2 	0x94 /* SMART CARD EVENT 1 COMMAND 2 REGISTER  */
#define BSCD_P_EVENT1_CMD_3	0x98/* SMART CARD EVENT 1 COMMAND 3 REGISTER  */
#define BSCD_P_EVENT1_CMD_4	0x9c /* SMART CARD EVENT 1 COMMAND 4 REGISTER  */
#define BSCD_P_EVENT2_CMP		0xa0 /* SMART CARD EVENT 2 COMPARE REGISTER  */
#define BSCD_P_EVENT2_CNT		0xa8 /* SMART CARD EVENT 2 COUNT REGISTER  */
#define BSCD_P_EVENT2_CMD_1	0xb0 /* SMART CARD EVENT 2 COMMAND 1 REGISTER */ 
#define BSCD_P_EVENT2_CMD_2	0xb4 /* SMART CARD EVENT 2 COMMAND 2 REGISTER  */
#define BSCD_P_EVENT2_CMD_3	0xb8 /* SMART CARD EVENT 2 COMMAND 3 REGISTER  */
#define BSCD_P_EVENT2_CMD_4	0xbc /* SMART CARD EVENT 2 COMMAND 4 REGISTER  */
#endif


/* Smart Card Module Event source for Event interrupt */
#ifdef BSCD_EMV2000_CWT_PLUS_4_EVENT_INTR
#define  BSCD_P_EVENT1_INTR_EVENT_SRC		0x00
#define  BSCD_P_TEMPTY_INTR_EVENT_SRC		0x01
#define  BSCD_P_RETRY_INTR_EVENT_SRC		0x02
#define  BSCD_P_TDONE_INTR_EVENT_SRC		0x03
#define  BSCD_P_BGT_INTR_EVENT_SRC			0x04
#define  BSCD_P_PRES_INTR_EVENT_SRC			0x05
#define  BSCD_P_TIMER_INTR_EVENT_SRC		0x06
#define  BSCD_P_TPAR_INTR_EVENT_SRC			0x07
#define  BSCD_P_RREADY_INTR_EVENT_SRC		0x08
#define  BSCD_P_RCV_INTR_EVENT_SRC			0x09
#define  BSCD_P_EVENT2_INTR_EVENT_SRC		0x0a
#define  BSCD_P_WAIT_INTR_EVENT_SRC		0x0b
#define  BSCD_P_RLEN_INTR_EVENT_SRC			0x0c
#define  BSCD_P_CWT_INTR_EVENT_SRC			0x0d
#define  BSCD_P_ATRS_INTR_EVENT_SRC			0x0e
#define  BSCD_P_RPAR_INTR_EVENT_SRC			0x0f
#define  BSCD_P_RX_ETU_TICK_EVENT_SRC		0x10
#define  BSCD_P_TX_ETU_TICK_EVENT_SRC		0x11
#define  BSCD_P_HALF_TX_ETU_TICK_EVENT_SRC	0x12
#define  BSCD_P_RX_START_BIT_EVENT_SRC		0x13
#define  BSCD_P_TX_START_BIT_EVENT_SRC		0x14
#define  BSCD_P_LAST_TX_START_BIT_BLOCK_EVENT_SRC		0x15
#define  BSCD_P_ICC_FLOW_CTRL_ASSERTED_EVENT_SRC			0x16
#define  BSCD_P_ICC_FLOW_CTRL_DONE_EVENT_SRC	0x17
#define  BSCD_P_START_IMMEDIATE_EVENT_SRC	0x1f
#define  BSCD_P_DISABLE_COUNTING_EVENT_SRC	0x1f
#define  BSCD_P_NO_EVENT_EVENT_SRC	0x1f
#endif

/* Smart Card Module magic number used to check if opaque handle is corrupt */
#define BSCD_P_HANDLE_MAGIC_NUMBER           0xdeadbeef

/* Smart Card Channel magic number used to check if opaque handle is corrupt */
#define BSCD_P_CHANNEL_HANDLE_MAGIC_NUMBER   0xfeedbabe

#if ((BCHP_CHIP==7125)&& (BCHP_VER<BCHP_VER_C0))
#define BCHP_SCA_SC_UART_CMD_1 BCHP_SCB_SC_UART_CMD_1
#define BCHP_SCA_SC_UART_CMD_2 BCHP_SCB_SC_UART_CMD2_
#define BCHP_SCA_SC_STATUS_1_card_pres_MASK BCHP_SCB_SC_STATUS_1_card_pres_MASK
#define BCHP_SCA_SC_IF_CMD_1_pres_pol_MASK BCHP_SCB_SC_IF_CMD_1_pres_pol_MASK 
#define BCHP_SCA_SC_CLK_CMD_clk_en_MASK BCHP_SCB_SC_CLK_CMD_1_clk_en_MASK
#define	BCHP_SCA_SC_CLK_CMD_bauddiv_MASK BCHP_SCB_SC_CLK_CMD_1_bauddiv_MASK
#define BCHP_SCA_SC_UART_CMD_2_convention_MASK BCHP_SCB_SC_UART_CMD_2_convention_MASK
#define BCHP_SCA_SC_UART_CMD_2_rpar_retry_SHIFT BCHP_SCB_SC_UART_CMD_2_rpar_retry_SHIFT
#define BCHP_SCA_SC_PROTO_CMD_edc_en_MASK  BCHP_SCB_SC_PROTO_CMD_edc_en_MASK
#define BCHP_SCA_SC_PROTO_CMD_crc_lrc_MASK BCHP_SCB_SC_PROTO_CMD_crc_lrc_MASK
#define BCHP_SCA_SC_FLOW_CMD_flow_en_MASK  BCHP_SCB_SC_FLOW_CMD_flow_en_MASK
#define BCHP_SCA_SC_IF_CMD_2_db_en_MASK  BCHP_SCB_SC_IF_CMD_2_db_en_MASK
#define BCHP_SCA_SC_IF_CMD_2_db_mask_MASK  BCHP_SCB_SC_IF_CMD_2_db_mask_MASK
#define BCHP_SCA_SC_IF_CMD_1_vcc_MASK BCHP_SCB_SC_IF_CMD_1_vcc_MASK
#define BCHP_SCA_SC_IF_CMD_1_rst_MASK BCHP_SCB_SC_IF_CMD_1_rst_MASK 
#define BCHP_SCA_SC_IF_CMD_1_io_MASK BCHP_SCB_SC_IF_CMD_1_io_MASK 
#define BCHP_SCA_SC_PROTO_CMD_rbuf_rst_MASK  BCHP_SCB_SC_PROTO_CMD_rbuf_rst_MASK
#define BCHP_SCA_SC_PROTO_CMD_tbuf_rst_MASK  BCHP_SCB_SC_PROTO_CMD_tbuf_rst_MASK 
#define BCHP_SCA_SC_UART_CMD_1_io_en_MASK  BCHP_SCB_SC_UART_CMD_1_io_en_MASK 
#define BCHP_SCA_SC_IF_CMD_1_auto_clk_MASK BCHP_SCB_SC_IF_CMD_1_auto_clk_MASK
#define BCHP_SCA_SC_IF_CMD_1_auto_io_MASK BCHP_SCB_SC_IF_CMD_1_auto_io_MASK
#define BCHP_SCA_SC_UART_CMD_1_uart_rst_MASK BCHP_SCB_SC_UART_CMD_1_uart_rst_MASK
#define BCHP_SCA_SC_TIMER_CMD_timer_en_MASK BCHP_SCB_SC_TIMER_CMD_timer_en_MASK
#define BCHP_SCA_SC_INTR_STAT_1_timer_intr_MASK BCHP_SCB_SC_INTR_STAT_1_timer_intr_MASK
#define BCHP_SCA_SC_TIMER_CMD_timer_src_MASK BCHP_SCB_SC_TIMER_CMD_timer_src_MASK
#define BCHP_SCA_SC_TIMER_CMD_timer_mode_MASK BCHP_SCB_SC_TIMER_CMD_timer_mode_MASK
#define BCHP_SCA_SC_TIMER_CMD_wait_mode_MASK BCHP_SCB_SC_TIMER_CMD_wait_mode_MASK
#define BCHP_SCA_SC_TIMER_CMD_wait_en_MASK BCHP_SCB_SC_TIMER_CMD_wait_en_MASK
#define BCHP_SCA_SC_INTR_STAT_2_wait_intr_MASK BCHP_SCB_SC_INTR_STAT_2_wait_intr_MASK
#define BCHP_SCA_SC_INTR_STAT_1_tpar_intr_MASK BCHP_SCB_SC_INTR_STAT_1_tpar_intr_MASK
#define BCHP_SCA_SC_INTR_STAT_1_pres_intr_MASK  BCHP_SCB_SC_INTR_STAT_1_pres_intr_MASK
#define BCHP_SCA_SC_INTR_STAT_1_bgt_intr_MASK  BCHP_SCB_SC_INTR_STAT_1_bgt_intr_MASK
#define BCHP_SCA_SC_INTR_STAT_1_tdone_intr_MASK  BCHP_SCB_SC_INTR_STAT_1_tdone_intr_MASK
#define BCHP_SCA_SC_INTR_STAT_1_retry_intr_MASK  BCHP_SCB_SC_INTR_STAT_1_retry_intr_MASK
#define BCHP_SCA_SC_INTR_STAT_1_tempty_intr_MASK  BCHP_SCB_SC_INTR_STAT_1_tempty_intr_MASK
#define BCHP_SCA_SC_INTR_STAT_2_rpar_intr_MASK  BCHP_SCB_SC_INTR_STAT_2_rpar_intr_MASK
#define BCHP_SCA_SC_INTR_STAT_2_atrs_intr_MASK  BCHP_SCB_SC_INTR_STAT_2_atrs_intr_MASK
#define BCHP_SCA_SC_INTR_STAT_2_cwt_intr_MASK  BCHP_SCB_SC_INTR_STAT_2_cwt_intr_MASK
#define BCHP_SCA_SC_INTR_STAT_2_rcv_intr_MASK  BCHP_SCB_SC_INTR_STAT_2_rcv_intr_MASK
#define BCHP_SCA_SC_INTR_STAT_2_rready_intr_MASK  BCHP_SCB_SC_INTR_STAT_2_rready_intr_MASK
#define BCHP_SCA_SC_INTR_STAT_2_rlen_intr_MASK  BCHP_SCB_SC_INTR_STAT_2_rlen_intr_MASK
#define BCHP_SCA_SC_INTR_STAT_1_event1_intr_MASK  BCHP_SCB_SC_INTR_STAT_1_event1_intr_MASK
#define BCHP_SCA_SC_INTR_STAT_2_event2_intr_MASK  BCHP_SCB_SC_INTR_STAT_2_event2_intr_MASK
#define BCHP_SCA_SC_STATUS_2_rpar_err_MASK BCHP_SCB_SC_STATUS_2_rpar_err_MASK
#define BCHP_SCA_SC_STATUS_2_rempty_MASK BCHP_SCB_SC_STATUS_2_rempty_MASK
#define BCHP_SCA_SC_STATUS_1_tdone_MASK BCHP_SCB_SC_STATUS_1_tdone_MASK
#define BCHP_SCA_SC_STATUS_2_roverflow_MASK BCHP_SCB_SC_STATUS_2_roverflow_MASK
#define BCHP_SCA_SC_STATUS_2_rready_MASK BCHP_SCB_SC_STATUS_2_rready_MASK
#define BCHP_SCA_SC_IF_CMD_1_auto_vcc_MASK BCHP_SCB_SC_IF_CMD_1_auto_vcc_MASK
#define BCHP_SCA_SC_UART_CMD_1_get_atr_MASK BCHP_SCB_SC_UART_CMD_1_get_atr_MASK
#define BCHP_SCA_SC_IF_CMD_1_auto_rst_MASK BCHP_SCB_SC_IF_CMD_1_auto_rst_MASK
#define BCHP_SCA_SC_TIMER_CMD_cwt_en_MASK BCHP_SCB_SC_TIMER_CMD_cwt_en_MASK
#define BCHP_SCA_SC_STATUS_2_edc_err_MASK  BCHP_SCB_SC_STATUS_2_edc_err_MASK
#define BCHP_SCA_SC_INTR_EN_1_pres_ien_MASK BCHP_SCB_SC_INTR_EN_1_pres_ien_MASK
#define BCHP_SCA_SC_INTR_EN_1_tpar_ien_MASK BCHP_SCB_SC_INTR_EN_1_tpar_ien_MASK
#define BCHP_SCA_SC_INTR_EN_1_timer_ien_MASK BCHP_SCB_SC_INTR_EN_1_timer_ien_MASK
#define BCHP_SCA_SC_INTR_EN_1_bgt_ien7_MASK BCHP_SCB_SC_INTR_EN_1_bgt_ien7_MASK
#define BCHP_SCA_SC_INTR_EN_1_tdone_ien_MASK BCHP_SCB_SC_INTR_EN_1_tdone_ien_MASK
#define BCHP_SCA_SC_INTR_EN_1_retry_ien_MASK BCHP_SCB_SC_INTR_EN_1_retry_ien_MASK
#define BCHP_SCA_SC_INTR_EN_1_tempty_ien_MASK BCHP_SCB_SC_INTR_EN_1_tempty_ien_MASK
#define BCHP_SCA_SC_INTR_EN_2_rpar_ien_MASK BCHP_SCB_SC_INTR_EN_2_rpar_ien_MASK
#define BCHP_SCA_SC_INTR_EN_2_atrs_ien_MASK BCHP_SCB_SC_INTR_EN_2_atrs_ien_MASK
#define BCHP_SCA_SC_INTR_EN_2_cwt_ien_MASK BCHP_SCB_SC_INTR_EN_2_cwt_ien_MASK
#define BCHP_SCA_SC_INTR_EN_2_rlen_ien_MASK BCHP_SCB_SC_INTR_EN_2_rlen_ien_MASK
#define BCHP_SCA_SC_INTR_EN_2_wait_ien_MASK BCHP_SCB_SC_INTR_EN_2_wait_ien_MASK
#define BCHP_SCA_SC_INTR_EN_2_rcv_ien_MASK BCHP_SCB_SC_INTR_EN_2_rcv_ien_MASK
#define BCHP_SCA_SC_INTR_EN_2_rready_ien_MASK BCHP_SCB_SC_INTR_EN_2_rready_ien_MASK
#define BCHP_SCA_SC_UART_CMD_1_t_r_MASK BCHP_SCB_SC_UART_CMD_1_t_r_MASK
#define BCHP_SCA_SC_UART_CMD_1_auto_rcv_MASK BCHP_SCB_SC_UART_CMD_1_auto_rcv_MASK
#define BCHP_SCA_SC_UART_CMD_1_xmit_go_MASK BCHP_SCB_SC_UART_CMD_1_xmit_go_MASK
#define BCHP_SCA_SC_BGT_r2t_MASK BCHP_SCB_SC_BGT_r2t_MASK
#define BCHP_SCA_SC_UART_CMD_1_inv_par_MASK BCHP_SCB_SC_UART_CMD_1_inv_par_MASK
#endif	

/* End of Definitions */ 


/* Enum Types */





/* End of Enum Types */


/* Smart Card Private Data Structures */

/***************************************************************************
Summary:
Smart card event handle.

Description:
Upon receiving an interrupt, ISR shall signal this event to wake up the thread/task
that is waiting for this event.

See Also:
BSCD_ChannelSettings, BSCD_Open()

****************************************************************************/   
typedef struct BSCD_P_WaitEvent {

    BKNI_EventHandle cardWait;  /* card detection */
    BKNI_EventHandle tdoneWait;  /* transmit done */
    BKNI_EventHandle rcvWait;     /*receive at least one byte */
    BKNI_EventHandle atrStart;   /* receive start bit of the ATR */
    BKNI_EventHandle timerWait;   /* timer expired */
    BKNI_EventHandle event1Wait;   /* timer expired */
    BKNI_EventHandle event2Wait;   /* timer expired */	

} BSCD_P_WaitEvent;


/***************************************************************************
Summary:
Structure that defines Smart card Baud Rate and Clock divisor.

Description:
Smart card D and F Table according to SC_PRESCALE Table, where F is Clock Rate 
Conversion Factor and D is the Baud Rate Adjustment Factor.
This table contains the recommended set of values for the SC_PRESCALE and SC_CLK_CMD 
based  on the ATR TA1 byte. The resulting clock rate and baud rate will comply to 
ISO 7816-3.

See Also:
BSCD_OpenChannel()

****************************************************************************/ 
typedef struct
{
      uint8_t clkDiv;      /* clk divider in SC_CLK_CMD register */
      uint8_t prescale;    /* prescale in SC_PRESCALE register */
      uint8_t baudDiv;     /* baud rate divider in SC_CLK_CMD register */
      uint8_t etuClkDiv;   /* etu clk divider in SC_CLK_CMD register */
} BSCD_P_DFStruct;


/***************************************************************************
Summary:
Structure that defines Smart card calback function.

Description:
Structure that defines Smart card calback function.

See Also:


****************************************************************************/ 
typedef struct BSCD_P_IsrCallBackFunc
{
        /* CBFunc[0] is always for default CB function.  CBFunc[1] could be used for customized CB function */
	BSCD_IsrCallbackFunc	tParityIsrCBFunc[BSCD_MAX_NUM_CALLBACK_FUNC];             /* Interrupt Callback Function for Transmit Parity */
	BSCD_IsrCallbackFunc	timerIsrCBFunc[BSCD_MAX_NUM_CALLBACK_FUNC];               /* Interrupt Callback Function for eneral Purpose Timer */
	BSCD_IsrCallbackFunc  	cardInsertIsrCBFunc[BSCD_MAX_NUM_CALLBACK_FUNC];          /* Interrupt Callback Function for Card Insertion */
	BSCD_IsrCallbackFunc  	cardRemoveIsrCBFunc[BSCD_MAX_NUM_CALLBACK_FUNC];          /* Interrupt Callback Function for Card Removal */
	BSCD_IsrCallbackFunc  	bgtIsrCBFunc[BSCD_MAX_NUM_CALLBACK_FUNC];                 /* Interrupt Callback Function for Block Guard Time */
	BSCD_IsrCallbackFunc  	tDoneIsrCBFunc[BSCD_MAX_NUM_CALLBACK_FUNC];               /* Interrupt Callback Function for Transmit Done */
	BSCD_IsrCallbackFunc  	retryIsrCBFunc[BSCD_MAX_NUM_CALLBACK_FUNC];               /* Interrupt Callback Function for Transmit or Receive Retry Overflow */
	BSCD_IsrCallbackFunc  	tEmptyIsrCBFunc[BSCD_MAX_NUM_CALLBACK_FUNC];              /* Interrupt Callback Function for Transmit Empty */
	BSCD_IsrCallbackFunc  	rParityIsrCBFunc[BSCD_MAX_NUM_CALLBACK_FUNC];             /* Interrupt Callback Function for Receive Parity */
	BSCD_IsrCallbackFunc  	atrIsrCBFunc[BSCD_MAX_NUM_CALLBACK_FUNC];                 /* Interrupt Callback Function for ATR Start */
	BSCD_IsrCallbackFunc  	cwtIsrCBFunc[BSCD_MAX_NUM_CALLBACK_FUNC];                 /* Interrupt Callback Function for CWT */
	BSCD_IsrCallbackFunc  	rLenIsrCBFunc[BSCD_MAX_NUM_CALLBACK_FUNC];                /* Interrupt Callback Function for Receive Length Error */
	BSCD_IsrCallbackFunc  	waitIsrCBFunc[BSCD_MAX_NUM_CALLBACK_FUNC];                /* Interrupt Callback Function for Block or Work Waiting Time */
	BSCD_IsrCallbackFunc  	rcvIsrCBFunc[BSCD_MAX_NUM_CALLBACK_FUNC];                 /* Interrupt Callback Function for Receive Character */
	BSCD_IsrCallbackFunc  	rReadyIsrCBFunc[BSCD_MAX_NUM_CALLBACK_FUNC];              /* Interrupt Callback Function for Receive Ready */
	BSCD_IsrCallbackFunc  	edcIsrCBFunc[BSCD_MAX_NUM_CALLBACK_FUNC];                 /* Callback Function for EDC Error */
	BSCD_IsrCallbackFunc  	event1IsrCBFunc[BSCD_MAX_NUM_CALLBACK_FUNC];                 /* Callback Function for Event1 inerrupt */
	BSCD_IsrCallbackFunc  	event2IsrCBFunc[BSCD_MAX_NUM_CALLBACK_FUNC];                 /* Callback Function for Event2 inerrupt */
	
} BSCD_P_IsrCallBackFunc;

/***************************************************************************
Summary:
Structure that defines Smart card channel  handle.

Description:
Structure that defines Smart card channel  handle.

See Also:
BSCD_OpenChannel()

****************************************************************************/  
typedef struct BSCD_P_ChannelHandle
{
	unsigned long			ulMagicNumber; /* Must be  BSCD_P_CHANNEL_HANDLE_MAGIC_NUMBER */

	BSCD_Handle    		moduleHandle;   /* Module handle */

	BSCD_ChannelSettings	currentChannelSettings;   /* current channel settings */

	BINT_CallbackHandle	channelIntCallback;  /* Channel Interrupt Callback */

	unsigned char			ucChannelNumber;     /* channel number */

	uint32_t				ulStatus1;                      /* value of SC_STATUS_1 */

	uint32_t				ulStatus2;                      /* value of SC_STATUS_2 */

	uint32_t				ulIntrStatus1;                      /* value of SC_INTR_STATUS_1 */

	uint32_t				ulIntrStatus2;                      /* value of SC_INTR_STATUS_2 */

	BSCD_P_WaitEvent		channelWaitEvent;    /* Wait event */

	BSCD_ResetType		resetType;			/* Need this for EMV ATR to determine if this is warmReset or coldReset */

	BSCD_Status			channelStatus;         /* Channel status that returned by BSCD_Channel_GetStatus */

	bool					bIsOpen;    /* Is channel opened */

	unsigned char			aucTxBuf[BSCD_MAX_TX_SIZE];   /* transmission bytes */

	unsigned long			ulTxLen;                       /* number of transmission bytes */

	unsigned char			aucRxBuf[BSCD_MAX_RX_SIZE]; /*receiving bytes */

	unsigned long			ulRxLen;                   /* number receiving bytes */

	unsigned long			ulRegStartAddr;                   /* Start Address of this smart card  channel */

	BSCD_P_IsrCallBackFunc	callBack;			/* interrupt Callback functions */

#ifdef BSCD_EMV2000_CWT_PLUS_4
	bool					bIsReceive;    /* Is channel opened */
#endif

	bool					bIsCardRemoved;    /* Is the Card removed ? */
				
} BSCD_P_ChannelHandle;


/***************************************************************************
Summary:
Structure that defines Smart card module  handle.

Description:
Structure that defines Smart card module  handle.

See Also:
BSCD_Open()

****************************************************************************/  
 typedef struct BSCD_P_Handle
{
	unsigned long			ulMagicNumber; /* Must be  BSCD_P_HANDLE_MAGIC_NUMBER */

	BSCD_ChannelHandle	channelHandles[BSCD_MAX_SUPPOTED_CHANNELS];

	BSCD_Settings			currentSettings;   /* current settings */

	BREG_Handle			regHandle;    /* register handle */
	BCHP_Handle			chipHandle;  /* chip handle */
	BINT_Handle			interruptHandle;   /* interrupt handle */

	bool					bIsOpen;    /* Is Module opened */

	uint16_t 		chipId;
	uint16_t 		chipRev;		

} BSCD_P_Handle;






/* End of Smart Card Private Data Structures */


/* Private Function */

unsigned char BSCD_P_GetClkDiv(
	BSCD_ChannelHandle	in_channelHandle, 
      unsigned char in_ucDFactor,
      unsigned char in_ucFFactor
);

unsigned char BSCD_P_GetETUClkDiv(
	BSCD_ChannelHandle	in_channelHandle, 
      unsigned char in_ucDFactor,
      unsigned char in_ucFFactor
);

unsigned char BSCD_P_GetISOBaudRateAdjustor(
      unsigned char in_ucDFactor
);

unsigned int BSCD_P_GetISOClockRateConversionFactor(
      unsigned char in_ucFFactor
);

unsigned char BSCD_P_MapScClkDivToMaskValue(
      unsigned char in_ucClkDiv
);

unsigned char BSCD_P_GetPrescale(
	BSCD_ChannelHandle	in_channelHandle, 
      unsigned char in_ucDFactor,
      unsigned char in_ucFFactor
);

unsigned char BSCD_P_GetBaudDiv(
	BSCD_ChannelHandle	in_channelHandle, 
      unsigned char in_ucDFactor,
      unsigned char in_ucFFactor
);

BERR_Code BSCD_P_FDAdjust(
		BSCD_ChannelHandle	in_channelHandle, 
		unsigned char 			in_ucFFactor, 
		unsigned char			in_ucDFactor
);

BERR_Code BSCD_P_AdjustWWT(
      		BSCD_ChannelHandle	in_channelHandle, 
		unsigned char 			in_ucFFactor, 
		unsigned char 			in_ucDFactor,
		unsigned char			in_ucWorkWaitTImeInteger
);

void BSCD_Channel_P_CardInsertCB_isr( 
      BSCD_ChannelHandle	in_channelHandle, 
      void       				*inp_data 
) ;


void  BSCD_Channel_P_CardRemoveCB_isr( 
      BSCD_ChannelHandle	in_channelHandle, 
      void       				*inp_data  
) ;

void BSCD_Channel_P_RcvCB_isr( 
      BSCD_ChannelHandle	in_channelHandle, 
      void       				*inp_data  
);

void BSCD_Channel_P_ATRCB_isr( 
      BSCD_ChannelHandle	in_channelHandle, 
      void       				*inp_data  
);

void BSCD_Channel_P_WaitCB_isr( 
      BSCD_ChannelHandle	in_channelHandle, 
      void       *inp_data 
) ;

void BSCD_Channel_P_TimerCB_isr( 
      BSCD_ChannelHandle	in_channelHandle, 
      void       *inp_data 
) ;

void BSCD_Channel_P_RetryCB_isr( 
      BSCD_ChannelHandle	in_channelHandle, 
      void       *inp_data 
);

void BSCD_Channel_P_TimerCB_isr( 
      BSCD_ChannelHandle	in_channelHandle,
      void       *inp_data 
);

void BSCD_Channel_P_RParityCB_isr( 
      BSCD_ChannelHandle	in_channelHandle,
      void       *inp_data 
);

void BSCD_Channel_P_TParityCB_isr(
      BSCD_ChannelHandle	in_channelHandle,
      void       *inp_data 
);

void BSCD_Channel_P_CWTCB_isr( 
      BSCD_ChannelHandle	in_channelHandle,
      void       *inp_data 
);

void BSCD_Channel_P_BGTCB_isr( 
      BSCD_ChannelHandle	in_channelHandle,
      void       *inp_data 
);

void BSCD_Channel_P_RLenCB_isr( 
      BSCD_ChannelHandle	in_channelHandle,
      void       *inp_data 
);

void BSCD_Channel_P_RReadyCB_isr( 
      BSCD_ChannelHandle	in_channelHandle,
      void       *inp_data 
);

void BSCD_Channel_P_TDoneCB_isr( 
      BSCD_ChannelHandle	in_channelHandle,
      void       *inp_data 
);

#ifdef BSCD_EMV2000_CWT_PLUS_4_EVENT_INTR
void BSCD_Channel_P_Event1CB_isr( 
      BSCD_ChannelHandle	in_channelHandle,
      void       *inp_data 
);
#endif

void BSCD_Channel_P_Event2CB_isr( 
      BSCD_ChannelHandle	in_channelHandle,
      void       *inp_data 
);

BERR_Code BSCD_Channel_P_WaitForCardInsertion(
		BSCD_ChannelHandle	in_channelHandle
);

BERR_Code BSCD_Channel_P_WaitForCardRemove(
		BSCD_ChannelHandle	in_channelHandle
);

BERR_Code BSCD_Channel_P_WaitForTimerEvent(
		BSCD_ChannelHandle	in_channelHandle
);

BERR_Code BSCD_Channel_P_WaitForATRStart(
		BSCD_ChannelHandle	in_channelHandle
);

BERR_Code BSCD_Channel_P_WaitForTDone(
		BSCD_ChannelHandle	in_channelHandle
);


BERR_Code BSCD_Channel_P_WaitForRcv(
		BSCD_ChannelHandle	in_channelHandle
);

BERR_Code BSCD_Channel_P_WaitForRReady(
		BSCD_ChannelHandle	in_channelHandle
);


BERR_Code BSCD_Channel_P_Activating(
		BSCD_ChannelHandle	in_channelHandle
);


void BSCD_P_HexDump(
      char          *inp_cTitle, 
      unsigned char *inp_ucBuf, 
      unsigned int  in_unLen
);

BERR_Code BSCD_Channel_P_T0ReadData(
		BSCD_ChannelHandle       in_channelHandle,
		uint8_t                  		*outp_ucRcvData,
		unsigned long                 *outp_unNumRcvBytes,
		unsigned long                 in_unMaxReadBytes
);

BERR_Code BSCD_Channel_P_T1ReadData(
		BSCD_ChannelHandle       in_channelHandle,
		uint8_t                  *outp_ucRcvData,
		unsigned long                  *outp_ulNumRcvBytes,
		unsigned long                  in_ulMaxReadBytes 
);

BERR_Code BSCD_Channel_P_ByteRead(
		BSCD_ChannelHandle	in_channelHandle,
		unsigned char *outp_ucData		
);

BERR_Code BSCD_Channel_P_EnableInterrupts_isr( 
		BSCD_ChannelHandle	in_channelHandle
);

void BSCD_Channel_P_IntHandler_isr(
	void *inp_param1,		/* Device channel handle */
	int in_param2				/* reserved */

);

BERR_Code BSCD_Channel_P_T0T1Transmit(		
		BSCD_ChannelHandle          in_channelHandle,
		uint8_t                     *inp_ucXmitData,
		unsigned long                    in_ulNumXmitBytes
);

BERR_Code BSCD_Channel_P_T14IrdetoTransmit(		
		BSCD_ChannelHandle          in_channelHandle,
		uint8_t                     *inp_ucXmitData,
		unsigned long                    in_ulNumXmitBytes
);


/*****************************************************************************
Summary:
This function creates the events that associated with the channel.

Description:
This function creates the events that associated with the channel.

Calling Context:
The function shall be called in BSCD_ChannelOpen .

Performance and Timing:
This is a synchronous function that will return when it is done. 

Input:
in_channelHandle  - BSCD_ChannelHandle, a smart card 
					channel handle.

Returns:
BERR_SUCCESS - success
To Do: Need more error code


See Also:


******************************************************************************/
BERR_Code BSCD_P_CreateWaitEvent(
      BSCD_ChannelHandle in_channelHandle
);

/*****************************************************************************
Summary:
This function destroys the events that associated with the channel.

Description:
This function destroys the events that associated with the channel.

Calling Context:
The function shall be called in BSCD_ChannelClose .

Performance and Timing:
This is a synchronous function that will return when it is done. 

Input:
in_channelHandle  - BSCD_ChannelHandle, a smart card 
					channel handle.

Returns:
BERR_SUCCESS - success
To Do: Need more error code


See Also:


******************************************************************************/
BERR_Code BSCD_P_DestroyWaitEvent(
      BSCD_ChannelHandle in_channelHandle
);

/*****************************************************************************
Summary:
This function control the state transition of smart card channel.

Description:
This function control the state transition of smart card channel.

Calling Context:


Performance and Timing:
This is a synchronous function that will return when it is done. 

Input:
in_event - BSCD_P_Event, state transition event.
      
Input/Output:
in_channelHandle  - BSCD_ChannelHandle, a smart card 
					channel handle.

Returns:
BERR_SUCCESS - success
To Do: Need more error code


See Also:


******************************************************************************/


BERR_Code BSCD_Channel_P_ReceiveAndDecode(
		BSCD_ChannelHandle	in_channelHandle
);

BERR_Code BSCD_Channel_P_SetStandard(
		BSCD_ChannelHandle	in_channelHandle,
		const BSCD_ChannelSettings	*inp_sSettings
);


BERR_Code BSCD_Channel_P_SetFreq(
		BSCD_ChannelHandle	in_channelHandle,
		const BSCD_ChannelSettings	*inp_sSettings
);

BERR_Code BSCD_Channel_P_SetWaitTime(
		BSCD_ChannelHandle	in_channelHandle,
		const BSCD_ChannelSettings	*inp_sSettings
);

BERR_Code BSCD_Channel_P_SetGuardTime(
		BSCD_ChannelHandle	in_channelHandle,
		const BSCD_ChannelSettings	*inp_sSettings
);

BERR_Code BSCD_Channel_P_SetTransactionTimeout(
		BSCD_ChannelHandle	in_channelHandle,
		const BSCD_ChannelSettings	*inp_sSettings
);

BERR_Code BSCD_Channel_P_SetEdcParity(
		BSCD_ChannelHandle	in_channelHandle,
		const BSCD_ChannelSettings	*inp_sSettings
);

/* End of Private Function */

#ifdef __cplusplus
}
#endif

#endif /* BSCD_PRIV_H__ */
