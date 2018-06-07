/***************************************************************************
 *  Copyright (C) 2007-2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
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
 *******************************************************************************/
#include "bstd.h"
#include "bmkv_util.h"
#include "bkni.h"
#include "biobits.h"

BDBG_MODULE(bmkv_util);

#define BDBG_MSG_TRACE(x)   /* BDBG_MSG(x) */

int64_t
bmkv_parse_signed64(batom_cursor *cursor)
{
	int64_t result=0;
	int c;

#if B_ATOM_FAST
	if(cursor->left > 3) { /* 3 bytes size */
		do {
			unsigned word = cursor->cursor[0];
		    if(word&0x80) {
				word = word&0x7F;
				result = (1<<6)-1;
				c=1;
			} else if(word&0x40) {
	            word = ((word  & 0x3f)<<8) | cursor->cursor[1];
				result = (1<<(6+7))-1;
				c=2;
			} else {
				break; /* slow path */
			}
			result = ((int64_t)word) - result;
			cursor->left -= c;
			cursor->cursor += c;
			goto  done;
		} while(false);
	}
    BDBG_MSG_TRACE(("bmkv_parse_signed64: %#lx slow path", (unsigned long)cursor));
#endif
    c=batom_cursor_next(cursor);
	if(c>0) {
		unsigned shift;
    	BDBG_CASSERT(BATOM_EOF<0);
		for(shift=0;;shift++) {
			if((unsigned)c&(0x80>>shift)) {
				c = ((unsigned)c)&(0x7F>>shift);
				result |= c << (shift*8);
				result -= (1<<(6+(shift*7)))-1;
				break;
			}
			result = (result<<8) | batom_cursor_byte(cursor);
		}
	} else {
		BDBG_WRN(("bmkv_parse_signed64: %#lx invalid unsigned, forcing EOS", (unsigned long)cursor));
		batom_cursor_skip(cursor, batom_cursor_size(cursor)+1);
	}
done:
    BDBG_MSG_TRACE(("bmkv_parse_signed64:%#lx %lld(%#llx)", (unsigned long)cursor, result, result));
    return result;
}

/* Matroska Specifications http://www.matroska.org/technical/specs/index.html */
uint64_t
bmkv_parse_unsigned64(batom_cursor *cursor)
{
	uint64_t result=0;
	int c;
/*
Data size, in octets, is also coded with an UTF-8 like system :
 bits, big-endian
 1xxx xxxx                                                                              - value 0 to  2^7-2
 01xx xxxx  xxxx xxxx                                                                   - value 0 to 2^14-2
 001x xxxx  xxxx xxxx  xxxx xxxx                                                        - value 0 to 2^21-2
 0001 xxxx  xxxx xxxx  xxxx xxxx  xxxx xxxx                                             - value 0 to 2^28-2
 0000 1xxx  xxxx xxxx  xxxx xxxx  xxxx xxxx  xxxx xxxx                                  - value 0 to 2^35-2
 0000 01xx  xxxx xxxx  xxxx xxxx  xxxx xxxx  xxxx xxxx  xxxx xxxx                       - value 0 to 2^42-2
 0000 001x  xxxx xxxx  xxxx xxxx  xxxx xxxx  xxxx xxxx  xxxx xxxx  xxxx xxxx            - value 0 to 2^49-2
 0000 0001  xxxx xxxx  xxxx xxxx  xxxx xxxx  xxxx xxxx  xxxx xxxx  xxxx xxxx  xxxx xxxx - value 0 to 2^56-2
*/
#if B_ATOM_FAST
	if(cursor->left > 3) { /* 3 bytes size */
		do {
			unsigned word = cursor->cursor[0];
		    if(word&0x80) {
				word = word&0x7F;
				c=1;
			} else if(word&0x40) {
	            word = ((word  & 0x3f)<<8) | cursor->cursor[1];
				c=2;
			} else if(word&0x20) {
	            word = ((word  & 0x1f)<<16) | ((unsigned)cursor->cursor[1]<<8) |  cursor->cursor[2];
				c=3;
			} else {
				break; /* slow path */
			}
			result = word;
			cursor->left -= c;
			cursor->cursor += c;
			goto  done;
		} while(false);
	}
    BDBG_MSG_TRACE(("bmkv_parse_unsigned64: %#lx slow path", (unsigned long)cursor));
#endif
    c=batom_cursor_next(cursor);
	if(c>0) {
		unsigned shift;
    	BDBG_CASSERT(BATOM_EOF<0);
		for(shift=0;;shift++) {
			if((unsigned)c&(0x80>>shift)) {
				c = ((unsigned)c)&(0x7F>>shift);
				result |= c << (shift*8);
				break;
			}
			result = (result<<8) | batom_cursor_byte(cursor);
		}
	} else {
		BDBG_WRN(("bmkv_parse_unsigned64: %#lx invalid unsigned, forcing EOS", (unsigned long)cursor));
		batom_cursor_skip(cursor, batom_cursor_size(cursor)+1);
	}
done:
    BDBG_MSG_TRACE(("bmkv_parse_unsigned64:%#lx %llu(%#llx)", (unsigned long)cursor, result, result));
    return result;
}

bool
bmkv_parse_header(batom_cursor *cursor, bmkv_header *header)
{
    int c;
    uint32_t word;
    uint64_t reserved_size;
	unsigned i;

#if B_ATOM_FAST
	if(cursor->left > 5) { /* 2 bytes id and 3 bytes size */
		do {
			const uint8_t *data = cursor->cursor;
			word = data[0];
		    if(word&0x80) {
				header->id = word;
			} else if(word&0x40) {
				header->id = (word<<8) | data[1];
				data++;
			} else { /* slow path */
				break;
			}
			word = data[1];
		    if(word&0x80) {
				word = word&0x7F;
				if(word==0x7F) {
					break; /* slow path */
				}
			} else if(word&0x40) {
	            word = ((word & 0x3F)<<8) | data[2];
				data++;
				if(word==0x3FFF) {
					break; /* slow path */
				}
			} else if(word&0x20) {
	            word = ((word & 0x1F)<<16) | (((unsigned)data[2])<<8) | data[3];
				data+=2;
				if(word==0x1FFFFF) {
					break; /* slow path */
				}
			} else {
				break; /* slow path */
			}
			data+=2;
			header->size = word;
			cursor->left -= data - cursor->cursor;
			cursor->cursor = data;
			goto  done;
		} while(false);
	}
    BDBG_MSG_TRACE(("bmkv_parse_header: %#lx slow path", (unsigned long)cursor));
#endif /*  B_ATOM_FAST */


/*
Element ID coded with an UTF-8 like system :

bits, big-endian
1xxx xxxx                                  - Class A IDs (2^7 -1 possible values) (base 0x8X)
01xx xxxx  xxxx xxxx                       - Class B IDs (2^14-1 possible values) (base 0x4X 0xXX)
001x xxxx  xxxx xxxx  xxxx xxxx            - Class C IDs (2^21-1 possible values) (base 0x2X 0xXX 0xXX)
0001 xxxx  xxxx xxxx  xxxx xxxx  xxxx xxxx - Class D IDs (2^28-1 possible values) (base 0x1X 0xXX 0xXX 0xXX)
Some Notes:
    * The leading bits of the Class IDs are used to identify the length of the ID. The number of leading 0's + 1 is the length of the ID in octets. We will refer to the leading bits as the Length Descriptor.
    * Any ID where all x's are composed entirely of 1's is a Reserved ID, thus the -1 in the definitions above.
    * The Reserved IDs (all x set to 1) are the only IDs that may change the Length Descriptor.
*/

    c=batom_cursor_next(cursor);
	if(c>0) {
    	BDBG_CASSERT(BATOM_EOF<0);
		for(word=c,i=0;;i++) {
			if((unsigned)c&(0x80>>i)) {
				break;
			}
			word = (word<<8)|batom_cursor_byte(cursor);
		}
		header->id = word;
	} else {
        if(c==BATOM_EOF) { goto eos;}
        BDBG_MSG(("bmkv_parse_header: %#lx invalid ID", (unsigned long)cursor));
        goto error;
    }
	i=batom_cursor_pos(cursor);
	header->size = bmkv_parse_unsigned64(cursor);
	if(BATOM_IS_EOF(cursor)) { goto eos;}
	i = batom_cursor_pos(cursor)-i;
    if( i*7 <= 64 && i!=0 ) {
        reserved_size = ((uint64_t)(-1)) >> (64-i*7);
    } else {
        reserved_size = 0;
    }
	if( header->size == reserved_size) {
		BDBG_MSG(("bmkv_parse_header: %#lx reserved block length", (unsigned long)cursor));
		header->size = BMKV_RESERVED_SIZE;
	}

done:
    BDBG_MSG_TRACE(("bmkv_parse_header: %#lx id:%#x size:%u(%#llx)", (unsigned long)cursor, (unsigned)header->id, (unsigned)header->size,(uint64_t)header->size));
    return true;
error:
    header->id = BMKV_INVALID_ID;
    header->size = 0;
    return true;
eos:
    BDBG_MSG_TRACE(("bmkv_parse_header: %#lx EOS", (unsigned long)cursor));
    header->size = 0;
    header->id = 0;
    return false;
}

static uint64_t
bmkv_read_unsigned(batom_cursor *cursor, size_t size)
{
    uint64_t result;

#if B_ATOM_FAST
	if(cursor->left > 4) {
		if(size==1) {
			result = cursor->cursor[0];
		} else if(size==2) {
			result = (((uint32_t)cursor->cursor[0])<<8) | cursor->cursor[1];
		} else if(size==3) {
			result = (((uint32_t)cursor->cursor[0])<<16) | (((uint32_t)cursor->cursor[1])<<8) | cursor->cursor[2];
		} else if(size==4) {
			result = (((uint32_t)cursor->cursor[0])<<24) | (((uint32_t)cursor->cursor[1])<<16) | (((uint32_t)cursor->cursor[2])<<8) | cursor->cursor[3];
		} else {
			goto slow;
		}
		cursor->cursor += size;
		cursor->left -= size;
		goto done;
	}
slow:
    BDBG_MSG_TRACE(("bmkv_read_unsigned: %#lx slow path", (unsigned long)cursor));
#endif
    for(result=0;size>0;size--) {
        int c;
        c = batom_cursor_next(cursor);
        result = (result <<8) | ((uint8_t)c);
    }
done:
    BDBG_MSG_TRACE(("bmkv_read_unsigned:%#lx %llu(%#llx)", (unsigned long)cursor, result, result));
    return result;
}

