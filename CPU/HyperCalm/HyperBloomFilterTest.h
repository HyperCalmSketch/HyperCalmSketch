#ifndef HYPER_BLOOM_FILTER_REF_H
#define HYPER_BLOOM_FILTER_REF_H

#include <random>
#include <algorithm>

namespace HyperBF {

// Test mode HBF mimics the behavior of the original HBF,
// but use a whole 64-bit to store a cell.
// It's used for testing the accuracy of the HBF,
// meanwhile also useful for understanding the original algorithm.
template <size_t CellBits = 2,
          size_t CounterCellBits = 0,
          bool UseHeader = true,
          size_t _MaxReportSize = ((1 << CounterCellBits) - !UseHeader),
          size_t _CellPerBucket = sizeof(uint64_t) * 8 / CellBits>
class TestModeHyperBF {
    static constexpr size_t CellPerBucket = _CellPerBucket;
    static constexpr size_t TableNum = 8;
public:
    uint64_t* buckets;
    uint64_t* counters;
    double time_threshold;
    uint32_t bucket_num;
    uint32_t seeds[TableNum + 1];
    static constexpr size_t MaxReportSize = _MaxReportSize;

    TestModeHyperBF(uint32_t memory, double time_threshold, int seed = 123) :
        time_threshold(time_threshold), bucket_num(
            memory / sizeof(uint64_t) * CellBits / (CounterCellBits + CellBits)) {
        bucket_num -= bucket_num % TableNum;
        buckets = new (align_val_t { 64 }) uint64_t[bucket_num * CellPerBucket] {};
        counters = new (align_val_t { 64 }) uint64_t[bucket_num * CellPerBucket] {};
        mt19937 rng(seed);
        for (int i = 0; i <= TableNum; ++i) {
            seeds[i] = rng();
        }
        if (memory >= 1024)
            printf("Memory = %.1f KB\t (Memory used in HyperBF)\n", memory / 1000.0);
        else
            printf("Memory = %u B\t (Memory used in HyperBF)\n", memory);
        printf("d = %d\t (Number of mimic arrays in HyperBF)\n", bucket_num);
    }

    ~TestModeHyperBF() {
        delete[] buckets;
        delete[] counters;
    }

    template <bool CM2Count = false>
    int insert_cnt(int key, double time) {
        int first_bucket_pos = CalculatePos(key, TableNum) % bucket_num & ~(TableNum - 1);
        int min_cnt = MaxReportSize;
        vector<int> cnts;
        for (int i = 0; i < TableNum; ++i) {
            int cell_pos = CalculatePos(key, i) % CellPerBucket;
            int bucket_pos = (first_bucket_pos + i) * CellPerBucket;

            int now_tag = int(time / time_threshold + 1.0 * i / TableNum) % 3 + 1;
            int ban_tag = now_tag % 3 + 1;

            for (int j = 0; j < CellPerBucket; ++j) {
                if (buckets[bucket_pos + j] == ban_tag) {
                    buckets[bucket_pos + j] = 0;
                    counters[bucket_pos + j] = 0;
                }
            }
            bool with_header = false;
            if constexpr (UseHeader)
                with_header = buckets[bucket_pos + cell_pos];
            buckets[bucket_pos + cell_pos] = now_tag;
            int cnt = counters[bucket_pos + cell_pos];
            min_cnt = min(min_cnt, cnt + with_header);
            if constexpr (CM2Count) {
                int delta = CalculatePos(key, i + 1) % 2 ? 1 : -1;
                cnts.push_back(cnt * delta);
                cnt += delta;
                cnt = max(-int(MaxReportSize), min(cnt, int(MaxReportSize)));
                counters[bucket_pos + cell_pos] = cnt;
                continue;
            }
            if constexpr (UseHeader) {
                if (!with_header) continue;
            }
            if (cnt + with_header != MaxReportSize) {
                ++counters[bucket_pos + cell_pos];
            }
        }
        if constexpr (CM2Count) {
            sort(cnts.begin(), cnts.end());
            int cnt = cnts[cnts.size() / 2];
            return max(0, min(cnt, int(MaxReportSize)));
        }
        return min_cnt;
    }
    bool insert(int key, double time) {
        return insert_cnt(key, time) == 0;
    }

private:
    inline uint32_t CalculatePos(uint32_t key, int i) {
        return (key * seeds[i]) >> 15;
    }
};

}; // namespace HyperBF

#endif // HYPER_BLOOM_FILTER_REF_H
