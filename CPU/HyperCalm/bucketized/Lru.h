#ifndef _LRUSS_H_
#define _LRUSS_H_

#include "SpaceSaving.h"

#define Mod(x, y) (((x+y)%q_size+q_size)%q_size)
#define CELL_PER_DEQUE 32



class LRU : public SpaceSaving
{
    
protected:
    struct Buckets{
public:
        uint16_t keys[CELL_PER_DEQUE];
    };
    uint32_t *freqs;
    char *head_lru;
    Buckets * lrus;
    const int count_threshold, q_size, depth = CELL_PER_DEQUE;
    uint32_t value(uint32_t val, int i){
        return (val>>i)&1;
    }

    void change(uint32_t &val, int i, int j){
        val ^= (value(val, i) ^ j) << i;
    }
    void append_new_key(uint32_t key, uint16_t time, int freq = 1) {
        const int key_mod_n = Mod(key, time);
        uint16_t key_finger = (key + time) % 65535 + 1;
        uint32_t matched = 0;
        if constexpr(use_simd) {
            const __m512i item = _mm512_set1_epi16(key_finger);                                                                                                                                                                                             
            __m512i *keys_p = (__m512i*)(lrus[key_mod_n].keys);
            matched = _mm512_cmpeq_epi16_mask(item, *keys_p);
            if(matched != 0){
                int index = __tzcnt_u32(matched);
                SS_update(key, time, count_threshold);
                lrus[key_mod_n].keys[index] = 0;
                change(freqs[key_mod_n], index, 0);
                return ;
            }
        }
        else{
            for(int i=0; i<depth; ++i)
                if(lrus[key_mod_n].keys[i] == key_finger){
                    SS_update(key, time, count_threshold);
                    lrus[key_mod_n].keys[i] = 0;
                    change(freqs[key_mod_n], i, 0);
                    return ;
                }
        }
        matched = ~freqs[key_mod_n];
        if(matched != 0){
            int index = __tzcnt_u32(matched);
            change(freqs[key_mod_n], index, freq);
            lrus[key_mod_n].keys[index] = key_finger;
            return ;
        }
        lrus[key_mod_n].keys[head_lru[key_mod_n]] = key_finger;
        head_lru[key_mod_n] = (head_lru[key_mod_n] + 1)%depth;
    }

public:
    LRU(int memory1, int _count_threshold,int _q_mem) :
            q_size(_q_mem/(sizeof(Buckets)+sizeof(char)+sizeof(uint16_t))),
            SpaceSaving(memory1 - _q_mem),
            count_threshold(_count_threshold){
        lrus = new(std::align_val_t{64}) Buckets[q_size]; 
        freqs = new uint32_t[q_size];
        head_lru = new char[q_size];
        memset(lrus, 0, sizeof(Buckets)*q_size);
        memset(freqs, 0, sizeof(uint32_t)*q_size);
        memset(head_lru, 0, sizeof(char)*q_size);
    }
    ~LRU(){
        delete [] lrus;
        delete [] freqs;
        delete [] head_lru;
    }
    int insert_calm(uint32_t key, uint16_t time, int freq = 1){
        //printf("insert_lru\n");
        int i = find(key, time);
        if (i == 0) {
            // key not found
            append_new_key(key, time, freq);
        } else {
            // key found
            add_counter(key, time, i-1, freq);
        }
        return i;
    }
};
#undef Mod
#endif //_CDEQUESTOPK_H_