/***************************************************************************
 *     (c)2007-2008 Broadcom Corporation
 *  
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.  
 *   
 *  Except as expressly set forth in the Authorized License,
 *   
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *   
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" 
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR 
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO 
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES 
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, 
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION 
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF 
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *  
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS 
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR 
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR 
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF 
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT 
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE 
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF 
 *  ANY LIMITED REMEDY.
 * 
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:  OpenCable Common Download storage interface. Functions
 *                      in this module provides nvram access
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "cdl_common.h"
#include "b_cdl_lib.h"
#include "lib_queue.h"
#ifdef DSG_SUPPORT
#include "cdl_ctl.h"
#endif
#if defined(MPOD_SUPPORT) 
#include "cdl_adpu.h"
#endif
#include "cdl_bootinfo_nvram.h"

BDBG_MODULE(cdl_bootinfo_nvram);

/* TODO: handle larger NAND page size */
#define NAND_PAGE_SIZE 0x800

/* 
 * The monolithic image should really be stored in /dev/mtdocap5 (image0) or 
 * /dev/mtdocap6(image1), 
 * (see mtdocap.c for mtdocap partitions, you can also find out the partitions
 * by 'cat /proc/mtdocap' command. and VALID_IMG should point to the good image that is 
 * been used right now.)
 * But in current reference platforms, only 97456 have >=64M flahs, 7118 and 7458
 * only have 32M, and there's no space for imag0 and image1 partition. So as a 
 * workaround, /dev/mtd0 (avail0) partition is used to store the monolithic image.
 * The side effect is, the /dev/mtd0 cannot be used to save jffs2 rootfs any more.
 */
static char storage_file_final[MAX_DEV_STR_LEN] = "/dev/mtd0"; 
static char storage_file_final_cfe[MAX_DEV_STR_LEN] = "flash0.avail0"; 

/* this should match mtdocap.c driver */
static char nvram_devname[MAX_DEV_STR_LEN] = "/dev/mtdocap4";

/*
 * TLV types.  These codes are used in the "type-length-value"
 * encoding of the items stored in the NVRAM device (flash or EEPROM)
 *
 * The layout of the flash/nvram is as follows:
 *
 * <type> <length> <data ...> <type> <length> <data ...> <type_end>
 *
 * The type code of "ENV_TLV_TYPE_END" marks the end of the list.
 * The "length" field marks the length of the data section, not
 * including the type and length fields.
 *
 * Environment variables are stored as follows:
 *
 * <type_env> <length> <flags> <name> = <value>
 *
 * If bit 0 (low bit) is set, the length is an 8-bit value.
 * If bit 0 (low bit) is clear, the length is a 16-bit value
 * 
 * Bit 7 set indicates "user" TLVs.  In this case, bit 0 still
 * indicates the size of the length field.  
 *
 * Flags are from the constants below:
 *
 */

#define ENV_LENGTH_16BITS	0x00	/* for low bit */
#define ENV_LENGTH_8BITS	0x01

#define ENV_TYPE_USER		0x80

#define ENV_CODE_SYS(n,l) (((n)<<1)|(l))
#define ENV_CODE_USER(n,l) ((((n)<<1)|(l)) | ENV_TYPE_USER)


/*
 * The actual TLV types we support
 */

#define ENV_TLV_TYPE_END	0x00	
#define ENV_TLV_TYPE_ENV	ENV_CODE_SYS(0,ENV_LENGTH_8BITS)

/*
 * Environment variable flags 
 */

#define ENV_FLG_NORMAL		0x00	/* normal read/write */
#define ENV_FLG_BUILTIN		0x01	/* builtin - not stored in flash */
#define ENV_FLG_READONLY	0x02	/* read-only - cannot be changed */

#define ENV_FLG_MASK		0xFF	/* mask of attributes we keep */
#define ENV_FLG_ADMIN		0x100	/* lets us internally override permissions */

static int nvram_handle = -1;

typedef struct envvar_s {
    queue_t qb;
    int flags;
    char *name;
    char *value;
    /* name and value go here */
} envvar_t;

int nvram_close(void);

/*  *********************************************************************
    *  Globals
    ********************************************************************* */

queue_t env_envvars = {&env_envvars,&env_envvars};
extern unsigned int startflags;


