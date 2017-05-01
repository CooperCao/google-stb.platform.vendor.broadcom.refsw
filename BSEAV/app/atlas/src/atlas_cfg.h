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

#ifndef ATLAS_CFG_H__
#define ATLAS_CFG_H__

#include "platform.h"
#include "mstringhash.h"

/* config macros to simplify get/set */
#define GET_NAME(CFG, NAME)      ((CFG)->getName(CConfiguration::NAME))
#define GET_DESCR(CFG, NAME)     ((CFG)->getDescription(CConfiguration::NAME))
#define GET_BOOL(CFG, NAME)      ((CFG)->getBool(CConfiguration::NAME))
#define GET_DOUBLE(CFG, NAME)    ((CFG)->getDouble(CConfiguration::NAME))
#define GET_INT(CFG, NAME)       ((CFG)->getInt(CConfiguration::NAME))
#define GET_STR(CFG, NAME)       ((CFG)->get(CConfiguration::NAME))
#define SET(CFG, NAME, VALUE)    ((CFG)->set(CConfiguration::NAME, VALUE))

#ifndef ATLAS_DECLARE_CONFIGSETTING_VALUES
#define MAX_CONFIGSETTING_VALUE 256
#define ATLAS_DECLARE(SETTING,TYPE,DEFAULT,DESCRIPTION) SETTING

/*=*********************************
Config is like the "Windows Registry" for atlas.
It contains all configuration information for the application.
Some config values can be set by command line parameters.

The command line does not override the config file.

Data storage is name, value pairs.
There are predefined names which can be accessed with an enum. This gives the application
some compile-time checking without eliminating the ability to do programmatic access
(e.g. using sprintf to have an indexed setting.)

It stores and retrives string, integer, float and boolean data types.
Data conversion is implicit. Native storage format is a string.
Boolean true is defined as any string which begins with 't' or 'T', or which
is converted to a numeric non-zero value.

***********************************/

