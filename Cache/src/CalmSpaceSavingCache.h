#ifndef _CSPACESAVINGC_H_
#define _CSPACESAVINGC_H_

#include "../../CPU/HyperCalm/CalmSpaceSaving.h"

using namespace std;

#define tail_node SS_nodes[0].val_prev

class CalmSpaceSavingCache : public CalmSpaceSaving
{
public:
    CalmSpaceSavingCache (double _time_threshold, double _unit_time,
		int memory, int _count_threshold, int q_size,
		int _circular_array_size):CalmSpaceSaving(_time_threshold, _unit_time,
            memory, _count_threshold, q_size, _circular_array_size){}
    bool make_sure(uint32_t key, float time){
        auto itr = hash_table.find(key);
        if (itr == hash_table.end())return 0;
        int16_t delta = (time - itr->second.last_batch_time) / unit_time;
        for(SS_Node *p = itr->second.first_SS_node; p; p = p->key_next)
        if(p->delta == delta)
            return 1;
        return 0;
    }
    float insert(uint32_t key, float time, bool bf_new, int freq = 1){
        auto itr = hash_table.find(key);
        if (itr == hash_table.end()) {
            // key not found
            if(!bf_new) return -1;
            array_push(key);
            hash_table[key] = Info(time, time);
            if(now_element < capacity)
                append_new_key(key, -1, time, 0, hash_table[key]);
            return -1;
        } else {
            itr->second.count ++;//array count++
            // key found
            array_push(key);
            if(time - itr->second.last_item_time < time_threshold){
                itr->second.last_item_time = time;
                return -1;
            }
            float alb=time - itr->second.last_batch_time;
            int16_t delta = alb / unit_time;
            alb=time+alb;
            itr->second.last_item_time = itr->second.last_batch_time = time;
            for(SS_Node *p = itr->second.first_SS_node; p; p = p->key_next)
            if(p->delta == delta){
                add_counter(p, freq);
                return alb;
            }
            for(LRU_Node *p = itr->second.first_LRU_node, *prev_p = nullptr; p; p = p->next){
                if(p->delta == delta){
                    if(++(p->count) >= count_threshold){
                        p->key = p->count = 0;
                        if(p == itr->second.first_LRU_node)
                            itr->second.first_LRU_node = p->next;
                        else 
                            prev_p->next = p->next;
                        replace_new_key(key, delta, time);
                        tail_node->key_next = itr->second.first_SS_node;
                        itr->second.first_SS_node = tail_node;
                        add_counter(tail_node, count_threshold);
                        return alb;
                    }
                    return -1;
                }
                prev_p = p;
            }
            append_new_key(key, delta, time, freq, itr->second);
            return -1;
        }
    }
};
#undef tail_node

#endif //_CSPACESAVINGC_H_
