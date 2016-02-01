/***************************************************************************
*     (c)2004-2008 Broadcom Corporation
*  
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,
*  and may only be used, duplicated, modified or distributed pursuant to the terms and
*  conditions of a separate, written license agreement executed between you and Broadcom
*  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
*  no license (express or implied), right to use, or waiver of any kind with respect to the
*  Software, and Broadcom expressly reserves all rights in and to the Software and all
*  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
*  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
*  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.  
*   
*  Except as expressly set forth in the Authorized License,
*   
*  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
*  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
*  and to use this information only in connection with your use of Broadcom integrated circuit products.
*   
*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" 
*  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR 
*  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO 
*  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES 
*  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, 
*  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION 
*  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF 
*  USE OR PERFORMANCE OF THE SOFTWARE.
*  
*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS 
*  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR 
*  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR 
*  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF 
*  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT 
*  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE 
*  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF 
*  ANY LIMITED REMEDY.
* 
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* Description:
*   API name: PSIP
*    Library debugging routines for PSIP
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#include "b_os_lib.h"
#include "b_psip_lib.h"
#include "blst_list.h"
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include "ts_psi.h"
#include "ts_pat.h"
#include "ts_pmt.h"
#include "psip_mgt.h"
#include "psip_vct.h"
#include "psip_eit.h"
#include "psip_ett.h"
#include "psip_mss.h"
#include "psip_stt.h"
#include "psip_rrt.h"
#include "psip_descriptor.h"

#include "tsid_table.h"

BDBG_MODULE(b_psip_lib);

#include "b_psip_defines.h"
#include "b_psip_states.h"
#include "b_psip_gettable.h"
#include "b_psip_job.h"
#include "b_psip_psip.h"
#include "b_psip_states_decl.h"
#include "b_psip_stateengine.h"
#include "b_psip_table.h"
#include "b_psip_utils.h"

/***************************************************************************
Summary:
    Print the Program Allocation Table (PAT)
***************************************************************************/
void printPAT(
    B_PSIP * pSi, 
    uint8_t * buffer, 
    size_t size
    )
{
    int            i        = 0;
    TS_PAT_program program;

    BSTD_UNUSED(pSi);

    BDBG_MSG(("PAT - %d Programs", TS_PAT_getNumPrograms(buffer)));

    for (i = 0; TS_PAT_getProgram(buffer, size, i, &program)==BERR_SUCCESS; i++) 
    {
        BDBG_MSG(("program_number: %d, PID: 0x%04X", program.program_number, program.PID)); 
    }
}

/***************************************************************************
Summary:
    Print the Program Map Table (PMT)
***************************************************************************/
void printPMT(
    B_PSIP * pSi, 
    uint8_t * buffer, 
    size_t size
    )
{
    int           i       = 0;
    TS_PMT_stream stream;
    /*TS_PSI_descriptor descriptor;*/
    BSTD_UNUSED(pSi);

    BDBG_MSG(("PMT - PCR PID: 0x%04X - num streams:%d", 
        TS_PMT_getPcrPid(buffer, size), 
        TS_PMT_getNumStreams(buffer, size)));

    for (i = 0; i < TS_PMT_getNumStreams(buffer, size); i++)
    {
        TS_PMT_getStream(buffer, size, i, &stream);
        BDBG_MSG(("stream_type:0x%02X, PID:0x%04X", 
            stream.stream_type, 
            stream.elementary_PID));
    }
}

