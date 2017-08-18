#!/bin/bash
#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
# $Id$

more <<EOF
LICENSE AGREEMENT

Definitions.  For purposes of this Agreement: (i) "BROADCOM Products"
means certain proprietary products of BROADCOM purchased by Licensee
from BROADCOM; (ii) "Documentation" means any and all written
technical documentation furnished by BROADCOM to Licensee during the
term of this Agreement that relates to the Software; (iii) "Licensee
Product" means any system level product sold by Licensee that
incorporates one or more BROADCOM Products and the Software and that
includes other hardware and software provided by Licensee; and (iv)
"Software" means the BROADCOM driver software, in both source code and
object code form, made available for download by BROADCOM or otherwise
provided to Licensee by BROADCOM.

License.  Subject to the terms and conditions of this Agreement,
BROADCOM grants to Licensee the non-exclusive, non-transferable,
revocable right to (i) use and modify the Software, without the right
to sublicense and (ii) reproduce and distribute, in object code form
only, copies of the Software (and any modifications thereof) to
resellers, distributors and end users of Licensee Products, only for
their use in connection with BROADCOM Products, pursuant to an end
user license agreement no less protective of BROADCOM than the terms
of this Agreement.

Restrictions.  This Software is protected by U.S. Copyright Law.  This
Software is licensed, not sold.  Licensee may not use, disclose,
modify, reproduce or distribute the Software except as expressly
permitted in this Agreement.  Without limiting the foregoing, Licensee
may not distribute, disclose or otherwise provide access to the
Software to any third parties in source code form without the prior
written consent of BROADCOM.  Pursuant to the license above, Licensee
may modify the Software so long as such modification does not cause
the Software (as modified) or the accompanying BROADCOM Products to be
incompatible or non-compliant with any specifications promulgated by
the HomePhoneline Networking Alliance ("HomePNA") including without
limitation the HomePNA 2.0 Specification.  Licensee acknowledges and
agrees that any breach of this Section as determined by BROADCOM or
the HomePNA constitutes a material breach of this Agreement and will
enable BROADCOM to terminate this Agreement as provided below.
Pursuant to the license grant above, any sublicense by Licensee to
third parties must restrict the use of the Software (and any
modifications thereof) by such third parties to use in combination
with the BROADCOM Products only.  In addition, Licensee must prohibit
its end users from attempting to translate, decompile, disassemble,
reverse engineer or otherwise attempt to derive the source code for
the Software in any way, except to the extent permitted by law.
Licensee shall use its best efforts to enforce the obligations of its
end user license agreements and shall notify BROADCOM immediately of
any known breach of such obligation.  Additionally, Licensee may not
remove, efface or otherwise obscure any proprietary notices, labels,
or marks on the Software or Documentation.  Licensee agrees that each
copy of the Software and Documentation will include reproductions of
all proprietary notices, labels or marks included therein.  BROADCOM
retains all right, title and interest in and to the Software.  ALL
RIGHTS NOT EXPRESSLY GRANTED HEREIN ARE RESERVED BY BROADCOM.
Licensee owns all right, title and interest in any improvements,
modifications or enhancements to the Software made by Licensee,
subject to Broadcom's ownership of the underlying Software and
restrictions on the use of the Software contained herein; provided,
however, that Licensee hereby grants to BROADCOM and its affiliates
and subsidiaries a perpetual, irrevocable, worldwide, royalty-free,
fully paid-up license, with the right to sublicense, to such
improvements, modifications or enhancements to make, have made, use,
sell, offer for sale, export, copy, create derivative works, publicly
perform and display, distribute and otherwise exploit products and
services which are based upon or incorporate such improvements,
modifications or enhancements.

Term and Termination.  This license is effective until terminated.
Licensee may terminate it by destroying the Software and Documentation
and all copies thereof.  This license will also terminate if Licensee
fails to comply with any term or condition of this Agreement.  Upon
termination, at BROADCOM'S OPTION, Licensee shall either return to
BROADCOM or destroy the Software and Documentation and all copies
thereof that are in Licensee's possession or control.

