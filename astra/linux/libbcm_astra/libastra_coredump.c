/***************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 ***************************************************************************/

#include "config.h"
#include <stdio.h>
#include <string.h>
#include <sys/ucontext.h>
#include "bfd.h"
#include "elf-bfd.h"		/* for elfcore_write_* */
#include "obstack.h"

#include "signals.h"
#include "gregset.h"
#include "libastra_coredump.h"

typedef struct coredump_msg{
    char  msgCmd;
    char  msg[20];  /* actual msg can be more than 20 bytes */
} coredump_msg;

typedef struct coredump_msg_memregion {
    char                memRegionCnt;
    coredump_memregion  region[2]; /* actual region can be more than 2 */
} coredump_msg_memregion;

typedef struct coredump_msg_memcpy {
    char *startAddr;
    int  length;
    char bytes[8]; /* actual bytes can be more than 8 bytes */
} coredump_msg_memcpy;

typedef enum coredump_cmd{
    coredump_get_filename, /* program name */
    coredump_get_psargs,   /* program arguments */
    coredump_get_stop_signal, /* signal that cause the program to stop */
    coredump_get_prstatus, /* general registers */
    coredump_get_memregions, /* get memory sections, starting addr, ending addr, offset, name */
    coredump_get_memcpy,  /* copy content of memory region at addr */
}coredump_cmd;
static int bytes_read =0;

int handleProc(coredump_msg *pMsg,unsigned int *mem)
{
	switch(pMsg->msgCmd)
	{
	case coredump_get_filename:
    {
        strncpy(pMsg->msg,"astra",6);
        break;
    }
	case coredump_get_psargs:
    {
        strncpy(pMsg->msg,"astra",6);
        break;
    }
	case coredump_get_stop_signal:
    {
        pMsg->msg[0] = GDB_SIGNAL_TRAP;
        break;
    }
	case coredump_get_prstatus:
    {
		int *payLoad = (int *)(pMsg->msg);
        memcpy(payLoad,mem,sizeof(unsigned long)*17);
		bytes_read += sizeof(unsigned long)*20;
        break;
    }
    case coredump_get_memregions:
    {
		int i=0;
		unsigned int * buf = mem + bytes_read;
		coredump_msg_memregion *pMsgRegions = (coredump_msg_memregion *)(pMsg->msg);
        coredump_memregion *pRegion = pMsgRegions->region;
		coredump_memregion *data;
        int regionCnt = *buf;
		buf += sizeof(int);
		bytes_read += sizeof(int);

		data = (coredump_memregion *)buf;

		for(i=0;i<regionCnt;i++)
		{
			unsigned int flags;
			flags = SEC_ALLOC | SEC_HAS_CONTENTS | SEC_LOAD;
             if((data->flags & 0x2) == 0x2)
                flags |= SEC_DATA;
             else if((data->flags & 0x1) == 0x1)
                flags |= SEC_READONLY;
             if((data->flags & 0x8)  == 0x8)
                flags |= SEC_CODE;

			pRegion->startAddr = data->startAddr;
			pRegion->endAddr = data->endAddr;
			pRegion->flags = flags;
			pRegion->offset = data->offset;
			pRegion++;
			data++;
			bytes_read += sizeof(coredump_memregion);
		}
        pMsgRegions->memRegionCnt = regionCnt;
        break;
    }
	case coredump_get_memcpy:
    {
		coredump_msg_memcpy *pMemcpy = (coredump_msg_memcpy *)pMsg->msg;
        memcpy(pMemcpy->bytes,pMemcpy->startAddr,pMemcpy->length);
		bytes_read += pMemcpy->length;
        break;
    }
    default:
        printf("%s: message type %d not recognized!\n",__FUNCTION__,pMsg->msgCmd);
        return 1;
	}
    return 0;
}

typedef struct elf_section {
    asection  *section;
    char      *dataBuffer;
    int       size;
}elf_section;

int make_note_section(bfd *obfd,elf_section *noteSec,unsigned int *mem)
{
    char msg1[1024],msg2[1024],*fname,*psargs;
    coredump_msg *pMsg;
    char *note_data = NULL;
    asection *note_sec = NULL;
    int note_size=0;
    enum gdb_signal stop_signal;
    gregset_t *gregs;

    if(noteSec == NULL)
    {
        printf(" %s %d noteSec is NULL!",__FUNCTION__,__LINE__);
        return 1;
    }
    if(obfd == NULL)
    {
        printf(" %s %d obfd file handle is NULL!",__FUNCTION__,__LINE__);
        return 1;
    }

    pMsg = (coredump_msg *)msg1;
    pMsg->msgCmd = coredump_get_filename;
    handleProc(pMsg,mem);
    fname = pMsg->msg;

    pMsg = (coredump_msg *)msg2;
    pMsg->msgCmd = coredump_get_psargs;
    handleProc(pMsg,mem);
    psargs = pMsg->msg;

    note_data = (char *) elfcore_write_prpsinfo (obfd,note_data,&note_size,fname,psargs);

    pMsg = (coredump_msg *)msg1;
    pMsg->msgCmd = coredump_get_stop_signal;
    handleProc(pMsg,mem);
    stop_signal = pMsg->msg[0];

    pMsg = (coredump_msg *)msg2;
    pMsg->msgCmd = coredump_get_prstatus;
    handleProc(pMsg,mem);
    gregs = (gregset_t *)(pMsg->msg);

    note_data = (char *) elfcore_write_prstatus (obfd,
					       note_data,
					       &note_size,
					       1,
					       stop_signal,
					       gregs);

    note_sec = bfd_make_section_anyway_with_flags (obfd, "note0",SEC_HAS_CONTENTS| SEC_READONLY| SEC_ALLOC);

    bfd_set_section_vma (obfd, note_sec, 0);
    bfd_set_section_alignment (obfd, note_sec, 0);
	bfd_set_section_size (obfd, note_sec, note_size);

    noteSec->section = note_sec;
    noteSec->dataBuffer = note_data;
    noteSec->size = note_size;

    return 0;
}

