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

#include <stdio.h>
#include "cmd_parsing.h"

BDBG_MODULE(cmd_parsing);

/* Globals */
/* Commands list : List of command line options supported
 * To add a new command to the list of options, add an element (int, bool, string) to the
 * structure BIP_Play_Options and then add an entry to the list below. Each entry specifies
 * 1. The element name
 * 2. Optional argument string,
 * 3. Default value
 * 4. Brief Description.
 * 5. Parsing function - BIP_Play_CommandParse_<element_name>(char * argument, BIP_Play_Options *options)
 * The parsing function needs to be defined. This function is expected to parse the argument specified
 * and set the element in the options structure.
 * The following macros can be used to add a new element to the commands list
 * - BIP_PLAY_STRING_COMMAND_OPTION(elementName, argumentString, defaultValue, description)
 * - BIP_PLAY_INTEGER_COMMAND_OPTION(elementName, argumentString, defaultValue, description)
 * - BIP_PLAY_BOOL_COMMAND_OPTION(elementName, defaultValue, description)
 */
BIP_Play_CommandInfo g_commandsList[] = {
                                   /* Element name, Value string, Default Value, Description */
    BIP_PLAY_STRING_COMMAND_OPTION(interface, "<name>", NULL, "Optional Interface name to bind the player to"),
    BIP_PLAY_INTEGER_COMMAND_OPTION(pos, "<msec>", 0, "Initial Position to start Playback from"),
    BIP_PLAY_INTEGER_COMMAND_OPTION(jump, "<msec>", BIP_PLAY_DEFAULT_JUMP_OFFSET, "Time Position to use with runtime jump (jf/jb) option"),
    BIP_PLAY_BOOL_COMMAND_OPTION(loop, OFF, "Continuous looping after reach End of stream"),
    BIP_PLAY_INTEGER_COMMAND_OPTION(playTime, "<msec>", BIP_PLAY_DEFAULT_PLAYBACK_DURATION, "Playback duration (ms) - default is -1 : up to end of stream"),
    BIP_PLAY_BOOL_COMMAND_OPTION(printStatus, OFF, "Print periodic status"),
    BIP_PLAY_BOOL_COMMAND_OPTION(printServerStats, OFF, "Print periodic server status"),
    BIP_PLAY_BOOL_COMMAND_OPTION(disableVideo, OFF, "Disable Video decode"),
    BIP_PLAY_BOOL_COMMAND_OPTION(disableAudio, OFF, "Disable Audio decode"),
    BIP_PLAY_BOOL_COMMAND_OPTION(startPaused, OFF, "Start playback with a pause, play command needed to resume playback"),
    BIP_PLAY_INTEGER_COMMAND_OPTION(keyFormat, "<commonDrm|test>", B_DTCP_KeyFormat_eCommonDRM, "DTCP key format B_DTCP_KeyFormat_eCommonDRM or B_DTCP_KeyFormat_eTest"),
    BIP_PLAY_BOOL_COMMAND_OPTION(enableTimeShift, OFF, "To indicate playback source is time shifting before streaming out"),
    BIP_PLAY_INTEGER_COMMAND_OPTION(timeShiftDuration, "<msec>", BIP_PLAY_DEFAULT_TIMESHIFT_DURATION, "Maximum time shift duration buffer in ms: needed if enableTimeShift is set"),
    BIP_PLAY_INTEGER_COMMAND_OPTION(displayFormat, "<format>", NEXUS_VideoFormat_e720p, "Display resolution (ntsc|480i|\n" \
           "                                          pal|pal_m|pal_n|pal_nc|576i|1080i|1080i50|720p|720p24|720p25|720p30|\n" \
           "                                          720p50|480p|576p|1080p|1080p24|1080p25|1080p30|1080p50|1080p60|1080p100|\n" \
           "                                          1080p120|720p3D|1080p3D|3840x2160p24|3840x2160p25|3840x2160p30|3840x2160p50|\n" \
           "                                          3840x2160p60|4096x2160p24|4096x2160p25|4096x2160p30|4096x2160p50|4096x2160p60)"),
    BIP_PLAY_STRING_COMMAND_OPTION(language, "<lang>", NULL, "Select audio track language. ISO 639-2code audio codes (eng|por|...)"),
    BIP_PLAY_INTEGER_COMMAND_OPTION(ac3ServiceType, "<bsmod>", UINT_MAX, "Ac3 serviceType. See BIP_MediaInfoAudioAc3Bsmod in bip_media_info.h"),
    BIP_PLAY_BOOL_COMMAND_OPTION(usePlaypump, OFF, "Allows user to force usage of Nexus Playpump"),
    BIP_PLAY_BOOL_COMMAND_OPTION(enablePayloadScanning, OFF, "Enables ES level Media Probe"),
    BIP_PLAY_BOOL_COMMAND_OPTION(enableAutoPlayAfterStartingPaused, OFF, "If starting paused, then immediately start playing on starting bip_play"),
    BIP_PLAY_BOOL_COMMAND_OPTION(disableRemoteKeys, OFF, "Turn off IR control"),
    BIP_PLAY_BOOL_COMMAND_OPTION(disableGui, OFF, "Turn off GUI"),
    BIP_PLAY_INTEGER_COMMAND_OPTION(alpha, "<0-255>", 0xFF, "Graphics transparency 0 - 255"),
    BIP_PLAY_INTEGER_COMMAND_OPTION(videoCdbSize, "<size>", 10, "Video compressed buffer size in MB"),
    BIP_PLAY_INTEGER_COMMAND_OPTION(videoSharpness, "<0-5>", 0, "0 - off, 1..5"),
    BIP_PLAY_BOOL_COMMAND_OPTION(enableMad, OFF, "Enable video MAD (de-interlacer)"),
    BIP_PLAY_BOOL_COMMAND_OPTION(enableSmoothing, OFF, "Set smoothResolutionChange"),
    BIP_PLAY_BOOL_COMMAND_OPTION(enableDeringing, OFF, "Enable video scaler de-ringing"),
    BIP_PLAY_BOOL_COMMAND_OPTION(enableDejagging, OFF, "Enable video scaler de-jagging"),
    BIP_PLAY_INTEGER_COMMAND_OPTION(dolbyDrcMode, "<drc_mode>", NEXUS_AudioDecoderDolbyDrcMode_eLine, "Dolby decoder DRC mode (line|rf|custom_a|custom_d|off)"),
    BIP_PLAY_INTEGER_COMMAND_OPTION(contentMode, "<mode>", NEXUS_VideoWindowContentMode_eFull, "Video window mode (zoom|box|panscan|full|nonlinear)")
};