int64_t
bmkv_read_signed(batom_cursor *cursor, size_t size)
{
    int64_t result;
	unsigned bit;

    for(result=0,bit=0;bit<size;bit++) {
        int c;
        BATOM_NEXT(c, cursor);
		result <<=8;
		if(bit==0) {
			result |= (int8_t)c;
		} else {
			result |= (uint8_t)c;
		}
    }
    BDBG_MSG_TRACE(("bmkv_read_signed: %#lx %lld",(unsigned long)cursor, result));
	return result;
}

static bool
bmkv_element_parse_from_desc_body(batom_cursor *cursor, const bmkv_parser_desc *desc, size_t elem_size, void *elem)
{
    if(desc->init) {
        desc->init(elem);
    } else {
        BKNI_Memset(elem, 0, desc->size);
    }

    if(!desc->entries[0].id){
        uint64_t udata;
        const bmkv_parser_entry *entry = desc->entries;
        void *data = (void*)((uint8_t *)(elem) + entry->elem_off);

        BDBG_ASSERT(elem_size<=8);
        udata = bmkv_read_unsigned(cursor, elem_size);
        if(entry->type!=bmkv_parser_entry_type_unsigned64) {
            if(entry->type!=bmkv_parser_entry_type_bool) {
                if(udata<0xFFFFFFFFul) {
                    if(entry->type==bmkv_parser_entry_type_unsigned) {
                        *(unsigned *)data = (uint32_t)udata;
                    } else { /* id */
                        *(bmkv_id*)data = (bmkv_id)udata;
                    }
                } else {
                    BDBG_WRN(("%s:%#lx %s overflow in value of u-integer element %s:" BDBG_UINT64_FMT, "bmkv_element_parse", (unsigned long)cursor, desc->bmkv_parser_desc_name, entry->name, BDBG_UINT64_ARG(udata)));
                    return false;
                }
            } else {
                if(udata==0) {
                    *(bool *)data = false;
                } else if(udata==1) {
                    *(bool *)data = true;
                } else {
                    BDBG_WRN(("%s:%#lx %s invalid value of bool element %s:" BDBG_UINT64_FMT, "bmkv_element_parse", (unsigned long)cursor, desc->bmkv_parser_desc_name, entry->name, BDBG_UINT64_ARG(udata)));
                    return false;
                }
            }
        } else {
            *(uint64_t *)data = udata;
        }
        *(bool*)((uint8_t *)(elem) + entry->validate_off) = true;
        return true;
    }

    return bmkv_element_parse(cursor, elem_size, desc->entries, desc->bmkv_parser_desc_name, elem);
}


static bool
bmkv_table_parse(batom_cursor *cursor, size_t elem_size, bmkv_table *table, const bmkv_parser_desc *desc)
{
	batom_checkpoint checkpoint;
	void *data;
	if(table->bmkv_table_alloc_nelems <= table->nelems) {
		unsigned alloc_nelems=(table->bmkv_table_alloc_nelems*3)/2+1;
		data = BKNI_Malloc(desc->size*alloc_nelems);
		BDBG_MSG_TRACE(("bmkv_table_parse: %#lx alloc %#lx->%#lx %u->%u(%u)", (unsigned long)cursor, (unsigned long)table->data, (unsigned long)data, table->bmkv_table_alloc_nelems, alloc_nelems, alloc_nelems*desc->size));
		BDBG_ASSERT(alloc_nelems > table->nelems); /* after rellocation there is shall be enough space */
		if(!data) {
			return false;
		}
		if(table->nelems) {
			BDBG_ASSERT(table->data);
			BKNI_Memcpy(data, table->data, table->nelems*desc->size);
			BKNI_Free(table->data);
		}
		table->data = data;
		table->bmkv_table_alloc_nelems = alloc_nelems;
	}
	data = (uint8_t *)table->data + table->nelems*desc->size;
	BDBG_MSG_TRACE(("bmkv_table_parse: %#lx parsing %#lx:%#lx(%u))", (unsigned long)cursor, (unsigned long)data, (unsigned long)table->data, table->nelems));
	table->nelems++;
	BATOM_SAVE(cursor, &checkpoint);
	if(bmkv_element_parse_from_desc_body(cursor, desc, elem_size, data)) {
		return true;
	}
	batom_cursor_rollback(cursor, &checkpoint);
	batom_cursor_skip(cursor, elem_size);
	return false;
}

static void
bmkv_table_shutdown(bmkv_table *table, const bmkv_parser_desc *desc)
{
	unsigned i;
	void *data;

	for(data=table->data,i=0; i<table->nelems; i++, data = (uint8_t *)data + desc->size) {
		bmkv_element_shutdown(desc->entries, data);
	}
	if(table->data) {
		BKNI_Free(table->data);
	}
	table->data = NULL;
	table->bmkv_table_alloc_nelems = 0;
	table->nelems = 0;
	return;
}

#define B_MKV_PRINT(level, format) do { if(level==BDBG_eMsg) {BDBG_MSG(format);} else if(level==BDBG_eWrn) { BDBG_WRN(format);} else if(level==BDBG_eErr) { BDBG_ERR(format);} else if(level==BDBG_eLog) {BDBG_LOG(format);}}while(0)

static void
bmkv_table_print(const bmkv_table *table, BDBG_Level level, unsigned padding, const bmkv_parser_desc *desc)
{
	unsigned i;
	const void *data;

	for(data=table->data,i=0; i<table->nelems; i++, data = (uint8_t *)data + desc->size) {
		if(table->nelems>1) {
			B_MKV_PRINT(level, ("%*c %s[%u]", (int)padding, ' ', desc->bmkv_parser_desc_name, i));
		}
		bmkv_element_print(desc->entries, level, padding+4, desc->bmkv_parser_desc_name, data);
	}
	return;
}