/***************************************************************************
Summary:
    Print the Master Guide Table (MGT)
***************************************************************************/
void printMGT(
    B_PSIP * pSi, 
    uint8_t * buffer, 
    size_t size
    )
{
    int i = 0;
    BSTD_UNUSED(pSi);
    BSTD_UNUSED(size);

    BDBG_MSG(("Number of MGT tables:%d", PSIP_MGT_getTablesDefined(buffer)));
    for (i = 0; i < PSIP_MGT_getTablesDefined(buffer); i++)
    {
        PSIP_MGT_table table;
        if (!PSIP_MGT_getTable(buffer, i, &table))
        {
            if (table.table_type == 0x0000)
                BDBG_MSG(("table type: Terrestrial VCT (current_next_ind==1)"));
            else
            if (table.table_type == 0x0001)
                BDBG_MSG(("table type: Terrestrial VCT (current_next_ind==0)"));
            else
            if (table.table_type == 0x0002)
                BDBG_MSG(("table type: Cable VCT (current_next_ind==1)"));
            else
            if (table.table_type == 0x0003)
                BDBG_MSG(("table type: Cable VCT (current_next_ind==0)"));
            else
            if (table.table_type == 0x0004)
                BDBG_MSG(("table type: Channel ETT"));
            else
            if (table.table_type == 0x0005)
                BDBG_MSG(("table type: DCCSCT"));
            else
            if ((table.table_type >= 0x0006) && (table.table_type <= 0x00FF))
                BDBG_MSG(("table type: 0x%04x reserved for future atsc", table.table_type));
            else
            if ((table.table_type >= 0x0100) && (table.table_type <= 0x017F))
                BDBG_MSG(("table type: 0x%04x EIT-0 thru EIT-127", table.table_type));
            else
            if ((table.table_type >= 0x0180) && (table.table_type <= 0x01FF))
                BDBG_MSG(("table type: 0x%04x reserved for future atsc", table.table_type));
            else
            if ((table.table_type >= 0x0200) && (table.table_type <= 0x027F))
                BDBG_MSG(("table type: 0x%04x ETT-0 thru ETT-127", table.table_type));
            else
            if ((table.table_type >= 0x0280) && (table.table_type <= 0x0300))
                BDBG_MSG(("table type: 0x%04x reserved for future atsc", table.table_type));
            else
            if ((table.table_type >= 0x0301) && (table.table_type <= 0x03FF))
                BDBG_MSG(("table type: 0x%04x RRT region 1-255", table.table_type));
            else
            if ((table.table_type >= 0x0400) && (table.table_type <= 0x0FFF))
                BDBG_MSG(("table type: 0x%04x User private", table.table_type));
            else
            if ((table.table_type >= 0x1000) && (table.table_type <= 0x13FF))
                BDBG_MSG(("table type: 0x%04x reserved for future atsc", table.table_type));
            else
            if ((table.table_type >= 0x1400) && (table.table_type <= 0x14FF))
                BDBG_MSG(("table type: 0x%04x DCCT (dc_id 0x00-0xFF)", table.table_type));
            else
            if ((table.table_type >= 0x1500))
                BDBG_MSG(("table type: 0x%04x reserved for future atsc", table.table_type));

            BDBG_MSG(("table_type_PID:0x%04x", table.table_type_PID));
            BDBG_MSG(("table_type_version_number:%d", table.table_type_version_number));
            BDBG_MSG(("number_bytes:%d", table.number_bytes));
        }
    }
}

