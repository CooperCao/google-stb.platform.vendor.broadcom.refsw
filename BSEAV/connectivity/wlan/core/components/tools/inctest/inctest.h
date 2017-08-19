/**
 * @file
 * @brief
 * header file for inctest
 *
 * $ Copyright Broadcom Corporation $
 *
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id$
 */

#define WLAWDL
#define WLMESH
#define BCMCCX
#define CONFIG_COMPAT
#define SIMPLE_ISCAN
#define EFI  /* for STATIC_ASSERT(sizeof(T4_t) == 12); */
#include <wlioctl.h>

extern char inc_test_arr[][100];
extern uint32 inc_test_arr_packed[];
extern uint32 inc_test_arr_unpacked[];

char * get_str_name(uint32 i);
uint32 get_arr_size();
