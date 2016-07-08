/******************************************************************************
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
 * Module Description:
 *
******************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "nexus_hdmi_types.h"
#include "dynrng_utils.h"
#include "dynrng_smd.h"
#include "dynrng_smd_priv.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/******************************************************************************
 *  static metadata tables
 *****************************************************************************/
static const NEXUS_HdmiDynamicRangeMasteringStaticMetadata zeroSmd =
{
    NEXUS_HdmiDynamicRangeMasteringStaticMetadataType_e1,
    { /* typeSettings */
        { /* Type1 */
            { /* MasteringDisplayColorVolume */
                { 0, 0 }, /* redPrimary (x,y) values 0 to 0xc350 represent 0 to 1.000 in steps of 0.00002 */
                { 0, 0 }, /* greenPrimary (x,y) values 0 to 0xc350 represent 0 to 1.000 in steps of 0.00002 */
                { 0, 0 }, /* bluePrimary (x,y) values 0 to 0xc350 represent 0 to 1.000 in steps of 0.00002 */
                { 0, 0 }, /* whitePoint (x,y) values 0 to 0xc350 represent 0 to 1.000 in steps of 0.00002 */
                { 0, 0 }, /* displayLuminance (max, min) units of 1 cd / m2 and 0.0001 cd / m2 respectively */
            },
            { /* ContentLightLevel */
                0, /* maxContentLightLevel units of 1 cd/m2 */
                0 /* maxFrameAverageLightLevel units of 1 cd/m2 */
            }
        }
    }
};

static const NEXUS_HdmiDynamicRangeMasteringStaticMetadata bt2020Smd =
{
    NEXUS_HdmiDynamicRangeMasteringStaticMetadataType_e1,
    { /* typeSettings */
        { /* Type1 */
            { /* MasteringDisplayColorVolume */
                { SMD_TO_SMPTE_ST2086(0.708), SMD_TO_SMPTE_ST2086(0.292) }, /* redPrimary (x,y) values 0 to 0xc350 represent 0 to 1.000 in steps of 0.00002 */
                { SMD_TO_SMPTE_ST2086(0.170), SMD_TO_SMPTE_ST2086(0.797) }, /* greenPrimary (x,y) values 0 to 0xc350 represent 0 to 1.000 in steps of 0.00002 */
                { SMD_TO_SMPTE_ST2086(0.131), SMD_TO_SMPTE_ST2086(0.046) }, /* bluePrimary (x,y) values 0 to 0xc350 represent 0 to 1.000 in steps of 0.00002 */
                { SMD_TO_SMPTE_ST2086(0.3127), SMD_TO_SMPTE_ST2086(0.3290) }, /* whitePoint (x,y) values 0 to 0xc350 represent 0 to 1.000 in steps of 0.00002 */
                { 0, 0 }, /* displayMasteringLuminance (max, min) units of 1 cd / m2 and 0.0001 cd / m2 respectively */
            },
            { /* ContentLightLevel */
                0, /* maxContentLightLevel units of 1 cd/m2 */
                0 /* maxFrameAverageLightLevel units of 1 cd/m2 */
            }
        }
    }
};

static const NEXUS_HdmiDynamicRangeMasteringStaticMetadata bt709Smd =
{
    NEXUS_HdmiDynamicRangeMasteringStaticMetadataType_e1,
    { /* typeSettings */
        { /* Type1 */
            { /* MasteringDisplayColorVolume */
                { SMD_TO_SMPTE_ST2086(0.64), SMD_TO_SMPTE_ST2086(0.33) }, /* redPrimary (x,y) values 0 to 0xc350 represent 0 to 1.000 in steps of 0.00002 */
                { SMD_TO_SMPTE_ST2086(0.30), SMD_TO_SMPTE_ST2086(0.60) }, /* greenPrimary (x,y) values 0 to 0xc350 represent 0 to 1.000 in steps of 0.00002 */
                { SMD_TO_SMPTE_ST2086(0.15), SMD_TO_SMPTE_ST2086(0.06) }, /* bluePrimary (x,y) values 0 to 0xc350 represent 0 to 1.000 in steps of 0.00002 */
                { SMD_TO_SMPTE_ST2086(0.3127), SMD_TO_SMPTE_ST2086(0.3290) }, /* whitePoint (x,y) values 0 to 0xc350 represent 0 to 1.000 in steps of 0.00002 */
                { 0, 0 }, /* displayMasteringLuminance (max, min) units of 1 cd / m2 and 0.0001 cd / m2 respectively */
            },
            { /* ContentLightLevel */
                0, /* maxContentLightLevel units of 1 cd/m2 */
                0 /* maxFrameAverageLightLevel units of 1 cd/m2 */
            }
        }
    }
};