/***************************************************************************
Summary:
    Print the Virtual Channel Table (VCT)
***************************************************************************/
void printVCT(
    B_PSIP * pSi, 
    uint8_t * buffer, 
    size_t size
    )
{
    int i = 0;
    BSTD_UNUSED(pSi);
    BSTD_UNUSED(size);

    /* process tvct */
    BDBG_MSG(("Number of TVCT channels:%d", PSIP_VCT_getNumChannels(buffer)));
    for (i = 0; i < PSIP_VCT_getNumChannels(buffer); i++)
    {
        PSIP_VCT_channel ch;
        if (!PSIP_VCT_getChannel(buffer, i, &ch))
        {
            BDBG_MSG(("\nshort_name:%s", ch.short_name));
            BDBG_MSG(("major_channel_number:%d", ch.major_channel_number));
            BDBG_MSG(("minor_channel_number:%d", ch.minor_channel_number));
            switch(ch.modulation_mode)
            {
            case PSIP_VCT_analog:
                BDBG_MSG(("modulation_mode: analog"));
                break;
            case PSIP_VCT_SCTE_mode_1:
                BDBG_MSG(("modulation_mode: SCTE1"));
                break;
            case PSIP_VCT_SCTE_mode_2:
                BDBG_MSG(("modulation_mode: SCTE2"));
                break;
            case PSIP_VCT_ATSC_8:
                BDBG_MSG(("modulation_mode: ATSC8"));
                break;
            case PSIP_VCT_ATSC_16:
                BDBG_MSG(("modulation_mode: ATSC16"));
                break;
            default:
                BDBG_MSG(("modulation_mode: unknown"));
                break;
            }
            BDBG_MSG(("carrier_freq:%d", ch.carrier_frequency));
            BDBG_MSG(("channel_TSID:%d", ch.channel_TSID));
            BDBG_MSG(("program number:%d", ch.program_number));
            switch(ch.ETM_location)
            {
            case PSIP_no_ETM:
                BDBG_MSG(("ETM location: NO ETM"));
                break;
            case PSIP_ETM:
                BDBG_MSG(("ETM location: ETM"));
                break;
            case PSIP_ETM_channel_TSID:
                BDBG_MSG(("ETM location: ETM channel TSID"));
                break;
            default:
                BDBG_MSG(("ETM location: Unknown"));
                break;
            }
            BDBG_MSG(("access_controlled:%d", ch.access_controlled));
            BDBG_MSG(("hidden:%d", ch.hidden));
            BDBG_MSG(("hide guide:%d", ch.hide_guide));
            switch(ch.service_type)
            {
            case PSIP_VCT_analog_television:
                BDBG_MSG(("service type: analog"));
                break;
            case PSIP_VCT_ATSC_digital_television:
                BDBG_MSG(("service type: ATSC digital"));
                break;
            case PSIP_VCT_ATSC_audio:
                BDBG_MSG(("service type: ATSC audio"));
                break;
            case PSIP_VCT_ATSC_data_only_service:
                BDBG_MSG(("service type: ATSC data only service"));
                break;
            default:
                BDBG_MSG(("service type: Unknown"));
                break;
            }
            BDBG_MSG(("source id:%d", ch.source_id));
        }
    }

}

/***************************************************************************
Summary:
    Print the Region Rating Table (RRT)
***************************************************************************/
void printRRT(
    B_PSIP * pSi, 
    uint8_t * buffer, 
    size_t size
    )
{
    PSIP_RRT_header    header;
    PSIP_RRT_dimension dim;
    PSIP_RRT_value     val;
    uint8_t            tmp_str[64];
    uint16_t           length = 64;
    int                dim_idx = 0;
    int                val_idx = 0;

    BSTD_UNUSED(size);

    if (0 == size)
    {
        return;
    }

    PSIP_RRT_getHeader(buffer, &header);

    memset(tmp_str, 0, 64);
    if (B_ERROR_SUCCESS == B_PSIP_GetStringFromMSS(pSi, header.p_rating_region_name_text, tmp_str, &length))
    {
        BDBG_MSG(("RRT %8s\n",tmp_str));
    }

    for (dim_idx = 0; dim_idx < header.dimensions_defined; ++dim_idx)
    {
        if (B_ERROR_SUCCESS != PSIP_RRT_getDimension(buffer, dim_idx, &dim))
        {
            continue;
        }

        length = 64;
        memset(tmp_str, 0, 64);
        if (B_ERROR_SUCCESS == B_PSIP_GetStringFromMSS(pSi, dim.p_dimension_name_text, tmp_str, &length))
        {
            BDBG_WRN(("  RRT[%d] %8s(%d)\n", dim_idx, tmp_str, dim.graduated_scale));
        }

        for (val_idx = 0; val_idx < dim.values_defined; ++val_idx)
        {
            if (B_ERROR_SUCCESS != PSIP_RRT_getValue(buffer, dim_idx, val_idx,& val))
            {
                continue;
            }

            length = 64;
            memset(tmp_str, 0, 64);
            if (B_ERROR_SUCCESS == B_PSIP_GetStringFromMSS(pSi, val.p_rating_value_text, tmp_str, &length))
            {
                BDBG_WRN(("  RRT[%d,%d] %8s\n", dim_idx, val_idx, tmp_str));
            }

            length = 64;
            memset(tmp_str, 0, 64);
            if (B_ERROR_SUCCESS == B_PSIP_GetStringFromMSS(pSi, val.p_abbrev_rating_value_text, tmp_str, &length))
            {
                BDBG_WRN(("  RRT[%d,%d] %8s\n", dim_idx, val_idx, tmp_str));
            }
        }
    }

}

