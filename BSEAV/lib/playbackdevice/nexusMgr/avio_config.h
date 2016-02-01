/***************************************************************************
 *     (c)1997-2011 Broadcom Corporation
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
 * Module Description: This header is used to handle AV I/O interface
 *                     and was derived from the Kylin application.
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef AVIO_CONFIG_H__
#define AVIO_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

/* export output_type to config display as panel or component output.
* default config is panel output.
*   - export output_type=scart to config the output as scart. This only works on
*     93556 board. The output is a half scart and right now, it only support SD
*     composite.
*   - export output_type=component to config the output as component. This only
*     works on 93549 board. For A0, This output is component and its initial format is
*     720p. Only 480p and 720p are avaiable. For Bx, This output in fact is composite 
*     and its only supported format is 480i.
*   - export output_type to any other value or don't export output_type, output
*     will config the output type to be panel.
*/
#define AVIO_CONFIG_OUTPUT_TYPE                     "output_type"
#define AVIO_CONFIG_OUTPUT_TYPE_SCART               "scart"
#define AVIO_CONFIG_OUTPUT_TYPE_COMPONENT           "component"
#define AVIO_CONFIG_OUTPUT_TYPE_PANEL               "panel"

/* Is output type configured as component? */
#define AVIO_CONFIG_IS_COMPONENT_OUTPUT \
    ((getenv(AVIO_CONFIG_OUTPUT_TYPE)) && \
    (strcmp(getenv(AVIO_CONFIG_OUTPUT_TYPE), AVIO_CONFIG_OUTPUT_TYPE_COMPONENT) == 0))

/* Is output type configured as scart? */
#define AVIO_CONFIG_IS_SCART_OUTPUT \
    ((getenv(AVIO_CONFIG_OUTPUT_TYPE)) && \
    (strcmp(getenv(AVIO_CONFIG_OUTPUT_TYPE), AVIO_CONFIG_OUTPUT_TYPE_SCART) == 0))

/* Is output type configured as panel? */
#define AVIO_CONFIG_IS_PANEL_OUTPUT \
    ((getenv(AVIO_CONFIG_OUTPUT_TYPE)) && \
    (strcmp(getenv(AVIO_CONFIG_OUTPUT_TYPE), AVIO_CONFIG_OUTPUT_TYPE_PANEL) == 0))

/* export output_jeida_mode to config jeida mode when output type is panel. It can
* be jeida or openldi. default config is decide by nexus.
*   - export output_jeida_mode=openldi to config the panel jeida mode to be openldi.
*   - export output_jeida_mode=jeida to config the panel to jeida mode.
    - any other value or don't export output_jeida_mode
*     will config the panel set to default.
* NOTE - output_jeida_mode only take effect when output_type is panel.
*/
#define AVIO_CONFIG_PANEL_JEIDA_MODE                "output_jeida_mode"
#define AVIO_CONFIG_PANEL_JEIDA_MODE_JEIDA          "jeida"
#define AVIO_CONFIG_PANEL_JEIDA_MODE_OPENLDI        "openldi"

/* Is output jeida mode configured as jeida? */
#define AVIO_CONFIG_IS_PANEL_JEIDA_MODE_JEIDA \
    ((getenv(AVIO_CONFIG_PANEL_JEIDA_MODE)) && \
    (strcmp(getenv(AVIO_CONFIG_PANEL_JEIDA_MODE), AVIO_CONFIG_PANEL_JEIDA_MODE_JEIDA) == 0))

/* Is output jedia mode configured as openldi? */
#define AVIO_CONFIG_IS_PANEL_JEIDA_MODE_OPENLDI \
    ((getenv(AVIO_CONFIG_PANEL_JEIDA_MODE)) && \
    (strcmp(getenv(AVIO_CONFIG_PANEL_JEIDA_MODE), AVIO_CONFIG_PANEL_JEIDA_MODE_OPENLDI) == 0))

/* export output_color_mode to config color mode when output type is panel. It can
* be 6,8,10 or 12. default config is decide by nexus.
*   - export output_color_mode=x to config the panel color mode to be x.
    - any other value or don't export output_color_mode
*     will config the panel set to default.
* NOTE - output_color_mode only take effect when output_type is panel.
*/
#define AVIO_CONFIG_PANEL_COLOR_MODE                "output_color_mode"
#define AVIO_CONFIG_PANEL_COLOR_MODE_6              "6"
#define AVIO_CONFIG_PANEL_COLOR_MODE_8              "8"
#define AVIO_CONFIG_PANEL_COLOR_MODE_10             "10"
#define AVIO_CONFIG_PANEL_COLOR_MODE_12             "12"