int SMD_ParseFileLine(SMD_SmdHandle smd, char * line)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    char * name;
    char * value;
    char * comment;

    if (line[0] == '#') goto end;

    comment = strchr(line, '#');
    if (comment) { *comment = 0; }

    /* now look for n/v pairs */
    name = line;
    value = strchr(line, '=');
    if (value)
    {
        *value++ = 0;
        value = UTILS_Trim(value);
    }

    name = UTILS_Trim(name);
    if (!name) { rc = BERR_TRACE(NEXUS_INVALID_PARAMETER); goto end; }
    rc = UTILS_PutStringMapValue(smd->file.map, name, value);
    if (rc) { rc = BERR_TRACE(rc); goto end; }

end:
    return rc;
}

int SMD_LoadFileMetadata(SMD_SmdHandle smd)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    char line[SMD_MAX_LINE_LENGTH];
    FILE * smdFile = NULL;

    if (smd->file.path)
    {
        smdFile = fopen(smd->file.path, "r");
        if (smdFile)
        {
            while (!feof(smdFile))
            {
                if (fgets(line, SMD_MAX_LINE_LENGTH, smdFile))
                {
                    rc = SMD_ParseFileLine(smd, line);
                    if (rc) { rc = BERR_TRACE(rc); } /* print and move on */
                }
            }

            SMD_ParseFileMetadata(smd);
        }
        else
        {
            fprintf(stderr, "Error opening file for static metadata, using defaults: '%s'\n", smd->file.path);
            rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
        }
    }

    if (smdFile)
    {
        fclose(smdFile);
    }
    return rc;
}

void SMD_ParseFileMetadataType(SMD_SmdHandle smd)
{
    const char * type;

    /* default is type 1 for now anyway */
    smd->file.metadata.type = NEXUS_HdmiDynamicRangeMasteringStaticMetadataType_e1;

    type = UTILS_GetStringMapValue(smd->file.map, "type");

    if (type)
    {
        switch (atoi(type))
        {
            case 1:
                smd->file.metadata.type = NEXUS_HdmiDynamicRangeMasteringStaticMetadataType_e1;
                break;
            default:
                break;
        }
    }
}