/***************************************************************************
Summary:
    Print the Event Information Table (EIT)
***************************************************************************/
void printEIT(
    B_PSIP * pSi, 
    uint8_t * buffer, 
    size_t size
    )
{
    TS_PSI_header PsiHeader;
    int i = 0;
    int j = 0;
    BSTD_UNUSED(size);

    TS_PSI_getSectionHeader(buffer, &PsiHeader);

    BDBG_MSG(("number of EIT events:%d", PSIP_EIT_getNumEvents(buffer)));
    for (i = 0; i < PSIP_EIT_getNumEvents(buffer); i++)
    {
        PSIP_EIT_event event;
        if (!PSIP_EIT_getEvent(buffer, i, &event))
        {
            BDBG_MSG(("event_id:0x%04x", event.event_id));
            BDBG_MSG(("start_time:%lu", event.start_time));
            switch(event.ETM_location)
            {
            case PSIP_no_ETM:
                BDBG_MSG(("ETM location: NO ETM"));
                break;
            case PSIP_ETM:
                {
                    B_PSIP_CollectionRequest data;
                    uint8_t *ettBuffer = (uint8_t *)B_Os_Malloc(ETT_BUF_LEN);
                    uint32_t ettBufferLen = ETT_BUF_LEN;

                    BDBG_MSG(("ETM location: ETM"));
                    B_PSIP_GetDefaultCollectionData(&data);
                    data.pid             = 0x1200; /* ETT PID */
                    data.filter.coef[0]  = 0xCC;   /* ETT */
                    data.filter.mask[0]  = 0x00;
                    data.filter.excl[0]  = 0xFF;
                    data.filter.coef[8]  = 0x00;    /* protocol_version */
                    data.filter.mask[8]  = 0x00;
                    data.filter.excl[8]  = 0xFF;
                    /* search using ETM id */
                    data.filter.coef[9]  = (uint8_t)((PsiHeader.table_id_extension >> 8) & 0xFF);
                    data.filter.mask[9]  = 0x00;
                    data.filter.excl[9]  = 0xFF;
                    data.filter.coef[10] = (uint8_t)(PsiHeader.table_id_extension & 0xFF);
                    data.filter.mask[10] = 0x00;
                    data.filter.excl[10] = 0xFF;
                    data.filter.coef[11] = (uint8_t)((event.event_id >> 6) & 0xFF);
                    data.filter.mask[11] = 0x00;
                    data.filter.excl[11] = 0xFF;
                    data.filter.coef[12] = (uint8_t)((((event.event_id << 2) & 0xFFFC) | 2) & 0xFF);
                    /*data.filter.coef[12] = 0x02;*/ /* event ETMs only */
                    data.filter.mask[12] = 0xFC;
                    data.filter.excl[12] = 0xFF;
                    data.pBuffer         = ettBuffer;
                    data.pBufferLength   = &ettBufferLen;

                    BDBG_MSG(("searching for etm id:0x%02x%02x%02x%02x", 
                              data.filter.coef[9], data.filter.coef[10], data.filter.coef[11], data.filter.coef[12]));

                    pSi->CollectionFunc(&data, pSi->CollectionContext);
                    if (*(data.pBufferLength))
                    {
                        int j = 0;

                        PSIP_ETT_header headerETT;
                        BDBG_MSG(("ETM received! length:%d", *(data.pBufferLength)));
                        PSIP_ETT_getHeader(data.pBuffer, &headerETT);
                        switch (headerETT.ETM_id.id_type)
                        {
                        case PSIP_ETT_channel_id:
                            BDBG_MSG(("ETT: id_type: PSIP_ETT_channel_id"));
                            break;
                        case PSIP_ETT_event_id:
                            BDBG_MSG(("ETT: id_type: PSIP_ETT_event_id"));
                            break;
                        default:
                            BDBG_MSG(("ETT: id_type: INVALID!"));
                            break;
                        }

                        BDBG_MSG(("ETT: source_id:%d", headerETT.ETM_id.source_id));
                        BDBG_MSG(("ETT: event_id:%d", headerETT.ETM_id.event_id));

                        for (j = 0; j < PSIP_MSS_getNumStrings(headerETT.p_extended_text_message); j++)
                        {
                            int str_size = 1024;
                            char str[1024];

                            str_size = 1024;
                            memset(str, 0, 1024);
                            if (!PSIP_MSS_getString(headerETT.p_extended_text_message, j, &str_size, str))
                            {
                                BDBG_MSG(("ETT: text:%s", str));
                            }
                        }
                    }
                    else
                    {
                        BDBG_MSG(("ETM NOT received! length:%d", *(data.pBufferLength)));
                    }
                    B_Os_Free(ettBuffer);
                }
                break;
            case PSIP_ETM_channel_TSID:
                BDBG_MSG(("ETM location: ETM channel TSID"));
                break;
            default:
                BDBG_MSG(("ETM location: Unknown"));
                break;
            }
            BDBG_MSG(("length_in_seconds:%d", event.length_in_seconds));
            for (j = 0; j < PSIP_MSS_getNumStrings(event.p_title_text); j++)
            {
                int str_size = 1024;
                char str[1024];

                str_size = 1024;
                memset(str, 0, 1024);
                if (!PSIP_MSS_getString(event.p_title_text, j, &str_size, str))
                {
                    BDBG_MSG(("--------------------------------- title_text:%s", str));
                }
            }
        }
    }
}