No Support.  Nothing in this Agreement shall obligate BROADCOM to
provide any support for the Software including without limitation any
obligation to correct any defects or provide any updates to the
Software to Licensee.

Limited Warranty and Disclaimer.  TO THE MAXIMUM EXTENT PERMITTED BY
LAW, THE SOFTWARE AND ALL DOCUMENTATION AND ANY (IF ANY) SUPPORT
SERVICES RELATED TO THE SOFTWARE OR DOCUMENTATION ARE PROVIDED "AS IS"
AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
RESPECT TO THE SOFTWARE, DOCUMENTATION OR SUPPORT SERVICES, INCLUDING
ITS CONDITION, ITS CONFORMITY TO ANY REPRESENTATION OR DESCRIPTION, OR
THE EXISTENCE OF ANY LATENT OR PATENT DEFECTS, AND BROADCOM
SPECIFICALLY DISCLAIMS ALL IMPLIED (IF ANY) WARRANTIES OF TITLE,
MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET
POSSESSION OR CORRESPONDENCE TO DESCRIPTION.  THE ENTIRE RISK ARISING
OUT OF USE OR PERFORMANCE OF THE SOFTWARE, DOCUMENTATION OR SUPPORT
SERVICES LIES WITH LICENSEE.

Exclusion of Incidental, Consequential and Certain Other Damages.  TO
THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
SUPPLIERS BE LIABLE FOR CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING
TO THIS AGREEMENT OR LICENSEE'S USE OF OR INABILITY TO USE THE
SOFTWARE, DOCUMENTATION OR SUPPORT SERVICES, OR THE PROVISION OR
FAILURE TO PROVIDE SUPPORT SERVICES, INCLUDING BUT NOT LIMITED TO LOST
PROFITS, LOSS OF CONFIDENTIAL OR OTHER INFORMATION, BUSINESS
INTERRUPTION, PERSONAL INJURY, LOSS OF PRIVACY, FAILURE TO MEET ANY
DUTY (INCLUDING OF GOOD FAITH OR REASONABLE CARE), NEGLIGENCE, COSTS
OF PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES, OR ANY OTHER CLAIM FOR
PECUNIARY OR OTHER LOSS WHATSOEVER, OR FOR ANY CLAIM OR DEMAND AGAINST
YOU BY ANY OTHER PARTY, EVEN IF BROADCOM HAS BEEN ADVISED OF THE
POSSIBILITY OF SUCH DAMAGES.  THESE LIMITATIONS SHALL APPLY
NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED
REMEDY.  THE PARTIES AGREE THAT THE FOREGOING LIMITATIONS REPRESENT A
REASONABLE ALLOCATION OF RISK.

Limitation of Liability and Remedies.  NOTWITHSTANDING ANY DAMAGES YOU
MAY INCUR FOR ANY REASON WHATSOEVER (INCLUDING, WITHOUT LIMITATION,
ALL DAMAGES REFERENCED ABOVE AND ALL DIRECT OR GENERAL DAMAGES), THE
ENTIRE LIABILITY OF BROADCOM AND ANY OF ITS SUPPLIERS UNDER ANY
PROVISION OF THIS AGREEMENT AND YOUR EXCLUSIVE REMEDY FOR ALL OF THE
FOREGOING SHALL BE LIMITED TO THE GREATER OF THE AMOUNT ACTUALLY PAID
FOR THE SOFTWARE, DOCUMENTATION OR SUPPORT SERVICES OR U.S. $100.  THE
FOREGOING LIMITATIONS, EXCLUSIONS AND DISCLAIMERS SHALL APPLY TO THE
MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, EVEN IF ANY REMEDY FAILS
ITS ESSENTIAL PURPOSE.