/*  *********************************************************************
    *  nvram_open()
    *  
    *  Open the default NVRAM device and get the information from the
    *  device driver.
    *  
    *  Input parameters: 
    *  	   nothing
    *  	   
    *  Return value:
    *  	   0 if ok
    *  	   else error code
    ********************************************************************* */

int nvram_open(void)
{
    BDBG_MSG(("Enter %s\n", __FUNCTION__));

    if (nvram_handle != -1) {
	nvram_close();
	}

    if ( (nvram_handle = open(nvram_devname, O_RDWR)) < 0) {
	    BDBG_ERR(("cannot open %s\n", nvram_devname));
	    perror("nvram_open");
	    return nvram_handle;
    }
    return 0;
}

/*  *********************************************************************
    *  nvram_close()
    *  
    *  Close the NVRAM device
    *  
    *  Input parameters: 
    *  	   nothing
    *  	   
    *  Return value:
    *  	   0
    ********************************************************************* */

int nvram_close(void)
{
    if (nvram_handle != -1) {
	    close(nvram_handle);
	    nvram_handle = -1;
    }

    return 0;
}


/*  *********************************************************************
    *  nvram_getsize()
    *  
    *  Return the total size of the NVRAM device.  Note that
    *  this is the total size that is used for the NVRAM functions,
    *  not the size of the underlying media.
    *  
    *  Input parameters: 
    *  	   nothing
    *  	   
    *  Return value:
    *  	   size.  <0 if error
    ********************************************************************* */

int nvram_getsize(void)
{
	int size;
	BDBG_MSG(("Enter %s\n", __FUNCTION__));
	if (nvram_handle < 0) return 0;
	/*	size = lseek(nvram_handle, 0, SEEK_END);*/
	size = 0x2000; /* hack it lseek not working !!!*/
	BDBG_MSG(("nvram size %d\n", size));
	return size;
}


/*  *********************************************************************
    *  nvram_read(buffer,offset,length)
    *  
    *  Read data from the NVRAM device
    *  
    *  Input parameters: 
    *  	   buffer - destination buffer
    *  	   offset - offset of data to read
    *  	   length - number of bytes to read
    *  	   
    *  Return value:
    *  	   number of bytes read, or <0 if error occured
    ********************************************************************* */
int nvram_read(unsigned char *buffer,int offset,int length)
{
	int ret = -1;
	BDBG_MSG(("Enter %s\n", __FUNCTION__));
	if (nvram_handle == -1) return -1;

	if ((ret = lseek(nvram_handle, offset, SEEK_SET)) < 0) {
		BDBG_ERR(("%s: lseek error\n", __FUNCTION__));
		goto out;
	} 
	if ((ret = read(nvram_handle, buffer, length)) < 0) {
		BDBG_ERR(("%s: read error\n", __FUNCTION__));
		goto out;
	} 
	/*
	{
		int i;
		char * p = buffer;
		for (i = 0; i < length; i++) {
			BDBG_MSG(("%02x %02x %02x %02x ", p[0], p[1], p[2], p[3]));
			p+=4;
			if ( i%4 == 3 ) BDBG_MSG(("\n"));
		}
	}
	*/
 out: 
	return ret;
}

/*  *********************************************************************
    *  nvram_write(buffer,offset,length)
    *  
    *  Write data to the NVRAM device
    *  
    *  Input parameters: 
    *  	   buffer - source buffer
    *  	   offset - offset of data to write
    *  	   length - number of bytes to write
    *  	   
    *  Return value:
    *  	   number of bytes written, or -1 if error occured
    ********************************************************************* */
int nvram_write(unsigned char *buffer,int offset,int length)
{
	int ret = -1;
	BDBG_MSG(("Enter %s\n", __FUNCTION__));
	if (nvram_handle == -1) return -1;

	if ((ret = lseek(nvram_handle, offset, SEEK_SET)) < 0) {
		BDBG_ERR(("%s: lseek error\n", __FUNCTION__));
		goto out;
	} 
	if ((ret = write(nvram_handle, buffer, length)) < 0) {
		BDBG_ERR(("%s: write error\n", __FUNCTION__));
		goto out;
	} 
 out: 
	return ret;
}


/*  *********************************************************************
    *  nvram_erase()
    *  
    *  Erase the NVRAM device.  Not all devices need to be erased,
    *  but flash memory does.
    *  
    *  Input parameters: 
    *  	   nothing
    *  	   
    *  Return value:
    *  	   0 if ok
    *  	   else error code
    ********************************************************************* */

