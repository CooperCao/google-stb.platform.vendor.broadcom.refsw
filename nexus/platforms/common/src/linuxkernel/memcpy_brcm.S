############################################################
#     (c)2009-2010 Broadcom Corporation
#
#  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
#
# $brcm_Workfile: $
# $brcm_Revision: $
# $brcm_Date: $
#
# Module Description:
#   A precompiled version of the Broadcom developed memcpy_fpu_5000_Android.S
#
# Revision History:
#
# $brcm_Log: $
# 
############################################################
 .text
 .global memcpy
 .ent memcpy
 .set noreorder
memcpy:

 slti $8, $6, 8 # Less than 8 bytes?
 bne $8, $0, last8ByteCopy # Yes, proceed to process 8 bytes.
 move $2, $4 # setup exit value before too late

 xor $8, $5, $4 # find $4/$5 displacement
 andi $8, 0x7



 subu $9, $0, $5

 andi $8, 0x3
 beq $8, $0, wordAlign # go handle the word-aligned case
 nop
 b unAlignSrcDest # go handle the un-aligned case.
 subu $7, $0, $4




doubleWordAlign:
 andi $9, 0x7 # $4/$5 are aligned, but r we
 beq $9, $0, dwCheck8w # starting in middle of a word?
 subu $6, $9

adjust:
 andi $10, $9, 0x3
 lwl $8, 0($5) # src is in the middle of a word...
 addu $5, $9
 swl $8, 0($4)
 addu $4, $9

 andi $9, 0x4 # if extra word, then adjust again.
 beq $9, $0, dwCheck8w
 nop
 lw $8, -4($5)
 sw $8, -4($4)

dwCheck8w: # SRC is at begin of word
 andi $8, $6, 0x1ff # 512 or more bytes left ?
 beq $8, $6, check4w # NO, less than 512, proceed to process 4w/16B
 subu $7, $6, $8 # Yes, more than 512, maybe we can use FPU copy

 addu $7, $4 # $7 = end address of loop
 subu $7, $7, 0x100
 .align 4
 move $6, $8 # $6 = what will be left after loop







 subu $29, $29, 40
 sdc1 $f12, 0($29)
 sdc1 $f14, 8($29)
 sdc1 $f20, 16($29)
 sdc1 $f24, 24($29)
 sdc1 $f26, 32($29)


 ldc1 $f4, 0x0($5)
 ldc1 $f20, 0x80($5)
 ldc1 $f6, 0x20($5)
 ldc1 $f8, 0x40($5)
 ldc1 $f10, 0x60($5)
 ldc1 $f18, 0xa0($5)
 ldc1 $f24, 0xc0($5)
 ldc1 $f26, 0xe0($5)

 #pref 30, 0x20($4) # (prepare for store)
 pref 30, 0x40($4)
 pref 30, 0x60($4)