bool
bmkv_element_parse(batom_cursor *cursor, size_t elem_size, const bmkv_parser_entry *entries, const char *elem_name, void *elem)
{
    const bmkv_parser_entry *entry;
    size_t nentries;
	unsigned i;
    BDBG_ASSERT(elem_name);
    BDBG_ASSERT(elem);
    BDBG_ASSERT(cursor);
    BDBG_ASSERT(entries);

	BSTD_UNUSED(elem_name);

    BDBG_MSG_TRACE(("%s:%#lx pos:%u elem_size:%u", "bmkv_element_parse", (unsigned long)cursor, batom_cursor_pos(cursor), elem_size));
    elem_size += batom_cursor_pos(cursor); /* convert size to the offset */

    /* count number elements in the table and validate table elements */
    for(nentries=0,entry=entries;entry->type!=bmkv_parser_entry_type_last;nentries++,entry++) {
        if(entry->validate_size != sizeof(bool)) {
            BDBG_ERR(("%s:%p %s invalid size of validate '%s' %u[%u]", "bmkv_element_parse", (void *)cursor, elem_name, entry->name, entry->validate_size, (unsigned)sizeof(bool)));
        }
        switch(entry->type) {
        case bmkv_parser_entry_type_unsigned:
            if(entry->elem_size!=sizeof(unsigned)) {
                BDBG_ERR(("%s:%p %s invalid size of u-integer element '%s' %u[%u]", "bmkv_element_parse", (void *)cursor, elem_name, entry->name, entry->elem_size, (unsigned)sizeof(unsigned)));
                goto err_elem_size;
            }
            break;
        case bmkv_parser_entry_type_id:
            if(entry->elem_size!=sizeof(bmkv_id)) {
                BDBG_ERR(("%s:%p %s invalid size of id element '%s' %u[%u]", "bmkv_element_parse", (void *)cursor, elem_name, entry->name, entry->elem_size, (unsigned)sizeof(bmkv_id)));
                goto err_elem_size;
            }
            break;
        case bmkv_parser_entry_type_unsigned64:
            if(entry->elem_size!=sizeof(uint64_t)) {
                BDBG_ERR(("%s:%p %s invalid size of u-integer64 element '%s' %u[%u]", "bmkv_element_parse", (void *)cursor, elem_name, entry->name, entry->elem_size, (unsigned)sizeof(uint64_t)));
                goto err_elem_size;
            }
            break;
        case bmkv_parser_entry_type_signed:
            if(entry->elem_size!=sizeof(signed)) {
                BDBG_ERR(("%s:%p %s invalid size of s-integer element '%s' %u[%u]", "bmkv_element_parse", (void *)cursor, elem_name, entry->name, entry->elem_size, (unsigned)sizeof(signed)));
                goto err_elem_size;
            }
            break;
        case bmkv_parser_entry_type_unique_id:
            if(entry->elem_size!= sizeof(bmkv_unique_id)) {
                BDBG_ERR(("%s:%p %s invalid size of unique-ID element '%s' %u[%u]", "bmkv_element_parse", (void *)cursor, elem_name, entry->name, entry->elem_size, (unsigned)sizeof(bmkv_unique_id)));
                goto err_elem_size;
            }
            break;
        case bmkv_parser_entry_type_date:
            if(entry->elem_size!= sizeof(bmkv_date)) {
                BDBG_ERR(("%s:%p %s invalid size of data element '%s' %u[%u]", "bmkv_element_parse", (void *)cursor, elem_name, entry->name, entry->elem_size, (unsigned)sizeof(bmkv_date)));
                goto err_elem_size;
            }
            break;
        case bmkv_parser_entry_type_float:
            if(entry->elem_size!= sizeof(bmkv_float)) {
                BDBG_ERR(("%s:%p %s invalid size of float element '%s' %u[%u]", "bmkv_element_parse", (void *)cursor, elem_name, entry->name, entry->elem_size, (unsigned)sizeof(bmkv_float)));
                goto err_elem_size;
            }
            break;
        case bmkv_parser_entry_type_utf8:
            if(entry->elem_size!=sizeof(bmkv_utf8)) {
                BDBG_ERR(("%s:%p %s invalid size of UTF8 element '%s' %u[%u]", "bmkv_element_parse", (void *)cursor, elem_name, entry->name, entry->elem_size, (unsigned)sizeof(bmkv_utf8)));
                goto err_elem_size;
            }
			break;
        case bmkv_parser_entry_type_data:
            if(entry->elem_size!=sizeof(bmkv_data)) {
                BDBG_ERR(("%s:%p %s invalid size of DATA element '%s' %u[%u]", "bmkv_element_parse", (void *)cursor, elem_name, entry->name, entry->elem_size, (unsigned)sizeof(bmkv_utf8)));
                goto err_elem_size;
            }
			break;
        case bmkv_parser_entry_type_large_string:
            if(entry->elem_size!=sizeof(bmkv_string)) {
                BDBG_ERR(("%s:%p %s invalid size of string element '%s' %u[%u]", "bmkv_element_parse", (void *)cursor, elem_name, entry->name, entry->elem_size, (unsigned)sizeof(bmkv_utf8)));
                goto err_elem_size;
            }
			break;
        case bmkv_parser_entry_type_table:
            if(entry->elem_size!=sizeof(bmkv_table)) {
                BDBG_ERR(("%s:%p %s invalid size of UTF8 element '%s' %u[%u]", "bmkv_element_parse", (void *)cursor, elem_name, entry->name, entry->elem_size, (unsigned)sizeof(bmkv_table)));
                goto err_elem_size;
            }
			break;
        case bmkv_parser_entry_type_bool:
            if(entry->elem_size!=sizeof(bool)) {
                BDBG_ERR(("%s:%p %s invalid size of bool element '%s' %u[%u]", "bmkv_element_parse", (void *)cursor, elem_name, entry->name, entry->elem_size, (unsigned)sizeof(unsigned)));
                goto err_elem_size;
            }
            break;
        case bmkv_parser_entry_type_stop:
            if(entry->elem_size!=sizeof(bmkv_size)) {
                BDBG_ERR(("%s:%p %s invalid size of stop element '%s' %u[%u]", "bmkv_element_parse", (void *)cursor, elem_name, entry->name, entry->elem_size, (unsigned)sizeof(bmkv_size)));
                goto err_elem_size;
            }
            break;
        case bmkv_parser_entry_type_string:
        case bmkv_parser_entry_type_last:
            break;

        default:
            BDBG_ERR(("%s::%#lx %s unknown elemet type '%s' %u", "bmkv_element_parse", (unsigned long)cursor, elem_name, entry->name, (unsigned)entry->type));
            goto err_type;
        }
    }

    for(i=0;batom_cursor_pos(cursor)<elem_size;) {
        bmkv_header header;
        void *data;

        BDBG_MSG_TRACE(("%s:%#lx pos:%u elem_end:%u", "bmkv_element_parse", (unsigned long)cursor, batom_cursor_pos(cursor), elem_size));

        if(!entries[i].id){
            i++;
            continue;
        }
        if(!bmkv_parse_header(cursor, &header)) {
            goto err_header;
        }

        /* look for id in the entry table */
        if(i<nentries && entries[i].id == header.id) { /* first try next element in the table */
            BDBG_MSG_TRACE(("%s:%#lx fast match %u", "bmkv_element_parse", (unsigned long)cursor, i));
        } else {
            for(i=0;i<nentries;i++) {
                if(entries[i].id == header.id) {
                    break;
                }
            }
            BDBG_MSG_TRACE(("%s:%#lx slow match %u/%u", "bmkv_element_parse", (unsigned long)cursor, i, nentries));
        }
        entry = &entries[i]; /* if not found, then entry would point to the last entry */
        if(entry->type!=bmkv_parser_entry_type_last) {
            BDBG_MSG(("%s:%#lx %s parsing element '%s' of type %u (%#x:%u)", "bmkv_element_parse", (unsigned long)cursor, elem_name, entry->name, (unsigned)entry->type, (unsigned)header.id, (unsigned)header.size));
            *(bool*)((uint8_t *)(elem) + entry->validate_off) = true;
            data = (void*)((uint8_t *)(elem) + entry->elem_off);
            i++; /* advance to the next entry in hope for the perfect match */
        } else {
            data = NULL;
        }

        switch(entry->type) {
            case bmkv_parser_entry_type_unsigned64:
            case bmkv_parser_entry_type_unsigned:
            case bmkv_parser_entry_type_bool:
            case bmkv_parser_entry_type_id:
                if(header.size<=8) {
                    uint64_t udata;
                    udata = bmkv_read_unsigned(cursor, header.size);
                    if(entry->type!=bmkv_parser_entry_type_unsigned64) {
                        if(entry->type!=bmkv_parser_entry_type_bool) {
                            if(udata<0xFFFFFFFFul) {
                                if(entry->type==bmkv_parser_entry_type_unsigned) {
                                    *(unsigned *)data = (uint32_t)udata;
                                } else { /* id */
                                    *(bmkv_id*)data = (bmkv_id)udata;
                                }
                            } else {
                                BDBG_WRN(("%s:%#lx %s overflow in value of u-integer element %s:" BDBG_UINT64_FMT, "bmkv_element_parse", (unsigned long)cursor, elem_name, entry->name, BDBG_UINT64_ARG(udata)));
                                goto elem_bad_data;
                            }
                        } else {
                            if(udata==0) {
                                *(bool *)data = false;
                            } else if(udata==1) {
                                *(bool *)data = true;
                            } else {
                                BDBG_WRN(("%s:%#lx %s invalid value of bool element %s:" BDBG_UINT64_FMT, "bmkv_element_parse", (unsigned long)cursor, elem_name, entry->name, BDBG_UINT64_ARG(udata)));
                                goto elem_bad_data;
                            }
                        }
                    } else {
                        *(uint64_t *)data = udata;
                    }
                } else {
                    BDBG_WRN(("%s:%p %s invalid size of u-integer element '%s' %u[0..4]", "bmkv_element_parse", (void *)cursor, elem_name, entry->name, (unsigned)header.size));
                    goto elem_invalid;
                }
                break;
            case bmkv_parser_entry_type_signed:
                if(header.size<=8) {
                    int64_t sdata  = bmkv_read_signed(cursor, header.size);
                    if(sdata<0x7FFFFFFF && sdata>-0x7FFFFFFF) {
                        *(signed *)data = (int32_t)sdata;
                    } else {
                        BDBG_WRN(("%s:%p %s:%s overflow in value of s-integer element " BDBG_UINT64_FMT, "bmkv_element_parse", (void *)cursor, elem_name, entry->name, BDBG_UINT64_ARG(sdata)));
                        goto elem_bad_data;
                    }
                } else {
                    BDBG_WRN(("%s:%p %s invalid size of s-integer element '%s' %u[0..8]", "bmkv_element_parse",(void *)cursor, elem_name, entry->name, (unsigned)header.size));
                    goto elem_invalid;
                }
                break;
            case bmkv_parser_entry_type_unique_id:
                if(header.size == sizeof(bmkv_unique_id)) {
                    batom_cursor_copy(cursor, data, header.size);
                } else {
                    BDBG_WRN(("%s:%p %s invalid size of unique ID element '%s' %u[%u]", "bmkv_element_parse", (void *)cursor, elem_name, entry->name, (unsigned)header.size, (unsigned)sizeof(bmkv_unique_id)));
                    goto elem_invalid;
                }
                break;
            case bmkv_parser_entry_type_date:
                if(header.size == sizeof(bmkv_date)) {
                    *(bmkv_date*)data = batom_cursor_uint64_be(cursor);
                } else {
                    BDBG_WRN(("%s:%p %s invalid size of date element '%s' %u[%u]", "bmkv_element_parse", (void *)cursor, elem_name, entry->name, (unsigned)header.size, (unsigned)sizeof(bmkv_date)));
                    goto elem_invalid;
                }
                break;
            case bmkv_parser_entry_type_float:
                /* XXX here we assume that binary presentation of float and double is the same in file and host (module endiannes) */
                BDBG_CASSERT(sizeof(float)==4);
                BDBG_CASSERT(sizeof(double)==8);
                if(header.size == sizeof(float)) {
                    /* we use volatile in order to force data loading into the memory, prevent compiler from skipping float->double conversion */
                    *(volatile uint32_t*)data = batom_cursor_uint32_be(cursor);
                    BDBG_MSG_TRACE(("%s:%#lx %s '%s' float value:%#x", "bmkv_element_parse", (unsigned long)cursor, elem_name, entry->name, (unsigned)*(uint32_t *)data));
                    *(volatile double*)data = *(volatile float*)data;
                } else if(header.size == sizeof(double)) {
                    *(uint64_t*)data = batom_cursor_uint64_be(cursor);
                    BDBG_MSG_TRACE(("%s:%#lx %s '%s' double value:%#llx", "bmkv_element_parse", (unsigned long)cursor, elem_name, entry->name, *(uint64_t *)data));
                } else {
                    BDBG_WRN(("%s:%p %s invalid size of float element '%s' %u[%u,%u]", "bmkv_element_parse", (void *)cursor, elem_name, entry->name, (unsigned)header.size, (unsigned)sizeof(float),(unsigned)sizeof(double)));
                    goto elem_invalid;
                }
                break;
            case bmkv_parser_entry_type_string:
                if(header.size < entry->elem_size) {
                    batom_cursor_copy(cursor, data, header.size);
                    ((uint8_t *)data)[header.size] = '\0';
                } else {
                    BDBG_WRN(("%s:%#lx %s too small block for string-u element '%s' %u[%u]", "bmkv_element_parse", (unsigned long)cursor, elem_name, entry->name, (unsigned)entry->elem_size, (unsigned)header.size));
                    goto elem_invalid;
                }
                break;
            case bmkv_parser_entry_type_table:
                if(!bmkv_table_parse(cursor, header.size,  data, (const bmkv_parser_desc *)entry->name)) {
                    goto elem_bad_data;
                }
                break;
            case bmkv_parser_entry_type_large_string:
                if( ((bmkv_string *)data)->string_data) {
                    BKNI_Free(((bmkv_string*)data)->string_data);
                }
                ((bmkv_string*)data)->string_data = BKNI_Malloc(header.size+1);
                if( ((bmkv_string*)data)->string_data) {
                    batom_cursor_copy(cursor, ((bmkv_string*)data)->string_data, header.size);
                    ((bmkv_string*)data)->string_data[header.size]='\0';
                } else {
                    BDBG_WRN(("%s:%#lx %s can't allocate buffer %u for string element '%s'", "bmkv_element_parse", (unsigned long)cursor, elem_name, (unsigned)header.size, entry->name));
                    goto elem_drop;
                }
                break;
            case bmkv_parser_entry_type_data:
                if( ((bmkv_data*)data)->data) {
                    BKNI_Free(((bmkv_data*)data)->data);
                }
                ((bmkv_data*)data)->data = BKNI_Malloc(header.size);
                if( ((bmkv_data*)data)->data) {
                    ((bmkv_data*)data)->data_len = header.size;
                    batom_cursor_copy(cursor, ((bmkv_data*)data)->data, header.size);
                } else {
                    BDBG_WRN(("%s:%#lx %s can't allocate buffer %u for data element '%s'", "bmkv_element_parse", (unsigned long)cursor, elem_name, (unsigned)header.size, entry->name));
                    goto elem_drop;
                }
                break;
            case bmkv_parser_entry_type_utf8:
                if( ((bmkv_utf8*)data)->utf8_len <= header.size) {
                    if( ((bmkv_utf8 *)data)->utf8_data) {
                        BKNI_Free(((bmkv_utf8 *)data)->utf8_data);
                    }
                    ((bmkv_utf8 *)data)->utf8_data = BKNI_Malloc(header.size+1);
                }
                if( ((bmkv_utf8 *)data)->utf8_data) {
                    ((bmkv_data*)data)->data_len = header.size+1;
                    batom_cursor_copy(cursor, ((bmkv_utf8 *)data)->utf8_data, header.size);
                    ((bmkv_utf8 *)data)->utf8_data[header.size]='\0';
                } else {
                    BDBG_WRN(("%s:%#lx %s can't allocate buffer %u for UTF-8 element '%s'", "bmkv_element_parse", (unsigned long)cursor, elem_name, (unsigned)header.size, entry->name));
                    goto elem_drop;
                }
                break;
            case bmkv_parser_entry_type_stop:
                *(bmkv_size*)data = header.size;
                goto done;
            case bmkv_parser_entry_type_last:
                /* test for special ID's */
                switch(header.id) {
                    case BMKV_INVALID_ID:
                        goto err_header;
                    case 0xBF: /* CRC-32 */
                    case BMKV_VOID_ID:
                        goto elem_drop;
                    default:
                        BDBG_MSG(("%s:%p %s unknown element id %#lx", "bmkv_element_parse", (void *)cursor, elem_name, (unsigned long)header.id));
                        goto elem_drop;
                }
                break;
        }
        if(!BATOM_IS_EOF(cursor)) {
            continue;
        }
        BDBG_WRN(("%s:%p %s premature end of data", "bmkv_element_parse", (void *)cursor, elem_name));
        goto elem_bad_data;
elem_drop:
elem_invalid:
        batom_cursor_skip(cursor, header.size);;
elem_bad_data:
        if(entry->type!=bmkv_parser_entry_type_last) {
    		*(bool*)((uint8_t *)(elem) + entry->validate_off) = false;
        }
		if(BATOM_IS_EOF(cursor)) {
			goto err_eof;
		}
    }
done:
    return true;

err_header:
err_elem_size:
err_type:
	if(batom_cursor_pos(cursor)>elem_size) {
    	batom_cursor_skip(cursor, elem_size - batom_cursor_pos(cursor));
	}
err_eof:
    return false;
}