/***************************************************************************
Summary:
    Print the System Time Table (STT)
***************************************************************************/
void printSTT(
    uint8_t * buffer, 
    size_t size
    )
{
    PSIP_STT_header header;
    BSTD_UNUSED(size);

    PSIP_STT_getHeader(buffer, &header);
    BDBG_MSG(("system_time:%lu", header.system_time));
    BDBG_MSG(("GPS_UTC_offset:%lu", header.GPS_UTC_offset));
    BDBG_MSG(("DS_status:%d", header.daylight_savings.DS_status));
    BDBG_MSG(("DS_day_of_month:%d", header.daylight_savings.DS_day_of_month));
    BDBG_MSG(("DS_hour:%d", header.daylight_savings.DS_hour));

}

/***************************************************************************
Summary:
    Print the PSIP header
***************************************************************************/
void printHeader(
    TS_PSI_header headerpsi
    )
{
    BDBG_MSG(("******"));
    BDBG_MSG(("****** table_id:0x%02x", headerpsi.table_id));
    BDBG_MSG(("****** section_syntax_indicator:%d", headerpsi.section_syntax_indicator));
    BDBG_MSG(("****** private_indicator:%d", headerpsi.private_indicator));
    BDBG_MSG(("****** section_length:%d", headerpsi.section_length));
    BDBG_MSG(("****** table_id_extension:%d", headerpsi.table_id_extension));
    BDBG_MSG(("****** version_number:%d", headerpsi.version_number));
    BDBG_MSG(("****** current_next_indicator:%d", headerpsi.current_next_indicator));
    BDBG_MSG(("****** section_number:%d", headerpsi.section_number));
    BDBG_MSG(("****** last_section_number:%d", headerpsi.last_section_number));
    BDBG_MSG(("****** CRC_32:%lu", headerpsi.CRC_32));
    BDBG_MSG(("******"));
}

/***************************************************************************
Summary:
    Print the given PSIP table
***************************************************************************/
void printTable(
    B_PSIP * pSi, 
    uint8_t table_id, 
    uint8_t * buffer, 
    size_t size
    )
{
    switch(table_id)
    {
    case 0x00:
        printPAT(pSi, buffer, size);
        break;
    case 0x02:
        printPMT(pSi, buffer, size);
        break;
    case 0xC7:
        printMGT(pSi, buffer, size);
        break;
    case 0xC8:
        /* fall through */
    case 0xC9:
        printVCT(pSi, buffer, size);
        break;
    case 0xCA:
        printRRT(pSi, buffer, size);
        break;
    case 0xCB:
        printEIT(pSi, buffer, size);
        break;
    case 0xCC:
        break;
    case 0xCD:
        printSTT(buffer, size);
        break;
    case 0xD3:
        break;
    case 0xD4:
        break;
    default:
        BDBG_MSG(("*********************** unhandled si table"));
        break;
    }
}