fmCopyLoopBack:

 ldc1 $f12, 0x8($5)
 ldc1 $f14, 0x10($5)
 ldc1 $f16, 0x18($5)
 sdc1 $f4, 0x0($4)
 ldc1 $f4, 0x100($5)
 sdc1 $f12, 0x8($4)
 sdc1 $f14, 0x10($4)
 sdc1 $f16, 0x18($4)

 pref 30, 0x80($4)

 ldc1 $f12, 0x28($5)
 ldc1 $f14, 0x30($5)
 ldc1 $f16, 0x38($5)
 sdc1 $f6, 0x20($4)
 ldc1 $f6, 0x120($5)
 sdc1 $f12, 0x28($4)
 sdc1 $f14, 0x30($4)
 sdc1 $f16, 0x38($4)

 pref 30, 0xa0($4)

 ldc1 $f12, 0x48($5)
 ldc1 $f14, 0x50($5)
 ldc1 $f16, 0x58($5)
 sdc1 $f8, 0x40($4)
 ldc1 $f8, 0x140($5)
 sdc1 $f12, 0x48($4)
 sdc1 $f14, 0x50($4)
 sdc1 $f16, 0x58($4)

 pref 30, 0xc0($4)

 ldc1 $f12, 0x68($5)
 ldc1 $f14, 0x70($5)
 ldc1 $f16, 0x78($5)
 sdc1 $f10, 0x60($4)
 ldc1 $f10, 0x160($5)
 sdc1 $f12, 0x68($4)
 sdc1 $f14, 0x70($4)
 sdc1 $f16, 0x78($4)

 pref 30, 0xe0($4)


 ldc1 $f12, 0x88($5)
 ldc1 $f14, 0x90($5)
 ldc1 $f16, 0x98($5)
 sdc1 $f20, 0x80($4)
 ldc1 $f20, 0x180($5)
 sdc1 $f12, 0x88($4)
 sdc1 $f14, 0x90($4)
 sdc1 $f16, 0x98($4)

 pref 30, 0x100($4)

 ldc1 $f12, 0xa8($5)
 ldc1 $f14, 0xb0($5)
 ldc1 $f16, 0xb8($5)
 sdc1 $f18, 0xa0($4)
 ldc1 $f18, 0x1a0($5)
 sdc1 $f12, 0xa8($4)
 sdc1 $f14, 0xb0($4)
 sdc1 $f16, 0xb8($4)

 pref 30, 0x120($4)

 ldc1 $f12, 0xc8($5)
 ldc1 $f14, 0xd0($5)
 ldc1 $f16, 0xd8($5)
 sdc1 $f24, 0xc0($4)
 ldc1 $f24, 0x1c0($5)
 sdc1 $f12, 0xc8($4)
 sdc1 $f14, 0xd0($4)
 sdc1 $f16, 0xd8($4)

 pref 30, 0x140($4)

 ldc1 $f12, 0xe8($5)
 ldc1 $f14, 0xf0($5)
 ldc1 $f16, 0xf8($5)
 sdc1 $f26, 0xe0($4)
 ldc1 $f26, 0x1e0($5)
 sdc1 $f12, 0xe8($4)
 sdc1 $f14, 0xf0($4)
 sdc1 $f16, 0xf8($4)

 pref 30, 0x160($4)

        add $4, $4, 0x100
        bne $4, $7, fmCopyLoopBack
        add $5, $5, 0x100


 ldc1 $f4, 0x0($5)
 ldc1 $f20, 0x80($5)
 ldc1 $f6, 0x20($5)
 ldc1 $f8, 0x40($5)
 ldc1 $f10, 0x60($5)
 ldc1 $f18, 0xa0($5)
 ldc1 $f24, 0xc0($5)
 ldc1 $f26, 0xe0($5)

 ldc1 $f12, 0x8($5)
 ldc1 $f14, 0x10($5)
 ldc1 $f16, 0x18($5)
 sdc1 $f4, 0x0($4)
 sdc1 $f12, 0x8($4)
 sdc1 $f14, 0x10($4)
 sdc1 $f16, 0x18($4)

 ldc1 $f12, 0x28($5)

 ldc1 $f14, 0x30($5)
 ldc1 $f16, 0x38($5)
 sdc1 $f6, 0x20($4)
 sdc1 $f12, 0x28($4)
 sdc1 $f14, 0x30($4)
 sdc1 $f16, 0x38($4)

 ldc1 $f12, 0x48($5)
 ldc1 $f14, 0x50($5)
 ldc1 $f16, 0x58($5)
 sdc1 $f8, 0x40($4)
 sdc1 $f12, 0x48($4)
 sdc1 $f14, 0x50($4)
 sdc1 $f16, 0x58($4)

 ldc1 $f12, 0x68($5)
 ldc1 $f14, 0x70($5)
 ldc1 $f16, 0x78($5)
 sdc1 $f10, 0x60($4)
 sdc1 $f12, 0x68($4)
 sdc1 $f14, 0x70($4)
 sdc1 $f16, 0x78($4)


 ldc1 $f12, 0x88($5)
 ldc1 $f14, 0x90($5)
 ldc1 $f16, 0x98($5)
 sdc1 $f20, 0x80($4)
 sdc1 $f12, 0x88($4)
 sdc1 $f14, 0x90($4)
 sdc1 $f16, 0x98($4)

 ldc1 $f12, 0xa8($5)
 ldc1 $f14, 0xb0($5)
 ldc1 $f16, 0xb8($5)
 sdc1 $f18, 0xa0($4)
 sdc1 $f12, 0xa8($4)
 sdc1 $f14, 0xb0($4)
 sdc1 $f16, 0xb8($4)

 ldc1 $f12, 0xc8($5)
 ldc1 $f14, 0xd0($5)
 ldc1 $f16, 0xd8($5)
 sdc1 $f24, 0xc0($4)
 sdc1 $f12, 0xc8($4)
 sdc1 $f14, 0xd0($4)
 sdc1 $f16, 0xd8($4)

 ldc1 $f12, 0xe8($5)
 ldc1 $f14, 0xf0($5)
 ldc1 $f16, 0xf8($5)
 sdc1 $f26, 0xe0($4)
 sdc1 $f12, 0xe8($4)
 sdc1 $f14, 0xf0($4)
 sdc1 $f16, 0xf8($4)

        add $5, $5, 0x100
        add $4, $4, 0x100


 ldc1 $f12, 0($29)
 ldc1 $f14, 8($29)
 ldc1 $f20, 16($29)
 ldc1 $f24, 24($29)
 ldc1 $f26, 32($29)
 addu $29, $29, 40


 # Check if we could use LW/SW to copy.

