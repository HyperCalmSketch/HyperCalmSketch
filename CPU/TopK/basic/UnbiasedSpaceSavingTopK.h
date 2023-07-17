#ifndef _UNBIASEDSPACESAVINGTOPK_H_
#define _UNBIASEDSPACESAVINGTOPK_H_

#include "SpaceSavingTopK.h"
#include <random>

#define tail_node nodes[0].prev

class UnbiasedSpaceSavingTopK : public SpaceSavingTopK
{
    mt19937 rng;

    void append_new_key2(uint32_t key, int freq) {
        if(rng() % (tail_node->val+1) == 0){
            uint32_t old_key = tail_node->key;
            hash_table.erase(old_key);
            tail_node->key = key;
            hash_table[key] = tail_node;
        }
        add_counter(tail_node, freq);
    }
    void append_new_key(uint32_t key, int freq) {
        if (now_element < capacity) {
            append_new_key1(key, freq);
        } else {
            append_new_key2(key, freq);
        }
    }

public:
    UnbiasedSpaceSavingTopK(int memory) : 
        SpaceSavingTopK(memory),rng(123){
    }
    ~UnbiasedSpaceSavingTopK(){
    }

    void insert(uint32_t key, int freq = 1)
    {
        auto itr = hash_table.find(key);
        if (itr == hash_table.end()) {
            // key not found
            append_new_key(key, freq);
        } else {
            // key found
            add_counter(itr->second, freq);
        }
    }
};
#undef tail_node

#endif //_UNBIASEDSPACESAVINGTOPK_H_