/* Is output color mode configured as x? */
#define AVIO_CONFIG_IS_PANEL_COLOR_MODE_6 \
    ((getenv(AVIO_CONFIG_PANEL_COLOR_MODE)) && \
    (strcmp(getenv(AVIO_CONFIG_PANEL_COLOR_MODE), AVIO_CONFIG_PANEL_COLOR_MODE_6) == 0))

#define AVIO_CONFIG_IS_PANEL_COLOR_MODE_8 \
    ((getenv(AVIO_CONFIG_PANEL_COLOR_MODE)) && \
    (strcmp(getenv(AVIO_CONFIG_PANEL_COLOR_MODE), AVIO_CONFIG_PANEL_COLOR_MODE_8) == 0))

#define AVIO_CONFIG_IS_PANEL_COLOR_MODE_10 \
    ((getenv(AVIO_CONFIG_PANEL_COLOR_MODE)) && \
    (strcmp(getenv(AVIO_CONFIG_PANEL_COLOR_MODE), AVIO_CONFIG_PANEL_COLOR_MODE_10) == 0))

#define AVIO_CONFIG_IS_PANEL_COLOR_MODE_12 \
    ((getenv(AVIO_CONFIG_PANEL_COLOR_MODE)) && \
    (strcmp(getenv(AVIO_CONFIG_PANEL_COLOR_MODE), AVIO_CONFIG_PANEL_COLOR_MODE_12) == 0))

/* export output_link_map_1 to config link1 map when output type is panel. It can
* be 0,1,2,3. default config is decide by nexus.
*   - export output_link_map_1=x to config the panel link map to be x.
    - any other value or don't export output_link_map_1
*     will config the panel set to default.
* NOTE - output_link_map_1 only take effect when output_type is panel.
*/
#define AVIO_CONFIG_PANEL_LINK_MAP_1                "output_link_map_1"
#define AVIO_CONFIG_PANEL_LINK_MAP_1_0              "0"
#define AVIO_CONFIG_PANEL_LINK_MAP_1_1              "1"
#define AVIO_CONFIG_PANEL_LINK_MAP_1_2              "2"
#define AVIO_CONFIG_PANEL_LINK_MAP_1_3              "3"

/* Is output link map configured as x? */
#define AVIO_CONFIG_IS_PANEL_LINK_MAP_1_0 \
    ((getenv(AVIO_CONFIG_PANEL_LINK_MAP_1)) && \
    (strcmp(getenv(AVIO_CONFIG_PANEL_LINK_MAP_1), AVIO_CONFIG_PANEL_LINK_MAP_1_0) == 0))

#define AVIO_CONFIG_IS_PANEL_LINK_MAP_1_1 \
    ((getenv(AVIO_CONFIG_PANEL_LINK_MAP_1)) && \
    (strcmp(getenv(AVIO_CONFIG_PANEL_LINK_MAP_1), AVIO_CONFIG_PANEL_LINK_MAP_1_1) == 0))

#define AVIO_CONFIG_IS_PANEL_LINK_MAP_1_2 \
    ((getenv(AVIO_CONFIG_PANEL_LINK_MAP_1)) && \
    (strcmp(getenv(AVIO_CONFIG_PANEL_LINK_MAP_1), AVIO_CONFIG_PANEL_LINK_MAP_1_2) == 0))

#define AVIO_CONFIG_IS_PANEL_LINK_MAP_1_3 \
    ((getenv(AVIO_CONFIG_PANEL_LINK_MAP_1)) && \
    (strcmp(getenv(AVIO_CONFIG_PANEL_LINK_MAP_1), AVIO_CONFIG_PANEL_LINK_MAP_1_3) == 0))

/* export output_link_map_2 to config link2 map when output type is panel. It can
* be 0,1,2,3. default config is decide by nexus.
*   - export output_link_map_2=x to config the panel link map to be x.
    - any other value or don't export output_link_map_2
*     will config the panel set to default.
* NOTE - output_link_map_2 only take effect when output_type is panel.
*/
#define AVIO_CONFIG_PANEL_LINK_MAP_2                "output_link_map_2"
#define AVIO_CONFIG_PANEL_LINK_MAP_2_0              "0"
#define AVIO_CONFIG_PANEL_LINK_MAP_2_1              "1"
#define AVIO_CONFIG_PANEL_LINK_MAP_2_2              "2"
#define AVIO_CONFIG_PANEL_LINK_MAP_2_3              "3"