check4w: andi $8, $6, 0xf # 16 or more bytes left?
        beq $8, $6, check1w # NO, less than 16, proceed to check1w (4bytes loop)
        subu $7, $6, $8 # Yes, handle them in 16 bytes loop.

        addu $7, $5 # $7 = end address.
        move $6, $8

loop4w: lw $8, 0($5) # loop for 16 bytes/4 words at a time.
 lw $9, 4($5)
 lw $10, 8($5)
 lw $11, 0xc($5)
 sw $8, 0($4)
 sw $9, 4($4)
 sw $10, 8($4)
 addiu $4, 16
 addiu $5, 16
 bne $5, $7, loop4w
 sw $11, -4($4)

check1w: andi $8, $6, 0x3 # 4 or more bytes left?
        beq $8, $6, last8ByteCopy # NO, less than 4 bytes, proceed to process 3 bytes
        subu $7, $6, $8 # Yes, handle them 1 word at a time
        addu $7, $5 # $7 = end address.
        move $6, $8

loop1w: lw $8, 0($5) # loop 4 bytes/1 word at a time.
 addiu $4, 4
 addiu $5, 4
 bne $5, $7, loop1w
 sw $8, -4($4)

last8ByteCopy: blez $6, last8BCExit # handle last 8 bytes, one byte at a time.
 addu $7, $6, $5

last8BCLoopBack: lb $8, 0($5) # last 8 bytes copy loop.
 addiu $4, 1
 addiu $5, 1
 bne $5, $7, last8BCLoopBack
 sb $8, -1($4)

last8BCExit:
 jr $31 # return to caller.
 nop





wordAlign:
 andi $9, 0x3 # $4/$5 are aligned, but r we
 beq $9, $0, intCheck8w # starting in middle of a word?
 subu $6, $9

 lwl $8, 0($5) # src is in the middle of a word...
 addu $5, $9
 swl $8, 0($4)
 addu $4, $9

intCheck8w: # SRC is at begin of word
 andi $8, $6, 0x1ff # 512 or more bytes left ?
 beq $8, $6, check4w # NO, less than 512, proceed to process 4w/16B
 subu $7, $6, $8 # Yes, more than 512, maybe we can use FPU copy

    # $7 = copy size
 subu $7, $7, 0x100
 .align 4
 move $6, $8 # $6 = what will be left after loop







 add $2, $5, $7 #start address A($5), end address A($2)
        add $3, $4, $7 #start address B($4), end address B($3)


 subu $29, $29, 28
 sw $16, 0($29)
 sw $17, 4($29)
 sw $18, 8($29)
 sw $19, 12($29)
 sw $20, 16($29)
 sw $21, 20($29)
 sw $22, 24($29)

 lw $8, 0x0($5) # The first 2 to trigger h/w prefetch
 lw $9, 0x20($5)
 lw $12, 0x80($5) # trigger double prefetch
 lw $10, 0x40($5)
 lw $11, 0x60($5)
 lw $13, 0xa0($5)
 lw $14, 0xc0($5)
 lw $15, 0xe0($5)

 #pref 30, 0x20($4) # (prepare for store)
 pref 30, 0x40($4)
 pref 30, 0x60($4)

