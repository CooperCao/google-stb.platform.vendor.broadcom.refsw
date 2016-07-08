#############################################################
#  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
#
#  This program is the proprietary software of Broadcom and/or its licensors,
#  and may only be used, duplicated, modified or distributed pursuant to the terms and
#  conditions of a separate, written license agreement executed between you and Broadcom
#  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
#  no license (express or implied), right to use, or waiver of any kind with respect to the
#  Software, and Broadcom expressly reserves all rights in and to the Software and all
#  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
#  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
#  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
#  Except as expressly set forth in the Authorized License,
#
#  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
#  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
#  and to use this information only in connection with your use of Broadcom integrated circuit products.
#
#  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
#  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
#  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
#  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
#  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
#  USE OR PERFORMANCE OF THE SOFTWARE.
#
#  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
#  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
#  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
#  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
#  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
#  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
#  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
#  ANY LIMITED REMEDY.
############################################################

#-----------------------------------------------------------------------------------#
# HSM build-command-line-options:
#  CUST=ALL        to select customer mode
#  CUST=Not Set or Invalid -  Customer Mode defaults to GENERIC3A
#  SCVALUE=ON                to turn on scrambling ctrl bits setting with specified value
#  BHSM_BSECK=ON             to turn on/off BSECK feature 
#  BHSM_IPTV=ON              to turn on/off IPTV feature
#  BHSM_KEYLADDER=ON         to select spitted, protected secure PIs 
#  BHSM_MISC=ON              to select spitted, protected secure PIs 
#  BHSM_OTPMSP=ON            to select spitted, protected secure PIs 
#  BHSM_SECURE_RSA=ON        to turn on Secure RSA feature
#  BHSM_SECURE_ACCESS=ON     to turn on Secure Access (DRM Data Op) feature
#  BHSM_IP_LICENSING=ON      to turn on IP Licensing feature
#  BHSM_MISC=ON              to turn on support for DEBUG_DUMP_EXEC command
#  BHSM_PWR_MGMT=ON          to turn on support for power management
#
#-----------------------------------------------------------------------------------#


include $(MAGNUM)/portinginterface/hsm/bhsm_defs.inc

#
# Include the right bhsm_*.inc file
#

ifeq ($(filter $(ZEUSVER), core65nmNonASKM core65nmASKM ), $(ZEUSVER))
    include $(MAGNUM)/portinginterface/hsm/bhsm_legacy.inc
else
    include $(MAGNUM)/portinginterface/hsm/bhsm_zeus.inc
endif