void BIP_Play_PrintUsage(
    char *argv0
    )
{
    unsigned int i;
    char commandStr[128];
    BIP_Play_CommandInfo *pCmdInfo;

    BDBG_ENTER(BIP_Play_PrintUsage);

    /* Print usage using the commands list */
    printf( "Usage: %s [Options] <URL> \n", argv0);
    for (i = 0; i < sizeof(g_commandsList)/sizeof(BIP_Play_CommandInfo); i++)
    {
        pCmdInfo = &g_commandsList[i];
        snprintf(commandStr, 128, "-%s %s", pCmdInfo->pKeyStr, pCmdInfo->pValueStr == NULL ? "" : pCmdInfo->pValueStr);
        printf ("  %-36s #  %s\n", commandStr, pCmdInfo->pDescription);
    }
    BDBG_LEAVE(BIP_Play_PrintUsage);
}

static void setOptionDefaults(
    BIP_Play_Options *pOptions
    )
{
    unsigned int j;
    char str[128];
    BIP_Play_CommandInfo *pCmdInfo;

    BDBG_ENTER(setOptionDefaults);

    /* Initialize options structure to the default values using the commands list */
    for (j = 0; j < sizeof(g_commandsList)/sizeof(BIP_Play_CommandInfo); j++)
    {
        pCmdInfo = &g_commandsList[j];
        switch (pCmdInfo->type)
        {
            case BIP_Play_OptionType_eInt:
                snprintf(str, 128, "%d", pCmdInfo->intValue);
                pCmdInfo->fHandler(str, pOptions);
                break;
            case BIP_Play_OptionType_eString:
                pCmdInfo->fHandler(pCmdInfo->pStrValue, pOptions);
                break;
            case BIP_Play_OptionType_eBool:
                pCmdInfo->fHandler((pCmdInfo->intValue == ON) ? "true" : "false", pOptions);
            default:
                break;
        }
    }

    BDBG_LEAVE(setOptionDefaults);
}

