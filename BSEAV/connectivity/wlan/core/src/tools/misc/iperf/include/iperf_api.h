/*
 * Entry points for iPerf application for platforms where the iPerf utility
 * is not a standalone application.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 * $Id$
 */


#ifndef iperf_api_h
#define iperf_api_h

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Include Files ---------------------------------------------------- */
/* ---- Constants and Types ---------------------------------------------- */
/* ---- Variable Externs ------------------------------------------------- */
/* ---- Function Prototypes ---------------------------------------------- */


/****************************************************************************
* Function:   iperf_main_args
*
* Purpose:    Entry point for iPerf utility application. Uses same prototype
*             as standard main() functions - argc and argv parameters.
*
* Parameters: argc (in) Number of 'argv' arguments.
*             argv (in) Array of command-line arguments.
*
* Returns:    0 on success, else error code.
*****************************************************************************
*/
int iperf_main_args(int argc, char **argv);


/****************************************************************************
* Function:   iperf_main_str
*
* Purpose:    Alternate entry point for iPerf utility application. This function
*             will tokenize the input command line string, before calling
*             iperf_main_args() to process the command line arguments.
*
* Parameters: str (mod) Input command line string. Contents will be modified!
*
* Returns:    0 on success, else error code.
*****************************************************************************
*/
int iperf_main_str(char *str);


#ifdef __cplusplus
	}
#endif

#endif  /* iperf_api_h  */
