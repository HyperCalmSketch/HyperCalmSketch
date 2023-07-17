#ifndef _HYPERBLOOMFILTER_H_
#define _HYPERBLOOMFILTER_H_

#include <immintrin.h>
#include <random>

namespace HyperBF {

static constexpr uint64_t ODD_BIT_MASK = 0x5555555555555555;
enum Bits : uint64_t {
    _00 = 0x0000000000000000,
    _01 = 0x5555555555555555,
    _10 = 0xaaaaaaaaaaaaaaaa,
    _11 = 0xffffffffffffffff,
};
static constexpr uint64_t STATE_MASKS[] = { Bits::_01, Bits::_10, Bits::_11 };
static constexpr uint64_t MASK[] = { ~Bits::_01, ~Bits::_10, ~Bits::_11 };
static constexpr size_t kStateNum = 3;

enum CounterType {
    None,
    SyncWithBucket,
    FastSync,
};

// HyperBloomFilter is a time-sensitive variant of Bloom Filter.
template <size_t CellBits = 2, CounterType counterType = SyncWithBucket>
class HyperBloomFilter {
protected:
    static constexpr size_t TableNum = 8;

public:
    uint64_t* buckets;
    uint64_t* counters;
    double time_threshold;
    uint32_t bucket_num;
    uint32_t seeds[TableNum + 1];

protected:
    static constexpr size_t counter_type = counterType;
    static constexpr size_t CellPerBucket = sizeof(*buckets) * 8 / CellBits;
    static constexpr uint64_t CellMask = (1 << CellBits) - 1;
    static_assert(kStateNum + 1 <= (1 << CellBits));

    static constexpr size_t getSizePerBucket() {
        constexpr size_t bucket_size = sizeof(*buckets);
        if constexpr(counterType == None)
            return bucket_size;
        else
            return bucket_size + sizeof(*counters);
    }

    static constexpr size_t getMaxReportSize() {
        if constexpr(counterType == None)
            return 1;
        else if constexpr(counterType == FastSync)
            return CellMask;
        else
            return CellMask + 1;
    }

public:
#ifdef SIMD
    static constexpr bool use_simd = true;
#else
    static constexpr bool use_simd = false;
#endif
    static constexpr bool use_counter = counterType != None;

    /// @brief The maximum report size.
    static constexpr size_t MaxReportSize = getMaxReportSize();

    HyperBloomFilter(uint32_t memory, double time_threshold, int seed = 123);
    ~HyperBloomFilter() {
        delete[] buckets;
        delete[] counters;
    }

