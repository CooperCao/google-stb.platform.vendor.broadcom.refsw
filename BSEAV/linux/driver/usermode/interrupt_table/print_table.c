/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/
#include "bstd.h"
#include "bkni_multi.h"
#include "bint_plat.h"
#include <stdio.h>
#include <string.h>

#define BINT_GETSETTINGS_PROTOTYPE_P(CHIP) BINT_##CHIP##_GetSettings
#define BINT_GETSETTINGS_PROTOTYPE(CHIP) BINT_GETSETTINGS_PROTOTYPE_P(CHIP)
#define BINT_GETSETTINGS() BINT_GETSETTINGS_PROTOTYPE(BCHP_CHIP)()
const BINT_Settings *BINT_GETSETTINGS_PROTOTYPE(BCHP_CHIP)( void );

#define MAX_INTERRUPTS  256
struct intr_entry {
    const char *name;
    bool valid;
};

void print_table(FILE *fout, const struct intr_entry *table, unsigned table_len, bool webcpu)
{
    unsigned i;
    fprintf(fout, "static s_InteruptTable interruptName[] = {\n");
    fprintf(fout, "\t{NULL, 0},                   /* Linux IRQ #'s are offset by 1 */\n");
    for(i=0;i<table_len;i++) {
        char name[64];
        bool valid = table[i].valid;
#if defined(NEXUS_GISB_ARB)
        if (table[i].name && !strcmp(table[i].name, "SYS")) continue;
#endif
        if(valid && webcpu) {
            valid = table[i].name && strstr(table[i].name,"M2MC1")==table[i].name;
        }
        if(table[i].name && *table[i].name) {
            snprintf(name, sizeof(name), "\"%s\",", table[i].name);
        } else {
            snprintf(name, sizeof(name), "NULL,");
        }
        fprintf(fout, "\t{%-30s%s", name, valid ? "INT_ENABLE_MASK" : "0");
        if(i && i%32==0) {
            fprintf(fout, " | INT_REG_WRAP");
        }
        fprintf(fout, " }, /* %u (%u) */\n", i%32, i+1);
    }
    fprintf(fout, "};\n");
    return;
}

int main(int argc, const char *argv[])
{
    const BINT_Settings *settings;
    unsigned i;
    struct intr_entry table[MAX_INTERRUPTS];
    unsigned max_interrupt=0;
    FILE *fout;
    FILE *fin;

    if(argc!=3) {
        printf("Usage:\n%s <template> <destination\n", argv[0]);
        return 1;
    }
    fin = fopen(argv[1],"r");
    if(!fin) {
        perror(argv[1]);
        return 1;
    }
    fout = fopen(argv[2],"w");
    if(!fout) {
        perror(argv[2]);
        return 1;
    }
    fprintf(fout,"/* THIS FILE IS AUTO GENERATED. DO NOT EDIT\n */");
    for(;;) {
        char buf[256];
        const char *line = fgets(buf, sizeof(buf), fin);
        if(line==NULL) {
            break;
        }
        fputs(buf, fout);
    }
    fprintf(fout,"/* THIS FILE IS AUTO GENERATED. DO NOT EDIT\n */\n");
    fclose(fin);

    BKNI_Init();
    BDBG_Init();
    memset(table, 0, sizeof(table));
    settings = BINT_GETSETTINGS();
    for(i=0;settings->pIntMap[i].L1Shift>=0;i++) {
        const BINT_P_IntMap *map = settings->pIntMap+i;
        unsigned shift = BINT_MAP_GET_L1SHIFT(map);
        BDBG_ASSERT(shift<MAX_INTERRUPTS);
        table[shift].valid = true;
        table[shift].name = map->L2Name;
        if(shift>max_interrupt) {
            max_interrupt = shift;
        }
    }
    max_interrupt = max_interrupt+31;
    max_interrupt -= max_interrupt % 32;
    fprintf(fout, "#define NUM_INTC_WORDS %u\n", max_interrupt/32);
    fprintf(fout, "#ifndef NEXUS_WEBCPU\n");
    print_table(fout, table, max_interrupt, false);
    fprintf(fout, "#else\n");
    fprintf(fout, "/* NEXUS_WEBCPU client */\n");
    print_table(fout, table, max_interrupt, true);
    fprintf(fout, "#endif\n");
    fprintf(fout, "s_ChipConfig  g_sChipConfig ={ \"%s\",interruptName, 0x0 /* intcAddr, unused */ , sizeof(interruptName)/sizeof(interruptName[0]),NUM_INTC_WORDS};\n",settings->name);
    fclose(fout);
#if 0
    BDBG_Uninit();
    BKNI_Uninit();
#endif
    return 0;
}
