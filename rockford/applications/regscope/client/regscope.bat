@echo off
rem ###########################################################
rem  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
rem
rem  This program is the proprietary software of Broadcom and/or its licensors,
rem  and may only be used, duplicated, modified or distributed pursuant to the terms and
rem  conditions of a separate, written license agreement executed between you and Broadcom
rem  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
rem  no license (express or implied), right to use, or waiver of any kind with respect to the
rem  Software, and Broadcom expressly reserves all rights in and to the Software and all
rem  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
rem  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
rem  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
rem
rem  Except as expressly set forth in the Authorized License,
rem
rem  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
rem  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
rem  and to use this information only in connection with your use of Broadcom integrated circuit products.
rem
rem  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
rem  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
rem  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
rem  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
rem  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
rem  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
rem  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
rem  USE OR PERFORMANCE OF THE SOFTWARE.
rem
rem  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
rem  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
rem  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
rem  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
rem  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
rem  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
rem  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
rem  ANY LIMITED REMEDY.
rem
rem  Module Description:
rem
rem ###########################################################

rem set regscope.pl to pick up perl modules from release directory if have
rem not been set.

:use_release
rem set _SAVED_PERLLIB=%PERLLIB%
rem set PERLLIB=.;.\lib;%PERLLIB%

:doit
perl -w regscope.pl %*
goto end

:end
rem set PERLLIB=%_SAVED_PERLLIB%
rem set _SAVED_PERLLIB=

rem End of file