void SMD_ParseFileType1Metadata(SMD_SmdHandle smd)
{
    const char * value;

    BKNI_Memset(&smd->file.metadata.typeSettings.type1, 0, sizeof(NEXUS_HdmiType1DynamicRangeMasteringStaticMetadata));

    value = UTILS_GetStringMapValue(smd->file.map, "maxContentLightLevel");
    if (value)
    {
        smd->file.metadata.typeSettings.type1.contentLightLevel.max = atoi(value);
    }
    value = UTILS_GetStringMapValue(smd->file.map, "maxFrameAverageLightLevel");
    if (value)
    {
        smd->file.metadata.typeSettings.type1.contentLightLevel.maxFrameAverage = atoi(value);
    }
    value = UTILS_GetStringMapValue(smd->file.map, "displayMasteringLuminance.max");
    if (value)
    {
        smd->file.metadata.typeSettings.type1.masteringDisplayColorVolume.luminance.max = atoi(value);
    }
    value = UTILS_GetStringMapValue(smd->file.map, "displayMasteringLuminance.min");
    if (value)
    {
        smd->file.metadata.typeSettings.type1.masteringDisplayColorVolume.luminance.min = atoi(value);
    }
    value = UTILS_GetStringMapValue(smd->file.map, "whitePoint.x");
    if (value)
    {
        smd->file.metadata.typeSettings.type1.masteringDisplayColorVolume.whitePoint.x = atoi(value);
    }
    value = UTILS_GetStringMapValue(smd->file.map, "whitePoint.y");
    if (value)
    {
        smd->file.metadata.typeSettings.type1.masteringDisplayColorVolume.whitePoint.y = atoi(value);
    }
    value = UTILS_GetStringMapValue(smd->file.map, "redPrimary.x");
    if (value)
    {
        smd->file.metadata.typeSettings.type1.masteringDisplayColorVolume.redPrimary.x = atoi(value);
    }
    value = UTILS_GetStringMapValue(smd->file.map, "redPrimary.y");
    if (value)
    {
        smd->file.metadata.typeSettings.type1.masteringDisplayColorVolume.redPrimary.y = atoi(value);
    }
    value = UTILS_GetStringMapValue(smd->file.map, "greenPrimary.x");
    if (value)
    {
        smd->file.metadata.typeSettings.type1.masteringDisplayColorVolume.greenPrimary.x = atoi(value);
    }
    value = UTILS_GetStringMapValue(smd->file.map, "greenPrimary.y");
    if (value)
    {
        smd->file.metadata.typeSettings.type1.masteringDisplayColorVolume.greenPrimary.y = atoi(value);
    }
    value = UTILS_GetStringMapValue(smd->file.map, "bluePrimary.x");
    if (value)
    {
        smd->file.metadata.typeSettings.type1.masteringDisplayColorVolume.bluePrimary.x = atoi(value);
    }
    value = UTILS_GetStringMapValue(smd->file.map, "bluePrimary.y");
    if (value)
    {
        smd->file.metadata.typeSettings.type1.masteringDisplayColorVolume.bluePrimary.y = atoi(value);
    }
}

void SMD_ParseFileMetadata(SMD_SmdHandle smd)
{
    SMD_ParseFileMetadataType(smd);

    switch (smd->file.metadata.type)
    {
        case NEXUS_HdmiDynamicRangeMasteringStaticMetadataType_e1:
            SMD_ParseFileType1Metadata(smd);
            break;
        default:
            break;
    }
}

void SMD_Destroy(SMD_SmdHandle smd)
{
    if (smd)
    {
        if (smd->file.map)
        {
            UTILS_DestroyStringMap(smd->file.map);
        }
        BKNI_Free(smd);
    }
}

SMD_SmdHandle SMD_Create(void)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    SMD_SmdHandle smd = NULL;

    smd = BKNI_Malloc(sizeof(struct SMD_Smd));
    if (!smd) { rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto error; }
    BKNI_Memset(smd, 0, sizeof(struct SMD_Smd));

    smd->file.map = UTILS_CreateStringMap();
    if (!smd->file.map) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto error; }

    smd->source = SMD_SmdSource_eInput;

    return smd;

error:

    if (smd)
    {
        SMD_Destroy(smd);
    }

    return NULL;
}

int SMD_SetFilePath(SMD_SmdHandle smd, const char * path)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    if (!smd) { rc = BERR_TRACE(NEXUS_INVALID_PARAMETER); goto end; }

    smd->file.path = UTILS_SetString(smd->file.path, path);

end:
    return rc;
}

int SMD_GetMetadata(SMD_SmdHandle smd, NEXUS_HdmiDynamicRangeMasteringStaticMetadata * pMetadata)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    if (!smd || !pMetadata) { rc = BERR_TRACE(NEXUS_INVALID_PARAMETER); goto end; }

    memcpy(pMetadata, smd->pOutput, sizeof(NEXUS_HdmiDynamicRangeMasteringStaticMetadata));

end:
    return rc;
}

void SMD_GetFileMetadata(SMD_SmdHandle smd, NEXUS_HdmiDynamicRangeMasteringStaticMetadata * pMetadata)
{
    if (smd && pMetadata)
    {
        memcpy(pMetadata, &smd->file.metadata, sizeof(NEXUS_HdmiDynamicRangeMasteringStaticMetadata));
    }
}

