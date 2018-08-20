/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

OUTPUT_FORMAT("elf64-littleaarch64")
OUTPUT_ARCH(aarch64)
ENTRY(mon_entry_point)

_mon_link_base = 0x100000000;
_mon_link_size = 0x100000;

SECTIONS
{
	/* Load address */
	. = _mon_link_base;
	ASSERT(. == ALIGN(4096), "Monitor image is not aligned on a page boundary.")

	_mon_img_start = .;
	.vectors :
	{
		_vectors_start = .;
		*(.vectors)
		_vectors_end = .;
	}

	.text : ALIGN(4096)
	{
		_text_start = .;
		_bootstrap_start = .;
		*head.o(.text.bootstrap)
		KEEP(*(.text.bootstrap))
		_bootstrap_end = .;

		*(.text* .gnu.linkonce.t.*)
		_text_end = .;
	}

	.rodata : ALIGN(4096)
	{
		_rodata_start = .;

		. = ALIGN(16);
		_interrupt_descs_start = .;
		KEEP(*(.interrupt_descs))
		_interrupt_descs_end = .;

		. = ALIGN(16);
		_service_descs_start = .;
		KEEP(*(.service_descs))
		_service_descs_end = .;

		. = ALIGN(16);
		*(.rodata*)
		_rodata_end = .;
	}

	. = ALIGN(4096); /* align to page boundary */

	.data : ALIGN(16)
	{
		_data_start = .;
		*(.data*)
		_data_end = .;
	}

	.boot_params : ALIGN(16)
	{
		_boot_params_start = .;
		*(.boot_params)
		_boot_params_end = .;
	}

	.bss (NOLOAD) : ALIGN(16)
	{
		_bss_start = .;
		*(.bss*)
		*(COMMON)
		_bss_end = .;
	}

	.stacks (NOLOAD) : ALIGN(4096)
	{
		_stacks_start = .;
		*(.stacks)
		_stacks_end = .;
	}

	.xlat_tables (NOLOAD) : ALIGN(4096)
	{
		_xlat_tables_start = .;
		*(.xlat_tables)
		_xlat_tables_end = .;
	}

	.coherent_ram (NOLOAD) : ALIGN(4096)
	{
		_coherent_ram_start = .;
		*(.coherent_ram)
		_coherent_ram_end = .;
	}
	_mon_img_end = .;

	ASSERT(. <= (_mon_link_base + _mon_link_size), "Monitor image has exceeded its size limit.")
}