void
bmkv_element_shutdown(const bmkv_parser_entry *entries, void *elem)
{
    const bmkv_parser_entry *entry;

	BDBG_ASSERT(entries);
	BDBG_ASSERT(elem);

    /* count number elements in the table and validate table elements */
    for(entry=entries;entry->type!=bmkv_parser_entry_type_last;entry++) {
        void *data = (void*)((uint8_t *)(elem) + entry->validate_off);
		*(bool *)data = false;
        data = (void*)((uint8_t *)(elem) + entry->elem_off);
        switch(entry->type) {
        case bmkv_parser_entry_type_unsigned:
        case bmkv_parser_entry_type_id:
        case bmkv_parser_entry_type_unsigned64:
        case bmkv_parser_entry_type_signed:
        case bmkv_parser_entry_type_unique_id:
        case bmkv_parser_entry_type_date:
        case bmkv_parser_entry_type_float:
        case bmkv_parser_entry_type_string:
        case bmkv_parser_entry_type_bool:
		case bmkv_parser_entry_type_stop:
        case bmkv_parser_entry_type_last:
            break;
		case bmkv_parser_entry_type_large_string:
   			if(((bmkv_string*)data)->string_data) {
				BKNI_Free(((bmkv_string*)data)->string_data);
				((bmkv_string*)data)->string_data= NULL;
			}
			((bmkv_string*)data)->string_len= 0;
			break;
		case bmkv_parser_entry_type_data:
   			if(((bmkv_data*)data)->data) {
				BKNI_Free(((bmkv_data*)data)->data);
				((bmkv_data*)data)->data= NULL;
			}
			((bmkv_data*)data)->data_len= 0;
			break;
		case bmkv_parser_entry_type_utf8:
			if(((bmkv_utf8 *)data)->utf8_data) {
				BKNI_Free(((bmkv_utf8 *)data)->utf8_data);
				((bmkv_utf8 *)data)->utf8_data = NULL;
			}
			((bmkv_utf8 *)data)->utf8_len = 0;
			break;
        case bmkv_parser_entry_type_table:
			bmkv_table_shutdown(data, (const bmkv_parser_desc *)entry->name);
			break;
        }
    }
	return;
}


void
bmkv_element_print(const bmkv_parser_entry *entries, BDBG_Level level, unsigned padding, const char *name, const void *elem)
{
    const bmkv_parser_entry *entry;

	BDBG_ASSERT(entries);
	BDBG_ASSERT(elem);
	BSTD_UNUSED(name);
	BDBG_ASSERT(name);

    /* count number elements in the table and validate table elements */
    for(entry=entries;entry->type!=bmkv_parser_entry_type_last;entry++) {
		char buf[128/8*2+1];
		char *value = buf;
		unsigned i;
        void *data = (void*)((uint8_t *)(elem) + entry->validate_off);
		int rc;

		if(!(*(bool *)data)) {
			continue;
		}
        data = (void*)((uint8_t *)(elem) + entry->elem_off);
		*buf = '\0';
        switch(entry->type) {
        case bmkv_parser_entry_type_bool:
			value = *(bool*)data ? "on":"off";
			break;
        case bmkv_parser_entry_type_unsigned:
			if(*(unsigned *)data<33) {
				BKNI_Snprintf(buf, sizeof(buf), "%u", *(unsigned *)data);
			} else {
				BKNI_Snprintf(buf, sizeof(buf), "%u:%#x", *(unsigned *)data, *(unsigned *)data);
			}
			break;
        case bmkv_parser_entry_type_id:
			BKNI_Snprintf(buf, sizeof(buf), "%#x", *(unsigned *)data);
			break;
		case bmkv_parser_entry_type_unsigned64:
			if(*(uint64_t*)data<16) {
				BKNI_Snprintf(buf, sizeof(buf), B_OFFT_FMT, B_OFFT_ARG(*(uint64_t*)data));
			} else {
				BKNI_Snprintf(buf, sizeof(buf), B_OFFT_FMT ":" BDBG_UINT64_FMT, B_OFFT_ARG(*(uint64_t*)data), BDBG_UINT64_ARG(*(uint64_t*)data));
			}
			break;
        case bmkv_parser_entry_type_signed:
			BKNI_Snprintf(buf, sizeof(buf), "%d", *(signed *)data);
			break;
		case bmkv_parser_entry_type_data:
			rc = BKNI_Snprintf(value, sizeof(buf), "[%u]", (unsigned)((bmkv_data*)data)->data_len);
			if(rc<0) {
				break;
			}
			value += rc;
			for(i=0;((bmkv_data*)data)->data && ((bmkv_data*)data)->data_len>i;i++) {
				if(value>=buf+sizeof(buf)) {
					BDBG_CASSERT(sizeof(buf)>4);
					buf[sizeof(buf)-4] = '.';
					buf[sizeof(buf)-3] = '.';
					buf[sizeof(buf)-2] = '.';
					buf[sizeof(buf)-1] = '\0';
					break;
				}
				rc = BKNI_Snprintf(value, buf+sizeof(buf)-value, "%02x", ((uint8_t*)((bmkv_data*)data)->data)[i]);
				if(rc<0) {
					break;
				}
				value+=rc;
			}
			value = buf;
			break;
		case bmkv_parser_entry_type_unique_id:
			for(i=0;i<(sizeof( ((bmkv_unique_id*)data)->data)/ sizeof( ((bmkv_unique_id*)data)->data[0]));i++) {
				if(value>=buf+sizeof(buf)) {
					break;
				}
				rc = BKNI_Snprintf(value, value-buf+sizeof(buf), "%02x", ((bmkv_unique_id*)data)->data[i]);
				if(rc<0) {
					break;
				}
				value +=rc;
			}
			value = buf;
			break;
        case bmkv_parser_entry_type_date:
			BKNI_Snprintf(buf, sizeof(buf), B_OFFT_FMT, B_OFFT_ARG(*(int64_t*)data));
			break;
        case bmkv_parser_entry_type_float:
			BKNI_Snprintf(buf, sizeof(buf), "%f", (float)*(bmkv_float*)data);
			break;
        case bmkv_parser_entry_type_string:
			value = data;
			break;
        case bmkv_parser_entry_type_large_string:
			value = ((bmkv_string*)data)->string_data;
			break;
        case bmkv_parser_entry_type_utf8:
			value = (char *)((bmkv_utf8*)data)->utf8_data;
			break;
        case bmkv_parser_entry_type_last:
			break;
        case bmkv_parser_entry_type_table:
			bmkv_table_print(data, level, padding, (const bmkv_parser_desc *)entry->name);
			value = NULL;
			break;
		case bmkv_parser_entry_type_stop:
			BKNI_Snprintf(buf, sizeof(buf), "[%u]", (unsigned)*(bmkv_size*)data);
   			value = NULL;
			break;
		}
		if(!value) {
			continue;
		}
		B_MKV_PRINT(level, ("%*c%s %s=%s", (int)padding, ' ', name, entry->name, value));
    }
	return;
}