/* In this mode, define the enum and class. */
class CConfiguration {
private:
    struct Predefined {
        // enum is the index into the array
        const char * _name;
        const char * _type;
        const char * _default;
        const char * _description;
    };
public:
enum {

#else // ATLAS_DECLARE_CONFIGSETTING_VALUES

/* In this mode, define the data structure. */
#define ATLAS_DECLARE(SETTING,TYPE,DEFAULT,DESCRIPTION) {#SETTING, #TYPE, DEFAULT, DESCRIPTION}
CConfiguration::Predefined CConfiguration::_predefines[] = {
#endif


/* Here's the dual-purpose code */
    ATLAS_DECLARE(ATLAS_NAME,                              string, "Atlas",                     "Name of the Atlas application.  This value is used for the process name when using Atlas as an NxClient.  This name will also be used when broadcasting Atlas server name to clients."),
    ATLAS_DECLARE(BOARD_NAME,                              string, "Unknown",                   "Name of the board. This is filled in at runtime"),
    ATLAS_DECLARE(ATLAS_VERSION_MAJOR,                     int,    "14",                        "Atlas Major version number."),
    ATLAS_DECLARE(ATLAS_VERSION_MINOR,                     int,    "3",                         "Atlas Minor version number."),
    ATLAS_DECLARE(XML_VERSION_MAJOR_CHLIST,                int,    "3",                         "Channel List Major XML version number. XML version with the same major number are backwards compatible."),
    ATLAS_DECLARE(XML_VERSION_MINOR_CHLIST,                int,    "0",                         "Channel List Minor XML version number. XML version with the same major number are backwards compatible."),
    ATLAS_DECLARE(XML_VERSION_MAJOR_UDPLIST,               int,    "1",                         "UDP URL List Major XML version number. XML version with the same major number are backwards compatible."),
    ATLAS_DECLARE(XML_VERSION_MINOR_UDPLIST,               int,    "0",                         "UDP URL List Minor XML version number. XML version with the same major number are backwards compatible."),
    ATLAS_DECLARE(XML_VERSION_MAJOR_NFO,                   int,    "10",                        ".nfo file Major XML version number. XML version with the same major number are backwards compatible."),
    ATLAS_DECLARE(XML_VERSION_MINOR_NFO,                   int,    "0",                         ".nfo file Minor XML version number. XML version with the same major number are backwards compatible."),
    ATLAS_DECLARE(EXIT_APPLICATION,                        bool,   "false",                     "Gracefully quit the application."),
    ATLAS_DECLARE(FIRST_TUNE,                              bool,   "true",                      "Tune to current channel in idle loop."),
    ATLAS_DECLARE(ENABLE_IDLE_TUNE,                        bool,   "false",                     "Enable auto-tuning in idle loop. Default is false"),
    ATLAS_DECLARE(ENABLE_LUA,                              bool,   "true",                      "Enable Lua command prompt."),
    ATLAS_DECLARE(CONFIG_FILENAME,                         string, "atlas.cfg",                 "Filename of Atlas configuration file (contains default settings)"),
    ATLAS_DECLARE(GRAPHICS_SURFACE_WIDTH,                  int,    "960",                       "Width of the display's graphics video surface"),
    ATLAS_DECLARE(GRAPHICS_SURFACE_HEIGHT,                 int,    "540",                       "Height of the display's graphics video surface"),
    ATLAS_DECLARE(FRAMEBUFFER_SURFACE_WIDTH_HD,            int,    "1920",                      "Width of the display's HD graphics video framebuffer"),
    ATLAS_DECLARE(FRAMEBUFFER_SURFACE_HEIGHT_HD,           int,    "1080",                      "Height of the display's HD graphics video framebuffer"),
    ATLAS_DECLARE(FRAMEBUFFER_SURFACE_WIDTH_SD,            int,    "720",                       "Width of the display's SD graphics video framebuffer"),
    ATLAS_DECLARE(FRAMEBUFFER_SURFACE_HEIGHT_SD,           int,    "576",                       "Height of the display's SD graphics video framebuffer"),
    ATLAS_DECLARE(PREFERRED_FORMAT_SD,                     int,    "NTSC",                      "Preferred video format for SD outputs when HDMI is disconnected or non-existent (see stringToVideoFormat)"),
    ATLAS_DECLARE(PREFERRED_FORMAT_HD,                     int,    "1080i",                     "Preferred video format for HD outputs when HDMI is disconnected or non-existent (see stringToVideoFormat)"),
    ATLAS_DECLARE(PREFERRED_NTSC,                          int,    "NTSC",                      "Preferred type of NTSC (see stringToVideoFormat())"),
    ATLAS_DECLARE(PREFERRED_PAL,                           int,    "Pal-G",                     "Preferred type of Pal (see stringToVideoFormat())"),
    ATLAS_DECLARE(VIDEO_WINDOW_WIDTH,                      int,    "1920",                      "Default width of the display's video windows"),
    ATLAS_DECLARE(VIDEO_WINDOW_HEIGHT,                     int,    "1080",                      "Default width of the display's video windows"),
    ATLAS_DECLARE(WINDOW_BACKGROUND_COLOR,                 int,    "0x00000000",                "Default windows background color"),
    ATLAS_DECLARE(SPDIF_OUTPUT_DELAY,                      int,    "0",                         "Additional delay for SPDIF outputs in millisecs"),
    ATLAS_DECLARE(HDMI_OUTPUT_DELAY,                       int,    "0",                         "Additional delay for hdmi audio outputs in millisecs"),
    ATLAS_DECLARE(HDMI_IGNORE_HOTPLUG,                     bool,   "false",                     "Ignore the HDMI hotplug events"),
    ATLAS_DECLARE(DAC_OUTPUT_DELAY,                        int,    "0",                         "Additional delay for dac audio outputs in millisecs"),
    ATLAS_DECLARE(VIDEO_DECODER_FIFO_SIZE,                 int,    "0",                         "Size of the video decoder FIFO in kilobytes"),
    ATLAS_DECLARE(VIDEO_DECODER_IP_FIFO_SIZE,              int,    "10485760",                  "Size of the video decoder with IP Jitter FIFO in kilobytes 10*1024*1024"),
    ATLAS_DECLARE(AUDIO_DECODER_FIFO_SIZE,                 int,    "0",                         "Size of the audio decoder FIFO in kilobytes"),
    ATLAS_DECLARE(AUDIO_DECODER_IP_FIFO_SIZE,              int,    "1048576",                   "Size of the audio decoder with IP Jitter FIFO in kilobytes 2*512*1024"),
    ATLAS_DECLARE(TUNE_QAM_TIMEOUT,                        int,    "3000",                      "Length of QAM tune lock timeout in millisecs"),
    ATLAS_DECLARE(TUNE_QAM_STATUS_TIMEOUT,                 int,    "500",                       "Length of QAM tune async status timeout in millisecs"),
    ATLAS_DECLARE(TUNE_QAM_SCAN_TIMEOUT,                   int,    "500",                       "Length of QAM tune lock timeout in millisecs during channel scan"),
    ATLAS_DECLARE(TUNE_QAM_PAT_TIMEOUT,                    int,    "500",                       "PAT Timeout in Millisecs "),
    ATLAS_DECLARE(TUNE_QAM_PMT_TIMEOUT,                    int,    "500",                       "PMT Timeout in Millisecs "),
    ATLAS_DECLARE(TUNE_OFDM_TIMEOUT,                       int,    "3000",                      "Length of OFDM tune lock timeout in millisecs"),
    ATLAS_DECLARE(TUNE_OFDM_SCAN_TIMEOUT,                  int,    "1000",                      "Length of OFDM tune lock timeout in millisecs during channel scan"),
    ATLAS_DECLARE(TUNE_OFDM_PAT_TIMEOUT,                   int,    "5000",                      "PAT Timeout in Millisecs "),
    ATLAS_DECLARE(TUNE_OFDM_PMT_TIMEOUT,                   int,    "5000",                      "PMT Timeout in Millisecs "),
    ATLAS_DECLARE(TUNE_OFDM_STATUS_TIMEOUT,                int,    "500",                       "Async OFDM tuner status timeout in Millisecs"),
    ATLAS_DECLARE(TUNE_VSB_TIMEOUT,                        int,    "3000",                      "Length of VSB tune lock timeout in millisecs"),
    ATLAS_DECLARE(TUNE_VSB_SCAN_TIMEOUT,                   int,    "500",                       "Length of VSB tune lock timeout in millisecs during channel scan"),
    ATLAS_DECLARE(TUNE_SAT_TIMEOUT,                        int,    "5000",                      "Length of SAT tune lock timeout in millisecs"),
    ATLAS_DECLARE(DEFERRED_CHANNEL_CHANGE_UP_DOWN_TIMEOUT, int,    "500",                       "Length of deferred channel change timeout for ch up/down in millisecs"),
    ATLAS_DECLARE(DEFERRED_CHANNEL_CHANGE_10_KEY_TIMEOUT,  int,    "1000",                      "Length of deferred channel change timeout for 10 key in millisecs"),
    ATLAS_DECLARE(UI_TIMELINE_TIMEOUT,                     int,    "3000",                      "Timeout period for user interface audio/video buffer panel in millisecs (-1 = forever)"),
    ATLAS_DECLARE(UI_BUFFERS_UPDATE_TIMEOUT,               int,    "333",                       "Update period for user interface audio/video buffer updates in millisecs"),
    ATLAS_DECLARE(UI_CONNECTION_STATUS_TIMEOUT,            int,    "3000",                      "Display period for user interface Atlas server connect/disconnect events in millisecs"),
    ATLAS_DECLARE(UI_GRID_UPDATE_TIMEOUT,                  int,    "500",                       "Update period for Grid points used in Constellation update in millisecs"),
    ATLAS_DECLARE(GRID_POINT_MAX,                          int,    "30",                        "Maximum number of Grid Points in constellation"),
    ATLAS_DECLARE(UI_TUNER_UPDATE_TIMEOUT,                 int,    "500",                       "Update period for tuner statistics panel in millisecs"),
    ATLAS_DECLARE(UI_NETWORK_WIFI_UPDATE_TIMEOUT,          int,    "5000",                      "Update period for wifi network panel in millisecs"),
    ATLAS_DECLARE(UI_NETWORK_WIFI_SCAN_TIMEOUT,            int,    "10000",                     "Update period for wifi network panel scan in millisecs"),
    ATLAS_DECLARE(UI_BLUETOOTH_UPDATE_TIMEOUT,             int,    "5000",                      "Update period for bluetooth network panel in millisecs"),
    ATLAS_DECLARE(UI_QAM_SCAN_START_FREQ,                  int,    "57000000",                  "QAM scan start frequency in Hz"),
    ATLAS_DECLARE(UI_QAM_SCAN_END_FREQ,                    int,    "789000000",                 "QAM scan end frequency in Hz"),
    ATLAS_DECLARE(TUNER_LOCK_CHECK_TIMEOUT,                int,    "2000",                      "Length of time between check for tuner lock in millisecs"),
    ATLAS_DECLARE(POWER_ON_TIMEOUT,                        int,    "3000",                      "If powering Off with timeout, length of time before power On in millisecs"),
    ATLAS_DECLARE(DROP_FIELD_MODE_ENABLED,                 bool,   "false",                     "If true, when the video decoder needs to drop pictures because of TSM, it will drop only single fields.  Otherwise it will drop field pairs."),
    ATLAS_DECLARE(DECODER_COLOR_DEPTH,                     int,    "8",                         "8 or 10 bit color is enabled for the video decoder."),
    ATLAS_DECLARE(AUDIO_DECODER_MULTICH_FORMAT,            int,    "1",                         "See NEXUS_AudioMultichannelFormat enum definition for valid values"),
    ATLAS_DECLARE(AUDIO_CAPTURE,                           bool,   "false",                     "Capture audio PCM data"),
    ATLAS_DECLARE(AUDIO_CAPTURE_COMPRESSED,                bool,   "false",                     "Capture compressed audio PCM data"),
    ATLAS_DECLARE(AUDIO_DOLBY_DRC_AC3_BOOST,               int,    "100",                       "Dolby Dynamic Range Control boost value for moderate setting"),
    ATLAS_DECLARE(AUDIO_DOLBY_DRC_AC3_CUT,                 int,    "100",                       "Dolby Dynamic Range Control cut value for moderate setting"),
    ATLAS_DECLARE(MPAA_ENABLED,                            bool,   "false",                     "Enable MPAA if available"),
    ATLAS_DECLARE(USE_FIRST_PTS,                           bool,   "false",                     "The STC will be driven by either the video or audio PTS, depending on stream muxing and error conditions."),
    ATLAS_DECLARE(PRECISION_LIPSYNC_ENABLED,               bool,   "true",                      "Enables subframe audio adjustments based on video feedback"),
    ATLAS_DECLARE(ASTM_ENABLED,                            bool,   "false",                     "Enables ASTM support in simple STC"),
    ATLAS_DECLARE(LUA_PROMPT,                              string, "atlas lua> ",               "Prompt string used by Lua scripting engine"),
    ATLAS_DECLARE(LUA_SCRIPT       ,                       string, "",                          "Script to run on startup."),
    ATLAS_DECLARE(LUA_HISTORY_FILENAME,                    string, "lua_history.txt",           "Filename used by Lua scripting engine to save command history"),
    ATLAS_DECLARE(CHANNELS_LIST,                           string, "channels.xml",              "Filename containing the default channel list which is read in on startup"),
    ATLAS_DECLARE(SCAN_ENCRYPTED_CHANNELS,                 bool,   "false",                     "Add found encrypted channels to the channel list during scan."),
    ATLAS_DECLARE(SCAN_AUDIO_ONLY_CHANNELS,                bool,   "false",                     "Add found audio only channels to the channel list during scan."),
    ATLAS_DECLARE(SCAN_DUPLICATE_CHANNELS,                 bool,   "false",                     "Add found duplicate channels to the channel list during scan."),
    ATLAS_DECLARE(UI_CHANNEL_NUM_TIMEOUT,                  int,    "3000",                      "Length of time to display channel number on screen after it is updated in millisecs"),
    ATLAS_DECLARE(UI_VOLUME_TIMEOUT,                       int,    "1000",                      "Length of time to display volume on screen after it is updated in millisecs"),
    ATLAS_DECLARE(VIDEOS_PATH,                             string, "videos",                    "Path where Videos are stored"),
    ATLAS_DECLARE(INFO_PATH,                               string, "videos",                    "Path where nfo files are stored"),
    ATLAS_DECLARE(INDEX_PATH,                              string, "videos",                    "Path where pvr index files are stored"),
    ATLAS_DECLARE(SCRIPTS_PATH,                            string, "scripts",                   "Path where Lua scripts are stored"),
    ATLAS_DECLARE(FORCE_BCM_TRICK_MODES,                   bool,   "false",                     "Force Atlas to use only BRCM trickmodes"),
    ATLAS_DECLARE(FORCE_HOST_TRICK_MODES,                  bool,   "false",                     "Force Atlas to use only HOST trickmodes"),
    ATLAS_DECLARE(ALLOW_ALL_AVC_TRICK_MODES,               bool,   "false",                     "Allow all AVC trickmodes in Atlas"),
    ATLAS_DECLARE(FORCE_IFRAME_TRICK_MODES,                bool,   "false",                     "Force Atlas to only do I frame trickmodes"),
    ATLAS_DECLARE(DQT_ENABLED,                             bool,   "false",                     "Enable DQT trickmodes for AVC streams"),
    ATLAS_DECLARE(DQT_PICS_PER_GOP,                        int,    "10",                        "Number of Pics per DQT"),
    ATLAS_DECLARE(TRICK_MODE_MAX_DECODE_RATE,              float,  "2.0",                       "Maximum trickmode rate for audio decode. Recommended range 1.2 to 2.0"),
    ATLAS_DECLARE(TRICK_MODE_RATE_MAX,                     int,    "512",                       "Maximum trickmode rate for video"),
    ATLAS_DECLARE(TRICK_MODE_RATE_MIN,                     int,    "-512",                      "Minimum trickmode rate for video"),
    ATLAS_DECLARE(TRICK_MODE_SEEK,                         int,    "30",                        "Seek Time in Seconds"),
    ATLAS_DECLARE(VIDEODECODE_ENABLED,                     bool,   "true",                      "Enable decode for video pids"),
    ATLAS_DECLARE(AUDIODECODE_ENABLED,                     bool,   "true",                      "Enable decode for audio pids"),
    ATLAS_DECLARE(REMOTE_TYPE_PRIMARY,                     string, "Broadcom",                  "Primary remote type [OneForAll, Broadcom || CirEchoStar]"),
    ATLAS_DECLARE(REMOTE_TYPE_SECONDARY,                   string, "DishUhf",                   "Secondary remote type [DishUhf | DirectvUhf]"),
    ATLAS_DECLARE(VOLUME_STEPS,                            int,    "20",                        "Number of volume steps from min to max"),
    ATLAS_DECLARE(PIP_PERCENTAGE,                          int,    "250",                       "Percentage to shrink PIP window based on full screen window 0-1000 = 0-100.0%"),
    ATLAS_DECLARE(PIP_BORDER_PERCENTAGE,                   int,    "80",                        "Percentage of border between PIP and main window 0-1000 = 0-100.0%"),
    ATLAS_DECLARE(PIP_POSITION,                            int,    "3",                         "PIP window location 0:upper right, 1: upper left, 2: lower left, 3: lower right"),
    ATLAS_DECLARE(PIP_INITIAL_CHANNEL,                     int,    "-1",                        "Channel that PIP initially tunes to when opened.  -1 indicates default action which is to tune to same channel as main."),
    ATLAS_DECLARE(IP_NETWORK_MAX_JITTER,                   int,    "300",                       "Max Network IP Jitter used in timebase"),
    ATLAS_DECLARE(MAXDATARATE_PARSERBAND,                  int,    "0",                         "Adjust maximum parserband data rate for all parserbands"),
    ATLAS_DECLARE(MAXDATARATE_PLAYBACK,                    int,    "0",                         "Adjust maximum playpump data rate for first playback"),
    ATLAS_DECLARE(DAC_BANDGAP_ADJUST,                      int,    "0",                         "Adjust video DAC amplification factor (bandgap)"),
    ATLAS_DECLARE(DAC_DETECTION,                           int,    "0",                         "Adjust video DAC detection 0:auto 1:off 2:on"),
    ATLAS_DECLARE(AVC_51_SUPPORT,                          bool,   "false",                     "Enable AVC level 5.1 support in video decoder"),
    ATLAS_DECLARE(PLAYBACK_BEGIN_STREAM_ACTION,            int,    "2",                         "Action that occurs when the beginning of a stream is reached during playback (0:loop, 1:pause, 2:play)"),
    ATLAS_DECLARE(PLAYBACK_END_STREAM_ACTION,              int,    "0",                         "Action that occurs when the end of a stream is reached during playback (0:loop, 1:pause)"),
    ATLAS_DECLARE(DCC_ENABLED,                             bool,   "true",                      "Enable Closed Caption parsing engine"),
    ATLAS_DECLARE(SNMP_ENABLED,                            bool,   "true",                      "Enable SNMP"),
    ATLAS_DECLARE(RESOURCE_NUM_DISPLAYS,                   int,    "-1",                        "Override num of display resources reported by Nexus.  -1 disables override."),
    ATLAS_DECLARE(RESOURCE_NUM_GRAPHICS,                   int,    "-1",                        "Override num of graphics resources reported by Nexus.  -1 disables override."),
    ATLAS_DECLARE(RESOURCE_NUM_SIMPLE_VIDEO_DECODERS,      int,    "-1",                        "Override num of simple video decoder resources reported by Nexus.  -1 disables override."),
    ATLAS_DECLARE(RESOURCE_NUM_SIMPLE_AUDIO_DECODERS,      int,    "-1",                        "Override num of simple audio decoder resources reported by Nexus.  -1 disables override."),
    ATLAS_DECLARE(RESOURCE_NUM_STILL_DECODERS,             int,    "-1",                        "Override num of still decoder resources reported by Nexus.  -1 disables override."),
    ATLAS_DECLARE(RESOURCE_NUM_STREAMERS,                  int,    "-1",                        "Override num of streamer resources reported by Nexus.  -1 disables override."),
    ATLAS_DECLARE(RESOURCE_NUM_REMOTES_IR,                 int,    "-1",                        "Override num of ir remotes resources reported by Nexus.  -1 disables override."),
    ATLAS_DECLARE(RESOURCE_NUM_REMOTES_RF4CE,              int,    "-1",                        "Override num of rf4ce remotes resources reported by Nexus.  -1 disables override."),
    ATLAS_DECLARE(RESOURCE_NUM_REMOTES_BLUETOOTH,          int,    "-1",                        "Override num of bluetooth remotes resources reported by Nexus.  -1 disables override."),
    ATLAS_DECLARE(RESOURCE_NUM_REMOTES_UHF,                int,    "-1",                        "Override num of uhf remotes resources reported by Nexus.  -1 disables override."),
    ATLAS_DECLARE(RESOURCE_NUM_FRONTENDS,                  int,    "-1",                        "Override num of frontend resources reported by Nexus.  -1 disables override."),
    ATLAS_DECLARE(RESOURCE_NUM_OUTPUTS_SPDIF,              int,    "-1",                        "Override num of spdif output resources reported by Nexus.  -1 disables override."),
    ATLAS_DECLARE(RESOURCE_NUM_OUTPUTS_DAC,                int,    "-1",                        "Override num of audio dac output resources reported by Nexus.  -1 disables override."),
    ATLAS_DECLARE(RESOURCE_NUM_OUTPUTS_DAC_I2S,            int,    "-1",                        "Override num of audio I2S dac output resources reported by Nexus.  -1 disables override."),
    ATLAS_DECLARE(RESOURCE_NUM_OUTPUTS_HDMI,               int,    "-1",                        "Override num of hdmi resources reported by Nexus.  -1 disables override."),
    ATLAS_DECLARE(RESOURCE_NUM_OUTPUTS_COMPONENT,          int,    "-1",                        "Override num of component resources reported by Nexus.  -1 disables override."),
    ATLAS_DECLARE(RESOURCE_NUM_OUTPUTS_COMPOSITE,          int,    "-1",                        "Override num of composite resources reported by Nexus.  -1 disables override."),
    ATLAS_DECLARE(RESOURCE_NUM_OUTPUTS_RFM,                int,    "-1",                        "Override num of rfm resources reported by Nexus.  -1 disables override."),
    ATLAS_DECLARE(DNR_DEFAULT_BNR_MODE,                    int,    "-1",                        "Default mode for DNR block noise reduction -1:nexus default 0:disable, 1:bypass, 2:enabled"),
    ATLAS_DECLARE(DNR_DEFAULT_MNR_MODE,                    int,    "-1",                        "Default mode for DNR mosquito noise reduction -1:nexus default 0:disable, 1:bypass, 2:enabled"),
    ATLAS_DECLARE(DNR_DEFAULT_DCR_MODE,                    int,    "-1",                        "Default mode for DNR digital contour reduction -1:nexus default 0:disable, 1:bypass, 2:enabled"),
    ATLAS_DECLARE(ANR_DEFAULT_MODE,                        int,    "-1",                        "Default mode for ANR 0:disable, 1:bypass, 2:enabled"),
    ATLAS_DECLARE(POWER_STATE,                             int,    "0",                         "Indicates the current power mode"),
    ATLAS_DECLARE(POWER_MGMT_SATA,                         bool,   "true",                      "Enable SATA power management"),
    ATLAS_DECLARE(POWER_MGMT_USB,                          bool,   "true",                      "Enable USB power management"),
    ATLAS_DECLARE(POWER_MGMT_TP1,                          bool,   "true",                      "Enable TP1 power management"),
    ATLAS_DECLARE(POWER_MGMT_TP2,                          bool,   "true",                      "Enable TP2 power management"),
    ATLAS_DECLARE(POWER_MGMT_TP3,                          bool,   "true",                      "Enable TP3 power management"),
    ATLAS_DECLARE(POWER_MGMT_DDR,                          bool,   "true",                      "Enable DDR power management"),
    ATLAS_DECLARE(POWER_MGMT_MEMC1,                        bool,   "true",                      "Enable memc1 power management"),
    ATLAS_DECLARE(POWER_MGMT_ENET,                         bool,   "false",                     "Enable ethernet power management"),
    ATLAS_DECLARE(POWER_MGMT_MOCA,                         bool,   "true",                      "Enable moca power management"),
    ATLAS_DECLARE(POWER_MGMT_CPU_DIVISOR,                  int,    "8",                         "CPU divisior used for S1/S2/S3 power modes"),
    ATLAS_DECLARE(RFM_CHANNEL_NUM,                         int,    "3",                         "Default RFM channel number"),
    ATLAS_DECLARE(DEINTERLACER_ENABLED_SD,                 bool,   "false",                     "Deinterlacer settings apply only to the HD displays unless this setting is true"),
    ATLAS_DECLARE(TTS_AUTO_DETECT,                         bool,   "true",                      "Automatically detect if the stream is TTS or TS"),
    ATLAS_DECLARE(TTS_PACING_MAX_ERROR,                    int,    "2636",                      "Set the timestamp error bound, as used by the playback pacing logic"),
    ATLAS_DECLARE(TTS_INIT_BUF_DEPTH,                      int,    "625000",                    "Initial buffer depth to achieve before starting playback"),
    ATLAS_DECLARE(TTS_MIN_BUF_DEPTH,                       int,    "125000",                    "Minimum buffer depth before tts throttle buffer depth violation"),
    ATLAS_DECLARE(TTS_MAX_BUF_DEPTH,                       int,    "1250000",                   "Maximum buffer depth before tts throttle buffer depth violation"),
    ATLAS_DECLARE(TTS_MAX_CLOCK_MISMATCH,                  int,    "100",                       "Specify the maximum clock mismatch between server/encoder and STB"),
    ATLAS_DECLARE(NETWORK_DB_PATH,                         bool,   "/data/netapp",              "Location of the NetApp database (caches previous WiFi network) - do not use NFS mounted directory"),
    ATLAS_DECLARE(ATLAS_SERVER_ENABLED,                    bool,   "true",                      "If true,then it enables atlas server."),
    ATLAS_DECLARE(HTTP_SERVER_LISTENING_PORT,              string, "8089",                      "Port on which Http Server Listens for HTTP Requests from clients."),
    ATLAS_DECLARE(HTTP_SERVER_INTERFACE_NAME,              string, "",                          "Binds Media Server to this specific interface name.Defaults to none, meaning HttpServer will listen on all interfaces."),
    ATLAS_DECLARE(HTTP_SERVER_MAX_CONCURRENT_REQUEST,      int,    "16",                        "Maximum HTTP Requests HttpServer will queue & will not accept anymore new connections Request."),
    ATLAS_DECLARE(HTTP_SERVER_PERSISTENT_TIMEOUT_IN_MS,    int,    "5000",                      "Non-zero timeout values enables HTTP Persistent Connection. Timeout duration is in msec after which idle connection will be timed out."),
    ATLAS_DECLARE(HTTP_SERVER_ENABLE_HW_PACING,            bool,   "true",                      "If true, BIP will use File -> playpump -> recpump -> network path to enable h/w based pacing."),
    ATLAS_DECLARE(HTTP_SERVER_ENABLE_DTCP,                 bool,   "false",                     "To stream media using DTCP/IP, this should be set to true."),
    ATLAS_DECLARE(HTTP_SERVER_DTCP_AKE_PORT,               string, "8000",                      "DTCP AKE PORT"),
    ATLAS_DECLARE(HTTP_SERVER_DTCP_KEY_TYPE,               string, "commonDRM",                 "DTCP_IP communication key. [test | production | commonDRM]"),
    ATLAS_DECLARE(HTTP_SERVER_PLAYLIST_PORT,               string, "89",                        "Port on which Http Server Listens for Playlist Requests from clients."),
    ATLAS_DECLARE(AUTO_DISCOVERY_BEACON_MCAST_ADDRESS,     string, "239.99.99.99",              "udp multicast address for autodiscovery server and client. Change this to create a separate atlas server cleint envirnment on the same network."),
    ATLAS_DECLARE(AUTO_DISCOVERY_BEACON_MCAST_PORT,        int,    "9999",                      "udp port number for autodiscovery server and client. Change this if there is port number conflict."),
    ATLAS_DECLARE(AUTO_DISCOVERY_BEACON_INTERVAL,          int,    "5000",                      "In msec, determines how frequently autodiscovery beacons are sent, make this higher if too many atlas found in the network."),
    ATLAS_DECLARE(AUTO_DISCOVERY_CLIENT_ENABLED,           bool,   "true",                      "Turn on off atlas auto discovery client"),
    ATLAS_DECLARE(AUTO_DISCOVERY_SERVER_ENABLED,           bool,   "true",                      "Turn on off atlas auto discovery client"),
    ATLAS_DECLARE(GENERATE_INDEXES,                        bool,   "true",                      "Turn on off atlas threaded auto index generation for existing playback files"),
    ATLAS_DECLARE(UDP_URL_LIST,                            string, "udpUrl.xml",                "Name of the list of UDP/RTP URLS to stream out"),
    ATLAS_DECLARE(WPA_SUPPLICANT_IFACE_PATH,               string, "/var/run/wpa_supplicant",   "Path to the WPA Supplicant directory containing the interface to use"),
    ATLAS_DECLARE(WPA_SUPPLICANT_STATIC_IP,                string, "0.0.0.0",                   "Static ip to be used in lieu of dhcp.  If 0.0.0.0 is specified, then use dhcp"),
    ATLAS_DECLARE(WPA_SUPPLICANT_STATIC_NETMASK,           string, "255.255.255.0",             "Netmask to be used only if WPA_SUPPLICANT_STATIC_IP contains a valid value"),
    ATLAS_DECLARE(CPUTEST_MAX_DELAY,                       int,    "250",                       "Maximum delay in cputest jpeg decompress loop"),
    ATLAS_DECLARE(IMAGE_CHANNEL_SDR,                       string, "sdr_1080.jpg",              "SDR image"),
    ATLAS_DECLARE(IMAGE_CHANNEL_HDR10,                     string, "hdr10_1080.jpg",            "HDR-10 image"),
    ATLAS_DECLARE(IMAGE_CHANNEL_HLG,                       string, "hlg_1080.jpg",              "HLG image"),
    ATLAS_DECLARE(IMAGE_CHANNEL_DOLBYVISION,               string, "ddnarrow_1080.jpg",         "DOLBY VISION image"),
    ATLAS_DECLARE(IMAGE_PLM_ENABLED,                       string, "plm-enabled_1080.jpg",      "PLM image - disabled"),
    ATLAS_DECLARE(IMAGE_PLM_DISABLED,                      string, "plm-disabled_1080.jpg",     "PLM image - enabled"),
    ATLAS_DECLARE(IMAGE_PLM_PASSTHRU,                      string, "plm-passthru_1080.jpg",     "PLM image passthru"),
    ATLAS_DECLARE(FORCE_HDMI_HDR_OUTPUT,                   bool,   "false",                     "Force HDMI HDR output even if TV does not support it")
};

#ifndef ATLAS_DECLARE_CONFIGSETTING_VALUES