/* Command line parser */
BIP_Status BIP_Play_ParseOptions(
    int argc,
    char **argv,
    BIP_Play_Options *pOptions,
    BIP_StringHandle *hUrls,
    unsigned int *numUrls)
{
    int i, ret;
    unsigned int j, numTokens = 0;
    char *saveptr, *token, *delim = ",";
    BIP_Status bipStatus = BIP_ERR_INVALID_PARAMETER;
    BIP_Play_CommandInfo *pCmdInfo;

    BDBG_ENTER(BIP_Play_ParseOptions);

    /* First initialize options to default */
    setOptionDefaults(pOptions);

    /* For each argument specified */
    for (i = 1; i < argc; i++)
    {
        bool option_found = false;

        /* Look for a match in the command list */
        for (j = 0; j < sizeof(g_commandsList)/sizeof(BIP_Play_CommandInfo); j++)
        {
            pCmdInfo = &g_commandsList[j];

            /* Found a matching command */
            if (strcmp(&argv[i][1], pCmdInfo->pKeyStr) == 0)
            {
                /* For integer/string options the value cannot begin with a '-' */
                if ((pCmdInfo->type != BIP_Play_OptionType_eBool) && (argv[i+1][0] == '-'))
                {
                    printf("No value specified for option %s\n", argv[i]);
                    break;
                }
                /* Call parser function */
                ret = pCmdInfo->fHandler(argv[i+1], pOptions);
                if (ret >= 0)
                {
                    option_found = true;
                    i += ret;
                }
                break;
            }
        }

        if (option_found == false)
        {
            /* Last argument is the URL list - Check for it */
            if ((i == (argc - 1)) && (argv[i][0] != '-'))
            {
                token = strtok_r(argv[i], delim, &saveptr);
                while (token != NULL)
                {
                    if (*numUrls > numTokens)
                        BIP_String_StrcpyChar(hUrls[numTokens++], token);
                    else
                        break;
                    token = strtok_r(NULL, delim, &saveptr);
                }
                /* Return no. of URLS parsed*/
                *numUrls = numTokens;
            }
            else
            {
                printf ("Invalid option/value : %s\n", argv[i]);
                BIP_Play_PrintUsage(argv[0]);
                break;
            }
        }
    }

    if (numTokens == 0)
    {
        BDBG_ERR((BIP_MSG_PRE_FMT "NO URL specified!!!" BIP_MSG_PRE_ARG));
        BIP_Play_PrintUsage(argv[0]);
    }

    /* Parsed all options without error and found at least one URL */
    if ((i == argc) && (numTokens != 0))
    {
        bipStatus = BIP_SUCCESS;
    }

    *numUrls = numTokens;

    BDBG_LEAVE(BIP_Play_ParseOptions);
    return bipStatus;
}

int BIP_Play_CommandParse_interface(
    char *pValue,
    BIP_Play_Options *pOptions)
{
    pOptions->interface = pValue;
    return 1;
}

int BIP_Play_CommandParse_pos(
    char *pValue,
    BIP_Play_Options *pOptions)
{
    pOptions->initialPosition = strtoul(pValue, NULL, 0);
    return 1;
}