int nvram_erase(void)
{
	char tmp[256];
	snprintf(tmp, 255, "eraseall %s", nvram_devname);
	return system(tmp);
}



/*  *********************************************************************
    *  env_findenv(name)
    *  
    *  Locate an environment variable in the in-memory list
    *  
    *  Input parameters: 
    *  	   name - name of env var to find
    *  	   
    *  Return value:
    *  	   envvar_t pointer, or NULL if not found
    ********************************************************************* */

static envvar_t *env_findenv(const char *name)
{
    queue_t *qb;
    envvar_t *env;

    for (qb = env_envvars.q_next; qb != &env_envvars; qb = qb->q_next) {
	env = (envvar_t *) qb;
	if (strcmp(env->name,name) == 0) break;
	}

    if (qb == &env_envvars) return NULL;

    return (envvar_t *) qb;

}

/*  *********************************************************************
    *  env_enum(idx,name,namelen,val,vallen)
    *  
    *  Enumerate environment variables.  This routine locates
    *  the nth environment variable and copies its name and value
    *  to user buffers.
    *
    *  The namelen and vallen variables must be preinitialized to 
    *  the maximum size of the output buffer.
    *  
    *  Input parameters: 
    *  	   idx - variable index to find (starting with zero)
    *  	   name,namelen - name buffer and length
    *  	   val,vallen - value buffer and length
    *  	   
    *  Return value:
    *  	   0 if ok
    *  	   else error code
    ********************************************************************* */

static int env_enum(int idx,char *name,int *namelen,char *val,int *vallen)
{
    queue_t *qb;
    envvar_t *env;

    BDBG_MSG(("Enter %s\n", __FUNCTION__));

    for (qb = env_envvars.q_next; qb != &env_envvars; qb = qb->q_next) {
	if (idx == 0) break;
	idx--;
	}

    if (qb == &env_envvars) return -1;
    env = (envvar_t *) qb;

    strncpy(name,env->name,*namelen);
    strncpy(val,env->value,*vallen);
    name[strlen(env->name)] = '\0';
    val[strlen(env->value)] = '\0';
    *namelen = strlen(env->name);
    *vallen = strlen(env->value);

    return 0;

}

/*  *********************************************************************
    *  env_envtype(name)
    *  
    *  Return the type of the environment variable
    *  
    *  Input parameters: 
    *  	   name - name of environment variable
    *  	   
    *  Return value:
    *  	   flags, or <0 if error occured
    ********************************************************************* */
static int env_envtype(const char *name)
{
    envvar_t *env;

    env = env_findenv(name);

    if (env) {
	return env->flags;
	}

    return -1;
}



/*  *********************************************************************
    *  env_delenv(name)
    *  
    *  Delete an environment variable
    *  
    *  Input parameters: 
    *  	   name - environment variable to delete
    *  	   
    *  Return value:
    *  	   0 if ok
    *  	   else error code
    ********************************************************************* */

static int env_delenv(const char *name)
{
    envvar_t *env;

    env = env_findenv(name);

    if (!env) return 0;

    if (!(env->flags & ENV_FLG_READONLY)) {
	q_dequeue((queue_t *) env);
	free(env);
	return 0;
	}

    return -1;
}

/*  *********************************************************************
    *  env_getenv(name)
    *  
    *  Retrieve the value of an environment variable
    *  
    *  Input parameters: 
    *  	   name - name of environment variable to find
    *  	   
    *  Return value:
    *  	   value, or NULL if variable is not found
    ********************************************************************* */

static char *env_getenv(const char *name)
{
    envvar_t *env;

    env = env_findenv(name);

    if (env) {
	return env->value;
	}

    return NULL;
}


/*  *********************************************************************
    *  env_setenv(name,value,flags)
    *  
    *  Set the value of an environment variable
    *  
    *  Input parameters: 
    *  	   name - name of variable
    *  	   value - value of variable
    *  	   flags - flags for variable (ENV_FLG_xxx)
    *  	   
    *  Return value:
    *  	   0 if ok
    *  	   else error code
    ********************************************************************* */

