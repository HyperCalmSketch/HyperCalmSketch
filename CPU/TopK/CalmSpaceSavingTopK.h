#ifndef _CSPACESAVINGTOPK_H_
#define _CSPACESAVINGTOPK_H_

#include "SpaceSavingTopK.h"

#define tail_node nodes[0].prev

class CalmSpaceSavingTopK : public SpaceSavingTopK
{
    pair<int,int> *q;//pair(key,count)
    int q_head;
const int count_threshold, q_size;

    void append_new_key2(uint32_t key, int freq) {
        for(int i = 0; i < q_size; ++i)
        if(q[i].first==key){
            ++q[i].second;
            if(q[i].second<count_threshold)return ;
            q[i]={0,0};
            uint32_t old_key = tail_node->key;
            hash_table.erase(old_key);
            tail_node->key = key;
            hash_table[key] = tail_node;
            add_counter(tail_node, count_threshold);
            return ;
        }
        for(int i = 0; i < q_size; ++i)
        if(q[i]==pair<int,int>(0,0)){
            q[i] = {key, 1};
            if(q[i].second<count_threshold)return ;
            q[i]={0,0};
            uint32_t old_key = tail_node->key;
            hash_table.erase(old_key);
            tail_node->key = key;
            hash_table[key] = tail_node;
            add_counter(tail_node, count_threshold);
            return ;
        }
        q[q_head] = {key, 1};
        q_head = (q_head + 1) % q_size;
    }
    void append_new_key(uint32_t key, int freq) {
        if (now_element < capacity) {
            append_new_key1(key, freq);
        } else {
            append_new_key2(key, freq);
        }
    }

public:
    CalmSpaceSavingTopK(int memory, int _count_threshold,int _q_size) : 
            SpaceSavingTopK(memory - _q_size * sizeof(pair<int,int>)),
            count_threshold(_count_threshold),q_size(_q_size){
        q = new pair<int,int> [q_size]{};
        q_head = 0;
    }
    ~CalmSpaceSavingTopK(){
        delete [] q;
    }
    void insert(uint32_t key, int freq = 1){
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

#endif //_CSPACESAVINGTOPK_H_