int BIP_Play_CommandParse_jump(
    char *pValue,
    BIP_Play_Options *pOptions)
{
    pOptions->jumpOffset = strtoul(pValue, NULL, 0);
    return 1;
}

int BIP_Play_CommandParse_playTime(
    char *pValue,
    BIP_Play_Options *pOptions)
{
    pOptions->playTime = strtoul(pValue, NULL, 0);
    return 1;
}

int BIP_Play_CommandParse_keyFormat(
    char *pValue,
    BIP_Play_Options *pOptions)
{
    if (strcmp(pValue, "commonDrm") == 0)
        pOptions->keyFormat = B_DTCP_KeyFormat_eCommonDRM;
    else if (strcmp(pValue, "test") == 0)
        pOptions->keyFormat = B_DTCP_KeyFormat_eTest;
    else
        return -1;

    return 1;
}

int BIP_Play_CommandParse_timeShiftDuration(
    char *pValue,
    BIP_Play_Options *pOptions)
{
    pOptions->timeShiftDuration = strtoul(pValue, NULL, 0);
    return 1;
}

int BIP_Play_CommandParse_displayFormat(
    char *pValue,
    BIP_Play_Options *pOptions)
{
    char *endptr;

    /* Check for an integer value first */
    pOptions->displayFormat = strtoul(pValue, &endptr, 0);

    /* If it was a string lookup the value */
    if (*endptr != '\0')
        pOptions->displayFormat = BIP_StrTo_NEXUS_VideoFormat(pValue);
    return 1;
}

int BIP_Play_CommandParse_language(
    char *pValue,
    BIP_Play_Options *pOptions)
{
    if (pValue)
    {
        strncpy(pOptions->language, pValue, AUDIO_LANG_LEN);
        pOptions->language[AUDIO_LANG_LEN-1] = '\0';
    }
    return 1;
}

int BIP_Play_CommandParse_ac3ServiceType(
    char *pValue,
    BIP_Play_Options *pOptions)
{
    pOptions->ac3ServiceType = strtoul(pValue, NULL, 0);
    return 1;
}

int BIP_Play_CommandParse_alpha(
    char *pValue,
    BIP_Play_Options *pOptions)
{
    pOptions->alpha = strtoul(pValue, NULL, 0);
    if ((unsigned)pOptions->alpha > 0xFF)
        return -1;
    return 1;
}

int BIP_Play_CommandParse_videoCdbSize(
    char *pValue,
    BIP_Play_Options *pOptions)
{
    pOptions->videoCdbSize = strtoul(pValue, NULL, 0);
    /* Limit max CDB size to 100 MB */
    if ((unsigned)pOptions->videoCdbSize > 100)
        pOptions->videoCdbSize = 100;
    return 1;
}

int BIP_Play_CommandParse_videoSharpness(
    char *pValue,
    BIP_Play_Options *pOptions)
{
    pOptions->videoSharpness = strtoul(pValue, NULL, 0);
    if ((unsigned)pOptions->videoSharpness > 5)
        return -1;
    return 1;
}

int BIP_Play_CommandParse_dolbyDrcMode(
    char *pValue,
    BIP_Play_Options *pOptions)
{
    char *endptr;

    /* Check for an integer value first */
    pOptions->dolbyDrcMode = strtoul(pValue, &endptr, 0);

    /* If it was a string lookup the value */
    if (*endptr != '\0')
        pOptions->dolbyDrcMode = BIP_StrTo_NEXUS_AudioDecoderDolbyDrcMode(pValue);

    return 1;
}

int BIP_Play_CommandParse_contentMode(
    char *pValue,
    BIP_Play_Options *pOptions)
{
    char *endptr;

    /* Check for an integer value first */
    pOptions->contentMode = strtoul(pValue, &endptr, 0);

    /* If it was a string lookup the value */
    if (*endptr != '\0')
        pOptions->contentMode = BIP_StrTo_NEXUS_VideoWindowContentMode(pValue);

    return 1;
}

