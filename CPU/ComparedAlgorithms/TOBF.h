#ifndef _TOBF_H_
#define _TOBF_H_

#include <cstring>

#include "../lib/murmur3.h"

template <bool use_counter = false>
class TOBF {
    float* last_time;
    int* last_size = nullptr;
    double time_threshold;
    int hash_num, tot, seed;

    static constexpr size_t getSizePerValue() {
        constexpr size_t base_size = sizeof(float);
        if constexpr (use_counter)
            return base_size + sizeof(int);
        else
            return base_size;
    }

public:
    TOBF(uint32_t memory, double time_threshold, int hash_num, int seed)
        : time_threshold(time_threshold), hash_num(hash_num), seed(seed) {
        tot = memory / getSizePerValue();
        last_time = new float[tot] {};
        if constexpr (use_counter) {
            last_size = new int[tot] {};
            memset(last_size, -1, tot * sizeof(int));
        }
    }
    ~TOBF() {
        delete[] last_time;
        delete[] last_size;
    }
    bool insert(int key, float time);
    /// @brief returns the item count of the batch excluding current item.
    /// @param key the key of the item
    /// @param time current timestamp
    /// @warning insert_cnt is supported only when counter is enabled
    /// @return the item count of the batch, 0 if current item is new.
    int insert_cnt(int key, float time);
};

template <>
bool TOBF<false>::insert(int key, float time) {
    bool is_new = 0;
    for (int i = 0; i < hash_num; ++i) {
        int pos = MurmurHash3_x86_32((const char*)(&key), sizeof(int), seed * hash_num + i) % tot;
        is_new |= time - last_time[pos] >= time_threshold;
        last_time[pos] = time;
    }
    return is_new;
}

template <>
int TOBF<true>::insert_cnt(int key, float time) {
    int min_cnt = 0x3f3f3f3f;
    for (int i = 0; i < hash_num; ++i) {
        int pos = MurmurHash3_x86_32((const char*)(&key), sizeof(int), seed * hash_num + i) % tot;
        if (time - last_time[pos] > time_threshold)
            min_cnt = last_size[pos] = 0;
        else
            min_cnt = min(min_cnt, ++last_size[pos]);
        last_time[pos] = time;
    }
    return min_cnt;
}

template <>
bool TOBF<true>::insert(int key, float time) {
    return insert_cnt(key, time) == 0;
}

#endif //_TOBF_H_
