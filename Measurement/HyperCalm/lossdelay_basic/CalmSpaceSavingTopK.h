#ifndef _CSPACESAVINGTOPK_H_
#define _CSPACESAVINGTOPK_H_

#include "SpaceSavingTopK.h"

#define tail_node nodes[0].prev

class CalmSpaceSavingTopK : public SpaceSavingTopK
{
    pair<pair<uint32_t, uint16_t>, pair<char, float>> *q;//pair(key,count)
    int q_head;
    const int count_threshold, q_size;

    void append_new_key2(pair<uint32_t, uint16_t> key, pair<char, float> freq) {
        for(int i = 0; i < q_size; ++i)
            if(q[i].first==key){
                q[i].second = q[i].second + freq;
                if(q[i].second.first<count_threshold)return ;
                pair<uint32_t, uint16_t> old_key = tail_node->key;
                hash_table.erase(old_key);
                tail_node->key = key;
                tail_node->val = {0,0};
                hash_table[key] = tail_node;
                add_counter(tail_node, q[i].second);
                q[i]={{0,0},{0,0}};
                return ;
            }
        for(int i = 0; i < q_size; ++i)
        if(q[i].first.first == 0){
            q[i] = {key, freq};
            return ;
        }
        q[q_head] = {key, freq};
        q_head = (q_head + 1) % q_size;
    }
    void append_new_key(pair<uint32_t, uint16_t> key, pair<char, float> freq) {
        if (now_element < capacity) {
            append_new_key1(key, freq);
        } else {
            append_new_key2(key, freq);
        }
    }

public:
    CalmSpaceSavingTopK(int memory, int _count_threshold, int _q_size, int _time_or_size) : 
            SpaceSavingTopK(memory - _q_size * sizeof(pair<pair<uint32_t, uint16_t>,pair<char, float>>), _time_or_size),
            count_threshold(_count_threshold),q_size(_q_size){
        q = new pair<pair<uint32_t, uint16_t>,pair<char, float>> [q_size]{};
        q_head = 0;
    }
    ~CalmSpaceSavingTopK(){
        delete [] q;
    }
    int insert_calm(pair<uint32_t, uint16_t> key, pair<char, float> freq = {1,0}){
        auto itr = hash_table.find(key);
        if (itr == hash_table.end()) {
            // key not found
            append_new_key(key, freq);
        } else {
            // key found
            add_counter(itr->second, freq);
        }
        return 1;
    }
};
#undef tail_node

#endif //_CSPACESAVINGTOPK_H_