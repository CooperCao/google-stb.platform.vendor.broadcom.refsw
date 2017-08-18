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
import new

#
# Global state
#
bxdmQmMsg = []
bxvdDqtMsg = []

# Check that the firmware is reporting a non-zero value for both the
# number of available picture buffers and the number of open GOP pictures.
# If either of these is zero, the firmware is broken. (or the platform
# doesn't support multi-pass DQT).

def verify_hvd_parameters():
    numberOfMemoryBuffers = 0
    numberOfOpenGopPics = 0
    testFailed = False

    for bxvdDqt in bxvdDqtMsg :
        dbgMsg = bxvdDqt.get_parameter(xdm_utils.DBG_MSG)
        if "BXVD_Decoder_S_MPDQT_EOG_isr" in dbgMsg and "mbuffs:" in dbgMsg and "oPics:" in dbgMsg :
            # print dbgMsg ,

            # BXVD_Decoder_S_MPDQT_EOG_isr:: mbuffs:10 oPics:8
            idx = dbgMsg.find("mbuffs:")

            if idx != -1 :
                idx += len("mbuffs:")

                subStr = dbgMsg[ idx : idx + 2 ]

                try :
                    numberOfMemoryBuffers = int(subStr, 10)
                except :
                    numberOfMemoryBuffers = 0

            idx = dbgMsg.find("oPics:")

            if idx != -1 :
                idx += len("oPics:")

                subStr = dbgMsg[ idx : idx + 2 ]

                try :
                    numberOfOpenGopPics = int(subStr, 10)
                except :
                    numberOfOpenGopPics = 0

    # NOTE: the values being evaluated will be for the last message in the list.
    # What if the message is corrupted?
    if numberOfMemoryBuffers == 0 or numberOfOpenGopPics == 0 :
        testFailed = True
        print "ERROR: the parameter check failed. Both the number of memory buffers and open GOP pictures should be non-zero."
        print "\tThis is a firmware issue (or else the platform doesn't support multi-pass DQT)."
        print "\tNumber of memory buffers: %d" % (numberOfMemoryBuffers)
        print "\tNumber of open GOP pictures: %d" % (numberOfOpenGopPics)
    else :
        print "PASS: the parameter check passed. The number of memory buffers and open GOP pictures are both non-zero."
        print "\tNumber of memory buffers: %d" % (numberOfMemoryBuffers)
        print "\tNumber of open GOP pictures: %d" % (numberOfOpenGopPics)

    return testFailed

# end of verify_hvd_parameters()

def verify_gop_handling_dqtv1():

    successful = 0  # the number of succesful transitions
    failed = 0  # the number of failed transitions
    currentTag = -1
    nextTag = -1
    testFailed = False

    # For DQT V1 check that we are transitioning from one GOP to the next correctly.
    # The following needs to be made more robust to handle
    # corruption in the log file.

    # example:
    # BXVD_Decoder_S_UnifiedQ_Update_isr:: EOG: picture tag changed: current: 0 new: 1
    # BXVD_Decoder_S_UnifiedQ_Update_isr:: EOG: picture tag changed: current: 1 new: 2

    for bxvdDqt in bxvdDqtMsg :
        dbgMsg = bxvdDqt.get_parameter(xdm_utils.DBG_MSG)

        if 'EOG: picture tag changed' in dbgMsg :
            msg = dbgMsg.split(' ')

            if msg[-4] == 'current:' :
                try :
                    current = int(msg[-3], 10)
                except :
                    current = -1
            else :
                current = -1

            if msg[-2] == 'new:' :
                try :
                    new = int(msg[-1].strip(), 10)
                except :
                    new = -1
            else :
                new = -1

            if current == -1 or new == -1 :
                currentTag = -1
                newTag = -1
            elif currentTag == -1 or newTag == -1 :
                currentTag = current
                newTag = new
            else :
                if currentTag + 1 == newTag and newTag == current and newTag + 1 == new :
                    successful += 1
                else :
                    failed += 1

                currentTag = current
                newTag = new


    # Check that a reasonable number of GOPs were processed. 10 is an arbitrary
    # number, it would be better to calculate the  value.

    if successful < 10 :
        testFailed = True
        print "ERROR: the GOP count failed.  Only %d GOPs were processed" % (successful)
    else :
        print "PASS: the GOP count passed, %d GOPs were processed" % (successful)

    if failed != 0 :
        testFailed = True
        print "ERROR: a failure when transitioning to the next GOP.  There were %d failures." % (failed)

    return testFailed

