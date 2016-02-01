/***************************************************************************
 *     (c)2011-2013 Broadcom Corporation
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
 **************************************************************************/
/* IMPLEMENTATION */

BDBG_FILE_MODULE(b_memory_regions);

void b_memory_region_set_init(b_memory_region_set *set)
{
    set->length = 0;
    set->last = 0;
    set->regions = NULL;
    return;
}

void b_memory_region_set_shutdown(b_memory_region_set *set)
{
    if(set->regions) {
        BKNI_Free(set->regions);
    }
    return;
}

static int b_memory_region_find(const b_memory_region_set *set, uint64_t base)
{
    unsigned i;
    for(i=0;i<set->last;i++) {
        if( set->regions[i].base > base) {
            return i;
        }
    }
    return -1;
}

static void b_memory_region_remove(b_memory_region_set *set, unsigned offset)
{
    BDBG_ASSERT(set->last>0);
    for(offset++;offset<set->last;offset++) {
        set->regions[offset-1] = set->regions[offset];
    }
    set->last --;
    return;
}

static int b_memory_region_insert(b_memory_region_set *set, unsigned offset, const b_memory_region *region)
{
    unsigned i;
    BDBG_ASSERT(offset<=set->last);
    if(set->length <= set->last) {
        unsigned new_length = set->length ? set->length * 2 : 1;
        b_memory_region *regions  = BKNI_Malloc(sizeof(*regions)*new_length);
        if(regions==NULL) {
            return -1;
        }
        for(i=0;i<set->last;i++) {
            regions[i] = set->regions[i];
        }
        if(set->regions) {
            BKNI_Free(set->regions);
        }
        set->regions = regions;
        set->length = new_length;
        BDBG_ASSERT(new_length>set->last);
    }
    if(offset<set->length && set->last < set->length ) {
        for(i=set->last;i>offset;i--) {
            set->regions[i] = set->regions[i-1];
        }
        set->regions[offset] = *region;
        set->last++;
    } else {
        BDBG_ASSERT(0);
    }
    return 0;
}

/* 'b' inside 'a' */
static bool b_memory_region_contains(const b_memory_region *a, const b_memory_region *b)
{
    return (b->base >= a->base) && ((b->base+b->length) <= (a->base + a->length));
}

static bool b_memory_region_intersects(const b_memory_region *a, const b_memory_region *b)
{
    return ((a->base + a->length) >= b->base) && ((b->base + b->length) >= a->base);
}

static void b_memory_region_merge(const b_memory_region *a, const b_memory_region *b, b_memory_region *c)
{
    uint64_t last_a = a->base + a->length;
    uint64_t last_b = b->base + b->length;
    uint64_t last = last_a > last_b ? last_a : last_b;
    c->base = a->base < b->base ? a->base : b->base;
    c->length = last - c->base;
    return;
}

bool b_memory_region_match(const b_memory_region_set *set, const b_memory_region *region)
{
    int match = b_memory_region_find(set, region->base);
    if(match<0) {
        match = set->last;
        if(match<=0) {
            return false;
        }
    }

    if(match>0) {
        match--;
    }
    return b_memory_region_contains(&set->regions[match], region);
}

#if 0
static void b_memory_region_compact(b_memory_region_set *set)
{
    unsigned i;
    for(i=0;i+1<set->last;i++) {
        if(set->regions[i].base + set->regions[i].length == set->regions[i+1].base) {
            set->regions[i].length += set->regions[i+1].length;
            b_memory_region_remove(set, i+1);
        }
    }
    return;
}
#endif

