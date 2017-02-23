/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
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
 **********************************************************************/
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

BDBG_MODULE(dtutool);

static int g_fd;

enum ownerID {
    dtu_owner_eBsp_0 = 0,
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
};

static char *ownerID_to_name(enum ownerID owner)
{
    switch(owner)
    {
        case dtu_owner_eBsp_0:return "bsp_0";
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
    addr = mmap(0, REGISTER_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, g_fd, REGISTER_BASE);
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
    );
}

enum maptype {
    maptype_unmapped,
    maptype_identity,
    maptype_remapped
};

static void print_map(enum maptype maptype, uint64_t fromaddr, uint64_t toaddr)
{
    const char *maptype_str[] = {"unmapped", "identity", "remapped"};
    printf("%s " BDBG_UINT64_FMT "-" BDBG_UINT64_FMT "\n", maptype_str[maptype], BDBG_UINT64_ARG(fromaddr), BDBG_UINT64_ARG(toaddr));
}

int main(int argc, char **argv)
{
    BREG_Handle reg;
    int rc;
    BDTU_Handle dtu;
    BDTU_CreateSettings settings;
    int curarg = 1;
    enum {
        action_help,
        action_print,
        action_restore,
        action_scrub,
        action_show_owners
    } action = action_help;
#define _2MB (2*1024*1024)
    uint64_t dtu_base_unmapped_ba = (uint64_t)2*1024*1024*1024; /* TODO: chip specific */
    uint64_t dtu_max_ba = (uint64_t)4*1024*1024*1024; /* TODO: chip specific */
    unsigned scrub_addr = 0, scrub_size;
    BDTU_Status status;

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
                scrub_addr = strtoul(argv[curarg], NULL, 0);
                scrub_size = strtoul(s, NULL, 0);
            }
            if (!scrub_addr || !scrub_size) {
                print_usage();
                return -1;
            }
            action = action_scrub;
        }
        else if (!strcmp(argv[curarg], "-show_owners")) {
            action = action_show_owners;
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

    BDTU_GetDefaultCreateSettings(&settings);
    settings.reg = reg;
    settings.memcIndex = 0;
    settings.physAddrBase = 0;
    rc = BDTU_Create(&dtu, &settings);
    if (rc) {BERR_TRACE(rc); goto done;}

    rc = BDTU_GetStatus(dtu, &status);
    if (rc) {BERR_TRACE(rc); goto done;}

    if (status.state != BDTU_State_eEnabled) {
        BDBG_ERR(("DTU not enabled: state %u", status.state));
    }

    if (action == action_print) {
        uint64_t addr = 0, prev_addr = 0, devaddr;
        enum maptype prev_maptype = maptype_identity, maptype;
        for (;addr<dtu_max_ba;addr+=_2MB) {
            rc = BDTU_ReadDeviceAddress(dtu, addr, &devaddr);
            if (rc) {
                maptype = maptype_unmapped;
            }
            else {
                maptype = addr==devaddr?maptype_identity:maptype_remapped;
            }
            if (maptype != prev_maptype) {
                print_map(prev_maptype, prev_addr, addr);
                prev_maptype = maptype;
                prev_addr = addr;
            }
        }
        print_map(prev_maptype, prev_addr, addr);
    }
    else if (action == action_restore) {
        /* first, remap back to identity. */
        uint64_t addr = 0, devaddr;
        unsigned n = 0;
        unsigned failures = 0;
        rc = 0;
        for (;addr<dtu_max_ba;addr+=_2MB) {
            rc = BDTU_ReadDeviceAddress(dtu, addr, &devaddr);
            if (!rc && addr != devaddr) {
                n++;
                rc = BDTU_Remap(dtu, devaddr, addr, devaddr);
                if (rc) { BDBG_ERR(("unable to restore ba " BDBG_UINT64_FMT, addr)); failures++;}
            }
        }

        /* second, if no remap failures, reset cma. */
        if (!failures) {
            system("cmatool resetall");
            printf("restored %u superpage(s)\n", n);
        }
    }
    else if (action == action_scrub) {
        rc = BDTU_Scrub(dtu, scrub_addr, scrub_size);
        if (rc) BERR_TRACE(rc);
    }
    else if (action == action_show_owners) {
        BDTU_PageInfo info;
        BSTD_DeviceOffset addr = 0;
        BSTD_DeviceOffset start = ~0;
        enum ownedState {eOS_unknown, eOS_owned, eOS_notOwned} owned=eOS_unknown, lastOwned=eOS_unknown;
        enum validState {eVS_unknown, eVS_valid, eVS_invalid} valid=eVS_unknown, lastValid=eVS_unknown;
        uint8_t lastOwner=~0, owner=~0;

        for (;addr<dtu_max_ba;addr+=_2MB) {
            rc = BDTU_ReadInfo(dtu, addr, &info);
            if(rc!=BERR_SUCCESS)
            {
                BDBG_ERR(("Unable to obtain info on address " BDBG_UINT64_FMT, BDBG_UINT64_ARG(addr)));
                goto done;
            }

            valid = info.valid ? eVS_valid : eVS_invalid;
            if(valid==eVS_valid) {
                owned = info.owned ? eOS_owned : eOS_notOwned;
                owner = info.owned ? info.ownerID : ~0;
            } else {
                owned = eOS_unknown;
            }

            if((addr==0) || (valid != lastValid) || ((valid == eVS_valid) && (owned != lastOwned)) || (owner != lastOwner)) {
                if(addr && (start!=(BSTD_DeviceOffset)~0))
                {
                    printf(" - " BDBG_UINT64_FMT "\n", BDBG_UINT64_ARG(addr));
                }

                printf("%s : %9.9s " BDBG_UINT64_FMT, (valid == eVS_valid) ? "VALID  " : "INVALID", (owned == eOS_owned) ? ownerID_to_name(owner) : "UNOWNED", BDBG_UINT64_ARG(addr));
                start=addr;
            }

            lastValid=valid;
            lastOwned=owned;
            lastOwner=owner;
        }
        printf(" - " BDBG_UINT64_FMT "\n", BDBG_UINT64_ARG(addr));
    }

done:
    if (dtu) {
        BDTU_Destroy(dtu);
    }
    BREG_Close(reg);
    BDBG_Uninit();
    BKNI_Uninit();

    return 0;
}