Confidentiality.  (a) Confidential Information.  The term
"Confidential Information" means: (i) any information which is
disclosed by BROADCOM to Licensee, either directly or indirectly, in
writing, orally or by inspection of tangible objects including without
limitation, documents, notes, designs, data, data sheets, test
results, specifications, prototypes or samples; and the (ii) Software
and Documentation.  Confidential Information does not include,
however, information which the Licensee can establish is (i) publicly
known or, through no wrongful act or failure to act by Licensee,
becomes publicly known; (ii) in the possession of Licensee, without
confidentiality restrictions, at the time of disclosure by BROADCOM as
shown by Licensee's files and records immediately prior to the
disclosure; (iii) hereafter furnished to Licensee by a third party who
is not under an obligation of confidentiality; or (iv) is
independently developed by employees of Licensee who have not been
exposed to the Confidential Information without reference to or
reliance on the Confidential Information.  (b) Limited Use.  Licensee
shall use the Confidential Information disclosed hereunder solely for
the purpose of fulfilling the Licensee's obligations and exercising
its rights under this Agreement.  (c) Non-Disclosure.  Except as
expressly permitted in this Agreement, Licensee shall not directly or
indirectly disclose, display, provide, transfer or otherwise make
available all or any part of the Confidential Information to any
person other than employees or subcontractors of Licensee necessary
for Licensee to perform its obligations or exercise its rights under
this Agreement.  Licensee represents and warrants that each employee
or subcontractor having access to the Confidential Information has
executed an agreement at least as protective of Confidential
Information as the terms and conditions of this Agreement.  Except as
permitted in this Agreement, Licensee shall not provide access to the
Confidential Information to any third parties, including consultants
and independent contractors.  (d) Duty to Hold in Confidence.
Licensee shall take all reasonable measures to protect the secrecy of
and avoid the disclosure and unauthorized use of the Confidential
Information.  In preserving the confidence of the Confidential
Information, Licensee shall use the same standard of care that it
would use to secure and safeguard its own confidential information,
but in no event less than reasonable care.  Licensee shall, at its own
expense: (i) immediately notify BROADCOM of any material unauthorized
possession, use or knowledge, or attempt thereof, of the Confidential
Information by any person or entity which may have become known to
such person arising from the disclosure to Licensee; (ii) promptly
furnish to BROADCOM full details of the unauthorized possession, use
or knowledge, or attempt thereof, and use reasonable efforts to assist
BROADCOM in investigating or preventing the recurrence of any
unauthorized possession, use of knowledge or attempt thereof, of the
Confidential Information; and (iii) promptly use all reasonable
efforts to prevent a recurrence of any unauthorized possession, use or
knowledge of the Confidential Information.  (e) Remedies.  Licensee
and BROADCOM agree that the unauthorized use by a party of
Confidential Information will diminish the value of such information.
Therefore, if a Licensee breaches any of its obligations with respect
to the confidentiality or use of the Confidential Information
hereunder, BROADCOM is entitled to seek equitable relief to protect
its interest herein, including injunctive relief, as well as money
damages.

Export Regulations.  Licensee understands that BROADCOM is subject to
regulation by agencies of the U.S., government, including, but not
limited to, the U.S. Department of Commerce, which prohibit export or
diversion of certain technical products to certain countries.  Any and
all obligations of BROADCOM to provide as well as any other technical
assistance, shall be subject in all respects to such United States
laws and regulations as shall from time to time govern the license and
delivery of technology and products abroad by persons subject to the
jurisdiction of the United States, including the Export Administration
Act of 1979, as amended, any successor legislation, and the Export
Administration Regulations issued by the Department of commerce,
Bureau of Export Administration.  Licensee warrants that it will
comply in all respects with the Export Administration Regulations and
all other export and re-export restrictions applicable to the software
and documentation licensed hereunder.

Non-Assignability.  Licensee may not sell, transfer, assign or
subcontract any right or obligation set forth in this Agreement
without the prior written consent of BROADCOM.  Any act in derogation
of the foregoing shall be null and void.