const char bmkv_matroska[] = "matroska";
const char bmkv_webm[] = "webm";

static void
bmkv_EBML_init(bmkv_EBML *ebml)
{

    BDBG_ASSERT(ebml);
    BKNI_Memset(ebml, 0, sizeof(*ebml));
    ebml->EBMLVersion = 1;
    ebml->EBMLReadVersion = 1;
    ebml->EBMLMaxIDLength = 4;
    ebml->EBMLMaxSizeLength = 8;
    BKNI_Memcpy(ebml->DocType, bmkv_matroska, sizeof(bmkv_matroska));
    ebml->DocTypeVersion= 1;
    ebml->DocTypeReadVersion = 1;
    ebml->validate.EBMLVersion = true;
    ebml->validate.EBMLReadVersion = true;
    ebml->validate.EBMLMaxIDLength = true;
    ebml->validate.EBMLMaxSizeLength = true;
    ebml->validate.DocTypeVersion = true;
    ebml->validate.DocTypeReadVersion = true;
    ebml->validate.DocType = true;
    return;
}

static const BMKV_PARSER_BEGIN(bmkv_EBML_parser)
    BMKV_PARSER_FIELD_UNSIGNED(bmkv_EBML, 0x4286, EBMLVersion)
    BMKV_PARSER_FIELD_UNSIGNED(bmkv_EBML, 0x42F7, EBMLReadVersion)
    BMKV_PARSER_FIELD_UNSIGNED(bmkv_EBML, 0x42F2, EBMLMaxIDLength)
    BMKV_PARSER_FIELD_UNSIGNED(bmkv_EBML, 0x42F3, EBMLMaxSizeLength)
    BMKV_PARSER_FIELD_STRING  (bmkv_EBML, 0x4282, DocType)
    BMKV_PARSER_FIELD_UNSIGNED(bmkv_EBML, 0x4287, DocTypeVersion)
    BMKV_PARSER_FIELD_UNSIGNED(bmkv_EBML, 0x4285, DocTypeReadVersion)
BMKV_PARSER_END(bmkv_EBML_parser);

bool
bmkv_EBML_parse(batom_cursor *cursor, size_t elem_size, bmkv_EBML *ebml)
{
    bool success;
    bmkv_EBML_init(ebml);
    success = bmkv_element_parse(cursor, elem_size, bmkv_EBML_parser, "EBML", ebml);
    if(success) {
        bmkv_element_print(bmkv_EBML_parser, BDBG_eMsg, 0, "EBML", ebml);
    }
    return success;
}

bool
bmkv_EBML_validate(const bmkv_EBML *ebml)
{
    if( (ebml->validate.EBMLMaxIDLength && ebml->EBMLMaxIDLength<=4) &&
        (ebml->validate.EBMLMaxSizeLength && ebml->EBMLMaxSizeLength<=8) &&
        (ebml->validate.EBMLReadVersion && ebml->EBMLReadVersion==1) &&
        (ebml->validate.DocType &&
         ( (BKNI_Memcmp(bmkv_matroska, ebml->DocType, sizeof(bmkv_matroska))==0) ||
           (BKNI_Memcmp(bmkv_webm, ebml->DocType, sizeof(bmkv_webm))==0))
        )) {
        return true;
    }
    return false;
}

static const BMKV_PARSER_BEGIN(bmkv_SeekElement_parser)
    BMKV_PARSER_FIELD_UNSIGNED(bmkv_SeekElement, 0x53AB, SeekID)
    BMKV_PARSER_FIELD_UNSIGNED64(bmkv_SeekElement, 0x53AC, SeekPosition)
BMKV_PARSER_END(bmkv_SeekElement_parser);

const bmkv_parser_desc bmkv_SeekElement_desc = {
    "SeekElement",
    bmkv_SeekElement_parser,
    sizeof(bmkv_SeekElement),
    NULL /* memset initializer */
};

static const BMKV_PARSER_BEGIN(bmkv_SeekHead_parser)
    BMKV_PARSER_FIELD_TABLE(bmkv_SeekHead, 0x4DBB, Seek, bmkv_SeekElement_desc)
BMKV_PARSER_END(bmkv_SeekHead_parser);


bool
bmkv_SeekHead_parse(batom_cursor *cursor, size_t elem_size, bmkv_SeekHead *seek)
{
    bool success;
    BKNI_Memset(seek, 0, sizeof(*seek));
    success=bmkv_element_parse(cursor, elem_size, bmkv_SeekHead_parser, "SeekHead", seek);
    if(success) {
        bmkv_element_print(bmkv_SeekHead_parser, BDBG_eMsg, 0, "SeekHead", seek);
    }
    return success;
}

void
bmkv_SeekHead_shutdown(bmkv_SeekHead *seek)
{
    bmkv_element_shutdown(bmkv_SeekHead_parser, seek);
    return;
}

static const BMKV_PARSER_BEGIN(bmkv_SegmentInformation_parser)
    BMKV_PARSER_FIELD_UID(bmkv_SegmentInformation, 0x73A4, SegmentUID)
    BMKV_PARSER_FIELD_UTF8(bmkv_SegmentInformation, 0x7384, SegmentFilename)
    BMKV_PARSER_FIELD_UID(bmkv_SegmentInformation, 0x3CB923, PrevUID)
    BMKV_PARSER_FIELD_UTF8(bmkv_SegmentInformation, 0x3C83AB, PrevFilename)
    BMKV_PARSER_FIELD_UID(bmkv_SegmentInformation, 0x3CB923, NextUID)
    BMKV_PARSER_FIELD_UTF8(bmkv_SegmentInformation, 0x3E83BB, NextFilename)
    BMKV_PARSER_FIELD_UID(bmkv_SegmentInformation, 0x4444, SegmentFamily)
    BMKV_PARSER_FIELD_UNSIGNED(bmkv_SegmentInformation, 0x2AD7B1, TimecodeScale)
    BMKV_PARSER_FIELD_FLOAT(bmkv_SegmentInformation, 0x4489, Duration)
    BMKV_PARSER_FIELD_DATE(bmkv_SegmentInformation, 0x4461, DateUTC)
    BMKV_PARSER_FIELD_UTF8(bmkv_SegmentInformation, 0x7BA9, Title)
    BMKV_PARSER_FIELD_UTF8(bmkv_SegmentInformation, 0x4D80, MuxingApp)
    BMKV_PARSER_FIELD_UTF8(bmkv_SegmentInformation, 0x5741, WritingApp)
BMKV_PARSER_END(bmkv_SegmentInformation_parser);


bool
bmkv_SegmentInfo_parse(batom_cursor *cursor, size_t elem_size, bmkv_SegmentInformation *segment)
{
    bool success;
    BKNI_Memset(segment, 0, sizeof(*segment));
    segment->TimecodeScale = 1000000;
    segment->validate.TimecodeScale = true;
    success=bmkv_element_parse(cursor, elem_size, bmkv_SegmentInformation_parser, "SegmentInformation", segment);
    if(success) {
        bmkv_element_print(bmkv_SegmentInformation_parser, BDBG_eMsg, 0, "SegmentInformation", segment);
    }
    return success;

}


void
bmkv_SegmentInfo_shutdown(bmkv_SegmentInformation *segment)
{
	bmkv_element_shutdown(bmkv_SegmentInformation_parser, segment);
    return;
}



static const BMKV_PARSER_BEGIN(bmkv_TrackEntryVideo_parser)
    BMKV_PARSER_FIELD_BOOL(bmkv_TrackEntryVideo, 0x9A, FlagInterlaced)
    BMKV_PARSER_FIELD_UNSIGNED(bmkv_TrackEntryVideo, 0x53B8, StereoMode)
    BMKV_PARSER_FIELD_UNSIGNED(bmkv_TrackEntryVideo, 0xB0, PixelWidth)
    BMKV_PARSER_FIELD_UNSIGNED(bmkv_TrackEntryVideo, 0xBA, PixelHeight)
    BMKV_PARSER_FIELD_UNSIGNED(bmkv_TrackEntryVideo, 0x54AA, PixelCropBottom)
    BMKV_PARSER_FIELD_UNSIGNED(bmkv_TrackEntryVideo, 0x54BB, PixelCropTop)
    BMKV_PARSER_FIELD_UNSIGNED(bmkv_TrackEntryVideo, 0x54CC, PixelCropLeft)
    BMKV_PARSER_FIELD_UNSIGNED(bmkv_TrackEntryVideo, 0x54DD, PixelCropRight)
    BMKV_PARSER_FIELD_UNSIGNED(bmkv_TrackEntryVideo, 0x54B0, DisplayWidth)
    BMKV_PARSER_FIELD_UNSIGNED(bmkv_TrackEntryVideo, 0x54BA, DisplayHeight)
    BMKV_PARSER_FIELD_UNSIGNED(bmkv_TrackEntryVideo, 0x54B2, DisplayUnit)
    BMKV_PARSER_FIELD_UNSIGNED(bmkv_TrackEntryVideo, 0x54B3, AspectRatioType)
    BMKV_PARSER_FIELD_UNSIGNED(bmkv_TrackEntryVideo, 0x2EB524, ColourSpace)
    BMKV_PARSER_FIELD_FLOAT(bmkv_TrackEntryVideo, 0x2EB523, GammaValue)
    BMKV_PARSER_FIELD_DATA(bmkv_TrackEntryVideo, 0x55B0, Colour)