/* Parsing definition for all boolean parsing funtions */
BIP_PLAY_BOOLEAN_PARSE_DEFINITION(loop)
BIP_PLAY_BOOLEAN_PARSE_DEFINITION(printStatus)
BIP_PLAY_BOOLEAN_PARSE_DEFINITION(printServerStats)
BIP_PLAY_BOOLEAN_PARSE_DEFINITION(disableVideo)
BIP_PLAY_BOOLEAN_PARSE_DEFINITION(disableAudio)
BIP_PLAY_BOOLEAN_PARSE_DEFINITION(startPaused)
BIP_PLAY_BOOLEAN_PARSE_DEFINITION(enableTimeShift)
BIP_PLAY_BOOLEAN_PARSE_DEFINITION(usePlaypump)
BIP_PLAY_BOOLEAN_PARSE_DEFINITION(enablePayloadScanning)
BIP_PLAY_BOOLEAN_PARSE_DEFINITION(enableAutoPlayAfterStartingPaused)
BIP_PLAY_BOOLEAN_PARSE_DEFINITION(disableRemoteKeys)
BIP_PLAY_BOOLEAN_PARSE_DEFINITION(disableGui)
BIP_PLAY_BOOLEAN_PARSE_DEFINITION(enableMad)
BIP_PLAY_BOOLEAN_PARSE_DEFINITION(enableSmoothing)
BIP_PLAY_BOOLEAN_PARSE_DEFINITION(enableDeringing)
BIP_PLAY_BOOLEAN_PARSE_DEFINITION(enableDejagging)

/***************************************/
/******* RUN TIME COMMAND PARSING ******/
/***************************************/

/* Runtime Commands list : List of Runtime command line options supported
 * To add a new command to the list of runtime options, add an enum to
 * BIP_Play_RuntimeCmd and then add an entry to the list below.
 * Each entry specifies
 * 1. Command string
 * 2. Optional argument string
 * 3. Command Type (enum itself)
 * 4. Option Type (int, bool, string).
 * 5. Brief Description.
 */