intCopyLoopBack:

 lw $16, 0x4($5)
 lw $17, 0x8($5)
 lw $18, 0xc($5)
 lw $19, 0x10($5)
 lw $20, 0x14($5)
 lw $21, 0x18($5)
 lw $22, 0x1c($5)

 sw $8, 0x0($4)
 lw $8, 0x100($5)

 sw $16, 0x4($4)
 sw $17, 0x8($4)
 sw $18, 0xc($4)
 sw $19, 0x10($4)
 sw $20, 0x14($4)
 sw $21, 0x18($4)
 sw $22, 0x1c($4)

 pref 30, 0x80($4)

 lw $16, 0x24($5)
 lw $17, 0x28($5)
 lw $18, 0x2c($5)
 lw $19, 0x30($5)
 lw $20, 0x34($5)
 lw $21, 0x38($5)
 lw $22, 0x3c($5)

 sw $9, 0x20($4)
 lw $9, 0x120($5)

 sw $16, 0x24($4)
 sw $17, 0x28($4)
 sw $18, 0x2c($4)
 sw $19, 0x30($4)
 sw $20, 0x34($4)
 sw $21, 0x38($4)
 sw $22, 0x3c($4)

 pref 30, 0xa1($4)

 lw $16, 0x44($5)
 lw $17, 0x48($5)
 lw $18, 0x4c($5)
 lw $19, 0x50($5)
 lw $20, 0x54($5)
 lw $21, 0x58($5)
 lw $22, 0x5c($5)

 sw $10, 0x40($4)
 lw $10, 0x140($5)

 sw $16, 0x44($4)
 sw $17, 0x48($4)
 sw $18, 0x4c($4)
 sw $19, 0x50($4)
 sw $20, 0x54($4)
 sw $21, 0x58($4)
 sw $22, 0x5c($4)

 pref 30, 0xc0($4)

 lw $16, 0x64($5)
 lw $17, 0x68($5)
 lw $18, 0x6c($5)
 lw $19, 0x70($5)
 lw $20, 0x74($5)
 lw $21, 0x78($5)
 lw $22, 0x7c($5)

 sw $11, 0x60($4)
 lw $11, 0x160($5)

 sw $16, 0x64($4)
 sw $17, 0x68($4)
 sw $18, 0x6c($4)
 sw $19, 0x70($4)
 sw $20, 0x74($4)
 sw $21, 0x78($4)
 sw $22, 0x7c($4)

 pref 30, 0xe0($4)


 lw $16, 0x84($5)
 lw $17, 0x88($5)
 lw $18, 0x8c($5)
 lw $19, 0x90($5)
 lw $20, 0x94($5)
 lw $21, 0x98($5)
 lw $22, 0x9c($5)

 sw $12, 0x80($4)
 lw $12, 0x180($5)

 sw $16, 0x84($4)
 sw $17, 0x88($4)
 sw $18, 0x8c($4)
 sw $19, 0x90($4)
 sw $20, 0x94($4)
 sw $21, 0x98($4)
 sw $22, 0x9c($4)

 pref 30, 0x100($4)

 lw $16, 0xa4($5)
 lw $17, 0xa8($5)
 lw $18, 0xac($5)
 lw $19, 0xb0($5)
 lw $20, 0xb4($5)
 lw $21, 0xb8($5)
 lw $22, 0xbc($5)

 sw $13, 0xa0($4)
 lw $13, 0x1a0($5)

 sw $16, 0xa4($4)
 sw $17, 0xa8($4)
 sw $18, 0xac($4)
 sw $19, 0xb0($4)
 sw $20, 0xb4($4)
 sw $21, 0xb8($4)
 sw $22, 0xbc($4)

 pref 30, 0x120($4)

 lw $16, 0xc4($5)
 lw $17, 0xc8($5)
 lw $18, 0xcc($5)
 lw $19, 0xd0($5)
 lw $20, 0xd4($5)
 lw $21, 0xd8($5)
 lw $22, 0xdc($5)

 sw $14, 0xc0($4)
 lw $14, 0x1c0($5)

 sw $16, 0xc4($4)
 sw $17, 0xc8($4)
 sw $18, 0xcc($4)
 sw $19, 0xd0($4)
 sw $20, 0xd4($4)
 sw $21, 0xd8($4)
 sw $22, 0xdc($4)

 pref 30, 0x140($4)

 lw $16, 0xe4($5)
 lw $17, 0xe8($5)
 lw $18, 0xec($5)
 lw $19, 0xf0($5)
 lw $20, 0xf4($5)
 lw $21, 0xf8($5)
 lw $22, 0xfc($5)

 sw $15, 0xe0($4)
 lw $15, 0x1e0($5)

 sw $16, 0xe4($4)
 sw $17, 0xe8($4)
 sw $18, 0xec($4)
 sw $19, 0xf0($4)
 sw $20, 0xf4($4)
 sw $21, 0xf8($4)
 sw $22, 0xfc($4)

 pref 30, 0x160($4)

        add $4, $4, 0x100
        bne $4, $3, intCopyLoopBack
        add $5, $5, 0x100


 lw $16, 0x4($5)
 lw $17, 0x8($5)
 lw $18, 0xc($5)
 lw $19, 0x10($5)
 lw $20, 0x14($5)
 lw $21, 0x18($5)
 lw $22, 0x1c($5)

 sw $8, 0x00($4)

 sw $16, 0x04($4)
 sw $17, 0x08($4)
 sw $18, 0x0c($4)
 sw $19, 0x10($4)
 sw $20, 0x14($4)
 sw $21, 0x18($4)
 sw $22, 0x1c($4)

 lw $16, 0x24($5)
 lw $17, 0x28($5)
 lw $18, 0x2c($5)
 lw $19, 0x30($5)
 lw $20, 0x34($5)
 lw $21, 0x38($5)
 lw $22, 0x3c($5)

 sw $9, 0x20($4)

 sw $16, 0x24($4)
 sw $17, 0x28($4)
 sw $18, 0x2c($4)
 sw $19, 0x30($4)
 sw $20, 0x34($4)
 sw $21, 0x38($4)
 sw $22, 0x3c($4)

 lw $16, 0x44($5)
 lw $17, 0x48($5)
 lw $18, 0x4c($5)
 lw $19, 0x50($5)
 lw $20, 0x54($5)
 lw $21, 0x58($5)
 lw $22, 0x5c($5)

 sw $10, 0x40($4)

 sw $16, 0x44($4)
 sw $17, 0x48($4)
 sw $18, 0x4c($4)
 sw $19, 0x50($4)
 sw $20, 0x54($4)
 sw $21, 0x58($4)
 sw $22, 0x5c($4)

 lw $16, 0x64($5)
 lw $17, 0x68($5)
 lw $18, 0x6c($5)
 lw $19, 0x70($5)
 lw $20, 0x74($5)
 lw $21, 0x78($5)
 lw $22, 0x7c($5)

 sw $11, 0x60($4)

 sw $16, 0x64($4)
 sw $17, 0x68($4)
 sw $18, 0x6c($4)
 sw $19, 0x70($4)
 sw $20, 0x74($4)
 sw $21, 0x78($4)
 sw $22, 0x7c($4)


 lw $16, 0x84($5)
 lw $17, 0x88($5)
 lw $18, 0x8c($5)
 lw $19, 0x90($5)
 lw $20, 0x94($5)
 lw $21, 0x98($5)
 lw $22, 0x9c($5)

 sw $12, 0x80($4)

 sw $16, 0x84($4)
 sw $17, 0x88($4)
 sw $18, 0x8c($4)
 sw $19, 0x90($4)
 sw $20, 0x94($4)
 sw $21, 0x98($4)
 sw $22, 0x9c($4)

 lw $16, 0xa4($5)
 lw $17, 0xa8($5)
 lw $18, 0xac($5)
 lw $19, 0xb0($5)
 lw $20, 0xb4($5)
 lw $21, 0xb8($5)
 lw $22, 0xbc($5)

 sw $13, 0xa0($4)

 sw $16, 0xa4($4)
 sw $17, 0xa8($4)
 sw $18, 0xac($4)
 sw $19, 0xb0($4)
 sw $20, 0xb4($4)
 sw $21, 0xb8($4)
 sw $22, 0xbc($4)

 lw $16, 0xc4($5)
 lw $17, 0xc8($5)
 lw $18, 0xcc($5)
 lw $19, 0xd0($5)
 lw $20, 0xd4($5)
 lw $21, 0xd8($5)
 lw $22, 0xdc($5)

 sw $14, 0xc0($4)

 sw $16, 0xc4($4)
 sw $17, 0xc8($4)
 sw $18, 0xcc($4)
 sw $19, 0xd0($4)
 sw $20, 0xd4($4)
 sw $21, 0xd8($4)
 sw $22, 0xdc($4)

 lw $16, 0xe4($5)
 lw $17, 0xe8($5)
 lw $18, 0xec($5)
 lw $19, 0xf0($5)
 lw $20, 0xf4($5)
 lw $21, 0xf8($5)
 lw $22, 0xfc($5)

 sw $15, 0xe0($4)

 sw $16, 0xe4($4)
 sw $17, 0xe8($4)
 sw $18, 0xec($4)
 sw $19, 0xf0($4)
 sw $20, 0xf4($4)
 sw $21, 0xf8($4)
 sw $22, 0xfc($4)

        add $4, $4, 0x100
        add $5, $5, 0x100


 lw $16, 0($29)
 lw $17, 4($29)
 lw $18, 8($29)
 lw $19, 12($29)
 lw $20, 16($29)
 lw $21, 20($29)
 lw $22, 24($29)
 addu $29, $29, 28

 b check4w
 nop