# end of verify_gop_handling_dqtv1()


def verify_gop_handling_multipass_dqt():
    searchMode = "sync"
    triggerIndex = 0
    pictureIndex = 0
    chunkTestFailed = False
    numberOfChunks = 0
    numberOfGOPs = 0

    testFailed = False

    # For multipass DQT, check that we are transitioning correctly and handling
    # each pass of a GOP correctly.
    # The following needs to be made more robust to handle
    # corruption in the log file.

    for bxvdDqt in bxvdDqtMsg :
        dbgMsg = bxvdDqt.get_parameter(xdm_utils.DBG_MSG)

        if "BXVD_Decoder_S_UnifiedQ_Update_isr" in dbgMsg or "BXVD_Decoder_S_EndOfGOP_isr" in dbgMsg :

            # Look for stings indicating the end of a GOP.
            if "next pass: 0" in dbgMsg : numberOfGOPs += 1

            # Search until we find the first "detected last picture flag" message
            if searchMode is "sync" :
                if "detected last picture flag" in dbgMsg :
                    searchMode = "trigger index"

            elif searchMode is "trigger index" :
                if "trigger index for next pass:" in dbgMsg :

                    idx = dbgMsg.find("trigger index for next pass: ")

                    if idx != -1 :
                        idx += len("trigger index for next pass: ")

                        subStr = dbgMsg[ idx : idx + 2 ]

                        try :
                            triggerIndex = int(subStr, 10)
                            searchMode = "picture index"
                        except :
                            triggerIndex = 0

            elif searchMode is "picture index" :
                if "EOG:" in dbgMsg :

                    # Looking for either of the following messages to indicate XVD has received all
                    # of the pictures associated with the current chunk.
                    # "EOG: hit target picture idx" -- for all but the final chunk
                    # "EOG: detected last picture flag." -- for the final chunk

                    if "detected last picture flag" in dbgMsg :
                        if triggerIndex == 0 :
                            searchMode = "trigger index"
                            numberOfChunks += 1
                        else :
                            chunkTestFailed = True

                    elif "hit target picture idx" in dbgMsg :

                        idx = dbgMsg.find("hit target picture idx:")

                        if idx != -1 :
                            idx += len("hit target picture idx:")

                            subStr = dbgMsg[ idx : idx + 2 ]

                            try :
                                pictureIndex = int(subStr, 10)

                                searchMode = "trigger index"
                                numberOfChunks += 1
                                if triggerIndex != pictureIndex :
                                    chunkTestFailed = True
                            except :
                                pictureIndex = 0

    # Check that a reasonable number of GOPs were processed. 3 is an arbitrary
    # number, it would be better to calculate the  value.  The number varies
    # from platform to platform, it seems that the MIPS platforms process
    # fewer GOPs.  Is this due to the CPU performance?

    if numberOfGOPs < 3 :
        testFailed = True
        print "ERROR: the GOP count failed.  Only %d GOPs were processed" % (numberOfGOPs)
    else :
        print "PASS: the GOP count passed, %d GOPs were processed" % (numberOfGOPs)

    if chunkTestFailed is False :
        print "PASS: chunk test passed."
    else :
        testFailed = True
        print "ERROR: chunk test failed!"

    # Check that a reasonable number of chunks were processed. 10 is an arbitrary
    # number, it would be better to calculate the  value.

    if numberOfChunks < 10 :
        testFailed = True
        print "ERROR: the chunk count failed.  Only %d chunks were processed" % (numberOfChunks)
    else :
        print "PASS: the chunk count passed, %d chunks were processed" % (numberOfChunks)

    return testFailed

# end of verify_gop_handling_multipass_dqt():


