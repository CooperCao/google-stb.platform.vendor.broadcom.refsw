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
 * Module Description:  Generic prompt for Diagnostics S/W.
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#if __cplusplus
extern "C" {
#endif

typedef void (*BkgdPromptFunc)(
	void *p_generic						/* [in] generic pointer */
    );

extern char det_in_char(void);
extern char det_in_char_no_background(void);
extern int rdstr(char *s);
extern char GetChar(void);
extern int Prompt(void);
extern int NoPrompt(void);
extern char PromptChar(void);
extern char NoPromptChar(void);
extern int BkgdPrompt( BkgdPromptFunc p_BkgdFunc, void *p_generic );
extern char BkgdPromptChar( BkgdPromptFunc p_BkgdFunc, void *p_generic );

#if __cplusplus
}
#endif

