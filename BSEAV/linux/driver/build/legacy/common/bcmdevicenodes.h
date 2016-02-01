/***************************************************************************
 *     Copyright (c) 2002-2006, Broadcom Corporation
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
 ********************************************************************/
#ifndef BCMDEVICENODES_H__
#define BCMDEVICENODES_H__

/**
* Backend minor numbers
**/
enum
{
	eBRCM_GFX_0		     	= 0,
	eBRCM_GFX_1				= 1,
	eBRCM_RECORD_0        	= 2,
	eBRCM_RECORD_1        	= 3,
	eBRCM_RECORD_2        	= 4,
	eBRCM_INDEX_0       	= 5,
	eBRCM_INDEX_1       	= 6,
	eBRCM_INDEX_2       	= 7,
	eBRCM_PLAYBACK_0      	= 8,
	eBRCM_PLAYBACK_1      	= 9,
	eBRCM_PLAYBACK_2      	= 10,
	eBRCM_PLAYBACK_3      	= 11,
	eBRCM_PCM_0 			= 12,
	eBRCM_PCM_1 			= 13,
	eBRCM_PCMCAP_0 			= 14,
	eBRCM_PCMCAP_1 			= 15,
	eBRCM_TRANSMSG			= 18,
	eBRCM_TRANSPORT     	= 19,
	eBRCM_DMA      			= 20,
	eBRCM_DEBUG      		= 21,
	eBRCM_PLAY_ES_0      	= 24,
	eBRCM_PLAY_ES_1      	= 25,
	eBRCM_PLAY_ES_2      	= 26,
	eBRCM_PLAY_ES_3      	= 27,
	eBRCM_PCMSAMPLE_0     	= 28,
	eBRCM_PCMSAMPLE_1     	= 29,
	eBRCM_VBI				= 30,
	eBRCM_OUTPUT_0	     	= 33,
	eBRCM_OUTPUT_1			= 34,
	eBRCM_IO				= 35,
	eBRCM_PAXEL_0	     	= 36,
	eBRCM_ICAM_FILTER		= 55,
	eBRCM_ICAM				= 56,
	eBRCM_RECORD_3        	= 60,
	eBRCM_INDEX_3       	= 61,
};

/**
* Frontend minor numbers
**/
enum
{
    eBRCM_LEDS				= 0,
    eBRCM_IRBLASTER			= 2,
    eBRCM_UARTA				= 4,
    eBRCM_UARTB				= 5,
    eBRCM_QAMGLOBAL			= 6,
    eBRCM_QAM_DOWN_STREAM_A	= 7,
    eBRCM_QAM_DOWN_STREAM_B	= 8,
    eBRCM_QAM_OUT_OF_BAND	= 9,
    eBRCM_QAM_UP_STREAM		= 10,
	eBRCM_CHANMGR 			= 11,
	eBRCM_UPGGPIO			= 12,
	eBCM_FE_DEBUG			= 13,

    eBRCM_SEJINKBD_INPUT	= 3,
	eBRCM_REMA_INPUT		= 14,
    eBRCM_REMB_INPUT		= 15,
    eBRCM_SEJINREMOTE_INPUT	= 16,
    eBRCM_KEYPAD_INPUT		= 1,

	eBRCM_SMARTCARD_A       = 20,
	eBRCM_SMARTCARD_B       = 21,
    eBRCM_UPGI2C            = 22          


};

#endif /* BCMDEVICENODES_H__ */