void SMD_GetBt2020Metadata(SMD_SmdHandle smd, NEXUS_HdmiDynamicRangeMasteringStaticMetadata * pMetadata)
{
    if (smd && pMetadata)
    {
        memcpy(pMetadata, &bt2020Smd, sizeof(NEXUS_HdmiDynamicRangeMasteringStaticMetadata));
    }
}

void SMD_GetBt709Metadata(SMD_SmdHandle smd, NEXUS_HdmiDynamicRangeMasteringStaticMetadata * pMetadata)
{
    if (smd && pMetadata)
    {
        memcpy(pMetadata, &bt709Smd, sizeof(NEXUS_HdmiDynamicRangeMasteringStaticMetadata));
    }
}

void SMD_GetUserMetadata(SMD_SmdHandle smd, NEXUS_HdmiDynamicRangeMasteringStaticMetadata * pMetadata)
{
    if (smd && pMetadata)
    {
        memcpy(pMetadata, &smd->user, sizeof(NEXUS_HdmiDynamicRangeMasteringStaticMetadata));
    }
}

int SMD_SetUserMetadata(SMD_SmdHandle smd, const NEXUS_HdmiDynamicRangeMasteringStaticMetadata * pMetadata)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    if (!smd) { rc = BERR_TRACE(NEXUS_INVALID_PARAMETER); goto end; }

    if (pMetadata)
    {
        memcpy(&smd->user, pMetadata, sizeof(NEXUS_HdmiDynamicRangeMasteringStaticMetadata));
    }
    else
    {
        memcpy(&smd->user, &zeroSmd, sizeof(NEXUS_HdmiDynamicRangeMasteringStaticMetadata));
    }

end:
    return rc;
}

void SMD_GetInputMetadata(SMD_SmdHandle smd, NEXUS_HdmiDynamicRangeMasteringStaticMetadata * pMetadata)
{
    if (smd && pMetadata)
    {
        memcpy(pMetadata, &smd->input, sizeof(NEXUS_HdmiDynamicRangeMasteringStaticMetadata));
    }
}

int SMD_SetInputMetadata(SMD_SmdHandle smd, const NEXUS_HdmiDynamicRangeMasteringStaticMetadata * pMetadata)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    if (!smd) { rc = BERR_TRACE(NEXUS_INVALID_PARAMETER); goto end; }

    if (pMetadata)
    {
        memcpy(&smd->input, pMetadata, sizeof(NEXUS_HdmiDynamicRangeMasteringStaticMetadata));
    }
    else
    {
        memcpy(&smd->input, &zeroSmd, sizeof(NEXUS_HdmiDynamicRangeMasteringStaticMetadata));
    }

end:
    return rc;
}

int SMD_SetSmdSource(SMD_SmdHandle smd, SMD_SmdSource source)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    if (!smd) { rc = BERR_TRACE(NEXUS_INVALID_PARAMETER); goto end; }

    switch (source)
    {
        case SMD_SmdSource_eZero:
            smd->pOutput = &zeroSmd;
            break;
        case SMD_SmdSource_eFile:
            if (smd->file.path)
            {
                rc = SMD_LoadFileMetadata(smd);
                if (rc) { rc = BERR_TRACE(rc); goto end; }
                smd->pOutput = &smd->file.metadata;
            }
            break;
        case SMD_SmdSource_eUser:
            smd->pOutput = &smd->user;
            break;
        case SMD_SmdSource_eInput:
            smd->pOutput = &smd->input;
            break;
        case SMD_SmdSource_eBt709:
            smd->pOutput = &bt709Smd;
            break;
        case SMD_SmdSource_eBt2020:
            smd->pOutput = &bt2020Smd;
            break;
        default:
            break;
    }

    smd->source = source;

end:
    return rc;
}

static const UTILS_StringIntMapEntry smdSourceNames[] =
{
    { "zero", SMD_SmdSource_eZero },
    { "file", SMD_SmdSource_eFile },
    { "input", SMD_SmdSource_eInput },
    { "bt709", SMD_SmdSource_eBt709 },
    { "bt2020", SMD_SmdSource_eBt2020 },
    { "user", SMD_SmdSource_eUser },
    { NULL, SMD_SmdSource_eMax }
};

