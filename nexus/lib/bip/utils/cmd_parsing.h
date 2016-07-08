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

#ifndef _BIP_CMD_PARSING_H
#define _BIP_CMD_PARSING_H

#include <stdbool.h>
#include "bip.h"

/* COMMAND LINE PARSING:
 * The command line parsing logic uses a global table g_CommandsList[] to simplify the addition
 * of new command line arguments. This table is used to print comand usage and parse command line
 * options. This table contains one entry corresponding to every member of the structure
 * 'BIP_Play_Options'. To add a new command line option, add an element (int, bool, string) to structure
 * 'BIP_Play_Options' and then add a corresponding entry to g_CommandsList[]. Each entry specifies ...
 * 1. Command string
 * 2. Optional argument string
 * 3. Default value
 * 4. Brief Description
 * 5. element type (int, bool, string)
 * 6. Parsing function - BIP_Play_CommandParse_<element_name>(char * argument, BIP_Play_Options *options)
 * The parsing function needs to be defined in cmd_parsing.c and declared in cmd_parsing.h.
 * This function is expected to parse the argument specified and set the element in the 'BIP_Play_Options'
 * structure.
 * The following macros can be used to add a new element to the g_CommandsList[]
 * - BIP_PLAY_STRING_COMMAND_OPTION(elementName, argumentString, defaultValue, description)
 * - BIP_PLAY_INTEGER_COMMAND_OPTION(elementName, argumentString, defaultValue, description)
 * - BIP_PLAY_BOOL_COMMAND_OPTION(elementName, defaultValue, description)
 * The idea of using a list is to simpilfy the printUsage() and parseCommand() functions by
 * avoiding else if's everytime a new command line argument needs to be added.
 *
 * RUNTIME COMMAND PARSING:
 * The runtime command parsing logic also uses a global table g_runtimeCommandsList to simplify the addition
 * of new runtime commands. This table contains one entry corresponding every enumerated value in
 * 'BIP_Play_RuntimeCmd'. To add a new runtime command to the list of runtime options, add an enum value to
 * 'BIP_Play_RuntimeCmd' and then add an entry to the g_runtimeCommandsList[]. Each entry specifies
 * 1. Command string
 * 2. Optional argument string
 * 3. Command Type (enum itself)
 * 4. Option Type (int, bool, string).
 * 5. Brief Description.
 * The idea of using a list is to simpilfy the printUsage() and parseCommand() functions by
 * avoiding else if's everytime a new command line argument needs to be added.
 */


#define AUDIO_LANG_LEN  4 /* 3-byte ISO 639-2code for audio + null character */
#define OFF 0
#define ON  1

#define BIP_PLAY_DEFAULT_JUMP_OFFSET            30000   /* in ms */
#define BIP_PLAY_DEFAULT_PLAYBACK_DURATION      -1      /* till end of stream */
#define BIP_PLAY_DEFAULT_TIMESHIFT_DURATION     300000   /* 5 minutes */

#define MAX_RUNTIME_COMMAND_SIZE    256

/* Command entry initialization macros
 * These macros are used to add a new entry to the commands list 'g_commandsList' */