static int b_memory_region_add_impl(b_memory_region_set *set, const b_memory_region *region)
{
    int match = b_memory_region_find(set, region->base);
    if(match<0) { /* insert/update at the tail */
        if(set->last) {
            if(b_memory_region_intersects( &set->regions[set->last-1], region)) {
                b_memory_region new_region;
                b_memory_region_merge(region, &set->regions[set->last-1], &new_region);
                set->regions[set->last-1] = new_region;
                return 0;
            }
        }
        if(b_memory_region_insert(set, set->last, region)<0) {
            return -1;
        }
        return 0;
    } else {
        unsigned i;
        b_memory_region new_region = *region;
        if(match>0) {
            if(b_memory_region_intersects(&set->regions[match-1], &new_region)) {
                match--;
            }
        }
        for(i=match;i<set->last;) {
            b_memory_region merged_region;
            if(!b_memory_region_intersects(&set->regions[i], &new_region)) {
                break;
            }
            b_memory_region_merge(&set->regions[i], &new_region, &merged_region);
            new_region = merged_region;
            b_memory_region_remove(set, i);
        }
        if(b_memory_region_insert(set, i, &new_region)<0) {
            return -1;
        }
        return 0;
    }
}

static int b_memory_region_delete_impl(b_memory_region_set *set, const b_memory_region *region)
{
    int match = b_memory_region_find(set, region->base);
    b_memory_region old_region;
    b_memory_region new_region;

    if(match>0) {
        match--;
    } else if(match<0) {
        if(set->last==0) {
            return -1;
        } else {
            match = set->last - 1;
        }
    }
    old_region = set->regions[match];
    if(!b_memory_region_contains(&old_region, region)) {
        return -1;
    }
    b_memory_region_remove(set, match);
    if(region->base != old_region.base) {
        BDBG_ASSERT(region->base > old_region.base);
        new_region.base = old_region.base;
        new_region.length = region->base - old_region.base;
        if(b_memory_region_insert(set, match, &new_region)<0) {
            return -1;
        }
        match++;
    }
    if(region->base + region->length != old_region.base + old_region.length) {
        new_region.base = region->base + region->length;
        BDBG_ASSERT(old_region.base + old_region.length >= new_region.base);
        new_region.length = (old_region.base + old_region.length) - new_region.base;
        if(b_memory_region_insert(set, match, &new_region)<0) {
            return -1;
        }
    }
    return 0;
}

int b_memory_region_add(b_memory_region_set *set, const b_memory_region *region)
{
    int rc = b_memory_region_add_impl(set, region);
    BDBG_MODULE_MSG(b_memory_regions, ("b_memory_region_add%p(%u,%u) %#x,%u -> %d", set, set->last, set->length, (unsigned)region->base,(unsigned)region->length, rc));
    return rc;
}

int b_memory_region_delete(b_memory_region_set *set, const b_memory_region *region)
{
    int rc = b_memory_region_delete_impl(set, region);
    BDBG_MODULE_MSG(b_memory_regions, ("b_memory_region_delete: %p(%u,%u) %#x,%u -> %d", set, set->last, set->length, (unsigned)region->base,(unsigned)region->length, rc));
    return rc;
}


