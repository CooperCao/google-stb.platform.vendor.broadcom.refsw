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
import xdm_utils

#
# main processing loop
#
def main(argv) :
    """ main function """

    logging.basicConfig(format="--- %(funcName)s: %(message)s", level=logging.INFO)

    channelData = dict()

    # create an argument parser
    parser = argparse.ArgumentParser(description='log input commands')

    # Note: the first long option name will become an object of "inputArgs"
    # can also be specified with 'dest='
    parser.add_argument('-i', '--input', '--inp', action='store', help='Input file name.')
    parser.add_argument('--channel_change', action='store_true', default=False, help='The results are from the channel change test.')

    inputArgs = parser.parse_args()
    inputFileName = inputArgs.input
    channelChangeTest = inputArgs.channel_change

    channelClosedCount = 0

    srcFile = open(inputFileName, "r")

    # Indicates with which channel the PPDBG messages are associated.
    currentDbgChannel = -1

    runningFromUsbStick = False

    # interate over the lines in "srcFile"
    underflowCount = 0
    for inputLine in srcFile :

        # occasionally there is a long line which messes up the output
        #  if one is detected, just ignore it.
        if len(inputLine) > 512 :
            #===================================================================
            # print "line length of %d is too long" % len(inputLine)
            # print inputLine
            #===================================================================
            continue

        # Is the system using an a stick?
        if inputLine.find("disk removable") != -1 :
            runningFromUsbStick = inputLine.find("1") != -1

        # If the channel change test, check for channel closed messages
        elif "BXVD: Closechannel" in inputLine :
                channelClosedCount += 1

        # The following message indicates that a decode has just started
        # BXDM_PPDBG: --- 0:[00.xxx] BXDM_PictureProvider_StartDecode_isr has been called (hXdmPP:0x1049256) ---
        #
        # Create a context for the channel if it doesn't exist.
        # Create a context for the decode.
        elif "BXDM_PictureProvider_StartDecode_isr" in inputLine :
            dbgMsg = xdm_utils.BxdmPpDbgMessage(inputLine)
            channelId = dbgMsg.get_channel_id()

            if channelId == -1 :
                logging.info("failed to get channel ID at decode start")
                print inputLine
                continue

            # If this is the first decode for this channel,
            # create the required channel context object.
            if channelId not in channelData :
                channelData[channelId] = xdm_utils.ChannelContext()

            # Create and initialize the decode context
            currentDecode = channelData[channelId].add_decode()

            # initialize the parameters that will be need at the end
            currentDecode.set_parameter("frameRate", 0)
            currentDecode.set_parameter("averagePts", 0)
            currentDecode.set_parameter("frameRateType", "unknown")

            # sample the start time and picture Id
            currentDecode.set_parameter("searchingForFirstPass", True)

            # Want to ignore the first PPDBG / TM: messge.  Really want to
            # ignore the initial cases of the delivery queue being empty.
            currentDecode.set_parameter("foundFirstTmMsg", False)

            currentDecode.set_parameter("decode active", True)


        # Exit this loop if StopDecode has been called.  This is only needed
        # for test runs when the stream wraps, i.e. the stream is shorter
        # than the test time.
        # BXDM_PPDBG: --- 4:[00.xxx] BXDM_PictureProvider_StopDecode_isr has been called (hXdmPP:0x1049256) ---
        elif "BXDM_PictureProvider_StopDecode_isr" in inputLine:
            # break

            currentDbgChannel = -1

            dbgMsg = xdm_utils.BxdmPpDbgMessage(inputLine)
            channelId = dbgMsg.get_channel_id()

            if channelId == -1 :
                logging.info("failed to get channel ID at decode stop")
                print inputLine
                continue

            if channelId not in channelData :
                logging.info("decode stop for a non-existent channel")
                continue

            # Create and initialize the decode context
            currentDecode = channelData[channelId].get_current_decode()

            if currentDecode is None :
                logging.info("decode stop: failed to get decode object")
                continue

            currentDecode.set_parameter("decode active", True)

        #
        # Process the PPQM messages.
        #
        elif "PPQM" in inputLine :

            qmMsg = xdm_utils.BxdmPpqmMessage(inputLine)

            channelId = qmMsg.get_channel_id()

            if channelId not in channelData :
                continue

            currentDecode = channelData[channelId].get_current_decode()

            if currentDecode == None :
                logging.info("failed to get decode object for current channel")
                continue

            # Extract the time stamp from the input line.
            timeStamp = xdm_utils.get_timestamp(inputLine)

            # If the "bonus" QM message (as indicated by the "afr:" string), snapshot the appropriate data.
            # Use this message to start collecting data for a given channel.
            # In general this should be the first PPQM message received.
            afrIndex = inputLine.find("afr:")

            if  afrIndex != -1 :

                # extract and verify the frame rate, only store the
                # value if it is between 0 and 120
                hzIndex = inputLine.find("Hz", afrIndex)
                if hzIndex != -1 :
                    try :
                        frameRate = float(inputLine[ afrIndex + 4 : hzIndex ])
                    except :
                        logging.info("failed to convert frameRate")
                        print inputLine
                        frameRate = 0

                    if frameRate >= 0 or frameRate <= 120 :
                        currentDecode.set_parameter("frameRate", frameRate)
                #
                # extract and verify the frame rate type
                #
                openIndex = inputLine.find("(", afrIndex)
                closeIndex = inputLine.find(")", afrIndex)
                frameRateType = inputLine[openIndex + 1 : closeIndex ]

                if frameRateType in xdm_utils.validFrameRateTypes :
                    currentDecode.set_parameter("frameRateType", frameRateType)

                #
                # extract and verify the average PTS value
                #
                ptsIndex = inputLine.find("avPts:", afrIndex)
                spaceIndex = inputLine.find(" ", ptsIndex)
                if ptsIndex != -1 :
                    try :
                        averagePts = float(inputLine[ ptsIndex + 6 : spaceIndex ])
                    except :
                        logging.info("failed to convert average PTS")
                        print inputLine
                        averagePts = 0

                    currentDecode.set_parameter("averagePts", averagePts)

                # currentDecode.dump_parameters()

            elif "pts" in inputLine :
                if channelId in channelData :
                    # The picture ID is need for a variety
                    pictureId = qmMsg.get_picture_id()

                    if pictureId == -1 :
                        continue


                    # The assumption is that "searchingForFirstPass" will only be true at the start
                    # of decode.  Snapshot the information for the first picture which
                    # passes TSM.  This will need to be tweaked if stream wrap
                    # needs to be supported.
                    # We will only hit this code block after the first "bonus" QM message
                    # has been received and the channel context has been allocated.

                    if currentDecode.get_parameter("searchingForFirstPass") is True :

                        # need to find the TSM test result.
                        bracketIndex = inputLine.find("]")
                        colonIndex = -1
                        tsmResult = "u"

                        if bracketIndex != -1 :
                            colonIndex = inputLine.find(":", bracketIndex)
                            if colonIndex != -1 :
                                tsmResult = inputLine[colonIndex + 1 : colonIndex + 2]

                        # if the test passed, snap shot the data
                        if tsmResult == "p" :
                            currentDecode.set_parameter("searchingForFirstPass", False)
                            currentDecode.set_parameter("startId", pictureId)
                            currentDecode.set_parameter("startTime", timeStamp)

                    currentDecode.set_parameter("currentId", pictureId)
                    currentDecode.set_parameter("currentTime", timeStamp)

        elif "PPDBG" in inputLine :

            # Check for the first line in the block of PPDBG messages.

            if inputLine.find("DM Log") != -1 :
                # If this is the "BXDM_PPDBG:  DM Log" message, extract the
                # channel ID.  This ID will be associated with all subsequent
                # "PPDBG" messages until the next "DM Log" message.
                index = inputLine.find("ch:")

                # Check index just in case there is corruption
                if index != -1 :
                    currentDbgChannel = inputLine[ index + 3 : index + 5 ]
                else:
                    currentDbgChannel = -1

            elif inputLine.find("TM:") != -1 :

                # If this channel is being processed, check for delivery queue underflows

                if currentDbgChannel in channelData :

                    currentDecode = channelData[channelId].get_current_decode()

                    if currentDecode == None :
                        logging.info("failed to get decode object for current channel")
                        continue

                    if currentDecode.get_parameter("searchingForFirstPass") is False :
                        if currentDecode.get_parameter("foundFirstTmMsg") is False :
                            currentDecode.set_parameter("foundFirstTmMsg", True)
                            currentDecode.set_parameter("dqUnderflow", 0)
                        else:
                            # The string ':-' indicates a delivery queue underflow. The underflows
                            # at the start of decode are filtered out by the code which searches
                            # for the first passing picture.  All the machinations here are to filter
                            # out the underflows at the end of decode.  In particular the scenario where
                            # the stream length is shorter than the test time, i.e. the stream wraps.
                            # Walk through the 'TM:' message:
                            # - count each ':-' to determine the number of underflows
                            # - save the underflow count when there is a ':-' -> ':+' transition
                            # The preceding transition won't occur when the stream wraps.

                            (trash, sep, goodPart) = inputLine.partition('TM: ')
                            splitLine = goodPart.split(' ')

                            for element in splitLine :
                                if ':+' in element :
                                    currentCount = currentDecode.get_parameter("dqUnderflow")
                                    currentCount += underflowCount
                                    currentDecode.set_parameter("dqUnderflow", currentCount)
                                    underflowCount = 0
                                elif ':-' in element :
                                    underflowCount += 1


    # end of "for inputLine in srcFile :"

    testFailed = False
    dqUnderflow = False

    print "\n----- results -----"


    if channelChangeTest is True :
        if channelClosedCount == 0 :
            print "channel was not closed between decodes"
        else:
            print "<<Warning>> channel was closed %d times!!!!" % (channelClosedCount)

    if len(channelData) == 0 :
        testFailed = True
        print "There is an issue processing the results log."
        print "Verify the contents of runner.log, it should contain PPQM and PPDBG messages."

    for channel in channelData :
        # channelData[channel].dump_parameters()

        for index in range(channelData[channel].get_num_decodes()) :

            print "\nchannel: %s decode: %d" % (channel, index)

            decode = channelData[channel].get_decode(index)

            underflowCount = decode.get_parameter("dqUnderflow")

            pictureCount = decode.get_parameter("currentId") - decode.get_parameter("startId") + 1
            print "first picture: 0x%x last picture: 0x%x total pictures: 0x%x (%d)" % (decode.get_parameter("startId"), decode.get_parameter("currentId"), pictureCount, pictureCount)

            elapseTime = xdm_utils.calculate_elapse_time(decode.get_parameter("startTime"), decode.get_parameter("currentTime"))

            # Use the frame rate if the type is "cod" (coded) or "frd" (frame rate detected).
            # Otherwise use the average PTS value to calculated the frame rate.
            # Why not use just one of the above?  Need to use "cod" for "es" streams.  The
            # frame rates provided by the "def" (default) and  "hcd" (hard coded) cases
            # were not accurate, hence the need to use the average PTS value.

            frameRateType = decode.get_parameter("frameRateType")
            print "frame rate type is: " + frameRateType
            if frameRateType == "cod" or frameRateType == "frd" :
                frameRate = decode.get_parameter("frameRate")
                print "using the frame rate of " + str(frameRate)

            else:
                averagePts = decode.get_parameter("averagePts")
                if averagePts == 0 :
                    print "<<Warning>> average PTS is 0."
                    testFailed = True
                    frameRate = 30
                else :
                    frameRate = 45000 / averagePts

                print "using the average PTS " + str(averagePts) + " to calcuate a frame rate of %0.2f" % (frameRate)

            expectedCount = (elapseTime * frameRate) / 1000
            print "expected picture: %d = ( %d * %0.2f ) / 1000" % (expectedCount, elapseTime, frameRate)

            # If the delivery queue underflows when using a USB stick, reduce the expected picture count
            # based on the number of times the delivery queue underflowed.  Also, flag this scenario
            # with a warning.
            if underflowCount != 0:
                dqUnderflow = True

                # Treat underflows as a warning for now.
                print "<<Warning>> the delivery queue undeflowed %d times!" % (underflowCount)

                if runningFromUsbStick is True :
                    print "Looks like the system is using a USB stick"
                else :
                    print "Looks like the system is using a hard drive, not a USB stick."

                # This calculation should probably take the pulldown and frame rate into account.
                expectedCount -= underflowCount
                print "reducing expected picture to: %d" % (expectedCount)

            # Consider the test a success if the number of pictures delivered 98% of the expected number delivered.
            # A little fudging to deal with start up issues and queue underflows.
            if pictureCount > (expectedCount * 0.98) :
                print "test passed: actual picture count %d is greater than expected count %d (* 0.98 fudge factor)" % (pictureCount, expectedCount)
            else :
                print "test failed: actual picture count %d is NOT greater than expected count %d  (* 0.98 fudge factor)" % (pictureCount, expectedCount)
                testFailed = True

    if testFailed is True :
        print "\nBAT_RETURNCODE[1]"
    elif dqUnderflow is True and runningFromUsbStick is True :
        print "\nBAT_RETURNCODE[BAT_WARNING]"
    else :
        print "\nBAT_RETURNCODE[0]"

    return testFailed

    # end of "main"

if __name__ == "__main__":
    import sys

    print sys.version

    # version check
    if sys.version_info < (2, 7):
        print "Require python 2.7 or greater"
        sys.exit(-1)

    if sys.version_info >= (3, 0):
        print "Warning! This package has not been verified for use with Python 3.x"

    # just to help with debug
    i = 0
    for arg in sys.argv:
        print "%d:%s" % (i, arg)
        i += 1

    sys.exit(main(sys.argv[1:]))
