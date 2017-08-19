#############################################################################
# Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
#
# This program is the proprietary software of Broadcom and/or its licensors,
# and may only be used, duplicated, modified or distributed pursuant to the terms and
# conditions of a separate, written license agreement executed between you and Broadcom
# (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
# no license (express or implied), right to use, or waiver of any kind with respect to the
# Software, and Broadcom expressly reserves all rights in and to the Software and all
# intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
# HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
# NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
# Except as expressly set forth in the Authorized License,
#
# 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
# secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
# and to use this information only in connection with your use of Broadcom integrated circuit products.
#
# 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
# AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
# WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
# THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
# OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
# LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
# OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
# USE OR PERFORMANCE OF THE SOFTWARE.
#
# 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
# LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
# EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
# USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
# THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
# ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
# LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
# ANY LIMITED REMEDY.
#############################################################################

#! /usr/local/bin/python

import argparse
import logging

validFrameRateTypes = [
    "cod",  # coded in the stream
    "def",  # specified by BXDM_PictureProvider_SetDefaultFrameRate_isr
    "ovr",  # specified by BXDM_PictureProvider_SetFrameRateOverride_isr
    "frd",  # calculated in the FRD code using the PTS values
    "hcd"  # values hardcoded in BXDM_PPTSM_P_PtsCalculateParameters_isr
]

# A standard way to associated parameters with a class, saves having to create
# accessor/mutator methods for each paramater. Supports both the non-keyword
# and keyword variable argument lists to make the class more flexible.
# TODO: explore using attributes
class Parameters(object):
    def __init__(self, *args, **kwargs):

        self.parameters = dict()

        # Add the non-keyword parameters to the local dictionary.
        for param in args :
            self.parameters[param] = ""

        # Add the keyword parameters to the local dictionary.
        # Should the variables be checked in any way?
        for param in kwargs :
            self.parameters[param] = kwargs[param]

        return

    def set_parameter(self, param, value):
        # For debug
        # if param not in self.parameters :
        #     print "set_parameter: adding new parameter " + param

        self.parameters[param] = value

        return

    def get_parameter(self, param):

        if param in self.parameters :
            retValue = self.parameters[param]
        else :
            print "get_parameter: " + param + " has not been added"
            retValue = ""

        return retValue

    def dump_parameters(self):
        for param in self.parameters :
                print param + " " + str(self.parameters[param])
        return


#
# Message types
#
DBG_MSG = "debug message"
DBG_MSG_TYPE = "debug message type"
BXVD_DQT = "BXVD_DQT"
BXDM_PPQM = "BXDM_PPQM"
BXDM_PPDBG = "BXDM_PPDBG"  # but not the PPDBG messages associated with the "timeline" that is printed every 12 vsyncs
BXDM_PPDBG_TL = "BXDM_PPDBG_TL"  # the PPDBG messages associated with the "timeline"

PTS = "pts"
PTS_STATE = "pts_state"


#
# Base class for debug message
#
class DebugMessage(Parameters):
    def __init__(self, msg, msgType, **kwargs):
        Parameters.__init__(self, **kwargs)
        self.set_parameter(DBG_MSG, msg)
        self.set_parameter(DBG_MSG_TYPE, msgType)
        self.extract_pts()
        return

    # Extract the channel Id from the beginning of the string.
    # The string will look like this "[00.002]".
    # The digits to the left of the '.' are the channel ID.
    # Return a string for the channel ID.

    def get_channel_id(self):

        dbgMsg = self.get_parameter(DBG_MSG)

        indexOpen = dbgMsg.find(":[")
        indexPeriod = dbgMsg.find(".", indexOpen)

        if indexOpen != -1 and indexPeriod != -1 :
            channelId = dbgMsg[ indexOpen + 2 : indexPeriod ]
        else:
            channelId = ""

        return channelId

    # Extract the picture Id from the beginning of the string.
    # The string will look like this "[00.002]".
    # The digits to the right of the '.' are the channel ID.
    # Return a numeric value for the channel ID.

    def get_picture_id(self):

        dbgMsg = self.get_parameter(DBG_MSG)

        indexOpen = dbgMsg.find(":[")
        indexPeriod = dbgMsg.find(".", indexOpen)
        indexClose = dbgMsg.find("]", indexPeriod)

        pictureId = -1

        if indexOpen != -1 and indexPeriod != -1 and indexClose != -1 :
            try :
                pictureId = int(dbgMsg[ indexPeriod + 1 : indexClose ], 16)
            except :
                # Some messages have a picture ID of 'xxx', this is not an error.
                if dbgMsg[ indexPeriod + 1 : indexClose ] != "xxx" :
                    logging.info("failed to convert picture Id string to an int")

        return pictureId


    def extract_pts(self):

        # search the string for a PTS value
        # look for the string ,pts:xxxxxxxx(1)

        self.parameters[PTS] = 0
        self.parameters[PTS_STATE] = False

        # First check if the message should contain a PTS
        if BXDM_PPQM in self.parameters[DBG_MSG] :

            idxPts = self.parameters[DBG_MSG].find(",pts:")

            if idxPts != -1 :
                idxPts += 5  # bump the index to the beginning of the PTS data

                strPts = self.parameters[DBG_MSG][ idxPts : idxPts + 8 ]

                # check the valid bit
                idxPts += 8

                if  self.parameters[DBG_MSG][idxPts] == "(" and self.parameters[DBG_MSG][idxPts + 1] == "1" :
                    self.parameters[PTS_STATE] = True

                try :
                    self.parameters[PTS] = int(strPts, 16)
                except :
                    self.parameters[PTS] = 0
                    self.parameters[PTS_STATE] = False
        return