    CConfiguration(const char *filename = NULL);
    CConfiguration(const CConfiguration & cfg);

    virtual ~CConfiguration();
    void initialize(void);
    int read(const char *filename);
    void print() const {_pRegistry->print();}
    void printHelp() const;
    CPlatform * getPlatformConfig(void) { return &_platformConfig; };

    /**
    * Predefines
    **/
    static int total();
    static const char *getName(int index);
    static const char *getDescription(int index);

    const char *get(const char *name, const char *defaultvalue = NULL) const;
    int getInt(const char *name, int defaultvalue = 0) const;
    double getDouble(const char *name, double defaultvalue = 0.0) const;
    bool getBool(const char *name, bool defaultvalue = false) const;

    void set(const char *name, const char *value);
    void set(const char *name, double value);
    void set(const char *name, int value);

    /**
    * Convert index to name
    **/
    const char *get(int predefinedIndex, const char *defaultvalue = NULL) const
        {return get(getName(predefinedIndex), defaultvalue);}
    int getInt(int predefinedIndex, int defaultvalue = 0) const
        {return getInt(getName(predefinedIndex), defaultvalue);}
    double getDouble(int predefinedIndex, double defaultvalue = 0.0) const
        {return getDouble(getName(predefinedIndex), defaultvalue);}
    bool getBool(int predefinedIndex, bool defaultvalue = false) const
        {return getBool(getName(predefinedIndex), defaultvalue);}
    void set(int predefinedIndex, const char *value)
        {set(getName(predefinedIndex), value);}
    void set(int predefinedIndex, double value)
        {set(getName(predefinedIndex), value);}
    void set(int predefinedIndex, int value)
        {set(getName(predefinedIndex), value);}

private:
    static Predefined _predefines[];
    CPlatform _platformConfig;
    MStringHash * _pRegistry;
};
#endif

#endif /* ATLAS_CFG_H__ */