/* Is output link map configured as x? */
#define AVIO_CONFIG_IS_PANEL_LINK_MAP_2_0 \
    ((getenv(AVIO_CONFIG_PANEL_LINK_MAP_2)) && \
    (strcmp(getenv(AVIO_CONFIG_PANEL_LINK_MAP_2), AVIO_CONFIG_PANEL_LINK_MAP_2_0) == 0))

#define AVIO_CONFIG_IS_PANEL_LINK_MAP_2_1 \
    ((getenv(AVIO_CONFIG_PANEL_LINK_MAP_2)) && \
    (strcmp(getenv(AVIO_CONFIG_PANEL_LINK_MAP_2), AVIO_CONFIG_PANEL_LINK_MAP_2_1) == 0))

#define AVIO_CONFIG_IS_PANEL_LINK_MAP_2_2 \
    ((getenv(AVIO_CONFIG_PANEL_LINK_MAP_2)) && \
    (strcmp(getenv(AVIO_CONFIG_PANEL_LINK_MAP_2), AVIO_CONFIG_PANEL_LINK_MAP_2_2) == 0))

#define AVIO_CONFIG_IS_PANEL_LINK_MAP_2_3 \
    ((getenv(AVIO_CONFIG_PANEL_LINK_MAP_2)) && \
    (strcmp(getenv(AVIO_CONFIG_PANEL_LINK_MAP_2), AVIO_CONFIG_PANEL_LINK_MAP_2_3) == 0))

/* export output_lvds_mode to config lvds mode when output type is panel. It can
* be single, dual or quad. default config is decide by nexus.
*   - export output_lvds_mode=single to config the panel lvds mode to be single.
*   - export output_lvds_mode=dual to config the panel to dual mode.
*   - export output_lvds_mode=quad to config the panel to quad mode.
    - any other value or don't export output_lvds_mode
*     will config the panel set to default.
* NOTE - output_lvds_mode only take effect when output_type is panel.
*/
#define AVIO_CONFIG_PANEL_LVDS_MODE                 "output_lvds_mode"
#define AVIO_CONFIG_PANEL_LVDS_MODE_SINGLE          "single"
#define AVIO_CONFIG_PANEL_LVDS_MODE_DUAL            "dual"
#define AVIO_CONFIG_PANEL_LVDS_MODE_QUAD            "quad"

/* Is output lvds mode configured as single? */
#define AVIO_CONFIG_IS_PANEL_LVDS_MODE_SINGLE \
    ((getenv(AVIO_CONFIG_PANEL_LVDS_MODE)) && \
    (strcmp(getenv(AVIO_CONFIG_PANEL_LVDS_MODE), AVIO_CONFIG_PANEL_LVDS_MODE_SINGLE) == 0))

/* Is output lvds mode configured as dual? */
#define AVIO_CONFIG_IS_PANEL_LVDS_MODE_DUAL \
    ((getenv(AVIO_CONFIG_PANEL_LVDS_MODE)) && \
    (strcmp(getenv(AVIO_CONFIG_PANEL_LVDS_MODE), AVIO_CONFIG_PANEL_LVDS_MODE_DUAL) == 0))

/* Is output lvds mode configured as single? */
#define AVIO_CONFIG_IS_PANEL_LVDS_MODE_QUAD \
    ((getenv(AVIO_CONFIG_PANEL_LVDS_MODE)) && \
    (strcmp(getenv(AVIO_CONFIG_PANEL_LVDS_MODE), AVIO_CONFIG_PANEL_LVDS_MODE_QUAD) == 0))
    
/* export bvn_usage to select the BVN usage mode configuration. Please refer to
* 3548 Usage Modes document. Right now we support config1/config4/config4 on 3548/3556A0.
* And config1/config2 on 3548/3556B0. More configs might be added in the future.
* So here we defined 8 modes for the future.
*   - export bvn_usage=config1 to select config1
*   - export bvn_usage=config2 to select config2
*   - export bvn_usage=config3 to select config3
*   - ...
*   - export bvn_usage=config8 to select config8
*
* Valid configs for 3548/3556A0:
*   - config1: FHD without PIP
*   - config4: WXGA with PIP
*   - config5: XGA without PIP
*
* Valid configs for 3548/3556B0:
*   - config1 WXGA without PIP
*   - config2 FHD without PIP
*
*/
#define AVIO_CONFIG_BVN_USAGE                       "bvn_usage"
#define AVIO_CONFIG_BVN_CONFIG                      "config"

