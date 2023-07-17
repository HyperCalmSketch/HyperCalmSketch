#ifndef _CALMSPACESAVINGBUCKET_H_
#define _CALMSPACESAVINGBUCKET_H_

#include "SpaceSavingTopK_Buckets.h"

#define Mod(x) ((x%q_size+q_size)%q_size)
#define CELL_PER_LRU 32

class CalmSpaceSavingTopK_Buckets : public SpaceSavingTopK_Buckets
{
protected:
    struct Buckets{
        uint16_t keys[CELL_PER_LRU];
    }* lrus;
    uint32_t *freq_lru;
    char *head_lru;
    const int count_threshold = 2, q_size, depth = CELL_PER_LRU;
    uint32_t value(uint32_t val, int i){
        return (val>>i)&1;
    }

    void change(uint32_t &val, int i, int j){
        val ^= (value(val, i) ^ j) << i;
    }
    void append_new_key(uint32_t key, int freq = 1) {
        const uint32_t key_mod_n = Mod(key);
        const uint16_t key_finger = (key/q_size) % 65535 + 1; 
        if(use_SIMD){
            const __m512i item = _mm512_set1_epi16(key_finger);                                                                                                                                                                                             
            __m512i *keys_p = (__m512i*)(lrus[key_mod_n].keys);
            const uint32_t matched = _mm512_cmpeq_epi16_mask(item, *keys_p);
            if(matched != 0){
                int index = __tzcnt_u32(matched);
                lrus[key_mod_n].keys[index] = 0;
                change(freq_lru[key_mod_n], index, 0);
                SS_update(key, count_threshold);
                return ;
            }
        }
        else{
            static int c1=0,c2=0;
            for(int i=0; i<depth; i++)
                if(lrus[key_mod_n].keys[i]==key_finger){
                    lrus[key_mod_n].keys[i] = 0;
                    change(freq_lru[key_mod_n], i, 0);
                    SS_update(key, count_threshold);
                    return;
                }
        }
        const uint32_t matched = ~freq_lru[key_mod_n];
        if(matched != 0){
            int index = __tzcnt_u32(matched);
            change(freq_lru[key_mod_n], index, freq);
            lrus[key_mod_n].keys[index] = key_finger;
            return;
        }

        lrus[key_mod_n].keys[head_lru[key_mod_n]] = key_finger;
        head_lru[key_mod_n] = (head_lru[key_mod_n] + 1)%depth;
        return ;
    }

public:
    CalmSpaceSavingTopK_Buckets(int memory,int _q_size, int _use_SIMD = 0) :
            q_size(_q_size/(sizeof(Buckets) + sizeof(int) + sizeof(char))),
            SpaceSavingTopK_Buckets(memory - _q_size, _use_SIMD){
        lrus = new(std::align_val_t{32}) Buckets[q_size]; 
        freq_lru = new uint32_t[q_size];
        head_lru = new char[q_size];
        memset(lrus, 0, sizeof(Buckets)*q_size);
        memset(freq_lru, 0, sizeof(int)*q_size);
        memset(head_lru, 0, sizeof(char)*q_size);
    }
    ~CalmSpaceSavingTopK_Buckets(){
        delete [] lrus;
        delete [] freq_lru;
        delete [] head_lru;
    }
    void insert(uint32_t key, int freq = 1){

        int i = find(key);
        if (i == 0) append_new_key(key, freq);
        else
            add_counter(key, i-1, freq);
    }
};
#undef Mod

#endif