BIP_Play_RuntimeCommandInfo g_runtimeCommandsList[] = {
/*  {Cmd Tag, Arg,   Cmd Type,                               Option Type,                 Description} */
    {"play",  NULL,  BIP_Play_RuntimeCmd_ePlay,              BIP_Play_OptionType_eBool,   "Resume normal playback"},
    {"pause", NULL,  BIP_Play_RuntimeCmd_ePause,             BIP_Play_OptionType_eBool,   "Pause playback"},
    {"p",     NULL,  BIP_Play_RuntimeCmd_eTogglePlay,        BIP_Play_OptionType_eBool,   "Toggle pause/play"},
    {"fa",    NULL,  BIP_Play_RuntimeCmd_eFrameAdvance,      BIP_Play_OptionType_eBool,   "Frame advance"},
    {"fr",    NULL,  BIP_Play_RuntimeCmd_eFrameReverse,      BIP_Play_OptionType_eBool,   "Frame reverse"},
    {"rate",  "rate",BIP_Play_RuntimeCmd_ePlayAtRate,        BIP_Play_OptionType_eString, "(rate) - set trick rate (e.g. 6 for 6x, 1/4 for 1/4x, -6 for -6x, etc."},
    {"seek",  "pos", BIP_Play_RuntimeCmd_eSeek,              BIP_Play_OptionType_eTime,   "seek to absolute position (in ms. e.g. 2:10.30 for 2min, 10sec, & 30ms, 2:10 for 2min & 10sec, 120 for 120ms)"},
    {"jf",    NULL,  BIP_Play_RuntimeCmd_eJumpFwd,           BIP_Play_OptionType_eBool,   "jump forward by jump offset (default is 30000ms, changed via command line option -jump)"},
    {"jb",    NULL,  BIP_Play_RuntimeCmd_eJumpBack,          BIP_Play_OptionType_eBool,   "jump backward by jump offset (default is 30000ms, changed via command line option -jump)"},
    {"st",    NULL,  BIP_Play_RuntimeCmd_ePrintStatus,       BIP_Play_OptionType_eBool,   "print status"},
    {"sst",   NULL,  BIP_Play_RuntimeCmd_ePrintServerStatus, BIP_Play_OptionType_eBool,   "print server status"},
    {"chng",  "url", BIP_Play_RuntimeCmd_ePlayNewUrl,        BIP_Play_OptionType_eString, "play for the new url."},
    {"tune",  "no.", BIP_Play_RuntimeCmd_eTune,              BIP_Play_OptionType_eInt,    "play for the nth URL specified at command line <1..N>."},
    {"gui",   NULL,  BIP_Play_RuntimeCmd_eGui,               BIP_Play_OptionType_eBool,   "Enable GUI"},
    {"nogui", NULL,  BIP_Play_RuntimeCmd_eNoGui,             BIP_Play_OptionType_eBool,   "Disable GUI"},
    {"rmt",   NULL,  BIP_Play_RuntimeCmd_eRemoteKeys,        BIP_Play_OptionType_eBool,   "Enable Remote Keys"},
    {"normt", NULL,  BIP_Play_RuntimeCmd_eNoRemoteKeys,      BIP_Play_OptionType_eBool,   "Disable Remote Keys"},
    {"lng",   "cod", BIP_Play_RuntimeCmd_ePlayLanguage,      BIP_Play_OptionType_eString, "3-byte language code for audio - ISO 639-2code (eng|por|...)"},
    {"bsmod", "val", BIP_Play_RuntimeCmd_ePlayBsmod,         BIP_Play_OptionType_eInt,    "Ac3 service type - see BIP_MediaInfoAudioAc3Bsmod in bip_media_info.h, Eg: bsmod(2) for VisuallyImpaired"},
    {"last",  NULL,  BIP_Play_RuntimeCmd_eLast,              BIP_Play_OptionType_eBool,   "Repeat last command"},
    {"q",     NULL,  BIP_Play_RuntimeCmd_eQuit,              BIP_Play_OptionType_eBool,   "Quit application"}
};

void BIP_Play_PrintRuntimeCmdUsage(void)
{
    unsigned i;
    char key[128];
    BIP_Play_RuntimeCommandInfo *pCmdInfo;

    BDBG_ENTER(BIP_Play_PrintRuntimeCmdUsage);

    /* Cycle through run time command list and print usage */
    printf ("\nRun time command usage\n");
    for (i = 0; i < sizeof(g_runtimeCommandsList)/sizeof(BIP_Play_RuntimeCommandInfo); i++)
    {
        pCmdInfo = &g_runtimeCommandsList[i];
        if (pCmdInfo->pValStr == NULL)
            snprintf(key, 128, "%s", pCmdInfo->pKeyStr);
        else
            snprintf(key, 128, "%s(%s)", pCmdInfo->pKeyStr, pCmdInfo->pValStr);
        printf ("    %-12s #  %s\n", key, pCmdInfo->pDescription);
    }
    BDBG_LEAVE(BIP_Play_PrintRuntimeCmdUsage);
}

/* Buffer to hold argument for the last run time command */
char runtimeCommandArg[MAX_RUNTIME_COMMAND_SIZE];

/* Parses hh:mm:ss.nnm time format and returns time in milli seconds */
static int parseTime(char *str)
{
    float time = 0.0;
    float secs;
    char *saveptr, *token, *delim = ":";

    BDBG_ENTER(parseTime);

    token = strtok_r(str, delim, &saveptr);

    while (token != NULL)
    {
        time *= 60;
        if (strchr(token, '.'))
        {
            sscanf(token, "%f", &secs);
            time += secs;
            break;
        }
        else
            time += strtoul(token, NULL, 0);
    }

    BDBG_LEAVE(parseTime);
    return (int)(time*1000);
}

