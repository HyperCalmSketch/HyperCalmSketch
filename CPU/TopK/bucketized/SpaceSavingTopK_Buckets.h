#ifndef _SPACESAVINGBUCKET_H_
#define _SPACESAVINGBUCKET_H_

#include<queue>
#include<cmath>
#include <immintrin.h>

#define CELL_PER_BUCKET 16

struct Bucket{
public:
    uint32_t keys[CELL_PER_BUCKET];
    uint32_t freq[CELL_PER_BUCKET];
};

#define Mod1(x) ((x%capacity+capacity)%capacity)
class SpaceSavingTopK_Buckets
{
protected:
    const uint32_t capacity;
    const int length = CELL_PER_BUCKET, use_SIMD;
    Bucket * buckets;
    int find(uint32_t key){
        int key_mod_n = Mod1(key);
        if(use_SIMD){
            const __m512i item = _mm512_set1_epi32(key);                                                                                                                                                                                             
            __m512i *keys_p = (__m512i*)(buckets[key_mod_n].keys);
            int matched = 0;
            matched = _mm512_cmpeq_epi32_mask(item, *keys_p);
            if(matched != 0){
                int index = _tzcnt_u32(matched);
                return index + 1;
            }
        }
        else{
            for(int i=0; i<length; i++)
                if(buckets[key_mod_n].keys[i] == key)
                    return i+1;
        }
        return 0;
    }
    void add_counter(uint32_t key, int i, int freq = 1){
        buckets[Mod1(key)].freq[i] += freq;
    }
    void SS_update(uint32_t key, int freq = 1)
    {
        const int key_mod_n = Mod1(key);
        int index = 0;
        if(use_SIMD){
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
                if(buckets[key_mod_n].freq[index] > buckets[key_mod_n].freq[i])
                    index = i;
        }
        buckets[key_mod_n].freq[index] += freq;
        buckets[key_mod_n].keys[index] = key;
    }
public:
    SpaceSavingTopK_Buckets(int memory, int _use_SIMD = 0) : capacity(memory/sizeof(Bucket)), use_SIMD(_use_SIMD){
        //srand(time(0));
        buckets = new(std::align_val_t{64}) Bucket [capacity];
        memset(buckets, 0, capacity*sizeof(Bucket));

    }
    ~SpaceSavingTopK_Buckets(){
        delete buckets;
    }
    void insert(uint32_t key, int freq = 1)
    {
        
        const int key_mod_n = Mod1(key);
        if(use_SIMD){
            const __m512i item = _mm512_set1_epi32(key);                                                                                                                                                                                             
            __m512i *keys_p = (__m512i*)(buckets[key_mod_n].keys);
            int matched = 0;
            matched = _mm512_cmpeq_epi32_mask(item, *keys_p);
            if(matched != 0){
                int index = _tzcnt_u32(matched);
                buckets[key_mod_n].freq[index] += freq;
                return ;
            }
            
            const __m512i zero_item = _mm512_set1_epi32(0);
            matched = _mm512_cmpeq_epi32_mask(zero_item, *keys_p);
            if(matched != 0){
                int index = __tzcnt_u32((uint32_t)matched);
                buckets[key_mod_n].freq[index] += freq;
                buckets[key_mod_n].keys[index] = key;
                return ;
            }
            

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
            int index = __tzcnt_u32((uint32_t)freq_matched);
            buckets[key_mod_n].freq[index] += freq;
            buckets[key_mod_n].keys[index] = key;
            return ;
        }
        else{
            for(int i=0; i<length; i++)
                if(buckets[key_mod_n].keys[i] == key){
                    buckets[key_mod_n].freq[i] += freq;
                    return ;
                }
            int index = 0;
            for(int i=1; i<length && buckets[key_mod_n].freq[index]; i++)
                if(buckets[key_mod_n].freq[index] > buckets[key_mod_n].freq[i])
                    index = i;
            buckets[key_mod_n].freq[index] += freq;
            buckets[key_mod_n].keys[index] = key;
            return ;
        }
        
    }

    int query(uint32_t key){
        const int key_mod_n = Mod1(key);
        const __m512i item = _mm512_set1_epi32(key);                                                                                                                                                                                             
        __m512i *keys_p = (__m512i*)(buckets[key_mod_n].keys);
        int matched = 0;
        matched = _mm512_cmpeq_epi32_mask(item, *keys_p);
        if(matched != 0){
            int index = _tzcnt_u32(matched);
            return buckets[key_mod_n].freq[index];
        }
        return 0;
    }


    vector<pair<int,int>> get_top_k(int k) {
        vector<pair<int,int>> ans;
        priority_queue<pair<int,int>,vector<pair<int,int>>,greater<pair<int,int>> > Q;
        for(int i=0; i<capacity; ++i)
            for(int j=0; j<length; ++j)
                if(buckets[i].freq[j]){
                    Q.push(make_pair(buckets[i].freq[j], buckets[i].keys[j]));
                    if(Q.size() > k) Q.pop();
                }
        
        while(Q.size()){
            int key = Q.top().second;
            int count = Q.top().first;
            //printf("%d\n", count);
            ans.push_back(make_pair(key, count));
            Q.pop();
        }
        return ans; 
    }
};
#endif //_SPACESAVINGTOPK_H_