static int env_setenv(const char *name,char *value,int flags)
{
    envvar_t *env;
    int namelen, vallen;

    BDBG_MSG(("Enter %s\n", __FUNCTION__));

    env = env_findenv(name);
    if (env) {
	if (!(flags & ENV_FLG_ADMIN)) {
	    if (env->flags & ENV_FLG_READONLY) return -1;
	    }
	q_dequeue((queue_t *) env);
	free(env);
	}

    namelen = strlen(name);

    env = malloc(sizeof(envvar_t) + namelen + 1 + strlen(value) + 1);
    if (!env) return -1;

    env->name = (char *) (env+1);
    env->value = env->name + namelen + 1;
    env->flags = (flags & ENV_FLG_MASK);

    strncpy(env->name,name, namelen);
    env->name[namelen] = '\0';
    vallen = strlen(value);
    strncpy(env->value,value, vallen);
    env->value[vallen] = '\0';
    BDBG_MSG(("%s: name %s, = %s\n", __FUNCTION__,  env->name, env->value));
    q_enqueue(&env_envvars,(queue_t *) env);

    return 0;
}


/*  *********************************************************************
    *  env_load()
    *  
    *  Load the environment from the NVRAM device.
    *  
    *  Input parameters: 
    *  	   nothing
    *  	   
    *  Return value:
    *  	   0 if ok
    *  	   else error code
    ********************************************************************* */

#if 1
/* 
 * instead of reading by bytes, read the whole block 
 * For NOR, byte-by-byte read is inefficient
 * For NAND, byte-by-byte read is prohibited.
 */
int env_load(void)
{
    int size;
    unsigned char *buffer;
    unsigned char *ptr;
    unsigned char *envval;
    unsigned int reclen;
    unsigned int rectype;
    int offset;
    int flg;
    int retval = -1;
    char valuestr[256];

    flg = nvram_open();
    if (flg < 0) return flg;
    
    size = nvram_getsize();
    buffer = malloc(size);

    if (buffer == NULL) return -1;
    
    ptr = buffer;
    offset = 0;
    
    /* Read the record type and length */
    if (nvram_read(ptr,offset,size) != size) {
        retval = -1;
        goto error;
	}

    while ((*ptr != ENV_TLV_TYPE_END)  && (size > 1)) {

        /* Adjust pointer for TLV type */
        rectype = *(ptr);
        offset++;
        size--;
        
        /* 
         * Read the length.  It can be either 1 or 2 bytes
         * depending on the code 
         */
        if (rectype & ENV_LENGTH_8BITS) {
            /* Read the record type and length - 8 bits */
            ptr = buffer + offset;
            reclen = *(ptr);
            size--;
            offset++;
	    }
        else {
            /* Read the record type and length - 16 bits, MSB first */
            ptr = buffer + offset;
            reclen = (((unsigned int) *(ptr)) << 8) + (unsigned int) *(ptr+1);
            size -= 2;
            offset += 2;
	    }
        
        if (reclen > size) break;	/* should not happen, bad NVRAM */
        
        switch (rectype) {
	    case ENV_TLV_TYPE_ENV:
            /* Read the TLV data */
            ptr = buffer + offset;
            flg = *ptr++;
            envval = (unsigned char *) strchr((char *)ptr,'=');
            if (envval) {
                *envval++ = '\0';
                memcpy(valuestr,envval,(reclen-1)-(envval-ptr));
                valuestr[(reclen-1)-(envval-ptr)] = '\0';
                env_setenv(ptr,valuestr,flg);
		    }
            break;
            
	    default: 
            /* Unknown TLV type, skip it. */
            break;
	    }
        
        /*
         * Advance to next TLV 
         */
		
        size -= (int)reclen;
        offset += reclen;
        
        /* Read the next record type */
        ptr = buffer + offset;
	}
		
    retval = 0;		/* success! */
    
 error:
    free(buffer);
    nvram_close();

    return retval;
}
#else
static int env_load(void)
{
    int size;
    unsigned char *buffer;
    unsigned char *ptr;
    unsigned char *envval;
    unsigned int reclen;
    unsigned int rectype;
    int offset;
    int flg;
    int retval = -1;
    char valuestr[256];

    flg = nvram_open();
    if (flg < 0) return flg;

    size = nvram_getsize();
    buffer = malloc(size);

    if (buffer == NULL) return -1;

    ptr = buffer;
    offset = 0;

    /* Read the record type and length */
    if (nvram_read(ptr,offset,1) != 1) {
	retval = -1;
	goto error;
	}

    while ((*ptr != ENV_TLV_TYPE_END)  && (size > 1)) {

	/* Adjust pointer for TLV type */
	rectype = *(ptr);
	offset++;
	size--;

	/* 
	 * Read the length.  It can be either 1 or 2 bytes
	 * depending on the code 
	 */
	if (rectype & ENV_LENGTH_8BITS) {
	    /* Read the record type and length - 8 bits */
	    if (nvram_read(ptr,offset,1) != 1) {
		retval = -1;
		goto error;
		}
	    reclen = *(ptr);
	    size--;
	    offset++;
	    }
	else {
	    /* Read the record type and length - 16 bits, MSB first */
	    if (nvram_read(ptr,offset,2) != 2) {
		retval = -1;
		goto error;
		}
	    reclen = (((unsigned int) *(ptr)) << 8) + (unsigned int) *(ptr+1);
	    size -= 2;
	    offset += 2;
	    }

	if ((int)reclen > size) break;	/* should not happen, bad NVRAM */

	switch (rectype) {
	    case ENV_TLV_TYPE_ENV:
		/* Read the TLV data */
		if (nvram_read(ptr,offset,reclen) != (int) reclen) goto error;
		flg = *ptr++;
		envval = (unsigned char *) strchr((char *)ptr,'=');
		if (envval) {
		    *envval++ = '\0';
		    memcpy(valuestr,envval,(reclen-1)-(envval-ptr));
		    valuestr[(reclen-1)-(envval-ptr)] = '\0';
		    env_setenv((char *)ptr,valuestr,flg);
		    }
		break;
		
	    default: 
		/* Unknown TLV type, skip it. */
		break;
	    }

	/*
	 * Advance to next TLV 
	 */
		
	size -= (int)reclen;
	offset += reclen;

	/* Read the next record type */
	ptr = buffer;
	if (nvram_read(ptr,offset,1) != 1) goto error;
	}

    retval = 0;		/* success! */

error:
    free(buffer);
    nvram_close();

    return retval;

}
#endif

