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
        float freq2[CELL_PER_DEQUE];
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
    void append_new_key(uint32_t key, uint16_t time, int freq1 = 1, long double freq2 = 1) {
        const int key_mod_n = Mod(key, time);
        const uint16_t key_finger = (key + time) % 65535 + 1;
        int index = -1;
        for(int i=0; i<depth; ++i)
            if(lrus[key_mod_n].keys[i] == key_finger){
                if(freq1 && value(freqs[key_mod_n], i)){
                    SS_update(key, time, count_threshold, lrus[key_mod_n].freq2[i]+freq2);
                    lrus[key_mod_n].keys[i] = 0;
                    lrus[key_mod_n].freq2[i] = 0;
                    change(freqs[key_mod_n], i, 0);
                    return;
                }
                change(freqs[key_mod_n], i, value(freqs[key_mod_n], i) + freq1);
                lrus[key_mod_n].freq2[i] += freq2;
                return ;
            }
            else if(index == -1 && !lrus[key_mod_n].keys[i])
                index = i;
        if(index != -1){
            lrus[key_mod_n].keys[index] = key_finger;
            lrus[key_mod_n].freq2[index] = freq2;
            change(freqs[key_mod_n], index, freq1);
            return ;
        }
        lrus[key_mod_n].keys[head_lru[key_mod_n]] = key_finger;
        lrus[key_mod_n].freq2[head_lru[key_mod_n]] = freq2;
        change(freqs[key_mod_n], head_lru[key_mod_n], freq1);
        head_lru[key_mod_n] = (head_lru[key_mod_n] + 1)%depth;
    }

public:
    LRU(int memory1, int _count_threshold,int _q_mem,int _time_or_num) :
            q_size(_q_mem/(sizeof(Buckets)+sizeof(char)+sizeof(int))),
            SpaceSaving(memory1 - _q_mem, _time_or_num),
            count_threshold(_count_threshold){
        lrus = new Buckets[q_size]; 
        freqs = new uint32_t[q_size];
        head_lru = new char[q_size];
        memset(lrus, 0, sizeof(Buckets)*q_size);
        memset(freqs, 0, sizeof(int)*q_size);
        memset(head_lru, 0, sizeof(char)*q_size);
    }
    ~LRU(){
        delete [] lrus;
        delete [] freqs;
        delete [] head_lru;
    }
    int insert_calm(uint32_t key, uint16_t time, int freq1 = 1, long double freq2 = 1){
        //printf("insert_lru\n");
        int i = find(key, time);
        if (i == 0) {
            // key not found
            append_new_key(key, time, freq1, freq2);
        } else {
            // key found
            add_counter(key, time, i-1, freq1, freq2);
        }
        return i;
    }
};
#undef Mod
#endif //_CDEQUESTOPK_H_