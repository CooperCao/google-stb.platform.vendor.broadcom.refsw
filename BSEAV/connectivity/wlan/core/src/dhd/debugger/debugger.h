/*
 * Copyright (C) 2013, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id$
 */

void debugger_init(void);
void *dbg_mdelay(int d);
void dbg_fputc(char c, void *handle);
int dbg_fgetc(void *handle);
char *dbg_fgets(char *s, void *handle);
void dbg_fputs(char *s, void *handle);
void dbg_reg_write(uint32_t addr, uint32_t val);
void dbg_fclose(void *handle);
uint32_t dbg_reg_read(uint32_t addr);

extern int sprintf (char *s, const char *format, ...);
extern int sscanf (const char *s, const char *format, ...);
extern char *strcpy (char *dest, const char *src);
extern int strlen (const char *s);
extern int strcmp (const char *s1, const char *s2);
extern void do_exit(long);


#define FPUTC	dbg_fputc
#define FGETC	dbg_fgetc
#define FPUTS	dbg_fputs
#define FGETS	dbg_fgets
#define FCLOSE	dbg_fclose
