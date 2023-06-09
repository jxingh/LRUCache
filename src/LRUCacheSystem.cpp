#include "LRUCacheSystem.h"

#include "cache.h"
#include "hashtable.h"

#define Blocksize (4 * 1024)

const char *filepath_ = "./cache.txt";

extern Cache *NewLRUCache(size_t capacity);

LRUCacheSystem::LRUCacheSystem(const std::string filepath)
{
    const size_t kCacheCapacity = 16 * 4 * 1024;
    cache_ = NewLRUCache(kCacheCapacity);
    //    filepath_ = filepath.data();
    std::ifstream infile(filepath, std::ifstream::binary);
    infile.seekg(0, infile.end);
    file_size_ = infile.tellg();
    std::cout << file_size_ << std::endl;
    infile.close();
    Total_blocks_ = ((file_size_) + (Blocksize - 1)) / Blocksize;
    std::cout << Total_blocks_ << std::endl;
}

LRUCacheSystem::~LRUCacheSystem() { delete cache_; }

LRUNode *LRUCacheSystem::Seek(const uint32_t block_id)
{
    Cache::Node *n;
    if ((n = cache_->Lookup(std::to_string(block_id))) != nullptr)
    {
        //        cache_->Release(n);
        return reinterpret_cast<LRUNode *>(n);
    };
    if (block_id > Total_blocks_ || block_id <= 0)
    {
        std::cout << "block_id not exist! block id from 1 to " << Total_blocks_ << std::endl;
        return nullptr;
    }
    FILE *f = fopen(filepath_, "rb");
    if (f == NULL)
    {
        std::cout << "open file error! " << errno << std::endl;
        return nullptr;
    }
    char *pread = (char *)malloc(Blocksize);
    memset(pread, 0, Blocksize);
    fseek(f, Blocksize * (block_id - 1), SEEK_SET);
    fread(pread, sizeof(char), Blocksize, f);
    fclose(f);
    n = cache_->Insert(std::to_string(block_id), pread, Blocksize, nullptr);
    return reinterpret_cast<LRUNode *>(n);
}

void LRUCacheSystem::Release(LRUNode *n)
{
    cache_->Release(reinterpret_cast<Cache::Node *>(n));
}

LRUNode *LRUCacheSystem::Put(LRUNode *block, std::string s)
{
    if (block == nullptr)
    {
        std::cout << "block does not exist" << std::endl;
        return nullptr;
    }

    if (s.length() > Blocksize)
    {
        std::cout << "over size of block:" << Blocksize << "bytes" << std::endl;
        return nullptr;
    }
    Cache::Node *n = cache_->Insert(block->key(), &s, s.length(), nullptr);
    return reinterpret_cast<LRUNode *>(n);
}