/***************************************************************************
 *     Copyright (c) 2006-2007, Broadcom Corporation
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
 * Video Streaming Driver for BCM7401
 * This is a application stub for launching all recorders and streamers
 *  
 * Revision History:
 *    
 * $brcm_Log: $
 * 
 *************************************************************/ 

void launch_all_streamers(int max_streamers, int chmap_servers);

int main(int argc, char **argv){
	launch_all_streamers(5,5);
}