/*  *********************************************************************
    *  env_save()
    *  
    *  Write the environment to the NVRAM device.
    *  
    *  Input parameters: 
    *  	   nothing
    *  	   
    *  Return value:
    *  	   0 if ok, else error code
    ********************************************************************* */

static int env_save(void)
{
    int size;
    unsigned char *buffer;
    unsigned char *buffer_end;
    unsigned char *ptr;
    queue_t *qb;
    envvar_t *env;
    int namelen;
    int valuelen;
    int flg;
	int i, len, padding_length; /* For the Nand Flash */


    nvram_erase();

    flg = nvram_open();
    if (flg < 0) return flg;

    size = nvram_getsize();
    buffer = malloc(size);

    if (buffer == NULL) return -1;

    buffer_end = buffer + size;

    ptr = buffer;

    for (qb = env_envvars.q_next; qb != &env_envvars; qb = qb->q_next) {
	env = (envvar_t *) qb;

	if (env->flags & (ENV_FLG_BUILTIN)) continue;

	namelen = strlen(env->name);
	valuelen = strlen(env->value);

	if ((ptr + 2 + namelen + valuelen + 1 + 1 + 1) > buffer_end) break;

	*ptr++ = ENV_TLV_TYPE_ENV;		/* TLV record type */
	*ptr++ = (namelen + valuelen + 1 + 1);	/* TLV record length */

	*ptr++ = (unsigned char)env->flags;
	memcpy(ptr,env->name,namelen);		/* TLV record data */
	ptr += namelen;
	*ptr++ = '=';
	memcpy(ptr,env->value,valuelen);
	ptr += valuelen;

	}

    *ptr++ = ENV_TLV_TYPE_END;
/* For the NAND FLASH.The Nand Flash requires padding of bytes = 0xff to fill the buffer of length = page size
   This is the requirement of the Nand Flash.
 */
	len = ptr-buffer;
	if(len < NAND_PAGE_SIZE) {
		padding_length = NAND_PAGE_SIZE - len;
		for(i=0; i<padding_length; i++) {
			buffer[i+len] = 0xff;
		}
		len = NAND_PAGE_SIZE;
	}
/* For the Nand Flash */
    size = nvram_write(buffer,0,len);

    free(buffer);

    nvram_close();

    return (size == (ptr-buffer)) ? 0 : -1;
}



