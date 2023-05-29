#ifndef SKIPLIST_H
#define SKIPLIST_H

#include <ctime>
#include <initializer_list>
#include <iostream>
#include <random>

template <typename Key>
class Skiplist {
public:
    struct Node {
        Node(Key k) : key(k) {}
        Key key;
        Node* next[1];  // C语言中的柔性数组技巧
    };

private:
    int maxLevel;
    Node* head;

    enum { kMaxLevel = 12 };

public:
    Skiplist() : maxLevel(1) {
        head = newNode(0, kMaxLevel);
    }

    Skiplist(std::initializer_list<Key> init) : Skiplist() {
        for (const Key& k : init) {
            insert(k);
        }
    }

    ~Skiplist() {
        Node* pNode = head;
        Node* delNode;
        while (nullptr != pNode) {
            delNode = pNode;
            pNode = pNode->next[0];
            free(delNode);  // 对应malloc
        }
    }

    // 禁止拷贝构造和赋值
    Skiplist(const Skiplist&) = delete;
    Skiplist& operator=(const Skiplist&) = delete;
    Skiplist& operator=(Skiplist&&) = delete;

private:
    Node* newNode(const Key& key, int level) {
        /*
         * 开辟sizeof(Node) + sizeof(Node*) * (level - 1)大小的空间
         * sizeof(Node*) * (level - 1)大小的空间是给Node.next[1]指针数组用的
         * 为什么是level-1而不是level，因为sizeof(Node)已包含一个Node*指针的空间
         */
        // void* node_memory = malloc(sizeof(Node) + sizeof(Node*) * (level - 1));
        // Node* node = new (node_memory) Node(key);
        Node* node = reinterpret_cast<Node*>(malloc(sizeof(Node) + sizeof(Node*) * (level - 1)));
        node->key = key;
        for (int i = 0; i < level; ++i) {
            node->next[i] = nullptr;
        }
        return node;
    }
    /*
     * 随机函数，范围[1, kMaxLevel]，越小概率越大
     */
    static int randomLevel() {
        int level = 1;
        while (rand() % 2 && level < kMaxLevel) {
            level++;
        }
        return level;
    }

public:
    Node* find(const Key& key) {
        // 从最高层开始查找，每层查找最后一个小于key的前继节点，不断缩小范围
        Node* pNode = head;
        for (int i = maxLevel - 1; i >= 0; --i) {
            while (pNode->next[i] != nullptr && pNode->next[i]->key < key) {
                pNode = pNode->next[i];
            }
        }

        // 如果第一层的pNode[0]->key == key，则返回pNode->next[0]，即找到key
        if (nullptr != pNode->next[0] && pNode->next[0]->key == key)
            return pNode->next[0];

        return nullptr;
    }

    void insert(const Key& key) {
        int level = randomLevel();
        Node* new_node = newNode(key, level);
        Node* prev[kMaxLevel];
        Node* pNode = head;
        // 从最高层开始查找，每层查找最后一个小于key的前继节点
        for (int i = level - 1; i >= 0; --i) {
            while (pNode->next[i] != nullptr && pNode->next[i]->key < key) {
                pNode = pNode->next[i];
            }
            prev[i] = pNode;
        }
        // 然后每层将新节点插入到前继节点后面
        for (int i = 0; i < level; ++i) {
            new_node->next[i] = prev[i]->next[i];
            prev[i]->next[i] = new_node;
        }

        if (maxLevel < level)  // 层数大于最大层数，更新最大层数
            maxLevel = level;
    }

    void erase(const Key& key) {
        Node* prev[maxLevel];
        Node* pNode = head;
        // 从最高层开始查找，每层查找最后一个小于key的前继节点
        for (int i = maxLevel - 1; i >= 0; --i) {
            while (pNode->next[i] != nullptr && pNode->next[i]->key < key)
                pNode = pNode->next[i];
            prev[i] = pNode;
        }

        // 如果找到key，
        if (pNode->next[0] != nullptr && pNode->next[0]->key == key) {
            Node* delNode = pNode->next[0];
            // 从最高层开始，如果当前层的next节点的值等于key，则删除next节点
            for (int i = maxLevel - 1; i >= 0; --i) {
                if (prev[i]->next[i] != nullptr && key == prev[i]->next[i]->key)
                    prev[i]->next[i] = prev[i]->next[i]->next[i];
            }
            free(delNode);  // 最后销毁pNode->next[0]节点
        }

        // 如果max_level>1且头结点的next指针为空，则该层已无数据，max_level减一
        while (maxLevel > 1 && head->next[maxLevel] == nullptr) {
            maxLevel--;
        }
    }
};

#endif