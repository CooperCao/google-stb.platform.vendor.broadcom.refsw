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

    # create an argument parser
    parser = argparse.ArgumentParser(description='AVD CRC verification')

    # Note: the first long option name will become an object of "inputArgs"
    # can also be specified with 'dest='
    parser.add_argument('-g', '--goldenFile', action='store', help='golden file name')
    parser.add_argument('-r', '--resultsFile', action='store', help='results file name')
    parser.add_argument('-c', '--coreRevision', action='store', help='AVD core revision of the chip under test')
    parser.add_argument('-p', '--videoProtocol', action='store', help='video protocol of the stream that was played')

    inputArgs = parser.parse_args()

    runningFromUsbStick = False

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

#===============================================================================
#             else :
#                 # The golden file exists, so filter the results file to simplify the comparison.
#                 (fileName, separator, extension) = resultsFileName.rpartition('.')
#                 filteredName = os.path.join(resultsPath, fileName + "_filtered.txt")
#                 print "filtered results file:\n %s" % (filteredName)
#                 filteredFile = open(filteredName, "w")
#
#                 filteredFile.write("video protocol: %s\n" % (inputArgs.videoProtocol))
#                 filteredFile.write("AVD core revision: %s\n" % (inputArgs.coreRevision))
#
#                 for inputLine in resultsFile :
#                     # The results can be flakey when running from a USB stick.  Flag the
#                     # case to help with debug.
#                     if inputLine.find("disk removable") != -1 :
#                         runningFromUsbStick = inputLine.find("1") != -1
#
#                     if inputLine.find("AVD CRC") != -1 :
#                         filteredFile.write(inputLine)
#
#                 filteredFile.close()
#===============================================================================

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
    ignored = 0

    goldenSynced = False

    # Search for the correct CRC in the golden file.
    # For H264 need to use different data for pre-revision S cores.

    onlyCompareTopData = False


    print "protocol: %s" % (inputArgs.videoProtocol)
    print "core rev: %s" % (inputArgs.coreRevision)

    stringToFind = "revision"

    if inputArgs.videoProtocol == "H264" or inputArgs.videoProtocol == "MPEG2" :
        if inputArgs.coreRevision < 'S' :
            stringToFind = "revision pre-S"
        else:
            stringToFind = "revision S"
            onlyCompareTopData = True

    elif inputArgs.videoProtocol == "VP8" :
        if inputArgs.coreRevision < 'Q' :
            stringToFind = "revision pre-Q"
        else:
            stringToFind = "revision Q"

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


    # filteredFile = open(filteredName, "r")

    # for resultData in filteredFile :
    foundStartOfCrcData = False
    dataInSync = True
    sampleIsValid = False
    for resultData in resultsFile :

        # Search for the first "AVD CRC" string in the result file.
        # Once it is found, all the subsequent strings should be CRC data.
        # However there can be corruption, handle these cases down below.

        if "AVD CRC" not in resultData and foundStartOfCrcData is False :

            # The results can be flakey when running from a USB stick.  Flag the
            # case to help with debug.
            if "disk removable" in resultData :
                runningFromUsbStick = "1" in resultData

            continue

        else :
            foundStartOfCrcData = True

        # Here is a sample of the data.
        # AVD CRC fe76d81 3fcbf25 3f95401; 0 0 0
        # Make a rough guess if the sample is valid based on its format.

        splitSample = resultData.split(' ');
        sampleIsValid = False
        if len(splitSample) == 8 :
            sampleIsValid = ('AVD' == splitSample[0])
            sampleIsValid &= ('CRC' == splitSample[1])
            sampleIsValid &= (';' in splitSample[4])

        # Filter the data.
        if onlyCompareTopData is True :
            (resultData, separator, unused) = resultData.partition(';')
            (goldenData, separator, unused) = goldenData.partition(';')


        # Process the data sample

        if 'Exec' in resultData or 'kill' in resultData :
            # the presence of the kill command indicates that the end is near for
            # the results data. There can be junk in the file from this point on,
            # so stop processing the data.
            break

        elif dataInSync is False :
            # There was a corrupted data sample, find the next clean sample.
            # If this sample is clean, sync up with the golden data by
            # reading through the golden file until there is a match.

            breakFromMainLoop = False  # this is a hack

            if sampleIsValid is False :
                print "searching, ignoring the line:  %s" % (resultData)
                ignored += 1
            else :
                while (1) :

                    if onlyCompareTopData is True :
                        (goldenData, separator, unused) = goldenData.partition(';')

                    if not goldenData or "---" in goldenData :
                        breakFromMainLoop = True
                        break

                    elif resultData.rstrip() == goldenData.rstrip() :
                        dataInSync = True
                        print "resyned CRC values"
                        print "goldenData:",
                        print repr(goldenData.rstrip())
                        print "resultData:" ,
                        print repr(resultData.rstrip())
                        break

                    else :
                        ignored += 1
                        print "bumped golden: CRC values don't match"
                        print "goldenData:",
                        print repr(goldenData.rstrip())
                        print "resultData:" ,
                        print repr(resultData.rstrip())

                    # This read needs to occur after the preceding evaluations.
                    # This is because the golden data is read at the bottom of this
                    # "for" loop, i.e. we are already have the golden data for this
                    # time through the loop.
                    goldenData = goldenFile.readline()

                # end of while ( 1 )

                if breakFromMainLoop is True : break  # This is a hack

        elif sampleIsValid is False :
            # The most common corruption is from a message with the "***" prefix.
            # If this isn't the only way to get corrupt data, the code will need to be smarter.
            # Sometimes the corrupted lines start with "AVD CRC" and sometimes just with "AVD".
            # Generally the message will cause the CRC data to be on two lines.
            # Need to ignore this sample in both the results and golden file.
            # Also need to find the next clean CRC sample.
            print "CRC's are out of sync"
            print "Ignoring the line:  %s" % (resultData)
            ignored += 1
            dataInSync = False

        elif resultData.rstrip() != goldenData.rstrip() :
            print "CRC values don't match"
            print "goldenData:",
            print repr(goldenData.rstrip())
            print "resultData:" ,
            print repr(resultData.rstrip())
            mismatches += 1

        else:
            matches += 1

        # Should we require 100% match or allow some mismatches?
        # If we get mismatches, it should probably be flagged with a warning.
        if mismatches >= 10 :
            print "\nBAT_RETURNCODE[1]"
            return True


        # If the data has gotten out of sync, don't read any more golden
        # data until we are back in sync.

        if dataInSync is True :
            goldenData = goldenFile.readline()

        # Break here if we've reached the end of the golden file
        # or the line "---" which separates the blocks of golden data.
        if not goldenData or "---" in goldenData :
            # print "reached the end of golden data before the end of the results file"
            break

    # matches will be 0 if there isn't any CRC data
    if matches == 0 : testFailed = True

    print "\n----- results -----"
    print "CRC matches: %d" % (matches)
    print "CRC mismatches: %d" % (mismatches)
    print "lines ignored: %d" % (ignored)
    if runningFromUsbStick is True :
        print "note: running from USB stick"

    if testFailed is True :
        print "\nBAT_RETURNCODE[1]"
    else :
        print "\nBAT_RETURNCODE[0]"

    return testFailed

    #
    # end of "main"
    #

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
    # i = 0
    # for arg in sys.argv:
    #    print "%d:%s" % (i, arg)
    #    i += 1

    sys.exit(main(sys.argv[1:]))