#define BIP_PLAY_STRING_COMMAND_OPTION(KEY, VAL, DEF, DESC) \
    {#KEY, VAL, BIP_Play_OptionType_eString, 0, DEF, DESC, BIP_Play_CommandParse_##KEY}
#define BIP_PLAY_INTEGER_COMMAND_OPTION(KEY, VAL, DEF, DESC) \
    {#KEY, VAL, BIP_Play_OptionType_eInt, DEF, NULL, DESC, BIP_Play_CommandParse_##KEY}
#define BIP_PLAY_BOOL_COMMAND_OPTION(KEY, DEF, DESC) \
    {#KEY, NULL, BIP_Play_OptionType_eBool, DEF, NULL, DESC, BIP_Play_CommandParse_##KEY}

/* Boolean command parsing function - This is a standard boolean parsing function
 * definition - Use this macro to define the function in the cmd_parsing.c */
#define BIP_PLAY_BOOLEAN_PARSE_DEFINITION(KEY) \
    int BIP_Play_CommandParse_##KEY( \
        char *pValue,               \
        BIP_Play_Options *pOptions) \
    {                               \
        if (pValue == NULL)         \
            pOptions->KEY = true;   \
        else if (strcmp(pValue, "false") == 0) \
            pOptions->KEY = false;  \
        else                        \
            pOptions->KEY = true;   \
        return 0;                   \
    }

/* For each of the elements defined below in the structure, a new entry needs to be
 * added to g_commandsList[] and a function BIP_Play_CommandParse_<elem_name> must be
 * defined in cmd_parsing.c and declared in cmd_parsing.h (BIP_PLAY_COMMAND_PARSE_DECLARE) */
typedef struct BIP_Play_Options {
    char* interface;        /* Interface name - optional (eth0, lo ...)*/
    int initialPosition;    /* Initial playback position (in ms) */
    int jumpOffset;         /* Run time jump quantum duration (in ms) */
    bool loop;              /* Loop around stream at the end of playback */
    int playTime;           /* Playback duration (ms) - default is -1 : up to end of stream */
    bool printStatus;       /* Print periodic status */
    bool printServerStats;  /* Print periodic server status */
    bool disableVideo;      /* Disable Video decode */
    bool disableAudio;      /* Disable Audio decode */
    bool startPaused;       /* Start playback with a pause, play command needed to resume playback */
    B_DTCP_KeyFormat_T keyFormat;   /* DTCP key format B_DTCP_KeyFormat_eCommonDRM or B_DTCP_KeyFormat_eTest */
    bool enableTimeShift;   /* Set to indicate playback is from a source that is time shifting before streaming out */
    int timeShiftDuration;  /* Maximum time shift duration buffer in ms (needed if enableTimeShift is set */
    NEXUS_VideoFormat displayFormat;/* Display resolution */
    char language[AUDIO_LANG_LEN];  /* Audio track language */
    BIP_MediaInfoAudioAc3Bsmod ac3ServiceType;  /* Default is UINT_MAX which means undefined */
    bool usePlaypump;               /* Use playpump for nexus playback */
    bool enablePayloadScanning;     /* Enable payload scanning */
    bool enableAutoPlayAfterStartingPaused; /* Enable AutoPlay -> only relevant when startPaused is true */
    bool disableRemoteKeys; /* Turn off IR control */
    bool disableGui;        /* Turn off GUI */
    int alpha;              /* Graphics transparency 0 - 255 */
    int videoCdbSize;       /* Video compressed buffer size in mega bytes */
    int videoSharpness;     /* 0 - off, 1..5*/
    bool enableMad;         /* Enable video MAD (de-interlacer) */
    bool enableSmoothing;   /* Set smoothResolutionChange */
    bool enableDeringing;   /* Enable video scaler de-ringing */
    bool enableDejagging;   /* Enable video scaler de-jagging */
    NEXUS_AudioDecoderDolbyDrcMode dolbyDrcMode; /* Dolby decoder DRC mode */
    NEXUS_VideoWindowContentMode contentMode;    /* Video window mode */
} BIP_Play_Options;

/* Function pointer to option parser */
typedef int (*BIP_Play_OptionHandler)(char *pValue, BIP_Play_Options *pOptions);

/* Option Type */
typedef enum BIP_Play_OptionType {
    BIP_Play_OptionType_eInt,
    BIP_Play_OptionType_eString,
    BIP_Play_OptionType_eBool,
    BIP_Play_OptionType_eTime,
    BIP_Play_OptionType_eMax
} BIP_Play_OptionType;

/* Command argument details */
typedef struct BIP_Play_CommandInfo {
    const char *pKeyStr;        /* Option Key string */
    const char *pValueStr;       /* NULL - indicates no value needed */
    BIP_Play_OptionType type;   /* Argument type - String, Int, Bool ...*/
    int        intValue;        /* Integer Value of the option - Set to 0/1 for boolean */
    char       *pStrValue;      /* Supplied value for String type option */
    const char *pDescription;    /* Argument usage description string */
    BIP_Play_OptionHandler fHandler; /* Pointer to function that handles the option */
} BIP_Play_CommandInfo;

/* Command parsing function declarations - Parsing function declaration */
#define BIP_PLAY_COMMAND_PARSE_DECLARE(KEY)   \
    int BIP_Play_CommandParse_##KEY( \
        char *pValue,               \
        BIP_Play_Options *pOptions)

BIP_PLAY_COMMAND_PARSE_DECLARE(interface);
BIP_PLAY_COMMAND_PARSE_DECLARE(pos);
BIP_PLAY_COMMAND_PARSE_DECLARE(jump);
BIP_PLAY_COMMAND_PARSE_DECLARE(playTime);
BIP_PLAY_COMMAND_PARSE_DECLARE(keyFormat);
BIP_PLAY_COMMAND_PARSE_DECLARE(timeShiftDuration);
BIP_PLAY_COMMAND_PARSE_DECLARE(displayFormat);
BIP_PLAY_COMMAND_PARSE_DECLARE(language);
BIP_PLAY_COMMAND_PARSE_DECLARE(ac3ServiceType);
BIP_PLAY_COMMAND_PARSE_DECLARE(usePlaypump);
BIP_PLAY_COMMAND_PARSE_DECLARE(enablePayloadScanning);
BIP_PLAY_COMMAND_PARSE_DECLARE(enableAutoPlayAfterStartingPaused);
BIP_PLAY_COMMAND_PARSE_DECLARE(alpha);
BIP_PLAY_COMMAND_PARSE_DECLARE(videoCdbSize);
BIP_PLAY_COMMAND_PARSE_DECLARE(videoSharpness);
BIP_PLAY_COMMAND_PARSE_DECLARE(dolbyDrcMode);
BIP_PLAY_COMMAND_PARSE_DECLARE(contentMode);
BIP_PLAY_COMMAND_PARSE_DECLARE(loop);
BIP_PLAY_COMMAND_PARSE_DECLARE(printStatus);
BIP_PLAY_COMMAND_PARSE_DECLARE(printServerStats);
BIP_PLAY_COMMAND_PARSE_DECLARE(disableVideo);
BIP_PLAY_COMMAND_PARSE_DECLARE(disableAudio);
BIP_PLAY_COMMAND_PARSE_DECLARE(startPaused);
BIP_PLAY_COMMAND_PARSE_DECLARE(enableTimeShift);
BIP_PLAY_COMMAND_PARSE_DECLARE(disableRemoteKeys);
BIP_PLAY_COMMAND_PARSE_DECLARE(disableGui);
BIP_PLAY_COMMAND_PARSE_DECLARE(enableMad);
BIP_PLAY_COMMAND_PARSE_DECLARE(enableSmoothing);
BIP_PLAY_COMMAND_PARSE_DECLARE(enableDeringing);
BIP_PLAY_COMMAND_PARSE_DECLARE(enableDejagging);

/* Print command line usage */
void BIP_Play_PrintUsage(
    char *argv0); /* Input */

/* Parse command line arguments and populate options structure */
BIP_Status BIP_Play_ParseOptions(
    int argc,                   /* In */
    char **argv,                /* In */
    BIP_Play_Options *pOptions, /* Out */
    BIP_StringHandle *hUrls,    /* Out */
    unsigned int *numUrls);     /* InOut */

/***************************************/
/******* RUN TIME COMMAND PARSING ******/
/***************************************/

/* Run time command types - For each enumerated type added to this list
 * with the exception of 'eLast' add a new entry to g_runtimeCommandsList[]
 * in the cmd_parsing.c - The actual handling of the command will be done
 * outside of the parser (ex: bip_play.c) */
typedef enum BIP_Play_RuntimeCommand
{
    BIP_Play_RuntimeCmd_ePlay,              /* Play Stream */
    BIP_Play_RuntimeCmd_ePause,             /* Pause playback */
    BIP_Play_RuntimeCmd_eTogglePlay,        /* Play/Pause toggle */
    BIP_Play_RuntimeCmd_eSeek,              /* Seek to a given position */
    BIP_Play_RuntimeCmd_eJumpFwd,           /* Jump forward by jumpOffset (ms) */
    BIP_Play_RuntimeCmd_eJumpBack,          /* Jump backward by jumpOffset (ms) */
    BIP_Play_RuntimeCmd_ePlayAtRate,        /* Change playback rate (ffwd, rewind) */
    BIP_Play_RuntimeCmd_eFrameAdvance,      /* Play frame by frame forward */
    BIP_Play_RuntimeCmd_eFrameReverse,      /* Play frame by frame reverse */
    BIP_Play_RuntimeCmd_ePrintStatus,       /* Print player status */
    BIP_Play_RuntimeCmd_ePrintServerStatus, /* Print servers status */
    BIP_Play_RuntimeCmd_eTune,              /* Tune to the index specified */
    BIP_Play_RuntimeCmd_eGui,               /* Enable GUI */
    BIP_Play_RuntimeCmd_eNoGui,             /* Turn off GUI */
    BIP_Play_RuntimeCmd_eRemoteKeys,        /* Enable Remote keys */
    BIP_Play_RuntimeCmd_eNoRemoteKeys,      /* Turn off remote keys */
    BIP_Play_RuntimeCmd_ePlayNewUrl,        /* Stream a new URL */
    BIP_Play_RuntimeCmd_ePlayLanguage,      /* Change audio language */
    BIP_Play_RuntimeCmd_ePlayBsmod,         /* Change audio stream BSMOD (dolby only) */
    BIP_Play_RuntimeCmd_eLast,              /* Repeat last command */
    BIP_Play_RuntimeCmd_eQuit,              /* Quit the application */
    BIP_Play_RuntimeCmd_eNone,
    BIP_Play_RuntimeCmd_eMax
} BIP_Play_RuntimeCommand;

/* Runtime command details */
typedef struct BIP_Play_RuntimeCommandInfo {
    const char *pKeyStr;            /*  Option Key string */
    const char *pValStr;            /* Option Value string */
    BIP_Play_RuntimeCommand command;/* Command type */
    BIP_Play_OptionType type;       /* Argument type - String, Int, Bool, Time ...*/
    const char *pDescription;       /* Argument usage description string */
} BIP_Play_RuntimeCommandInfo;

/* Parsed Runtime command details */
typedef struct BIP_Play_ParsedRuntimeCmdInfo {
    BIP_Play_RuntimeCommand command;
    int intArg;
    char strArg[MAX_RUNTIME_COMMAND_SIZE];
} BIP_Play_ParsedRuntimeCmdInfo;

/* Print runtime command usage */
void BIP_Play_PrintRuntimeCmdUsage(void);

/* Parse runtime command */
BIP_Status BIP_Play_ParseRunTimeCmd(
    char *cmd,                                /* In */
    BIP_Play_ParsedRuntimeCmdInfo *pParsedCmd /* Out */
    );
/* Return command string for a given runtime command */
const char* BIP_Play_LookupRuntimeCommand(
        BIP_Play_RuntimeCommand command       /* In */
    );

#endif /* _BIP_CMD_PARSING_H */
