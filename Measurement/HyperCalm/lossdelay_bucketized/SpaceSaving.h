#ifndef _SPACESAVING_H_
#define _SPACESAVING_H_

#include<queue>
#include<cmath>
#include <immintrin.h>

#define CELL_PER_BUCKETS 16

#define Mod1(x, y) (((x + y)%capacity+capacity)%capacity)
class SpaceSaving
{
protected:
    struct Bucket{
        uint32_t keys[CELL_PER_BUCKETS];
        uint32_t freq1[CELL_PER_BUCKETS];
        float freq2[CELL_PER_BUCKETS];
        uint16_t delta[CELL_PER_BUCKETS];
    };
    const uint32_t capacity;
    const int length = CELL_PER_BUCKETS;
    const int time_or_num; //0 means time, 1 means num
    Bucket * buckets;
    int find(uint32_t key, uint16_t time){
        const int key_mod_n = Mod1(key, time);
        int j = -1;
        for(int i=0; i<length; ++i)
            if(buckets[key_mod_n].keys[i] == key && buckets[key_mod_n].delta[i] == time)
                return i + 1;
            else if(!buckets[key_mod_n].keys[i])
                j = i;
        return j+1;
    }
    void add_counter(uint32_t key, uint16_t time, int i, int freq1 = 1, long double freq2 = 0){
        const int key_mod_n = Mod1(key, time);
        buckets[key_mod_n].freq1[i] += freq1;
        buckets[key_mod_n].freq2[i] += freq2;
        buckets[key_mod_n].delta[i] = time;
        buckets[key_mod_n].keys[i] = key;
    }
    void SS_update(uint32_t key, uint16_t time, int freq1 = 1, long double freq2 = 0){
        const int key_mod_n = Mod1(key, time);
        int index = 0;
        for(int i=1; i<length && buckets[key_mod_n].freq1[index]; i++)
            if(buckets[key_mod_n].freq1[i] < buckets[key_mod_n].freq1[index])
                index = i;
        long double sum = 1.0L*buckets[key_mod_n].freq1[index] * freq2 + time_or_num;
        buckets[key_mod_n].freq1[index] = freq1;
        buckets[key_mod_n].freq2[index] = freq2;
        buckets[key_mod_n].delta[index] = time;
        buckets[key_mod_n].keys[index] = key;
    }
public:
    SpaceSaving(int memory2, int _time_or_num = 0) : capacity(memory2/sizeof(Bucket)),time_or_num(_time_or_num){
        buckets = new Bucket [capacity];
        memset(buckets, 0, capacity*sizeof(Bucket));
    }
    ~SpaceSaving(){
        delete [] buckets;
    }
    vector<pair<pair<int, int16_t>, pair<int, double>>> get_top_k(int k) const{
        vector<pair<pair<int, int16_t>, pair<int, double>>> ans;
        priority_queue<pair<pair<int, double>, pair<int, int16_t> >,
                        vector<pair<pair<int, double>, pair<int, int16_t>>>,
                        greater<pair<pair<int, double>, pair<int, int16_t>>>> Q;
        for(int i=0; i<capacity; ++i)
            for(int j=0; j<length; ++j){
                if(buckets[i].freq1[j]){
                    Q.push({{buckets[i].freq1[j], buckets[i].freq2[j]}, 
                                     {buckets[i].keys[j],buckets[i].delta[j]}});   
                    if(Q.size() > k) Q.pop();
                }
            }
        int Max=0;
        while(Q.size()){
            pair<int, int16_t> key = Q.top().second;
            pair<int, double> count = Q.top().first;
            ans.push_back({key, count});
            Q.pop();
        }
        return ans; 
    }
};
#endif //_SPACESAVINGTOPK_H_