#define AVIO_CONFIG_BVN_USAGE_CONFIG(n) AVIO_CONFIG_BVN_CONFIG#n
#define AVIO_CONFIG_BVN_USAGE_CONFIG1 AVIO_CONFIG_BVN_USAGE_CONFIG(1)
#define AVIO_CONFIG_BVN_USAGE_CONFIG2 AVIO_CONFIG_BVN_USAGE_CONFIG(2)
#define AVIO_CONFIG_BVN_USAGE_CONFIG3 AVIO_CONFIG_BVN_USAGE_CONFIG(3)
#define AVIO_CONFIG_BVN_USAGE_CONFIG4 AVIO_CONFIG_BVN_USAGE_CONFIG(4)
#define AVIO_CONFIG_BVN_USAGE_CONFIG5 AVIO_CONFIG_BVN_USAGE_CONFIG(5)
#define AVIO_CONFIG_BVN_USAGE_CONFIG6 AVIO_CONFIG_BVN_USAGE_CONFIG(6)
#define AVIO_CONFIG_BVN_USAGE_CONFIG7 AVIO_CONFIG_BVN_USAGE_CONFIG(7)
#define AVIO_CONFIG_BVN_USAGE_CONFIG8 AVIO_CONFIG_BVN_USAGE_CONFIG(8)
#define AVIO_CONFIG_BVN_USAGE_CONFIG9 AVIO_CONFIG_BVN_USAGE_CONFIG(9)    
#define AVIO_CONFIG_BVN_USAGE_CONFIG10 AVIO_CONFIG_BVN_USAGE_CONFIG(10)    
#define AVIO_CONFIG_BVN_USAGE_CONFIG11 AVIO_CONFIG_BVN_USAGE_CONFIG(11)    
#define AVIO_CONFIG_BVN_USAGE_CONFIG12 AVIO_CONFIG_BVN_USAGE_CONFIG(12)    

/* Is BVN usage mode selected as config1? */
#define AVIO_CONFIG_IS_BVN_USAGE_CONFIG(n) \
    ((getenv(AVIO_CONFIG_BVN_USAGE)) && \
    (strcmp(getenv(AVIO_CONFIG_BVN_USAGE), AVIO_CONFIG_BVN_USAGE_CONFIG(n)) == 0))

#define AVIO_CONFIG_IS_BVN_USAGE_CONFIG1 AVIO_CONFIG_IS_BVN_USAGE_CONFIG(1)
#define AVIO_CONFIG_IS_BVN_USAGE_CONFIG2 AVIO_CONFIG_IS_BVN_USAGE_CONFIG(2)
#define AVIO_CONFIG_IS_BVN_USAGE_CONFIG3 AVIO_CONFIG_IS_BVN_USAGE_CONFIG(3)
#define AVIO_CONFIG_IS_BVN_USAGE_CONFIG4 AVIO_CONFIG_IS_BVN_USAGE_CONFIG(4)
#define AVIO_CONFIG_IS_BVN_USAGE_CONFIG5 AVIO_CONFIG_IS_BVN_USAGE_CONFIG(5)
#define AVIO_CONFIG_IS_BVN_USAGE_CONFIG6 AVIO_CONFIG_IS_BVN_USAGE_CONFIG(6)
#define AVIO_CONFIG_IS_BVN_USAGE_CONFIG7 AVIO_CONFIG_IS_BVN_USAGE_CONFIG(7)
#define AVIO_CONFIG_IS_BVN_USAGE_CONFIG8 AVIO_CONFIG_IS_BVN_USAGE_CONFIG(8)
#define AVIO_CONFIG_IS_BVN_USAGE_CONFIG9 AVIO_CONFIG_IS_BVN_USAGE_CONFIG(9)
#define AVIO_CONFIG_IS_BVN_USAGE_CONFIG10 AVIO_CONFIG_IS_BVN_USAGE_CONFIG(10)
#define AVIO_CONFIG_IS_BVN_USAGE_CONFIG11 AVIO_CONFIG_IS_BVN_USAGE_CONFIG(11)
#define AVIO_CONFIG_IS_BVN_USAGE_CONFIG12 AVIO_CONFIG_IS_BVN_USAGE_CONFIG(12)
        
#ifdef __cplusplus
}
#endif


#endif /* AVIO_CONFIG_H__ */

/* End of file */