int cdl_bootinof_nvram_setenv(char * varname, char * val) {
	int ret = -1;
	int roflag = ENV_FLG_NORMAL;
	if ((ret = env_setenv(varname,val,roflag))) {
		BDBG_ERR(("Could not set environment variable %s" , varname));
		goto out;
	}
	else {
		env_save();
	}
	ret = 0;
 out:
	return ret;
}

int cdl_bootinof_nvram_getenv(char * name, char * val) {
	char varname[80];
	char value[256];
	int varlen,vallen;
	int idx;

	idx = 0;
	for (;;) {
		varlen = sizeof(varname);
		vallen = sizeof(value);
		if (env_enum(idx,varname,&varlen,value,&vallen) < 0) break;
		if (!strncmp(name, varname, varlen)) {
			strncpy(val, value, vallen);
			return 0;
		}
		idx++;
	}

	return -1;
}

int cdl_bootinof_nvram_printenv(void)
{
    char varname[80];
    char value[256];
    int varlen,vallen;
    int idx;

    printf("Variable Name        Value\n");
    printf("-------------------- --------------------------------------------------\n");

    idx = 0;
    for (;;) {
	varlen = sizeof(varname);
	vallen = sizeof(value);
	if (env_enum(idx,varname,&varlen,value,&vallen) < 0) break;
	printf("%20s %s\n",varname,value);
	idx++;
	}

    return 0;
    
}


/* return < 0 if fail, >= 0 if succeed */

int cdl_bootinfo_nvram_ready_to_reboot() {
	char startup[1024];

	if (getenv("STARTUP")== NULL) {
		printf("STARTUP environment variable not set\n");
		snprintf(startup, 1024, "expand -forceerase %s", storage_file_final_cfe);
	} else {
		snprintf(startup, 1024, "expand -forceerase %s;%s", 
			 storage_file_final_cfe, getenv("STARTUP"));
	}
	BDBG_MSG(("Set CFE STARTUP variable: %s\n", startup));
	if ( cdl_bootinof_nvram_setenv("STARTUP", startup) < 0 ) {
		printf("Cannot set nvram\n");
		return -1;
	}
	
	if ( cdl_bootinof_nvram_setenv("CDL_STATE", "REBOOT0") < 0 ) {
		BDBG_ERR(("Cannot set nvram\n"));
		return -1;
	}
#ifdef DSG_SUPPORT
	cdl_ctl_set_state(CDL_CTL_ST_REBOOT_0);
#endif
	return 1;
}

/* return < 0 if fail, >= 0 if succeed */

