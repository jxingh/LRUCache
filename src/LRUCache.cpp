#include "LRUCache.h"

#include <iostream>

#include "cache.h"

using namespace std;

Cache *NewLRUCache(size_t capacity) {
    return new ShardLRUCache(capacity);
}

LRUCache::LRUCache() : m_capacity(0), m_usage(0) {
    // Make empty circular linked list
    m_lru.next = &m_lru;
    m_lru.prev = &m_lru;
    m_in_use.next = &m_in_use;
    m_in_use.prev = &m_in_use;
}

LRUCache::~LRUCache() {
    // assert(m_in_use.next == &m_in_use);  // Error if caller has an unreleased handle
    for (LRUNode *n = m_lru.next; n != &m_lru;) {
        LRUNode *next = n->next;
        assert(n->in_cache);
        n->in_cache = false;
        assert(n->refs == 1);
        Unref(n);
        n = next;
    }
}

void LRUCache::Ref(LRUNode *n) {
    if (n->refs == 1 && n->in_cache) {  // If on lru list, move to in_use list.
        LRU_Remove(n);
        LRU_Append(&m_in_use, n);
    }
    n->refs++;
}

void LRUCache::Unref(LRUNode *n) {
    assert(n->refs > 0);
    n->refs--;
    if (n->refs == 0) {
        assert(!n->in_cache);
        //(*n->deleter)(n->key(), n->value);
        free(n);
    } else if (n->in_cache && n->refs == 1) {
        LRU_Remove(n);
        LRU_Append(&m_lru, n);
    }
}
// 把e插入到list前面
void LRUCache::LRU_Append(LRUNode *list, LRUNode *n) {
    // Make "n" newest entry by inserting just before m_lru
    n->next = list;
    n->prev = list->prev;
    n->prev->next = n;
    n->next->prev = n;
}

// Remove from list, but not free the memory
void LRUCache::LRU_Remove(LRUNode *n) {
    n->next->prev = n->prev;
    n->prev->next = n->next;
}

void LRUCache::FinishErase(LRUNode *n) {
    if (n != nullptr) {
        assert(n->in_cache);
        LRU_Remove(n);
        n->in_cache = false;
        m_usage -= n->charge;
        Unref(n);
    }
}

Cache::Node *LRUCache::Lookup(const string &key, uint32_t hash) {
    std::lock_guard<std::mutex> lock(m_mutex);
    LRUNode *n = m_table.Lookup(key, hash);
    if (n != nullptr) {
        Ref(n);
    }
    return reinterpret_cast<Cache::Node *>(n);
}

Cache::Node *LRUCache::Insert(const std::string &key, uint32_t hash, void *value, size_t charge,
                              void (*deleter)(const string &key, void *value)) {
    std::lock_guard<std::mutex> lock(m_mutex);

    LRUNode *n = reinterpret_cast<LRUNode *>(malloc(sizeof(LRUNode) - 1 + key.size()));
    n->value = value;
    n->deleter = deleter;
    n->charge = charge;
    n->hash = hash;
    n->key_length = key.size();
    n->in_cache = false;
    n->refs = 1;
    memcpy(n->key_data, key.data(), key.size());

    if (m_capacity > 0) {
        n->refs++;
        n->in_cache = true;
        LRU_Append(&m_in_use, n);
        m_usage += charge;
        FinishErase(m_table.Insert(n));
    } else {
        n->next = nullptr;
    }
    while (m_usage > m_capacity && m_lru.next != &m_lru) {
        LRUNode *oldest = m_lru.next;
        FinishErase(m_table.Remove(oldest->key(), oldest->hash));
    }
    return reinterpret_cast<Cache::Node *>(n);
}

void LRUCache::Earse(const string &key, uint32_t hash) {
    std::lock_guard<std::mutex> lock(m_mutex);
    FinishErase(m_table.Remove(key, hash));
}

void LRUCache::Release(Cache::Node *node) {
    std::lock_guard<std::mutex> lock(m_mutex);
    Unref(reinterpret_cast<LRUNode *>(node));
}

// for ShardLRUCache
ShardLRUCache::ShardLRUCache(size_t capacity) {
    const size_t size_per_shard = (capacity + (kNumShards - 1)) / kNumShards;
    for (int i = 0; i < kNumShards; i++) {
        m_shard[i].set_capacity(size_per_shard);
    }
}

Cache::Node *ShardLRUCache::Insert(const std::string &key, void *value, size_t charge,
                                   void (*deleter)(const std::string &key, void *value)) {
    uint32_t hash = HashString(key);
    uint32_t shard = Shard(hash);
    return m_shard[shard].Insert(key, hash, value, charge, deleter);
}

Cache::Node *ShardLRUCache::Lookup(const std::string &key) {
    uint32_t hash = HashString(key);
    uint32_t shard = Shard(hash);
    return m_shard[shard].Lookup(key, hash);
}

void ShardLRUCache::Release(Node *node) {
    LRUNode *n = reinterpret_cast<LRUNode *>(node);
    uint32_t shard = Shard(n->hash);
    m_shard[shard].Release(node);
}

void ShardLRUCache::Earse(const std::string &key) {
    uint32_t hash = HashString(key);
    uint32_t shard = Shard(hash);
    m_shard[shard].Earse(key, hash);
}