#include <pthread.h>
#include <stdlib.h>
#include <time.h>

#include <algorithm> //for std::generate_n
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "src/LRUCache.h"
#include "src/LRUCacheSystem.h"

#define NUM_THREADS 8
using namespace std;

void *test(LRUCacheSystem *s)
{
    int count = 0;
    for (int i = 0; i < s->Total_blocks_; i++)
    {
        int count = 0;
        LRUNode *n = s->Seek(rand() % 15 + 1);
        s->Put(n, "block1");
        s->Release(n);

        LRUNode *n2 = s->Seek(rand() % 15 + 1);
        s->Put(n2, "block2");
        s->Release(n2);

        LRUNode *n3 = s->Seek(rand() % 15 + 1);
        s->Put(n3, "block3");
        s->Release(n3);
        count += 3;
    }
    cout << count << endl;

    return 0;
}

int main(int argc, char **argv)
{
    LRUCacheSystem *s = new LRUCacheSystem("./cache.txt");
    vector<std::thread> threads;
    clock_t start, end;
    start = clock();
    for (int i = 0; i < 9; i++)
    {
        threads.push_back(std::thread(test, std::ref(s)));
    }
    for (auto &th : threads)
    {
        th.join();
    }
    end = clock();
    double time = double(end - start) / CLOCKS_PER_SEC;
    cout << "time = " << time << "s" << endl;
    delete s;
    return 0;
}