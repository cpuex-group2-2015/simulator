#include <stdlib.h>
#include <stdio.h>
#include "cache.h"
#include "util.h"

/*
 * Address
 *
 * width            n            m          offset
 *        | Tag | index | inner line addr | offset |
 *
 * line length = 2 ^ n
 * line size   = 2 ^ m WORDs
 *
 */

void cache_init(CACHE *cache, int index_width, int ila_width, int offset) {
    cache->index_width = index_width;
    cache->ila_width = ila_width;
    cache->offset = offset;
    cache->tag_width = 32 - (index_width + ila_width + offset);
    cache->tag_array = calloc(1 << index_width, sizeof(uint32_t));
    cache->access_count = 0;
    cache->hit_count = 0;
}

void cache_free(CACHE *cache) {
    free(cache->tag_array);
}

void cache_access(unsigned int addr, CACHE *cache) {
    uint32_t tag = DOWNTO(addr, 31, 32 - cache->tag_width);
    int index = DOWNTO(addr,
            32 - cache->tag_width - 1,
            32 - cache->tag_width - 1 - cache->index_width);

    uint32_t cached_tag = (cache->tag_array)[index];

    cache->access_count += 1;

    if (cached_tag & (1 << 31)) {
        cached_tag = cached_tag & ~(1 << 31);
        if (tag == cached_tag) {
            cache->hit_count += 1;
            /*
            fprintf(stderr, "O");
            */
            return;
        }
    }

    /* miss */
    cache->tag_array[index] = tag | (1 << 31);
    /* fprintf(stderr, "X"); */
}