unAlignSrcDest: # SRC and DEST are NOT aligned.
 andi $7, 0x3 # Is DEST word aligned?
 beq $7, $0, uaCheck512 # YES, DEST is word-aligned, SW may be used.
     # NO, DEST is NOT word-aligned, has to adjust.

 subu $6, $7 # $6 = number of bytes left

 lwl $8, 0($5) # DEST is NOT word aligned...
 lwr $8, 3($5) # adjust so DEST will be aligned.
 addu $5, $7
 swl $8, 0($4)
 addu $4, $7
uaCheck512: # DEST is word-aligned.
 andi $8, $6, 0x1ff # 512 or more bytes left ?
 beq $8, $6, uaCheck4w # No, less than 512, cannot execute "pref"
 subu $7, $6, $8 # Yes, more than 512, loop & "pref"

 addu $7, $4 # $7 = end address of loop
 subu $7, $7, 0x100
     .align 4
 move $6, $8 # $6 = what will be left after loop
     lwl $14, 0($5) # Loop taking 32 words at a time




 add $15, $4, 0x300 # prefetch dest 2 line size ahead.
uaLoopBack:
     pref 30, 0x40($4)
 lwl $13, 0x40($5)

 lwl $10, 0x4($5)
 lwl $11, 0x8($5)
 lwl $12, 0xc($5)

     lwr $14, 3($5)
 lwr $10, 0x7($5)
 lwr $11, 0xb($5)
 lwr $12, 0xf($5)

 sw $14, 0x0($4)
 sw $10, 0x4($4)
 sw $11, 0x8($4)
 sw $12, 0xc($4)

 # preload source
        bge $15, $7, uaSkip
 add $15, $15, 0x100
        lb $0, 0x300($5)