BMKV_PARSER_END(bmkv_TrackEntryVideo_parser);

static void
bmkv_TrackEntryVideo_init(void *data)
{
    bmkv_TrackEntryVideo *video = data;
    BKNI_Memset(video, 0, sizeof(*video));
    video->FlagInterlaced = false;
    video->StereoMode = 0;
    video->PixelCropBottom = 0;
    video->PixelCropTop = 0;
    video->PixelCropLeft = 0;
    video->PixelCropRight = 0;
    video->DisplayUnit = 0;
    video->AspectRatioType = 0;
    video->validate.FlagInterlaced = true;
    video->validate.StereoMode = true;
    video->validate.PixelCropBottom = true;
    video->validate.PixelCropTop = true;
    video->validate.PixelCropLeft = true;
    video->validate.PixelCropRight = true;
    video->validate.DisplayUnit = true;
    video->validate.AspectRatioType = true;
    return;
}


static const bmkv_parser_desc bmkv_TrackEntryVideo_desc = {
    "Video",
    bmkv_TrackEntryVideo_parser,
    sizeof(bmkv_TrackEntryVideo),
    bmkv_TrackEntryVideo_init
};

static const BMKV_PARSER_BEGIN(bmkv_TrackEntryAudio_parser)
    BMKV_PARSER_FIELD_FLOAT(bmkv_TrackEntryAudio, 0xB5, SamplingFrequency)
    BMKV_PARSER_FIELD_FLOAT(bmkv_TrackEntryAudio, 0x78B5, OutputSamplingFrequency)
    BMKV_PARSER_FIELD_UNSIGNED(bmkv_TrackEntryAudio, 0x9F, Channels)
    BMKV_PARSER_FIELD_DATA(bmkv_TrackEntryAudio, 0x7D7B, ChannelPositions)
    BMKV_PARSER_FIELD_UNSIGNED(bmkv_TrackEntryAudio, 0x6264, BitDepth)
BMKV_PARSER_END(bmkv_TrackEntryAudio_parser);

static void
bmkv_TrackEntryAudio_init(void *data)
{
    bmkv_TrackEntryAudio *audio = data;
    BKNI_Memset(audio, 0, sizeof(*audio));
    audio->SamplingFrequency = 8000.0;
    audio->Channels = 1;
    audio->validate.SamplingFrequency = true;
    audio->validate.Channels = true;
    return;
}

static const bmkv_parser_desc bmkv_TrackEntryAudio_desc = {
    "Audio",
    bmkv_TrackEntryAudio_parser,
    sizeof(bmkv_TrackEntryAudio),
    bmkv_TrackEntryAudio_init
};

static const BMKV_PARSER_BEGIN(bmkv_ContentCompression_parser)
    BMKV_PARSER_FIELD_UNSIGNED(bmkv_ContentCompression, 0x4254, ContentCompAlgo)
    BMKV_PARSER_FIELD_DATA(bmkv_ContentCompression, 0x4255, ContentCompSettings)
BMKV_PARSER_END(bmkv_ContentCompression_parser);

static void
bmkv_ContentCompression_init(void *data)
{
    bmkv_ContentCompression *compression = data;
    BKNI_Memset(compression, 0, sizeof(*compression));
    compression->ContentCompAlgo = 0;
    compression->validate.ContentCompAlgo = true;
    return;
}

static const bmkv_parser_desc bmkv_ContentCompression_desc = {
    "ContentCompression",
    bmkv_ContentCompression_parser,
    sizeof(bmkv_ContentCompression),
    bmkv_ContentCompression_init
};

static const BMKV_PARSER_BEGIN(bmkv_ContentEncryption_parser)
    BMKV_PARSER_FIELD_UNSIGNED(bmkv_ContentEncryption, 0x47E1, ContentEncAlgo)
    BMKV_PARSER_FIELD_DATA(bmkv_ContentEncryption, 0x47E2, ContentEncKeyID)
    BMKV_PARSER_FIELD_DATA(bmkv_ContentEncryption, 0x47E3, ContentSignature)
    BMKV_PARSER_FIELD_DATA(bmkv_ContentEncryption, 0x47E4, ContentSigKeyID)
    BMKV_PARSER_FIELD_UNSIGNED(bmkv_ContentEncryption, 0x47E5, ContentSigAlgo)
    BMKV_PARSER_FIELD_UNSIGNED(bmkv_ContentEncryption, 0x47E6, ContentSigHashAlgo)
BMKV_PARSER_END(bmkv_ContentEncryption_parser);

static void
bmkv_ContentEncryption_init(void *data)
{
    bmkv_ContentEncryption *encryption = data;
    BKNI_Memset(encryption, 0, sizeof(*encryption));
    encryption->ContentEncAlgo  = 0;
    encryption->ContentSigAlgo = 0;
    encryption->ContentSigHashAlgo = 0;
    encryption->validate.ContentEncAlgo = true;
    encryption->validate.ContentSigAlgo = true;
    encryption->validate.ContentSigHashAlgo = true;
    return;
}

static const bmkv_parser_desc bmkv_ContentEncryption_desc = {
    "ContentEncryption",
    bmkv_ContentEncryption_parser,
    sizeof(bmkv_ContentEncryption),
    bmkv_ContentEncryption_init
};

static const BMKV_PARSER_BEGIN(bmkv_ContentEncoding_parser)
    BMKV_PARSER_FIELD_UNSIGNED(bmkv_ContentEncoding, 0x5031, ContentEncodingOrder)
    BMKV_PARSER_FIELD_UNSIGNED(bmkv_ContentEncoding, 0x5032, ContentEncodingScope)
    BMKV_PARSER_FIELD_UNSIGNED(bmkv_ContentEncoding, 0x5033, ContentEncodingType)
    BMKV_PARSER_FIELD_TABLE(bmkv_ContentEncoding, 0x5034, ContentCompression, bmkv_ContentCompression_desc)
    BMKV_PARSER_FIELD_TABLE(bmkv_ContentEncoding, 0x5034, ContentEncryption, bmkv_ContentEncryption_desc)
BMKV_PARSER_END(bmkv_ContentEncoding_parser);


static void
bmkv_ContentEncoding_init(void *data)
{
    bmkv_ContentEncoding *encoding = data;
    BKNI_Memset(encoding, 0, sizeof(*encoding));
    encoding->ContentEncodingOrder = 0;
    encoding->ContentEncodingScope = 1;
    encoding->ContentEncodingType = 0;
    encoding->validate.ContentEncodingOrder = true;
    encoding->validate.ContentEncodingScope = true;
    encoding->validate.ContentEncodingType = true;
    return;
}

static const bmkv_parser_desc bmkv_ContentEncoding_desc = {
    "ContentEncoding",
    bmkv_ContentEncoding_parser,
    sizeof(bmkv_ContentEncoding),
    bmkv_ContentEncoding_init
};

static const BMKV_PARSER_BEGIN(bmkv_ContentEncodings_parser)
    BMKV_PARSER_FIELD_TABLE(bmkv_ContentEncodings, 0x6240, ContentEncoding, bmkv_ContentEncoding_desc)
BMKV_PARSER_END(bmkv_ContentEncoding_parser);

static const bmkv_parser_desc bmkv_ContentEncodings_desc = {
    "ContentEncodings",
    bmkv_ContentEncodings_parser,
    sizeof(bmkv_ContentEncodings),
    NULL /* BKNI_Memset */
};

static const BMKV_PARSER_BEGIN(bmkv_AttachmentLink_parser)
    BMKV_PARSER_FIELD_UNSIGNED64(bmkv_AttachmentLink, 0, AttachmentLink)
BMKV_PARSER_END(bmkv_ContentEncoding_parser);

static const bmkv_parser_desc bmkv_AttachmentLink_desc = {
    "AttachmentLink",
    bmkv_AttachmentLink_parser,
    sizeof(bmkv_AttachmentLink),
    NULL /* BKNI_Memset */
};

