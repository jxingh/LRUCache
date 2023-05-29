#ifndef __LEI_LRU_CACHE_LRU_CACHE_H__
#define __LEI_LRU_CACHE_LRU_CACHE_H__
#include <mutex>

#include "../Murmur3/Murmur3.h"
#include "cache.h"
#include "hashtable.h"

extern Cache *NewLRUCache(size_t capacity);

class LRUCache {
public:
    LRUCache();
    ~LRUCache();
    void set_capacity(size_t capacity) { m_capacity = capacity; }
    Cache::Node *Insert(const std::string &key, uint32_t hash, void *value,
                        size_t charge,
                        void (*deleter)(const std::string &key, void *value));
    Cache::Node *Lookup(const std::string &key, uint32_t hash);
    void Earse(const std::string &key, uint32_t hash);
    void Release(Cache::Node *node);

private:
    size_t m_capacity;
    // m_mutex protects the following state
    std::mutex m_mutex;
    size_t m_usage;
    // size_t m_last_id;
    // Dummy head of LRU list.
    // lru.prev is newest entry, lru.next is oldest entry.
    LRUNode m_lru;
    LRUNode m_in_use;
    HashTable m_table;

private:
    void LRU_Append(LRUNode *list, LRUNode *n);
    void LRU_Remove(LRUNode *n);
    void Ref(LRUNode *n);
    void Unref(LRUNode *n);
    void FinishErase(LRUNode *n);
};

// use multi shards to reduce race condition
static const int kNumShardBits = 4;
static const int kNumShards = 1 << kNumShardBits;  // 2^4 = 16
static uint32_t kSeed = 42;                        /* Seed value for hash */

class ShardLRUCache : public Cache {
private:
    LRUCache m_shard[kNumShards];

    // 哈希分片
    static inline uint32_t HashString(const std::string &s) {
        uint32_t hash = 0;
        MurmurHash3_x86_32(s.c_str(), s.size(), kSeed, &hash);
        return hash;
    }

    static uint32_t Shard(uint32_t hash) {
        // hash / 2^(32 - kNumShardBits), so shard will be [0-16)
        return hash >> (32 - kNumShardBits);
    }

public:
    explicit ShardLRUCache(size_t capacity);
    virtual ~ShardLRUCache() {}

    virtual Cache::Node *Insert(const std::string &key, void *value,
                                size_t charge,
                                void (*deleter)(const std::string &key,
                                                void *value));
    virtual Cache::Node *Lookup(const std::string &key);
    virtual void Release(Node *n);
    virtual void Earse(const std::string &key);
};

#endif