#
# main processing loop
#
def main(argv) :
    """ main function """

    logging.basicConfig(format="--- %(funcName)s: %(message)s", level=logging.INFO)

    testFailed = False

    # create an argument parser
    parser = argparse.ArgumentParser(description='log input commands')

    # Note: the first long option name will become an object of "inputArgs"
    # can also be specified with 'dest='
    parser.add_argument('-i', '--input', '--inp', action='store', help='Input file name.')
    parser.add_argument('--dqt', action='store_true', default=False, help='Input file name.')

    inputArgs = parser.parse_args()
    inputFileName = inputArgs.input
    dqtV1 = inputArgs.dqt

    srcFile = open(inputFileName, "r")

    bFoundWrapPoint = False
    bDeadlockDetected = False

    #
    # pull out the PPQM and DQT messages from srcFile
    #
    for inputLine in srcFile :

        # This may not be necessary, but go with it for now.  Search for the string "beginning of stream"
        # before processing data.  This should get us to a clean starting point after the stream wraps.
        # Also beware that this message comes from Nexus playback and could go away.  Alternative approaches
        # would be to look for the picture number going to '0' or using the BXDM_PPDBG messages
        # to find where StartDecode is called.
        # And if we hit "beginning of stream" a second time, the stream has wrapped again. To simplfy
        # verification, stop parsing the data.

        if bFoundWrapPoint == False :
            if "beginning of stream" in inputLine :
                bFoundWrapPoint = True
            continue
        elif "beginning of stream" in inputLine :
            # The stream has wrapped a second time.
            break

        # Save the PPQM messages
        if "PPQM" in inputLine :
            bxdmQmMsg.append(xdm_utils.BxdmPpqmMessage(inputLine))

        # Save the BXVD_DQT messages
        elif "BXVD_DQT" in inputLine :
            bxvdDqtMsg.append(xdm_utils.BxvdDqtMessage(inputLine))

            # Indicates a system failure
            if "deadlock"  in inputLine :
                bDeadlockDetected = True

    # end of for inputLine in srcFile

    # If the wrap is not dectected, the stream probably didn't rewind.
    if bFoundWrapPoint is False :
        print "\nERROR: the stream failed to wrap"
        testFailed = True


    # Check for a deadlock, this indicates a system problem, perhaps in Nexus
    # or the HVD firmware. Should we abort processing now?
    if bDeadlockDetected is True :
        print "\nERROR: delivery queue deadlock detected"
        testFailed = True

    #
    # First check that pictures were delivered
    #
    if len(bxdmQmMsg) == 0 or len(bxvdDqtMsg) == 0 :
        print "\nERROR: number of PPQM messages %d number of DQT messages %d" % (len(bxdmQmMsg), len(bxvdDqtMsg))
        print "They should both be non-zero, it looks like there is an issue with decoding."
        testFailed = True
    #
    # Check that the PTS values are all in descending order.
    #
    ptsCheckFailed = False
    for index in range(len(bxdmQmMsg) - 1) :
        # If the current index and the next are valid, check that the next one is less than the current one.
        if  (bxdmQmMsg[index].get_parameter(xdm_utils.PTS_STATE) == True and
              bxdmQmMsg[index + 1].get_parameter(xdm_utils.PTS_STATE) == True) :

            if bxdmQmMsg[index].get_parameter(xdm_utils.PTS_STATE) < bxdmQmMsg[index + 1].get_parameter(xdm_utils.PTS_STATE) :
                print "\nERROR: Next PTS was not less than the current PTS"
                print "Current PTS: %x %d " % (bxdmQmMsg[index].get_parameter(xdm_utils.PTS), bxdmQmMsg[index].get_parameter(xdm_utils.PTS_STATE))
                print "Next PTS: %x %d " % (bxdmQmMsg[index + 1].get_parameter(xdm_utils.PTS), bxdmQmMsg[index + 1].get_parameter(xdm_utils.PTS_STATE))
                ptsCheckFailed = True
                break

    if ptsCheckFailed == True :
        testFailed = True
    else :
        print "\nPASS: PTS check passed, the values were all in descending order."

    # For multipass DQT, check that the firmware is reporting a non-zero value for both the
    # number of available picture buffers and the number of open GOP pictures. If either of
    # these is zero, the firmware is broken. (or the platform doesn't support multi-pass DQT).

    if dqtV1 is False :
        testFailed |= verify_hvd_parameters()

    # For DQT V1 check that we are transitioning from one GOP to the next correctly.
    # For multipass DQT, check that we are transitioning correctly and handling
    # each pass of a GOP correctly.

    if dqtV1 is True :
        testFailed |= verify_gop_handling_dqtv1()
    else :
        testFailed |= verify_gop_handling_multipass_dqt()

    if testFailed == True :
        print "\nBAT_RETURNCODE[1]"
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
