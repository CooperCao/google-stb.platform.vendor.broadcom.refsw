/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *****************************************************************************/

#include "atlas_os.h"
#include <net/if.h>
#include <sys/ioctl.h>
#include <netinet/in.h>

BDBG_MODULE(atlas_os);

CScopedMutex::CScopedMutex(B_MutexHandle mutex) :
    _mutex(mutex)
{
    BDBG_ASSERT(mutex);
    B_Mutex_Lock(_mutex);
}

CScopedMutex::~CScopedMutex()
{
    BDBG_ASSERT(_mutex);
    B_Mutex_Unlock(_mutex);
}

if_interface::if_interface() :
    if_name(""),
    s_addr(0)
{
}

if_interface::if_interface(
        const char *  name,
        unsigned long saddr
        )
{
    if (name)
    {
        if_name += name;
    }
    else
    {
        if_name = "";
    }
    s_addr = saddr;
}

if_interface::~if_interface()
{
    if_name.clear();
}

MList <if_interface> * get_ifaddrs(void)
{
    struct if_nameindex * pIfNameIndex = NULL, * pTempIfNameIndex = NULL;
    int                   fd           = 0;
    struct ifreq          ifr;
    int                   rc = 0;
    struct sockaddr_in *  sa;

    MList <if_interface> * pIfInterfaceList = NULL;
    if_interface *         pIfInterface     = NULL;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1)
    {
        BDBG_ERR(("Failed to open socket"));
        perror("socket");
        goto error;
    }

    pIfNameIndex = if_nameindex();
    if (pIfNameIndex == NULL)
    {
        BDBG_ERR(("Failed to read interface names"));
        perror("if_nameindex");
        goto error;
    }

    pIfInterfaceList = new MList <if_interface>;
    if (pIfInterfaceList == NULL)
    {
        BDBG_ERR(("Failed to create if_interface list"));
        goto error;
    }

    /* Obtain IPv4 addresses*/
    ifr.ifr_addr.sa_family = AF_INET;
    for (pTempIfNameIndex = pIfNameIndex; pTempIfNameIndex->if_name != NULL; pTempIfNameIndex++)
    {
        size_t ifNameLength = strlen(pTempIfNameIndex->if_name);
        if (ifNameLength > (sizeof(ifr.ifr_name)-1))
        {
            BDBG_ERR(("interface name too big %s skipping", pTempIfNameIndex->if_name));
            continue;
        }
        BKNI_Memset(ifr.ifr_name, 0, sizeof(ifr.ifr_name));
        BKNI_Memcpy(ifr.ifr_name, pTempIfNameIndex->if_name, ifNameLength);
        rc = ioctl(fd, SIOCGIFADDR, &ifr);
        if (rc != 0)
        {
            BDBG_MSG(("Unable to read ip address for %s", ifr.ifr_name));
            continue;
        }
        sa = (sockaddr_in *)&ifr.ifr_addr;
        BDBG_MSG(("Interface name:%s ip: " INET_ADDR_PRINTF_FMT, pTempIfNameIndex->if_name, INET_ADDR_PRINTF_ARG(sa->sin_addr.s_addr)));
        if ((sa->sin_addr.s_addr == 0x0100007f) || (sa->sin_addr.s_addr == 0))
        {
            continue;
        }

        pIfInterface = new if_interface(pTempIfNameIndex->if_name, sa->sin_addr.s_addr);
        if (pIfInterface == NULL)
        {
            BDBG_ERR(("Failed to alloc %d", sizeof(if_interface)));
            break;
        }
        pIfInterfaceList->add(pIfInterface);
    }
    if (pIfNameIndex)
    {
        if_freenameindex(pIfNameIndex);
    }

error:
    if (-1 != fd)
    {
        close(fd);
        fd = 0;
    }
    return(pIfInterfaceList);
} /* get_ifaddrs */

void free_ifaddrs(MList <if_interface> * pIfInterfaceList)
{
    if_interface * pInterface = NULL;

    if (pIfInterfaceList == NULL)
    {
        return;
    }

    while (pIfInterfaceList->total())
    {
        pInterface = pIfInterfaceList->first();
        pIfInterfaceList->remove(pInterface);
        pInterface->if_name.clear();
        delete(pInterface);
    }
    delete pIfInterfaceList;
} /* free_ifaddrs */