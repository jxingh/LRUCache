#ifndef LRUCACHE_2_LRUCACHESYSTEM_H
#define LRUCACHE_2_LRUCACHESYSTEM_H

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>

#include "cache.h"
#include "hashtable.h"

class LRUCacheSystem
{
public:
    LRUCacheSystem(std::string filepath);
    ~LRUCacheSystem();
    LRUNode *Seek(const uint32_t block_id);
    LRUNode *Put(LRUNode *block, std::string s);

    void Release(LRUNode *n);
    LRUCacheSystem(const LRUCacheSystem &) = delete;
    void operator=(const LRUCacheSystem &) = delete;

public:
    uint32_t Total_blocks_;
    size_t file_size_;
    Cache *cache_;
    //    const char *filepath_;
};

#endif // LRUCACHE_2_LRUCACHESYSTEM_H