/* Returns true if the string is empty or contains only white spaces */
static bool isAllSpaces(char *pStr)
{
    unsigned i;
    bool ret = true;

    for (i = 0; i < strlen(pStr); i++)
        ret = ret && isspace(pStr[i]);

    return ret;
}

BIP_Status BIP_Play_ParseRunTimeCmd(
    char *cmd,
    BIP_Play_ParsedRuntimeCmdInfo *pParsedCmd
    )
{
    unsigned i;
    char *saveptr, *token, *delim = ")";
    BIP_Status bipStatus = BIP_ERR_INVALID_PARAMETER;
    BIP_Play_RuntimeCommandInfo *pCmdInfo;
    char command[MAX_RUNTIME_COMMAND_SIZE];

    BDBG_ENTER(BIP_Play_ParseRunTimeCmd);

    /* Make a local copy of the command */
    strncpy(command, cmd, MAX_RUNTIME_COMMAND_SIZE);

    /* Initialize return structure */
    pParsedCmd->command = BIP_Play_RuntimeCmd_eMax;
    pParsedCmd->intArg = -1;
    pParsedCmd->strArg[0] = '\0';

    /* If the command is emptry just return */
    if (isAllSpaces(command) || (strlen(command) == 0))
    {
        pParsedCmd->command = BIP_Play_RuntimeCmd_eNone;
    }
    else
    {
        /* For each run time command in the command list*/
        for (i = 0; i < sizeof(g_runtimeCommandsList)/sizeof(BIP_Play_RuntimeCommandInfo); i++)
        {
            pCmdInfo = &g_runtimeCommandsList[i];

            /* Check for a match */
            if (strncmp(pCmdInfo->pKeyStr, command, strlen(pCmdInfo->pKeyStr)) == 0)
            {
                /* Save the argument */
                if (pCmdInfo->type != BIP_Play_OptionType_eBool)
                {
                    token = strtok_r(&command[strlen(pCmdInfo->pKeyStr)+1], delim, &saveptr);
                    runtimeCommandArg[0] = '\0';
                    if (token == NULL)
                    {
                        printf("Expected an argument for option : %s\ncommand - %s\n", pCmdInfo->pKeyStr, command);
                        break;
                    }
                    strncpy(runtimeCommandArg, token, sizeof(runtimeCommandArg));
                }
                /* Set the output structure, command type, integer/string/time argument value */
                pParsedCmd->command = pCmdInfo->command;
                switch (pCmdInfo->type)
                {
                    case BIP_Play_OptionType_eInt:
                        pParsedCmd->intArg = strtoul(runtimeCommandArg, NULL, 0);
                        break;
                    case BIP_Play_OptionType_eString:
                        strncpy(pParsedCmd->strArg, runtimeCommandArg, MAX_RUNTIME_COMMAND_SIZE);
                        break;
                    case BIP_Play_OptionType_eTime:
                        pParsedCmd->intArg = parseTime(runtimeCommandArg);
                        break;
                    case BIP_Play_OptionType_eBool:
                    default:
                        break;
                }
            }
        }
    }

    if(pParsedCmd->command != BIP_Play_RuntimeCmd_eMax)
        bipStatus = BIP_SUCCESS;

    BDBG_LEAVE(BIP_Play_ParseRunTimeCmd);
    return bipStatus;
}

/* Get the command string for a give run time command (enum) */
const char* BIP_Play_LookupRuntimeCommand(
        BIP_Play_RuntimeCommand command
    )
{
    unsigned j;
    const char *pKey = "command";
    BIP_Play_RuntimeCommandInfo *pCmdInfo;

    for (j = 0; j < sizeof(g_runtimeCommandsList)/sizeof(BIP_Play_RuntimeCommandInfo); j++)
    {
        pCmdInfo = &g_runtimeCommandsList[j];

        if (pCmdInfo->command == command)
        {
            pKey = pCmdInfo->pKeyStr;
            break;
        }
    }

    return pKey;
}