int make_memory_section(bfd *obfd,elf_section *memorySec,char *vaddr,int size,flagword flags)
{

    asection *osec;

    if(memorySec == NULL)
    {
        printf(" %s %d noteSec is NULL!",__FUNCTION__,__LINE__);
        return 1;
    }
    if(obfd == NULL)
    {
        printf(" %s %d obfd file handle is NULL!",__FUNCTION__,__LINE__);
        return 1;
    }

    osec = bfd_make_section_anyway_with_flags (obfd, "load", flags);
    if (osec == NULL)
    {
        printf("\n Couldn't make gcore segment: %s",bfd_errmsg (bfd_get_error ()));
        return 1;
    }

    bfd_set_section_vma (obfd, osec, (bfd_vma)vaddr);
    bfd_section_lma (obfd, osec) = 0;

    memorySec->section = osec;
    memorySec->dataBuffer = vaddr;
    memorySec->size = size;

    return 0;
}

int set_section_header(bfd *obfd,elf_section *elfSec)
{
    if(elfSec == NULL)
    {
        printf(" %s %d elfSec is NULL!",__FUNCTION__,__LINE__);
        return 1;
    }
    if(obfd == NULL)
    {
        printf(" %s %d obfd file handle is NULL!",__FUNCTION__,__LINE__);
        return 1;
    }

    int p_flags = 0;
    int p_type = 0;
    asection *osec = elfSec->section;

    bfd_set_section_size (obfd, osec, elfSec->size);

	if (strncmp (bfd_section_name (obfd, osec), "load", 4) == 0)
        p_type = PT_LOAD;
    else if (strncmp (bfd_section_name (obfd, osec), "note", 4) == 0)
        p_type = PT_NOTE;
    else
        p_type = PT_NULL;

    p_flags |= PF_R;	/* Segment is readable.  */
    if (!(bfd_get_section_flags (obfd, osec) & SEC_READONLY))
      p_flags |= PF_W;	/* Segment is writable.  */
    if (bfd_get_section_flags (obfd, osec) & SEC_CODE)
      p_flags |= PF_X;	/* Segment is executable.  */

    bfd_record_phdr (obfd, p_type, 1, p_flags, 0, 0, 0, 0, 1, &osec);

    return 0;

}

int fill_section_content(bfd *obfd,elf_section *elfSec)
{
    if(elfSec == NULL)
    {
        printf(" %s %d elfSec is NULL!",__FUNCTION__,__LINE__);
        return 1;
    }
    if(obfd == NULL)
    {
        printf(" %s %d obfd file handle is NULL!",__FUNCTION__,__LINE__);
        return 1;
    }

    asection *osec = elfSec->section;

    if(!bfd_set_section_contents (obfd, osec, elfSec->dataBuffer, 0, elfSec->size))
    {
        printf("\n writing section failed @ line(%d)",__LINE__);
        return 1;
    }

    return 0;

}

int fill_memory_section_content(bfd *obfd,elf_section *elfSec,unsigned int *mem)
{
    if(elfSec == NULL)
    {
        printf(" %s %d elfSec is NULL!",__FUNCTION__,__LINE__);
        return 1;
    }
    if(obfd == NULL)
    {
        printf(" %s %d obfd file handle is NULL!",__FUNCTION__,__LINE__);
        return 1;
    }

    asection *osec = elfSec->section;

    int total_size;
    char *startAddr;

    startAddr = (char *)mem + bytes_read;

    total_size = elfSec->size;

	if(!bfd_set_section_contents (obfd, osec, startAddr, 0, total_size))
	{
	    printf("\n writing section failed @ line(%d)",__LINE__);
	    return 1;
	}
	bytes_read += total_size;

    return 0;

}

int _astra_uapp_coredump_proc(unsigned int * mem)
{
    elf_section noteSec,memorySec[50];
    coredump_msg *pMsg;
    char msg1[1024];
    int memorySecCnt,i;

    bfd *obfd = bfd_openw ("core.1", "default");

    if (!obfd)
        printf("\n Failed to open '%s' for output.", "core.1");
	else
		printf("\n Opened '%s' for output.", "core.1");

    bfd_set_format (obfd, bfd_core);

    bfd_set_arch_mach (obfd, bfd_arch_arm, bfd_mach_arm_5);

    make_note_section(obfd,&noteSec,mem);

    pMsg = (coredump_msg *)msg1;
    pMsg->msgCmd = coredump_get_memregions;
    handleProc(pMsg,mem);

    coredump_msg_memregion *pMsgRegions = (coredump_msg_memregion *)(pMsg->msg);
    memorySecCnt = pMsgRegions->memRegionCnt;
    coredump_memregion *pRegion = pMsgRegions->region;

    for(i=0;i<memorySecCnt;i++,pRegion++)
    {
         make_memory_section(obfd,&memorySec[i],(void *)pRegion->startAddr,(int)(pRegion->endAddr - pRegion->startAddr +1),pRegion->flags);
    }

    set_section_header(obfd,&noteSec);

    for(i=0;i<memorySecCnt;i++)
        set_section_header(obfd,&memorySec[i]);

    fill_section_content(obfd,&noteSec);

    for(i=0;i<memorySecCnt;i++)
    {
        if(i != 8)
        fill_memory_section_content(obfd,&memorySec[i],mem);
    }
    bfd_close(obfd);

    return 0;
}