static const BMKV_PARSER_BEGIN(bmkv_TrackEntry_parser)
    BMKV_PARSER_FIELD_UNSIGNED(bmkv_TrackEntry, 0xD7, TrackNumber)
    BMKV_PARSER_FIELD_UNSIGNED64(bmkv_TrackEntry, 0x73C5, TrackUID)
    BMKV_PARSER_FIELD_UNSIGNED(bmkv_TrackEntry, 0x83, TrackType)
    BMKV_PARSER_FIELD_BOOL(bmkv_TrackEntry, 0xB9, FlagEnabled)
    BMKV_PARSER_FIELD_BOOL(bmkv_TrackEntry, 0x88, FlagDefault)
    BMKV_PARSER_FIELD_BOOL(bmkv_TrackEntry, 0x55AA, FlagForced)
    BMKV_PARSER_FIELD_BOOL(bmkv_TrackEntry, 0x9C, FlagLacing)
    BMKV_PARSER_FIELD_UNSIGNED(bmkv_TrackEntry, 0x6DE7, MinCache)
    BMKV_PARSER_FIELD_UNSIGNED(bmkv_TrackEntry, 0x6DF8, MaxCache)
    BMKV_PARSER_FIELD_UNSIGNED64(bmkv_TrackEntry, 0x23E383, DefaultDuration)
    BMKV_PARSER_FIELD_FLOAT(bmkv_TrackEntry, 0x23314F, TrackTimecodeScale)
    BMKV_PARSER_FIELD_SIGNED(bmkv_TrackEntry, 0x537F, TrackOffset)
    BMKV_PARSER_FIELD_UNSIGNED(bmkv_TrackEntry, 0x55EE, MaxBlockAdditionID)
    BMKV_PARSER_FIELD_UTF8(bmkv_TrackEntry, 0x536E, Name)
    BMKV_PARSER_FIELD_STRING(bmkv_TrackEntry, 0x22B59C, Language)
    BMKV_PARSER_FIELD_STRING(bmkv_TrackEntry, 0x86, CodecID)
    BMKV_PARSER_FIELD_DATA(bmkv_TrackEntry, 0x63A2, CodecPrivate)
    BMKV_PARSER_FIELD_UTF8(bmkv_TrackEntry, 0x258688, CodecName)
    BMKV_PARSER_FIELD_TABLE(bmkv_TrackEntry, 0x7446, AttachmentLink, bmkv_AttachmentLink_desc)
    BMKV_PARSER_FIELD_UTF8(bmkv_TrackEntry, 0x3A9697, CodecSettings)
    BMKV_PARSER_FIELD_VSTRING(bmkv_TrackEntry, 0x3B4040, CodecInfoURL)
    BMKV_PARSER_FIELD_VSTRING(bmkv_TrackEntry, 0x26B240, CodecDownloadURL)
    BMKV_PARSER_FIELD_BOOL(bmkv_TrackEntry, 0xAA, CodecDecodeAll)
    BMKV_PARSER_FIELD_UNSIGNED(bmkv_TrackEntry, 0xAA, TrackOverlay)
    BMKV_PARSER_FIELD_UNSIGNED64(bmkv_TrackEntry, 0xC0, TrickTrackUID)
    BMKV_PARSER_FIELD_UNSIGNED64(bmkv_TrackEntry, 0xC7, MasterTrackUID)
    BMKV_PARSER_FIELD_BOOL(bmkv_TrackEntry, 0xC6, TrickTrackFlag)
    BMKV_PARSER_FIELD_TABLE(bmkv_TrackEntry, 0xE0, Video, bmkv_TrackEntryVideo_desc)
    BMKV_PARSER_FIELD_TABLE(bmkv_TrackEntry, 0xE1, Audio, bmkv_TrackEntryAudio_desc)
    BMKV_PARSER_FIELD_TABLE(bmkv_TrackEntry, 0x6D80, ContentEncodings, bmkv_ContentEncodings_desc)
BMKV_PARSER_END(bmkv_TrackEntry_parser);

static void
bmkv_TrackEntry_init(void *data)
{
    bmkv_TrackEntry *track = data;

    BKNI_Memset(track, 0, sizeof(*track));
    track->FlagEnabled = true;
    track->FlagDefault = true;
    track->FlagForced = false;
    track->FlagLacing = true;
    track->MinCache = 0;
    track->TrackTimecodeScale = 1.0;
    track->validate.FlagEnabled = true;
    track->validate.FlagDefault = true;
    track->validate.FlagForced = true;
    track->validate.FlagLacing = true;
    track->validate.MinCache = true;
    track->validate.TrackTimecodeScale = true;
    return;
}

static const bmkv_parser_desc bmkv_TrackEntry_desc = {
    "TrackEntry",
    bmkv_TrackEntry_parser,
    sizeof(bmkv_TrackEntry),
    bmkv_TrackEntry_init
};

static const BMKV_PARSER_BEGIN(bmkv_TracksData_parser)
     BMKV_PARSER_FIELD_UNSIGNED(bmkv_TracksData, 0xDB, TracksDataVersion)
     BMKV_PARSER_FIELD_UNSIGNED(bmkv_TracksData, 0xDC, TracksDataSize)
     BMKV_PARSER_FIELD_DATA(bmkv_TracksData, 0xDE, TracksDataPayload)
BMKV_PARSER_END(bmkv_TracksData_parser);

static void
bmkv_TracksData_init(void *data)
{
    bmkv_TracksData *track = data;

    BKNI_Memset(track, 0, sizeof(*track));
    return;
}

static const bmkv_parser_desc bmkv_TracksData_desc = {
    "TracksData",
    bmkv_TracksData_parser,
    sizeof(bmkv_TracksData),
    bmkv_TracksData_init
};

static const BMKV_PARSER_BEGIN(bmkv_Tracks_parser)
   BMKV_PARSER_FIELD_TABLE(bmkv_Tracks, 0xAE, TrackEntry, bmkv_TrackEntry_desc)
   BMKV_PARSER_FIELD_TABLE(bmkv_Tracks, 0xDA, TracksData, bmkv_TracksData_desc)
BMKV_PARSER_END(bmkv_Tracks_parser);

bool
bmkv_Tracks_parse(batom_cursor *cursor, size_t elem_size, bmkv_Tracks *track)
{
    bool success;

    BKNI_Memset(track, 0, sizeof(*track));

    success=bmkv_element_parse(cursor, elem_size, bmkv_Tracks_parser, "Tracks", track);
    if(success) {
        bmkv_element_print(bmkv_Tracks_parser, BDBG_eMsg, 0, "Tracks", track);
    }
    return success;
}

void
bmkv_Tracks_shutdown(bmkv_Tracks *track)
{
    bmkv_element_shutdown(bmkv_Tracks_parser, track);
    return;
}

bmkv_element_parse_result
bmkv_element_parse_from_desc(batom_cursor *cursor, const bmkv_parser_desc *desc, bmkv_id id, void *elem)
{
    bmkv_header header;
    batom_checkpoint checkpoint;
    bmkv_element_parse_result result = bmkv_element_parse_result_error;

    BDBG_ASSERT(cursor);
    BDBG_ASSERT(desc);
    BDBG_ASSERT(elem);
    BATOM_SAVE(cursor, &checkpoint);

    if(!bmkv_parse_header(cursor, &header)) {
        goto error;
    }
    if(header.size == BMKV_RESERVED_SIZE) {
        goto error;
    }
    if(header.id != id) {
        BDBG_MSG(("bmkv_element_parse_from_desc: %#lx unknown element %#x(%#x)", (unsigned long)cursor, (unsigned)header.id, (unsigned)id));
        result = bmkv_element_parse_result_unknown_id;
        goto error;
    }
    if (bmkv_element_parse_from_desc_body(cursor, desc, header.size, elem)) {
        return bmkv_element_parse_result_success;
    }

error:
    batom_cursor_rollback(cursor, &checkpoint);
    return result;
}

bool
bmkv_element_skip(batom_cursor *cursor)
{
    bmkv_header header;
    batom_checkpoint checkpoint;

    BDBG_ASSERT(cursor);
    BATOM_SAVE(cursor, &checkpoint);

    if(!bmkv_parse_header(cursor, &header)) {
        goto error;
    }
    if(header.size == BMKV_RESERVED_SIZE) {
        goto error;
    }

    if (batom_cursor_skip(cursor, header.size) == header.size) {
        return true;
    }

error:
    batom_cursor_rollback(cursor, &checkpoint);
    return false;
}

/* codecs ID http://haali.cs.msu.ru/mkv/codecs.pdf */

#define B_MKV_TEST_CODEC(track, codec) { static const char codec_id[] = codec; \
    return track->validate.CodecID && BKNI_Memcmp(track->CodecID, codec_id, sizeof(codec_id))==0; }

bool
bmkv_IsTrackVideoAvc(const bmkv_TrackEntry *track)
{
    BDBG_ASSERT(track);
    B_MKV_TEST_CODEC(track, "V_MPEG4/ISO/AVC");
}

bool
bmkv_IsTrackVideoHevc(const bmkv_TrackEntry *track)
{
    BDBG_ASSERT(track);
    B_MKV_TEST_CODEC(track, "V_MPEGH/ISO/HEVC");
}

bool
bmkv_IsTrackVideoMpeg2(const bmkv_TrackEntry *track)
{
    BDBG_ASSERT(track);
    B_MKV_TEST_CODEC(track, "V_MPEG2");
}

bool
bmkv_IsTrackVideoMpeg1(const bmkv_TrackEntry *track)
{
    BDBG_ASSERT(track);
    B_MKV_TEST_CODEC(track, "V_MPEG1");
}


bool
bmkv_IsTrackVideoVFW(const bmkv_TrackEntry *track)
{
    BDBG_ASSERT(track);
    B_MKV_TEST_CODEC(track, "V_MS/VFW/FOURCC");
}

    bool
bmkv_IsTrackVideoMpeg4Asp(const bmkv_TrackEntry *track)
{
    B_MKV_TEST_CODEC(track, "V_MPEG4/ISO/ASP");
}


    bool
bmkv_IsTrackAudioMkvAac(const bmkv_TrackEntry *track)
{
    BDBG_ASSERT(track);
    B_MKV_TEST_CODEC(track, "A_AAC");
}


