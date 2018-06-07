/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/
#include "bstd.h"
#include "breg_mem.h"

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bkni.h"
#include "bchp.h"
#include "bchp_common.h"
#include "bdtu.h"
#include "bchp_memc_clients.h"
#include "../src/bdtu_map.inc" /* TEMP for BDTU_P_ReadMappingInfo */
#include "bchp_memc_gen_0.h"

BDBG_MODULE(dtutool);

static int g_fd;

#define _2MB (2*1024*1024)

enum ownerID {
    dtu_owner_eGisb_0 = 0, /* For MEMSYS >= rev B3.0 this is GISB.. for < B3.0 this is BSP */
    dtu_owner_eScpu_0,
    dtu_owner_eCpu_0 = 6,
    dtu_owner_eWebCpu_0,
    dtu_owner_eJtag_0,
    dtu_owner_eSsp_0,
    dtu_owner_eRdc_0,
    dtu_owner_eHvd_0,
    dtu_owner_eHvd_1,
    dtu_owner_eRaaga_0 = 17,
    dtu_owner_ePcie_0 = 19,
    dtu_owner_eBbsi_spi = 23,
    dtu_owner_eBsp_0 = 29 /* For MEMSYS >= rev B3.0 */
};

static char *ownerID_to_name(enum ownerID owner)
{
    switch(owner)
    {
        case dtu_owner_eGisb_0:return "gisb_0";
        case dtu_owner_eScpu_0:return "scpu_0";
        case dtu_owner_eCpu_0:return "cpu_0";
        case dtu_owner_eWebCpu_0:return "webcpu_0";
        case dtu_owner_eJtag_0:return "jtag_0";
        case dtu_owner_eSsp_0:return "ssp_0";
        case dtu_owner_eRdc_0:return "rdc_0";
        case dtu_owner_eHvd_0:return "hvd_0";
        case dtu_owner_eHvd_1:return "hvd_1";
        case dtu_owner_eRaaga_0:return "raaga_0";
        case dtu_owner_ePcie_0:return "pcie_0";
        case dtu_owner_eBbsi_spi:return "bbsi_spi";
        case dtu_owner_eBsp_0:return "bsp_0";
        default:
            break;
    }

    return "Reserved";
}

static BREG_Handle map_registers(void)
{
    void *addr;
    BREG_Handle reg;

    g_fd = open("/dev/mem", O_RDWR|O_SYNC);
    if (g_fd < 0) {
        BDBG_ERR(("Unable to open /dev/mem: %d", errno));
        return NULL;
    }

    /* map register space */
#define REGISTER_BASE   (BCHP_PHYSICAL_OFFSET + (BCHP_REGISTER_START & ~0xFFF))
#define REGISTER_SIZE   (BCHP_REGISTER_END - (BCHP_REGISTER_START & ~0xFFF))
    addr = mmap64(0, REGISTER_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, g_fd, REGISTER_BASE);
    if (addr == (void*)-1) {
        BDBG_ERR(("Unable to mmap registers: %d", errno));
        return NULL;
    }
    BREG_Open(&reg, addr, BCHP_REGISTER_END, NULL);

    return reg;
}

static void print_usage(void)
{
    printf(
    "dtutool [OPTIONS]\n"
    "\n"
    "-print           print dtu map summary\n"
    "-restore         restore identity map and reset cma\n"
    "-scrub ADDR,SIZE\n"
    "-show_owners     show page ownership\n"
    "-dump ADDR,SIZE\n"
    "-dall            dump ALL dtu pages.\n"
    );
}

enum maptype {
    maptype_unmapped,
    maptype_identity,
    maptype_remapped
};

static uint8_t addr_to_memc(BSTD_DeviceOffset addr, BDTU_CreateSettings settings[BCHP_P_MEMC_COUNT])
{
    uint8_t memc, region;
    BSTD_DeviceOffset end;

    for (memc=0; memc<BCHP_P_MEMC_COUNT; memc++) {
        for (region=0; region<BCHP_MAX_MEMC_REGIONS; region++) {
            if(!settings[memc].memoryLayout.region[region].size) {
                continue;
            }
            end = settings[memc].memoryLayout.region[region].addr + settings[memc].memoryLayout.region[region].size;
            end -= _2MB;

            if((addr >= settings[memc].memoryLayout.region[region].addr) && (addr <= end)) {
                goto FOUND;
            }
        }
    }

FOUND:

    if(memc>=BCHP_P_MEMC_COUNT) {
        BDBG_ERR(("Cannot determine memc. Defaulting to 0. May fail"));
        memc = 0;
    }

    return memc;
}