class BxvdDqtMessage(DebugMessage):
    def __init__(self, msg, **kwargs):
        DebugMessage.__init__(self, msg, BXVD_DQT, **kwargs)
        return

class BxdmPpqmMessage(DebugMessage):
    def __init__(self, msg, **kwargs):
        DebugMessage.__init__(self, msg, BXDM_PPQM, **kwargs)
        return

class BxdmPpDbgMessage(DebugMessage):
    def __init__(self, msg, **kwargs):
        DebugMessage.__init__(self, msg, BXDM_PPDBG, **kwargs)
        return

#
# For saving the per channel state.
#
class ChannelContext(Parameters):
    def __init__(self, *args, **kwargs):
        Parameters.__init__(self, *args, **kwargs)

        # A list of decodes on this channel.  Everytime start deocde is
        # detected, a new decode will be added. The channel change test
        # is an example of when this will be used.

        self.decode = []
        self.currentDecode = -1

        return

    def add_decode(self):
        self.currentDecode += 1
        self.decode.append(DecodeContext())

        return self.decode[self.currentDecode]

    def get_num_decodes(self):
        return len(self.decode)

    def get_current_decode(self):
        if self.currentDecode == -1 :
            currentDecode = None
            logging.info("no decode objects have been allocated")
        else :
            currentDecode = self.decode[self.currentDecode]

        return currentDecode

    def get_decode(self, index):

        if index > self.currentDecode :
            currentDecode = None
            logging.info("inedex is out of range")
        else :
            currentDecode = self.decode[index]

        return currentDecode


#
# For saving the per decode state.  There can be multiple decodes on a channel.
#
class DecodeContext(Parameters):
    def __init__(self, *args, **kwargs):
        Parameters.__init__(self, *args, **kwargs)
        return

#
# Pull the time string out of the BXDM_PPQM message.
#
def get_timestamp(timeString):

    result = ""

    # an example of the beginning of the QM string with the time
    # --- 00:01:06.349 BXDM_PPQM:

    indexPrefix = timeString.find("--- ")
    indexSuffix = timeString.find(" BXDM_PPQM")

    if indexPrefix != -1 and indexSuffix != -1 :
        result = timeString[ indexPrefix + 4 : indexSuffix ]

    return result

#
# Convert time string to milliseconds
#
def convert_to_milliseconds(timeString):

    timeInMilliSecs = 0

    # convert the time string, an example 00:01:06.349

    indexColon1 = timeString.find(":")
    indexColon2 = timeString.find(":", indexColon1 + 1)
    indexPeriod = timeString.find(".", indexColon2)

    if indexColon1 != -1 and indexColon2 != -1 and indexPeriod != -1 :
        hours = timeString[ : indexColon1 ]
        mins = timeString[ indexColon1 + 1 : indexColon2 ]
        secs = timeString[ indexColon2 + 1 : indexPeriod ]
        msecs = timeString[ indexPeriod + 1 : ]

        printTime = False

        try :
            timeInMilliSecs = int(msecs)
        except :
            logging.info("failed to convert milliseconds")
            printTime = True

        try :
            iSecs = int(secs)
            timeInMilliSecs += iSecs * 1000
        except :
            logging.info("failed to convert seconds")
            printTime = True

        try :
            iMins = int(mins)
            timeInMilliSecs += iMins * 60 * 1000
        except :
            logging.info("failed to convert minutes")
            printTime = True

        try :
            iHours = int(hours)
            timeInMilliSecs += iHours * 60 * 60 * 1000
        except :
            logging.info("failed to convert hours")
            printTime = True

        if printTime == True :
            print "time: " + timeString
            print "hr: " + hours ,
            print "mins: " + mins ,
            print "secs: " + secs ,
            print "msecs: " + msecs

        # end of if indexColon1 != -1 and indexColon2 != -1 and indexPeriod != -1 :

    return timeInMilliSecs

#
# Given two time strings, calculate the elapse time in milliseconds
#
def calculate_elapse_time(startTime, endTime):

    elapseTime = 0

    iStartTime = convert_to_milliseconds(startTime)

    iEndTime = convert_to_milliseconds(endTime)

    elapseTime = iEndTime - iStartTime

    print "elapse time (ms): %d = %d - %d" % (elapseTime, iEndTime, iStartTime)

    return elapseTime