uaSkip:
 lwl $9, 0x10($5)
 lwl $10, 0x14($5)
 lwl $11, 0x18($5)
 lwl $12, 0x1c($5)
 lwr $9, 0x13($5)
 lwr $10, 0x17($5)
 lwr $11, 0x1b($5)
 lwr $12, 0x1f($5)

 sw $9, 0x10($4)
 sw $10, 0x14($4)
 sw $11, 0x18($4)
 sw $12, 0x1c($4)

 lwl $9, 0x20($5)
 lwl $10, 0x24($5)
 lwl $11, 0x28($5)
 lwl $12, 0x2c($5)
 lwr $9, 0x23($5)
 lwr $10, 0x27($5)
 lwr $11, 0x2b($5)
 lwr $12, 0x2f($5)

 sw $9, 0x20($4)
 sw $10, 0x24($4)
 sw $11, 0x28($4)
 sw $12, 0x2c($4)

 lwl $9, 0x30($5)
 lwl $10, 0x34($5)
 lwl $11, 0x38($5)
 lwl $12, 0x3c($5)
 lwr $9, 0x33($5)
 lwr $10, 0x37($5)
 lwr $11, 0x3b($5)
 lwr $12, 0x3f($5)

 sw $9, 0x30($4)
 sw $10, 0x34($4)
 sw $11, 0x38($4)
 sw $12, 0x3c($4)

     pref 30, 0x80($4)
 lwl $14, 0x80($5)

 lwl $10, 0x44($5)
 lwl $11, 0x48($5)
 lwl $12, 0x4c($5)
     lwr $13, 0x43($5)
 lwr $10, 0x47($5)
 lwr $11, 0x4b($5)
 lwr $12, 0x4f($5)

 sw $13, 0x40($4)
 sw $10, 0x44($4)
 sw $11, 0x48($4)
 sw $12, 0x4c($4)

 lwl $9, 0x50($5)
 lwl $10, 0x54($5)
 lwl $11, 0x58($5)
 lwl $12, 0x5c($5)
 lwr $9, 0x53($5)
 lwr $10, 0x57($5)
 lwr $11, 0x5b($5)
 lwr $12, 0x5f($5)

 sw $9, 0x50($4)
 sw $10, 0x54($4)
 sw $11, 0x58($4)
 sw $12, 0x5c($4)

 lwl $9, 0x60($5)
 lwl $10, 0x64($5)
 lwl $11, 0x68($5)
 lwl $12, 0x6c($5)
 lwr $9, 0x63($5)
 lwr $10, 0x67($5)
 lwr $11, 0x6b($5)
 lwr $12, 0x6f($5)

 sw $9, 0x60($4)
 sw $10, 0x64($4)
 sw $11, 0x68($4)
 sw $12, 0x6c($4)

 lwl $9, 0x70($5)
 lwl $10, 0x74($5)
 lwl $11, 0x78($5)
 lwl $12, 0x7c($5)
 lwr $9, 0x73($5)
 lwr $10, 0x77($5)
 lwr $11, 0x7b($5)
 lwr $12, 0x7f($5)

 sw $9, 0x70($4)
 sw $10, 0x74($4)
 sw $11, 0x78($4)
 sw $12, 0x7c($4)

     pref 30, 0xc0($4)
 lwl $13, 0xc0($5)

 lwl $10, 0x84($5)
 lwl $11, 0x88($5)
 lwl $12, 0x8c($5)
     lwr $14, 0x83($5)
 lwr $10, 0x87($5)
 lwr $11, 0x8b($5)
 lwr $12, 0x8f($5)

 sw $14, 0x80($4)
 sw $10, 0x84($4)
 sw $11, 0x88($4)
 sw $12, 0x8c($4)

 lwl $9, 0x90($5)
 lwl $10, 0x94($5)
 lwl $11, 0x98($5)
 lwl $12, 0x9c($5)
 lwr $9, 0x93($5)
 lwr $10, 0x97($5)
 lwr $11, 0x9b($5)
 lwr $12, 0x9f($5)

 sw $9, 0x90($4)
 sw $10, 0x94($4)
 sw $11, 0x98($4)
 sw $12, 0x9c($4)

 lwl $9, 0xa0($5)
 lwl $10, 0xa4($5)
 lwl $11, 0xa8($5)
 lwl $12, 0xac($5)
 lwr $9, 0xa3($5)
 lwr $10, 0xa7($5)
 lwr $11, 0xab($5)
 lwr $12, 0xaf($5)

 sw $9, 0xa0($4)
 sw $10, 0xa4($4)
 sw $11, 0xa8($4)
 sw $12, 0xac($4)

 lwl $9, 0xb0($5)
 lwl $10, 0xb4($5)
 lwl $11, 0xb8($5)
 lwl $12, 0xbc($5)
 lwr $9, 0xb3($5)
 lwr $10, 0xb7($5)
 lwr $11, 0xbb($5)
 lwr $12, 0xbf($5)

 sw $9, 0xb0($4)
 sw $10, 0xb4($4)
 sw $11, 0xb8($4)
 sw $12, 0xbc($4)

     pref 30, 0x100($4)
 lwl $14, 0x100($5)

 lwl $10, 0xc4($5)
 lwl $11, 0xc8($5)
 lwl $12, 0xcc($5)
     lwr $13, 0xc3($5)
 lwr $10, 0xc7($5)
 lwr $11, 0xcb($5)
 lwr $12, 0xcf($5)

 sw $13, 0xc0($4)
 sw $10, 0xc4($4)
 sw $11, 0xc8($4)
 sw $12, 0xcc($4)

 lwl $9, 0xd0($5)
 lwl $10, 0xd4($5)
 lwl $11, 0xd8($5)
 lwl $12, 0xdc($5)
 lwr $9, 0xd3($5)
 lwr $10, 0xd7($5)
 lwr $11, 0xdb($5)
 lwr $12, 0xdf($5)

 sw $9, 0xd0($4)
 sw $10, 0xd4($4)
 sw $11, 0xd8($4)
 sw $12, 0xdc($4)

 lwl $9, 0xe0($5)
 lwl $10, 0xe4($5)
 lwl $11, 0xe8($5)
 lwl $12, 0xec($5)
 lwr $9, 0xe3($5)
 lwr $10, 0xe7($5)
 lwr $11, 0xeb($5)
 lwr $12, 0xef($5)

 sw $9, 0xe0($4)
 sw $10, 0xe4($4)
 sw $11, 0xe8($4)
 sw $12, 0xec($4)

 lwl $9, 0xf0($5)
 lwl $10, 0xf4($5)
 lwl $11, 0xf8($5)
 lwl $12, 0xfc($5)
 lwr $9, 0xf3($5)
 lwr $10, 0xf7($5)
 lwr $11, 0xfb($5)
 lwr $12, 0xff($5)

 sw $9, 0xf0($4)
 sw $10, 0xf4($4)
 sw $11, 0xf8($4)
 sw $12, 0xfc($4)

 add $4, $4, 0x100
 bne $4, $7, uaLoopBack
 add $5, $5, 0x100

 addu $7, 0x100 # add 0x100 back


 # copy loop 32 words at a time.