int cdl_bootinfo_nvram_just_started() {
	char val[MAX_DEV_STR_LEN];
	char * str;

	if (cdl_bootinof_nvram_getenv("CDL_STATE", val)>= 0) {
		if (!strncmp(val, "SUCCEED", MAX_DEV_STR_LEN)) {
			BDBG_ERR(("\n\n!!!!!!!!!!!!!! Last common download  succeeded !!!!!!!!!!!!!!!\n"));
#ifdef DSG_SUPPORT
			cdl_ctl_set_state(CDL_CTL_ST_INIT);
#endif
			if ( cdl_bootinof_nvram_setenv("CDL_STATE", "INIT") < 0 ) {
				printf("Cannot set nvram\n");
				return -1;
			}
			if (cdl_bootinof_nvram_getenv("NEW_IMG", val)>= 0) {
				if ( cdl_bootinof_nvram_setenv("VALID_IMG", val) < 0 ) {
					BDBG_ERR(("Cannot set nvram\n"));
					return -1;
				}
				/* chage NEW_IMG location */
				if (strncmp(val, "flash0.image0", MAX_DEV_STR_LEN) ==0) {
					cdl_bootinof_nvram_setenv("NEW_IMG", "flash0.image1");
				} else if (strncmp(val, "flash0.image1", MAX_DEV_STR_LEN) ==0) {
					cdl_bootinof_nvram_setenv("NEW_IMG", "flash0.image0");
				}
			} else {
				BDBG_ERR(("Cannot get NEW_IMG"));
				return -1;
			}
		} else if (!strncmp(val, "RESTORE", MAX_DEV_STR_LEN)) {
			BDBG_ERR(("\n\n!!!!!!!!!!! Download failed: Reboot exceed 2. Restore image  !!!!!!!!!!!\n"));
#ifdef DSG_SUPPORT
			cdl_ctl_set_state(CDL_CTL_ST_RESTORE);
#endif
#if defined(MPOD_SUPPORT) 
			cdl_adpu_send_host_download_control(B_MPOD_DL_REBOOT_MAX_RETRY);
#endif
			if ( cdl_bootinof_nvram_setenv("CDL_STATE", "INIT") < 0 ) {
				printf("Cannot set nvram\n");
				return -1;
			}			
		} else {
			BDBG_ERR(("CDL_STATE %s", val));
		}
		
	}
	/*
	 * find the flash partition for downloaded image
	 * for > 32M flash, there are 2 locations for images, (/dev/mtdocap5 in kernel, 
	 * or flash0.image0 in cfe ) and (/dev/mtdocap6 in kernel, or flash0.image1 in 
	 * cfe), one is used to save the current good image, another one
	 *  is for the new image.
	 * for 32M flash, the only partition that can be used is /dev/mtd0, or flash0.avail0.
	 * By default, the flash0.avail0 is used. 
	 * To use flash0.image0 and flash0.image1, set envirnment variable on the linux
	 *  export cdl_new_img=flash0.image0
	 */

	strncpy(storage_file_final, "/dev/mtd0", (MAX_DEV_STR_LEN - 1));
	strncpy(storage_file_final_cfe, "flash0.avail0", (MAX_DEV_STR_LEN - 1));
	if ((str = getenv(CDL_ENV_NEW_IMG))) {
		if (strncmp(str, "flash0.image0", (MAX_DEV_STR_LEN - 1)) ==0) {
			strncpy(storage_file_final, "/dev/mtdocap5", (MAX_DEV_STR_LEN - 1));
			strncpy(storage_file_final_cfe, str, (MAX_DEV_STR_LEN - 1));
		} else if (strncmp(str, "flash0.image1", (MAX_DEV_STR_LEN - 1)) ==0) {
			strncpy(storage_file_final, "/dev/mtdocap6", (MAX_DEV_STR_LEN - 1));
			strncpy(storage_file_final_cfe, str, (MAX_DEV_STR_LEN - 1));
		} else {
			BDBG_WRN(("Use default NEW_IMG %s", storage_file_final_cfe));
			cdl_bootinof_nvram_setenv("NEW_IMG", storage_file_final_cfe);
		}
		cdl_bootinof_nvram_setenv("NEW_IMG", storage_file_final_cfe);
	} else if (cdl_bootinof_nvram_getenv("NEW_IMG", val)>= 0) {
		if (strncmp(val, "flash0.image0", (MAX_DEV_STR_LEN - 1)) ==0) {
			strncpy(storage_file_final, "/dev/mtdocap5", (MAX_DEV_STR_LEN - 1));
			strncpy(storage_file_final_cfe, val, (MAX_DEV_STR_LEN - 1));
		} else if (strncmp(val, "flash0.image1", (MAX_DEV_STR_LEN - 1)) ==0) {
			strncpy(storage_file_final, "/dev/mtdocap6", (MAX_DEV_STR_LEN - 1));
			strncpy(storage_file_final_cfe, val, (MAX_DEV_STR_LEN - 1));
		} else {
			BDBG_WRN(("Use default NEW_IMG %s", storage_file_final_cfe));
			cdl_bootinof_nvram_setenv("NEW_IMG", storage_file_final_cfe);
		}
	} else {
		BDBG_WRN(("Use default NEW_IMG %s", storage_file_final_cfe));
		cdl_bootinof_nvram_setenv("NEW_IMG", storage_file_final_cfe);
	}

	BDBG_MSG(("storage_file = %s, storage_file_cfe = %s", storage_file_final, storage_file_final_cfe));

	return 1;
}

int cdl_bootinfo_nvram_get_new_storage(char * fn_kernel, char * fn_bootloader){
	strncpy(fn_kernel, storage_file_final, (MAX_DEV_STR_LEN - 1));
	strncpy(fn_bootloader, storage_file_final_cfe, (MAX_DEV_STR_LEN - 1));
	return 1;
}

char * cdl_bootinfo_nvram_get_dev_name(void) {
	/*FIXME! the device name should be detected automatically */ 
	return nvram_devname;
}
int cdl_bootinfo_nvram_open(char *dev_name) {
    int ret;
	strncpy(nvram_devname, dev_name, MAX_DEV_STR_LEN - 1);
	printf("nvram_Devnamd %s\n", nvram_devname);

	ret = env_load();
	return ret;
}
int cdl_bootinfo_nvram_close() {
	return 0;
}