    /// @brief insert the item and return batch size excluding current item.
    /// @param key the key of the item
    /// @param time current timestamp
    /// @attention report size is limited by MaxReportSize @see HyperBloomFilter::MaxReportSize
    /// @return the item count of the batch, 0 if current item is new.
    int insert_cnt(int key, double time);
    /// @brief insert the item and return whether it's new.
    bool insert(int key, double time);

private:
    inline uint32_t CalculatePos(uint32_t key, int i) {
        return (key * seeds[i]) >> 15;
    }
};


template <size_t CellBits, CounterType counterType>
HyperBloomFilter<CellBits, counterType>::HyperBloomFilter(
    uint32_t memory, double time_threshold, int seed
) : counters(nullptr), time_threshold(time_threshold) {
    bucket_num = memory / getSizePerBucket();
    bucket_num -= bucket_num % TableNum;
    buckets = new (std::align_val_t { 64 }) uint64_t[bucket_num] {};
    if constexpr(use_counter) {
        counters = new (std::align_val_t { 64 }) uint64_t[bucket_num] {};
    }
    std::mt19937 rng(seed);
    for (int i = 0; i <= TableNum; ++i) {
        seeds[i] = rng();
    }
    // if (memory >= 1024)
    //     printf("Memory = %.1f KB\t (Memory used in HyperBF)\n", memory / 1000.0);
    // else
    //     printf("Memory = %u B\t (Memory used in HyperBF)\n", memory);
    // printf("d = %d\t (Number of arrays in HyperBF)\n", bucket_num);
}

template <>
int HyperBloomFilter<2>::insert_cnt(int key, double time) {
    constexpr size_t CellBits = 2;
    int first_bucket_pos = CalculatePos(key, TableNum) % bucket_num & ~(TableNum - 1);
    int min_cnt = MaxReportSize, max_cnt = 0;
    if constexpr(use_simd) {
        static_assert(TableNum % 8 == 0);
        #define _update_cnt(cnt, i) if constexpr (TableNum == 8) \
            { cnt = (i); } else { cnt = std::min(cnt, (i)); }
        #define _generate_vector(gen) _mm512_set_epi64( \
            gen(7), gen(6), gen(5), gen(4), \
            gen(3), gen(2), gen(1), gen(0) \
        )
        // Normally, we don't need to take a loop here, indent is unnecessary.
        // For readability, we use a new line to separate the loop body.
        for (int batch_start = 0; batch_start < TableNum; batch_start += 8) {

        double time_base = time / time_threshold + 1.0 * batch_start / TableNum;
        #define _now_tag(i) (int(time_base + 1.0 * i / TableNum) % 3 + 1)
        #define _tag_idx(i) (_now_tag(i) % 3)
        #define _msk(i) (STATE_MASKS[_tag_idx(i)])
        __m512i* x = (__m512i*)(buckets + first_bucket_pos + batch_start);
        __m512i cache = *x;
        __m512i is_ban_bits = _generate_vector(_msk);
        is_ban_bits ^= cache;
        is_ban_bits |= is_ban_bits >> 1;
        is_ban_bits &= ODD_BIT_MASK; // broadcast by default
        __m512i mask = is_ban_bits | (is_ban_bits << 1);
        cache &= mask;
        #define _bits(i) ((CalculatePos(key, i) % CellPerBucket) * CellBits)
        __m512i move_bits = _generate_vector(_bits);
        __m512i now_tags = _generate_vector(_now_tag);
        if constexpr(counter_type == None) {
            __m512i old_tags = cache & (CellMask << move_bits); // broadcast and vectorize
            if (_mm512_reduce_min_epu64(old_tags) == 0)
                min_cnt = 0;
            *x = cache ^ old_tags ^ (now_tags << move_bits); // vectorized shift
        } else if constexpr(counter_type == FastSync) {
            cache &= ~(CellMask << move_bits);
            cache |= now_tags << move_bits;
            *x = cache;

            x = (__m512i*)(counters + first_bucket_pos + batch_start);
            cache = *x;
            cache &= mask;
            cache = _mm512_rorv_epi64(cache, move_bits);
            __m512i cnts = cache & CellMask;
            _update_cnt(min_cnt, int(_mm512_reduce_min_epu64(cnts)));
            __mmask8 add_mask = _mm512_cmpneq_epi64_mask(cnts, _mm512_set1_epi64(CellMask));
            __m512i new_cnts = _mm512_mask_add_epi64(cnts, add_mask,
                                                     cnts, _mm512_set1_epi64(1));
            *x = _mm512_rolv_epi64(cache ^ cnts ^ new_cnts, move_bits);
        } else {
            __m512i old_tags = cache & (CellMask << move_bits);
            __mmask8 with_header = _mm512_cmpneq_epi64_mask(old_tags, _mm512_set1_epi64(0));
            *x = cache ^ old_tags ^ (now_tags << move_bits);

            x = (__m512i*)(counters + first_bucket_pos + batch_start);
            cache = *x;
            cache &= mask;
            cache = _mm512_rorv_epi64(cache, move_bits);
            __m512i cnts = cache & CellMask;
            __m512i real_cnts = _mm512_maskz_add_epi64(
                with_header, cnts, _mm512_set1_epi64(1));
            _update_cnt(min_cnt, int(_mm512_reduce_min_epu64(real_cnts)));
            __mmask8 max_mask = _mm512_cmpeq_epi64_mask(cnts, _mm512_set1_epi64(CellMask));
            __m512i new_cnts = _mm512_mask_set1_epi64(real_cnts, max_mask, CellMask);
            *x = _mm512_rolv_epi64(cache ^ cnts ^ new_cnts, move_bits);
        }
        #undef _bits
        #undef _now_tag
        #undef _tag_idx
        #undef _msk

        } // end of loop
        #undef _generate_vector
        #undef _update_cnt
    } else {
        for (int i = 0; i < TableNum; ++i) {
            int cell_pos = CalculatePos(key, i) % CellPerBucket;
            int bucket_pos = (first_bucket_pos + i);

            int now_tag = int(time / time_threshold + 1.0 * i / TableNum) % 3 + 1;
            int ban_tag_m1 = now_tag % 3;

            uint64_t is_ban_bits = buckets[bucket_pos] ^ STATE_MASKS[ban_tag_m1];
            // 1 = any(not same), 0 = all(same)
            is_ban_bits |= is_ban_bits >> 1;
            is_ban_bits &= ODD_BIT_MASK;
            // if all(same), clear
            uint64_t mask = is_ban_bits | (is_ban_bits << 1);
            uint64_t bucket = buckets[bucket_pos];
            bucket &= mask;

            auto move_bits = CellBits * cell_pos;
            if constexpr(counter_type == None) {
                int old_tag = (bucket >> move_bits) & CellMask;
                if (old_tag == 0)
                    min_cnt = 0;
                bucket ^= uint64_t(now_tag ^ old_tag) << move_bits;
                buckets[bucket_pos] = bucket;
            } else {
                bool with_header = false;
                if constexpr(counter_type == SyncWithBucket) {
                    with_header = (bucket & (CellMask << move_bits));
                }
                bucket &= ~(CellMask << move_bits);
                bucket |= uint64_t(now_tag) << move_bits;
                buckets[bucket_pos] = bucket;

                uint64_t counter = counters[bucket_pos];
                counter &= mask;
                if constexpr(counter_type == SyncWithBucket) {
                    if (!with_header) {
                        min_cnt = 0;
                        // leave the counter empty, state will record the header,
                        // so that we can know whether it's a new batch.
                        counters[bucket_pos] = counter;
                        continue;
                    }
                }
                int cnt = (counter >> move_bits) & CellMask;
                min_cnt = std::min(min_cnt, cnt + with_header); // add the header
                if (cnt != CellMask) {
                    counter ^= uint64_t(cnt + 1 ^ cnt) << move_bits;
                }
                counters[bucket_pos] = counter;
            }
        }
    }
    return min_cnt;
}

template <size_t CellBits, CounterType counterType>
bool HyperBloomFilter<CellBits, counterType>::insert(int key, double time) {
    return insert_cnt(key, time) == 0;
}

} // namespace HyperBF

#include "HyperBloomFilterTest.h"

using HyperBF::HyperBloomFilter;

#endif // _HYPERBLOOMFILTER_H_
