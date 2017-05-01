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
import os
# import parse_xdm_utils as xutil

#
# main processing loop
#
def main(argv) :
    """ main function """

    logging.basicConfig(format="--- %(funcName)s: %(message)s", level=logging.INFO)

    channelData = dict()

    # create an argument parser
    parser = argparse.ArgumentParser(description='AVD CRC verification')

    # Note: the first long option name will become an object of "inputArgs"
    # can also be specified with 'dest='
    parser.add_argument('-g', '--goldenFile', action='store', help='golden file name')
    parser.add_argument('-r', '--resultsFile', action='store', help='results file name')
    parser.add_argument('-c', '--coreRevision', action='store', help='AVD core revision of the chip under test')
    parser.add_argument('-p', '--videoProtocol', action='store', help='video protocol of the stream that was played')

    inputArgs = parser.parse_args()


    #
    # Was a golden file specified?
    #
    createGoldenFile = False
    (goldenPath, separator, goldenFileName) = inputArgs.goldenFile.rpartition('/')
    if len(goldenFileName) == 0 :
        print "<<ERROR>> failed to specify a golden file."
        print "\nBAT_RETURNCODE[1]"
        return True
    else :
        print "goldenPath: %s" % (goldenPath)
        print "goldenFileName: %s" % (goldenFileName)

        # Open the golden file if it exists.
        if os.path.isfile(inputArgs.goldenFile) is True :
            goldenFile = open(inputArgs.goldenFile, "r")

        else :
            # Create the golden file after opening the results file.
            createGoldenFile = True

    #
    # Was a results file specified?
    #
    (resultsPath, separator, resultsFileName) = inputArgs.resultsFile.rpartition('/')
    if len(resultsFileName) == 0 :
        print "<<ERROR>> failed to specify a results file."
        print "\nBAT_RETURNCODE[1]"
        return True
    else :
        print "resultsPath: %s" % (resultsPath)
        print "resultsFileName: %s" % (resultsFileName)

        # Open the results file if it exists.
        if os.path.isfile(inputArgs.resultsFile) is True :
            resultsFile = open(inputArgs.resultsFile, "r")
            print "Opening results file: %s" % (inputArgs.resultsFile)

            if createGoldenFile == True :
                # Create the golden file; search for lines containing the CRC data and
                # write these to the new golden file. Prefix the core revision to the name
                # to make it easier to merge generated files from different platforms.

                newGoldenName = os.path.join(resultsPath, "rev" + "%s" % (inputArgs.coreRevision) + "_" + goldenFileName)
                print "<<Error>> golden file does not exist."
                print "<<Error>> creating %s" % (newGoldenName)
                goldenFile = open(newGoldenName, "w")

                goldenFile.write("video protocol: %s\n" % (inputArgs.videoProtocol))
                goldenFile.write("AVD core revision: %s\n" % (inputArgs.coreRevision))
                for inputLine in resultsFile :
                    if inputLine.find("AVD CRC") != -1 :
                        goldenFile.write(inputLine)

                # needed to work around the cleanup code which was deleting the newly created file
                print "\nBAT_RETURNCODE[1]"
                return True

            else :
                # The golden file exists, so filter the results file to simplify the comparison.
                (fileName, separator, extension) = resultsFileName.rpartition('.')
                filteredName = os.path.join(resultsPath, fileName + "_filtered.txt")
                print "filtered results file:\n %s" % (filteredName)
                filteredFile = open(filteredName, "w")

                filteredFile.write("video protocol: %s\n" % (inputArgs.videoProtocol))
                filteredFile.write("AVD core revision: %s\n" % (inputArgs.coreRevision))
                for inputLine in resultsFile :
                    if inputLine.find("AVD CRC") != -1 :
                        filteredFile.write(inputLine)

                filteredFile.close()

        else :
            print "<<ERROR>> failed to open the results file %s." % (inputArgs.resultsFile)
            print "\nBAT_RETURNCODE[1]"
            return True

    testFailed = False

    #
    # Compare the filtered results file with the golden file.
    #
    matches = 0
    mismatches = 0

    goldenSynced = False

    # Search for the correct CRC in the golden file.
    # For H264 need to use different data for pre-revision S cores.

    onlyCompareTopData = False


    print "protocol: %s" % (inputArgs.videoProtocol)
    print "core rev: %s" % (inputArgs.coreRevision)

    stringToFind = "revision"

    if inputArgs.videoProtocol == "H264" or inputArgs.videoProtocol == "MPEG2" :
        # stringToFind = "revision pre-S" if inputArgs.coreRevision < 'S' else  "revision S"
        if inputArgs.coreRevision < 'S' :
            stringToFind = "revision pre-S"
        else:
            stringToFind = "revision S"
            onlyCompareTopData = True

    goldenData = goldenFile.readline()
    while goldenData :
        if goldenData.find(stringToFind) != -1 :
            break
        goldenData = goldenFile.readline()

    # Now search for the next AVD CRC line in the golden file
    goldenData = goldenFile.readline()
    while goldenData :
        if goldenData.find("AVD CRC") != -1 :
            break
        goldenData = goldenFile.readline()


    filteredFile = open(filteredName, "r")

    for resultData in filteredFile :

        # Search for the first AVD CRC line in the result file
        if resultData.find("AVD CRC") == -1 :
            continue

        if onlyCompareTopData is True :
            (resultData, separator, unused) = resultData.partition(';')
            (goldenData, separator, unused) = goldenData.partition(';')

        if '***' in resultData :
            print "<<Warning>> Ignoring the following warning in the results data"
            print resultData
        elif resultData.rstrip() != goldenData.rstrip() :
            print "values don't match"
            print "goldenData:",
            print repr(goldenData.rstrip())
            print "resultData:" ,
            print repr(resultData.rstrip())
            mismatches += 1
        else:
            matches += 1

        if mismatches >= 10 :
            print "\nBAT_RETURNCODE[1]"
            return True

        goldenData = goldenFile.readline()

        # Break here if we've reached the end of the golden file
        # or the line "---" which separates the blocks of golden data.
        if not goldenData or "---" in goldenData :
            # print "reached the end of golden data before the end of the results file"
            break

    print "matches: %d" % (matches)
    print "mismatches: %d" % (mismatches)

    if testFailed is True :
        print "\nBAT_RETURNCODE[1]"
    else :
        print "\nBAT_RETURNCODE[0]"

    return testFailed


    # Indicates with which channel the PPDBG messages are associated.
    currentDbgChannel = -1

    runningFromUsbStick = False

    # interate over the lines in "srcFile"
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

        # Find the PPQM messages
        elif inputLine.find("PPQM") != -1 :
            channelId = ""
            pictureId = 0

            # Extract the channel Id and picture Id.  Perhaps this is over kill,
            # but assume that the log can be incomplete and handle corrupted data.
            indexOpen = inputLine.find(":[")
            indexPeriod = inputLine.find(".", indexOpen)
            indexClose = inputLine.find("]", indexPeriod)

            if indexOpen != -1 and indexPeriod != -1 and indexClose != -1 :
                channelId = inputLine[ indexOpen + 2 : indexPeriod ]
                try :
                    pictureId = int(inputLine[ indexPeriod + 1 : indexClose ], 16)
                except :
                    if inputLine[ indexPeriod + 1 : indexClose ] != "xxx" :
                        logging.info("failed to convert picture Id string to an int")
                        print inputLine
                        continue

            # Extract the time stamp from the input line.
            timeStamp = xutil.extract_time(inputLine)

            # If the "bonus" QM message (as indicated by the "afr:" string), snapshot the appropriate data.
            # Use this message to start collecting data for a given channel.
            # In general this should be the first PPQM message received.
            afrIndex = inputLine.find("afr:")

            if  afrIndex != -1 :

                # First check the channel number.  If this is the first data sample
                # for this channel, create the required channel context object.
                if channelId not in channelData :
                    channelData[channelId] = xutil.ChannelContext()

                    # initialize the parameters that will be need at the end
                    channelData[channelId].set_parameter("frameRate", 0)
                    channelData[channelId].set_parameter("averagePts", 0)
                    channelData[channelId].set_parameter("frameRateType", "unknown")

                    # sample the start time and picture Id
                    channelData[channelId].set_parameter("searchingForFirstPass", True)

                    # Want to ignore the first PPDBG / TM: messge.  Really want to
                    # ignore the initial cases of the delivery queue being empty.
                    channelData[channelId].set_parameter("foundFirstTmMsg", False)


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
                        channelData[channelId].set_parameter("frameRate", frameRate)
                #
                # extract and verify the frame rate type
                #
                openIndex = inputLine.find("(", afrIndex)
                closeIndex = inputLine.find(")", afrIndex)
                frameRateType = inputLine[openIndex + 1 : closeIndex ]

                if frameRateType in xutil.validFrameRateTypes :
                    channelData[channelId].set_parameter("frameRateType", frameRateType)

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

                    channelData[channelId].set_parameter("averagePts", averagePts)

                # channelData[channelId].dump_parameters()

            else :
                if channelId in channelData :
                    # The assumption is that "searchingForFirstPass" will only be true at the start
                    # of decode.  Snapshot the information for the first picture which
                    # passes TSM.  This will need to be tweaked if stream wrap
                    # needs to be supported.
                    # We will only hit this code block after the first "bonus" QM message
                    # has been received and the channel context has been allocated.

                    if channelData[channelId].get_parameter("searchingForFirstPass") is True :

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
                            channelData[channelId].set_parameter("searchingForFirstPass", False)
                            channelData[channelId].set_parameter("startId", pictureId)
                            channelData[channelId].set_parameter("startTime", timeStamp)

                    channelData[channelId].set_parameter("currentId", pictureId)
                    channelData[channelId].set_parameter("currentTime", timeStamp)

        elif inputLine.find("PPDBG") != -1 :

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
                    if channelData[currentDbgChannel].get_parameter("searchingForFirstPass") is False :
                        if channelData[currentDbgChannel].get_parameter("foundFirstTmMsg") is False :
                            channelData[currentDbgChannel].set_parameter("foundFirstTmMsg", True)
                            channelData[currentDbgChannel].set_parameter("dqUnderflow", 0)
                        else:
                            underFlowCount = channelData[currentDbgChannel].get_parameter("dqUnderflow")
                            underFlowCount += inputLine.count(":-")
                            channelData[currentDbgChannel].set_parameter("dqUnderflow", underFlowCount)

    # end of "for inputLine in srcFile :"

    testFailed = False
    dqUnderflow = False

    for channel in channelData :
        # channelData[channel].dump_parameters()

        print "\nchannel: %s" % (channel)

        underFlowCount = channelData[channel].get_parameter("dqUnderflow")

        pictureCount = channelData[channel].get_parameter("currentId") - channelData[channel].get_parameter("startId") + 1
        print "first picture: 0x%x last picture: 0x%x total pictures: 0x%x (%d)" % (channelData[channel].get_parameter("startId"), channelData[channel].get_parameter("currentId"), pictureCount, pictureCount)

        elapseTime = xutil.calculate_elapse_time(channelData[channel].get_parameter("startTime"), channelData[channel].get_parameter("currentTime"))

        # Use the frame rate if the type is "cod" (coded) or "frd" (frame rate detected).
        # Otherwise use the average PTS value to calculated the frame rate.
        # Why not use just one of the above?  Need to use "cod" for "es" streams.  The
        # frame rates provided by the "def" (default) and  "hcd" (hard coded) cases
        # were not accurate, hence the need to use the average PTS value.

        frameRateType = channelData[channel].get_parameter("frameRateType")
        print "frame rate type is: " + frameRateType
        if frameRateType == "cod" or frameRateType == "frd" :
            frameRate = channelData[channel].get_parameter("frameRate")
            print "using the frame rate of " + str(frameRate)

        else:
            averagePts = channelData[channel].get_parameter("averagePts")
            frameRate = 45000 / averagePts

            print "using the average PTS " + str(averagePts) + " to calcuate a frame rate of %0.2f" % (frameRate)

        expectedCount = (elapseTime * frameRate) / 1000
        print "expected picture: %d = ( %d * %0.2f ) / 1000" % (expectedCount, elapseTime, frameRate)

        # If the delivery queue underflows when using a USB stick, reduce the expected picture count
        # based on the number of times the delivery queue underflowed.  Also, flag this scenario
        # with a warning.
        if underFlowCount != 0:
            dqUnderflow = True
            if runningFromUsbStick is True :
                print "<<Warning>> the delivery queue undeflowed %d times!" % (underFlowCount)
                print "Looks like the system is using a USB stick"
                # This calculation should probably take the pulldown and frame rate into account.
                expectedCount -= underFlowCount
                print "reducing expected picture to: %d" % (expectedCount)
            else :
                print "The delivery queue undeflowed %d times!" % (underFlowCount)
                print "Looks like the system is using a hard drive, not a USB stick."

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