static void print_map(enum maptype maptype, uint64_t fromaddr, uint64_t toaddr)
{
    const char *maptype_str[] = {"unmapped", "identity", "remapped"};
    printf("%s " BDBG_UINT64_FMT "-" BDBG_UINT64_FMT "\n", maptype_str[maptype], BDBG_UINT64_ARG(fromaddr), BDBG_UINT64_ARG(toaddr));
}

int main(int argc, char **argv)
{
    BREG_Handle reg;
    int rc;
    BDTU_Handle dtu[BCHP_P_MEMC_COUNT];
    BDTU_CreateSettings settings[BCHP_P_MEMC_COUNT];
    int curarg = 1;
    enum {
        action_help,
        action_print,
        action_restore,
        action_scrub,
        action_show_owners,
        action_dump,
        action_dump_all
    } action = action_help;
    BSTD_DeviceOffset scrub_addr = 0;
    uint64_t scrub_size = 0;
    BDTU_Status status[BCHP_P_MEMC_COUNT];
    unsigned i, j;

    while (curarg < argc) {
        if (!strcmp(argv[curarg], "-h") || !strcmp(argv[curarg], "--help")) {
            action = action_help;
            break;
        }
        else if (!strcmp(argv[curarg], "-print")) {
            action = action_print;
        }
        else if (!strcmp(argv[curarg], "-restore")) {
            action = action_restore;
        }
        else if (!strcmp(argv[curarg], "-scrub") && curarg+1<argc) {
            char *s = strchr(argv[++curarg], ',');
            if (s) {
                *s++ = 0;
                scrub_addr = strtoull(argv[curarg], NULL, 0);
                scrub_size = strtoull(s, NULL, 0);
            }
            if (!scrub_addr || !scrub_size) {
                print_usage();
                return -1;
            }
            action = action_scrub;
        }
        else if (!strcmp(argv[curarg], "-dump") && curarg+1<argc) {
            char *s = strchr(argv[++curarg], ',');
            if (s) {
                *s++ = 0;
                scrub_addr = strtoull(argv[curarg], NULL, 0);
                scrub_size = strtoull(s, NULL, 0);
            }
            if (!scrub_size) {
                print_usage();
                return -1;
            }
            action = action_dump;
        }
        else if (!strcmp(argv[curarg], "-show_owners")) {
            action = action_show_owners;
        }
        else if (!strcmp(argv[curarg], "-dall")) {
            action = action_dump_all;
        }
        else {
            print_usage();
            return -1;
        }
        curarg++;
    }

    if (action == action_help) {
        print_usage();
        return 0;
    }

    BKNI_Init();
    BDBG_Init();
    reg = map_registers();

    for (i=0; i<BCHP_P_MEMC_COUNT; i++) {
        BDTU_GetDefaultCreateSettings(&settings[i]);
        settings[i].reg = reg;
        BDTU_P_ReadMappingInfo(reg, i, &settings[i].memoryLayout);
        rc = BDTU_Create(&dtu[i], &settings[i]);
        if (rc) {BERR_TRACE(rc); goto done;}

        rc = BDTU_GetStatus(dtu[i], &status[i]);
        if (rc) {BERR_TRACE(rc); goto done;}

        if (status[i].state != BDTU_State_eEnabled) {
            BDBG_ERR(("DTU[%d] not enabled: state %u", i, status[i].state));
        }
    }

    if (action == action_print) {
        uint64_t addr = 0, prev_addr = 0, org_addr;
        uint8_t memc;
        BDTU_MappingInfo mapping;

        for (memc=0; memc<BCHP_P_MEMC_COUNT; memc++) {
            mapping = settings[memc].memoryLayout;
            for (i=0; i<BCHP_MAX_MEMC_REGIONS; i++) {
                enum maptype prev_maptype = maptype_identity, maptype;
                addr = mapping.region[i].addr;
                prev_addr = mapping.region[i].addr;
                if (!mapping.region[i].size) {
                    continue;
                }
                for (;mapping.region[i].size;mapping.region[i].size-=_2MB) {
                    rc = BDTU_ReadOriginalAddress(dtu[memc], addr, &org_addr);

                    if (rc) {
                        maptype = maptype_unmapped;
                    }
                    else {
                        maptype = addr==org_addr?maptype_identity:maptype_remapped;
                    }
                    if (maptype != prev_maptype) {
                        if (prev_addr != addr) {
                            print_map(prev_maptype, prev_addr, addr);
                        }
                        prev_maptype = maptype;
                        prev_addr = addr;
                    }
                    addr += _2MB;
                }
                if (prev_addr != addr) {
                    print_map(prev_maptype, prev_addr, addr);
                }
            }
        }
    }
    else if (action == action_restore) {
        /* first, remap back to identity. */
        uint64_t addr = 0, org_addr;
        unsigned n = 0;
        unsigned failures = 0;
        uint8_t memc;
        BSTD_DeviceOffset end;

        rc = 0;
        for (memc=0; memc<BCHP_P_MEMC_COUNT; memc++) {
            for (i=0; i<BCHP_MAX_MEMC_REGIONS; i++) {
                addr = settings[memc].memoryLayout.region[i].addr;
                end = addr + settings[memc].memoryLayout.region[i].size;

                for (;addr<end;addr+=_2MB) {
                    rc = BDTU_ReadOriginalAddress(dtu[memc], addr, &org_addr);
                    if (!rc && addr != org_addr) {
                        BDTU_RemapSettings settings;
                        n++;
                        BDTU_GetDefaultRemapSettings(&settings);
                        settings.list[0].orgPhysAddr = org_addr;
                        settings.list[0].fromPhysAddr = addr;
                        settings.list[0].toPhysAddr = org_addr;
                        rc = BDTU_Remap(dtu[memc], &settings);
                        if (rc) { BDBG_ERR(("unable to restore ba " BDBG_UINT64_FMT, addr)); failures++;}
                    }
                }
            }
        }
        /* second, if no remap failures, reset cma. */
        if (!failures) {
            system("cmatool resetall");
            printf("restored %u superpage(s)\n", n);
        }
    }
    else if (action == action_scrub) {
        /* No checks on size here, BDTU call should fail in invalid */
        rc = BDTU_Scrub(dtu[addr_to_memc(scrub_addr, settings)], scrub_addr, scrub_size);
        if (rc) BERR_TRACE(rc);
    }
    else if (action == action_show_owners) {
        BDTU_PageInfo info;
        BSTD_DeviceOffset addr = 0;
        BSTD_DeviceOffset start = ~0;
        enum ownedState {eOS_unknown, eOS_owned, eOS_notOwned} owned=eOS_unknown, lastOwned=eOS_unknown;
        enum validState {eVS_unknown, eVS_valid, eVS_invalid} valid=eVS_unknown, lastValid=eVS_unknown;
        uint8_t lastOwner=~0, owner=~0;
        uint8_t memc;
        BDTU_MappingInfo mapping;
        uint32_t memsys;

        memsys=BREG_Read32(reg, BCHP_MEMC_GEN_0_CORE_REV_ID);

        /* BSP client ID changed from 0 to 29 >= vB3.0 */
        if((BCHP_GET_FIELD_DATA(memsys, MEMC_GEN_0_CORE_REV_ID, ARCH_REV_ID) < 0xB) ||
           ((BCHP_GET_FIELD_DATA(memsys, MEMC_GEN_0_CORE_REV_ID, ARCH_REV_ID) == 0xB) && BCHP_GET_FIELD_DATA(memsys, MEMC_GEN_0_CORE_REV_ID, CFG_REV_ID) < 3)) {
            memsys = 0;
        }
        else {
            memsys = 1;
        }

        for (memc=0; memc<BCHP_P_MEMC_COUNT; memc++) {
            mapping = settings[memc].memoryLayout;
            for (i=0; i<BCHP_MAX_MEMC_REGIONS; i++) {
                addr = mapping.region[i].addr;
                start = ~0;
                owned = lastOwned = eOS_unknown;
                valid = lastValid = eVS_unknown;
                lastOwner = owner = ~0;
                if (!mapping.region[i].size) {
                    continue;
                }
                for (;mapping.region[i].size;mapping.region[i].size-=_2MB) {
                    rc = BDTU_ReadInfo(dtu[memc], addr, &info);
                    if(rc!=BERR_SUCCESS)
                    {
                        BDBG_ERR(("Unable to obtain info on address " BDBG_UINT64_FMT, BDBG_UINT64_ARG(addr)));
                        goto done;
                    }

                    valid = info.valid ? eVS_valid : eVS_invalid;
                    if(valid==eVS_valid) {
                        owned = info.owned ? eOS_owned : eOS_notOwned;
                        owner = info.owned ? info.ownerID : ~0;
                        if (!memsys && (owner==0)) {
                            /* manually adjust owner to reflect the correct name when printing below */
                            owner = dtu_owner_eBsp_0;
                        }
                    } else {
                        owned = eOS_unknown;
                    }

                    if((addr==mapping.region[i].addr) || (valid != lastValid) || ((valid == eVS_valid) && (owned != lastOwned)) || (owner != lastOwner)) {
                        if((addr!=mapping.region[i].addr) && (start!=(BSTD_DeviceOffset)~0)) {
                            printf(" - " BDBG_UINT64_FMT "\n", BDBG_UINT64_ARG(addr-1));
                        }

                        printf("MEMC[%d][%d] %s : %9.9s " BDBG_UINT64_FMT, mapping.memcIndex, i,
                               (valid == eVS_valid) ? "VALID  " : "INVALID", (owned == eOS_owned) ? ownerID_to_name(owner) : "UNOWNED", BDBG_UINT64_ARG(addr));
                        start=addr;
                    }

                    lastValid=valid;
                    lastOwned=owned;
                    lastOwner=owner;
                    addr += _2MB;
                }
                printf(" - " BDBG_UINT64_FMT "\n", BDBG_UINT64_ARG(addr-1));
            }
        }
    }
    else if(action==action_dump) {
        BDBG_Level level = BDBG_eErr;
        rc = BDBG_GetModuleLevel("BDTU", &level);
        if (rc) BERR_TRACE(rc);
        rc = BDBG_SetModuleLevel("BDTU", BDBG_eMsg);
        if (rc) BERR_TRACE(rc);
        BDTU_PrintMap(dtu[addr_to_memc(scrub_addr, settings)], scrub_addr, scrub_size);
        rc = BDBG_SetModuleLevel("BDTU", level);
        if (rc) BERR_TRACE(rc);
    }
    else if(action==action_dump_all) {
        BDBG_Level level = BDBG_eErr;
        rc = BDBG_GetModuleLevel("BDTU", &level);
        if (rc) BERR_TRACE(rc);
        rc = BDBG_SetModuleLevel("BDTU", BDBG_eMsg);
        if (rc) BERR_TRACE(rc);
        for (i=0; i<BCHP_P_MEMC_COUNT; i++) {
            if (dtu[i]) {
                scrub_addr = settings[i].memoryLayout.region[0].addr;
                scrub_size = settings[i].memoryLayout.region[0].size;
                for (j = 1; j < BCHP_MAX_MEMC_REGIONS; j++) {
                    scrub_size += settings[i].memoryLayout.region[j].size;
                }
                BDTU_PrintMap(dtu[i], scrub_addr, scrub_size);
            }
        }
        rc = BDBG_SetModuleLevel("BDTU", level);
        if (rc) BERR_TRACE(rc);
    }

done:
    for (i=0; i<BCHP_P_MEMC_COUNT; i++) {
        if (dtu[i]) {
            BDTU_Destroy(dtu[i]);
        }
    }
    BREG_Close(reg);
    BDBG_Uninit();
    BKNI_Uninit();

    return 0;
}