Government Customers.  If any of the rights or licenses granted
hereunder are acquired by or on behalf of a unit or agency of the
United States Government, this Section applies.  The Software is a
trade secret of BROADCOM for all purposes of the Freedom of
Information Act and is, in all respects, proprietary data belonging
solely to BROADCOM.  The Software: (i) was developed at private
expense, is existing computer software, and no part of it was
developed with government funds, (ii) is "restricted computer
software" submitted with restricted rights in accordance with
subparagraphs (a) through (d) of the Commercial Computer
Software-Restricted Rights clause at 48 CFR 52.227-19 and its
successors, (iii) is unpublished and all rights are reserved under the
copyright laws of the United States.  For units of the Department of
Defense (DoD), the Software is licensed only with "Restricted Rights"
as that term is defined in the DoD Supplement to the Federal
Acquisition Regulation ("DFARS"), 252.227-7013(c)(1)(ii), Rights in
Technical Data and Computer Software and its successors, and use,
duplication, or disclosure is subject to the restrictions set forth in
subdivision (c)(1)(ii) of the Rights in Technical Data and Computer
Software clause at DFARS 252.227-7013. The Contractor/manufacturer of
the Software is Broadcom Corporation, 16215 Alton Parkway, Irvine, CA
92718.  If the Software is acquired under a GSA Schedule, Licensee
agrees to refrain from: (a) changing or removing any insignia or
lettering from such software or the documentation that is provided;
(b) producing copies of related manuals or media (except for backup
purposes); and (c) allowing any third party to do that which is
prohibited in this Section.

Miscellaneous.  BROADCOM and Licensee are independent contractors.
This is the entire Agreement between the parties relating to the
subject matter hereof, supersedes any and all prior proposals,
agreements and representations between the parties, whether written or
oral, and no waiver, modification or amendment of the Agreement shall
be valid unless in writing signed by each party.  The waiver of a
breach of any term hereof shall in no way be construed as a waiver of
any other term or breach hereof.  If any provision of this Agreement
shall be held by a court of competent jurisdiction to be contrary to
law, the remaining provisions of this Agreement shall remain in full
force and effect.  This Agreement is governed by the laws of the State
of California without reference to conflict of laws principles. The
parties expressly stipulate that the 1980 United Nations Convention on
Contracts for the International Sale of Goods shall not apply. All
disputes arising out of this Agreement shall be subject to the
exclusive jurisdiction of the state and federal courts located in
Orange County, California, and the parties agree and submit to the
personal and exclusive jurisdiction and venue of these courts.

This is a legal agreement between you ("Licensee") and Broadcom
Corporation. ("BROADCOM").  BY OPENING THE SOFTWARE PACKAGE, TYPING
"ACCEPT" OR ACCESSING OR INSTALLING THE SOFTWARE, YOU ACKNOWLEDGE THAT
YOU HAVE READ THE LICENSE AGREEMENT, UNDERSTAND IT AND AGREE TO BE
BOUND BY ITS TERMS AND CONDITIONS.  If you do not agree to the terms
of this Agreement, type "DECLINE" and cease all further access to or
use of the Software made available by BROADCOM.

EOF

echo -n 'If you agree to these terms, type "ACCEPT": '
read accept
[ "${accept}" != "ACCEPT" ] && exit 0

