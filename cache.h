#include <stdint.h>

#ifndef __CACHE_H
#define __CACHE_H
typedef struct cache_t {
    int tag_width;
    int index_width;
    int ila_width;
    int offset;
    uint32_t *tag_array;
    long long unsigned int access_count;
    long long unsigned int hit_count;
} CACHE;

#endif

void cache_init(CACHE *cache, int index_width, int ila_width, int offset);
void cache_free(CACHE *cache);
void cache_access(unsigned int addr, CACHE *cache);
