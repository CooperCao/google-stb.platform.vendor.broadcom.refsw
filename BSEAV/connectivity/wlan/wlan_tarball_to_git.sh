#!/bin/bash
#############################################################################
# Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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


function doConfirm() {

    local prompt=$1
    local retSts=0

    if [[ "${unattendedArg}" != "" ]] ; then
        return "${eStsSuccess}"
    fi

    echo ""

    read -p "${prompt} [y/n]: " -r
    if [[ ! $REPLY =~ ^[Yy]$ ]] ; then
        retSts=${eStsUserCancel}
    fi

    echo ""

    return "${retSts}"
}


function doCmdCapture() {

    local cmd=$1                                    # First arg: Command to run
    local statusVar=
    if [[ $# -ge 2 ]] ; then statusVar=$2 ; fi      # Optional 2nd arg: Var to receive completion status
    local captureArray=
    if [[ $# -ge 3 ]] ; then captureArray=$3 ; fi   # Optional 3rd arg: Var to receive captured output
    local lastline
    local redirect="2> >( sed -r -e  \"/^\+---.*---\+\$|^\|   .*  \|\$/d\"  >/dev/stderr)"
    local i=
    local retSts=0
    local sts=0

    #  Global variables for use by caller.
    doCmd_cmdSts=0
    doCmd_captureArray=

    #  Execute the command, then check the readarray status.
    readarray -t doCmd_captureArray < <(eval "${cmd} ${redirect} ; echo \$?")
    sts=$? ; if [[ ${sts} != 0 ]] ; then
        retSts=${sts}
    fi

    if [[ "${retSts}" == "${eStsSuccess}" ]] ; then

        #  The last line of the captured output should be the command's completion status.
        #  Save it in a variable and delete it from the captured output.
        lastline=${#doCmd_captureArray[*]}-1
        doCmd_cmdSts=${doCmd_captureArray[${lastline}]}
        unset doCmd_captureArray[${lastline}]

        #  Now print out the command and the captured output.
        if [[ "${verboseArg}" != "" ]] ; then
            echo "  +------------------------------------------------------------------------------"
            echo "  | \$ ${cmd}"

            for (( i=0; i<${#doCmd_captureArray[*]}; i++ )) ; do
                echo "  | ${doCmd_captureArray[$i]}"
            done
            printf "  | [Exit status: ${doCmd_cmdSts}]\n"
            echo "  +------------------------------------------------------------------------------"
        fi

        #  Save in caller's specified status variable.
        if [[ "${statusVar}" != "" ]] ; then
            eval "${statusVar}=(${doCmd_cmdSts})"
        fi

        #  In case doCmd_captureArray doesn't exist...
        doCmd_captureArray+=

        #  If caller passed a capture array, update it with the captured output.
        if [[ "${captureArray}" != "" ]] ; then
            eval "${captureArray}=( \"\${doCmd_captureArray[@]}\" )"
        fi
    fi

    return ${retSts}
}


function errMsg () {
    local funcStr="$(caller 0 | cut -d " "  -f 2)"
    local lineNumStr="$(caller 0 | cut -d " " -f 1)"

    printf "\n"
    printf "###################################################\n"
    printf "###%30s %5s : $*\n" "${funcStr}" "${lineNumStr}"
    printf "###################################################\n"
    return 0
}


#  Here is a list of function names to supress debug prints for:
dbNoprintFuncs=
#dbNoprintFuncs+=" importTarballToGit "
#dbNoprintFuncs+=" main "

#  This should only be called by xxxPrint functions like infoPrint and vbPrint.
function basePrint () {
    local funcStr="$(caller 1 | cut -d " "  -f 2)"
    local lineNumStr="$(caller 1 | cut -d " " -f 1)"
    local timestamp=$(date "+%Y-%m-%d %H:%M:%S.%3N")

#   if [[ "${dbNoprintFuncs}" =~ [^[:alnum:]]${funcStr}[^[:alnum:]] ]] ; then
    if [[ "${dbNoprintFuncs}" =~ ${funcStr} ]] ; then
        :
    else
        printf "   %s %30s %5s : $*\n" "${timestamp}" "${funcStr}" "${lineNumStr}"
    fi
    return 0
}

function infoPrint () {
    basePrint "$@"
    return 0
}

function vbPrint () {
    if [[ "${verboseArg}" != "" ]] ; then
        basePrint "$@"
    fi
    return 0
}

#  Exit handler
function doCleanup () {

    local sts=

    printf "\n"
    printf "Cleaning up...\n"

    if [[ "${savedBranch}" != "" ]] ; then

        printf "Resetting HEAD...\n"

        doCmdCapture "git reset HEAD -- ${destDir}"
        sts=$? ; if [[ ${sts} != 0 || ${doCmd_cmdSts} != 0 ]] ; then
            errMsg "Failed to reset HEAD!  sts=${sts}  doCmd_cmdSts=${doCmd_cmdSts}"
        fi

        printf "Reverting any modified or deleted files...\n"
        doCmdCapture "git checkout -f -- ${destDir}"
        sts=$? ; if [[ ${sts} != 0 || ${doCmd_cmdSts} != 0 ]] ; then
            errMsg "Failed to revert modified files!  sts=${sts}  doCmd_cmdSts=${doCmd_cmdSts}"
        fi

        printf "Deleting any added files...\n"
        doCmdCapture "git clean -dxf -- ${destDir}"
        sts=$? ; if [[ ${sts} != 0 || ${doCmd_cmdSts} != 0 ]] ; then
            errMsg "Failed to clean changed files!  sts=${sts}  doCmd_cmdSts=${doCmd_cmdSts}"
        fi

        printf "Changing branch back to: \"${savedBranch}\"...\n"
        doCmdCapture "git checkout ${savedBranch}"
        sts=$? ; if [[ ${sts} != 0 || ${doCmd_cmdSts} != 0 ]] ; then
            errMsg "Failed to checkout branch!  sts=${sts}  doCmd_cmdSts=${doCmd_cmdSts}"
        fi
    fi

    if [[ "${exitSts}" == "${eStsSuccess}" || "${exitSts}" == "${eStsUserCancel}" ]] ; then
#--------Begin here document-----------#
sendmail $g_mailingList  <<END
Subject: [WLAN Importer]: Normal Shutdown

WLAN Importer: Shutting down normally at $(date)
END
#--------End here document-------------#
        echo "Done."
    else
        # Send an email to let people that a tarball has been submitted for integration."
#--------Begin here document-----------#
sendmail $g_mailingList  <<END
Subject: [WLAN Importer]: Aborting!!! (exitSts:${exitSts})

WLAN Importer: Aborting with exitSts:${exitSts} at $(date)
END
#--------End here document-------------#
        vbPrint "exitSts:  ${exitSts}"
    fi

    exit ${exitSts}
}


function importTarballToGit () {
    local impTarballPath="$1"
    local impInfoPath="$2"
    local impTag="$3"
    local impBrand="$4"
    local impDate="$5"
    local impWorkitem="$6"
    local impBranch="$7"
    local impSshServer=
    local impSshPath=
    local impInfoSshServer=
    local impInfoSshPath=
    local infoLines=
    local infoText=
    local dirtyFiles=
    local cmd=
    local i=
    local retSts="${eStsSuccess}"
    local sts=0

    vbPrint "impTarballPath: ${impTarballPath}"
    vbPrint "impTag:         ${impTag}"
    vbPrint "impBrand:       ${impBrand}"
    vbPrint "impDate:        ${impDate}"

    if [[ "${retSts}" == "${eStsSuccess}" ]] ; then
        printf "Checking for release tarball...\n"

        if [[ "${impTarballPath}" =~ ${g_sshRegex} ]] ; then

            impSshServer=${BASH_REMATCH[1]}
            impSshPath=${BASH_REMATCH[2]}

#           vbPrint "impSshServer:\"${impSshServer}\"  impSshPath:\"${impSshPath}\""

            cmd='ssh  '${impSshServer}' ls '${impSshPath}

            doCmdCapture "${cmd}"
            sts=$? ; if [[ ${sts} != 0 || ${doCmd_cmdSts} != 0 ]] ; then
                errMsg "Command failed: \"${cmd}\""
                errMsg "sts:${sts} doCmd_cmdSts:${doCmd_cmdSts}"
                retSts=${eStsUnknownError}
            fi

        else
            errMsg "Unsupported tarball path: ${impTarballPath}"
            retSts=${eStsTarballPathBad}
            eval "${resultVar}="
        fi

        if [[ "${retSts}" != "${eStsSuccess}" ]] ; then
            errMsg "Can't find release tarball: \"${impTarballPath}\"!"
        else
            printf "Tarball found.\n"
        fi
    fi

    if [[ "${retSts}" == "${eStsSuccess}" ]] ; then
        printf "Checking for info file...\n"

        if [[ "${impInfoPath}" =~ ${g_sshRegex} ]] ; then

            impInfoSshServer=${BASH_REMATCH[1]}
            impInfoSshPath=${BASH_REMATCH[2]}

#           vbPrint "impInfoSshServer:\"${impInfoSshServer}\"  impInfoSshPath:\"${impInfoSshPath}\""

            cmd='ssh  '${impInfoSshServer}'  cat '${impInfoSshPath}
            vbPrint "Cmd: ${cmd}"

            doCmdCapture "${cmd}"
            sts=$? ; if [[ ${sts} != 0 || ${doCmd_cmdSts} != 0 ]] ; then
                errMsg "Command failed: \"${cmd}\""
                errMsg "sts:${sts} doCmd_cmdSts:${doCmd_cmdSts}"
                retSts=${eStsUnknownError}
            fi

            if [ "${doCmd_captureArray}" == "" ] ; then
                retSts=${eStsInfileArg}
            fi

            if [[ "${retSts}" == "${eStsSuccess}" ]] ; then

                for (( i=0; i<${#doCmd_captureArray[*]}; i++ )) ; do
                    vbPrint "doCmd_captureArray[$i] of ${#doCmd_captureArray[*]} :  ${doCmd_captureArray[$i]}"
                    infoText+=${doCmd_captureArray[$i]}$'\n'

                done

                vbPrint "infoText: \n${infoText}"
            fi

        else
            errMsg "Unsupported tarball path: ${impInfoPath}"
            retSts=${eStsTarballPathBad}
        fi

        vbPrint "retSts: ${retSts}"

        if [[ "${retSts}" != "${eStsSuccess}" ]] ; then
            errMsg "Can't find release info file: \"${impInfoPath}\"!"
        else
            printf "Info file found.\n"
        fi
    fi


    if [[ "${retSts}" == "${eStsSuccess}" ]] ; then

        printf "%-60s" "Checking for Git repo..."

        destDir=$(git rev-parse --show-toplevel)
        sts=$? ; if [[ ${sts} != 0 ]] ; then
            errMsg "Your current directory doesn't seem to be a Git repo!"
            retSts=${eStsGit}
        else
            printf "Okay.\n"
        fi
    fi


    if [[ "${retSts}" == "${eStsSuccess}" ]] ; then

        printf "%-60s" "Checking for wlan directory..."

        # destDir now contains the absolute path to the top of the current Git repo,
        # e.g., "/local/users/myname/myRepo/refsw"
        # Add the relative path to the WLAN source code, we should then have something like 
        # "/local/users/myname/myRepo/refsw/BSEAV/connectivity/wlan"
        destDir=${destDir}/${e_wlanTopDir}

        if [[ ! -d ${destDir} ]] ; then
            errMsg "Can't find WLAN directory: \"${destDir}\"!"
            errMsg "Are you sure you're in the right repo?"
            retSts=${eStsNoWlanDir}
        else
            printf "Okay.\n"
        fi
    fi


    if [[ "${retSts}" == "${eStsSuccess}" ]] ; then

        printf "%-60s" "Checking for clean Git state (no modified files)..."
        dirtyFiles=$(git status --porcelain -- ${destDir}  | wc -l)
        sts=$? ; if [[ ${sts} != 0 ]] ; then
            errMsg "Git problem.  Giving up."
            retSts=${eStsGit}
        fi
    fi


    if [[ "${retSts}" == "${eStsSuccess}" ]] ; then

        if [[ ${dirtyFiles} -gt 0 ]] ; then
            errMsg "You have ${dirtyFiles} modified or untracked files"
            doCmdCapture "git status -- ${destDir}"
            retSts=${eStsGit}
        else
            printf "Okay.\n"
        fi
    fi


    if [[ "${retSts}" == "${eStsSuccess}" ]] ; then

        if [[ "${savedBranch}" == "" ]] ; then

            printf "%-60s" "Getting the current branch name..."

            savedBranch=$(git rev-parse --abbrev-ref HEAD)
            sts=$? ; if [[ ${sts} != 0 ]] ; then
                errMsg "Git problem.  Giving up."
                retSts=${eStsGit}
            else
                printf "${savedBranch}.\n"
            fi
        fi
    fi


    if [[ "${retSts}" == "${eStsSuccess}" ]] ; then
        workingBranch="$(basename $0)_temp_working_branch"

        printf "%-60s" "Cleaning up any leftover working branch..."

        $(git branch -D ${workingBranch} &>/dev/null)
        # Don't bother checking status since branch might not be there
        printf "Okay.\n"
    fi


    if [[ "${retSts}" == "${eStsSuccess}" ]] ; then

        printf "Creating and checking out a new working branch...\n"

        doCmdCapture "git refsw ticket new ${workingBranch}"
        sts=$? ; if [[ ${sts} != 0 || ${doCmd_cmdSts} != 0 ]] ; then
            errMsg "Command failed: \"${cmd}\""
            errMsg "sts:${sts} doCmd_cmdSts:${doCmd_cmdSts}"
            retSts=${eStsUnknownError}
        fi
    fi


    if [[ "${retSts}" == "${eStsSuccess}" ]] ; then

        # Now update destDir to point to the place where the tarball will be extracted
        # (e.g., ""/local/users/myname/myRepo/refsw/BSEAV/connectivity/wlan/EAGLE_10_10_84/linux-external-media/"
        destDir=${destDir}/${impBranch}/${impBrand}

        printf "\n"
        printf "Ready to untar this file:\n"
        printf "    ${impTarballPath}\n"
        printf "To this directory:\n"
        printf "    ${destDir}\n"

        doConfirm "Are you sure?"
        retSts=$? # either eStsSuccess or eStsUserCancel
    fi

    if [[ "${retSts}" == "${eStsSuccess}" ]] ; then
        
        # Ensure that directory exists
        cmd="mkdir -p ${destDir}"
        doCmdCapture  "${cmd}"
        sts=$? ; if [[ ${sts} != 0 || ${doCmd_cmdSts} != 0 ]] ; then
            errMsg "Command failed: \"${cmd}\""
            errMsg "sts:${sts} doCmd_cmdSts:${doCmd_cmdSts}"
            retSts=${eStsUnknownError}
        fi
    fi


    if [[ "${retSts}" == "${eStsSuccess}" ]] ; then

        # Remove all files... we'll replace them from the tarball
        cmd="rm -rf ${destDir}/*"
        doCmdCapture  "${cmd}"
        sts=$? ; if [[ ${sts} != 0 || ${doCmd_cmdSts} != 0 ]] ; then
            errMsg "Command failed: \"${cmd}\""
            errMsg "sts:${sts} doCmd_cmdSts:${doCmd_cmdSts}"
            retSts=${eStsUnknownError}
        fi
    fi


    if [[ "${retSts}" == "${eStsSuccess}" ]] ; then

        printf "Untarring the tarball...\n"

        if [[ "${impSshServer}" != "" ]] ; then

            cmd='ssh '${impSshServer}' "cat  '${impSshPath}'" | tar -C ${destDir} --recursive-unlink -xzf -'
            doCmdCapture  "${cmd}"
            sts=$? ; if [[ ${sts} != 0 || ${doCmd_cmdSts} != 0 ]] ; then
                errMsg "Command failed: \"${cmd}\""
                errMsg "sts:${sts} doCmd_cmdSts:${doCmd_cmdSts}"
                retSts=${eStsUnknownError}
            fi
        else
            cmd="tar -C ${destDir} --recursive-unlink  -xzf ${impTarballPath}"
            doCmdCapture  "${cmd}"
            sts=$? ; if [[ ${sts} != 0 || ${doCmd_cmdSts} != 0 ]] ; then
                errMsg "Command failed: \"${cmd}\""
                errMsg "sts:${sts} doCmd_cmdSts:${doCmd_cmdSts}"
                retSts=${eStsUnknownError}
            fi
        fi

        if [[ "${retSts}" != "${eStsSuccess}" ]] ; then
            errMsg "Oh oh... tar had a problem.  Giving up."
        else
            printf "Okay.\n"
        fi
    fi


    if [[ "${retSts}" == "${eStsSuccess}" ]] ; then

        printf "Adding any new files to Git...\n"

        doCmdCapture  "git add --all -- ${destDir}"
        sts=$? ; if [[ ${sts} != 0 || ${doCmd_cmdSts} != 0 ]] ; then
            errMsg "Command failed: \"${cmd}\""
            errMsg "sts:${sts} doCmd_cmdSts:${doCmd_cmdSts}"
            retSts=${eStsUnknownError}
        else
            printf "Okay.\n"
        fi
    fi


    if [[ "${retSts}" == "${eStsSuccess}" ]] ; then

        printf "Checking for adds, modifies, deletes...\n"

        dirtyFiles=$(git status --porcelain -- ${destDir}  | wc -l)
        sts=$? ; if [[ ${sts} != 0 ]] ; then
            errMsg "Git problem.  Giving up."
            retSts=${eStsGit}
        fi
    fi


    if [[ "${retSts}" == "${eStsSuccess}" ]] ; then

        if [[ ${dirtyFiles} -eq 0 ]] ; then
            errMsg "No files have been added, modified, or deleted.  Nothing to do!"
            retSts=${eStsNoChanges}
        else
            printf "Here is a list of any changed files:\n"

            doCmdCapture "git commit --dry-run  --untracked-files=no -- ${destDir}"
            sts=$? ; if [[ ${sts} != 0 || ${doCmd_cmdSts} != 0 ]] ; then
                errMsg "Command failed: \"${cmd}\""
                errMsg "sts:${sts} doCmd_cmdSts:${doCmd_cmdSts}"
                retSts=${eStsUnknownError}
            else

                echo "  +------------------------------------------------------------------------------"
                for (( i=0; i<${#doCmd_captureArray[*]}; i++ )) ; do
                    echo "  | ${doCmd_captureArray[$i]}"
                done
                echo "  +------------------------------------------------------------------------------"

            fi
        fi
    fi


    if [[ "${retSts}" == "${eStsSuccess}" ]] ; then

        printf "Please review the changes listed above...\n"

        doConfirm "Would you like to commit these changes?"
        retSts=$? # either eStsSuccess or eStsUserCancel
    fi


    if [[ "${retSts}" == "${eStsSuccess}" ]] ; then

        printf "Committing the changes to Git...\n"

        cmd='git commit -m "'${e_jiraIssueForCommit}'WLAN Driver: tag='${impTag}' brand='${impBrand}' date='${impDate}'" -m "" -m "Workitem: '${impWorkitem}'" -m "Contents of '$(basename "${impInfoSshPath}")':" -m "'${infoText}'"'

        doCmdCapture "${cmd}"
        sts=$? ; if [[ ${sts} != 0 || ${doCmd_cmdSts} != 0 ]] ; then
            errMsg "Command failed: \"${cmd}\""
            errMsg "sts:${sts} doCmd_cmdSts:${doCmd_cmdSts}"
            retSts=${eStsUnknownError}
        else
            printf "Okay.\n"
        fi
    fi


    if [[ "${retSts}" == "${eStsSuccess}" ]] ; then

        doConfirm "Would you like to integrate the commit to \"baseline/git_refsw\"?"
        retSts=$? # either eStsSuccess or eStsUserCancel
    fi


    if [[ "${retSts}" == "${eStsSuccess}" ]] ; then

        printf "Doing integrate...\n"

        cmd="git refsw integrate"

        doCmdCapture "${cmd}"
        sts=$? ; if [[ ${sts} != 0 || ${doCmd_cmdSts} != 0 ]] ; then
            errMsg "Command failed: \"${cmd}\""
            errMsg "sts:${sts} doCmd_cmdSts:${doCmd_cmdSts}"
            retSts=${eStsUnknownError}
        fi
    fi

    vbPrint "Returning retSts: ${retSts}"
    return "${retSts}"
}


function getOldestWorkitem () {
    local resultVar="$1"
    local workitem=
    local cmd=
    local i=
    local retSts="${eStsSuccess}"
    local sts=0

    #  Look for the oldest Workitem without any prefix.
    vbPrint "Looking for available Workitem..."
    vbPrint "in:\"${g_workqPath}\""

    if [[ "${g_workqSshServer}" != "" ]] ; then

        if [[ "${retSts}" == "${eStsSuccess}" ]] ; then
#           vbPrint "g_workqSshServer:\"${g_workqSshServer}\"  g_workqSshPath:\"${g_workqSshPath}\""

            cmd='ssh  '${g_workqSshServer}'  "cd '${g_workqSshPath}' && find . -path \"./*/workitem_*/info.txt\" | sort -t "/" -k 3 | head -n1  "'

            doCmdCapture "${cmd}"
            sts=$? ; if [[ ${sts} != 0 || ${doCmd_cmdSts} != 0 ]] ; then
                errMsg "Command failed: \"${cmd}\""
                errMsg "sts:${sts} doCmd_cmdSts:${doCmd_cmdSts}"
                retSts=${eStsUnknownError}
            fi
        fi

        if [[ "${retSts}" == "${eStsSuccess}" ]] ; then

            for (( i=0; i<${#doCmd_captureArray[*]}; i++ )) ; do
                vbPrint "$i  /  ${#doCmd_captureArray[*]} :  ${doCmd_captureArray[$i]}"
            done

            # Get path to info file.
            workitem=${doCmd_captureArray[0]}
            if [ "${workitem}" != "" ] ; then

                #  Remove leading "./"
                workitem=${workitem#./}

                # Get workitem directory name.
                workitem=$(dirname ${workitem})
                vbPrint "Found available Workitem: ${workitem}"
            else
                vbPrint "No available Workitem."
            fi
        fi

        eval "${resultVar}=${workitem}"

    else
        errMsg "Unsupported workqueue path: ${g_workqPath}"
        retSts=${eStsWorkqPathBad}
        eval "${resultVar}="
    fi

    vbPrint "Returning retSts: ${retSts}"
    return "${retSts}"
}


function getPendingWorkitem () {
    local resultVar="$1"
    local workitem=
    local cmd=
    local i=
    local retSts="${eStsSuccess}"
    local sts=0

    #  Look for the oldest Workitem with the "pending_" prefix.
    infoPrint "Looking for pending Workitem..."
    infoPrint "in:\"${g_workqPath}\""

    if [[ "${g_workqSshServer}" != "" ]] ; then

        if [[ "${retSts}" == "${eStsSuccess}" ]] ; then
#           vbPrint "g_workqSshServer:\"${g_workqSshServer}\"  g_workqSshPath:\"${g_workqSshPath}\""

            cmd='ssh  '${g_workqSshServer}'  "cd '${g_workqSshPath}' && find . -path \"./*/pending_workitem_*/info.txt\" | sort -t "/" -k 3 | head -n1  "'

            doCmdCapture "${cmd}"
            sts=$? ; if [[ ${sts} != 0 || ${doCmd_cmdSts} != 0 ]] ; then
                errMsg "Command failed: \"${cmd}\""
                errMsg "sts:${sts} doCmd_cmdSts:${doCmd_cmdSts}"
                retSts=${eStsUnknownError}
            fi
        fi

        if [[ "${retSts}" == "${eStsSuccess}" ]] ; then
            for (( i=0; i<${#doCmd_captureArray[*]}; i++ )) ; do
                vbPrint "$i  /  ${#doCmd_captureArray[*]} :  ${doCmd_captureArray[$i]}"
            done

            # Get path to info file.
            workitem="${doCmd_captureArray[0]}"
            if [ "${workitem}" != "" ] ; then

                #  Remove leading "./"
                workitem=${workitem#./}

                # Get workitem directory name.
                workitem=$(dirname ${workitem})

                vbPrint "Found pending Workitem: ${workitem}"

            else
                vbPrint "No pending Workitem."
            fi
        fi

        eval "${resultVar}=${workitem}"

    else
        errMsg "Unsupported workqueue path: ${g_workqPath}"
        retSts=${eStsWorkqPathBad}
        eval "${resultVar}="
    fi

    vbPrint "Returning retSts: ${retSts}"
    return "${retSts}"
}


function getTarballFilename () {

    local workitemPath="$1"
    local resultVar="$2"
    local tarballFilename=
    local cmd=
    local i=
    local retSts="${eStsSuccess}"
    local sts=0

    vbPrint "Entering"
    vbPrint "workitemPath: ${workitemPath}"
    vbPrint "resultVar: ${resultVar}"


    if [[ "${g_workqSshServer}" != "" ]] ; then

#       vbPrint "Handling SSH path, dirPath:\"${g_workqPath}\""
        if [[ "${retSts}" == "${eStsSuccess}" ]] ; then
            vbPrint "g_workqSshServer:\"${g_workqSshServer}\"  g_workqSshPath:\"${g_workqSshPath}\""

            cmd='ssh  '${g_workqSshServer}'  "cd '${g_workqSshPath}' && find '${workitemPath}' -name \"*.tar.gz\" "'

            doCmdCapture "${cmd}"
            sts=$? ; if [[ ${sts} != 0 || ${doCmd_cmdSts} != 0 ]] ; then
                errMsg "Command failed: \"${cmd}\""
                errMsg "sts:${sts} doCmd_cmdSts:${doCmd_cmdSts}"
                retSts=${eStsUnknownError}
            fi
        fi

        if [[ "${retSts}" == "${eStsSuccess}" ]] ; then

            for (( i=0; i<${#doCmd_captureArray[*]}; i++ )) ; do
                vbPrint "$i  /  ${#doCmd_captureArray[*]} :  ${doCmd_captureArray[$i]}"
            done
        fi

        # Get path to info file.
        tarballFilename=${doCmd_captureArray[0]}
        vbPrint "tarballFilename: ${tarballFilename}"

        tarballFilename="${g_workqPath}/${tarballFilename}"
        vbPrint "tarballFilename: ${tarballFilename}"

        eval "${resultVar}=(${tarballFilename})"

    else

        errMsg "Unsupported workqueue path: ${g_workqPath}"
        retSts=${eStsWorkqPathBad}
        eval "${resultVar}="
    fi


    vbPrint "Returning retSts: ${retSts}"
    return "${retSts}"
}


function getInfoFilename () {

    local workitemPath="$1"
    local resultVar="$2"
    local infoFilename=
    local cmd=
    local i=
    local retSts="${eStsSuccess}"
    local sts=0

    vbPrint "Entering"
    vbPrint "workitemPath: ${workitemPath}"
    vbPrint "resultVar: ${resultVar}"

    if [[ "${g_workqSshServer}" != "" ]] ; then

#       vbPrint "Handling SSH path, dirPath:\"${g_workqPath}\""

        if [[ "${retSts}" == "${eStsSuccess}" ]] ; then

            vbPrint "g_workqSshServer:\"${g_workqSshServer}\"  g_workqSshPath:\"${g_workqSshPath}\""

            cmd='ssh  '${g_workqSshServer}'  "cd '${g_workqSshPath}' && find '${workitemPath}' -name \"info.txt\" "'

            doCmdCapture "${cmd}"
            sts=$? ; if [[ ${sts} != 0 || ${doCmd_cmdSts} != 0 ]] ; then
                errMsg "Command failed: \"${cmd}\""
                errMsg "sts:${sts} doCmd_cmdSts:${doCmd_cmdSts}"
                retSts=${eStsUnknownError}
            fi
        fi

        if [[ "${retSts}" == "${eStsSuccess}" ]] ; then

            for (( i=0; i<${#doCmd_captureArray[*]}; i++ )) ; do
                vbPrint "$i  /  ${#doCmd_captureArray[*]} :  ${doCmd_captureArray[$i]}"
            done
        fi

        if [[ "${retSts}" == "${eStsSuccess}" ]] ; then
            # Get path to info file.
            infoFilename=${doCmd_captureArray[0]}
            vbPrint "infoFilename: ${infoFilename}"

            infoFilename="${g_workqPath}/${infoFilename}"
            vbPrint "infoFilename: ${infoFilename}"
        fi

        eval "${resultVar}=(${infoFilename})"

    else

        errMsg "Unsupported workqueue path: ${g_workqPath}"
        retSts=${eStsWorkqPathBad}
        eval "${resultVar}="
    fi

    vbPrint "Returning retSts: ${retSts}"
    return "${retSts}"
}


function getValueFromInfoFile () {

    local workitemPath="$1"
    local keyName="$2"
    local resultVar="$3"
    local value=
    local cmd=
    local i=
    local retSts="${eStsSuccess}"
    local sts=0

    if [[ "${g_workqSshServer}" != "" ]] ; then

        if [[ "${retSts}" == "${eStsSuccess}" ]] ; then
#           vbPrint "g_workqSshServer:\"${g_workqSshServer}\"  g_workqSshPath:\"${g_workqSshPath}\""

#           cmd='ssh  '${g_workqSshServer}'  "cd '${g_workqSshPath}/${workitemPath}' && grep "'"${keyName}"'=" info.txt "'
            cmd='ssh  '${g_workqSshServer}'  "cd '${g_workqSshPath}/${workitemPath}' && grep \"'${keyName}'=\" info.txt "'

            doCmdCapture "${cmd}"
            sts=$? ; if [[ ${sts} != 0 || ${doCmd_cmdSts} != 0 ]] ; then
                errMsg "Command failed: \"${cmd}\""
                errMsg "sts:${sts} doCmd_cmdSts:${doCmd_cmdSts}"
                retSts=${eStsUnknownError}
            fi
        fi

        if [[ "${retSts}" == "${eStsSuccess}" ]] ; then
            for (( i=0; i<${#doCmd_captureArray[*]}; i++ )) ; do
                vbPrint "$i  /  ${#doCmd_captureArray[*]} :  ${doCmd_captureArray[$i]}"
            done

            # Get the value.
            value="${doCmd_captureArray[0]}"

            if [[ "${value}" =~ ${keyName}=(.*) ]] ; then
                value=${BASH_REMATCH[1]}
            else
                errMsg "Can't find key:\"${keyName}\" for Workitem: ${workitemPath}"
                retSts="${eStsUnknownError}"
            fi
        fi

        vbPrint "value: ${value}"
        eval "${resultVar}=(${value})"

    else
        errMsg "Unsupported workqueue path: ${g_workqPath}"
        retSts=${eStsWorkqPathBad}
        eval "${resultVar}="
    fi

    vbPrint "Returning retSts: ${retSts}"
    return "${retSts}"
}


function autoImportOneTarball () {
    local workitem="$1"
    local impTarballPath=
    local impInfoPath=
    local impTag=
    local impBrand=
    local impDate=
    local impBranch=
    local retSts="${eStsSuccess}"
    local sts=0

    vbPrint "Entering..."

    vbPrint "workitem: ${workitem}"

    getTarballFilename "${workitem}" "impTarballPath"
    sts=$? ; if [[ ${sts} != 0 ]] ; then
        retSts=${eStsTar}
    fi
    vbPrint "impTarballPath: ${impTarballPath}"

    getInfoFilename "${workitem}" "impInfoPath"
    sts=$? ; if [[ ${sts} != 0 ]] ; then
        retSts=${eStsTar}
    fi
    vbPrint "impInfoPath: ${impInfoPath}"

    getValueFromInfoFile "${workitem}" "tag"    "impTag"
    getValueFromInfoFile "${workitem}" "brand"  "impBrand"
    getValueFromInfoFile "${workitem}" "date"   "impDate"
    getValueFromInfoFile "${workitem}" "branch" "impBranch"

	# Handle an exception case here:
	#   Normally, a tarball is put into a directory that matches the name
	#   of the SVN branch that it came from, but the first incarnation of
	#   this import process didn't follow that convention.  So if the SVN 
	#   branch is "EAGLE_TWIG_10_10_84", put it into a directory named
	#   "EAGLE_10_10_84".
    if [[ "${impBranch}" == "EAGLE_TWIG_10_10_84" ]] ; then
        impBranch="EAGLE_10_10_84"
        infoPrint "Correcting info file \"branch=\" from EAGLE_TWIG_10_10_84 to EAGLE_10_10_84"
    fi

    if [[ "${impBranch}" == "" ]] ; then
        impBranch="${e_defInfoFileBranch}"
        infoPrint "Tag \"branch=\" not found in info file. Defaulting to ${impBranch}"
    fi
    
    infoPrint "Tag:    ${impTag}"
    infoPrint "Brand:  ${impBrand}"
    infoPrint "Date:   ${impDate}"
    infoPrint "Branch: ${impBranch}"

    importTarballToGit "${impTarballPath}" "${impInfoPath}" "${impTag}" "${impBrand}" "${impDate}" "${workitem}" "${impBranch}"
    sts=$? ; if [[ ${sts} != 0 ]] ; then
        retSts=${sts}
    fi

#--------Begin here document-----------#
sendmail $g_mailingList  <<END
Subject: [WLAN Importer]: Starting Integration: ${impTag}  ${impBrand}

WLAN Importer: The following workitem has been submitted for Git integration:

Workitem:        ${workitem}
Workitem Date:   ${impDate}
Workitem Branch: ${impBranch}
Tag:             ${impTag}
Brand:           ${impBrand}
END
#--------End here document-------------#



    vbPrint "Returning retSts: ${retSts}"
    return "${retSts}"
}


function setWorkitemPending () {
    local workitem="$1"
    local workitemBasename=
    local workitemDirname=
    local cmd=
    local i=
    local retSts="${eStsSuccess}"
    local sts=0

    workitemDirname="$(dirname ${workitem})"
    workitemBasename="$(basename ${workitem})"

    vbPrint "Changing workitem to pending: ${workitem}"
#   vbPrint "workitemDirname: ${workitemDirname}"
#   vbPrint "workitemBasename: ${workitemBasename}"

    if [[ "${g_workqSshServer}" != "" ]] ; then

        if [[ "${retSts}" == "${eStsSuccess}" ]] ; then

#           vbPrint "g_workqSshServer:\"${g_workqSshServer}\"  g_workqSshPath:\"${g_workqSshPath}\""

            doConfirm "Set workitem to pending: \"${workitem}\""
            retSts=$? # either eStsSuccess or eStsUserCancel
        fi

        if [[ "${retSts}" == "${eStsSuccess}" ]] ; then
            #  Rename the workitem (directory) with a "pending_" prefix.
            cmd='ssh  '${g_workqSshServer}'  "cd '${g_workqSshPath}/${workitemDirname}' && mv '${workitemBasename}' pending_'${workitemBasename}'"'

            doCmdCapture "${cmd}"
            sts=$? ; if [[ ${sts} != 0 || ${doCmd_cmdSts} != 0 ]] ; then
                errMsg "Command failed: \"${cmd}\""
                errMsg "sts:${sts} doCmd_cmdSts:${doCmd_cmdSts}"
                retSts=${eStsUnknownError}
            fi
        fi

        if [[ "${retSts}" == "${eStsSuccess}" ]] ; then
            for (( i=0; i<${#doCmd_captureArray[*]}; i++ )) ; do
                vbPrint "$i  /  ${#doCmd_captureArray[*]} :  ${doCmd_captureArray[$i]}"
            done
        fi

    else
        errMsg "Unsupported workqueue path: ${g_workqPath}"
        retSts=${eStsWorkqPathBad}
        eval "${resultVar}="
    fi

    vbPrint "Returning retSts: ${retSts}"
    return "${retSts}"
}


function deleteWorkitemIfIntegrated () {
    local pendingWorkitem="$1"
    local pendingWorkitemBasename=
    local workitemBasename=
    local workitemDirname=
    local workitem=
    local impTag=
    local impBrand=
    local impDate=
    local cmd=
    local i=
    local retSts="${eStsSuccess}"
    local sts=0

    workitemDirname="$(dirname ${pendingWorkitem})"
    pendingWorkitemBasename="$(basename ${pendingWorkitem})"

    if [[ "${pendingWorkitemBasename}" =~ pending_(.+) ]] ; then
        workitemBasename="${BASH_REMATCH[1]}"
    else
        errMsg "Expected pending workitem, but have: \"${pendingWorkitemBasename}\""
        errMsg "Continuing anyway..."
        workitemBasename="${pendingWorkitemBasename}"
    fi

    workitem="${workitemDirname}/${workitemBasename}"

    vbPrint "pendingWorkitem:           ${pendingWorkitem}"
    vbPrint "pendingWorkitemBasename:   ${pendingWorkitemBasename}"
    vbPrint "workitemDirname:           ${workitemDirname}"
    vbPrint "workitemBasename:          ${workitemBasename}"
    vbPrint "workitem:                  ${workitem}"

    if [[ "${g_workqSshServer}" != "" ]] ; then
#       vbPrint "Handling SSH path, dirPath:\"${g_workqPath}\""

        if [[ "${retSts}" == "${eStsSuccess}" ]] ; then

#           vbPrint "g_workqSshServer:\"${g_workqSshServer}\"  g_workqSshPath:\"${g_workqSshPath}\""

            #  Extract the pendingWorkitem's tag, brand, and date from the info file.
            getValueFromInfoFile "${pendingWorkitem}" "tag"   "impTag"
            getValueFromInfoFile "${pendingWorkitem}" "brand" "impBrand"
            getValueFromInfoFile "${pendingWorkitem}" "date"  "impDate"

            vbPrint "impTag:   ${impTag}"
            vbPrint "impBrand: ${impBrand}"
            vbPrint "impDate:  ${impDate}"
            vbPrint "pendingWorkitem: ${pendingWorkitem}"

            #  Update our Git repo from the baseline repo.
            cmd='git remote update'

            doCmdCapture "${cmd}"
            sts=$? ; if [[ ${sts} != 0 || ${doCmd_cmdSts} != 0 ]] ; then
                errMsg "Command failed: \"${cmd}\""
                errMsg "sts:${sts} doCmd_cmdSts:${doCmd_cmdSts}"
                retSts=${eStsUnknownError}
            fi
        fi

        if [[ "${retSts}" == "${eStsSuccess}" ]] ; then
            #  Now look for a Git commit that matches the pendingWorkitem's tag, brand, and date.
            cmd='git log --grep "'${e_jiraIssueForCommit}'WLAN Driver: tag='${impTag}' brand='${impBrand}' date='${impDate}'"  baseline/git_refsw'

            doCmdCapture "${cmd}"
            sts=$? ; if [[ ${sts} != 0 || ${doCmd_cmdSts} != 0 ]] ; then
                errMsg "Command failed: \"${cmd}\""
                errMsg "sts:${sts} doCmd_cmdSts:${doCmd_cmdSts}"
                retSts=${eStsUnknownError}
            fi
        fi

        if [[ "${retSts}" == "${eStsSuccess}" ]] ; then
            #  Examine the output from the git log command.
            retSts="${eStsNotIntegrated}"
            for (( i=0; i<${#doCmd_captureArray[*]}; i++ )) ; do
                vbPrint "$i  /  ${#doCmd_captureArray[*]} :  ${doCmd_captureArray[$i]}"
#####           if [[ "${doCmd_captureArray[$i]}" =~ ${e_jiraIssueForCommit}'WLAN Driver: tag='${impTag}' brand='${impBrand}' date='${impDate} ]] ; then
                if [[ "${doCmd_captureArray[$i]}" =~ Workitem:\ ${workitem} ]] ; then
                    vbPrint "************** Found it! ***********************"
                    retSts="${eStsSuccess}"
                    break
                fi
            done
        fi

        #  Rename the workitem (directory) with a "deleted_" prefix
        #  (but we need to remove the "pending_" prefix first).
        if [[ "${retSts}" == "${eStsSuccess}" ]] ; then
            local workitemNewName=

            infoPrint "Workitem integration is complete: ${pendingWorkitem}"

#--------Begin here document-----------#
sendmail $g_mailingList  <<END
Subject: [WLAN Importer]: Integration Complete: ${impTag}  ${impBrand}

WLAN Importer: The following workitem has been integrated to Git:

Workitem:        ${workitem}
Workitem Date:   ${impDate}
Tag:             ${impTag}
Brand:           ${impBrand}

$(git log -n1 --grep "${workitem}")
END
#--------End here document-------------#

            infoPrint "Setting Workitem to deleted: ${pendingWorkitem}"

            doConfirm "Delete pending Workitem: \"${pendingWorkitem}\""
            retSts=$? # either eStsSuccess or eStsUserCancel
        fi

        if [[ "${retSts}" == "${eStsSuccess}" ]] ; then
            cmd='ssh  '${g_workqSshServer}'  "cd '${g_workqSshPath}/${workitemDirname}' && pwd && mv '${pendingWorkitemBasename}' deleted_'${workitemBasename}'"'

            doCmdCapture "${cmd}" "cmdSts"
            sts=$? ; if [[ ${sts} != 0 || ${doCmd_cmdSts} != 0 ]] ; then
                errMsg "Command failed: \"${cmd}\""
                errMsg "sts:${sts} doCmd_cmdSts:${doCmd_cmdSts}"
                retSts=${eStsUnknownError}
            fi
        fi

        if [[ "${retSts}" == "${eStsSuccess}" ]] ; then
            for (( i=0; i<${#doCmd_captureArray[*]}; i++ )) ; do
                vbPrint "$i  /  ${#doCmd_captureArray[*]} :  ${doCmd_captureArray[$i]}"
            done
        fi

    else
        errMsg "Unsupported workqueue path: ${g_workqPath}"
        retSts=${eStsWorkqPathBad}
        eval "${resultVar}="
    fi

    vbPrint "Returning retSts: ${retSts}"
    return "${retSts}"
}


function autoImportTarballsToGit () {
    local retSts="${eStsSuccess}"
    local sts=0
    local oldestWorkitem=
    local pendingWorkitem=

    readonly  eStateIdle=0
    readonly  eStateChkPendingWorkitems=1
    readonly  eStateWaitPendingWorkitem=2
    readonly  eStateWaitForNewWorkitem=3
    readonly  eStateGetNewWorkitem=4

    local state=${eStateIdle}


    if [[ "${e_autoImportWorkqPath}" =~ ${g_sshRegex} ]] ; then
        g_workqPath="${e_autoImportWorkqPath}"
        g_workqSshServer=${BASH_REMATCH[1]}
        g_workqSshPath=${BASH_REMATCH[2]}
    else
        errMsg "Unsupported workqueue path: ${e_autoImportWorkqPath}"
        retSts=${eStsWorkqPathBad}
        return "${retSts}"
    fi

    # Send an email to let people that a tarball has been submitted for integration."
#--------Begin here document-----------#
sendmail $g_mailingList  <<END
Subject: [WLAN Importer]: Starting Up...

WLAN Importer: Automatic importing of SVN-to-Git tarballs is now in progress...
END
#--------End here document-------------#

    #  Start of Automated Import State Machine...

    #  Loop until error or requested to quit.
    while true ; do

        vbPrint "state: ${state}"

        #  Initial state.
        if [[ "${state}" == "${eStateIdle}" ]] ; then
            state=${eStateChkPendingWorkitems}
        fi

        #---------------------------------------------
        #  State: eStateChkPendingWorkitems
        #
        #  Check for pending workitems that need to
        #  finish integrating.
        #---------------------------------------------
        if [[ "${state}" == "${eStateChkPendingWorkitems}" ]] ; then

            #  Try to get a pending workitem.
            getPendingWorkitem "pendingWorkitem"
            sts=$? ; if [[ ${sts} != 0 ]] ; then
                retSts=${sts}
                break;
            fi

            if [ "${pendingWorkitem}" == "" ] ; then
                #  No pending workitems, exit the loop and move on to doing the next import.
                infoPrint "No pending Workitems"
                state=${eStateWaitForNewWorkitem}
                infoPrint "Waiting for new Workitem..."
                infoPrint "Enter q to quit. "
            else
                #  Found a pending workitem.
                infoPrint "Found pending Workitem: ${pendingWorkitem}"
                infoPrint "Waiting for integration of workitem: \"${pendingWorkitem}\""
                infoPrint "Enter q to quit. "
                state=${eStateWaitPendingWorkitem}
            fi
        fi

        #---------------------------------------------
        #  State: eStateWaitPendingWorkitem
        #
        #  Stay in this state until all pending
        #  workitems have finished the Git integration
        #  (or user enters "q").
        #---------------------------------------------
        if [[ "${state}" == "${eStateWaitPendingWorkitem}" ]] ; then

            #  Delete the workitem if it's finished integration.
            deleteWorkitemIfIntegrated "${pendingWorkitem}"
            sts=$?
            if [[ ${sts} != ${eStsNotIntegrated} && ${sts} != ${eStsSuccess} ]] ; then
                errMsg "deleteWorkitemIfIntegrated failed, sts: ${sts}"
                retSts=${sts}
                break;
            fi

            #  If the workitem has been deleted, just stay in this loop to look for
            #  another pending one.  There should only be one pending workitem at a time
            #  but we'll handle multiple ones just in case.
            if [[ "${sts}" == "${eStsSuccess}" ]] ; then
                infoPrint "Pending Workitem is now integrated: ${pendingWorkitem}"
                state=${eStateChkPendingWorkitems}

            elif [[ "${sts}" == "${eStsNotIntegrated}" ]] ; then
                #  The integration is still in progress.  Wait for a while, then check again.
                vbPrint "Reading user input with ${g_integrationCheckTimeoutInSecs} second timeout"
                read -t "${g_integrationCheckTimeoutInSecs}" myInput
                sts=$?
#               infoPrint "read sts is \"${sts}\""
                if [[ ${sts} == 142 ]] ; then # Timeout
                    retSts=${eStsSuccess}
                elif [[ ${sts} == 1 ]] ; then # End of file
                    retSts=${eStsUserCancel}
                elif [[ ${sts} == 0 ]] ; then # Read was successful
                    if [[ ${myInput} =~ ^[Qq]$ ]] ; then
                        retSts=${eStsUserCancel}
                    else
                        retSts=${eStsSuccess}
                    fi
                else
                    errMsg "Read failed with sts: ${sts}"
                    retSts=${eStsInputError}
                fi
                vbPrint "Read completed or timed out, waking up... "

                #  Stay in this state until workitem is integrated or error.
                if [[ "${retSts}" != "${eStsSuccess}" ]] ; then break; fi

            else
                errMsg "Unexpected sts: ${sts}"
                retSts=${eStsUnknownError}
                break;
            fi
        fi

        #---------------------------------------------
        #  State: eStateWaitForNewWorkitem
        #
        #  Stay in this state until a new Workitem
        #  appears (or user enters "q").
        #---------------------------------------------
        if [[ "${state}" == "${eStateWaitForNewWorkitem}" ]] ; then

            #  Look for a workitem that waiting to be imported to Git.
            getOldestWorkitem "oldestWorkitem"
            sts=$? ; if [[ ${sts} != 0 ]] ; then
                retSts=${sts}
                break;
            fi

            if [[ "${oldestWorkitem}" == "" ]] ; then

                #  There are no workitems waiting to be imported.
                #  Wait for a while, then check again.
                vbPrint "Reading user input with ${g_workqCheckTimeoutInSecs} second timeout"
                read -t "${g_workqCheckTimeoutInSecs}" myInput
                sts=$?
#               infoPrint "read sts is \"${sts}\""
                if [[ ${sts} == 142 ]] ; then # Timeout
                    retSts=${eStsSuccess}
                elif [[ ${sts} == 1 ]] ; then # End of file
                    retSts=${eStsUserCancel}
                elif [[ ${sts} == 0 ]] ; then # Read was successful
                    if [[ ${myInput} =~ ^[Qq]$ ]] ; then
                        retSts=${eStsUserCancel}
                    fi
                else
                    errMsg "Read failed with sts: ${sts}"
                    retSts=${eStsInputError}
                fi
                vbPrint "Read completed or timed out, waking up... "

                #  Stay in this state until workitem is found or  error.
                if [[ "${retSts}" != "${eStsSuccess}" ]] ; then break; fi

            else
                infoPrint "Found new Workitem: ${oldestWorkitem}"
                state=${eStateGetNewWorkitem}
            fi
        fi

        #---------------------------------------------
        #  State: eStateGetNewWorkitem
        #
        #  A new Workitem is in the queue, import it
        #  into Git.
        #---------------------------------------------
        if [[ "${state}" == "${eStateGetNewWorkitem}" ]] ; then

            #  Try to import the workitem into Git.
            infoPrint "Found a Workitem to import: ${oldestWorkitem}"

            autoImportOneTarball "${oldestWorkitem}"
            sts=$? ; if [[ ${sts} != 0 ]] ; then
                retSts=${sts}
                break;
            fi

            #  If the import was successful, then change the workitem to pending.
            infoPrint "Workitem imported, changing to pending: ${oldestWorkitem} "

            setWorkitemPending "${oldestWorkitem}"
            sts=$? ; if [[ ${sts} != 0 ]] ; then
                retSts=${sts}
                break;
            fi

            state=${eStateChkPendingWorkitems}
        fi

    done    #  End of state processing loop

    vbPrint "Returning retSts: ${retSts}"
    return "${retSts}"
}


#############################################################################
function main() {

    local arg=
    local sts=

    #
    #  Any of the following settings can be passed from the environment.
    #  But if they have not been specified, then set a default value.
    : ${e_wlanTopDir:="BSEAV/connectivity/wlan"}
    : ${e_defInfoFileBranch:="EAGLE_10_10_84"}
    : ${e_jiraIssueForCommit:="SWSTB-1638: "}

    : ${g_workqCheckTimeoutInSecs:="300"}
    : ${g_integrationCheckTimeoutInSecs:="60"}

    : ${g_mailingList:="   \
                       gary.skerl@broadcom.com  \
                   prashant.katre@broadcom.com  \
                       david.kwan@broadcom.com  \
    "}

    echo "Using mailing list: ${g_mailingList}"

    # ssh://
    # The real one!
    test ${e_autoImportWorkqPath:="ssh://stb-sj1-01.sj.broadcom.com:/projects/hnd_video7/svn_git_bridge/workqueues"}

    # Gary's test sandbox:
    #  test ${e_autoImportWorkqPath:="ssh://stb-sj1-01.sj.broadcom.com:/projects/hnd_video7/gskerl/svn_git_bridge_sandbox/workqueues"}

    # David's test sandbox:
    #  test ${e_autoImportWorkqPath:="ssh://stb-sj1-01.sj.broadcom.com:/projects/hnd_video7/dkwan/projects/bcg/svn_git_bridge/workqueues"}

    # native path
    # test ${e_autoImportWorkqPath:="/build/gskerl/bseswsd___/svn_git_bridge/workq"}

    #  Define some enums to use for return/exit statuses:
    readonly  eStsSuccess=0
    readonly  eStsGit=10
    readonly  eStsInfileArg=11
    readonly  eStsNoChanges=12
    readonly  eStsNoWlanDir=13
    readonly  eStsTar=14
    readonly  eStsUserCancel=15
    readonly  eStsStateFileErr=16
    readonly  eStsTarballPathBad=17
    readonly  eStsWorkqPathBad=18
    readonly  eStsNotIntegrated=19
    readonly  eStsInputError=20
    readonly  eStsUnknownError=21


    #   typeset -a myResultLines
    #   local i=
    #   myCmdSts=
    #   doCmdCapture "ls -l " "myCmdSts" "myResultLines"
    #
    #   sts=$? ; infoPrint "sts: $sts"
    #   infoPrint "myCmdSts: $myCmdSts"
    #   infoPrint "doCmd_cmdSts: $doCmd_cmdSts"
    #
    #   infoPrint "myResultLines:"
    #   if [[ "${#myResultLines[*]}" -gt "0" ]] ; then
    #       for (( i=0; i<${#myResultLines[*]}; i++ )) ; do
    #           infoPrint "$i  /  ${#myResultLines[*]} :  ${myResultLines[$i]}"
    #       done
    #   fi
    #
    #   infoPrint "doCmd_captureArray:"
    #   if [[ "${#doCmd_captureArray[*]}" -gt "0" ]] ; then
    #       for (( i=0; i<${#doCmd_captureArray[*]}; i++ )) ; do
    #           infoPrint "$i  /  ${#doCmd_captureArray[*]} :  ${doCmd_captureArray[$i]}"
    #       done
    #   fi
    #
    #   exit

    helpArg=
    autoArg=
    unattendedArg=
    verboseArg=
    infileArg=
    authFileArg=

    savedBranch=
    destDir=
    exitSts=0

    g_workqPath=

    g_sshRegex="^[[:space:]]*ssh://([^:]*):(.*)$"
    g_workqSshServer=
    g_workqSshPath=

    #  Call "doCleanup" function when exiting for any reason.
    trap  "doCleanup" EXIT

    #  Disablel Ctrl/C so user doesn't interrupt any Git commands.
    trap  "" SIGINT

    #  Loop through each argument on the command line.
    while [[ $# -ne 0 ]] ; do

        arg=$1
        vbPrint "Parsing args... arg: $arg  args left: $#"
        shift

        case $arg  in
            (--help | -h)
                vbPrint "Found --help!"
                helpArg=1
                ;;

            (--file | -f)
                vbPrint "Found --file!"
                infileArg="$1"
                shift
                ;;

            (--auto | -a)
                vbPrint "Found --auto!"
                autoArg=1
                ;;

            (--authenticationFile | -A)
                vbPrint "Found --authenticationFile!"
                authFileArg="$1"
                shift
                ;;

            (--unattended | -u)
                vbPrint "Found --unattended!"
                unattendedArg=1
                ;;

            (--verbose | -v)
                vbPrint "Found --unattended!"
                verboseArg=1
                ;;

            "")
                vbPrint "Found NULL string!"
                ;;

            -*)
                echo "Error: Unrecognized option: \"$arg\""
                helpArg=1
                ;;

            *)
                echo "Error: Unrecognized argument: \"$arg\""
                helpArg=1
                ;;
        esac
    done

    if [[ "${exitSts}" == "0" ]] ; then

        if [[ "${infileArg}" != "" ]] ; then

            infoPrint "infileArg: $infileArg\n"

            importTarballToGit ${infileArg}
            sts=$? ; if [[ ${sts} != 0 ]] ; then
                exitSts=${sts}
            fi

         elif [[ "${autoArg}" != "" ]] ; then

            autoImportTarballsToGit
            sts=$? ; if [[ ${sts} != 0 ]] ; then
                exitSts=${sts}
            fi
        else
            helpArg=1
        fi
    fi

    if [[ "${helpArg}" != "" ]] ; then
        echo ""
        echo "  Usage:"
        echo ""
        echo "    $(basename $0)  <options>"
        echo ""
        echo "  Where:"
        echo "      <options> can be any of the following options:"
        echo ""
        echo "      -h, --help          Print this usage info"
        echo ""
        echo "      -f, --file   <path-to-release-tarball>"
        echo "                          specifies a WLAN release tarball to be imported"
        echo "                          into Git (e.g, \"eagle_rel_10_10_84_9.tgz\")"
        echo ""
        echo "                          A tarball can be accessed via ssh if specified like this:"
        echo "                          \"ssh://server:path/file\", for example:"
        echo "                          \"ssh://stb-sj1-01.sj.broadcom.com:/projects/hnd_video7/svn_git_bridge/workqueues"
        echo ""
        echo "      -a, --auto          Run in automated mode, looking for tarballs in queued workitems"
        echo "                          from ${e_autoImportWorkqPath}"
        echo ""
        echo "      -u, --unattended    Run in non-interactive mode.  Assumes \"yes\" answers to"
        echo "                          all confirmations prompts."
        echo ""
        echo "      -v, --verbose       Print lots of debugging information."
        echo ""
        return 0
    fi

    return 0
}


#############################################################################
#  Entry Point
#############################################################################


set -u    # Treat unset variables as an error.

main "$@"

#  Everything after this point will be performed by the "doCleanup" exit handler function.