uaRemain64LoopBack:
     lwl $14, 0($5) # Loop taking 32 words at a time
 lwl $10, 0x4($5)
 lwl $11, 0x8($5)
 lwl $12, 0xc($5)
     lwr $14, 3($5)
 lwr $10, 0x7($5)
 lwr $11, 0xb($5)
 lwr $12, 0xf($5)

 sw $14, 0x0($4)
 sw $10, 0x4($4)
 sw $11, 0x8($4)
 sw $12, 0xc($4)

 lwl $14, 0x10($5)
 lwl $10, 0x14($5)
 lwl $11, 0x18($5)
 lwl $12, 0x1c($5)
 lwr $14, 0x13($5)
 lwr $10, 0x17($5)
 lwr $11, 0x1b($5)
 lwr $12, 0x1f($5)

 sw $14, 0x10($4)
 sw $10, 0x14($4)
 sw $11, 0x18($4)
 sw $12, 0x1c($4)

 addiu $4, 0x20
 bne $4, $7, uaRemain64LoopBack
 addiu $5, 0x20

     addu $7, $6




uaCheck4w: andi $8, $6, 0xf # 16 or more bytes left?
        beq $8, $6, uaCheck1w # NO, <16 bytes, proceed to process 1w
        subu $7, $6, $8 # Yes, >16, copy 16 bytes at a time.

        addu $7, $5 # $7 = end address.
        move $6, $8

ua4wLoopBack: # loop 16 bytes/4 words at a time.
        lwl $8, 0($5)
        lwl $9, 4($5)
        lwl $10, 8($5)
        lwl $11, 0xc($5)
        lwr $8, 3($5)
        lwr $9, 7($5)
        lwr $10, 0xb($5)
        lwr $11, 0xf($5)
        sw $8, 0($4)
        sw $9, 4($4)
        sw $10, 8($4)
        addiu $4, 16
        addiu $5, 16
        bne $5, $7, ua4wLoopBack
        sw $11, -4($4)

uaCheck1w: andi $8, $6, 0x3 # 4 or more bytes left?
        beq $8, $6, last8ByteCopy # NO, <4 bytes, proceed to 8-bytes-copy
 subu $7, $6, $8

 addu $7, $4 # YES, >4 bytes, can use LW/SW.

uaRemain:
 lwl $9, 0($5) # copy 1 word/4 bytes at a time.
 lwr $9, 3($5)
 addiu $4, 4
 addiu $5, 4
 bne $4, $7, uaRemain
 sw $9, -4($4)

 b last8ByteCopy # handle anything that may be left.
 move $6, $8

 .set reorder

 .end memcpy
