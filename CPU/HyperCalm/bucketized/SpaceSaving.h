#ifndef _SPACESAVING_H_
#define _SPACESAVING_H_

#include<queue>
#include<cmath>
#include <immintrin.h>

#define CELL_PER_BUCKETS 16

#define Mod1(x, y) (((x + y)%capacity+capacity)%capacity)
class SpaceSaving
{
public:
#ifdef SIMD
    static constexpr bool use_simd = true;
#else
    static constexpr bool use_simd = false;
#endif
protected:
    struct Bucket{
        uint32_t keys[CELL_PER_BUCKETS];
        uint32_t freq[CELL_PER_BUCKETS];
        uint16_t delta[CELL_PER_BUCKETS];
        uint16_t delta1[CELL_PER_BUCKETS];
        uint32_t freq1[CELL_PER_BUCKETS];
        uint32_t keys1[CELL_PER_BUCKETS];
    };
    const uint32_t capacity;
    const int length = CELL_PER_BUCKETS;
    Bucket * buckets;
    int find(uint32_t key, uint16_t time){
        const int key_mod_n1 = Mod1(key, time), key_mod_n = key_mod_n1/2;
        if(key_mod_n1&1){
            if constexpr(use_simd) {
                const __m512i item = _mm512_set1_epi32(key);                                                                                                                                                                                             
                __m512i *keys_p = (__m512i*)(buckets[key_mod_n].keys1);
                int matched = 0;
                matched = _mm512_cmpeq_epi32_mask(item, *keys_p);
                while(matched != 0){
                    int index = _tzcnt_u32(matched);
                    if(buckets[key_mod_n].delta1[index] == time)
                        return 1 + index;
                    matched ^= matched&-matched;
                }
            }
            else{
                for(int i=0; i<length; ++i)
                    if(buckets[key_mod_n].keys1[i] == key && buckets[key_mod_n].delta1[i] == time)
                        return i + 1;
            }
        }
        else{
            if constexpr(use_simd) {
                const __m512i item = _mm512_set1_epi32(key);                                                                                                                                                                                             
                __m512i *keys_p = (__m512i*)(buckets[key_mod_n].keys);
                int matched = 0;
                matched = _mm512_cmpeq_epi32_mask(item, *keys_p);
                while(matched != 0){
                    int index = _tzcnt_u32(matched);
                    if(buckets[key_mod_n].delta[index] == time)
                        return 1 + index;
                    matched ^= matched&-matched;
                }
            }
            else{
                for(int i=0; i<length; ++i)
                    if(buckets[key_mod_n].keys[i] == key && buckets[key_mod_n].delta[i] == time)
                        return i + 1;
            }
        }
        return 0;
    }
    void add_counter(uint32_t key, uint16_t time, int i, int freq = 1){
        const int key_mod_n1 = Mod1(key, time), key_mod_n = key_mod_n1/2;
        if(key_mod_n1&1) buckets[key_mod_n].freq1[i] += freq;
        else buckets[key_mod_n].freq[i] += freq;
    }
    void SS_update(uint32_t key, uint16_t time, int freq = 1){
        const int key_mod_n1 = Mod1(key, time), key_mod_n = key_mod_n1/2;
        int index = 0;
        if(key_mod_n1&1){
            if constexpr(use_simd) {
                const __m512 *freqs = (__m512*)(buckets[key_mod_n].freq1);
                
                __m256i low_256 = _mm256_castps_si256(_mm512_extractf32x8_ps(*freqs, 0));
                __m256i high_256 = _mm256_castps_si256(_mm512_extractf32x8_ps(*freqs, 1));
                __m256i x_256 = _mm256_min_epi32(low_256, high_256);

                __m128i low_128 = _mm_castps_si128(_mm256_extractf128_ps((__m256)x_256, 0));
                __m128i high_128 = _mm_castps_si128(_mm256_extractf128_ps((__m256)x_256, 1));
                __m128i x_128 = _mm_min_epi32(low_128, high_128);

                __m128i min1 = _mm_shuffle_epi32(x_128, _MM_SHUFFLE(0,0,3,2));
                __m128i min2 = _mm_min_epi32(x_128, min1);
                __m128i min3 = _mm_shuffle_epi32(min2, _MM_SHUFFLE(0,0,0,1));
                __m128i min4 = _mm_min_epi32(min2, min3);
                int min_freq = _mm_cvtsi128_si32(min4);
                
                const __m512i freq_item = _mm512_set1_epi32(min_freq);
                int freq_matched = _mm512_cmpeq_epi32_mask(freq_item, (__m512i)*freqs);
                index = __tzcnt_u32((uint32_t)freq_matched);
            }
            else{
                for(int i=1; i<length && buckets[key_mod_n].freq1[index]; i++)
                    if(buckets[key_mod_n].freq1[i] < buckets[key_mod_n].freq1[index])
                        index = i;
            }
            buckets[key_mod_n].freq1[index] += freq;
            buckets[key_mod_n].delta1[index] = time;
            buckets[key_mod_n].keys1[index] = key;
        }
        else{
            if constexpr(use_simd) {
                const __m512 *freqs = (__m512*)(buckets[key_mod_n].freq);
                
                __m256i low_256 = _mm256_castps_si256(_mm512_extractf32x8_ps(*freqs, 0));
                __m256i high_256 = _mm256_castps_si256(_mm512_extractf32x8_ps(*freqs, 1));
                __m256i x_256 = _mm256_min_epi32(low_256, high_256);

                __m128i low_128 = _mm_castps_si128(_mm256_extractf128_ps((__m256)x_256, 0));
                __m128i high_128 = _mm_castps_si128(_mm256_extractf128_ps((__m256)x_256, 1));
                __m128i x_128 = _mm_min_epi32(low_128, high_128);

                __m128i min1 = _mm_shuffle_epi32(x_128, _MM_SHUFFLE(0,0,3,2));
                __m128i min2 = _mm_min_epi32(x_128, min1);
                __m128i min3 = _mm_shuffle_epi32(min2, _MM_SHUFFLE(0,0,0,1));
                __m128i min4 = _mm_min_epi32(min2, min3);
                int min_freq = _mm_cvtsi128_si32(min4);
                
                const __m512i freq_item = _mm512_set1_epi32(min_freq);
                int freq_matched = _mm512_cmpeq_epi32_mask(freq_item, (__m512i)*freqs);
                index = __tzcnt_u32((uint32_t)freq_matched);
            }
            else{
                for(int i=1; i<length && buckets[key_mod_n].freq[index]; i++)
                    if(buckets[key_mod_n].freq[i] < buckets[key_mod_n].freq[index])
                        index = i;
            }
            buckets[key_mod_n].freq[index] += freq;
            buckets[key_mod_n].delta[index] = time;
            buckets[key_mod_n].keys[index] = key;
        }
    }
public:
    SpaceSaving(int memory2, int use_simd=0) : capacity(memory2/sizeof(Bucket)*2){
        buckets = new(std::align_val_t{64}) Bucket [capacity/2];
        memset(buckets, 0, capacity/2*sizeof(Bucket));
    }
    ~SpaceSaving(){
        delete [] buckets;
    }
    vector<pair<pair<int, int16_t>, int>> get_top_k(int k) const{
        vector<pair<pair<int, int16_t>, int>> ans;
        priority_queue<pair<int, pair<int, int16_t> >,vector<pair<int, pair<int, int16_t>>>,greater<pair<int, pair<int, int16_t>>>> Q;
        for(int i=0; i<capacity/2; ++i)
            for(int j=0; j<length; ++j){
                if(buckets[i].freq[j]){
                    Q.push(make_pair(buckets[i].freq[j], make_pair(buckets[i].keys[j],buckets[i].delta[j])));   
                    if(Q.size() > k) Q.pop();
                }
                if(buckets[i].freq1[j]){
                    Q.push(make_pair(buckets[i].freq1[j], make_pair(buckets[i].keys1[j],buckets[i].delta1[j])));
                    if(Q.size() > k) Q.pop();
                }
            }
        int Max=0;
        while(Q.size()){
            pair<int, int16_t> key = Q.top().second;
            int count = Q.top().first;
            ans.push_back(make_pair(key, count));
            Q.pop();
        }
        return ans; 
    }
};
#endif //_SPACESAVINGTOPK_H_