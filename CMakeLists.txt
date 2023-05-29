cmake_minimum_required(VERSION 3.15)
project(LRUCacheSystem)

set(CMAKE_BUILD_TYPE DEBUG)

add_executable(LRUCacheSystem main.cpp src/LRUCache.cpp src/LRUCache.h src/cache.h src/hashtable.cpp src/hashtable.h Murmur3/Murmur3.cpp Murmur3/Murmur3.h src/LRUCacheSystem.cpp src/LRUCacheSystem.h)
target_link_libraries(LRUCacheSystem pthread)