#ifndef _CLOCKSKETCH_H_
#define _CLOCKSKETCH_H_

#include <cstring>
#include <random>

template <bool use_counter = false>
class ClockSketch {
protected:
    static constexpr size_t TableNum = 4;

public:
    uint64_t* buckets;
    uint64_t* counters;
    double time_threshold;
    int64_t la_time;
    uint32_t bucket_num, la_pos;
    uint32_t seeds[TableNum + 1];

protected:
    static constexpr size_t CellBits = 4;
    static constexpr size_t CellsPerBucket = (sizeof(*buckets) * 8 / CellBits);
    static constexpr uint64_t CellMask = (1 << CellBits) - 1;
    static_assert(CellBits > 0 && CellBits <= sizeof(*buckets) * 8);

    static constexpr size_t getSizePerBucket() {
        constexpr size_t base_size = sizeof(*buckets);
        if constexpr (use_counter)
            return base_size + sizeof(*counters);
        else
            return base_size;
    }

public:
    /// @brief The maximum report size of the sketch.
    static constexpr size_t MaxReportSize = use_counter ? CellMask + 1 : 1;

    ClockSketch(uint32_t memory, double time_threshold, int seed = 233)
        : counters(nullptr), time_threshold(time_threshold), la_time(0), la_pos(0) {
        bucket_num = memory / getSizePerBucket();
        buckets = new uint64_t[bucket_num] {};
        if constexpr (use_counter) {
            counters = new uint64_t[bucket_num] {};
        }
        std::mt19937 rng(seed);
        for (int i = 0; i <= TableNum; ++i) {
            seeds[i] = rng();
        }
        printf("%d\t (Number of arrays in Clock-Sketch)\n", bucket_num);
    }

    ~ClockSketch() {
        delete[] buckets;
        delete[] counters;
    }

    bool insert(int key, double time);
    /// @brief returns the item count of the batch excluding current item.
    /// @param key the key of the item
    /// @param time current timestamp
    /// @attention report size is limited by MaxReportSize @see ClockSketch::MaxReportSize
    /// @return the item count of the batch, 0 if current item is new.
    int insert_cnt(int key, double time);

protected:
    /// @brief update time and clear expired cells.
    /// @param time current timestamp
    void updateTime(double time);

private:
    static constexpr uint64_t oneForEachCell(size_t cellbit) {
        if (cellbit == 0 || cellbit > sizeof(uint64_t) * 8)
            throw std::invalid_argument("cellbit must be in [1, 64] and be a power of 2");
        if (cellbit == sizeof(uint64_t) * 8)
            return 0x1;
        return (oneForEachCell(cellbit * 2) << cellbit) | oneForEachCell(cellbit * 2);
    }

    inline uint32_t CalculatePos(uint32_t key, int i) {
        return (key * seeds[i]) >> 15;
    }
};

template <bool use_counter>
void ClockSketch<use_counter>::updateTime(double time) {
    int64_t now = time * (CellMask - 1) * (bucket_num * CellsPerBucket) / time_threshold;
    if (!la_time) {
        la_time = now;
        return;
    }
    int d = now - la_time;
    if (d >= (int)bucket_num * CellsPerBucket * CellBits) {
        memset(buckets, 0, bucket_num * sizeof(*buckets));
        la_time = now;
        return;
    }
    for (; d > 0 && la_pos % CellsPerBucket; --d) {
        uint64_t one = uint64_t(1) << (la_pos % CellsPerBucket * CellBits);
        if (buckets[la_pos / CellsPerBucket] & one * CellMask)
            buckets[la_pos / CellsPerBucket] -= one;
        (++la_pos) %= (bucket_num * CellsPerBucket);
    }
    constexpr uint64_t onePerCell = oneForEachCell(CellBits);
    for (; d >= CellsPerBucket; d -= CellsPerBucket) {
        uint64_t x = buckets[la_pos / CellsPerBucket];
        if constexpr (CellBits % 8 == 0) x |= x >> 4;
        if constexpr (CellBits % 4 == 0) x |= x >> 2;
        if constexpr (CellBits % 2 == 0) x |= x >> 1;
        buckets[la_pos / CellsPerBucket] -= x & onePerCell;
        (la_pos += CellsPerBucket) %= (bucket_num * CellsPerBucket);
    }
    for (; d > 0; --d) {
        uint64_t one = uint64_t(1) << (la_pos % CellsPerBucket * CellBits);
        if (buckets[la_pos / CellsPerBucket] & one * CellMask)
            buckets[la_pos / CellsPerBucket] -= one;
        (++la_pos) %= (bucket_num * CellsPerBucket);
    }
    la_time = now;
}

template <>
bool ClockSketch<false>::insert(int key, double time) {
    updateTime(time);
    bool ans = 0;
    for (int i = 0; i < TableNum; ++i) {
        int pos = CalculatePos(key, i) % (bucket_num * CellsPerBucket);
        uint64_t msk = CellMask << (pos % CellsPerBucket * CellBits);
        if ((buckets[pos / CellsPerBucket] & msk) == 0)
            ans = 1;
        buckets[pos / CellsPerBucket] |= msk;
    }
    return ans;
}

template <>
int ClockSketch<false>::insert_cnt(int key, double time) {
    return insert(key, time) ? 0 : MaxReportSize;
}

template <>
int ClockSketch<true>::insert_cnt(int key, double time) {
    updateTime(time);
    int min_cnt = MaxReportSize;
    for (int i = 0; i < TableNum; ++i) {
        int pos = CalculatePos(key, i) % (bucket_num * CellsPerBucket);
        uint64_t one = uint64_t(1) << (pos % CellsPerBucket * CellBits);
        uint64_t msk = one * CellMask;
        bool is_new = !(buckets[pos / CellsPerBucket] & msk);
        buckets[pos / CellsPerBucket] |= msk;
        if (is_new) {
            counters[pos / CellsPerBucket] &= ~msk; // clear counter
            min_cnt = 0;
        } else {
            uint64_t cnt = counters[pos / CellsPerBucket] & msk;
            cnt >>= pos % CellsPerBucket * CellBits;
            if (cnt < CellMask) {
                counters[pos / CellsPerBucket] += one;
            }
            min_cnt = std::min(min_cnt, (int)cnt + 1);
        }

    }
    return min_cnt;
}

template <>
bool ClockSketch<true>::insert(int key, double time) {
    return insert_cnt(key, time) == 0;
}

#endif //_CLOCKSKETCH_H_