/* Unit test support */
static void b_memory_region_dump(const b_memory_region_set *set)
{
    if(0) {
        unsigned i;
        BDBG_LOG(("length:%u last:%u", set->length, set->last));
        for(i=0;i<set->last;i++) {
            BDBG_LOG(("{%u,%u}", (unsigned)set->regions[i].base, (unsigned)set->regions[i].length));
        }
    }
    return;
}
#define TEST_RESULT(result)
#define TEST_ADD(range,result) { int rc; BDBG_LOG(("TEST at %u", __LINE__)); rc = b_memory_region_add(&set, &range); BDBG_ASSERT(rc==0);b_memory_region_dump(&set);BDBG_ASSERT(set.last==sizeof(result)/sizeof(result[0]));BDBG_ASSERT(0==BKNI_Memcmp(result, set.regions, sizeof(result)));}
#define TEST_DELETE(range,result) { int rc; BDBG_LOG(("TEST at %u", __LINE__)); rc = b_memory_region_delete(&set, &range); BDBG_ASSERT(rc==0);b_memory_region_dump(&set);BDBG_ASSERT(set.last==sizeof(result)/sizeof(result[0]));BDBG_ASSERT(0==BKNI_Memcmp(result, set.regions, sizeof(result)));}
#define TEST_MATCH(base,len,result) { b_memory_region  range = {base,len}; bool rc; BDBG_LOG(("TEST at %u", __LINE__)); rc = b_memory_region_match(&set, &range); BDBG_ASSERT(rc==result);}
static void b_memory_region_unit_test(void)
{
    b_memory_region_set set;
    b_memory_region_set_init(&set);
    b_memory_region_set_shutdown(&set);
    {
        b_memory_region  range = {100,10};
        b_memory_region  result[] = {{100,10}};
        TEST_ADD(range,result);
    }
    {
        b_memory_region  range = {130,20};
        b_memory_region  result[] = {{100,10},{130,20}};
        TEST_ADD(range,result);
    }
    {
        b_memory_region  range = {150,20};
        b_memory_region  result[] = {{100,10},{130,40}};
        TEST_ADD(range,result);
    }
    {
        b_memory_region  range = {110,20};
        b_memory_region  result[] = {{100,70}};
        TEST_ADD(range,result);
    }
    {
        b_memory_region  range = {50,10};
        b_memory_region  result[] = {{50,10}, {100,70}};
        TEST_ADD(range,result);
    }
    {
        b_memory_region  range = {70,5};
        b_memory_region  result[] = {{50,10}, {70,5}, {100,70}};
        TEST_ADD(range,result);
    }
    {
        b_memory_region  range = {60,5};
        b_memory_region  result[] = {{50,15}, {70,5}, {100,70}};
        TEST_ADD(range,result);
    }
    {
        b_memory_region  range = {75,5};
        b_memory_region  result[] = {{50,15}, {70,10}, {100,70}};
        TEST_ADD(range,result);
    }
    {
        b_memory_region  range = {95,5};
        b_memory_region  result[] = {{50,15}, {70,10}, {95,75}};
        TEST_ADD(range,result);
    }
    {
        b_memory_region  range = {80,15};
        b_memory_region  result[] = {{50,15}, {70,100}};
        TEST_ADD(range,result);
    }

    {
        b_memory_region  range = {80,15};
        b_memory_region  result[] = {{50,15}, {70,10}, {95,75}};
        TEST_DELETE(range,result);
    }
    {
        b_memory_region  range = {95,5};
        b_memory_region  result[] = {{50,15}, {70,10}, {100,70}};
        TEST_DELETE(range,result);
    }
    {
        b_memory_region  range = {75,5};
        b_memory_region  result[] = {{50,15}, {70,5}, {100,70}};
        TEST_DELETE(range,result);
    }
    {
        b_memory_region  range = {60,5};
        b_memory_region  result[] = {{50,10}, {70,5}, {100,70}};
        TEST_DELETE(range,result);
    }
    {
        b_memory_region  range = {70,5};
        b_memory_region  result[] = {{50,10}, {100,70}};
        TEST_DELETE(range,result);
    }
    {
        b_memory_region  range = {50,10};
        b_memory_region  result[] = {{100,70}};
        TEST_DELETE(range,result);
    }
    {
        b_memory_region  range = {110,20};
        b_memory_region  result[] = {{100,10},{130,40}};
        TEST_DELETE(range,result);
    }
    {
        b_memory_region  range = {150,20};
        b_memory_region  result[] = {{100,10},{130,20}};
        TEST_DELETE(range,result);
    }
    {
        b_memory_region  range = {130,20};
        b_memory_region  result[] = {{100,10}};
        TEST_DELETE(range,result);
    }
    if(0){
        b_memory_region  range = {100,10};
        b_memory_region  result[] = {{100,10}};
        TEST_ADD(range,result);
    }
    TEST_MATCH(100,10,true);
    TEST_MATCH(105,1,true);
    TEST_MATCH(104,6,true);
    TEST_MATCH(104,7,false);

    return;
}

void b_memory_region_unit_test_run(void)
{
    if(0) {
        b_memory_region_unit_test();
    }
}