if [ -f src.tar.gz ] ; then
    echo -n "Source installation path [$(pwd)]: "
    read src
    [ "${src}" = "" ] && src="$(pwd)"

    echo -n "Installing source to ${src}..."
    install -d ${src}
    tar -xzf src.tar.gz -C ${src}
    echo "done"

    kern_src=""
    if [ -d /usr/src ]; then
	kern_src="/usr/src"
    fi

    echo -n "Enter kernel source tree path [${kern_src}]: "
    read resp
    [ "${resp}" != "" ] && kern_src="${resp}"
    if [ ! -d "${kern_src}" ] || [ ! -f "${kern_src}/sys/conf/files" ] ; then
	echo "${kern_src} not good.. Exiting"
	exit 1
    fi

    # Create file for sys/conf/files
    dev=wl
    conf_file="${src}/files.${dev}"
    [ -f ${conf_file} ] && mv ${conf_file} ${conf_file}.bak
    for i in `find ${src}/src -name "*.c"`; do
	if [ $i != "${src}/src/wl/sys/wl_bsd.c" ];  then
	    echo file $i ${dev} >> ${conf_file}
	fi
    done
    echo >> ${conf_file}
    echo prefix "${src}/src/include" >> ${conf_file}
    echo prefix >> ${conf_file}
    echo prefix "${src}/src/wl/sys" >> ${conf_file}
    echo prefix >> ${conf_file}
    echo File ${conf_file} created for sys/conf/files

    # Create file for kernel config
    conf_file="${src}/${dev}_CONF"
    [ -f ${conf_file} ] && mv ${conf_file} ${conf_file}.bak
    arch=i386
    echo -n "Enter kernel architecture [${arch}]: "
    read resp
    for i in BCMDBG AP STA APSTA WET WLLED WME WL11H WL11D DBAND WLRM WLCNT BCMWPA2 WLAMSDU WLAMSDU_SWDEAGG WLAMPDU BCMDRIVER BCMDMA64
    do
      echo "options $i" >> ${conf_file}
    done
    netbsd_rev=`grep __NetBSD_Version__ ${kern_src}/sys/sys/param.h | grep define | cut -f 3`
    # Add support for net80211 extension API with 3.99.0
    if [ $netbsd_rev -ge 399000000 ]
    then
	echo Enabling support for new Net80211 API
	echo options __Net80211_EXT__ >> ${conf_file}
    fi
    echo >> ${conf_file}
    echo "prefix ${kern_src}/sys/arch/${arch}/include" >> ${conf_file}
    echo "prefix" >> ${conf_file}
    echo File ${conf_file} created for kernel config file

    resp=""
    # Offer to modify configuration for the kernel
    echo -n "Do you want to modify kernel files? [n]:"
    read resp
    [ "${resp}" = "" ] || [ "${resp}" = "n" ] && exit 0

    # Fix sys config file
    conf_file="${kern_src}/sys/conf/files"
    echo "Saving original file ${conf_file} to ${conf_file}.bak"
    cp ${conf_file} ${conf_file}.bak

    if grep "device ${dev}:" ${conf_file}
    then
	echo "WARNING: A device with ${dev} already exists"
    fi

    echo >> ${conf_file}
    echo "#Broadcom 43xx 802.11" >> ${conf_file}
    echo "device ${dev}: arp, ether, ifnet, wlan" >> ${conf_file}
    echo include "${src}/files.${dev}" >> ${conf_file}
    echo ${conf_file} " is done"

    # Add PCI Configuration
    conf_file="${kern_src}/sys/dev/pci/files.pci"
    echo "Saving original file ${conf_file} to ${conf_file}.bak"
    cp ${conf_file} ${conf_file}.bak
    if grep "device ${dev}:" ${conf_file}
    then
	echo "WARNING: A device with ${dev} already exists in ${conf_file}"
    fi
    echo >> ${conf_file}
    echo "#Broadcom 43xx 802.11" >> ${conf_file}
    echo "attach ${dev} at pci with wl_pci" >> ${conf_file}
    echo file ${src}/src/wl/sys/wl_bsd.c wl_pci >> ${conf_file}
    echo ${conf_file} " is done"

    # Create Kernel configuration
    conf_file="${src}/WL_CONF"
    echo "${dev}*     at pci? dev ? function ?        # Broadcom 43xx 802.11" >> ${conf_file}
    echo include "${src}/${dev}_CONF" >> ${conf_file}
    echo "You will copy paste contents of ${conf_file} to kernel config"
    echo "Thank you. Please proceed to build the kernel."
fi
