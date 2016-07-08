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
 *****************************************************************************/

#include "atlas.h"
#include "atlas_cfg.h"

/* include it a second time in order to define the structure */
#undef ATLAS_CFG_H__
#undef ATLAS_DECLARE
#define ATLAS_DECLARE_CONFIGSETTING_VALUES
#include "atlas_cfg.h"

#include "bstd.h"

BDBG_MODULE(atlas_cfg);

CConfiguration::CConfiguration(const char * filename)
{
    int i = 0;

    _pRegistry = new MStringHash();

    /* add default name/value pairs */
    for (i = 0; i < total(); i++)
    {
        set(_predefines[i]._name, _predefines[i]._default);
    }

    if (filename)
    {
        read(filename);
    }
}

CConfiguration::CConfiguration(const CConfiguration & cfg)
{
    _pRegistry = new MStringHash(*cfg._pRegistry);
}

CConfiguration::~CConfiguration()
{
    DEL(_pRegistry);
}

void CConfiguration::initialize()
{
    _platformConfig.initialize();
}

#undef TOTAL_SETTINGS
#define TOTAL_SETTINGS()  (sizeof(_predefines)/sizeof(_predefines[0]))
int CConfiguration::total()
{
    return(TOTAL_SETTINGS());
}

const char * CConfiguration::getName(int index)
{
    return(_predefines[index]._name);
}

const char * CConfiguration::getDescription(int index)
{
    return(_predefines[index]._description);
}

const char * CConfiguration::get(
        const char * name,
        const char * defaultvalue
        ) const
{
    MString &s = _pRegistry->get(name);

    if (!s)
    {
        return(defaultvalue);
    }
    else
    {
        return(s);
    }
}

int CConfiguration::getInt(
        const char * name,
        int          defaultvalue
        ) const
{
    MString &s = _pRegistry->get(name);

    if (!s)
    {
        return(defaultvalue);
    }
    else
    {
        return(s.toInt());
    }
}

bool CConfiguration::getBool(
        const char * name,
        bool         defaultvalue
        ) const
{
    MString &s = _pRegistry->get(name);

    if (!s)
    {
        return(defaultvalue);
    }
    else
    {
        return(s[0] == 't' || s[0] == 'T' ||
               s[0] == 'y' || s[0] == 'Y' || s.toInt());
    }
}

double CConfiguration::getDouble(
        const char * name,
        double       defaultvalue
        ) const
{
    MString &s = _pRegistry->get(name);

    if (!s)
    {
        return(defaultvalue);
    }
    else
    {
        return(s.toDouble());
    }
}

void CConfiguration::set(
        const char * name,
        double       value
        )
{
    char buf[25];

    snprintf(buf, sizeof(buf), "%f", value);
    /* I don't care for the %e implementation of MString */
    set(name, buf);
}

void CConfiguration::set(
        const char * name,
        const char * value
        )
{
    /* printf("CConfiguration::set(%s,%s)\n", name, value); */
    _pRegistry->add(name, value);
}

void CConfiguration::set(
        const char * name,
        int          value
        )
{
    set(name, MString(value));
}

int CConfiguration::read(const char * filename)
{
    return(_pRegistry->read(filename, true) ? 0 : 1);
}

void CConfiguration::printHelp() const
{
    for (unsigned i = 0; i < TOTAL_SETTINGS(); i++)
    {
        printf("%s=%s (type %s)\n  %s\n",
                _predefines[i]._name,
                get(_predefines[i]._name),
                _predefines[i]._type,
                _predefines[i]._description);
    }
}