bool
bmkv_IsTrackAudioAac(const bmkv_TrackEntry *track, bmedia_info_aac *aac)
{
    const bmkv_TrackEntryAudio *audio;
    unsigned sampling_frequency;

    static const char a_aac_mpeg2_main[] = "A_AAC/MPEG2/MAIN";
    static const char a_aac_mpeg2_lc[] = "A_AAC/MPEG2/LC";
    static const char a_aac_mpeg2_lc_sbr[] = "A_AAC/MPEG2/LC/SBR";
    static const char a_aac_mpeg2_ssr[] = "A_AAC/MPEG2/SSR";
    static const char a_aac_mpeg4_lc[] = "A_AAC/MPEG4/LC";
    static const char a_aac_mpeg4_lc_sbr[] = "A_AAC/MPEG4/LC/SBR";
    static const char a_aac_mpeg4_main[] = "A_AAC/MPEG4/MAIN";
    static const char a_aac_mpeg4_ssr[] = "A_AAC/MPEG4/SSR";
    static const char a_aac_mpeg4_ltp[] = "A_AAC/MPEG4/LTP";

    BDBG_ASSERT(track);
    BDBG_ASSERT(aac);

    if(!track->validate.CodecID || !track->validate.Audio || track->Audio.nelems < 1) {
        return false;
    }
    audio = &BMKV_TABLE_ELEM(track->Audio,bmkv_TrackEntryAudio, 0);
    if(!audio->validate.SamplingFrequency || !audio->validate.Channels) {
        return false;
    }
    sampling_frequency = 2*((((unsigned)audio->SamplingFrequency) + 1)/2); /* round sampling frequency */
    if(!bmedia_info_aac_set_sampling_frequency_index(aac, sampling_frequency) || !bmedia_info_aac_set_channel_configuration(aac, audio->Channels)) {
        return false;
    }
    if(
            BKNI_Memcmp(track->CodecID, a_aac_mpeg2_lc, sizeof(a_aac_mpeg2_lc))==0
         || BKNI_Memcmp(track->CodecID, a_aac_mpeg2_lc_sbr, sizeof(a_aac_mpeg2_lc_sbr))==0
         || BKNI_Memcmp(track->CodecID, a_aac_mpeg4_lc, sizeof(a_aac_mpeg4_lc))==0
         || BKNI_Memcmp(track->CodecID, a_aac_mpeg4_lc_sbr, sizeof(a_aac_mpeg4_lc_sbr))==0
            ) {
        aac->profile = 0;
        return true;
    } else if(
            BKNI_Memcmp(track->CodecID, a_aac_mpeg2_main, sizeof(a_aac_mpeg2_main))==0
         || BKNI_Memcmp(track->CodecID, a_aac_mpeg4_main, sizeof(a_aac_mpeg4_main))==0) {
        aac->profile = 1;
        return true;
    } else if(
            BKNI_Memcmp(track->CodecID, a_aac_mpeg2_ssr, sizeof(a_aac_mpeg2_ssr))==0
         || BKNI_Memcmp(track->CodecID, a_aac_mpeg4_ssr, sizeof(a_aac_mpeg4_ssr))==0) {
        aac->profile = 2;
        return true;
    } else if(
            BKNI_Memcmp(track->CodecID, a_aac_mpeg4_ltp, sizeof(a_aac_mpeg4_ltp))==0) {
        aac->profile = 3;
        return true;
    }
    return false;
}


bool
bmkv_IsTrackAudioAc3(const bmkv_TrackEntry *track)
{
    BDBG_ASSERT(track);
    B_MKV_TEST_CODEC(track, "A_AC3");
}

bool
bmkv_IsTrackAudioDts(const bmkv_TrackEntry *track)
{
    BDBG_ASSERT(track);
    B_MKV_TEST_CODEC(track, "A_DTS");
}

bool
bmkv_IsTrackAudioAc3Plus(const bmkv_TrackEntry *track)
{
    BDBG_ASSERT(track);
    B_MKV_TEST_CODEC(track, "A_EAC3");
}

bool
bmkv_IsTrackAudioMpeg1Layer3(const bmkv_TrackEntry *track)
{
    BDBG_ASSERT(track);
    B_MKV_TEST_CODEC(track, "A_MPEG/L3");
}

bool
bmkv_IsTrackAudioMpeg1Layer1_2(const bmkv_TrackEntry *track)
{
    static const char a_mpeg_l1[] = "A_MPEG/L1";
    static const char a_mpeg_l2[] = "A_MPEG/L2";
    BDBG_ASSERT(track);
    return BKNI_Memcmp(track->CodecID, a_mpeg_l1, sizeof(a_mpeg_l1))==0
        || BKNI_Memcmp(track->CodecID, a_mpeg_l2, sizeof(a_mpeg_l2))==0;
}

bool
bmkv_IsTrackAuxiliary(const bmkv_TrackEntry *track)
{
    return track->TrackType==0x10 || track->TrackType==0x11 || track->TrackType==0x12 || track->TrackType==0x20;
}

bool
bmkv_IsTrackAudioACM(const bmkv_TrackEntry *track)
{
    BDBG_ASSERT(track);
    B_MKV_TEST_CODEC(track, "A_MS/ACM");
}

bool
bmkv_IsTrackAudioVorbis(const bmkv_TrackEntry *track)
{
    BDBG_ASSERT(track);
    B_MKV_TEST_CODEC(track, "A_VORBIS");
}

bool
bmkv_IsTrackVideoVp8(const bmkv_TrackEntry *track)
{
    BDBG_ASSERT(track);
    B_MKV_TEST_CODEC(track, "V_VP8");
}

bool
bmkv_IsTrackVideoVp9(const bmkv_TrackEntry *track)
{
    BDBG_ASSERT(track);
    B_MKV_TEST_CODEC(track, "V_VP9");
}

bool
bmkv_IsTrackAudioOpus(const bmkv_TrackEntry *track)
{
    BDBG_ASSERT(track);
    B_MKV_TEST_CODEC(track, "A_OPUS");
}

bool
bmkv_IsTrackAudioPcmInt(const bmkv_TrackEntry *track)
{
    BDBG_ASSERT(track);
    B_MKV_TEST_CODEC(track, "A_PCM/INT/LIT");
}

static const BMKV_PARSER_BEGIN(bmkv_TrackEntryVideoColourMasteringMetadata_parser)
    BMKV_PARSER_FIELD_FLOAT(bmkv_TrackEntryVideoColourMasteringMetadata, 0x55D1, PrimaryRChromaticityX)
    BMKV_PARSER_FIELD_FLOAT(bmkv_TrackEntryVideoColourMasteringMetadata, 0x55D2, PrimaryRChromaticityY)
    BMKV_PARSER_FIELD_FLOAT(bmkv_TrackEntryVideoColourMasteringMetadata, 0x55D3, PrimaryGChromaticityX)
    BMKV_PARSER_FIELD_FLOAT(bmkv_TrackEntryVideoColourMasteringMetadata, 0x55D4, PrimaryGChromaticityY)
    BMKV_PARSER_FIELD_FLOAT(bmkv_TrackEntryVideoColourMasteringMetadata, 0x55D5, PrimaryBChromaticityX)
    BMKV_PARSER_FIELD_FLOAT(bmkv_TrackEntryVideoColourMasteringMetadata, 0x55D6, PrimaryBChromaticityY)
    BMKV_PARSER_FIELD_FLOAT(bmkv_TrackEntryVideoColourMasteringMetadata, 0x55D7, WhitePointChromaticityX)
    BMKV_PARSER_FIELD_FLOAT(bmkv_TrackEntryVideoColourMasteringMetadata, 0x55D8, WhitePointChromaticityY)
    BMKV_PARSER_FIELD_FLOAT(bmkv_TrackEntryVideoColourMasteringMetadata, 0x55D9, LuminanceMax)
    BMKV_PARSER_FIELD_FLOAT(bmkv_TrackEntryVideoColourMasteringMetadata, 0x55DA, LuminanceMin)
BMKV_PARSER_END(bmkv_TrackEntryVideoColourMasteringMetadata_parser);

static void
bmkv_TrackEntryVideoColourMasteringMetadata_init(void *data)
{
    bmkv_TrackEntryVideoColourMasteringMetadata *metadata = data;

    BKNI_Memset(metadata, 0, sizeof(*metadata));
    return;
}

static const bmkv_parser_desc bmkv_TrackEntryVideoColourMasteringMetadata_desc = {
    "VideoColourMasteringMetadata",
    bmkv_TrackEntryVideoColourMasteringMetadata_parser,
    sizeof(bmkv_TrackEntryVideoColourMasteringMetadata),
    bmkv_TrackEntryVideoColourMasteringMetadata_init
};


static const BMKV_PARSER_BEGIN(bmkv_TrackEntryVideoColour_parser)
    BMKV_PARSER_FIELD_UNSIGNED(bmkv_TrackEntryVideoColour, 0x55B1, MatrixCoefficients)
    BMKV_PARSER_FIELD_UNSIGNED(bmkv_TrackEntryVideoColour, 0x55B2, BitsPerChannel)
    BMKV_PARSER_FIELD_UNSIGNED(bmkv_TrackEntryVideoColour, 0x55B3, ChromaSubsamplingHorz)
    BMKV_PARSER_FIELD_UNSIGNED(bmkv_TrackEntryVideoColour, 0x55B4, ChromaSubsamplingVert)
    BMKV_PARSER_FIELD_UNSIGNED(bmkv_TrackEntryVideoColour, 0x55B5, CbSubsamplingHorz)
    BMKV_PARSER_FIELD_UNSIGNED(bmkv_TrackEntryVideoColour, 0x55B6, CbSubsamplingVert)
    BMKV_PARSER_FIELD_UNSIGNED(bmkv_TrackEntryVideoColour, 0x55B7, ChromaSitingHorz)
    BMKV_PARSER_FIELD_UNSIGNED(bmkv_TrackEntryVideoColour, 0x55B8, ChromaSitingVert)
    BMKV_PARSER_FIELD_UNSIGNED(bmkv_TrackEntryVideoColour, 0x55B9, Range)
    BMKV_PARSER_FIELD_UNSIGNED(bmkv_TrackEntryVideoColour, 0x55BA, TransferCharacteristics)
    BMKV_PARSER_FIELD_UNSIGNED(bmkv_TrackEntryVideoColour, 0x55BB, Primaries)
    BMKV_PARSER_FIELD_UNSIGNED(bmkv_TrackEntryVideoColour, 0x55BC, MaxCLL)
    BMKV_PARSER_FIELD_UNSIGNED(bmkv_TrackEntryVideoColour, 0x55BD, MaxFALL)
    BMKV_PARSER_FIELD_TABLE(bmkv_TrackEntryVideoColour, 0x55D0, MasteringMetadata, bmkv_TrackEntryVideoColourMasteringMetadata_desc)
BMKV_PARSER_END(bmkv_TrackEntryVideoColour_parser);

static void
bmkv_TrackEntryVideoColour_init(void *data)
{
    bmkv_TrackEntryVideoColour *colour = data;

    BKNI_Memset(colour, 0, sizeof(*colour));
    colour->MatrixCoefficients = 2;
    colour->TransferCharacteristics = 2;
    colour->Primaries = 2;
    return;
}

const bmkv_parser_desc bmkv_TrackEntryVideoColour_desc = {
    "TrackEntryVideoColour",
    bmkv_TrackEntryVideoColour_parser,
    sizeof(bmkv_TrackEntryVideoColour),
    bmkv_TrackEntryVideoColour_init
};