SMD_SmdSource SMD_ParseSmdSource(const char * name)
{
    return (SMD_SmdSource)UTILS_ParseTableAlias(smdSourceNames, name);
}

const char * SMD_GetSmdSourceName(SMD_SmdSource source)
{
    return UTILS_GetTableName(smdSourceNames, source);
}

SMD_SmdSource SMD_GetSmdSource(SMD_SmdHandle smd)
{
    SMD_SmdSource source = SMD_SmdSource_eInput;

    if (smd)
    {
        source = smd->source;
    }

    return source;
}

void SMD_PrintMetadata(const char * source, const NEXUS_HdmiDynamicRangeMasteringStaticMetadata * pMetadata)
{
    fprintf(stdout, "# %s static metadata\n", source);
    fprintf(stdout, "type = %s\n", SMD_GetMetadataTypeName(pMetadata->type));
    switch (pMetadata->type)
    {
        case NEXUS_HdmiDynamicRangeMasteringStaticMetadataType_e1:
            fprintf(stdout, "redPrimary.x = %u\n", pMetadata->typeSettings.type1.masteringDisplayColorVolume.redPrimary.x);
            fprintf(stdout, "redPrimary.y = %u\n", pMetadata->typeSettings.type1.masteringDisplayColorVolume.redPrimary.y);
            fprintf(stdout, "greenPrimary.x = %u\n", pMetadata->typeSettings.type1.masteringDisplayColorVolume.greenPrimary.x);
            fprintf(stdout, "greenPrimary.y = %u\n", pMetadata->typeSettings.type1.masteringDisplayColorVolume.greenPrimary.y);
            fprintf(stdout, "bluePrimary.x = %u\n", pMetadata->typeSettings.type1.masteringDisplayColorVolume.bluePrimary.x);
            fprintf(stdout, "bluePrimary.y = %u\n", pMetadata->typeSettings.type1.masteringDisplayColorVolume.bluePrimary.y);
            fprintf(stdout, "whitePoint.x = %u\n", pMetadata->typeSettings.type1.masteringDisplayColorVolume.whitePoint.x);
            fprintf(stdout, "whitePoint.y = %u\n", pMetadata->typeSettings.type1.masteringDisplayColorVolume.whitePoint.y);
            fprintf(stdout, "displayMasteringLuminance.max = %u\n",
                pMetadata->typeSettings.type1.masteringDisplayColorVolume.luminance.max);
            fprintf(stdout, "displayMasteringLuminance.min = %u\n",
                pMetadata->typeSettings.type1.masteringDisplayColorVolume.luminance.min);
            fprintf(stdout, "maxContentLightLevel = %u\n",
                pMetadata->typeSettings.type1.contentLightLevel.max);
            fprintf(stdout, "maxFrameAverageLightLevel = %u\n",
                pMetadata->typeSettings.type1.contentLightLevel.maxFrameAverage);
            break;
        default:
            break;
    }
}

void SMD_Print(SMD_SmdHandle smd)
{
    fprintf(stdout, "# SMD\n");
    fprintf(stdout, "source = %s\n", SMD_GetSmdSourceName(smd->source));
    fprintf(stdout, "file.path = %s\n", smd->file.path ? smd->file.path : "<none>");
    SMD_PrintMetadata("file", &smd->file.metadata);
    SMD_PrintMetadata("input", &smd->input);
    SMD_PrintMetadata("user", &smd->user);
}

static const UTILS_StringIntMapEntry smdTypeNames[] =
{
    { "1", NEXUS_HdmiDynamicRangeMasteringStaticMetadataType_e1 },
    { NULL, NEXUS_HdmiDynamicRangeMasteringStaticMetadataType_eMax }
};

const char * SMD_GetMetadataTypeName(NEXUS_HdmiDynamicRangeMasteringStaticMetadataType type)
{
    return UTILS_GetTableName(smdTypeNames, type);
}
